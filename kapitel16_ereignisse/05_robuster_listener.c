// Kapitel 16: Ein robuster Ereignis-Listener – Neuverbinden, Wachhund, sauberes Beenden
//   ./starte.sh kapitel16_ereignisse/05_robuster_listener.c [--trennen] [--dauer=20]
#define _POSIX_C_SOURCE 200809L
#include <openhab/openhab.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define WACHHUND_MS 25000 // openHAB schickt alle 10 s ein ALIVE

static volatile sig_atomic_t aktiv = 1;
static openhab_sse_t* volatile strom; // für den Signal-Handler

// Wie printf, aber mit Uhrzeit davor: variable Argumente mit <stdarg.h>
static void log_zeile(const char* fmt, ...) {
    char uhr[16];
    time_t t = time(NULL);
    strftime(uhr, sizeof uhr, "%H:%M:%S", localtime(&t));
    printf("%s ", uhr);
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
    printf("\n");
    fflush(stdout);
}

// SIGINT/SIGTERM beenden, SIGALRM (--dauer) ebenso, SIGUSR1 (--trennen) kappt nur
static void signal_handler(int sig) {
    if (sig != SIGUSR1) aktiv = 0;
    if (strom) openhab_sse_interrupt(strom); // ausdrücklich signal-sicher
}

static void* test_trennen(void* arg) { // --trennen: nach 3 s die Leitung kappen
    (void)arg;
    sleep(3);
    log_zeile("TEST: Verbindung gekappt");
    kill(getpid(), SIGUSR1);
    return NULL;
}

static void verarbeiten(const cJSON* e) {
    char name[64];
    sscanf(openhab_json_str(e, "topic"), "openhab/items/%63[^/]", name);
    log_zeile("%s = %s", name, openhab_json_str(e, "payload.value"));
}

int main(int argc, char** argv) {
    const char* url =
        getenv("OPENHAB_URL") ? getenv("OPENHAB_URL") : "http://127.0.0.1:8080";
    openhab_client_t* c = openhab_client_new(url, .token = getenv("OPENHAB_TOKEN"));
    struct sigaction sa = { 0 };
    sa.sa_handler = signal_handler;
    for (int s = 0, sigs[] = { SIGINT, SIGTERM, SIGALRM, SIGUSR1 }; s < 4; s++)
        sigaction(sigs[s], &sa, NULL);
    for (int i = 1; i < argc; i++) {
        pthread_t t;
        if (!strcmp(argv[i], "--trennen"))
            pthread_create(&t, NULL, test_trennen, NULL);
        else if (!strncmp(argv[i], "--dauer=", 8))
            alarm(atoi(argv[i] + 8));
    }

    int pause = 1;
    while (aktiv) {
        openhab_sse_t* s =
            openhab_events_getEvents(c, .topics = "openhab/items/*/statechanged");
        if (openhab_sse_is_error(s)) {
            char text[256];
            snprintf(text, sizeof text, "%s", openhab_sse_error(s));
            text[strcspn(text, "(")] = '\0'; // ohne die Adresse
            for (size_t n = strlen(text); n && text[n - 1] == ' ';) text[--n] = '\0';
            log_zeile("keine Verbindung: %s – neuer Versuch in %d s", text, pause);
            openhab_sse_close(s);
        } else {
            strom = s;
            log_zeile("verbunden");
            pause = 1; // Erfolg: Wartezeit zurücksetzen
            cJSON* e;
            // Wachhund: Kommt WACHHUND_MS lang nichts – nicht einmal ALIVE –, ist die
            // Verbindung tot, obwohl sie offen aussieht
            while ((e = openhab_sse_next_event(s, WACHHUND_MS, 1))) {
                if (openhab_json_get(e, "topic")) verarbeiten(e);
                cJSON_Delete(e);
            }
            if (aktiv && openhab_sse_is_open(s))
                log_zeile("Wachhund: keine Nachricht – Verbindung wird neu aufgebaut");
            else if (aktiv)
                log_zeile("Verbindung beendet");
            strom = NULL;
            openhab_sse_close(s);
        }
        if (!aktiv) break;
        sleep(pause);                            // ein Signal weckt sleep() vorzeitig
        pause = pause * 2 > 30 ? 30 : pause * 2; // exponentielles Backoff bis 30 s
    }
    log_zeile("Listener beendet");
    openhab_client_destroy(c);
    return 0;
}

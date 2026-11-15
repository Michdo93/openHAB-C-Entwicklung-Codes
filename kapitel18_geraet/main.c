// Kapitel 18: ohgeraet – der Raspberry Pi als Gerät für openHAB
//   ohgeraet [--intervall=30] [--ausgang=attrappe | --ausgang=gpiochip0:17] [--dauer=s]
// Ein einziger Thread: Das Warten auf Commands IST der Takt. openhab_sse_next_event
// wartet höchstens bis zur nächsten fälligen Messung.
#define _POSIX_C_SOURCE 200809L
#include <openhab/openhab.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "geraet.h"

static volatile sig_atomic_t aktiv = 1;
static openhab_sse_t* volatile strom;

static void beenden(int sig) {
    (void)sig;
    aktiv = 0;
    if (strom) openhab_sse_interrupt(strom);
}

static void log_zeile(const char* text) {
    char uhr[16];
    time_t t = time(NULL);
    strftime(uhr, sizeof uhr, "%H:%M:%S", localtime(&t));
    printf("%s %s\n", uhr, text);
    fflush(stdout); // journald sieht jede Zeile sofort
}

static double jetzt(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec / 1e9;
}

static void messen(geraet* g) {
    messung m = messwerte_lesen(&g->netz_bytes, &g->netz_zeit);
    geraet_melden(g, &m);
    char text[160], temp[16] = "–"; // ohne Sensor (z. B. in einer VM)
    if (m.temperatur > -273) snprintf(temp, sizeof temp, "%.1f °C", m.temperatur);
    snprintf(text, sizeof text,
             "gemeldet: %s, Last %.0f %%, Speicher %.0f %%, Netz %.1f kbit/s", temp, m.last,
             m.speicher, m.netz_kbit < 0 ? 0 : m.netz_kbit);
    log_zeile(text);
}

int main(int argc, char** argv) {
    const char* beschreibung = "attrappe";
    int intervall = 30, dauer = 0;
    for (int i = 1; i < argc; i++) {
        if (!strncmp(argv[i], "--intervall=", 12))
            intervall = atoi(argv[i] + 12);
        else if (!strncmp(argv[i], "--ausgang=", 10))
            beschreibung = argv[i] + 10;
        else if (!strncmp(argv[i], "--dauer=", 8))
            dauer = atoi(argv[i] + 8);
    }
    struct sigaction sa = { 0 };
    sa.sa_handler = beenden;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL); // systemctl stop
    sigaction(SIGALRM, &sa, NULL);
    if (dauer) alarm(dauer);

    const char* url =
        getenv("OPENHAB_URL") ? getenv("OPENHAB_URL") : "http://127.0.0.1:8080";
    geraet g = { .client = openhab_client_new(url, .token = getenv("OPENHAB_TOKEN")),
                 .luefter = ausgang_oeffnen(beschreibung),
                 .intervall = intervall,
                 .netz_bytes = -1 };
    if (!g.luefter) return 1;
    char rechner[64] = "?", text[320];
    gethostname(rechner, sizeof rechner);

    int pause = 1;
    while (aktiv) {
        // (Neu-)Anmelden und den Strom der Commands an die eigenen Items öffnen
        if (geraet_anmelden(&g, rechner) < 0) goto warten;
        openhab_sse_t* s = openhab_events_getEvents(
            g.client,
            .topics = "openhab/items/Pi_Luefter/command,openhab/items/Pi_Aktion/command");
        if (openhab_sse_is_error(s)) {
            snprintf(text, sizeof text, "kein Ereignisstrom: %s", openhab_sse_error(s));
            log_zeile(text);
            openhab_sse_close(s);
            goto warten;
        }
        strom = s;
        snprintf(text, sizeof text, "angemeldet als gPi (%s), Ausgang %s, alle %d s",
                 rechner, beschreibung, intervall);
        log_zeile(text);
        pause = 1;
        double faellig = jetzt();
        while (aktiv && openhab_sse_is_open(s)) {
            if (jetzt() >= faellig) {
                messen(&g);
                faellig = jetzt() + intervall;
            }
            int warten_ms = (int)((faellig - jetzt()) * 1000);
            cJSON* e = openhab_sse_next_event(s, warten_ms > 0 ? warten_ms : 0, 0);
            if (!e) continue; // Takt erreicht oder Strom zu Ende
            char item[64], antwort[192];
            sscanf(openhab_json_str(e, "topic"), "openhab/items/%63[^/]", item);
            const char* befehl = openhab_json_str(e, "payload.value");
            if (befehl && geraet_command(&g, item, befehl, antwort, sizeof antwort)) {
                snprintf(text, sizeof text, "Command %s <- %s: %s", item, befehl, antwort);
                log_zeile(text);
            }
            cJSON_Delete(e);
        }
        strom = NULL;
        openhab_sse_close(s);
        if (!aktiv) break;
        log_zeile("Verbindung verloren");
    warten:
        if (!aktiv) break;
        sleep(pause);
        pause = pause * 2 > 60 ? 60 : pause * 2;
    }
    geraet_abmelden(&g);
    log_zeile("beendet, Pi_Online = OFF");
    ausgang_schliessen(g.luefter);
    openhab_client_destroy(g.client);
    return 0;
}

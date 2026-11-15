// Kapitel 16: Der State-Tracker – nur ausgewählte Items, fertig formatiert
#include <openhab/openhab.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static openhab_client_t* c;

static int breite(const char* s, int zeichen) { // printf zählt Bytes, nicht Zeichen
    for (; s && *s; s++)
        if ((*s & 0xC0) == 0x80) zeichen++;
    return zeichen;
}
static char verbindung[64];

static void beobachten(const char* a, const char* b) { // die Item-Liste der Verbindung
    cJSON* liste = cJSON_CreateStringArray((const char*[]){ a, b }, 2);
    cJSON_Delete(openhab_events_updateSSEConnectionItems(c, verbindung, liste));
    cJSON_Delete(liste);
}

static void* bedienen(void* strom) {
    sleep(1);
    cJSON_Delete(openhab_items_sendCommand(c, "Wohnzimmer_Dimmer", "55"));
    cJSON_Delete(openhab_items_postUpdate(c, "Werkstatt_Temperatur", "19.3 °C")); // nicht
    sleep(1); // abonniert
    printf("--- Ansicht gewechselt: neue Item-Liste\n");
    fflush(stdout);
    beobachten("Werkstatt_Temperatur", "Werkstatt_Feuchte");
    sleep(1);
    cJSON_Delete(openhab_items_postUpdate(c, "Werkstatt_Temperatur", "19.8 °C"));
    sleep(1);
    openhab_sse_interrupt(strom);
    return NULL;
}

int main(void) {
    const char* url =
        getenv("OPENHAB_URL") ? getenv("OPENHAB_URL") : "http://127.0.0.1:8080";
    c = openhab_client_new(url, .token = getenv("OPENHAB_TOKEN"));
    openhab_sse_t* strom = openhab_events_initiateStateTracker(c);
    pthread_t t;
    int gestartet = 0;
    char* zeile;
    while ((zeile = openhab_sse_next_line(strom, -1))) {
        if (!gestartet && !strncmp(zeile, "data:", 5)) {
            // 1. Nachricht: die Verbindungskennung als reiner Text
            snprintf(verbindung, sizeof verbindung, "%s", zeile + 5 + (zeile[5] == ' '));
            printf("Verbindung: %s\n", verbindung);
            beobachten("Wohnzimmer_Dimmer", "Wohnzimmer_Temperatur");
            pthread_create(&t, NULL, bedienen, strom);
            gestartet = 1;
        } else {
            cJSON* daten = openhab_sse_parse_data(zeile); // {Item: {state, displayState}}
            cJSON* z;
            if (daten && !openhab_json_get(daten, "type")) // ALIVE überspringen
                cJSON_ArrayForEach(z, daten) {
                    const char* anzeige = openhab_json_str(z, "displayState");
                    const char* state = openhab_json_str(z, "state");
                    printf("  %-22s state=%-*s anzeige=%s\n", z->string, breite(state, 12),
                           state, anzeige ? anzeige : openhab_json_str(z, "state"));
                }
            cJSON_Delete(daten);
        }
        openhab_free(zeile);
    }
    if (gestartet) pthread_join(t, NULL);
    openhab_sse_close(strom);
    openhab_client_destroy(c);
    return 0;
}

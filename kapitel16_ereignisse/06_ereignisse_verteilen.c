// Kapitel 16: Ein Strom für das ganze Programm – Ereignisse an Beobachter verteilen
#include <openhab/openhab.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Ein Beobachter: Funktion plus eigener Kontext (in C der Ersatz für ein Lambda)
typedef void (*beobachter_fn)(const char* item, const cJSON* payload, void* kontext);
typedef struct {
    const char* item; // Itemname oder "*" für alle
    beobachter_fn fn;
    void* kontext;
} beobachter;

typedef struct {
    openhab_sse_t* strom;
    beobachter liste[16];
    int anzahl;
} verteiler;

static void anmelden(verteiler* v, const char* item, beobachter_fn fn, void* kontext) {
    if (v->anzahl < 16) v->liste[v->anzahl++] = (beobachter){ item, fn, kontext };
}

static void* lesen(void* arg) { // der einzige Leser des Stroms
    verteiler* v = arg;
    cJSON* e;
    while ((e = openhab_sse_next_event(v->strom, -1, 0))) {
        char name[64];
        sscanf(openhab_json_str(e, "topic"), "openhab/items/%63[^/]", name);
        for (int i = 0; i < v->anzahl; i++)
            if (!strcmp(v->liste[i].item, name) || !strcmp(v->liste[i].item, "*"))
                v->liste[i].fn(name, openhab_json_get(e, "payload"), v->liste[i].kontext);
        cJSON_Delete(e);
    }
    return NULL;
}

static void anzeigen(const char* item, const cJSON* p, void* beschriftung) {
    (void)item;
    printf("%-16s %s -> %s\n", (const char*)beschriftung, openhab_json_str(p, "oldValue"),
           openhab_json_str(p, "value"));
}

static void zaehlen(const char* item, const cJSON* p, void* zaehler) {
    (void)item;
    (void)p;
    (*(int*)zaehler)++;
}

int main(void) {
    const char* url =
        getenv("OPENHAB_URL") ? getenv("OPENHAB_URL") : "http://127.0.0.1:8080";
    openhab_client_t* c = openhab_client_new(url, .token = getenv("OPENHAB_TOKEN"));

    verteiler v = { .strom = openhab_item_events_ItemStateChangedEvent(c) };
    int anzahl = 0;
    anmelden(&v, "Wohnzimmer_Licht", anzeigen, "Deckenlicht");
    anmelden(&v, "Wohnzimmer_Dimmer", anzeigen, "Leselampe");
    anmelden(&v, "Wohnzimmer_Dimmer", anzeigen, "Leselampe (2)"); // derselbe Strom
    anmelden(&v, "*", zaehlen, &anzahl);

    pthread_t leser;
    pthread_create(&leser, NULL, lesen, &v);
    sleep(1);
    cJSON_Delete(openhab_items_sendCommand(c, "Wohnzimmer_Licht", "OFF"));
    cJSON_Delete(openhab_items_sendCommand(c, "Wohnzimmer_Dimmer", "45"));
    sleep(3);
    openhab_sse_interrupt(v.strom); // Leser aufwecken und beenden
    pthread_join(leser, NULL);
    openhab_sse_close(v.strom);
    printf("%d Ereignisse über einen einzigen Strom verteilt\n", anzahl);
    openhab_client_destroy(c);
    return 0;
}

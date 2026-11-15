// Kapitel 16: Den Ereignisstrom direkt lesen – Zeile für Zeile
#include <openhab/openhab.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static openhab_client_t* c;

static void* dimmer_verstellen(void* arg) { // nach einer Sekunde etwas ändern
    (void)arg;
    sleep(1);
    char wert[8];
    snprintf(wert, sizeof wert, "%d", 20 + (int)(time(NULL) % 60));
    cJSON_Delete(openhab_items_sendCommand(c, "Wohnzimmer_Dimmer", wert));
    return NULL;
}

int main(void) {
    const char* url =
        getenv("OPENHAB_URL") ? getenv("OPENHAB_URL") : "http://127.0.0.1:8080";
    c = openhab_client_new(url, .token = getenv("OPENHAB_TOKEN"));

    // Alle Zustandsänderungen aller Items (Topic openhab/items/*/statechanged)
    openhab_sse_t* strom = openhab_item_events_ItemStateChangedEvent(c);
    if (openhab_sse_is_error(strom)) {
        fprintf(stderr, "%s\n", openhab_sse_error(strom));
        return 1;
    }
    pthread_t t;
    pthread_create(&t, NULL, dimmer_verstellen, NULL);
    printf("Warte auf Ereignisse …\n");

    time_t ende = time(NULL) + 15; // 15 Sekunden lang lesen
    while (time(NULL) < ende) {
        char* zeile = openhab_sse_next_line(strom, 1000); // höchstens 1 s warten
        if (!zeile) {
            if (!openhab_sse_is_open(strom)) break; // Strom zu Ende
            continue;                               // nur Zeit abgelaufen
        }
        printf("roh: %.70s\n", zeile);
        // 1. JSON: das Ereignis; die Bibliothek entpackt auch payload (2. JSON)
        cJSON* e = openhab_sse_parse_data(zeile); // NULL für event:, id:, ""
        const char* topic = openhab_json_str(e, "topic");
        if (topic) { // ALIVE hat kein topic
            char name[64];
            sscanf(topic, "openhab/items/%63[^/]", name);
            printf("  -> %s: %s -> %s\n", name, openhab_json_str(e, "payload.oldValue"),
                   openhab_json_str(e, "payload.value"));
        }
        cJSON_Delete(e);
        openhab_free(zeile);
    }
    openhab_sse_close(strom);
    pthread_join(t, NULL);
    printf("beendet\n");
    openhab_client_destroy(c);
    return 0;
}

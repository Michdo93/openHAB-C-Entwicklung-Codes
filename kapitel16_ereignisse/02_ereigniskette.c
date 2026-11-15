// Kapitel 16: Welche Ereignisse löst ein einziger Command aus?
#include <openhab/openhab.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ZIEL "Wohnzimmer_Licht"

static openhab_client_t* c;

static void* schalten(void* arg) {
    (void)arg;
    sleep(1);
    cJSON* s = openhab_items_getItemState(c, ZIEL);
    const char* neu = strcmp(s->valuestring, "ON") ? "ON" : "OFF";
    printf(">>> sende Command %s\n\n", neu);
    fflush(stdout);
    cJSON_Delete(openhab_items_sendCommand(c, ZIEL, neu));
    cJSON_Delete(s);
    return NULL;
}

int main(void) {
    const char* url =
        getenv("OPENHAB_URL") ? getenv("OPENHAB_URL") : "http://127.0.0.1:8080";
    c = openhab_client_new(url, .token = getenv("OPENHAB_TOKEN"));

    // Alle Ereignisse dieses Items: Topic openhab/items/Wohnzimmer_Licht/*
    openhab_sse_t* strom =
        openhab_events_getEvents(c, .topics = "openhab/items/" ZIEL "/*");
    pthread_t t;
    pthread_create(&t, NULL, schalten, NULL);
    cJSON* e;
    // Nach 2 s Stille ist die Kette vorbei: next_event liefert NULL
    while ((e = openhab_sse_next_event(strom, 2000, 0))) {
        const char* topic = openhab_json_str(e, "topic");
        printf("%-24s %-13s %s\n", openhab_json_str(e, "type"), strrchr(topic, '/') + 1,
               openhab_json_str(e, "payload.value"));
        cJSON_Delete(e);
    }
    openhab_sse_close(strom);
    pthread_join(t, NULL);
    openhab_client_destroy(c);
    return 0;
}

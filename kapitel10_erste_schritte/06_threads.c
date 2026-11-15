// Kapitel 10: Mehrere Anfragen gleichzeitig – ein Client, mehrere Threads
#include <openhab/openhab.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct {
    openhab_client_t* client; // ein Client darf von mehreren Threads benutzt werden
    const char* item;
    char state[64];
} auftrag;

static void* lesen(void* arg) {
    auftrag* a = arg;
    cJSON* s = openhab_items_getItemState(a->client, a->item);
    snprintf(a->state, sizeof a->state, "%s",
             cJSON_IsString(s) ? s->valuestring : openhab_error(s));
    cJSON_Delete(s);
    return NULL;
}

static double sekunden(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec / 1e9;
}

int main(void) {
    const char* url =
        getenv("OPENHAB_URL") ? getenv("OPENHAB_URL") : "http://127.0.0.1:8080";
    openhab_client_t* c = openhab_client_new(url, .token = getenv("OPENHAB_TOKEN"));
    const char* namen[] = { "Wohnzimmer_Licht",     "Wohnzimmer_Dimmer",
                            "Wohnzimmer_Farblicht", "Wohnzimmer_Temperatur",
                            "Kueche_Rollladen",     "Flur_Haustuer",
                            "Haus_Status",          "Werkstatt_Temperatur" };
    enum { N = sizeof namen / sizeof namen[0] };
    auftrag auftraege[N];
    pthread_t threads[N];

    double start = sekunden();
    for (int i = 0; i < N; i++) {
        auftraege[i] = (auftrag){ .client = c, .item = namen[i] };
        pthread_create(&threads[i], NULL, lesen, &auftraege[i]);
    }
    for (int i = 0; i < N; i++) pthread_join(threads[i], NULL);
    double parallel = sekunden() - start;

    start = sekunden();
    for (int i = 0; i < N; i++) lesen(&auftraege[i]); // zum Vergleich nacheinander
    double nacheinander = sekunden() - start;

    for (int i = 0; i < N; i++) printf("%-22s = %s\n", namen[i], auftraege[i].state);
    printf("%d Anfragen: parallel %.0f ms, nacheinander %.0f ms\n", N, parallel * 1000,
           nacheinander * 1000);
    openhab_client_destroy(c);
    return 0;
}

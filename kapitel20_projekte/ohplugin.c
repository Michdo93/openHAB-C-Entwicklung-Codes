// Kapitel 20: ohplugin – eine dünne Schicht über c-openhab-rest-client
#include "ohplugin.h"

#include <openhab/openhab.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WARTESCHLANGE 256

struct ohplugin {
    openhab_client_t* client;
    openhab_sse_t* strom;
    pthread_t leser;
    int beobachtet;
    pthread_mutex_t sperre;
    char aenderungen[WARTESCHLANGE]
                    [128]; // Ringpuffer: der Leser schreibt, die Engine liest
    int anfang, anzahl;
    char fehler[256];
};

static int ergebnis(ohplugin* p, cJSON* r) {
    int rc = openhab_is_error(r) ? -1 : 0;
    snprintf(p->fehler, sizeof p->fehler, "%s", rc ? openhab_error(r) : "");
    cJSON_Delete(r);
    return rc;
}

ohplugin* oh_verbinden(const char* url, const char* token) {
    ohplugin* p = calloc(1, sizeof *p);
    if (!p) return NULL;
    p->client = openhab_client_new(url, .token = token);
    if (!openhab_client_is_logged_in(p->client)) {
        openhab_client_destroy(p->client);
        free(p);
        return NULL;
    }
    pthread_mutex_init(&p->sperre, NULL);
    return p;
}

int oh_befehl(ohplugin* p, const char* item, const char* command) {
    return ergebnis(p, openhab_items_sendCommand(p->client, item, command));
}

int oh_melden(ohplugin* p, const char* item, const char* state) {
    return ergebnis(p, openhab_items_postUpdate(p->client, item, state));
}

int oh_zustand(ohplugin* p, const char* item, char* puffer, int groesse) {
    cJSON* r = openhab_items_getItemState(p->client, item);
    if (!cJSON_IsString(r)) return ergebnis(p, r);
    int n = snprintf(puffer, groesse, "%s", r->valuestring);
    cJSON_Delete(r);
    return n;
}

static void* lesen(void* arg) {
    ohplugin* p = arg;
    cJSON* e;
    while ((e = openhab_sse_next_event(p->strom, -1, 0))) {
        char name[64];
        sscanf(openhab_json_str(e, "topic"), "openhab/items/%63[^/]", name);
        pthread_mutex_lock(&p->sperre);
        int platz = (p->anfang + p->anzahl) % WARTESCHLANGE;
        if (p->anzahl == WARTESCHLANGE)
            p->anfang = (p->anfang + 1) % WARTESCHLANGE; // älteste weg
        else
            p->anzahl++;
        snprintf(p->aenderungen[platz], 128, "%s=%s", name,
                 openhab_json_str(e, "payload.value"));
        pthread_mutex_unlock(&p->sperre);
        cJSON_Delete(e);
    }
    return NULL;
}

int oh_beobachten(ohplugin* p, const char* items_komma) {
    if (p->beobachtet) return 0;
    char topics[2048] = "", kopie[1024];
    snprintf(kopie, sizeof kopie, "%s", items_komma);
    for (char* name = strtok(kopie, ","); name; name = strtok(NULL, ",")) {
        size_t l = strlen(topics);
        snprintf(topics + l, sizeof topics - l, "%sopenhab/items/%s/statechanged",
                 l ? "," : "", name);
    }
    p->strom = openhab_events_getEvents(p->client, .topics = topics);
    if (openhab_sse_is_error(p->strom)) {
        snprintf(p->fehler, sizeof p->fehler, "%s", openhab_sse_error(p->strom));
        openhab_sse_close(p->strom);
        p->strom = NULL;
        return -1;
    }
    p->beobachtet = pthread_create(&p->leser, NULL, lesen, p) == 0;
    return p->beobachtet ? 0 : -1;
}

int oh_naechste_aenderung(ohplugin* p, char* puffer, int groesse) {
    int n = 0;
    pthread_mutex_lock(&p->sperre);
    if (p->anzahl > 0) {
        n = snprintf(puffer, groesse, "%s", p->aenderungen[p->anfang]);
        p->anfang = (p->anfang + 1) % WARTESCHLANGE;
        p->anzahl--;
    }
    pthread_mutex_unlock(&p->sperre);
    return n;
}

const char* oh_fehler(ohplugin* p) { return p ? p->fehler : "nicht verbunden"; }

void oh_trennen(ohplugin* p) {
    if (!p) return;
    if (p->beobachtet) {
        openhab_sse_interrupt(p->strom);
        pthread_join(p->leser, NULL);
    }
    openhab_sse_close(p->strom);
    pthread_mutex_destroy(&p->sperre);
    openhab_client_destroy(p->client);
    free(p);
}

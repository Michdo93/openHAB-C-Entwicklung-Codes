// Kapitel 17: Der Controller
#include "controller.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct controller {
    openhab_client_t* client;
    model* m;
    openhab_sse_t* strom;
    pthread_t leser;
    int laeuft;
    char fehler[256];
};

controller* controller_neu(openhab_client_t* client, model* m) {
    controller* c = calloc(1, sizeof *c);
    if (c) {
        c->client = client;
        c->m = m;
    }
    return c;
}

const char* controller_fehler(controller* c) { return c->fehler[0] ? c->fehler : NULL; }

static void merke_fehler(controller* c, cJSON* ergebnis) {
    if (openhab_is_error(ergebnis))
        snprintf(c->fehler, sizeof c->fehler, "%s", openhab_error(ergebnis));
    else
        c->fehler[0] = '\0';
}

// Alle Items einer Gruppe (rekursiv) in das Model übernehmen, Gruppen selbst auslassen
static void uebernehmen(controller* c, const cJSON* mitglieder) {
    const cJSON* i;
    cJSON_ArrayForEach(i, mitglieder) {
        if (!strcmp(openhab_json_str(i, "type"), "Group")) {
            uebernehmen(c, openhab_json_get(i, "members"));
            continue;
        }
        if (model_suche(c->m, openhab_json_str(i, "name")) < 0)
            model_hinzufuegen(c->m, openhab_json_str(i, "name"),
                              openhab_json_str(i, "label"), openhab_json_str(i, "type"),
                              openhab_json_str(i, "state"));
    }
}

int controller_laden(controller* c, const char* gruppe) {
    cJSON* g = openhab_items_getItem(c->client, gruppe, .metadata = "");
    merke_fehler(c, g);
    if (openhab_is_error(g)) {
        cJSON_Delete(g);
        return -1;
    }
    uebernehmen(c, openhab_json_get(g, "members"));
    cJSON_Delete(g);
    return model_anzahl(c->m);
}

static void* lesen(void* arg) {
    controller* c = arg;
    cJSON* e;
    while ((e = openhab_sse_next_event(c->strom, -1, 0))) {
        char name[64];
        if (sscanf(openhab_json_str(e, "topic"), "openhab/items/%63[^/]", name) == 1)
            model_setze_state(c->m, name, openhab_json_str(e, "payload.value"));
        cJSON_Delete(e);
    }
    return NULL;
}

int controller_starten(controller* c) {
    c->strom = openhab_item_events_ItemStateChangedEvent(c->client);
    if (openhab_sse_is_error(c->strom)) {
        snprintf(c->fehler, sizeof c->fehler, "%s", openhab_sse_error(c->strom));
        return -1;
    }
    c->laeuft = pthread_create(&c->leser, NULL, lesen, c) == 0;
    return c->laeuft ? 0 : -1;
}

static void senden(controller* c, const char* name, const char* befehl) {
    cJSON* r = openhab_items_sendCommand(c->client, name, befehl, .source = "ohdash");
    merke_fehler(c, r);
    cJSON_Delete(r);
}

void controller_umschalten(controller* c, int index) {
    eintrag e = model_eintrag(c->m, index);
    if (!strcmp(e.typ, "Switch"))
        senden(c, e.name, strcmp(e.state, "ON") ? "ON" : "OFF");
    else if (!strcmp(e.typ, "Dimmer") || !strcmp(e.typ, "Color"))
        senden(c, e.name, atoi(e.state) > 0 || strchr(e.state, ',') ? "OFF" : "ON");
    else if (!strcmp(e.typ, "Rollershutter"))
        senden(c, e.name, atoi(e.state) > 50 ? "UP" : "DOWN");
    else
        snprintf(c->fehler, sizeof c->fehler, "%s (%s) lässt sich nicht schalten", e.name,
                 e.typ);
}

void controller_aendern(controller* c, int index, int schritt) {
    eintrag e = model_eintrag(c->m, index);
    if (strcmp(e.typ, "Dimmer") && strcmp(e.typ, "Rollershutter")) {
        snprintf(c->fehler, sizeof c->fehler, "%s ist kein Dimmer", e.name);
        return;
    }
    int wert = atoi(e.state) + schritt; // "NULL" ergibt 0
    char befehl[8];
    snprintf(befehl, sizeof befehl, "%d", wert < 0 ? 0 : wert > 100 ? 100 : wert);
    senden(c, e.name, befehl);
}

void controller_stoppen(controller* c) {
    if (!c) return;
    if (c->strom) openhab_sse_interrupt(c->strom); // Leser aufwecken
    if (c->laeuft) pthread_join(c->leser, NULL);
    openhab_sse_close(c->strom);
    free(c);
}

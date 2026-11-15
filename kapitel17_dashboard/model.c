// Kapitel 17: Das Model – thread-sicher durch einen Mutex
#include "model.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_EINTRAEGE  64
#define MAX_BEOBACHTER 8

typedef struct {
    model_beobachter fn;
    void* kontext;
} beobachter_t;

struct model {
    pthread_mutex_t sperre;
    eintrag eintraege[MAX_EINTRAEGE];
    int anzahl;
    beobachter_t beobachter[MAX_BEOBACHTER];
    int beobachter_anzahl;
};

model* model_neu(void) {
    model* m = calloc(1, sizeof *m);
    if (m) pthread_mutex_init(&m->sperre, NULL);
    return m;
}

void model_frei(model* m) {
    if (!m) return;
    pthread_mutex_destroy(&m->sperre);
    free(m);
}

static void kopiere(char* ziel, size_t groesse, const char* text) {
    snprintf(ziel, groesse, "%s", text ? text : "");
}

int model_hinzufuegen(model* m, const char* name, const char* label, const char* typ,
                      const char* state) {
    pthread_mutex_lock(&m->sperre);
    int index = m->anzahl < MAX_EINTRAEGE ? m->anzahl++ : -1;
    if (index >= 0) {
        eintrag* e = &m->eintraege[index];
        kopiere(e->name, sizeof e->name, name);
        kopiere(e->label, sizeof e->label, label);
        kopiere(e->typ, sizeof e->typ, typ);
        kopiere(e->state, sizeof e->state, state);
    }
    pthread_mutex_unlock(&m->sperre);
    return index;
}

int model_anzahl(model* m) {
    pthread_mutex_lock(&m->sperre);
    int n = m->anzahl;
    pthread_mutex_unlock(&m->sperre);
    return n;
}

int model_suche(model* m, const char* name) {
    pthread_mutex_lock(&m->sperre);
    int index = -1;
    for (int i = 0; i < m->anzahl && index < 0; i++)
        if (!strcmp(m->eintraege[i].name, name)) index = i;
    pthread_mutex_unlock(&m->sperre);
    return index;
}

eintrag model_eintrag(model* m, int index) {
    eintrag kopie = { 0 };
    pthread_mutex_lock(&m->sperre);
    if (index >= 0 && index < m->anzahl) kopie = m->eintraege[index];
    pthread_mutex_unlock(&m->sperre);
    return kopie;
}

int model_setze_state(model* m, const char* name, const char* state) {
    int index = model_suche(m, name);
    if (index < 0) return 0;
    pthread_mutex_lock(&m->sperre);
    eintrag* e = &m->eintraege[index];
    int geaendert = strcmp(e->state, state) != 0;
    if (geaendert) kopiere(e->state, sizeof e->state, state);
    pthread_mutex_unlock(&m->sperre);
    // Beobachter außerhalb der Sperre benachrichtigen: Sie dürfen das Model lesen
    if (geaendert)
        for (int i = 0; i < m->beobachter_anzahl; i++)
            m->beobachter[i].fn(m, index, m->beobachter[i].kontext);
    return geaendert;
}

void model_beobachten(model* m, model_beobachter fn, void* kontext) {
    if (m->beobachter_anzahl < MAX_BEOBACHTER)
        m->beobachter[m->beobachter_anzahl++] = (beobachter_t){ fn, kontext };
}

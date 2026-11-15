// Kapitel 18: Ein Schaltausgang
#include "ausgang.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef MIT_GPIOD
#include <gpiod.h>
#endif

struct ausgang {
    char name[64];
    int an;
#ifdef MIT_GPIOD
    struct gpiod_line_request* anfrage; // NULL = Attrappe
    unsigned int leitung;
#endif
};

#ifdef MIT_GPIOD
// libgpiod 2: Leitung als Ausgang anfordern – Einstellungen, Konfiguration, Anfrage
static struct gpiod_line_request* anfordern(const char* chipname, unsigned int leitung) {
    char pfad[64];
    snprintf(pfad, sizeof pfad, "/dev/%s", chipname);
    struct gpiod_chip* chip = gpiod_chip_open(pfad);
    if (!chip) return NULL;
    struct gpiod_line_settings* einst = gpiod_line_settings_new();
    gpiod_line_settings_set_direction(einst, GPIOD_LINE_DIRECTION_OUTPUT);
    gpiod_line_settings_set_output_value(einst, GPIOD_LINE_VALUE_INACTIVE);
    struct gpiod_line_config* konfig = gpiod_line_config_new();
    gpiod_line_config_add_line_settings(konfig, &leitung, 1, einst);
    struct gpiod_request_config* anfrage_konfig = gpiod_request_config_new();
    gpiod_request_config_set_consumer(anfrage_konfig, "ohgeraet");
    struct gpiod_line_request* anfrage =
        gpiod_chip_request_lines(chip, anfrage_konfig, konfig);
    gpiod_request_config_free(anfrage_konfig);
    gpiod_line_config_free(konfig);
    gpiod_line_settings_free(einst);
    gpiod_chip_close(chip); // die Anfrage bleibt gültig
    return anfrage;
}
#endif

ausgang* ausgang_oeffnen(const char* beschreibung) {
    ausgang* a = calloc(1, sizeof *a);
    if (!a) return NULL;
    snprintf(a->name, sizeof a->name, "%s", beschreibung);
    if (!strcmp(beschreibung, "attrappe")) return a;
#ifdef MIT_GPIOD
    char chip[32];
    unsigned int leitung;
    if (sscanf(beschreibung, "%31[^:]:%u", chip, &leitung) == 2) {
        a->leitung = leitung;
        a->anfrage = anfordern(chip, leitung);
        if (a->anfrage) return a;
        perror(beschreibung);
    }
#else
    fprintf(stderr, "%s: ohne MIT_GPIOD übersetzt – nur \"attrappe\" möglich\n",
            beschreibung);
#endif
    free(a);
    return NULL;
}

int ausgang_setzen(ausgang* a, int an) {
#ifdef MIT_GPIOD
    if (a->anfrage && gpiod_line_request_set_value(
                          a->anfrage, a->leitung,
                          an ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE))
        return -1;
#endif
    a->an = an != 0;
    return 0;
}

int ausgang_lesen(const ausgang* a) { return a->an; }

const char* ausgang_name(const ausgang* a) { return a->name; }

void ausgang_schliessen(ausgang* a) {
    if (!a) return;
#ifdef MIT_GPIOD
    if (a->anfrage) {
        gpiod_line_request_set_value(a->anfrage, a->leitung, GPIOD_LINE_VALUE_INACTIVE);
        gpiod_line_request_release(a->anfrage);
    }
#endif
    free(a);
}

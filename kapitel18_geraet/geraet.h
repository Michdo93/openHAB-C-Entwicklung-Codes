// Kapitel 18: Die Logik des Geräts – Philosophie 2: Das Programm ist die Quelle der
// Wahrheit für seine Items. Es MELDET Werte per postUpdate, EMPFÄNGT Commands über den
// Ereignisstrom und BESTÄTIGT sie, wenn die Hardware geschaltet hat.
#ifndef GERAET_H
#define GERAET_H

#include <openhab/openhab.h>

#include "ausgang.h"
#include "messwerte.h"

typedef struct {
    openhab_client_t* client;
    ausgang* luefter;
    int intervall;        // Sekunden zwischen zwei Meldungen
    long long netz_bytes; // letzte Netzmessung
    double netz_zeit;
} geraet;

int geraet_anmelden(geraet* g, const char* rechnername); // Items anlegen (idempotent)
void geraet_melden(geraet* g, const messung* m);         // Werte und Lebenszeichen
// Einen Command ausführen; schreibt die Antwort für Pi_Antwort in antwort. 1 = bekannt
int geraet_command(geraet* g, const char* item, const char* befehl, char* antwort,
                   size_t groesse);
void geraet_abmelden(geraet* g); // Lebenszeichen OFF

#endif

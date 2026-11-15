// Kapitel 9: Speicher – malloc, realloc, free und Besitzregeln
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Eine wachsende Liste von Zeichenketten. Die Liste besitzt ihre Kopien.
typedef struct {
    char** namen;
    size_t anzahl, kapazitaet;
} namensliste;

static int hinzufuegen(namensliste* l, const char* name) {
    if (l->anzahl == l->kapazitaet) {
        size_t neu = l->kapazitaet ? 2 * l->kapazitaet : 2;
        char** p = realloc(l->namen, neu * sizeof *p); // kann den Block verschieben
        if (!p) return -1;                             // alter Block bleibt gültig
        l->namen = p;
        l->kapazitaet = neu;
        printf("  Kapazität jetzt %zu\n", neu);
    }
    char* kopie = malloc(strlen(name) + 1); // +1 für das '\0'
    if (!kopie) return -1;
    strcpy(kopie, name);
    l->namen[l->anzahl++] = kopie;
    return 0;
}

static void freigeben(namensliste* l) { // wer besitzt, gibt frei – genau einmal
    for (size_t i = 0; i < l->anzahl; i++) free(l->namen[i]);
    free(l->namen);
    *l = (namensliste){ 0 };
}

int main(void) {
    namensliste l = { 0 };
    const char* items[] = { "Wohnzimmer_Licht", "Wohnzimmer_Dimmer", "Kueche_Rollladen",
                            "Flur_Haustuer", "Haus_Status" };
    for (size_t i = 0; i < sizeof items / sizeof items[0]; i++)
        if (hinzufuegen(&l, items[i]) != 0) {
            fprintf(stderr, "kein Speicher\n");
            freigeben(&l);
            return 1;
        }
    printf("%zu Namen, der dritte: %s\n", l.anzahl, l.namen[2]);

    // calloc setzt den Speicher auf 0 – praktisch für Zähler
    int* zaehler = calloc(l.anzahl, sizeof *zaehler);
    if (zaehler) {
        zaehler[1] += 3;
        printf("Zähler: %d %d %d\n", zaehler[0], zaehler[1], zaehler[2]);
        free(zaehler);
    }
    freigeben(&l);
    printf("nach freigeben: %zu Namen, Zeiger %s\n", l.anzahl,
           l.namen ? "gesetzt" : "NULL");
    return 0;
}

// Kapitel 8: Benannte Argumente – designierte Initialisierer hinter einem Makro
#include <stdio.h>

// Alle optionalen Parameter in einer Struktur; nicht genannte Felder sind 0 bzw. NULL
typedef struct {
    int _; // Platzhalter, damit die Struktur nie leer initialisiert wird
    const char* tags;
    const char* fields;
    int recursive;
} suche_args;

static void suche_(const char* typ, suche_args a) {
    printf("Typ %-7s tags=%-6s fields=%-11s recursive=%d\n", typ, a.tags ? a.tags : "-",
           a.fields ? a.fields : "-", a.recursive);
}

// Das Makro baut aus den Namen eine zusammengesetzte Struktur (compound literal)
#define suche(typ, ...) suche_((typ), (suche_args){ ._ = 0, __VA_ARGS__ })

int main(void) {
    suche("Switch");                                         // nur der Pflichtparameter
    suche("Dimmer", .tags = "Light");                        // ein Name
    suche("Number", .fields = "name,state", .recursive = 1); // Reihenfolge egal
    suche_("Color", (suche_args){ .tags = "Light" });        // ohne Makro
    return 0;
}

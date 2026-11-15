// Kapitel 8: Strukturen, Aufzählungen und Sortieren
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum { TYP_SWITCH, TYP_DIMMER, TYP_NUMBER } item_typ;

static const char* TYPNAMEN[] = { "Switch", "Dimmer", "Number" };

typedef struct {
    const char* name;
    item_typ typ;
    double wert;
} item_t;

// Vergleichsfunktion für qsort: negativ, 0 oder positiv
static int nach_name(const void* a, const void* b) {
    const item_t* x = a;
    const item_t* y = b;
    return strcmp(x->name, y->name);
}

static void ausgeben(const item_t* it) { // -> greift über einen Zeiger auf Felder zu
    printf("  %-22s %-7s %6.1f\n", it->name, TYPNAMEN[it->typ], it->wert);
}

int main(void) {
    item_t items[] = {
        { "Wohnzimmer_Temperatur", TYP_NUMBER, 21.5 },
        { .name = "Wohnzimmer_Dimmer", .typ = TYP_DIMMER, .wert = 60 }, // mit Feldnamen
        { "Wohnzimmer_Licht", TYP_SWITCH, 1 },
    };
    size_t n = sizeof items / sizeof items[0];

    qsort(items, n, sizeof items[0], nach_name);
    puts("Sortiert:");
    for (size_t i = 0; i < n; i++) ausgeben(&items[i]);

    // Strukturen werden kopiert – ein Zeiger teilt dagegen dasselbe Objekt
    item_t kopie = items[0];
    kopie.wert = 100;
    item_t* zeiger = &items[0];
    zeiger->wert = 75;
    printf("Original %.0f, Kopie %.0f\n", items[0].wert, kopie.wert);
    return 0;
}

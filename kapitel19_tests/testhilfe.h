// Kapitel 19: Eine Testhilfe in wenigen Zeilen – PRUEFE zählt, meldet und macht weiter.
// Ergebnis über den Exit-Code: 0 = bestanden, 1 = Fehler, 77 = übersprungen (CTest)
#ifndef TESTHILFE_H
#define TESTHILFE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int pruefungen = 0, fehlschlaege = 0;

#define PRUEFE(bedingung)                                                           \
    do {                                                                            \
        pruefungen++;                                                               \
        if (!(bedingung)) {                                                         \
            fehlschlaege++;                                                         \
            fprintf(stderr, "%s:%d: FEHLER: %s\n", __FILE__, __LINE__, #bedingung); \
        }                                                                           \
    } while (0)

#define PRUEFE_TEXT(ist, soll)                                                           \
    do {                                                                                 \
        const char* ist_ = (ist);                                                        \
        pruefungen++;                                                                    \
        if (!ist_ || strcmp(ist_, (soll))) {                                             \
            fehlschlaege++;                                                              \
            fprintf(stderr, "%s:%d: FEHLER: %s ist \"%s\", erwartet \"%s\"\n", __FILE__, \
                    __LINE__, #ist, ist_ ? ist_ : "(NULL)", (soll));                     \
        }                                                                                \
    } while (0)

static inline int test_ende(const char* name) {
    printf("%s: %d Prüfungen, %d fehlgeschlagen\n", name, pruefungen, fehlschlaege);
    return fehlschlaege ? 1 : 0;
}

// Integrationstests ohne openHAB überspringen statt scheitern
static inline void braucht_openhab(void) {
    const char* t = getenv("OPENHAB_TOKEN");
    if (!t || !*t) {
        printf("OPENHAB_TOKEN fehlt – übersprungen\n");
        exit(77);
    }
}

#endif

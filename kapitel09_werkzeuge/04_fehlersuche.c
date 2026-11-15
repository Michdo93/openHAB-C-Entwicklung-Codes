// Kapitel 9: Fehlersuche – drei typische Speicherfehler für AddressSanitizer und valgrind
//   cmake -S . -B build-asan -DMIT_ASAN=ON && cmake --build build-asan
//   build-asan/kapitel09_werkzeuge/04_fehlersuche ueberlauf
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void ueberlauf(void) {
    char* name = malloc(16);
    strcpy(name, "Wohnzimmer_Licht"); // 16 Zeichen + '\0' = 17 Byte
    printf("%s\n", name);
    free(name);
}

static void nach_free(void) {
    char* state = malloc(8);
    strcpy(state, "ON");
    free(state);
    printf("%s\n", state); // Zugriff auf freigegebenen Speicher
}

static void leck(void) {
    char* kopie = malloc(64); // wird nie freigegeben
    snprintf(kopie, 64, "Kopie von %s", "Haus_Status");
    printf("%s\n", kopie);
}

int main(int argc, char** argv) {
    const char* fall = argc > 1 ? argv[1] : "";
    if (!strcmp(fall, "ueberlauf"))
        ueberlauf();
    else if (!strcmp(fall, "nach_free"))
        nach_free();
    else if (!strcmp(fall, "leck"))
        leck();
    else {
        fprintf(stderr, "Aufruf: %s ueberlauf | nach_free | leck\n", argv[0]);
        return 2;
    }
    return 0;
}

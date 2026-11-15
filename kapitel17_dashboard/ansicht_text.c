// Kapitel 17: Die Text-View – jede Änderung eine Zeile, Befehle zeilenweise über stdin
//   t <Nr>   umschalten      + <Nr>  heller     - <Nr>  dunkler      q  beenden
#define _POSIX_C_SOURCE 200809L
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "ansicht.h"

static void geaendert(model* m, int index, void* kontext) { // Beobachter des Models
    (void)kontext;
    eintrag e = model_eintrag(m, index);
    printf("  [%d] %-22s -> %s\n", index + 1, e.name, e.state);
    fflush(stdout);
}

static void tabelle(model* m) {
    for (int i = 0; i < model_anzahl(m); i++) {
        eintrag e = model_eintrag(m, i);
        printf("  [%d] %-22s %-19s %s\n", i + 1, e.name, e.typ, e.state);
    }
}

int ansicht_text(model* m, controller* c, int dauer) {
    model_beobachten(m, geaendert, NULL);
    tabelle(m);
    time_t ende = time(NULL) + dauer;
    char zeile[64];
    int offen = 1; // stdin noch nicht zu Ende
    while (time(NULL) < ende) {
        struct pollfd p = { .fd = STDIN_FILENO, .events = POLLIN };
        if (!offen || poll(&p, 1, 200) <= 0) { // 200 ms auf Eingaben warten
            if (!offen) {
                struct timespec t = { 0, 200000000 };
                nanosleep(&t, NULL);
            }
            continue;
        }
        if (!fgets(zeile, sizeof zeile, stdin)) {
            offen = 0;
            continue;
        }
        char befehl;
        int nr = 0;
        if (sscanf(zeile, " %c %d", &befehl, &nr) < 1) continue;
        if (befehl == 'q') break;
        printf("> %s", zeile);
        if (befehl == 't')
            controller_umschalten(c, nr - 1);
        else if (befehl == '+')
            controller_aendern(c, nr - 1, 10);
        else if (befehl == '-')
            controller_aendern(c, nr - 1, -10);
        if (controller_fehler(c)) printf("  Fehler: %s\n", controller_fehler(c));
        struct timespec t = { 0, 300000000 };
        nanosleep(&t, NULL); // Ereignisse eintreffen lassen
    }
    printf("Endstand:\n");
    tabelle(m);
    return 0;
}

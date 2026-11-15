// Kapitel 17: ohdash – ein Terminal-Dashboard für openHAB (Model-View-Controller)
//   ohdash [--gruppe=gWohnzimmer] [--text [--dauer=Sekunden]]
#include <openhab/openhab.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ansicht.h"

int main(int argc, char** argv) {
    const char* gruppe = "gWohnzimmer";
    int text = 0, dauer = 30;
    for (int i = 1; i < argc; i++) {
        if (!strncmp(argv[i], "--gruppe=", 9))
            gruppe = argv[i] + 9;
        else if (!strcmp(argv[i], "--text"))
            text = 1;
        else if (!strncmp(argv[i], "--dauer=", 8))
            dauer = atoi(argv[i] + 8);
    }
    const char* url =
        getenv("OPENHAB_URL") ? getenv("OPENHAB_URL") : "http://127.0.0.1:8080";
    openhab_client_t* client = openhab_client_new(url, .token = getenv("OPENHAB_TOKEN"));
    if (!openhab_client_is_logged_in(client)) {
        fprintf(stderr, "keine Verbindung: %s\n", openhab_client_last_error(client));
        return 1;
    }

    model* m = model_neu();                    // M
    controller* c = controller_neu(client, m); // C
    if (controller_laden(c, gruppe) < 0 || controller_starten(c) < 0) {
        fprintf(stderr, "%s: %s\n", gruppe, controller_fehler(c));
        return 1;
    }
    printf("%d Items aus %s geladen\n", model_anzahl(m), gruppe);

    int rc; // V – die gewählte Ansicht
#ifdef MIT_NCURSES
    rc = text ? ansicht_text(m, c, dauer) : ansicht_ncurses(m, c);
#else
    (void)text;
    rc = ansicht_text(m, c, dauer);
#endif
    controller_stoppen(c);
    model_frei(m);
    openhab_client_destroy(client);
    return rc;
}

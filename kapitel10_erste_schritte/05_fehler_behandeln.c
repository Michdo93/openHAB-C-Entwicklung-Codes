// Kapitel 10: Fehler behandeln
#include <openhab/openhab.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    const char* url =
        getenv("OPENHAB_URL") ? getenv("OPENHAB_URL") : "http://127.0.0.1:8080";
    openhab_client_t* c = openhab_client_new(url, .token = getenv("OPENHAB_TOKEN"));

    // Alle Funktionen melden Ablehnungen als Ergebnis {"error": …} – nie als Absturz
    const char* versuche[][2] = {
        { "Wohnzimer_Licht", "ON" },    // Tippfehler im Namen
        { "Wohnzimmer_Licht", "HALB" }, // ungültiger Command
        { "Flur_Haustuer", "OPEN" },    // Contact: kein Command
        { "Wohnzimmer_Licht", "OFF" },
    };
    for (int i = 0; i < 4; i++) {
        cJSON* r = openhab_items_sendCommand(c, versuche[i][0], versuche[i][1]);
        if (openhab_is_error(r))
            printf("FEHLER  %s <- %s: %s\n", versuche[i][0], versuche[i][1],
                   openhab_error(r));
        else
            printf("OK      %s <- %s\n", versuche[i][0], versuche[i][1]);
        cJSON_Delete(r);
    }

    // Ein Item aus einer .items-Datei lässt sich steuern, aber nicht löschen
    cJSON* r = openhab_items_deleteItem(c, "Wohnzimmer_Licht");
    printf("\nlöschen: %s\n", openhab_error(r));
    cJSON_Delete(r);

    // Falsches Token: Der Client merkt es schon beim Verbinden
    openhab_client_t* fremd = openhab_client_new(url, .token = "oh.falsch.123");
    printf("falsches Token: angemeldet=%d (%s)\n", openhab_client_is_logged_in(fremd),
           openhab_client_last_error(fremd));
    r = openhab_items_getItemState(fremd, "Wohnzimmer_Licht");
    printf("  Antwort: %s\n", openhab_error(r));
    cJSON_Delete(r);
    openhab_client_destroy(fremd);

    // Netzwerkfehler: ein Server, den es nicht gibt, zwei Sekunden Wartezeit
    openhab_client_t* weg = openhab_client_new("http://127.0.0.1:9", .timeout = 2);
    r = openhab_items_getItemState(weg, "Wohnzimmer_Licht");
    printf("anderer Server: %s\n", openhab_error(r));
    cJSON_Delete(r);
    openhab_client_destroy(weg);

    // NULL ist kein Fehlerobjekt, aber openhab_is_error() erkennt es trotzdem
    printf("NULL ist Fehler: %d\n", openhab_is_error(NULL));
    openhab_client_destroy(c);
    return 0;
}

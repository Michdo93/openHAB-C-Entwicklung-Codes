// Kapitel 10: Commands senden
#include <openhab/openhab.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {
    const char* url =
        getenv("OPENHAB_URL") ? getenv("OPENHAB_URL") : "http://127.0.0.1:8080";
    openhab_client_t* c = openhab_client_new(url, .token = getenv("OPENHAB_TOKEN"));
    const char* befehle[][2] = {
        { "Wohnzimmer_Licht", "ON" },             // Switch
        { "Wohnzimmer_Dimmer", "60" },            // Dimmer: Prozent
        { "Wohnzimmer_Farblicht", "240,100,80" }, // Color: Farbton,Sättigung,Helligkeit
        { "Kueche_Rollladen", "DOWN" },           // Rollershutter
        { "Haus_Status", "Alle zu Hause" },       // String
    };
    int n = sizeof befehle / sizeof befehle[0];
    for (int i = 0; i < n; i++) {
        cJSON* antwort = openhab_items_sendCommand(c, befehle[i][0], befehle[i][1],
                                                   .source = "buch:kapitel10");
        char* text = openhab_json_print(antwort);
        printf("%-22s <- %-14s %s\n", befehle[i][0], befehle[i][1], text);
        openhab_free(text);
        cJSON_Delete(antwort);
    }
    usleep(500 * 1000); // openHAB verarbeitet Commands asynchron
    for (int i = 0; i < n; i++) {
        cJSON* state = openhab_items_getItemState(c, befehle[i][0]);
        printf("%-22s = %s\n", befehle[i][0], state->valuestring);
        cJSON_Delete(state);
    }
    openhab_client_destroy(c);
    return 0;
}

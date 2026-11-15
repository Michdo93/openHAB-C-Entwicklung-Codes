// Kapitel 10: States melden
#include <openhab/openhab.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Ergebnis ausgeben und freigeben – eine kleine Hilfe für die Beispiele
static void zeige(cJSON* ergebnis) {
    char* text = openhab_json_print(ergebnis);
    puts(text);
    openhab_free(text);
    cJSON_Delete(ergebnis);
}

int main(void) {
    const char* url =
        getenv("OPENHAB_URL") ? getenv("OPENHAB_URL") : "http://127.0.0.1:8080";
    openhab_client_t* c = openhab_client_new(url, .token = getenv("OPENHAB_TOKEN"));

    // Ein Messwert mit Einheit, eine Feuchte und ein Zeitstempel (ISO 8601, UTC)
    zeige(openhab_items_postUpdate(c, "Werkstatt_Temperatur", "18.4 °C"));
    zeige(openhab_items_postUpdate(c, "Werkstatt_Feuchte", "56 %"));
    char jetzt[32];
    time_t t = time(NULL);
    strftime(jetzt, sizeof jetzt, "%Y-%m-%dT%H:%M:%SZ", gmtime(&t));
    zeige(openhab_items_postUpdate(c, "Werkstatt_LetzteMeldung", jetzt));
    // Ein Contact kennt keine Commands – nur States
    zeige(openhab_items_postUpdate(c, "Flur_Haustuer", "OPEN"));
    // Einheiten werden umgerechnet: Fahrenheit rein, Celsius raus
    zeige(openhab_items_postUpdate(c, "Wohnzimmer_Temperatur", "71.6 °F"));

    const char* namen[] = { "Werkstatt_Temperatur", "Werkstatt_Feuchte", "Flur_Haustuer",
                            "Wohnzimmer_Temperatur" };
    for (int i = 0; i < 4; i++) {
        cJSON* s = openhab_items_getItemState(c, namen[i]);
        printf("%-22s = %s\n", namen[i], s->valuestring);
        cJSON_Delete(s);
    }
    // Für die weiteren Beispiele wieder 21.5 °C setzen
    cJSON_Delete(openhab_items_postUpdate(c, "Wohnzimmer_Temperatur", "21.5 °C"));
    openhab_client_destroy(c);
    return 0;
}

// Kapitel 10: Optionale Parameter – benannte Argumente und JSON-Pfade
#include <openhab/openhab.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    const char* url =
        getenv("OPENHAB_URL") ? getenv("OPENHAB_URL") : "http://127.0.0.1:8080";
    openhab_client_t* c = openhab_client_new(url, .token = getenv("OPENHAB_TOKEN"));

    // 1. nur Pflichtparameter: Alle optionalen haben die Vorgabe der Python-Bibliothek
    cJSON* alle = openhab_items_getItems(c);
    printf("alle Items:    %d\n", cJSON_GetArraySize(alle));
    cJSON_Delete(alle);

    // 2. benannte Argumente in beliebiger Reihenfolge
    cJSON* r = openhab_items_getItems(c, .fields = "name", .tags = "Light");
    char* text = openhab_json_print(r);
    printf("Licht, Felder: %s\n", text);
    openhab_free(text);
    cJSON_Delete(r);

    // 3. Wahrheitswerte als openhab_flag: OPENHAB_UNSET (Vorgabe), _TRUE, _FALSE
    cJSON* mit = openhab_items_getItem(c, "gLichter"); // Vorgabe: True
    cJSON* ohne = openhab_items_getItem(c, "gLichter", .recursive = OPENHAB_FALSE);
    printf("gLichter:      %d Mitglieder (recursive), %d ohne, Funktion %s\n",
           cJSON_GetArraySize(openhab_json_get(mit, "members")),
           cJSON_GetArraySize(openhab_json_get(ohne, "members")),
           openhab_json_str(mit, "function.name"));
    cJSON_Delete(mit);
    cJSON_Delete(ohne);

    // 4. ohne Makro: die Funktion mit Unterstrich und der Argument-Struktur
    r = openhab_items_getItems_(
        c, (openhab_items_getItems_args){ .type = "Switch", .fields = "name" });
    printf("Switch-Items:  %d\n", cJSON_GetArraySize(r));
    cJSON_Delete(r);

    // JSON-Pfade: Punkte trennen Felder, Zahlen sind Array-Positionen
    cJSON* item = openhab_items_getItem(c, "Wohnzimmer_Temperatur");
    printf("Muster:        %s\n", openhab_json_str(item, "stateDescription.pattern"));
    printf("erstes Tag:    %s\n", openhab_json_str(item, "tags.0"));
    printf("fehlt:         %s\n",
           openhab_json_str(item, "metadata.unit") ? "da" : "(NULL)");
    printf("Zahl/Vorgabe:  %.1f\n", openhab_json_num(item, "stateDescription.step", -1));
    cJSON_Delete(item);
    openhab_client_destroy(c);
    return 0;
}

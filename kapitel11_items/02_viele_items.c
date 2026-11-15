// Kapitel 11: Einen ganzen Raum mit einem einzigen Aufruf anlegen
#include <openhab/openhab.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const char* GARTENHAUS =
    "[{\"type\":\"Group\",\"name\":\"gGartenhaus\",\"label\":\"Gartenhaus\","
    "  \"groupNames\":[\"gHaus\"],\"tags\":[\"Garden\"]},"
    " {\"type\":\"Switch\",\"name\":\"Gartenhaus_Licht\",\"label\":\"Licht\","
    "  \"groupNames\":[\"gGartenhaus\",\"gGartenhaus_Lichter\"],"
    "  \"tags\":[\"Switch\",\"Light\"],\"category\":\"light\"},"
    " {\"type\":\"Number:Temperature\",\"name\":\"Gartenhaus_Temperatur\","
    "  \"label\":\"Temperatur\",\"groupNames\":[\"gGartenhaus\"],"
    "  \"tags\":[\"Measurement\",\"Temperature\"],\"category\":\"temperature\"},"
    " {\"type\":\"Contact\",\"name\":\"Gartenhaus_Tuer\",\"label\":\"Tür\","
    "  \"groupNames\":[\"gGartenhaus\"],\"tags\":[\"OpenState\",\"Door\"]},"
    " {\"type\":\"Group\",\"name\":\"gGartenhaus_Lichter\",\"label\":\"Lichter im "
    "Gartenhaus\","
    "  \"groupNames\":[\"gGartenhaus\"],\"groupType\":\"Switch\","
    "  \"function\":{\"name\":\"OR\",\"params\":[\"ON\",\"OFF\"]}}]";

static int nach_name(const void* a, const void* b) {
    return strcmp(openhab_json_str(*(cJSON* const*)a, "name"),
                  openhab_json_str(*(cJSON* const*)b, "name"));
}

int main(void) {
    const char* url =
        getenv("OPENHAB_URL") ? getenv("OPENHAB_URL") : "http://127.0.0.1:8080";
    openhab_client_t* c = openhab_client_new(url, .token = getenv("OPENHAB_TOKEN"));

    cJSON* liste = cJSON_Parse(GARTENHAUS);
    cJSON* antwort = openhab_items_addOrUpdateItems(c, liste); // ein PUT für alle
    cJSON* r;
    cJSON_ArrayForEach(r, antwort) printf(
        "anlegen: %-24s %s\n", openhab_json_str(r, "name"), openhab_json_str(r, "status"));
    cJSON_Delete(antwort);

    cJSON_Delete(openhab_items_postUpdate(c, "Gartenhaus_Temperatur", "12.5 °C"));
    cJSON_Delete(openhab_items_postUpdate(c, "Gartenhaus_Tuer", "CLOSED"));
    cJSON_Delete(openhab_items_sendCommand(c, "Gartenhaus_Licht", "ON"));

    usleep(300 * 1000);
    cJSON* raum = openhab_items_getItem(c, "gGartenhaus", .recursive = OPENHAB_TRUE);
    cJSON* mitglieder = openhab_json_get(raum, "members");
    int n = cJSON_GetArraySize(mitglieder);
    cJSON* sortiert[16];
    for (int i = 0; i < n && i < 16; i++) sortiert[i] = cJSON_GetArrayItem(mitglieder, i);
    qsort(sortiert, n, sizeof sortiert[0], nach_name);
    for (int i = 0; i < n; i++)
        printf("  %-24s %-20s %s\n", openhab_json_str(sortiert[i], "name"),
               openhab_json_str(sortiert[i], "type"),
               openhab_json_str(sortiert[i], "state"));
    cJSON_Delete(raum);

    // Die Gruppenfunktion OR(ON,OFF) fasst die Mitglieder zusammen
    cJSON* s = openhab_items_getItemState(c, "gGartenhaus_Lichter");
    printf("Lichter im Gartenhaus: %s\n", s->valuestring);
    cJSON_Delete(s);

    // Aufräumen: von hinten nach vorn – erst die Mitglieder, dann die Gruppen
    for (int i = cJSON_GetArraySize(liste) - 1; i >= 0; i--)
        cJSON_Delete(openhab_items_deleteItem(
            c, openhab_json_str(cJSON_GetArrayItem(liste, i), "name")));
    cJSON_Delete(liste);
    r = openhab_items_getItem(c, "gGartenhaus");
    printf("aufgeräumt: %s\n", openhab_is_error(r) ? "ja" : "nein");
    cJSON_Delete(r);
    openhab_client_destroy(c);
    return 0;
}

// Kapitel 11: Gruppen zur Laufzeit ändern und den Zustandsverlauf lesen
#include <openhab/openhab.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// Alle Ketten von Gruppen über einem Item ausgeben, z. B. gLichter -> gHaus (rekursiv)
static void ketten(const cJSON* item, const char* bisher, int* anzahl) {
    cJSON* eltern = openhab_json_get(item, "parents");
    if (cJSON_GetArraySize(eltern) == 0) {
        if (*bisher) printf("%s%s", (*anzahl)++ ? " | " : " ", bisher);
        return;
    }
    cJSON* p;
    cJSON_ArrayForEach(p, eltern) {
        char kette[256];
        snprintf(kette, sizeof kette, "%s%s%s", bisher, *bisher ? " -> " : "",
                 openhab_json_str(p, "name"));
        ketten(p, kette, anzahl);
    }
}

// Millisekunden seit 1970 als Uhrzeit
static const char* uhrzeit(double millis, char* puffer) {
    time_t t = (time_t)(millis / 1000);
    strftime(puffer, 16, "%H:%M:%S", localtime(&t));
    return puffer;
}

int main(void) {
    const char* url =
        getenv("OPENHAB_URL") ? getenv("OPENHAB_URL") : "http://127.0.0.1:8080";
    openhab_client_t* c = openhab_client_new(url, .token = getenv("OPENHAB_TOKEN"));

    // Ein per REST angelegtes Licht in die Gruppe aller Lichter aufnehmen
    cJSON* d = cJSON_Parse(
        "{\"type\":\"Switch\",\"name\":\"Flur_Licht\","
        "\"label\":\"Flurlicht\",\"tags\":[\"Switch\",\"Light\"]}");
    cJSON_Delete(openhab_items_addOrUpdateItem(c, "Flur_Licht", d));
    cJSON_Delete(d);
    cJSON* r = openhab_items_addGroupMember(c, "gLichter", "Flur_Licht");
    printf("aufnehmen: %s\n", openhab_json_str(r, "message"));
    cJSON_Delete(r);
    cJSON* g = openhab_items_getItem(c, "gLichter");
    printf("Mitglieder:");
    cJSON* m;
    cJSON_ArrayForEach(m, openhab_json_get(g, "members"))
        printf(" %s", openhab_json_str(m, "name"));
    printf("\n");
    cJSON_Delete(g);

    // Ein Command an die Gruppe erreicht alle Mitglieder
    cJSON_Delete(openhab_items_sendCommand(c, "gLichter", "OFF"));
    cJSON_Delete(openhab_items_sendCommand(c, "Flur_Licht", "ON"));
    usleep(300 * 1000);
    cJSON* s = openhab_items_getItemState(c, "gLichter");
    printf("gLichter: %s (mindestens eines ist an)\n", s->valuestring);
    cJSON_Delete(s);

    // openHAB 5 merkt sich den vorherigen Zustand und die Zeitpunkte
    cJSON* licht = openhab_items_getItem(c, "Flur_Licht");
    char u1[16], u2[16];
    printf("Flur_Licht: %s (vorher %s), aktualisiert %s, geändert %s\n",
           openhab_json_str(licht, "state"), openhab_json_str(licht, "lastState"),
           uhrzeit(openhab_json_num(licht, "lastStateUpdate", 0), u1),
           uhrzeit(openhab_json_num(licht, "lastStateChange", 0), u2));
    cJSON_Delete(licht);

    // In welchen Gruppen steckt das Item – bis ganz nach oben?
    cJSON* mit_eltern = openhab_items_getItem(c, "Flur_Licht", .parents = OPENHAB_TRUE);
    int anzahl = 0;
    printf("Eltern:");
    ketten(mit_eltern, "", &anzahl);
    printf("\n");
    cJSON_Delete(mit_eltern);

    r = openhab_items_removeGroupMember(c, "gLichter", "Flur_Licht");
    printf("entfernen: %s\n", openhab_json_str(r, "message"));
    cJSON_Delete(r);
    cJSON_Delete(openhab_items_deleteItem(c, "Flur_Licht"));
    openhab_client_destroy(c);
    return 0;
}

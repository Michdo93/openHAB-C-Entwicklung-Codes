// Kapitel 13: Regeln lesen – Übersicht, Module und Skript
#include <openhab/openhab.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    const char* url =
        getenv("OPENHAB_URL") ? getenv("OPENHAB_URL") : "http://127.0.0.1:8080";
    openhab_client_t* c = openhab_client_new(url, .token = getenv("OPENHAB_TOKEN"));

    cJSON* regeln = openhab_rules_getRules(c, .summary = OPENHAB_TRUE);
    cJSON* r;
    cJSON_ArrayForEach(r, regeln) {
        char* tags = openhab_json_print(openhab_json_get(r, "tags"));
        printf("%-12s %-14s %-6s Tags %s\n", openhab_json_str(r, "uid"),
               openhab_json_str(r, "name"), openhab_json_str(r, "status.status"), tags);
        openhab_free(tags);
    }
    cJSON_Delete(regeln);

    // Der Aufbau einer Regel: Trigger, Bedingungen, Aktionen – jeweils Module
    const char* uid = "restbuch-1";
    printf("\nRegel %s:\n", uid);
    const char* arten[] = { "Trigger", "Bedingung", "Aktion" };
    cJSON* teile[] = { openhab_rules_getTriggers(c, uid),
                       openhab_rules_getConditions(c, uid),
                       openhab_rules_getActions(c, uid) };
    for (int i = 0; i < 3; i++) {
        cJSON* m;
        cJSON_ArrayForEach(m, teile[i]) {
            char* cfg = openhab_json_print(openhab_json_get(m, "configuration"));
            printf("  %-10s %-7s %-26s %.36s…\n", arten[i], openhab_json_str(m, "id"),
                   openhab_json_str(m, "type"), cfg);
            openhab_free(cfg);
        }
        cJSON_Delete(teile[i]);
    }

    // Ein einzelner Parameter eines Moduls: das Skript der Aktion
    cJSON* skript =
        openhab_rules_getModuleConfigParam(c, uid, "actions", "script", "script");
    char kurz[256];
    snprintf(kurz, sizeof kurz, "%s", cJSON_IsString(skript) ? skript->valuestring : "");
    for (char* p = kurz; *p; p++)
        if (*p == '\n') *p = '|'; // Zeilen zusammenfassen
    printf("\nSkript der Aktion: %.66s\n", kurz + strspn(kurz, "| "));
    cJSON_Delete(skript);
    openhab_client_destroy(c);
    return 0;
}

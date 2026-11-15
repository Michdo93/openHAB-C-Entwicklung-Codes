// Kapitel 13: Welche Bausteine gibt es für Regeln?
#include <openhab/openhab.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int nach_uid(const void* a, const void* b) {
    return strcmp(openhab_json_str(*(cJSON* const*)a, "uid"),
                  openhab_json_str(*(cJSON* const*)b, "uid"));
}

int main(void) {
    const char* url =
        getenv("OPENHAB_URL") ? getenv("OPENHAB_URL") : "http://127.0.0.1:8080";
    openhab_client_t* c = openhab_client_new(url, .token = getenv("OPENHAB_TOKEN"));

    cJSON* alle = openhab_module_types_getModuleTypes(c, .language = "de");
    const char* arten[] = { "Trigger", "Bedingung", "Aktion" };
    for (int art = 0; art < 3; art++) {
        cJSON* liste[256];
        int n = 0;
        cJSON* m;
        cJSON_ArrayForEach(m, alle) {
            const char* uid = openhab_json_str(m, "uid");
            if (strcmp(openhab_json_str(m, "visibility"), "VISIBLE") || strchr(uid, '#'))
                continue; // ohne Thing-Actions
            int a = strstr(uid, "Trigger") ? 0 : strstr(uid, "Condition") ? 1 : 2;
            if (a == art && n < 256) liste[n++] = m;
        }
        qsort(liste, n, sizeof liste[0], nach_uid);
        printf("%s (%d):\n", arten[art], n);
        for (int i = 0; i < n && i < 7; i++)
            printf("  %-30s %s\n", openhab_json_str(liste[i], "uid"),
                   openhab_json_str(liste[i], "label"));
    }
    cJSON_Delete(alle);

    cJSON* typ =
        openhab_module_types_getModuleType(c, "core.ItemCommandAction", .language = "de");
    printf("\n%s – %s\n", openhab_json_str(typ, "uid"),
           openhab_json_str(typ, "description"));
    cJSON* p;
    cJSON_ArrayForEach(p, openhab_json_get(typ, "configDescriptions")) printf(
        "  %-10s %-6s %-8s %s\n", openhab_json_str(p, "name"), openhab_json_str(p, "type"),
        cJSON_IsTrue(openhab_json_get(p, "required")) ? "Pflicht" : "optional",
        openhab_json_str(p, "label"));
    cJSON_Delete(typ);

    cJSON* vorlagen = openhab_templates_getTemplates(c, .language = "de");
    printf("\nRegelvorlagen installiert: %d\n", cJSON_GetArraySize(vorlagen));
    cJSON_Delete(vorlagen);
    openhab_client_destroy(c);
    return 0;
}

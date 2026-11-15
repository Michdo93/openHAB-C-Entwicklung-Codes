// Kapitel 15: Systemdienste, Protokollierung, Zugänge und Symbole
#include <openhab/openhab.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LOGGER "org.openhab.binding.astro"

static void level(openhab_client_t* c, const char* was) {
    cJSON* l = openhab_logging_getLogger(c, LOGGER);
    printf("%s%s\n", was, openhab_json_str(l, "loggers.0.level"));
    cJSON_Delete(l);
}

int main(void) {
    const char* url =
        getenv("OPENHAB_URL") ? getenv("OPENHAB_URL") : "http://127.0.0.1:8080";
    openhab_client_t* c = openhab_client_new(url, .token = getenv("OPENHAB_TOKEN"));
    cJSON* s;

    // 1. Systemdienste und ihre Einstellungen (dieselben wie in der Main UI)
    cJSON* dienste = openhab_services_getServices(c, .language = "de");
    cJSON_ArrayForEach(s, dienste) if (!strcmp(openhab_json_str(s, "category"), "system"))
        printf("  %-29s %s\n", openhab_json_str(s, "id"), openhab_json_str(s, "label"));
    cJSON_Delete(dienste);
    cJSON* i18n = openhab_services_getServiceConfig(c, "org.openhab.i18n");
    char* text = openhab_json_print(i18n);
    printf("Regionale Einstellungen: %s\n", text);
    openhab_free(text);
    cJSON_Delete(i18n);

    // 2. Log-Level eines Bindings zur Fehlersuche anheben – und zurücksetzen
    level(c, "\nvorher: ");
    cJSON_Delete(openhab_logging_modifyOrAddLogger(c, LOGGER, "DEBUG"));
    level(c, "jetzt:  ");
    cJSON_Delete(openhab_logging_removeLogger(c, LOGGER)); // zurück auf den Standard
    level(c, "danach: ");

    // 3. Wer hat Zugang? API-Tokens und Sitzungen des angemeldeten Benutzers
    cJSON* tokens = openhab_auth_getAPITokens(c);
    cJSON_ArrayForEach(s, tokens)
        printf("\nToken \"%s\" (Bereich %s) seit %.10s\n", openhab_json_str(s, "name"),
               openhab_json_str(s, "scope"), openhab_json_str(s, "createdTime"));
    cJSON_Delete(tokens);
    cJSON* sitzungen = openhab_auth_getSessions(c);
    printf("offene Sitzungen: %d\n", cJSON_GetArraySize(sitzungen));
    cJSON_Delete(sitzungen);

    // 4. Symbolsätze und Maßeinheiten
    cJSON* sets = openhab_iconsets_getIconsets(c, .language = "de");
    printf("Symbolsätze:");
    cJSON_ArrayForEach(s, sets) printf(" %s", openhab_json_str(s, "label"));
    printf("\n");
    cJSON_Delete(sets);
    cJSON* uom = openhab_systeminfo_getUoMInfo(c);
    cJSON* groessen = openhab_json_get(uom, "uomInfo.dimensions");
    printf("%d physikalische Größen, z. B.", cJSON_GetArraySize(groessen));
    for (int k = 0; k < 3; k++) {
        cJSON* g = cJSON_GetArrayItem(groessen, k);
        printf("%s %s [%s]", k ? "," : "", openhab_json_str(g, "dimension"),
               openhab_json_str(g, "systemUnit"));
    }
    printf("\n");
    cJSON_Delete(uom);
    openhab_client_destroy(c);
    return 0;
}

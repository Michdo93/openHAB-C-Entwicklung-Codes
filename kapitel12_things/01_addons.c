// Kapitel 12: Welche Add-ons gibt es, welche sind installiert?
#include <openhab/openhab.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    const char* url =
        getenv("OPENHAB_URL") ? getenv("OPENHAB_URL") : "http://127.0.0.1:8080";
    openhab_client_t* c = openhab_client_new(url, .token = getenv("OPENHAB_TOKEN"));

    cJSON* alle = openhab_addons_getAddons(c, .language = "de");
    int installiert = 0;
    cJSON* a;
    cJSON_ArrayForEach(a, alle) installiert +=
        cJSON_IsTrue(openhab_json_get(a, "installed"));
    printf("%d Add-ons verfügbar, %d installiert:\n", cJSON_GetArraySize(alle),
           installiert);
    cJSON_ArrayForEach(a, alle) if (cJSON_IsTrue(openhab_json_get(a, "installed")))
        printf("  %-24s %-8s %s\n", openhab_json_str(a, "uid"),
               openhab_json_str(a, "version"), openhab_json_str(a, "label"));

    // Wie viele gibt es von jeder Art? Ein kleines Zähl-Array genügt.
    const char* arten[16];
    int anzahl[16], n = 0;
    cJSON_ArrayForEach(a, alle) {
        const char* typ = openhab_json_str(a, "type");
        int i = 0;
        while (i < n && strcmp(arten[i], typ)) i++;
        if (i == n && n < 16) {
            arten[n] = typ;
            anzahl[n++] = 0;
        }
        if (i < 16) anzahl[i]++;
    }
    printf("\nVerfügbar nach Art:");
    for (int i = 0; i < n; i++) printf(" %s=%d", arten[i], anzahl[i]);
    printf("\n");
    cJSON_Delete(alle); // arten[] zeigt in alle – erst jetzt freigeben

    cJSON* astro = openhab_addons_getAddon(c, "binding-astro", .language = "de");
    printf("\n%s: %.70s…\n", openhab_json_str(astro, "label"),
           openhab_json_str(astro, "description")); // %.70s: höchstens 70 Byte
    char* pakete = openhab_json_print(openhab_json_get(astro, "loggerPackages"));
    printf("Logger-Pakete: %s\n", pakete);
    openhab_free(pakete);
    cJSON_Delete(astro);

    cJSON* quellen = openhab_addons_getAddonServices(c);
    printf("Installationsquellen:");
    cJSON_ArrayForEach(a, quellen) printf(" %s", openhab_json_str(a, "id"));
    printf("\n");
    cJSON_Delete(quellen);
    openhab_client_destroy(c);
    return 0;
}

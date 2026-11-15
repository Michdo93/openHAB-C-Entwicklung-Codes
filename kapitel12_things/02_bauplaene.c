// Kapitel 12: Baupläne – Thing-Typen, Channel-Gruppen, Konfiguration, Channel-Typen
#include <openhab/openhab.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    const char* url =
        getenv("OPENHAB_URL") ? getenv("OPENHAB_URL") : "http://127.0.0.1:8080";
    openhab_client_t* c = openhab_client_new(url, .token = getenv("OPENHAB_TOKEN"));
    cJSON *t, *k;

    cJSON* typen =
        openhab_thing_types_getThingTypes(c, .bindingID = "astro", .language = "de");
    cJSON_ArrayForEach(t, typen)
        printf("%-12s %s\n", openhab_json_str(t, "UID"), openhab_json_str(t, "label"));
    cJSON_Delete(typen);

    cJSON* mond = openhab_thing_types_getThingType(c, "astro:moon", .language = "de");
    printf("\n%s – Channel-Gruppen:\n", openhab_json_str(mond, "label"));
    cJSON_ArrayForEach(t, openhab_json_get(mond, "channelGroups")) {
        printf("  %-10s", openhab_json_str(t, "id"));
        int nr = 0, alle = cJSON_GetArraySize(openhab_json_get(t, "channels"));
        cJSON_ArrayForEach(k, openhab_json_get(t, "channels")) if (nr++ < 5)
            printf("%s%s", nr > 1 ? ", " : " ", openhab_json_str(k, "id"));
        if (alle > 5) printf(" … (%d insgesamt)", alle);
        printf("\n");
    }
    cJSON_Delete(mond);

    // Welche Konfiguration braucht ein Mond-Thing?
    cJSON* beschreibung = openhab_config_descriptions_getConfigDescription(
        c, "thing-type:astro:moonconfig", .language = "de");
    printf("\nKonfiguration:\n");
    cJSON_ArrayForEach(t, openhab_json_get(beschreibung, "parameters")) {
        char pflicht[32];
        const char* standard = openhab_json_str(t, "default");
        if (cJSON_IsTrue(openhab_json_get(t, "required")))
            snprintf(pflicht, 32, "Pflicht");
        else
            snprintf(pflicht, 32, "Standard %s", standard ? standard : "–");
        printf("  %-12s %-8s %-17s %s\n", openhab_json_str(t, "name"),
               openhab_json_str(t, "type"), pflicht, openhab_json_str(t, "label"));
    }
    cJSON_Delete(beschreibung);

    // Ein Channel-Typ verrät, zu welchem Item-Typ er passt
    cJSON* kanal =
        openhab_channel_types_getChannelType(c, "astro:phaseName", .language = "de");
    printf("\nChannel-Typ %s: %s, Item-Typ %s\nOptionen:", openhab_json_str(kanal, "UID"),
           openhab_json_str(kanal, "label"), openhab_json_str(kanal, "itemType"));
    int i = 0;
    cJSON_ArrayForEach(k, openhab_json_get(kanal, "stateDescription.options")) if (i++ < 5)
        printf("%s %s", i > 1 ? "," : "", openhab_json_str(k, "label"));
    printf(" …\n");
    cJSON_Delete(kanal);
    openhab_client_destroy(c);
    return 0;
}

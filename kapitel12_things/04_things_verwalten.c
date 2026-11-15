// Kapitel 12: Ein Thing anlegen, umkonfigurieren, abschalten und löschen
#include <openhab/openhab.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define UID "astro:moon:buch"

static void status(openhab_client_t* c, const char* was) {
    cJSON* s = openhab_things_getThingStatus(c, UID);
    const char* detail = openhab_json_str(s, "statusDetail");
    printf("%-9s %s%s%s\n", was, openhab_json_str(s, "status"),
           strcmp(detail, "NONE") ? " / " : "", strcmp(detail, "NONE") ? detail : "");
    cJSON_Delete(s);
}

int main(void) {
    const char* url =
        getenv("OPENHAB_URL") ? getenv("OPENHAB_URL") : "http://127.0.0.1:8080";
    openhab_client_t* c = openhab_client_new(url, .token = getenv("OPENHAB_TOKEN"));

    cJSON* d = cJSON_Parse("{\"UID\":\"" UID
                           "\",\"thingTypeUID\":\"astro:moon\","
                           "\"label\":\"Mond (per REST)\",\"configuration\":"
                           "{\"geolocation\":\"48.0950,8.6630\",\"interval\":300}}");
    cJSON* neu = openhab_things_createThing(c, d);
    cJSON_Delete(d);
    printf("angelegt: %s mit %d Channels\n", openhab_json_str(neu, "UID"),
           cJSON_GetArraySize(openhab_json_get(neu, "channels")));
    cJSON_Delete(neu);
    sleep(2);
    status(c, "Status:");

    // Konfiguration ändern – openHAB initialisiert das Thing neu
    d = cJSON_Parse("{\"geolocation\":\"52.5200,13.4050\",\"interval\":60}");
    cJSON_Delete(openhab_things_updateThingConfiguration(c, UID, d));
    cJSON_Delete(d);
    cJSON* thing = openhab_things_getThing(c, UID);
    printf("Ort:      %s (Berlin)\n", openhab_json_str(thing, "configuration.geolocation"));

    // Label ändern: updateThing ersetzt die ganze Definition
    cJSON_ReplaceItemInObject(thing, "label", cJSON_CreateString("Mond über Berlin"));
    cJSON* geaendert = openhab_things_updateThing(c, UID, thing);
    printf("Label:    %s\n", openhab_json_str(geaendert, "label"));
    cJSON_Delete(geaendert);
    cJSON_Delete(thing);

    // Abschalten und wieder einschalten
    cJSON_Delete(openhab_things_disableThing(c, UID));
    sleep(1);
    status(c, "aus:");
    cJSON_Delete(openhab_things_enableThing(c, UID));
    sleep(2);
    status(c, "an:");

    cJSON* weg = openhab_things_deleteThing(c, UID);
    printf("löschen:  %s\n", openhab_json_str(weg, "statusInfo.status"));
    cJSON_Delete(weg);
    cJSON* alle = openhab_things_getThings(c, .summary = OPENHAB_TRUE);
    printf("Things:  ");
    cJSON* t;
    cJSON_ArrayForEach(t, alle) printf(" %s", openhab_json_str(t, "UID"));
    printf("\n");
    cJSON_Delete(alle);
    openhab_client_destroy(c);
    return 0;
}

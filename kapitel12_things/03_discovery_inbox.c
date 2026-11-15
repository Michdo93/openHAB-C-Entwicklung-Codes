// Kapitel 12: Geräte suchen lassen und aus der Inbox übernehmen
#include <openhab/openhab.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void zeige(const char* was, cJSON* r) {
    char* text = openhab_json_print(r);
    printf("%s%s\n", was, text);
    openhab_free(text);
    cJSON_Delete(r);
}

int main(void) {
    const char* url =
        getenv("OPENHAB_URL") ? getenv("OPENHAB_URL") : "http://127.0.0.1:8080";
    openhab_client_t* c = openhab_client_new(url, .token = getenv("OPENHAB_TOKEN"));

    // Das Astro-Binding findet "Geräte" für den eingestellten Standort.
    // Der Standort ist eine Systemeinstellung – auch sie lässt sich per REST setzen.
    cJSON* config = openhab_services_getServiceConfig(c, "org.openhab.i18n");
    cJSON_DeleteItemFromObject(config, "location");
    cJSON_AddStringToObject(config, "location", "48.0950,8.6630"); // Breite, Länge
    cJSON_Delete(openhab_services_updateServiceConfig(c, "org.openhab.i18n", config));
    cJSON_Delete(config);
    config = openhab_services_getServiceConfig(c, "org.openhab.i18n");
    printf("Standort: %s\n", openhab_json_str(config, "location"));
    cJSON_Delete(config);

    zeige("Bindings mit Gerätesuche: ", openhab_discovery_getDiscoveryBindings(c));
    zeige("Suche läuft, Dauer in s: ", openhab_discovery_startBindingScan(c, "astro"));
    sleep(3);
    cJSON* inbox = openhab_inbox_getDiscoveredThings(c);
    cJSON* e;
    cJSON_ArrayForEach(e, inbox) {
        printf("  %-8s %-18s %-20s %s\n", openhab_json_str(e, "flag"),
               openhab_json_str(e, "thingUID"), openhab_json_str(e, "label"),
               openhab_json_str(e, "properties.geolocation"));
    }
    cJSON_Delete(inbox);

    // Den Mond übernehmen: aus dem Inbox-Eintrag wird ein Thing
    zeige("\nübernehmen: ", openhab_inbox_approveDiscoveryResult(c, "astro:moon:local",
                                                                 "Mond über Aldingen"));
    sleep(2);
    cJSON* mond = openhab_things_getThing(c, "astro:moon:local");
    printf("%s %s %d Channels\n", openhab_json_str(mond, "label"),
           openhab_json_str(mond, "statusInfo.status"),
           cJSON_GetArraySize(openhab_json_get(mond, "channels")));
    cJSON_Delete(mond);

    // Wieder entfernen – der Eintrag erscheint bei der nächsten Suche erneut
    cJSON* weg = openhab_things_deleteThing(c, "astro:moon:local");
    printf("löschen: %s\n", openhab_json_str(weg, "statusInfo.status"));
    cJSON_Delete(weg);
    openhab_client_destroy(c);
    return 0;
}

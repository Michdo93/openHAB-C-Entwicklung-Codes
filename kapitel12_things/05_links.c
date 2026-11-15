// Kapitel 12: Items mit Channels verknüpfen – mit und ohne Profil
#include <openhab/openhab.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define SONNE "astro:sun:zuhause"

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

    // Zwei neue Items für Sonnenhöhe und Tagesphase
    cJSON* d = cJSON_Parse(
        "{\"type\":\"Number:Angle\",\"name\":\"Sonne_Hoehe\","
        "\"label\":\"Sonnenhöhe\",\"groupNames\":[\"gHaus\"]}");
    cJSON_Delete(openhab_items_addOrUpdateItem(c, "Sonne_Hoehe", d));
    cJSON_Delete(d);
    d = cJSON_Parse(
        "{\"type\":\"String\",\"name\":\"Sonne_Phase\","
        "\"label\":\"Tagesphase\",\"groupNames\":[\"gHaus\"]}");
    cJSON_Delete(openhab_items_addOrUpdateItem(c, "Sonne_Phase", d));
    cJSON_Delete(d);

    // Welche Profile passen zwischen diesen Channel und ein Number:Angle-Item?
    cJSON* profile = openhab_profile_types_getProfileTypes(
        c, .channelTypeUID = "astro:elevation", .itemType = "Number:Angle");
    printf("passende Profile:");
    cJSON* p;
    cJSON_ArrayForEach(p, profile) printf(" %s", openhab_json_str(p, "uid"));
    printf("\n");
    cJSON_Delete(profile);

    // Link ohne Profil und Link mit Profil "offset" (+90° -> 0° bis 180°)
    cJSON* leer = cJSON_CreateObject();
    zeige("Link 1: ",
          openhab_links_linkItemToChannel(c, "Sonne_Phase", SONNE ":phase#name", leer));
    cJSON_Delete(leer);
    cJSON* profil = cJSON_Parse("{\"profile\":\"system:offset\",\"offset\":\"90 °\"}");
    zeige("Link 2: ", openhab_links_linkItemToChannel(c, "Sonne_Hoehe",
                                                      SONNE ":position#elevation", profil));
    cJSON_Delete(profil);
    sleep(2); // das Binding liefert den Wert beim Verknüpfen
    cJSON* s = openhab_items_getItemState(c, "Sonne_Phase");
    printf("Sonne_Phase = %s\n", s->valuestring);
    cJSON_Delete(s);
    s = openhab_items_getItemState(c, "Sonne_Hoehe");
    printf("Sonne_Hoehe = %s (Höhe + 90°)\n", s->valuestring);
    cJSON_Delete(s);

    cJSON* links = openhab_links_getLinks(c, .itemName = "Sonne_Hoehe");
    cJSON* l;
    cJSON_ArrayForEach(l, links) {
        char* cfg = openhab_json_print(openhab_json_get(l, "configuration"));
        printf("gespeichert: %s -> %s %s\n", openhab_json_str(l, "itemName"),
               openhab_json_str(l, "channelUID"), cfg);
        openhab_free(cfg);
    }
    cJSON_Delete(links);

    // Aufräumen: Links lösen, Items löschen, verwaiste Links prüfen
    zeige("lösen: ",
          openhab_links_unlinkItemFromChannel(c, "Sonne_Phase", SONNE ":phase#name"));
    zeige("alle Links des Items löschen: ", openhab_links_deleteAllLinks(c, "Sonne_Hoehe"));
    cJSON_Delete(openhab_items_deleteItem(c, "Sonne_Hoehe"));
    cJSON_Delete(openhab_items_deleteItem(c, "Sonne_Phase"));
    zeige("verwaiste Links: ", openhab_links_getOrphanLinks(c));
    openhab_client_destroy(c);
    return 0;
}

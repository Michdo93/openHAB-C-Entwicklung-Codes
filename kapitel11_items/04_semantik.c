// Kapitel 11: Das semantische Modell – Orte, Ausstattung und Messpunkte
#include <openhab/openhab.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char* oder(const char* s) { return s ? s : ""; }

int main(void) {
    const char* url =
        getenv("OPENHAB_URL") ? getenv("OPENHAB_URL") : "http://127.0.0.1:8080";
    openhab_client_t* c = openhab_client_new(url, .token = getenv("OPENHAB_TOKEN"));

    // 1. Der Tag-Katalog: Hunderte vordefinierte semantische Tags, hier auf Deutsch
    cJSON* katalog = openhab_tags_getTags(c, .language = "de");
    int orte = 0;
    cJSON* t;
    printf("Orte, z. B.:");
    cJSON_ArrayForEach(
        t, katalog) if (!strncmp(openhab_json_str(t, "uid"), "Location_", 9) && orte++ < 6)
        printf(" %s,", openhab_json_str(t, "label"));
    printf(" …\n%d Tags, davon %d Orte\n", cJSON_GetArraySize(katalog), orte);
    cJSON_Delete(katalog);

    // 2. openHAB leitet aus den Tags Metadaten im Namensraum "semantics" ab
    cJSON* alle =
        openhab_items_getItems(c, .metadata = "semantics", .fields = "name,label,metadata");
    cJSON* it;
    cJSON_ArrayForEach(it, alle) {
        cJSON* sem = openhab_json_get(it, "metadata.semantics");
        if (!sem) continue;
        printf("  %-22s %-32s %-21s %s\n", openhab_json_str(it, "name"),
               openhab_json_str(sem, "value"),
               oder(openhab_json_str(sem, "config.relatesTo")),
               oder(openhab_json_str(sem, "config.hasLocation")));
    }
    cJSON_Delete(alle);

    // 3. Wo befindet sich ein Messpunkt? openHAB folgt dem Modell nach oben.
    cJSON* ort = openhab_items_getSemanticItem(c, "Wohnzimmer_Temperatur", "Location");
    printf("\nWohnzimmer_Temperatur liegt in: %s (%s)\n", openhab_json_str(ort, "name"),
           openhab_json_str(ort, "label"));
    cJSON_Delete(ort);

    // 4. Ein eigener Ort: das Tag wird per REST angelegt und sofort nutzbar
    cJSON* d = cJSON_Parse(
        "{\"uid\":\"Location_Outdoor_Garden_Gartenhaus\","
        "\"name\":\"Gartenhaus\",\"label\":\"Gartenhaus\","
        "\"description\":\"Hütte im Garten\",\"synonyms\":[\"Schuppen\"]}");
    cJSON* tag = openhab_tags_createTag(c, d);
    printf("\nTag angelegt: %s (editierbar: %s)\n", openhab_json_str(tag, "uid"),
           cJSON_IsTrue(openhab_json_get(tag, "editable")) ? "true" : "false");
    cJSON_Delete(tag);
    cJSON_Delete(d);
    d = cJSON_Parse(
        "{\"type\":\"Group\",\"name\":\"gGartenhaus\",\"label\":\"Gartenhaus\","
        "\"tags\":[\"Gartenhaus\"],\"groupNames\":[\"gHaus\"]}");
    cJSON_Delete(openhab_items_addOrUpdateItem(c, "gGartenhaus", d));
    cJSON_Delete(d);
    d = cJSON_Parse(
        "{\"type\":\"Number:Temperature\",\"name\":\"Gartenhaus_Temperatur\","
        "\"label\":\"Temperatur\",\"tags\":[\"Measurement\",\"Temperature\"],"
        "\"groupNames\":[\"gGartenhaus\"]}");
    cJSON_Delete(openhab_items_addOrUpdateItem(c, "Gartenhaus_Temperatur", d));
    cJSON_Delete(d);
    cJSON* neu = openhab_items_getItem(c, "Gartenhaus_Temperatur", .metadata = "semantics");
    char* cfg = openhab_json_print(openhab_json_get(neu, "metadata.semantics.config"));
    printf("Semantik: %s %s\n", openhab_json_str(neu, "metadata.semantics.value"), cfg);
    openhab_free(cfg);
    cJSON_Delete(neu);

    cJSON_Delete(openhab_items_deleteItem(c, "Gartenhaus_Temperatur"));
    cJSON_Delete(openhab_items_deleteItem(c, "gGartenhaus"));
    cJSON* r = openhab_tags_deleteTag(c, "Location_Outdoor_Garden_Gartenhaus");
    char* text = openhab_json_print(r);
    printf("Tag löschen: %s\n", text);
    openhab_free(text);
    cJSON_Delete(r);
    openhab_client_destroy(c);
    return 0;
}

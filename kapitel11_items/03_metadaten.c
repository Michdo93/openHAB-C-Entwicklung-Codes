// Kapitel 11: Metadaten – Zusatzinformationen, die openHABs Verhalten ändern
#include <openhab/openhab.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define NAME "Klingel_Status"

// Ergebnis kompakt ausgeben und freigeben
static void zeige(const char* was, cJSON* r) {
    char* text = openhab_json_print(r);
    printf("%s %s\n", was, text); // printf zählt Bytes: Umlaute von Hand ausgleichen
    openhab_free(text);
    cJSON_Delete(r);
}

static cJSON* json(const char* text) { return cJSON_Parse(text); }

int main(void) {
    const char* url =
        getenv("OPENHAB_URL") ? getenv("OPENHAB_URL") : "http://127.0.0.1:8080";
    openhab_client_t* c = openhab_client_new(url, .token = getenv("OPENHAB_TOKEN"));
    cJSON* d = json("{\"type\":\"String\",\"name\":\"" NAME
                    "\",\"label\":\"Klingel\","
                    "\"groupNames\":[\"gHaus\"]}");
    cJSON_Delete(openhab_items_addOrUpdateItem(c, NAME, d));
    cJSON_Delete(d);

    // 1. stateDescription: Anzeigeformat und erlaubte Werte für Oberflächen
    cJSON* m = json(
        "{\"value\":\" \",\"config\":{\"pattern\":\"%s\",\"readOnly\":false,"
        "\"options\":\"RUHE=Ruhe,KLINGELT=Es klingelt!,GEOEFFNET=Tür geöffnet\"}}");
    zeige("stateDescription:", openhab_items_addMetadata(c, NAME, "stateDescription", m));
    cJSON_Delete(m);
    // 2. expire: Zustand fällt nach zehn Sekunden automatisch zurück
    m = json("{\"value\":\"10s,state=RUHE\"}");
    zeige("expire:          ", openhab_items_addMetadata(c, NAME, "expire", m));
    cJSON_Delete(m);
    // 3. eigener Namensraum: beliebige Daten für die eigene Anwendung
    m = json("{\"value\":\"klingel\",\"config\":{\"raum\":\"Flur\",\"prioritaet\":2}}");
    zeige("eigener:         ", openhab_items_addMetadata(c, NAME, "restbuch", m));
    cJSON_Delete(m);
    zeige("Namensräume:     ", openhab_items_getMetadataNamespaces(c, NAME));

    cJSON* item =
        openhab_items_getItem(c, NAME, .metadata = "stateDescription,expire,restbuch");
    cJSON* ns;
    cJSON_ArrayForEach(ns, openhab_json_get(item, "metadata")) { // je Namensraum eine Zeile
        printf("  %-16s value=\"%s\"\n", ns->string, openhab_json_str(ns, "value"));
        cJSON* k; // ->string: der Schlüssel
        cJSON_ArrayForEach(k, openhab_json_get(ns, "config")) {
            char* text = openhab_json_print(k);
            printf("  %16s %s=%s\n", "", k->string, text);
            openhab_free(text);
        }
    }
    cJSON_Delete(item);

    // Die Anzeige nutzt die Optionen aus stateDescription
    cJSON_Delete(openhab_items_postUpdate(c, NAME, "KLINGELT"));
    usleep(300 * 1000); // openHAB verarbeitet Updates asynchron
    item = openhab_items_getItem(c, NAME);
    const char* state = openhab_json_str(item, "state");
    const char* anzeige = "?";
    cJSON* o;
    cJSON_ArrayForEach(
        o, openhab_json_get(
               item, "stateDescription.options")) if (!strcmp(openhab_json_str(o, "value"),
                                                              state)) anzeige =
        openhab_json_str(o, "label");
    printf("Zustand: %s | Anzeige: %s\n", state, anzeige);
    cJSON_Delete(item);

    sleep(11); // expire setzt nach zehn Sekunden zurück
    cJSON* s = openhab_items_getItemState(c, NAME);
    printf("nach 11 s: %s\n", s->valuestring);
    cJSON_Delete(s);
    zeige("entfernen:       ", openhab_items_removeMetadata(c, NAME, "restbuch"));
    cJSON_Delete(openhab_items_deleteItem(c, NAME));
    openhab_client_destroy(c);
    return 0;
}

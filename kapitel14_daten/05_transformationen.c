// Kapitel 14: Ein Add-on installieren und eine Transformation anlegen
#include <openhab/openhab.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define UID "config:map:tuer"

static int map_bereit(openhab_client_t* c) {
    cJSON* dienste = openhab_transformations_getTransformationServices(c);
    int bereit = 0;
    cJSON* d;
    cJSON_ArrayForEach(d, dienste) bereit |=
        cJSON_IsString(d) && !strcmp(d->valuestring, "MAP");
    cJSON_Delete(dienste);
    return bereit;
}

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

    // 1. Den MAP-Transformationsdienst installieren (falls noch nicht geschehen)
    cJSON* addon = openhab_addons_getAddon(c, "transformation-map");
    if (!cJSON_IsTrue(openhab_json_get(addon, "installed"))) {
        zeige("installieren: ", openhab_addons_installAddon(c, "transformation-map"));
        while (!map_bereit(c)) sleep(1);
    }
    cJSON_Delete(addon);
    zeige("Transformationsdienste: ", openhab_transformations_getTransformationServices(c));

    // 2. Eine eigene MAP-Transformation anlegen
    cJSON* t = cJSON_CreateObject();
    cJSON_AddStringToObject(t, "uid", UID);
    cJSON_AddStringToObject(t, "label", "Türzustand");
    cJSON_AddStringToObject(t, "type", "map");
    cJSON_AddStringToObject(cJSON_AddObjectToObject(t, "configuration"), "function",
                            "OPEN=offen\nCLOSED=geschlossen\nNULL=unbekannt\n");
    zeige("anlegen: ", openhab_transformations_updateTransformation(c, UID, t));
    cJSON_Delete(t);
    cJSON* alle = openhab_transformations_getTransformations(c);
    cJSON_ArrayForEach(t, alle)
        printf("  %-16s %-4s %s\n", openhab_json_str(t, "uid"), openhab_json_str(t, "type"),
               cJSON_IsTrue(openhab_json_get(t, "editable")) ? "änderbar" : "aus Datei");
    cJSON_Delete(alle);

    // 3. Im Anzeigeformat eines Items verwenden
    cJSON* m =
        cJSON_Parse("{\"value\":\" \",\"config\":{\"pattern\":\"MAP(" UID "):%s\"}}");
    cJSON_Delete(openhab_items_addMetadata(c, "Flur_Haustuer", "stateDescription", m));
    cJSON_Delete(m);
    const char* zustaende[] = { "OPEN", "CLOSED" };
    for (int i = 0; i < 2; i++) {
        cJSON_Delete(openhab_items_postUpdate(c, "Flur_Haustuer", zustaende[i]));
        usleep(300 * 1000); // postUpdate wird asynchron verarbeitet
        cJSON* tuer = openhab_items_getItem(c, "Flur_Haustuer");
        printf("Zustand %-6s -> Anzeige %s\n", openhab_json_str(tuer, "state"),
               openhab_json_str(tuer, "transformedState"));
        cJSON_Delete(tuer);
    }
    // Aufräumen – das Add-on bleibt installiert
    cJSON_Delete(openhab_items_removeMetadata(c, "Flur_Haustuer", "stateDescription"));
    zeige("löschen: ", openhab_transformations_deleteTransformation(c, UID));
    openhab_client_destroy(c);
    return 0;
}

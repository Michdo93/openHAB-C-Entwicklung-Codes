// Kapitel 11: Ein Item anlegen, lesen, ändern und löschen (CRUD)
#include <openhab/openhab.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NAME "Garten_Pumpe"

int main(void) {
    const char* url =
        getenv("OPENHAB_URL") ? getenv("OPENHAB_URL") : "http://127.0.0.1:8080";
    openhab_client_t* c = openhab_client_new(url, .token = getenv("OPENHAB_TOKEN"));

    // Create – die Definition als JSON-Text; cJSON_Parse macht ein Objekt daraus
    cJSON* definition = cJSON_Parse(
        "{\"type\":\"Switch\",\"name\":\"" NAME
        "\",\"label\":\"Gartenpumpe\","
        "\"category\":\"water\",\"tags\":[\"Switch\"],\"groupNames\":[\"gHaus\"]}");
    cJSON* neu = openhab_items_addOrUpdateItem(c, NAME, definition);
    printf("angelegt:  %s %s %s\n", openhab_json_str(neu, "name"),
           openhab_json_str(neu, "type"), openhab_json_str(neu, "state"));
    cJSON_Delete(neu);

    // Read – und gleich bedienen: ein per REST angelegtes Item ist ein normales Item
    cJSON_Delete(openhab_items_sendCommand(c, NAME, "ON"));
    usleep(300 * 1000); // Commands wirken asynchron
    cJSON* state = openhab_items_getItemState(c, NAME);
    printf("gelesen:   %s\n", state->valuestring);
    cJSON_Delete(state);

    // Update – PUT ersetzt die ganze Definition (idempotent)
    cJSON_ReplaceItemInObject(definition, "label",
                              cJSON_CreateString("Gartenpumpe Zisterne"));
    cJSON_AddItemToArray(cJSON_GetObjectItem(definition, "groupNames"),
                         cJSON_CreateString("gWerkstatt"));
    cJSON_Delete(openhab_items_addOrUpdateItem(c, NAME, definition));
    cJSON_Delete(definition);

    cJSON* pumpe = openhab_items_getItem(c, NAME);
    char* gruppen = openhab_json_print(openhab_json_get(pumpe, "groupNames"));
    printf("geändert:  %s %s Zustand: %s\n", openhab_json_str(pumpe, "label"), gruppen,
           openhab_json_str(pumpe, "state"));
    openhab_free(gruppen);
    printf("verwaltet: %s\n", cJSON_IsTrue(openhab_json_get(pumpe, "editable"))
                                  ? "per REST änderbar"
                                  : "aus einer Datei");
    cJSON_Delete(pumpe);

    // Delete – und prüfen, dass es weg ist
    cJSON* r = openhab_items_deleteItem(c, NAME);
    char* text = openhab_json_print(r);
    printf("löschen:   %s\n", text);
    openhab_free(text);
    cJSON_Delete(r);
    r = openhab_items_getItem(c, NAME);
    printf("danach:    %s\n", openhab_error(r));
    cJSON_Delete(r);
    openhab_client_destroy(c);
    return 0;
}

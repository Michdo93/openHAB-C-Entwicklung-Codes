// Kapitel 9: JSON mit cJSON – lesen, bauen, ausgeben, freigeben
#include <cjson/cJSON.h>
#include <stdio.h>

int main(void) {
    const char* text =
        "{\"name\":\"Wohnzimmer_Temperatur\",\"state\":\"21.5 "
        "°C\",\"type\":\"Number:Temperature\","
        "\"tags\":[\"Measurement\",\"Temperature\"],\"stateDescription\":{\"pattern\":\"%."
        "1f °C\"}}";

    cJSON* item = cJSON_Parse(text); // NULL bei ungültigem JSON
    if (!item) {
        fprintf(stderr, "kein JSON vor: %s\n", cJSON_GetErrorPtr());
        return 1;
    }
    const cJSON* name = cJSON_GetObjectItemCaseSensitive(item, "name");
    const cJSON* tags = cJSON_GetObjectItemCaseSensitive(item, "tags");
    const cJSON* muster = cJSON_GetObjectItemCaseSensitive(
        cJSON_GetObjectItemCaseSensitive(item, "stateDescription"), "pattern");
    if (cJSON_IsString(name)) printf("Name:   %s\n", name->valuestring);
    printf("Tags:   %d –", cJSON_GetArraySize(tags));
    const cJSON* tag;
    cJSON_ArrayForEach(tag, tags) printf(" %s", tag->valuestring);
    printf("\nMuster: %s\n", cJSON_IsString(muster) ? muster->valuestring : "(keins)");
    printf("Label:  %s\n", cJSON_GetObjectItem(item, "label") ? "vorhanden" : "fehlt");

    // Ein neues Item als JSON bauen: Unterobjekte gehören danach dem Elternobjekt
    cJSON* neu = cJSON_CreateObject();
    cJSON_AddStringToObject(neu, "type", "Switch");
    cJSON_AddStringToObject(neu, "name", "Buch_Testschalter");
    cJSON_AddStringToObject(neu, "label", "Testschalter");
    cJSON* gruppen = cJSON_AddArrayToObject(neu, "groupNames");
    cJSON_AddItemToArray(gruppen, cJSON_CreateString("gWohnzimmer"));
    char* kompakt = cJSON_PrintUnformatted(neu);
    printf("Neu:    %s\n", kompakt);

    cJSON_free(kompakt); // Text von cJSON mit cJSON_free
    cJSON_Delete(neu);   // löscht auch "groupNames"
    cJSON_Delete(item);
    return 0;
}

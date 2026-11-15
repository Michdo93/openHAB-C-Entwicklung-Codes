// Kapitel 10: Items lesen – alle, eines, nur den Zustand, gefiltert
#include <openhab/openhab.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int nach_name(const void* a, const void* b) {
    return strcmp(openhab_json_str(*(cJSON* const*)a, "name"),
                  openhab_json_str(*(cJSON* const*)b, "name"));
}

int main(void) {
    const char* url =
        getenv("OPENHAB_URL") ? getenv("OPENHAB_URL") : "http://127.0.0.1:8080";
    openhab_client_t* c = openhab_client_new(url, .token = getenv("OPENHAB_TOKEN"));

    // Alle Items, aber nur drei Felder – das spart Daten
    cJSON* items = openhab_items_getItems(c, .fields = "name,type,state");
    if (openhab_is_error(items)) {
        fprintf(stderr, "Fehler: %s\n", openhab_error(items));
        return 1;
    }
    int n = cJSON_GetArraySize(items);
    cJSON** liste = malloc(n * sizeof *liste); // zum Sortieren: Zeiger auf die Einträge
    for (int i = 0; i < n; i++) liste[i] = cJSON_GetArrayItem(items, i);
    qsort(liste, n, sizeof *liste, nach_name);
    printf("%d Items gefunden:\n", n);
    for (int i = 0; i < n; i++)
        printf("  %-28s %-22s %s\n", openhab_json_str(liste[i], "name"),
               openhab_json_str(liste[i], "type"), openhab_json_str(liste[i], "state"));
    free(liste);
    cJSON_Delete(items);

    // Ein Item mit allen Details
    cJSON* licht = openhab_items_getItem(c, "Wohnzimmer_Licht");
    printf("\nLabel: %s | Gruppen:", openhab_json_str(licht, "label"));
    cJSON* g;
    cJSON_ArrayForEach(g, openhab_json_get(licht, "groupNames"))
        printf(" %s", g->valuestring);
    printf("\nTags:");
    cJSON_ArrayForEach(g, openhab_json_get(licht, "tags")) printf(" %s", g->valuestring);
    cJSON_Delete(licht);

    // Nur der Zustand: ein cJSON-String
    cJSON* temp = openhab_items_getItemState(c, "Wohnzimmer_Temperatur");
    printf("\nTemperatur: %s\n", temp->valuestring);
    cJSON_Delete(temp);

    // Filter: nach Typ und nach Tag
    cJSON* dimmer = openhab_items_getItems(c, .type = "Dimmer", .fields = "name");
    printf("Dimmer: %s\n", openhab_json_str(dimmer, "0.name"));
    cJSON* lichter = openhab_items_getItems(c, .tags = "Light", .fields = "name");
    printf("Mit Tag 'Light':");
    cJSON_ArrayForEach(g, lichter) printf(" %s", openhab_json_str(g, "name"));
    printf("\n");
    cJSON_Delete(dimmer);
    cJSON_Delete(lichter);
    openhab_client_destroy(c);
    return 0;
}

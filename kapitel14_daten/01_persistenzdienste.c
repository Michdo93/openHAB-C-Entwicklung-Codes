// Kapitel 14: Persistenzdienste und die Items, die sie speichern
#include <openhab/openhab.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    const char* url =
        getenv("OPENHAB_URL") ? getenv("OPENHAB_URL") : "http://127.0.0.1:8080";
    openhab_client_t* c = openhab_client_new(url, .token = getenv("OPENHAB_TOKEN"));

    cJSON* dienste = openhab_persistence_getServices(c, .language = "de");
    cJSON* d;
    cJSON_ArrayForEach(d, dienste)
        printf("%-9s %-10s %s\n", openhab_json_str(d, "id"), openhab_json_str(d, "type"),
               openhab_json_str(d, "label"));
    cJSON_Delete(dienste);

    // Was liegt im Arbeitsspeicher-Dienst – und seit wann?
    cJSON* items = openhab_persistence_getItemsFromService(c, .serviceID = "inmemory");
    int i = 0;
    cJSON_ArrayForEach(d, items) if (i++ < 6)
        printf("  %-24s %4.0f Werte, ältester %s\n", openhab_json_str(d, "name"),
               openhab_json_num(d, "count", 0), openhab_json_str(d, "earliest"));
    printf("  … %d Items insgesamt\n", cJSON_GetArraySize(items));
    cJSON_Delete(items);

    // rrd4j kann keine Itemliste liefern – die Bibliothek erklärt den Grund
    cJSON* r = openhab_persistence_getItemsFromService(c, .serviceID = "rrd4j");
    char* text = openhab_json_print(r);
    printf("rrd4j: %s\n", text);
    openhab_free(text);
    cJSON_Delete(r);
    openhab_client_destroy(c);
    return 0;
}

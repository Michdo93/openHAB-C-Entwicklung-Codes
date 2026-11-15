// Kapitel 14: Gespeicherte Werte gezielt löschen
#include <openhab/openhab.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define DIENST "inmemory"
#define ITEM   "Werkstatt_Temperatur"

static void iso(time_t t, char* puffer) {
    strftime(puffer, 32, "%Y-%m-%dT%H:%M:%SZ", gmtime(&t));
}

static int anzahl(openhab_client_t* c, time_t jetzt) {
    char von[32], bis[32];
    iso(jetzt - 3 * 86400, von);
    iso(jetzt, bis);
    cJSON* r = openhab_persistence_getItemPersistenceData(c, ITEM, .serviceID = DIENST,
                                                          .startTime = von, .endTime = bis);
    int n = (int)openhab_json_num(r, "datapoints", -1);
    cJSON_Delete(r);
    return n;
}

int main(void) {
    const char* url =
        getenv("OPENHAB_URL") ? getenv("OPENHAB_URL") : "http://127.0.0.1:8080";
    openhab_client_t* c = openhab_client_new(url, .token = getenv("OPENHAB_TOKEN"));
    time_t jetzt = time(NULL);
    printf("vorher:  %d\n", anzahl(c, jetzt));

    // Nur die Nacht von 0 bis 6 Uhr von gestern löschen (Ortszeit)
    time_t gestern = jetzt - 86400;
    struct tm t = *localtime(&gestern);
    t.tm_hour = t.tm_min = t.tm_sec = 0;
    t.tm_isdst = -1;
    time_t mitternacht = mktime(&t);
    char von[32], bis[32];
    iso(mitternacht, von);
    iso(mitternacht + 6 * 3600, bis);
    cJSON* r = openhab_persistence_deleteItemData(c, ITEM, von, bis, DIENST);
    char* text = openhab_json_print(r);
    printf("%s\n", text);
    openhab_free(text);
    cJSON_Delete(r);
    printf("nachher: %d\n", anzahl(c, jetzt));

    // rrd4j lehnt das Löschen ab – genau wie das Schreiben
    iso(jetzt, bis);
    r = openhab_persistence_deleteItemData(c, ITEM, von, bis, "rrd4j");
    printf("rrd4j: %s\n", strrchr(openhab_error(r), '('));
    cJSON_Delete(r);
    openhab_client_destroy(c);
    return 0;
}

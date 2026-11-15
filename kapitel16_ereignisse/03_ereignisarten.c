// Kapitel 16: Ein Strom für vieles – Items, Things und Links gleichzeitig beobachten
#include <openhab/openhab.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SONNE "astro:sun:zuhause"
#define KANAL SONNE ":position#azimuth"
#define THEMEN                                                      \
    "openhab/items/Sonne_Azimut/added,"        /* Item angelegt */  \
    "openhab/items/Sonne_Azimut/removed,"      /* Item gelöscht */ \
    "openhab/items/Sonne_Azimut/statechanged," /* Zustand */        \
    "openhab/things/*/statuschanged,"          /* alle Things */    \
    "openhab/links/*/*"                        /* Links */

static openhab_client_t* c;

// Erzeugt nacheinander typische Ereignisse und unterbricht dann den Strom
static void* aktionen(void* strom) {
    sleep(1);
    cJSON* d = cJSON_Parse(
        "{\"type\":\"Number:Angle\",\"name\":\"Sonne_Azimut\","
        "\"label\":\"Sonnenrichtung\"}");
    cJSON_Delete(openhab_items_addOrUpdateItem(c, "Sonne_Azimut", d));
    cJSON_Delete(d);
    cJSON* leer = cJSON_CreateObject();
    cJSON_Delete(openhab_links_linkItemToChannel(c, "Sonne_Azimut", KANAL, leer));
    cJSON_Delete(leer);
    sleep(1);
    cJSON_Delete(openhab_things_disableThing(c, SONNE));
    sleep(1);
    cJSON_Delete(openhab_things_enableThing(c, SONNE));
    sleep(2);
    cJSON_Delete(openhab_links_unlinkItemFromChannel(c, "Sonne_Azimut", KANAL));
    cJSON_Delete(openhab_items_deleteItem(c, "Sonne_Azimut"));
    sleep(1);
    openhab_sse_interrupt(strom); // weckt den Leser, der Strom endet
    return NULL;
}

static void info(const char* typ, const cJSON* p) {
    if (!strcmp(typ, "ThingStatusInfoChangedEvent")) // payload: [neu, alt]
        printf("%s -> %s (%s)", openhab_json_str(p, "1.status"),
               openhab_json_str(p, "0.status"), openhab_json_str(p, "0.statusDetail"));
    else if (!strncmp(typ, "ItemChannelLink", 15))
        printf("%s <-> %s", openhab_json_str(p, "itemName"),
               strrchr(openhab_json_str(p, "channelUID"), ':') + 1);
    else if (openhab_json_str(p, "value"))
        printf("%s", openhab_json_str(p, "value"));
    else if (openhab_json_str(p, "name"))
        printf("%s", openhab_json_str(p, "name"));
}

int main(void) {
    const char* url =
        getenv("OPENHAB_URL") ? getenv("OPENHAB_URL") : "http://127.0.0.1:8080";
    c = openhab_client_new(url, .token = getenv("OPENHAB_TOKEN"));
    openhab_sse_t* strom = openhab_events_getEvents(c, .topics = THEMEN);
    pthread_t t;
    pthread_create(&t, NULL, aktionen, strom);
    cJSON* e;
    while ((e = openhab_sse_next_event(strom, -1, 0))) { // -1: ohne Zeitgrenze
        const char* typ = openhab_json_str(e, "type");
        printf("%-34s ", typ);
        info(typ, openhab_json_get(e, "payload"));
        printf("\n");
        cJSON_Delete(e);
    }
    pthread_join(t, NULL);
    openhab_sse_close(strom);
    openhab_client_destroy(c);
    return 0;
}

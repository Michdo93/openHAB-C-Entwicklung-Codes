// Kapitel 15: Änderungen auf einer Sitemap-Seite live verfolgen
#include <openhab/openhab.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SITEMAP "buch_tablet" // aus 02_sitemap_anlegen.c

static openhab_client_t* c;

// printf zählt Bytes: Umlaute belegen in UTF-8 zwei – die Breite um die Folgebytes erhöhen
static int breite(const char* s, int zeichen) {
    for (; s && *s; s++)
        if ((*s & 0xC0) == 0x80) zeichen++;
    return zeichen;
}

// openHAB antwortet auf den Ereignisstrom erst mit dem ersten Ereignis –
// deshalb ändert ein zweiter Thread etwas, während der Hauptthread den Strom öffnet
static void* aendern(void* arg) {
    (void)arg;
    sleep(1);
    cJSON* d = openhab_items_getItemState(c, "Wohnzimmer_Dimmer");
    cJSON_Delete(openhab_items_sendCommand(c, "Wohnzimmer_Dimmer",
                                           strcmp(d->valuestring, "35") ? "35" : "65"));
    cJSON_Delete(d);
    cJSON_Delete(openhab_items_postUpdate(c, "Flur_Haustuer", "OPEN"));
    return NULL;
}

int main(void) {
    const char* url =
        getenv("OPENHAB_URL") ? getenv("OPENHAB_URL") : "http://127.0.0.1:8080";
    c = openhab_client_new(url, .token = getenv("OPENHAB_TOKEN"));

    // 1. Abonnement anlegen und an eine Seite binden: die Seite mit der ID abrufen
    cJSON* abo = openhab_sitemaps_subscribeToSitemapEvents(c);
    const char* id = openhab_json_str(abo, "subscriptionID");
    cJSON* seite =
        openhab_sitemaps_getSitemapPage(c, SITEMAP, SITEMAP, .subscriptionID = id);
    printf("Abonnement %.8s… für Seite »%s«\n", id, openhab_json_str(seite, "title"));
    cJSON_Delete(seite);

    pthread_t t;
    pthread_create(&t, NULL, aendern, NULL);
    // 2. Den Strom lesen: Sitemap-Ereignisse haben kein "topic" – include_alive = 1
    openhab_sse_t* strom =
        openhab_sitemaps_getSitemapEvents(c, id, .sitemapName = SITEMAP, .pageID = SITEMAP);
    int anzahl = 0;
    while (anzahl < 3) {
        cJSON* e = openhab_sse_next_event(strom, 10000, 1); // höchstens 10 s warten
        if (!e) break;                                      // Zeit abgelaufen oder Fehler
        if (openhab_json_get(e, "widgetId")) {
            const char* label = openhab_json_str(e, "label");
            printf("  Widget %-5s %-*s Zustand %s\n", openhab_json_str(e, "widgetId"),
                   breite(label, 24), label, openhab_json_str(e, "item.state"));
            anzahl++;
        }
        cJSON_Delete(e);
    }
    openhab_sse_close(strom);
    pthread_join(t, NULL);
    cJSON_Delete(abo);

    cJSON_Delete(openhab_items_postUpdate(c, "Flur_Haustuer", "CLOSED"));
    cJSON* r = openhab_ui_deleteUIComponent(c, "system:sitemap", SITEMAP);
    char* text = openhab_json_print(r);
    printf("Sitemap löschen: %s\n", text);
    openhab_free(text);
    cJSON_Delete(r);
    openhab_client_destroy(c);
    return 0;
}

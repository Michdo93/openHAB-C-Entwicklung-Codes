// Kapitel 15: Sitemaps lesen – die Bedienseiten, die Apps und Basic UI zeigen
#include <openhab/openhab.h>
#include <stdio.h>
#include <stdlib.h>

// printf zählt Bytes: Umlaute belegen in UTF-8 zwei – die Breite um die Folgebytes erhöhen
static int breite(const char* s, int zeichen) {
    for (; s && *s; s++)
        if ((*s & 0xC0) == 0x80) zeichen++;
    return zeichen;
}

// Rekursiv: jedes Widget mit Typ, Beschriftung und Zustand, Unter-Widgets eingerückt
static void zeige(const cJSON* widgets, int tiefe) {
    cJSON* w;
    cJSON_ArrayForEach(w, widgets) {
        const char* state = openhab_json_str(w, "item.state");
        const char* label = openhab_json_str(w, "label");
        printf("%*s%-12s %-*s%s%s\n", 2 * tiefe, "", openhab_json_str(w, "type"),
               state ? breite(label, 28) : 0, label, state ? " = " : "",
               state ? state : "");
        zeige(openhab_json_get(w, "widgets"), tiefe + 1); // NULL: Schleife läuft nicht
    }
}

int main(void) {
    const char* url =
        getenv("OPENHAB_URL") ? getenv("OPENHAB_URL") : "http://127.0.0.1:8080";
    openhab_client_t* c = openhab_client_new(url, .token = getenv("OPENHAB_TOKEN"));

    cJSON* alle = openhab_sitemaps_getSitemaps(c);
    cJSON* s;
    cJSON_ArrayForEach(s, alle) printf("Sitemap %s: %s\n", openhab_json_str(s, "name"),
                                       openhab_json_str(s, "label"));
    cJSON_Delete(alle);

    cJSON* sitemap = openhab_sitemaps_getSitemap(c, "restbuch", .language = "de");
    printf("\nStartseite »%s«:\n", openhab_json_str(sitemap, "homepage.title"));
    zeige(openhab_json_get(sitemap, "homepage.widgets"), 1);
    cJSON_Delete(sitemap);
    openhab_client_destroy(c);
    return 0;
}

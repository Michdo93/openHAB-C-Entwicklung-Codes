// Kapitel 15: Eine Sitemap als UI-Komponente erzeugen
#include <openhab/openhab.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UID "buch_tablet"

static cJSON* json(const char* text) { // JSON mit einfachen Anführungszeichen
    char* kopie = strdup(text);
    for (char* p = kopie; *p; p++)
        if (*p == '\'') *p = '"';
    cJSON* j = cJSON_Parse(kopie);
    free(kopie);
    return j;
}

int main(void) {
    const char* url =
        getenv("OPENHAB_URL") ? getenv("OPENHAB_URL") : "http://127.0.0.1:8080";
    openhab_client_t* c = openhab_client_new(url, .token = getenv("OPENHAB_TOKEN"));

    cJSON* tablet = json(
        "{'uid':'" UID
        "','component':'Sitemap','config':{'label':'Tablet im Flur'},"
        " 'slots':{'widgets':["
        "  {'component':'Frame','config':{'label':'Licht'},'slots':{'widgets':["
        "    {'component':'Switch','config':{'item':'Wohnzimmer_Licht','label':'Decke'}},"
        "    "
        "{'component':'Slider','config':{'item':'Wohnzimmer_Dimmer','label':'Leselampe'}},"
        "    {'component':'Switch','config':{'item':'gLichter','label':'Alle Lichter',"
        "                                     'mappings':['OFF=Alle aus']}}]}},"
        "  {'component':'Frame','config':{'label':'Haus'},'slots':{'widgets':["
        "    {'component':'Text','config':{'item':'Flur_Haustuer','label':'Haustür'}},"
        "    {'component':'Selection','config':{'item':'Haus_Status','label':'Modus',"
        "     'mappings':['Alle zu Hause=Zu Hause','Kino=Kino','Gute "
        "Nacht=Nacht']}}]}}]}}");
    cJSON_Delete(openhab_ui_deleteUIComponent(c, "system:sitemap", UID)); // falls vorhanden
    cJSON* r = openhab_ui_addUIComponent(c, "system:sitemap", tablet);
    printf("anlegen: %s\n", openhab_json_str(r, "uid"));
    cJSON_Delete(r);
    cJSON_Delete(tablet);

    cJSON* alle = openhab_sitemaps_getSitemaps(c);
    cJSON *s, *w;
    printf("Sitemaps jetzt:");
    cJSON_ArrayForEach(s, alle) printf(" %s", openhab_json_str(s, "name"));
    printf("\n");
    cJSON_Delete(alle);

    cJSON* sitemap = openhab_sitemaps_getSitemap(c, UID);
    cJSON_ArrayForEach(s, openhab_json_get(sitemap, "homepage.widgets")) {
        printf("  %s:", openhab_json_str(s, "label"));
        cJSON_ArrayForEach(w, openhab_json_get(s, "widgets"))
            printf(" %s (%s)", openhab_json_str(w, "label"), openhab_json_str(w, "type"));
        printf("\n");
    }
    cJSON_Delete(sitemap);

    cJSON* komponenten = openhab_ui_getUIComponents(c, "system:sitemap");
    printf("UI-Komponenten:");
    cJSON_ArrayForEach(s, komponenten) printf(" %s", openhab_json_str(s, "uid"));
    printf("\n");
    cJSON_Delete(komponenten);
    // Die Sitemap bleibt bestehen – 03_sitemap_ereignisse nutzt sie.
    openhab_client_destroy(c);
    return 0;
}

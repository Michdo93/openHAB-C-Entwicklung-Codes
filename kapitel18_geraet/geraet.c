// Kapitel 18: Die Logik des Geräts
#define _POSIX_C_SOURCE 200809L
#include "geraet.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/statvfs.h>
#include <time.h>

static void melde(geraet* g, const char* item, const char* state) {
    cJSON* r = openhab_items_postUpdate(g->client, item, state);
    if (openhab_is_error(r)) fprintf(stderr, "%s: %s\n", item, openhab_error(r));
    cJSON_Delete(r);
}

static void metadaten(geraet* g, const char* item, const char* ns, const char* json) {
    cJSON* m = cJSON_Parse(json);
    cJSON_Delete(openhab_items_addMetadata(g->client, item, ns, m));
    cJSON_Delete(m);
}

// Die Items des Geräts: Name, Typ, Label, Einheit (oder NULL), Anzeigeformat
static const struct {
    const char *name, *typ, *label, *einheit, *muster;
} ITEMS[] = {
    { "Pi_Temperatur", "Number:Temperature", "CPU-Temperatur", "°C", "%.1f °C" },
    { "Pi_Last", "Number:Dimensionless", "Last", "%", "%.0f %%" },
    { "Pi_Speicher", "Number:Dimensionless", "Speicher belegt", "%", "%.0f %%" },
    { "Pi_Netz", "Number:DataTransferRate", "Netzverkehr", "kbit/s", "%.1f kbit/s" },
    { "Pi_Laufzeit", "Number:Time", "Laufzeit", "h", "%.1f h" },
    { "Pi_Luefter", "Switch", "Lüfter", NULL, NULL },
    { "Pi_Online", "Switch", "Gerät erreichbar", NULL, NULL },
    { "Pi_Aktion", "String", "Aktion", NULL, NULL },
    { "Pi_Antwort", "String", "Antwort", NULL, NULL },
};
enum { ANZAHL = sizeof ITEMS / sizeof ITEMS[0] };

int geraet_anmelden(geraet* g, const char* rechnername) {
    cJSON* liste = cJSON_CreateArray();
    cJSON* gruppe = cJSON_CreateObject();
    char label[128];
    snprintf(label, sizeof label, "Raspberry Pi %s", rechnername);
    cJSON_AddStringToObject(gruppe, "type", "Group");
    cJSON_AddStringToObject(gruppe, "name", "gPi");
    cJSON_AddStringToObject(gruppe, "label", label);
    cJSON_AddItemToObject(gruppe, "tags",
                          cJSON_CreateStringArray((const char*[]){ "Equipment" }, 1));
    cJSON_AddItemToArray(liste, gruppe);
    for (int i = 0; i < ANZAHL; i++) {
        cJSON* it = cJSON_CreateObject();
        cJSON_AddStringToObject(it, "type", ITEMS[i].typ);
        cJSON_AddStringToObject(it, "name", ITEMS[i].name);
        cJSON_AddStringToObject(it, "label", ITEMS[i].label);
        cJSON_AddItemToObject(it, "groupNames",
                              cJSON_CreateStringArray((const char*[]){ "gPi" }, 1));
        cJSON_AddItemToArray(liste, it);
    }
    cJSON* r = openhab_items_addOrUpdateItems(g->client, liste); // ein Aufruf, idempotent
    cJSON_Delete(liste);
    if (openhab_is_error(r)) {
        fprintf(stderr, "anmelden: %s\n", openhab_error(r));
        cJSON_Delete(r);
        return -1;
    }
    cJSON_Delete(r);
    char json[160];
    for (int i = 0; i < ANZAHL; i++) {
        if (!ITEMS[i].einheit) continue;
        snprintf(json, sizeof json, "{\"value\":\"%s\"}", ITEMS[i].einheit);
        metadaten(g, ITEMS[i].name, "unit", json);
        snprintf(json, sizeof json, "{\"value\":\" \",\"config\":{\"pattern\":\"%s\"}}",
                 ITEMS[i].muster);
        metadaten(g, ITEMS[i].name, "stateDescription", json);
    }
    // Lebenszeichen: nach drei Intervallen ohne Meldung fällt Pi_Online auf OFF
    snprintf(json, sizeof json, "{\"value\":\"%ds,state=OFF\"}", 3 * g->intervall);
    metadaten(g, "Pi_Online", "expire", json);
    // Commands an Lüfter und Aktion bestätigt das Gerät selbst
    metadaten(g, "Pi_Luefter", "autoupdate", "{\"value\":\"false\"}");
    metadaten(g, "Pi_Aktion", "autoupdate", "{\"value\":\"false\"}");
    metadaten(
        g, "Pi_Aktion", "stateDescription",
        "{\"value\":\" \",\"config\":{\"options\":"
        "\"ERREICHBARKEIT=openHAB anpingen,DATENTRAEGER=Datenträger,ZEIT=Uhrzeit\"}}");
    melde(g, "Pi_Luefter", ausgang_lesen(g->luefter) ? "ON" : "OFF");
    return ANZAHL;
}

void geraet_melden(geraet* g, const messung* m) {
    char wert[32];
    if (m->temperatur > -273) {
        snprintf(wert, sizeof wert, "%.1f °C", m->temperatur);
        melde(g, "Pi_Temperatur", wert);
    } else {
        melde(g, "Pi_Temperatur", "UNDEF"); // kein Sensor: undefiniert
    }
    snprintf(wert, sizeof wert, "%.0f %%", m->last);
    melde(g, "Pi_Last", wert);
    snprintf(wert, sizeof wert, "%.0f %%", m->speicher);
    melde(g, "Pi_Speicher", wert);
    if (m->netz_kbit >= 0) {
        snprintf(wert, sizeof wert, "%.1f kbit/s", m->netz_kbit);
        melde(g, "Pi_Netz", wert);
    }
    snprintf(wert, sizeof wert, "%.1f h", m->laufzeit_h);
    melde(g, "Pi_Laufzeit", wert);
    melde(g, "Pi_Online", "ON"); // Lebenszeichen
}

// Aktionen: nur diese drei, ohne Shell und ohne Argumente von außen
static void aktion(geraet* g, const char* name, char* antwort, size_t groesse) {
    if (!strcmp(name, "ERREICHBARKEIT")) {
        struct timespec a, b;
        clock_gettime(CLOCK_MONOTONIC, &a);
        cJSON* r = openhab_client_get(g->client, "/", NULL, NULL);
        clock_gettime(CLOCK_MONOTONIC, &b);
        snprintf(antwort, groesse, "openHAB antwortet in %.0f ms",
                 (b.tv_sec - a.tv_sec) * 1e3 + (b.tv_nsec - a.tv_nsec) / 1e6);
        cJSON_Delete(r);
    } else if (!strcmp(name, "DATENTRAEGER")) {
        struct statvfs s;
        if (statvfs("/", &s) == 0)
            snprintf(antwort, groesse, "/ zu %.0f %% belegt, %.1f GiB frei",
                     100.0 * (s.f_blocks - s.f_bavail) / s.f_blocks,
                     (double)s.f_bavail * s.f_frsize / (1 << 30));
        else
            snprintf(antwort, groesse, "Datenträger nicht lesbar");
    } else if (!strcmp(name, "ZEIT")) {
        time_t t = time(NULL);
        strftime(antwort, groesse, "Gerätezeit %H:%M:%S", localtime(&t));
    } else {
        snprintf(antwort, groesse, "unbekannte Aktion: %s", name);
    }
}

int geraet_command(geraet* g, const char* item, const char* befehl, char* antwort,
                   size_t groesse) {
    if (!strcmp(item, "Pi_Luefter")) {
        int an = !strcmp(befehl, "ON");
        if (ausgang_setzen(g->luefter, an) != 0) {
            snprintf(antwort, groesse, "Lüfter: GPIO-Fehler");
            return 1;
        }
        melde(g, "Pi_Luefter", an ? "ON" : "OFF"); // erst jetzt: bestätigen
        snprintf(antwort, groesse, "Lüfter %s (%s)", an ? "an" : "aus",
                 ausgang_name(g->luefter));
        return 1;
    }
    if (!strcmp(item, "Pi_Aktion")) {
        aktion(g, befehl, antwort, groesse);
        melde(g, "Pi_Aktion", befehl);
        melde(g, "Pi_Antwort", antwort);
        return 1;
    }
    return 0;
}

void geraet_abmelden(geraet* g) { melde(g, "Pi_Online", "OFF"); }

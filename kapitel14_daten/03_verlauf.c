// Kapitel 14: Historische Werte abfragen, auswerten und im Terminal zeichnen
#include <openhab/openhab.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define DIENST "inmemory" // Testdaten aus 02_testdaten.c
#define ITEM   "Wohnzimmer_Temperatur"
#define HOEHE  8 // Zeilen des Diagramms

static void iso(time_t t, char* puffer) {
    strftime(puffer, 32, "%Y-%m-%dT%H:%M:%SZ", gmtime(&t));
}

int main(void) {
    const char* url =
        getenv("OPENHAB_URL") ? getenv("OPENHAB_URL") : "http://127.0.0.1:8080";
    openhab_client_t* c = openhab_client_new(url, .token = getenv("OPENHAB_TOKEN"));
    time_t ende = time(NULL), start = ende - 24 * 3600;
    char von[32], bis[32];
    iso(start, von);
    iso(ende, bis);

    cJSON* antwort = openhab_persistence_getItemPersistenceData(
        c, ITEM, .serviceID = DIENST, .startTime = von, .endTime = bis);
    cJSON* daten = openhab_json_get(antwort, "data");
    int n = cJSON_GetArraySize(daten);
    if (n == 0) {
        fprintf(stderr, "keine Werte – zuerst 02_testdaten ausführen\n");
        return 1;
    }
    double* werte = malloc(n * sizeof *werte);
    double min = 1e9, max = -1e9, summe = 0;
    time_t warm = 0;
    for (int i = 0; i < n; i++) {
        cJSON* p = cJSON_GetArrayItem(daten, i);
        werte[i] = atof(openhab_json_str(p, "state")); // "21.44" – ohne Einheit
        summe += werte[i];
        if (werte[i] < min) min = werte[i];
        if (werte[i] > max) {
            max = werte[i];
            warm = (time_t)(openhab_json_num(p, "time", 0) / 1000);
        }
    }
    printf("%.0f Messwerte der letzten 24 Stunden (%s)\n",
           openhab_json_num(antwort, "datapoints", 0), openhab_json_str(antwort, "unit"));
    printf("Minimum %.1f °C   Maximum %.1f °C   Mittel %.1f °C\n", min, max, summe / n);
    char uhr[8];
    strftime(uhr, sizeof uhr, "%H:%M", localtime(&warm));
    printf("am wärmsten um %s Uhr\n\n", uhr);

    // Säulendiagramm: eine Spalte je Stunde, HOEHE Zeilen von oben nach unten
    for (int zeile = HOEHE; zeile >= 1; zeile--) {
        double schwelle = min + (max - min) * (zeile - 0.5) / HOEHE;
        if (zeile == HOEHE)
            printf("%5.1f |", max);
        else if (zeile == 1)
            printf("%5.1f |", min);
        else
            printf("      |");
        for (int i = 0; i < n; i += 4) putchar(werte[i] >= schwelle ? '#' : ' ');
        putchar('\n');
    }
    printf("      +");
    for (int i = 0; i < n; i += 4) putchar(i % 24 == 0 ? '+' : '-');
    printf("\n       vor 24 h%*sjetzt\n", n / 4 - 13, "");
    free(werte);
    cJSON_Delete(antwort);

    // Seitenweise abrufen: 10 Werte pro Seite – sofern der Dienst Seiten unterstützt
    cJSON* seite = openhab_persistence_getItemPersistenceData(
        c, ITEM, .serviceID = DIENST, .startTime = von, .endTime = bis, .page = 0,
        .pageLength = 10);
    printf("\nSeite 0: %s, %s, %s …\n", openhab_json_str(seite, "data.0.state"),
           openhab_json_str(seite, "data.1.state"),
           openhab_json_str(seite, "data.2.state"));
    cJSON_Delete(seite);

    // Mit boundary liefert openHAB zusätzlich den Wert vor und nach dem Zeitraum
    iso(ende - 300, von);
    cJSON* rand = openhab_persistence_getItemPersistenceData(
        c, ITEM, .serviceID = DIENST, .startTime = von, .endTime = bis,
        .boundary = OPENHAB_TRUE);
    printf("letzte 5 Minuten mit Rand: %.0f Wert(e)\n",
           openhab_json_num(rand, "datapoints", 0));
    cJSON_Delete(rand);
    openhab_client_destroy(c);
    return 0;
}

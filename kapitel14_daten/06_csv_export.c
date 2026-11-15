// Kapitel 14: Den Verlauf eines Items als CSV-Datei exportieren – in Zeitabschnitten
//   ./starte.sh kapitel14_daten/06_csv_export.c [Item] [Stunden] [Dienst]
#include <openhab/openhab.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void iso(time_t t, char* puffer) {
    strftime(puffer, 32, "%Y-%m-%dT%H:%M:%SZ", gmtime(&t));
}

int main(int argc, char** argv) {
    const char* item = argc > 1 ? argv[1] : "Wohnzimmer_Temperatur";
    int stunden = argc > 2 ? atoi(argv[2]) : 48;
    const char* dienst = argc > 3 ? argv[3] : "inmemory";
    const char* url =
        getenv("OPENHAB_URL") ? getenv("OPENHAB_URL") : "http://127.0.0.1:8080";
    openhab_client_t* c = openhab_client_new(url, .token = getenv("OPENHAB_TOKEN"));

    char datei[128];
    snprintf(datei, sizeof datei, "%s.csv", item);
    FILE* csv = fopen(datei, "w"); // FILE* puffert: Zeilen gehen blockweise
    if (!csv) {
        perror(datei);
        return 1;
    }
    fprintf(csv, "zeit;wert;einheit\n");

    // Lange Zeiträume in Abschnitten von 6 Stunden abfragen: kleine Antworten
    const int abschnitt = 6 * 3600;
    time_t ende = time(NULL);
    int zeilen = 0;
    for (time_t von = ende - stunden * 3600; von < ende; von += abschnitt) {
        time_t bis = von + abschnitt < ende ? von + abschnitt : ende;
        char a[32], b[32];
        iso(von, a);
        iso(bis - 1, b); // Grenzen nicht doppelt abfragen
        cJSON* antwort = openhab_persistence_getItemPersistenceData(
            c, item, .serviceID = dienst, .startTime = a, .endTime = b);
        if (openhab_is_error(antwort)) {
            fprintf(stderr, "%s\n", openhab_error(antwort));
            fclose(csv);
            return 1;
        }
        const char* einheit = openhab_json_str(antwort, "unit");
        cJSON* p;
        cJSON_ArrayForEach(p, openhab_json_get(antwort, "data")) {
            time_t t = (time_t)(openhab_json_num(p, "time", 0) / 1000);
            char zeit[32], wert[32];
            iso(t, zeit);
            snprintf(wert, sizeof wert, "%s", openhab_json_str(p, "state"));
            char* punkt = strchr(wert, '.');
            if (punkt) *punkt = ','; // deutsches Dezimalkomma für Tabellen
            fprintf(csv, "%s;%s;%s\n", zeit, wert, einheit ? einheit : "");
            zeilen++;
        }
        cJSON_Delete(antwort);
    }
    if (fclose(csv) != 0) {
        perror(datei);
        return 1;
    }
    printf("%d Werte (%d h, %s) nach %s geschrieben\n", zeilen, stunden, dienst, datei);
    openhab_client_destroy(c);
    return 0;
}

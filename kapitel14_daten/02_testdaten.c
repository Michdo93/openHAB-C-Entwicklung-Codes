// Kapitel 14: Werte mit eigenem Zeitstempel in einen Persistenzdienst schreiben
// 48 Stunden synthetische Temperaturen (alle 15 Minuten) für den Dienst "inmemory" –
// einschließlich eines Ausreißers.
#include <math.h>
#include <openhab/openhab.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define DIENST "inmemory"

static void iso(time_t t, char* puffer) {
    strftime(puffer, 32, "%Y-%m-%dT%H:%M:%SZ", gmtime(&t));
}

// Normalverteilter Zufall nach Box-Muller: Mittelwert 0, Standardabweichung 1
static double gauss(void) {
    double u = (rand() + 1.0) / (RAND_MAX + 2.0), v = (rand() + 1.0) / (RAND_MAX + 2.0);
    return sqrt(-2 * log(u)) * cos(2 * M_PI * v);
}

int main(void) {
    const char* url =
        getenv("OPENHAB_URL") ? getenv("OPENHAB_URL") : "http://127.0.0.1:8080";
    openhab_client_t* c = openhab_client_new(url, .token = getenv("OPENHAB_TOKEN"));
    srand(42); // fester Startwert: jeder Lauf erzeugt dieselben Daten
    time_t jetzt = time(NULL) / 60 * 60;
    const char* items[] = { "Wohnzimmer_Temperatur", "Werkstatt_Temperatur" };
    const double grund[] = { 21.0, 17.0 };
    char von[32], bis[32], zeit[32], zustand[32];

    for (int i = 0; i < 2; i++) {
        // alte Testdaten der letzten drei Tage entfernen
        iso(jetzt - 3 * 86400, von);
        iso(jetzt, bis);
        cJSON_Delete(openhab_persistence_deleteItemData(c, items[i], von, bis, DIENST));
        int geschrieben = 0;
        for (int k = 48 * 4; k > 0; k--) {
            time_t t = jetzt - 15 * 60 * k;
            struct tm* lokal = localtime(&t);
            double stunde = lokal->tm_hour + lokal->tm_min / 60.0;
            double wert =
                grund[i] + 1.5 * sin((stunde - 9) / 24 * 2 * M_PI) + 0.15 * gauss();
            if (i == 1 && k == 30) wert += 6.0; // ein Ausreißer
            iso(t, zeit);
            snprintf(zustand, sizeof zustand, "%.2f °C", wert);
            cJSON* r = openhab_persistence_storeItemData(c, items[i], zeit, zustand,
                                                         .serviceID = DIENST);
            if (openhab_is_error(r)) {
                fprintf(stderr, "%s: %s\n", items[i], openhab_error(r));
                return 1;
            }
            cJSON_Delete(r);
            geschrieben++;
        }
        printf("%s: %d Werte in »%s« geschrieben\n", items[i], geschrieben, DIENST);
    }

    // Gegenprobe: rrd4j nimmt keine Werte von außen an
    iso(jetzt, zeit);
    cJSON* r =
        openhab_persistence_storeItemData(c, items[0], zeit, "20 °C", .serviceID = "rrd4j");
    const char* text = openhab_error(r);
    printf("Gegenprobe rrd4j: %s\n", text ? strrchr(text, '(') : "angenommen?");
    cJSON_Delete(r);
    openhab_client_destroy(c);
    return 0;
}

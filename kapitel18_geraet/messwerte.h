// Kapitel 18: Messwerte des Rechners aus /proc und /sys.
// Die Zerlege-Funktionen arbeiten auf Text – so lassen sie sich ohne Rechner testen.
#ifndef MESSWERTE_H
#define MESSWERTE_H

typedef struct {
    double temperatur; // °C, < -273 = unbekannt
    double last;       // Last der letzten Minute je Kern in %
    double speicher;   // belegter Arbeitsspeicher in %
    double netz_kbit;  // Netzverkehr seit der letzten Messung, < 0 = unbekannt
    double laufzeit_h; // Laufzeit des Rechners in Stunden
} messung;

double messwerte_temperatur_aus(const char* text);         // "48312\n" -> 48.312
double messwerte_last_aus(const char* loadavg, int kerne); // "0.52 ..." -> 13 (%)
double messwerte_speicher_aus(const char* meminfo);        // MemTotal/MemAvailable
long long messwerte_netzbytes_aus(const char* netdev);     // Summe rx+tx ohne lo

// Liest alles auf einmal; vorher_bytes/vorher_zeit merken sich die letzte Netzmessung
messung messwerte_lesen(long long* vorher_bytes, double* vorher_zeit);

#endif

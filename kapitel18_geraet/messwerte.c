// Kapitel 18: Messwerte des Rechners
#define _POSIX_C_SOURCE 200809L
#include "messwerte.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

double messwerte_temperatur_aus(const char* text) {
    char* ende;
    long milli = strtol(text ? text : "", &ende, 10);
    return ende == text || !text ? -999 : milli / 1000.0;
}

double messwerte_last_aus(const char* loadavg, int kerne) {
    double last;
    if (!loadavg || sscanf(loadavg, "%lf", &last) != 1 || kerne < 1) return -1;
    return last / kerne * 100;
}

double messwerte_speicher_aus(const char* meminfo) {
    const char* g = meminfo ? strstr(meminfo, "MemTotal:") : NULL;
    const char* f = meminfo ? strstr(meminfo, "MemAvailable:") : NULL;
    long gesamt, frei;
    if (!g || !f || sscanf(g, "MemTotal: %ld", &gesamt) != 1 ||
        sscanf(f, "MemAvailable: %ld", &frei) != 1 || gesamt <= 0)
        return -1;
    return 100.0 * (gesamt - frei) / gesamt;
}

long long messwerte_netzbytes_aus(const char* netdev) {
    if (!netdev) return -1;
    long long summe = 0;
    const char* zeile = strchr(netdev, '\n'); // zwei Kopfzeilen überspringen
    zeile = zeile ? strchr(zeile + 1, '\n') : NULL;
    while (zeile && *++zeile) {
        char name[32];
        long long rx, tx, d;
        // "  eth0: rx_bytes packets errs drop fifo frame compressed multicast tx_bytes …"
        if (sscanf(zeile, " %31[^:]: %lld %lld %lld %lld %lld %lld %lld %lld %lld", name,
                   &rx, &d, &d, &d, &d, &d, &d, &d, &tx) == 10 &&
            strcmp(name, "lo"))
            summe += rx + tx;
        zeile = strchr(zeile, '\n');
    }
    return summe;
}

// Eine ganze (kleine) Datei als Zeichenkette lesen; Aufrufer gibt frei
static char* datei(const char* pfad) {
    FILE* f = fopen(pfad, "r");
    if (!f) return NULL;
    size_t groesse = 0, kapazitaet = 4096;
    char* text = malloc(kapazitaet);
    size_t n;
    while (text && (n = fread(text + groesse, 1, kapazitaet - groesse - 1, f)) > 0) {
        groesse += n;
        if (kapazitaet - groesse < 2) {
            char* neu = realloc(text, kapazitaet *= 2);
            if (!neu) {
                free(text);
                text = NULL;
            }
            text = neu;
        }
    }
    fclose(f);
    if (text) text[groesse] = '\0';
    return text;
}

messung messwerte_lesen(long long* vorher_bytes, double* vorher_zeit) {
    messung m;
    char* t = datei("/sys/class/thermal/thermal_zone0/temp");
    char* l = datei("/proc/loadavg");
    char* s = datei("/proc/meminfo");
    char* n = datei("/proc/net/dev");
    char* u = datei("/proc/uptime");
    m.temperatur = messwerte_temperatur_aus(t);
    m.last = messwerte_last_aus(l, (int)sysconf(_SC_NPROCESSORS_ONLN));
    m.speicher = messwerte_speicher_aus(s);
    m.laufzeit_h = u ? atof(u) / 3600 : -1;

    struct timespec jetzt;
    clock_gettime(CLOCK_MONOTONIC, &jetzt);
    double zeit = jetzt.tv_sec + jetzt.tv_nsec / 1e9;
    long long bytes = messwerte_netzbytes_aus(n);
    m.netz_kbit = -1;
    if (bytes >= 0 && *vorher_bytes >= 0 && zeit > *vorher_zeit)
        m.netz_kbit = (bytes - *vorher_bytes) * 8 / 1000.0 / (zeit - *vorher_zeit);
    *vorher_bytes = bytes;
    *vorher_zeit = zeit;
    free(t);
    free(l);
    free(s);
    free(n);
    free(u);
    return m;
}

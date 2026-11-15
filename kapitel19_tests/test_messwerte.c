// Kapitel 19: Unit-Test der Messwerte (Kapitel 18) – nur Text, kein Rechner, kein openHAB
#include "messwerte.h"
#include "testhilfe.h"

#include <math.h>

static int nahe(double a, double b) { return fabs(a - b) < 0.01; }

int main(void) {
    PRUEFE(nahe(messwerte_temperatur_aus("48312\n"), 48.312));
    PRUEFE(messwerte_temperatur_aus("") < -273);   // leer: unbekannt
    PRUEFE(messwerte_temperatur_aus(NULL) < -273); // Datei fehlt
    PRUEFE(nahe(messwerte_last_aus("0.52 0.48 0.40 2/931 12345\n", 4), 13.0));
    PRUEFE(messwerte_last_aus("kaputt", 4) < 0);

    const char* meminfo =
        "MemTotal:        8000000 kB\nMemFree:  100 kB\n"
        "MemAvailable:    6000000 kB\n";
    PRUEFE(nahe(messwerte_speicher_aus(meminfo), 25.0));
    PRUEFE(messwerte_speicher_aus("MemTotal: 0 kB\n") < 0);

    const char* netdev =
        "Inter-|   Receive                                                |  Transmit\n"
        " face |bytes    packets errs drop fifo frame compressed multicast|bytes    "
        "packets\n"
        "    lo: 5000      10    0    0    0     0          0         0     5000      10 0 "
        "0 0 0 0 0\n"
        "  eth0: 1000      20    0    0    0     0          0         0      300       5 0 "
        "0 0 0 0 0\n"
        " wlan0:  200       2    0    0    0     0          0         0       50       1 0 "
        "0 0 0 0 0\n";
    PRUEFE(messwerte_netzbytes_aus(netdev) == 1550); // ohne lo
    PRUEFE(messwerte_netzbytes_aus(NULL) == -1);
    return test_ende("test_messwerte");
}

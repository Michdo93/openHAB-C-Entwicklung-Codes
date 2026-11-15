// Kapitel 8: Funktionen, Prototypen und Rückgabewerte
#include <stdio.h>

// Prototyp: Der Compiler kennt die Funktion, bevor sie definiert ist
static double celsius_aus_fahrenheit(double f);

// static: nur in dieser Datei sichtbar – der Normalfall für Hilfsfunktionen
static int begrenze(int wert, int min, int max) {
    if (wert < min) return min;
    if (wert > max) return max;
    return wert;
}

// Rückgabewert 0 = Erfolg, alles andere = Fehler: die übliche Konvention in C
static int dimmer_pruefen(int prozent) {
    if (prozent < 0 || prozent > 100) {
        fprintf(stderr, "Dimmer: %d liegt nicht zwischen 0 und 100\n", prozent);
        return -1;
    }
    return 0;
}

// Ein zählender Aufruf: static in einer Funktion behält den Wert zwischen den Aufrufen
static int naechste_nummer(void) {
    static int zaehler = 0;
    return ++zaehler;
}

int main(void) {
    printf("71.6 °F = %.1f °C\n", celsius_aus_fahrenheit(71.6));
    printf("begrenze(130) = %d\n", begrenze(130, 0, 100));
    printf("dimmer_pruefen(60)  = %d\n", dimmer_pruefen(60));
    printf("dimmer_pruefen(120) = %d\n", dimmer_pruefen(120));
    for (int i = 0; i < 3; i++) printf("Anfrage Nr. %d\n", naechste_nummer());
    return 0;
}

static double celsius_aus_fahrenheit(double f) { return (f - 32.0) * 5.0 / 9.0; }

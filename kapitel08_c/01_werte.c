// Kapitel 8: Werte, Typen und Ausgabe
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

int main(void) {
    int helligkeit = 60;                 // ganze Zahl, meist 32 Bit
    double temperatur = 21.5;            // Gleitkommazahl, 64 Bit
    char einheit[] = "°C";               // Zeichenkette: Zeichen-Array mit '\0' am Ende
    bool licht_an = true;                // seit C99 mit <stdbool.h>
    uint8_t rot = 255;                   // genau 8 Bit ohne Vorzeichen
    int64_t zeitstempel = 1794781577298; // Millisekunden seit 1970

    printf("Helligkeit:  %d %%\n", helligkeit);
    printf("Temperatur:  %.1f %s\n", temperatur, einheit);
    printf("Licht:       %s\n", licht_an ? "ON" : "OFF");
    printf("Rot:         %u\n", rot);
    printf("Zeitstempel: %" PRId64 " ms\n", zeitstempel);

    // sizeof: Größe in Byte – abhängig von Plattform und Compiler
    printf("Größen:      int %zu, long %zu, double %zu, Zeiger %zu, \"°C\" %zu\n",
           sizeof(int), sizeof(long), sizeof(double), sizeof(void*), sizeof einheit);

    // Ganzzahlige Division schneidet ab, erst ein double rechnet genau
    int summe = 21 + 22 + 24;
    printf("Mittelwert:  %d (int) / %.2f (double)\n", summe / 3, summe / 3.0);

    // Überlauf ohne Vorzeichen ist definiert: Es geht bei 0 weiter
    rot = rot + 1;
    printf("Rot + 1:     %u\n", rot);
    return 0; // 0 = Erfolg für Shell und systemd
}

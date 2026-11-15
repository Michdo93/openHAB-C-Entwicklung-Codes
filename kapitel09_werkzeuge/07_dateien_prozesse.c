// Kapitel 9: Dateien und Prozesse – Messwerte aus /proc, eine Datei schreiben, popen
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    // /proc/loadavg: "0.52 0.48 0.40 2/931 12345" – die Systemlast der letzten Minute
    double last = 0;
    FILE* f = fopen("/proc/loadavg", "r");
    if (f) {
        if (fscanf(f, "%lf", &last) != 1) last = -1;
        fclose(f);
    }
    printf("Systemlast:   %.2f\n", last);

    // /proc/meminfo zeilenweise lesen und zwei Werte herausziehen
    long gesamt = 0, frei = 0;
    char zeile[256];
    if ((f = fopen("/proc/meminfo", "r"))) {
        while (fgets(zeile, sizeof zeile, f)) {
            sscanf(zeile, "MemTotal: %ld kB", &gesamt);
            sscanf(zeile, "MemAvailable: %ld kB", &frei);
        }
        fclose(f);
    }
    printf("Speicher:     %ld von %ld MiB frei\n", frei / 1024, gesamt / 1024);

    // Eine Datei schreiben – und mit dem Rückgabewert von fclose prüfen, ob alles ankam
    f = fopen("messwerte.csv", "w");
    if (!f) {
        perror("messwerte.csv");
        return 1;
    }
    fprintf(f, "messwert;wert\nlast;%.2f\nfrei_mib;%ld\n", last, frei / 1024);
    if (fclose(f) != 0) {
        perror("fclose");
        return 1;
    }
    puts("Datei:        messwerte.csv geschrieben");

    // popen startet ein Programm und liest seine Ausgabe
    FILE* p = popen("uname -sr", "r");
    if (p && fgets(zeile, sizeof zeile, p)) {
        zeile[strcspn(zeile, "\n")] = '\0';
        printf("Kernel:       %s\n", zeile);
    }
    if (p) pclose(p);
    remove("messwerte.csv");
    return 0;
}

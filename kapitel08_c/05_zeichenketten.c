// Kapitel 8: Zeichenketten – vergleichen, zusammensetzen, zerlegen
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    // snprintf schreibt nie über das Ende des Puffers hinaus
    char pfad[64];
    const char* item = "Wohnzimmer_Licht";
    int laenge = snprintf(pfad, sizeof pfad, "/rest/items/%s/state", item);
    printf("%s (%d Zeichen, %zu mit strlen)\n", pfad, laenge, strlen(pfad));

    // Zu kleiner Puffer: snprintf kürzt und meldet die benötigte Länge
    char kurz[16];
    if (snprintf(kurz, sizeof kurz, "/rest/items/%s", item) >= (int)sizeof kurz)
        printf("gekürzt: \"%s\"\n", kurz);

    // Eine HSB-Farbe "240,100,80" zerlegen – strtol meldet, wo die Zahl endet
    const char* farbe = "240,100,80";
    const char* p = farbe;
    long hsb[3];
    for (int i = 0; i < 3; i++) {
        char* ende;
        hsb[i] = strtol(p, &ende, 10);
        p = ende + (*ende == ','); // Komma überspringen
    }
    printf("Farbton %ld, Sättigung %ld, Helligkeit %ld\n", hsb[0], hsb[1], hsb[2]);

    // Zahl mit Einheit: "21.5 °C" – strtod liest die Zahl, der Rest ist die Einheit
    char* einheit;
    double wert = strtod("21.5 °C", &einheit);
    printf("Wert %.1f, Einheit \"%s\"\n", wert, einheit + 1);

    // Suchen: strchr findet ein Zeichen, strstr eine Teilzeichenkette
    const char* uid = "astro:sun:zuhause:rise#start";
    printf("Kanal-Gruppe: %.*s\n", (int)(strchr(uid, '#') - uid), uid);
    printf("Thing-Typ beginnt bei: %s\n", strstr(uid, "sun"));

    // strncmp: nur die ersten n Zeichen vergleichen
    printf("Wohnzimmer-Item? %s\n", strncmp(item, "Wohnzimmer_", 11) == 0 ? "ja" : "nein");
    return 0;
}

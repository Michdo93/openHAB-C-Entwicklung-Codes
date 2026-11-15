// Kapitel 8: Kontrollfluss – if, switch, Schleifen
#include <stdio.h>
#include <string.h>

int main(void) {
    const char* zustaende[] = { "ON", "OFF", "NULL", "UNDEF", "42" };
    int anzahl = sizeof zustaende / sizeof zustaende[0];

    for (int i = 0; i < anzahl; i++) {
        const char* s = zustaende[i];
        // Zeichenketten vergleicht strcmp – == vergleicht nur Adressen
        if (strcmp(s, "ON") == 0) {
            printf("%-6s eingeschaltet\n", s);
        } else if (strcmp(s, "NULL") == 0 || strcmp(s, "UNDEF") == 0) {
            printf("%-6s kein gültiger Wert\n", s);
        } else {
            printf("%-6s sonstiger Zustand\n", s);
        }
    }

    // switch arbeitet mit ganzen Zahlen und Zeichen – hier mit dem ersten Buchstaben
    const char* befehl = "DOWN";
    switch (befehl[0]) {
    case 'U':
        puts("Rollladen fährt hoch");
        break;
    case 'D':
        puts("Rollladen fährt runter");
        break;
    case 'S':
        puts("Rollladen stoppt");
        break;
    default:
        puts("unbekannter Befehl");
    }

    // while: so lange, bis die Bedingung nicht mehr gilt
    int position = 0;
    while (position < 100) position += 30;
    printf("Position nach while: %d\n", position);

    // do-while läuft mindestens einmal, break und continue steuern Schleifen
    int versuch = 0;
    do {
        versuch++;
        if (versuch == 2) continue; // Versuch 2 überspringen
        printf("Versuch %d\n", versuch);
    } while (versuch < 3);
    return 0;
}

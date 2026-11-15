// Kapitel 8: Zeiger und Arrays
#include <stdio.h>

// Ausgabeparameter: Die Funktion schreibt über Zeiger in Variablen des Aufrufers
static void min_max(const double* werte, int n, double* min, double* max) {
    *min = *max = werte[0];
    for (int i = 1; i < n; i++) {
        if (werte[i] < *min) *min = werte[i];
        if (werte[i] > *max) *max = werte[i];
    }
}

int main(void) {
    int helligkeit = 60;
    int* zeiger = &helligkeit; // & liefert die Adresse
    *zeiger = 80;              // * greift auf den Wert zu
    printf("helligkeit = %d, über Zeiger: %d\n", helligkeit, *zeiger);

    double temperaturen[] = { 21.5, 22.0, 19.8, 23.1, 20.4 };
    int n = sizeof temperaturen / sizeof temperaturen[0];
    double min, max;
    min_max(temperaturen, n, &min,
            &max); // ein Array zerfällt zum Zeiger aufs erste Element
    printf("%d Werte, min %.1f, max %.1f\n", n, min, max);

    // Zeigerarithmetik: p + 1 zeigt auf das nächste Element, nicht auf das nächste Byte
    double* p = temperaturen;
    printf("p[2] = %.1f, *(p + 2) = %.1f\n", p[2], *(p + 2));

    // NULL: Ein Zeiger, der auf nichts zeigt – vor jedem Zugriff prüfen
    const char* label = NULL;
    printf("Label: %s\n", label ? label : "(keins)");

    // Ein Array von Zeichenketten ist ein Array von Zeigern
    const char* items[] = { "Wohnzimmer_Licht", "Wohnzimmer_Dimmer", NULL };
    for (const char** it = items; *it; it++) printf("Item: %s\n", *it);
    return 0;
}

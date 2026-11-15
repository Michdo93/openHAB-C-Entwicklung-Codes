// Kapitel 20: ohplugin – openHAB als native Bibliothek für andere Sprachen und Engines
// (Unity, HoloLens, Python, C#, Go …). Nur einfache Typen an der Schnittstelle:
// Zeichenketten, Zahlen, ein undurchsichtiger Zeiger. Kein cJSON, kein struct.
#ifndef OHPLUGIN_H
#define OHPLUGIN_H

#if defined(_WIN32)
#define OHPLUGIN_API __declspec(dllexport)
#else
#define OHPLUGIN_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ohplugin ohplugin;

OHPLUGIN_API ohplugin* oh_verbinden(const char* url, const char* token); // NULL = Fehler
OHPLUGIN_API int oh_befehl(ohplugin* p, const char* item, const char* command); // 0 = ok
OHPLUGIN_API int oh_melden(ohplugin* p, const char* item, const char* state);   // 0 = ok
// Zustand in den Puffer des Aufrufers kopieren; Rückgabe: Länge oder -1
OHPLUGIN_API int oh_zustand(ohplugin* p, const char* item, char* puffer, int groesse);
// Änderungen beobachten: ein Hintergrund-Thread sammelt sie, der Aufrufer holt sie ab –
// z. B. einmal je Frame in Update(). "Item=State"; 0 = nichts da
OHPLUGIN_API int oh_beobachten(ohplugin* p, const char* items_komma);
OHPLUGIN_API int oh_naechste_aenderung(ohplugin* p, char* puffer, int groesse);
OHPLUGIN_API const char* oh_fehler(ohplugin* p); // letzte Fehlermeldung oder ""
OHPLUGIN_API void oh_trennen(ohplugin* p);

#ifdef __cplusplus
}
#endif
#endif

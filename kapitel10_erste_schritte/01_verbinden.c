// Kapitel 10: Verbindung herstellen und Systeminformationen lesen
#include <openhab/openhab.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    const char* url =
        getenv("OPENHAB_URL") ? getenv("OPENHAB_URL") : "http://127.0.0.1:8080";
    openhab_client_t* c = openhab_client_new(url, .token = getenv("OPENHAB_TOKEN"));
    if (!openhab_client_is_logged_in(c)) {
        fprintf(stderr, "keine Verbindung: %s\n", openhab_client_last_error(c));
        openhab_client_destroy(c);
        return 1; // für Skripte und systemd
    }

    cJSON* wurzel = openhab_client_get(c, "/", NULL, NULL); // /rest/ – Version, Endpunkte
    cJSON* info = openhab_systeminfo_getSystemInfo(c);
    cJSON* uuid = openhab_uuid_getUUID(c);

    printf("openHAB %s unter %s\n", openhab_json_str(wurzel, "runtimeInfo.version"),
           openhab_client_base_url(c));
    printf("  Sprache/Land:   %s, Maßsystem %s\n", openhab_json_str(wurzel, "locale"),
           openhab_json_str(wurzel, "measurementSystem"));
    printf("  Endpunkte:      %d\n", cJSON_GetArraySize(openhab_json_get(wurzel, "links")));
    printf("  Betriebssystem: %s %s (%s)\n", openhab_json_str(info, "systemInfo.osName"),
           openhab_json_str(info, "systemInfo.osVersion"),
           openhab_json_str(info, "systemInfo.osArchitecture"));
    printf("  Java (openHAB): %s von %s\n",
           openhab_json_str(info, "systemInfo.javaVersion"),
           openhab_json_str(info, "systemInfo.javaVendor"));
    printf("  Konfiguration:  %s\n", openhab_json_str(info, "systemInfo.configFolder"));
    printf("  Speicher:       %.0f von %.0f MiB frei\n",
           openhab_json_num(info, "systemInfo.freeMemory", 0) / 1048576,
           openhab_json_num(info, "systemInfo.totalMemory", 0) / 1048576);
    printf("  Instanz-UUID:   %s\n", uuid->valuestring); // Text als cJSON-String
    printf("  Bibliothek:     c-openhab-rest-client %s\n", OPENHAB_CLIENT_VERSION);

    cJSON_Delete(wurzel); // jedes Ergebnis gehört dem Aufrufer
    cJSON_Delete(info);
    cJSON_Delete(uuid);
    openhab_client_destroy(c);
    return 0;
}

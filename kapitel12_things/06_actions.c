// Kapitel 12: Thing-Actions – das Astro-Binding rechnet die Sonnenhöhe aus
#include <openhab/openhab.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SONNE "astro:sun:zuhause"

static char hoehe_uid[128]; // die Action-UID enthält einen Hash hinter '#'

static double hoehe(openhab_client_t* c, time_t zeit) {
    char iso[32], eingaben[64];
    strftime(iso, sizeof iso, "%Y-%m-%dT%H:%M:%SZ", gmtime(&zeit));
    snprintf(eingaben, sizeof eingaben, "{\"date\":\"%s\"}", iso);
    cJSON* in = cJSON_Parse(eingaben);
    cJSON* antwort = openhab_actions_executeAction(c, SONNE, hoehe_uid, in);
    const char* text = openhab_json_str(antwort, "result"); // z. B. "-56.97 °"
    double grad = text ? atof(text) : 0;
    cJSON_Delete(antwort);
    cJSON_Delete(in);
    return grad;
}

static void zeile(openhab_client_t* c, time_t zeit) {
    char wann[32];
    strftime(wann, sizeof wann, "%d.%m., %H:%M", localtime(&zeit));
    printf("  %s  %6.1f°\n", wann, hoehe(c, zeit));
}

int main(void) {
    const char* url =
        getenv("OPENHAB_URL") ? getenv("OPENHAB_URL") : "http://127.0.0.1:8080";
    openhab_client_t* c = openhab_client_new(url, .token = getenv("OPENHAB_TOKEN"));

    cJSON* actions = openhab_actions_getActions(c, SONNE, .language = "de");
    cJSON *a, *e;
    cJSON_ArrayForEach(a, actions) {
        const char* uid = openhab_json_str(a, "actionUid");
        int laenge = (int)strcspn(uid, "#"); // Name = Teil vor dem '#'
        if (!strncmp(uid, "astro.getElevation", laenge))
            snprintf(hoehe_uid, sizeof hoehe_uid, "%s", uid);
        printf("  %-26.*s (", laenge, uid);
        int erste = 1;
        cJSON_ArrayForEach(e, openhab_json_get(a, "inputs")) {
            printf("%s%s", erste ? "" : ", ", openhab_json_str(e, "name"));
            erste = 0;
        }
        printf(") – %s\n", openhab_json_str(a, "description"));
    }
    cJSON_Delete(actions);

    printf("\nSonnenhöhe über den Tag (jetzt und die nächsten Stunden):\n");
    time_t jetzt = time(NULL) / 3600 * 3600; // auf die volle Stunde
    for (int stunden = 0; stunden < 24; stunden += 3) zeile(c, jetzt + stunden * 3600);
    struct tm t = *localtime(&jetzt);
    for (int monat = 3; monat <= 12; monat += 3) {
        t.tm_mon = monat - 1;
        t.tm_mday = 21;
        t.tm_hour = 12;
        t.tm_isdst = -1; // Sommerzeit selbst bestimmen
        zeile(c, mktime(&t));
    }
    openhab_client_destroy(c);
    return 0;
}

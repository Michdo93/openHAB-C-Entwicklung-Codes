// Kapitel 13: Zeitplan-Trigger, ein Rules-DSL-Skript und die Vorschau des Zeitplans
#define _XOPEN_SOURCE 700 // für strptime
#include <openhab/openhab.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define UID "buch_lueften"

// Rules-DSL-Skript als Aktion: läuft werktags um 7:30 und 17:30
static const char* SKRIPT =
    "val feuchte = Werkstatt_Feuchte.state\n"
    "if (feuchte instanceof QuantityType\n"
    "        && (feuchte as QuantityType<?>).toUnit(\"%\").doubleValue > 60) {\n"
    "    Werkstatt_Luefter.sendCommand(ON)\n"
    "    Haus_Status.postUpdate(\"Werkstatt wird gelüftet\")\n"
    "}\n";

static cJSON* cron(const char* id, const char* ausdruck) {
    cJSON* t = cJSON_CreateObject();
    cJSON_AddStringToObject(t, "id", id);
    cJSON_AddStringToObject(t, "type", "timer.GenericCronTrigger");
    cJSON_AddStringToObject(cJSON_AddObjectToObject(t, "configuration"), "cronExpression",
                            ausdruck);
    return t;
}

static void zeige(const char* was, cJSON* r) { // Texte ohne, JSON mit Anführungszeichen
    char* text = cJSON_IsString(r) ? NULL : openhab_json_print(r);
    printf("%s%s\n", was, text ? text : r->valuestring);
    openhab_free(text);
    cJSON_Delete(r);
}

int main(void) {
    const char* url =
        getenv("OPENHAB_URL") ? getenv("OPENHAB_URL") : "http://127.0.0.1:8080";
    openhab_client_t* c = openhab_client_new(url, .token = getenv("OPENHAB_TOKEN"));

    // Die Regel mit den cJSON-Bausteinen zusammensetzen: das Skript enthält
    // Anführungszeichen
    cJSON* regel = cJSON_CreateObject();
    cJSON_AddStringToObject(regel, "uid", UID);
    cJSON_AddStringToObject(regel, "name", "Werkstatt lüften");
    cJSON* tags = cJSON_AddArrayToObject(regel, "tags");
    cJSON_AddItemToArray(tags, cJSON_CreateString("Buch"));
    cJSON_AddItemToArray(tags, cJSON_CreateString("Schedule"));
    cJSON* trigger = cJSON_AddArrayToObject(regel, "triggers");
    cJSON_AddItemToArray(trigger, cron("morgens", "0 30 7 ? * MON-FRI"));
    cJSON_AddItemToArray(trigger, cron("abends", "0 30 17 ? * MON-FRI"));
    cJSON* aktion = cJSON_CreateObject();
    cJSON_AddStringToObject(aktion, "id", "skript");
    cJSON_AddStringToObject(aktion, "type", "script.ScriptAction");
    cJSON* cfg = cJSON_AddObjectToObject(aktion, "configuration");
    cJSON_AddStringToObject(cfg, "type", "application/vnd.openhab.dsl.rule");
    cJSON_AddStringToObject(cfg, "script", SKRIPT);
    cJSON_AddItemToArray(cJSON_AddArrayToObject(regel, "actions"), aktion);
    cJSON_Delete(openhab_rules_deleteRule(c, UID));
    zeige("", openhab_rules_createRule(c, regel));
    cJSON_Delete(regel);

    // Wann läuft was? openHAB simuliert alle Regeln mit dem Tag "Schedule"
    char von[32], bis[32];
    time_t morgen = time(NULL) + 86400;
    struct tm t = *localtime(&morgen);
    t.tm_hour = t.tm_min = t.tm_sec = 0;
    time_t start = mktime(&t);
    time_t ende = start + 7 * 86400;
    strftime(von, sizeof von, "%Y-%m-%dT%H:%M:%SZ", gmtime(&start));
    strftime(bis, sizeof bis, "%Y-%m-%dT%H:%M:%SZ", gmtime(&ende));
    cJSON* plan = openhab_rules_simulateSchedule(c, von, bis);
    cJSON* e;
    cJSON_ArrayForEach(e, plan) {
        if (strcmp(openhab_json_str(e, "rule.uid"), UID)) continue;
        // openHAB liefert z. B. "Nov 16, 2026, 7:30:00 AM" (mit schmalem Leerzeichen)
        char datum[64], *z = datum;
        for (const char* q = openhab_json_str(e, "date"); *q && z < datum + 63; q++)
            if ((unsigned char)*q < 0x80)
                *z++ = *q; // nur ASCII behalten
            else if ((unsigned char)*q == 0xE2)
                *z++ = ' '; // U+202F -> Leerzeichen
        *z = '\0';
        struct tm w = { 0 };
        strptime(datum, "%b %d, %Y, %I:%M:%S %p", &w);
        char aus[32];
        strftime(aus, sizeof aus, "%a %d.%m., %H:%M", &w);
        printf("  %s  %s\n", aus, openhab_json_str(e, "rule.name"));
    }
    cJSON_Delete(plan);

    // Nicht bis morgen warten: Werkstatt feucht machen und die Regel sofort ausführen
    cJSON_Delete(openhab_items_postUpdate(c, "Werkstatt_Feuchte", "72 %"));
    cJSON_Delete(openhab_rules_runNow(c, UID));
    sleep(2); // DSL-Skripte werden beim ersten Lauf übersetzt
    zeige("Status: ", openhab_items_getItemState(c, "Haus_Status"));
    cJSON* l = openhab_items_getItemState(c, "Werkstatt_Luefter");
    printf("Lüfter: %s (autoupdate=false: erst das Gerät bestätigt den Command)\n",
           l->valuestring);
    cJSON_Delete(l);
    zeige("löschen: ", openhab_rules_deleteRule(c, UID));
    openhab_client_destroy(c);
    return 0;
}

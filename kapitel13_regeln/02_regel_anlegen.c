// Kapitel 13: Eine Regel anlegen – Trigger, Bedingung, Aktionen
#include <openhab/openhab.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define UID "buch_kinoabend"

// JSON mit einfachen Anführungszeichen schreiben – lesbarer als lauter \"
static cJSON* json(const char* text) {
    char* kopie = strdup(text);
    for (char* p = kopie; *p; p++)
        if (*p == '\'') *p = '"';
    cJSON* j = cJSON_Parse(kopie);
    free(kopie);
    return j;
}

static void zustand(openhab_client_t* c, const char* was) {
    const char* namen[] = { "Wohnzimmer_Licht", "Wohnzimmer_Dimmer", "Kueche_Rollladen" };
    printf("%s", was); // Umlaute: Breite von Hand, printf zählt Bytes
    for (int i = 0; i < 3; i++) {
        cJSON* s = openhab_items_getItemState(c, namen[i]);
        printf("%s%s=%s", i ? ", " : "", namen[i], s->valuestring);
        cJSON_Delete(s);
    }
    printf("\n");
}

static void befehl(openhab_client_t* c, const char* item, const char* cmd) {
    cJSON_Delete(openhab_items_sendCommand(c, item, cmd));
}

static void update(openhab_client_t* c, const char* item, const char* state) {
    cJSON_Delete(openhab_items_postUpdate(c, item, state));
}

int main(void) {
    const char* url =
        getenv("OPENHAB_URL") ? getenv("OPENHAB_URL") : "http://127.0.0.1:8080";
    openhab_client_t* c = openhab_client_new(url, .token = getenv("OPENHAB_TOKEN"));

    cJSON* regel = json(
        "{'uid':'" UID
        "','name':'Kinoabend','tags':['Buch','Szene'],"
        " 'description':'Licht dimmen und Rollladen schließen, wenn die Haustür zu ist.',"
        " 'triggers':[{'id':'1','type':'core.ItemCommandTrigger',"
        "   'configuration':{'itemName':'Haus_Status','command':'Kino'}}],"
        " 'conditions':[{'id':'2','type':'core.ItemStateCondition',"
        "   'configuration':{'itemName':'Flur_Haustuer','operator':'=','state':'CLOSED'}}],"
        " 'actions':["
        "  {'id':'3','type':'core.ItemCommandAction',"
        "   'configuration':{'itemName':'Wohnzimmer_Licht','command':'OFF'}},"
        "  {'id':'4','type':'core.ItemCommandAction',"
        "   'configuration':{'itemName':'Wohnzimmer_Dimmer','command':'20'}},"
        "  {'id':'5','type':'core.ItemCommandAction',"
        "   'configuration':{'itemName':'Kueche_Rollladen','command':'DOWN'}}]}");
    cJSON_Delete(openhab_rules_deleteRule(c, UID)); // falls vom letzten Lauf vorhanden
    cJSON* r = openhab_rules_createRule(c, regel);
    char* text = openhab_json_print(r);
    printf("anlegen: %s\n", text);
    openhab_free(text);
    cJSON_Delete(r);
    cJSON_Delete(regel);
    r = openhab_rules_getRule(c, UID);
    printf("Status:  %s\n", openhab_json_str(r, "status.status"));
    cJSON_Delete(r);

    // Ausgangslage herstellen
    befehl(c, "Wohnzimmer_Licht", "ON");
    befehl(c, "Wohnzimmer_Dimmer", "80");
    befehl(c, "Kueche_Rollladen", "UP");
    // 1. Tür offen: die Bedingung verhindert die Aktionen
    update(c, "Flur_Haustuer", "OPEN");
    befehl(c, "Haus_Status", "Kino");
    sleep(1);
    zustand(c, "Tür offen: ");
    // 2. Tür zu: die Regel greift
    update(c, "Flur_Haustuer", "CLOSED");
    befehl(c, "Haus_Status", "Kino");
    sleep(1);
    zustand(c, "Tür zu:    ");
    openhab_client_destroy(c);
    return 0;
}

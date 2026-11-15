// Kapitel 13: Eine Regel steuern – Parameter ändern, ausführen, abschalten, ersetzen
#include <openhab/openhab.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define UID "buch_kinoabend" // aus 02_regel_anlegen.c

static void zeige(const char* was, cJSON* r) { // Texte ohne, JSON mit Anführungszeichen
    char* text = cJSON_IsString(r) ? NULL : openhab_json_print(r);
    printf("%s%s\n", was, text ? text : r->valuestring);
    openhab_free(text);
    cJSON_Delete(r);
}

static void status(openhab_client_t* c, const char* was) {
    cJSON* r = openhab_rules_getRule(c, UID);
    printf("%s%s / %s\n", was, openhab_json_str(r, "status.status"),
           openhab_json_str(r, "status.statusDetail"));
    cJSON_Delete(r);
}

static void befehl(openhab_client_t* c, const char* item, const char* cmd) {
    cJSON_Delete(openhab_items_sendCommand(c, item, cmd));
}

int main(void) {
    const char* url =
        getenv("OPENHAB_URL") ? getenv("OPENHAB_URL") : "http://127.0.0.1:8080";
    openhab_client_t* c = openhab_client_new(url, .token = getenv("OPENHAB_TOKEN"));
    cJSON* regel = openhab_rules_getRule(c, UID);
    if (openhab_is_error(regel)) {
        fprintf(stderr, "Zuerst 02_regel_anlegen ausführen.\n");
        return 1;
    }
    cJSON_Delete(regel);

    // 1. Einen einzelnen Parameter eines Moduls ändern: Dimmer auf 10 statt 20 %
    cJSON_Delete(
        openhab_rules_setModuleConfigParam(c, UID, "actions", "4", "command", "10"));
    zeige("Dimmwert jetzt: ",
          openhab_rules_getModuleConfigParam(c, UID, "actions", "4", "command"));
    // 2. Sofort ausführen – runNow überspringt Trigger UND Bedingungen
    befehl(c, "Wohnzimmer_Dimmer", "80");
    cJSON_Delete(openhab_items_postUpdate(c, "Flur_Haustuer", "OPEN"));
    cJSON_Delete(openhab_rules_runNow(c, UID));
    sleep(1);
    zeige("runNow trotz offener Tür: ", openhab_items_getItemState(c, "Wohnzimmer_Dimmer"));

    // 3. Abschalten: der Trigger wird ignoriert
    cJSON_Delete(openhab_rules_disable(c, UID));
    status(c, "abgeschaltet: ");
    cJSON_Delete(openhab_items_postUpdate(c, "Flur_Haustuer", "CLOSED"));
    befehl(c, "Wohnzimmer_Dimmer", "80");
    befehl(c, "Haus_Status", "Kino");
    sleep(1);
    zeige("Trigger bei abgeschalteter Regel: ",
          openhab_items_getItemState(c, "Wohnzimmer_Dimmer"));
    cJSON_Delete(openhab_rules_enable(c, UID));
    status(c, "eingeschaltet: ");

    // 4. Die ganze Regel ersetzen: eine zusätzliche Aktion anhängen
    regel = openhab_rules_getRule(c, UID);
    cJSON* aktion = cJSON_Parse(
        "{\"id\":\"6\",\"type\":\"core.ItemStateUpdateAction\",\"configuration\":"
        "{\"itemName\":\"Haus_Status\",\"state\":\"Film läuft\"}}");
    cJSON_AddItemToArray(cJSON_GetObjectItem(regel, "actions"), aktion); // gehört nun regel
    cJSON_Delete(openhab_rules_updateRule(c, UID, regel));
    cJSON_Delete(regel);
    sleep(1); // openHAB initialisiert die Regel neu
    befehl(c, "Haus_Status", "Kino");
    sleep(1);
    zeige("nach updateRule: ", openhab_items_getItemState(c, "Haus_Status"));
    zeige("löschen: ", openhab_rules_deleteRule(c, UID));
    openhab_client_destroy(c);
    return 0;
}

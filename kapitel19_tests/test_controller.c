// Kapitel 19: Unit-Test des Controllers (Kapitel 17) gegen die Attrappe statt openHAB
#include <openhab/openhab.h>

#include "attrappe.h"
#include "controller.h"
#include "testhilfe.h"

static const char* GRUPPE =
    "{\"name\":\"gTest\",\"type\":\"Group\",\"members\":["
    " {\"name\":\"Licht\",\"type\":\"Switch\",\"label\":\"Licht\",\"state\":\"ON\"},"
    " {\"name\":\"Lampe\",\"type\":\"Dimmer\",\"label\":\"Lampe\",\"state\":\"95\"},"
    " {\"name\":\"gUnter\",\"type\":\"Group\",\"members\":["
    "   "
    "{\"name\":\"Rollo\",\"type\":\"Rollershutter\",\"label\":\"Rollo\",\"state\":\"0\"},"
    "   {\"name\":\"Temp\",\"type\":\"Number\",\"label\":\"Temp\",\"state\":\"21\"}]}]}";

int main(void) {
    attrappe* a = attrappe_starten();
    attrappe_antwort(a, "GET", "/rest/items/gTest", 200, GRUPPE);
    attrappe_antwort(a, "GET", "/rest/items/gFehlt", 404,
                     "{\"error\":{\"message\":\"Item gFehlt does not exist!\"}}");
    attrappe_antwort(a, "POST", "/rest/items/", 200, "");
    openhab_client_t* client = openhab_client_new(attrappe_url(a));
    PRUEFE(openhab_client_is_logged_in(client)); // GET /rest/ -> 200 "{}"

    model* m = model_neu();
    controller* c = controller_neu(client, m);
    PRUEFE(controller_laden(c, "gTest") == 4); // Untergruppen aufgelöst
    PRUEFE_TEXT(model_eintrag(m, 2).name, "Rollo");
    PRUEFE(controller_laden(c, "gFehlt") == -1);
    PRUEFE(controller_fehler(c) != NULL);

    controller_umschalten(c, 0); // Switch ON -> OFF
    PRUEFE_TEXT(attrappe_letzte(a), "POST /rest/items/Licht?source=ohdash OFF");
    controller_aendern(c, 1, 10); // 95 + 10 -> höchstens 100
    PRUEFE_TEXT(attrappe_letzte(a), "POST /rest/items/Lampe?source=ohdash 100");
    controller_umschalten(c, 2); // Rollladen 0 -> DOWN
    PRUEFE_TEXT(attrappe_letzte(a), "POST /rest/items/Rollo?source=ohdash DOWN");
    int vorher = attrappe_anfragen(a);
    controller_umschalten(c, 3); // Number: nichts senden
    PRUEFE(attrappe_anfragen(a) == vorher);
    PRUEFE(strstr(controller_fehler(c), "nicht schalten") != NULL);

    // Das Model ändert sich erst durch Ereignisse, nicht durch den Command
    PRUEFE_TEXT(model_eintrag(m, 0).state, "ON");

    controller_stoppen(c);
    model_frei(m);
    openhab_client_destroy(client);
    attrappe_stoppen(a);
    return test_ende("test_controller");
}

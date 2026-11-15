#!/usr/bin/env python3
"""Kapitel 20: ohplugin aus einer anderen Sprache laden – hier Python mit ctypes.
So prüft man eine native Bibliothek, bevor sie in Unity landet.
   python3 kapitel20_projekte/ohplugin_test.py build/kapitel20_projekte/libohplugin.so"""
import ctypes
import os
import sys
import time

lib = ctypes.CDLL(sys.argv[1] if len(sys.argv) > 1 else "./libohplugin.so")
# Die Signaturen aus ohplugin.h – ctypes kennt sie nicht von selbst
P, S, I = ctypes.c_void_p, ctypes.c_char_p, ctypes.c_int
lib.oh_verbinden.restype = P
lib.oh_verbinden.argtypes = [S, S]
lib.oh_befehl.argtypes = [P, S, S]
lib.oh_zustand.argtypes = [P, S, S, I]
lib.oh_beobachten.argtypes = [P, S]
lib.oh_naechste_aenderung.argtypes = [P, S, I]
lib.oh_trennen.argtypes = [P]

oh = lib.oh_verbinden(os.environ.get("OPENHAB_URL", "http://127.0.0.1:8080").encode(),
                      os.environ["OPENHAB_TOKEN"].encode())
puffer = ctypes.create_string_buffer(128)
lib.oh_zustand(oh, b"Wohnzimmer_Licht", puffer, 128)
print("Zustand:", puffer.value.decode())
lib.oh_beobachten(oh, b"Wohnzimmer_Licht,Wohnzimmer_Dimmer")
time.sleep(0.5)
lib.oh_befehl(oh, b"Wohnzimmer_Licht", b"OFF" if puffer.value == b"ON" else b"ON")
lib.oh_befehl(oh, b"Wohnzimmer_Dimmer", b"25")
time.sleep(1)
while lib.oh_naechste_aenderung(oh, puffer, 128) > 0:   # wie Update() in Unity
    print("Änderung:", puffer.value.decode())
lib.oh_trennen(oh)

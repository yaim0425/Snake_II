// ====================================================================================
// SNAKE II — Enlace de dependencias (wiring)
//
// Este archivo construye TODAS las clases y las inicia en setup():
//   - Display y Buttons: hardware (I2C del OLED y pines de los botones).
//   - Boot, Menu, Credits, Game, Legend: ventanas independientes
//     (hermanas, no anidadas), instancias únicas para todo el juego. Sus
//     begin() los llama Engine al entrar en cada estado, y entre transiciones
//     sus valores se conservan.
//   - Engine: despachador puro. Su estado interno decide qué ventana se ve;
//     loop() solo llama engine.update() y engine.print().
//
// La clase Engine está en Engine.h / Engine.cpp.
// ====================================================================================

#include "Config.h"
#include "Display.h"
#include "Buttons.h"
#include "Boot.h"
#include "Menu.h"
#include "Credits.h"
#include "Game.h"
#include "Legend.h"
#include "Buzzer.h"
#include "Sound.h"
#include "Engine.h"

#include <Arduino.h>

// ====================================================================================
// Instancias únicas (construidas antes de setup(), iniciadas en setup())
// ====================================================================================

// Hardware
Display display;

// Entrada
Buttons buttons(Config::Pin::BUTTONS);

// Sonido: Buzzer (hardware, un tono no bloqueante) + Sound
// (secuencias de los efectos del juego). El estado On/Off se
// edita inline en el menú (ver Menu::beginSoundEdit); Sound se
// usa en el menú y en las transiciones del Engine.
Buzzer buzzer;
Sound sound(buzzer);

// Ventanas: clases independientes (hermanas, compartidas por referencia).
// Persisten entre estados: sus valores se conservan.
Boot boot(display, buttons);
Menu menu(display, buttons, sound, 0, "v0.1");
Credits credits(display, buttons, sound);
Game game(display, buttons, sound);
Legend legend(display, buttons, sound);

// Despachador: recibe las ventanas y decide cuál se ve según su estado.
Engine engine(display, buttons, boot, menu, credits, game, legend, sound);

// ====================================================================================

void setup() {
  Serial.begin(115200);

  // Iniciar hardware y entrar al primer estado (menú -> menu.begin())
  display.begin();
  buttons.begin();
  buzzer.begin();
  sound.begin();
  engine.begin();

  Serial.println("Snake II");
}

// ====================================================================================

void loop() {
  buttons.read();     // una sola lectura de botones por frame (de esta lectura
                      // consumen los eventos todas las ventanas despachadas por Engine)
  engine.update();
  engine.print();
  sound.update();
  display.show();
}

// ====================================================================================
// Fin
// ====================================================================================
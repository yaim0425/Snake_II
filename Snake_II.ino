// ====================================================================================
// SNAKE II — Enlace de dependencias (wiring)
//
// Este archivo construye TODAS las clases y las inicia en setup():
//   - Display y Buttons: hardware (I2C del OLED y pines de los botones).
//   - Menu, Credits, InfoWindow: ventanas independientes (hermanas, no
//     anidadas), instancias únicas para todo el juego. Sus begin() los
//     llama Engine al entrar en cada estado, y entre transiciones sus
//     valores se conservan.
//   - Engine: despachador puro. Su estado interno decide qué ventana se ve;
//     loop() solo llama engine.update() y engine.print().
//
// La clase Engine está en Engine.h / Engine.cpp.
// ====================================================================================

#include "Display.h"
#include "Buttons.h"
#include "Menu.h"
#include "Credits.h"
#include "InfoWindow.h"
#include "Engine.h"

#include <Arduino.h>

// Pines de los botones (orden del enum Button)
const int8_t BUTTON_PINS[Buttons::MAX_BUTTONS] = {
  02, 01, 42, 41,  // MOVE_UP, MOVE_RIGHT, MOVE_DOWN, MOVE_LEFT
  38, 40, 39, 47   // ACTION_UP, ACTION_RIGHT, ACTION_DOWN, ACTION_LEFT
};

// ====================================================================================
// Instancias únicas (construidas antes de setup(), iniciadas en setup())
// ====================================================================================

// Hardware
Display display;

// Entrada
Buttons buttons(BUTTON_PINS);

// Ventanas: clases independientes (hermanas, compartidas por referencia).
// Persisten entre estados: sus valores se conservan.
Menu menu(display, buttons, 0, "v0.1");
Credits credits(display, buttons);
InfoWindow info(display, buttons);

// Despachador: recibe las ventanas y decide cuál se ve según su estado.
Engine engine(display, buttons, menu, credits, info);

// ====================================================================================

void setup() {
  Serial.begin(115200);

  // Iniciar hardware y entrar al primer estado (menú -> menu.begin())
  display.begin();
  buttons.begin();
  engine.begin();

  Serial.println("Snake II");
}

// ====================================================================================

void loop() {
  engine.update();
  engine.print();
  display.show();
}

// ====================================================================================
// Fin
// ====================================================================================
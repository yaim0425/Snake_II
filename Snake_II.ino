// ====================================================================================
// SNAKE II — Enlace de dependencias
//
// Una única instancia de Display (y de Buttons) es la misma para todo el juego:
// se crean aquí y se comparten por referencia con la clase App.
//
// App es el despachador: decide qué ventana corre según su estado interno y hace
// la transición (con begin() de la ventana entrante) sin que loop() participe.
// loop() solo llama a app.update() y app.print().
// ====================================================================================

#include "Display.h"
#include "Buttons.h"
#include "App.h"

#include <Arduino.h>

// Pines de los botones (orden del enum Button)
const int8_t BUTTON_PINS[Buttons::MAX_BUTTONS] = {
  37, 39, 38, 36,  // MOVE_UP, MOVE_RIGHT, MOVE_DOWN, MOVE_LEFT
  41, 01, 02, 40   // ACTION_UP, ACTION_RIGHT, ACTION_DOWN, ACTION_LEFT
};

// Instancias únicas compartidas por todo el juego
Display display;
Buttons buttons(BUTTON_PINS);

// Despachador de ventanas
App app(display, buttons);

// ====================================================================================

void setup() {
  Serial.begin(115200);

  display.begin();
  buttons.begin();

  app.begin();

  Serial.println("Snake II");
}

// ====================================================================================

void loop() {
  app.update();
  app.print();
  display.show();
}

// ====================================================================================
// Fin
// ====================================================================================
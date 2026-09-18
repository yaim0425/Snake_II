// ====================================================================================
// MENU INICIAL (clase Menu)
//
// Titulo  : Snake II (tamano 2, centrado en Header)
// Opciones: Nuevo, Continuar, Dificultad, Sonido, Creditos (tamano 2, deslizantes)
// Pie     : Top: X pts (izquierda) | version (derecha)
// Navegacion: MOVE_UP/MOVE_DOWN (y ACTION_UP/ACTION_DOWN)
// ====================================================================================

#include "Display.h"
#include "Buttons.h"
#include "Menu.h"

// Controlador del OLED
Display display;

// Pines de los botones (orden del enum Button)
const int8_t BUTTON_PINS[Buttons::MAX_BUTTONS] = {
  37, 39, 38, 36,   // MOVE_UP, MOVE_RIGHT, MOVE_DOWN, MOVE_LEFT
  41, 01, 02, 40    // ACTION_UP, ACTION_RIGHT, ACTION_DOWN, ACTION_LEFT
};

Buttons buttons(BUTTON_PINS);

// Menu inicial
Menu menu(display, buttons, 0, "v0.1");

// ====================================================================================

void setup() {
  Serial.begin(115200);

  display.begin();
  buttons.begin();
  menu.begin();

  Serial.println("Menu inicial");
}

// ====================================================================================

void loop() {
  menu.update();

  display.clear();
  menu.print();
  display.show();
}

// ====================================================================================
// Fin del menu inicial
// ====================================================================================
// ====================================================================================
// MENU INICIAL
//
// Titulo  : Snake II (tamano 2, centrado en Header)
// Body    : Down-Left "Top: 0 pts" | Down-Right "v0.1" (tamano 1)
// ====================================================================================

#include "Display.h"

// Controlador del OLED
Display display;

// ====================================================================================

void setup() {
  Serial.begin(115200);

  display.begin();

  Serial.println("Menu inicial");
}

// ====================================================================================

void loop() {

  display.clear();

  // Titulo centrado en el Header, tamano 2
  display.drawTextAligned("Snake II", CENTER, TEXT_12x16, REGION_HEADER);

  // Pie del Body: Top a la izquierda abajo, version a la derecha abajo
  display.drawTextAligned("Top: 0 pts", LEFT_DOWN, TEXT_6x8, REGION_BODY);
  display.drawTextAligned("v0.1", RIGHT_DOWN, TEXT_6x8, REGION_BODY);

  display.show();
}

// ====================================================================================
// Fin del menu inicial
// ====================================================================================
// ====================================================================================
// MENU PRUEBA
//
// Menu con 5 opciones y seleccion automatica (rebote) usando la clase Display
// ====================================================================================

#include "Display.h"

// Controlador del OLED
Display display;

// Opciones del menu
const char* const MENU_OPTIONS[] = {
  "Nueva",
  "Continuar",
  "Dificultad",
  "Sonido",
  "Creditos"
};

const uint8_t MENU_COUNT = 5;

// Seleccion actual y direccion de movimiento
int8_t menuIndex = 0;
int8_t menuDir = 1;

// Tiempo de cambio de seleccion
uint32_t menuLast = 0;
const uint32_t MENU_DELAY = 2000;

// Dibuja una opcion centrada horizontalmente en la fila y del cuerpo
void menuOption(const char* text, int8_t y, bool selected) {

  int8_t x = (display.getWidth() - display.getTextWidth(text, TEXT_6x8)) / 2;

  if (selected)
    display.drawHighlight(text, x, y, TEXT_6x8);
  else
    display.drawText(text, x, y, TEXT_6x8);
}

// Dibuja el titulo en el Header y las opciones centradas en el Body
void menuPrint() {

  // Titulo centrado en la region Header (0,0)-(128,15), tamano 2
  display.drawTextAligned("PRUEBA", CENTER_UP, TEXT_12x16, REGION_HEADER);

  // Opciones centradas en la region Body (0,16)-(128,64)
  const int8_t OPTIONS_START = 20;  // 16 + (48 - 5*8) / 2

  for (int8_t i = 0; i < MENU_COUNT; i++)
    menuOption(MENU_OPTIONS[i], OPTIONS_START + i * 8, i == menuIndex);
}

// Avanza la seleccion cada 2 segundos (rebote: primera y ultima no conectadas)
void menuUpdate() {
  if (millis() - menuLast < MENU_DELAY) return;
  menuLast = millis();

  menuIndex += menuDir;

  if (menuIndex >= MENU_COUNT - 1) menuDir = -1;
  if (menuIndex <= 0) menuDir = 1;
}

// ====================================================================================

void setup() {
  Serial.begin(115200);
  display.begin();
  Serial.println("Menu PRUEBA");
}

// ====================================================================================

void loop() {
  menuUpdate();

  display.clear();
  menuPrint();
  display.show();
}

// ====================================================================================
// Fin del menu
// ====================================================================================
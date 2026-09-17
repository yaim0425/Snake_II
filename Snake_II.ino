// ====================================================================================
// MENU PRUEBA
//
// Demo de menu con seleccion automatica (rebote) usando la clase Display
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

// Espaciado y regiones
const int8_t MENU_TITLE_TOP = 0;
const int8_t MENU_TITLE_BOTTOM = 15;
const int8_t MENU_OPTIONS_TOP = 16;
const int8_t MENU_OPTIONS_BOTTOM = 63;

// Seleccion actual y direccion de movimiento
int8_t menuIndex = 0;
int8_t menuDir = 1;

// Tiempo de cambio de seleccion
uint32_t menuLast = 0;
const uint32_t MENU_DELAY = 2000;

// Dibuja una opcion centrada horizontalmente en la fila y
void menuOption(const char* text, int8_t y, bool selected) {

  int8_t x = (display.getWidth() - display.getTextWidth(text, TEXT_6x8)) / 2;
  int8_t th = display.getTextHeight(TEXT_6x8);

  Adafruit_SSD1306& screen = display.screen();

  screen.setTextSize(TEXT_6x8);

  if (selected) {
    screen.fillRoundRect(x - 4, y - 4, display.getTextWidth(text, TEXT_6x8) + 6, th + 4, 0, SSD1306_WHITE);
    screen.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
  } else {
    screen.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
  }

  screen.setCursor(x, y);
  screen.print(text);
}

// Dibuja el titulo y las opciones
void menuPrint() {

  // Titulo centrado entre (0,0) y (128,15)
  display.drawTextAligned("PRUEBA", CENTER_UP, TEXT_6x8);

  // Opciones centradas entre (0,16) y (128,64)
  int8_t startY = MENU_OPTIONS_TOP + (MENU_OPTIONS_BOTTOM - MENU_OPTIONS_TOP + 1 - MENU_COUNT * 8) / 2;

  for (int8_t i = 0; i < MENU_COUNT; i++)
    menuOption(MENU_OPTIONS[i], startY + i * 8, i == menuIndex);
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
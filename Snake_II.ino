// ====================================================================================
// DEMO DISPLAY: REGIONES, TEXTOS Y RESALTADO
//
// Recorre en bucle las nuevas funciones de Display:
//   - 9 alineaciones dentro de REGION_HEADER y REGION_BODY
//   - Resaltado en los 3 tamaños de texto
//   - Texto normal vs resaltado
//   - Menu PRUEBA (final de cada ciclo)
// ====================================================================================

#include "Display.h"

#include <stdio.h>
#include <string.h>

// Controlador del OLED
Display display;

// Tiempo que se muestra cada pantalla
const uint32_t STEP_DELAY = 1500;

// Contador de pasos de la demo
uint8_t step = 0;

// Opciones del menu
const char* const MENU_OPTIONS[] = {
  "Nueva",
  "Continuar",
  "Dificultad",
  "Sonido",
  "Creditos"
};

const uint8_t MENU_COUNT = 5;

int8_t menuIndex = 0;
int8_t menuDir = 1;

uint32_t menuLast = 0;
const uint32_t MENU_DELAY = 2000;

// ====================================================================================
// Pantallas de la demo
// ====================================================================================

// Muestra la alineacion dentro de la region Header
void screenHeaderAlign(uint8_t n) {
  char buf[16];

  display.drawTextAligned("*", (TextAlign)n, TEXT_6x8, REGION_HEADER);

  sprintf(buf, "H%d HEADER", n + 1);
  display.drawTextAligned(buf, CENTER_DOWN, TEXT_6x8, REGION_BODY);
}

// Muestra la alineacion dentro de la region Body
void screenBodyAlign(uint8_t n) {
  char buf[16];

  display.drawTextAligned("*", (TextAlign)n, TEXT_18x24, REGION_BODY);
  display.drawTextAligned("BODY", CENTER_UP, TEXT_6x8, REGION_HEADER);

  sprintf(buf, "B%d", n + 1);
  display.drawTextAligned(buf, RIGHT_DOWN, TEXT_6x8, REGION_HEADER);
}

// Muestra un texto resaltado centrado en el Body con el label indicado
void screenHighlight(uint8_t size, const char* label) {
  display.drawTextAligned(label, CENTER_DOWN, TEXT_6x8, REGION_BODY);
  display.drawHighlightAligned("BOX", CENTER, size, REGION_BODY);
}

// Compara texto normal vs resaltado en el Body
void screenCompare() {
  display.drawTextAligned("TEXTO / RESALTADO", CENTER_UP, TEXT_6x8, REGION_HEADER);

  uint8_t tw = display.getTextWidth("SIMPLE", TEXT_6x8);
  int8_t x = (display.getWidth() - tw) / 2;

  display.drawText("SIMPLE", x, 24, TEXT_6x8);
  display.drawHighlight("SIMPLE", x, 44, TEXT_6x8);
}

// ====================================================================================
// Menu PRUEBA
// ====================================================================================

void menuOption(const char* text, int8_t y, bool selected) {

  int8_t x = (display.getWidth() - display.getTextWidth(text, TEXT_6x8)) / 2;

  if (selected)
    display.drawHighlight(text, x, y, TEXT_6x8);
  else
    display.drawText(text, x, y, TEXT_6x8);
}

void menuPrint() {

  // Titulo centrado en la region Header (0,0)-(128,15)
  display.drawTextAligned("PRUEBA", CENTER, TEXT_6x8, REGION_HEADER);

  // Opciones centradas en la region Body (0,16)-(128,64)
  const int8_t OPTIONS_START = 20;  // 16 + (48 - 5*8) / 2

  for (int8_t i = 0; i < MENU_COUNT; i++)
    menuOption(MENU_OPTIONS[i], OPTIONS_START + i * 8, i == menuIndex);
}

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
  Serial.println("Demo regiones Display");
}

// ====================================================================================

void loop() {
  display.clear();

  if (step < 9) {
    screenHeaderAlign(step);
  } else if (step < 18) {
    screenBodyAlign(step - 9);
  } else if (step == 18) {
    screenHighlight(TEXT_6x8, "RESALTADO 6x8");
  } else if (step == 19) {
    screenHighlight(TEXT_12x16, "RESALTADO 12x16");
  } else if (step == 20) {
    screenHighlight(TEXT_18x24, "RESALTADO 18x24");
  } else if (step == 21) {
    screenCompare();
  } else {
    menuUpdate();
    menuPrint();
  }

  display.show();

  delay(STEP_DELAY);

  step++;
  if (step > 22) step = 0;
}

// ====================================================================================
// Fin de la demo
// ====================================================================================
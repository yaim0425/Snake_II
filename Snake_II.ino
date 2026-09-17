// ====================================================================================
// DEMO DE LA CLASE Display
//
// Muestra en bucle las principales funciones de Display.h / Display.cpp
// ====================================================================================

#include "Display.h"

#include <stdio.h>
#include <string.h>

// Controlador del OLED
Display display;

// Tiempo que se muestra cada demo
const uint32_t STEP_DELAY = 1500;

// Nombres de las alineaciones
const char* const ALIGN_NAME[] = {
  "LEFT_UP", "CENTER_UP", "RIGHT_UP",
  "CENTER_LEFT", "CENTER", "CENTER_RIGHT",
  "LEFT_DOWN", "CENTER_DOWN", "RIGHT_DOWN"
};

// Contador de pasos de la demo
uint8_t step = 0;

// Titulo superior de cada paso
void demoTitle(const char* title) {
  display.drawTextAligned(title, CENTER_UP, TEXT_6x8);
}

// Valor en el centro de la pantalla
void demoValue(const char* title, const char* value) {
  demoTitle(title);
  display.drawTextAligned(value, CENTER, TEXT_12x16);
}

// Muestra una propiedad de la pantalla por pantalla
void demoInfo(uint8_t info) {
  char buf[24];

  if (info == 0) {
    sprintf(buf, "%u px", display.getWidth());
    demoValue("ANCHO", buf);
  } else if (info == 1) {
    sprintf(buf, "%u px", display.getHeight());
    demoValue("ALTO", buf);
  } else if (info == 2) {
    sprintf(buf, "%u px", display.getCellSize());
    demoValue("CELDA", buf);
  } else if (info == 3) {
    sprintf(buf, "%u x %u", display.getColumns(), display.getRows());
    demoValue("CELDAS COL x FIL", buf);
  } else if (info == 4) {
    sprintf(buf, "%u x %u", display.getTextWidth("X", 1), display.getTextHeight(1));
    demoValue("TEXTO 6x8", buf);
  } else {
    sprintf(buf, "%u x %u", display.getTextWidth("X", 2), display.getTextHeight(2));
    demoValue("TEXTO 12x16", buf);
  }
}

// Muestra las 9 alineaciones de drawTextAligned
void demoAlign(TextAlign align) {
  demoTitle("ALINEACION");
  display.drawTextAligned("*", align, TEXT_6x8);
}

// Muestra botones con y sin seleccion
void demoButtons() {
  demoTitle("BOTONES");

  display.drawButton("JUGAR", CENTER_UP, TEXT_6x8, false);
  display.drawButton("OPCION", CENTER, TEXT_12x16, true);
  display.drawButton("SALIR", CENTER_DOWN, TEXT_18x24, false);
}

// Muestra pixeles punto a punto y getTextPos
void demoPixel() {
  char buf[24];

  demoTitle("PIXELES");

  for (uint8_t x = 0; x < display.getWidth(); x += 4)
    display.drawPixel(x, 28);

  TextPos p = display.getTextPos("X", CENTER_RIGHT, TEXT_12x16);
  sprintf(buf, "POS %u,%u", p.x, p.y);
  display.drawTextAligned(buf, CENTER_DOWN, TEXT_6x8);
}

// ====================================================================================

void setup() {
  Serial.begin(115200);
  display.begin();
  Serial.println("Demo Display lista");
}

// ====================================================================================

void loop() {
  display.clear();

  if (step < 9) {
    demoAlign((TextAlign)step);
  } else if (step < 15) {
    demoInfo(step - 9);
  } else if (step == 15) {
    demoButtons();
  } else if (step == 16) {
    demoTitle("BOTON SELECCIONADO");
    display.drawButton("SELEC", CENTER, TEXT_18x24, true);
  } else {
    demoPixel();
  }

  display.show();

  delay(STEP_DELAY);

  step++;
  if (step > 17) step = 0;
}

// ====================================================================================
// Fin de la demo
// ====================================================================================
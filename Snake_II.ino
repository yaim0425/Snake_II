// ====================================================================================
// PRUEBA DE BUTTONS
//
// Titulo: PRUEBA (tamano 2, centrado en Header)
// En el cuerpo se muestran los botones: pin, nombre y estado.
// Leyenda: H = se mantiene presionado, P = recien presionado, R = liberado
// ====================================================================================

#include "Display.h"
#include "Buttons.h"

#include <stdio.h>

// Controlador del OLED
Display display;

// Pines de los botones (orden del enum Button)
const int8_t BUTTON_PINS[Buttons::MAX_BUTTONS] = {
  37, 39, 38, 36,   // MOVE_UP, MOVE_RIGHT, MOVE_DOWN, MOVE_LEFT
  41, 01, 02, 40    // ACTION_UP, ACTION_RIGHT, ACTION_DOWN, ACTION_LEFT
};

// Nombre corto de cada boton
const char* const BUTTON_NAME[Buttons::MAX_BUTTONS] = {
  "Up", "Ri", "Do", "Le",
  "AU", "AR", "AD", "AL"
};

Buttons buttons(BUTTON_PINS);

// ====================================================================================

void setup() {
  Serial.begin(115200);

  display.begin();
  buttons.begin();

  Serial.println("Prueba Buttons");
}

// ====================================================================================

void loop() {
  buttons.read();

  display.clear();

  // Titulo centrado en el Header, tamano 2
  display.drawTextAligned("PRUEBA", CENTER, TEXT_12x16, REGION_HEADER);

  // Leyenda de estados
  display.drawTextAligned("H=HOLD P=NEW R=REL", CENTER_UP, TEXT_6x8, REGION_BODY);

  // Estado de cada boton en el cuerpo
  for (uint8_t i = 0; i < Buttons::MAX_BUTTONS; i++) {

    char stateCh = ' ';
    if (buttons.released(i))
      stateCh = 'R';
    else if (buttons.pressed(i))
      stateCh = 'P';
    else if (buttons.state(i))
      stateCh = 'H';

    char buf[12];
    sprintf(buf, "%02d %s %c", BUTTON_PINS[i], BUTTON_NAME[i], stateCh);

    int8_t x = (i < 4) ? 8 : 72;
    int8_t y = 24 + (i % 4) * 8;

    display.drawText(buf, x, y, TEXT_6x8);

    // Eventos por serial
    if (buttons.pressed(i))
      Serial.printf("[%02d] %s PRESSED\n", BUTTON_PINS[i], BUTTON_NAME[i]);
    if (buttons.released(i))
      Serial.printf("[%02d] %s RELEASED\n", BUTTON_PINS[i], BUTTON_NAME[i]);
  }

  display.show();
}

// ====================================================================================
// Fin de la prueba
// ====================================================================================
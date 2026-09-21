#include "Legend.h"

#include <Adafruit_GFX.h>

// ========================================================
// Constructor
// ========================================================

Legend::Legend(Display& display, Buttons& buttons)
  : _display(display),
    _buttons(buttons),
    _exit(false) {}

// ========================================================
// Inicialización (al entrar en la ventana)
// ========================================================

void Legend::begin() {
  _exit = false;
}

// ========================================================
// Actualizar (lee botones y espera a que cualquier botón salga)
// ========================================================

void Legend::update() {
  _buttons.read();

  for (uint8_t i = 0; i < Buttons::MAX_BUTTONS; i++) {
    if (_buttons.pressed(i)) {
      _exit = true;
      return;
    }
  }
}

// ========================================================
// Dibujar (dos pad direccionales + rótulos)
// ========================================================

void Legend::print() {
  // Pad MOVE (izquierda) con su rótulo
  drawPad(PAD_MOVE_X);
  _display.drawText("Move", 18, SIGN_Y, TEXT_6x8);

  // Pad ACTION (derecha) con su rótulo y las etiquetas Btn1..Btn4
  drawPad(PAD_ACTION_X);
  _display.drawText("Action", 64, SIGN_Y, TEXT_6x8);
  _display.drawText("Btn1", LABEL_X, Btn_Y[0], TEXT_6x8);
  _display.drawText("Btn2", LABEL_X, Btn_Y[1], TEXT_6x8);
  _display.drawText("Btn3", LABEL_X, Btn_Y[2], TEXT_6x8);
  _display.drawText("Btn4", LABEL_X, Btn_Y[3], TEXT_6x8);
}

// ========================================================
// Rombo de 4 flechas centrado en (cx, cy)
// ========================================================

void Legend::drawPad(int16_t cx) {
  drawArrow(0, cx, CY - R);      // ↑
  drawArrow(1, cx + R, CY);      // →
  drawArrow(2, cx, CY + R);      // ↓
  drawArrow(3, cx - R, CY);      // ←
}

// ========================================================
// Flecha sólida (punta hacia afuera del rombo)
// ========================================================

void Legend::drawArrow(uint8_t dir, int16_t cx, int16_t cy) {
  Adafruit_SSD1306& s = _display.screen();

  const int16_t T = 4;   // altura de la punta
  const int16_t B = 5;   // media base

  switch (dir) {
    case 0:   // ↑: punta arriba
      s.fillTriangle(cx, cy - T, cx - B, cy + (T - 1), cx + B, cy + (T - 1),
                     SSD1306_WHITE);
      break;
    case 1:   // →: punta a la derecha
      s.fillTriangle(cx + T, cy, cx - (T - 1), cy - B, cx - (T - 1), cy + B,
                     SSD1306_WHITE);
      break;
    case 2:   // ↓: punta abajo
      s.fillTriangle(cx, cy + T, cx - B, cy - (T - 1), cx + B, cy - (T - 1),
                     SSD1306_WHITE);
      break;
    case 3:   // ←: punta a la izquierda
      s.fillTriangle(cx - T, cy, cx + (T - 1), cy - B, cx + (T - 1), cy + B,
                     SSD1306_WHITE);
      break;
  }
}

// ========================================================
// Salida (true = se pidió ir al menú)
// ========================================================

bool Legend::done() const {
  return _exit;
}
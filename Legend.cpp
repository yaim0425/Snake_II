#include "Legend.h"

#include <Adafruit_GFX.h>
#include <stdio.h>
#include <string.h>

// ========================================================
// Textos del pie (identificador y función de cada rombo)
// ========================================================

const char* const Legend::BTN_NAME[4] = { "Btn1", "Btn2", "Btn3", "Btn4" };
const char* const Legend::BTN_FUNC[4] = {
  "Back / Pause", "Select", "None", "None"
};

// ========================================================
// Constructor
// ========================================================

Legend::Legend(Display& display, Buttons& buttons)
  : _display(display),
    _buttons(buttons),
    _exit(false),
    _selected(0),
    _setTime(0) {}

// ========================================================
// Inicialización (al entrar en la ventana)
// ========================================================

void Legend::begin() {
  _exit = false;
  _selected = 0;
  _setTime = millis();
}

// ========================================================
// Actualizar (lee botones y avanza el ciclo de parpadeo)
// ========================================================

void Legend::update() {
  _buttons.read();

  for (uint8_t i = 0; i < Buttons::MAX_BUTTONS; i++) {
    if (_buttons.pressed(i)) {
      _exit = true;
      return;
    }
  }

  // El rombo activo cambia cada DWELL_MS (avance lento)
  if (millis() - _setTime >= DWELL_MS) {
    _selected = (_selected + 1) % 4;
    _setTime = millis();
  }
}

// ========================================================
// Dibujar (pad MOVE + rombos de ACTION + texto del pie)
// ========================================================

void Legend::print() {
  Adafruit_SSD1306& s = _display.screen();

  // Pie: borra la banda, línea separadora y texto con la función del rombo
  // activo (centrado, mismo diseño que el menú)
  s.fillRect(0, PIE_LINE_ROW, _display.getWidth(),
             64 - PIE_LINE_ROW, SSD1306_BLACK);
  s.drawFastHLine(0, PIE_LINE_ROW, _display.getWidth(), SSD1306_WHITE);

  char buf[24];
  snprintf(buf, sizeof(buf), "%s: %s", BTN_NAME[_selected],
           BTN_FUNC[_selected]);
  int16_t x = (int16_t)((_display.getWidth() -
                         _display.getTextWidth(buf, TEXT_6x8)) / 2);
  _display.drawText(buf, x, PIE_TOP, TEXT_6x8);

  // Rótulo y pad MOVE (izquierda)
  _display.drawText("Move", 18, SIGN_Y, TEXT_6x8);
  drawPad(PAD_MOVE_X);

  // Rótulo y rombos de ACTION (derecha): las posiciones de un pad, el activo
  // parpadea muy rápido y el resto queda fijo como rombo completo
  _display.drawText("Action", 78, SIGN_Y, TEXT_6x8);
  for (uint8_t i = 0; i < 4; i++) {
    int16_t cx;
    int16_t cy;

    switch (i) {
      case 0:  cx = DIA_PAD_X;         cy = CY - DIA_R; break;  // ↑ Btn1
      case 1:  cx = DIA_PAD_X + DIA_R; cy = CY;         break;  // → Btn2
      case 2:  cx = DIA_PAD_X;         cy = CY + DIA_R; break;  // ↓ Btn3
      default: cx = DIA_PAD_X - DIA_R; cy = CY;         break;  // ← Btn4
    }

    bool blink = (i == _selected);
    drawDiamond(cx, cy, (!blink) || blinkVisible());
  }
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
// Rombo completo de DIA_SIZE centrado en (cx, cy)
// ========================================================

void Legend::drawDiamond(int16_t cx, int16_t cy, bool show) {
  if (!show) return;

  Adafruit_SSD1306& s = _display.screen();
  const int16_t h = DIA_SIZE / 2;   // media altura / ancho medio

  // Rombo simétrico completo: punta superior (cy-h), hombros (cy), punta
  // inferior (cy+h)
  s.fillTriangle(cx, cy - h, cx + h, cy, cx, cy + h, SSD1306_WHITE);
  s.fillTriangle(cx, cy - h, cx - h, cy, cx, cy + h, SSD1306_WHITE);
}

// ========================================================
// ¿El rombo activo está visible? (fijo HOLD_MS, luego parpadeo MUY rápido)
// ========================================================

bool Legend::blinkVisible() const {
  uint32_t now = millis();

  // Fijo (visible) mientras se mantiene el rombo: primero HOLD_MS
  if (now - _setTime < HOLD_MS) return true;

  // Luego parpadea MUY rápido: oculto durante el OFF_PCT inicial de cada
  // BLINK_PERIOD
  return !((now % BLINK_PERIOD) <
           (uint32_t)BLINK_PERIOD * BLINK_OFF_PCT / 100);
}

// ========================================================
// Salida (true = se pidió ir al menú)
// ========================================================

bool Legend::done() const {
  return _exit;
}
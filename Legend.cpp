#include "Legend.h"
#include "Globals.h"

#include <Adafruit_GFX.h>
#include <stdio.h>
#include <string.h>

// ========================================================
// Textos del pie (identificador y función de cada rombo)
// ========================================================

const char* const Legend::BTN_NAME[4] = { "Btn1", "Btn2", "Btn3", "Btn4" };
const char* const Legend::BTN_FUNC[4] = {
  "Back", "Select / Pause", "None", "None"
};

// ========================================================
// Constructor
// ========================================================

Legend::Legend()
  : _exit(false),
    _selected(0),
    _timer(),
    _redraw(true),
    _lastActive(0),
    _lastText(-1) {}

// ========================================================
// Inicialización (al entrar en la ventana)
// ========================================================

void Legend::begin() {
  _exit = false;
  _selected = 0;
  _timer.start();
  _redraw = true;
  _lastActive = 0;
  _lastText = -1;
}

// ========================================================
// Actualizar (consume eventos de botones ya leídos, reproduce
// sonido según el botón y avanza el ciclo de parpadeo)
// ========================================================

void Legend::update() {
  // Cualquier botón cierra la leyenda. El sonido depende del
  // botón presionado (prioridad si se pulsan varios a la vez):
  // MOVE = SFX_CLICK, ACTION_UP (Back) = SFX_BACK,
  // ACTION_RIGHT (Select / Pause) = SFX_CONFIRM,
  // ACTION_DOWN/ACTION_LEFT (None) = SFX_CLICK
  bool exit = false;

  if (buttons.moveUpPressed() || buttons.moveRightPressed() ||
      buttons.moveDownPressed() || buttons.moveLeftPressed()) {
    sound.play(Sound::SFX_CLICK);
    exit = true;
  } else if (buttons.actionUpPressed()) {
    sound.play(Sound::SFX_BACK);
    exit = true;
  } else if (buttons.actionRightPressed()) {
    sound.play(Sound::SFX_CONFIRM);
    exit = true;
  } else if (buttons.actionDownPressed() || buttons.actionLeftPressed()) {
    sound.play(Sound::SFX_CLICK);
    exit = true;
  }

  if (exit) {
    _exit = true;
    return;
  }

  // El rombo activo cambia cada DWELL_MS (avance lento)
  if (_timer.expired(DWELL_MS)) {
    _selected = (_selected + 1) % 4;
    _timer.start();
  }
}

// ========================================================
// Dibujar (pad MOVE + rombos de ACTION + texto del pie)
//
// Solo se limpia lo necesario: en el primer frame se hace un
// clear() completo y se dibujan los rótulos, los pads y los 4
// rombos fijos (estáticos). Después solo se borra/redibuja:
//   - el texto del pie, cuando cambia el rombo activo;
//   - la zona del rombo activo (parpadeo).
// El resto de la pantalla se mantiene intacto.
// ========================================================

void Legend::print() {
  Adafruit_SSD1306& s = display.screen();
  const int16_t w = display.getWidth();

  // Estáticos (una sola vez al entrar)
  if (_redraw) {
    display.clear();

    // Rótulo y pad MOVE (izquierda)
    display.drawText("Move", 18, SIGN_Y, TEXT_6x8);
    drawPad(PAD_MOVE_X);

    // Rótulo y rombos de ACTION (derecha): las posiciones de un pad
    display.drawText("Action", 78, SIGN_Y, TEXT_6x8);
    for (uint8_t i = 0; i < 4; i++) {
      int16_t cx;
      int16_t cy;
      diamondCenter(i, cx, cy);
      drawDiamond(cx, cy, true);
    }

    _redraw = false;
  }

  // Pie: solo se borra la banda y se redibuja cuando cambia la función
  // del rombo activo (mismo diseño que el menú)
  if (_lastText != (int16_t)_selected) {
    s.fillRect(0, PIE_LINE_ROW, w, 64 - PIE_LINE_ROW, SSD1306_BLACK);
    s.drawFastHLine(0, PIE_LINE_ROW, w, SSD1306_WHITE);

    char buf[24];
    snprintf(buf, sizeof(buf), "%s: %s", BTN_NAME[_selected],
             BTN_FUNC[_selected]);
    int16_t x = (int16_t)((w - display.getTextWidth(buf, TEXT_6x8)) / 2);
    display.drawText(buf, x, PIE_TOP, TEXT_6x8);

    _lastText = (int16_t)_selected;
  }

  // Rombo activo: se borra solo su zona y se redibuja según el parpadeo;
  // los inactivos ya están fijos en pantalla
  int16_t acx;
  int16_t acy;
  diamondCenter(_selected, acx, acy);
  s.fillRect(acx - DIA_SIZE / 2, acy - DIA_SIZE / 2,
             DIA_SIZE + 1, DIA_SIZE + 1, SSD1306_BLACK);
  drawDiamond(acx, acy, blinkVisible());

  // El rombo que dejó de ser activo debe quedar completo. Si el cambio lo
  // pilló en su fase oculta del parpadeo, su zona quedó borrada y nadie la
  // volvería a dibujar: se restaura el rombo completo como estático.
  if (_lastActive != (int8_t)_selected) {
    int16_t px;
    int16_t py;
    diamondCenter((uint8_t)_lastActive, px, py);
    drawDiamond(px, py, true);
    _lastActive = (int8_t)_selected;
  }
}

// ========================================================
// Posición del rombo i (centro), según el pad direccional
// ========================================================

void Legend::diamondCenter(uint8_t i, int16_t& cx, int16_t& cy) const {
  switch (i) {
    case 0:  cx = DIA_PAD_X;         cy = CY - DIA_R; break;  // ↑ Btn1
    case 1:  cx = DIA_PAD_X + DIA_R; cy = CY;         break;  // → Btn2
    case 2:  cx = DIA_PAD_X;         cy = CY + DIA_R; break;  // ↓ Btn3
    default: cx = DIA_PAD_X - DIA_R; cy = CY;         break;  // ← Btn4
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
  Adafruit_SSD1306& s = display.screen();

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

  Adafruit_SSD1306& s = display.screen();
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
  // Fijo (visible) mientras se mantiene el rombo: primero HOLD_MS
  if (!_timer.expired(HOLD_MS)) return true;

  // Luego parpadea MUY rápido: oculto durante el OFF_PCT inicial de cada
  // BLINK_PERIOD (anclado al _timer: sin salto de fase con el reloj 64 bits)
  return _timer.blinkOn(BLINK_PERIOD, BLINK_OFF_PCT);
}

// ========================================================
// Salida (true = se pidió ir al menú)
// ========================================================

bool Legend::done() const {
  return _exit;
}

// ====================================================================================
// Fin
// ====================================================================================

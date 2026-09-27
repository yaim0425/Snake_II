#include "Legend.h"
#include "Globals.h"

#include <Adafruit_GFX.h>
#include <stdio.h>
#include <string.h>

// ========================================================
// Textos del pie (identificador y función de cada rombo)
// ========================================================

const char* const Legend::BTN[4] = {
  "Btn1 / Back",
  "Btn2 / Select / Pause",
  "Btn3 / None",
  "Btn4 / None"
};

const char* const Legend::BTN_NAME[4] = { "Btn1", "Btn2", "Btn3", "Btn4" };
const char* const Legend::BTN_FUNC[4] = {
  "Back", "Select / Pause", "None", "None"
};

// ========================================================
// Constructor
// ========================================================

Legend::Legend()
  : _done(false),
    _step(0),
    _ticker(DWELL_MS),
    _timer(),
    _visibleDiamond(true),
    _redraw(true),
    _lastActive(0),
    _lastText(-1) {}

// ========================================================
// Inicialización (al entrar en la ventana)
// ========================================================

void Legend::begin() {
  _done = false;
  _step = 0;
  _ticker.start();
  _timer.start();
  _visibleDiamond = true;
  _redraw = true;
  _lastActive = 0;
  _lastText = -1;
}

// ========================================================
// Actualizar (consume eventos de botones ya leídos, reproduce
// sonido según el botón y avanza el ciclo de parpadeo)
// ========================================================

void Legend::update() {
  // Cualquier botón cierra la ventana. El sonido depende del
  // botón presionado (prioridad si se pulsan varios a la vez):
  // MOVE = SFX_CLICK, ACTION_UP (Back) = SFX_BACK,
  // ACTION_RIGHT (Select / Pause) = SFX_CONFIRM,
  // ACTION_DOWN/ACTION_LEFT (None) = SFX_CLICK
  bool exit = false;

  if (buttons.pressed(Buttons::MOVE_UP) || buttons.pressed(Buttons::MOVE_RIGHT) || buttons.pressed(Buttons::MOVE_DOWN) || buttons.pressed(Buttons::MOVE_LEFT)) {
    sound.play(Sound::SFX_CLICK);
    exit = true;
  } else if (buttons.pressed(Buttons::ACTION_UP)) {
    sound.play(Sound::SFX_BACK);
    exit = true;
  } else if (buttons.pressed(Buttons::ACTION_RIGHT)) {
    sound.play(Sound::SFX_CONFIRM);
    exit = true;
  } else if (buttons.pressed(Buttons::ACTION_DOWN) || buttons.pressed(Buttons::ACTION_LEFT)) {
    sound.play(Sound::SFX_CLICK);
    exit = true;
  }

  if (exit) {
    _done = true;
    return;
  }

  // El rombo activo cambia cada DWELL_MS (avance lento)
  uint32_t steps = _ticker.consume();
  if (steps) _step = (_step + steps) % 4;
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
  const int16_t w = display.getWidth();

  // Estáticos (una sola vez al entrar)
  if (_redraw) {
    display.clear();
    firstPrint();
    _redraw = false;
  }

  if (_lastText != _step) {
    // El rombo que dejó de ser activo debe quedar completo. Si el cambio lo
    // pilló en su fase oculta del parpadeo, su zona quedó borrada y nadie la
    // volvería a dibujar: se restaura el rombo completo como estático.
    int16_t acx, acy;
    diamondCenter(_step, acx, acy);
    drawDiamond(acx, acy, false);

    // Texto del pie: función del rombo activo
    display.fillRect(0, Config::Screen::FOOT_TOP, w, 8, true);
    display.drawText(BTN[_step], (Config::Screen::WIDTH - strlen(BTN[_step]) * 6) / 2, Config::Screen::FOOT_TOP, TEXT_6x8);
    _lastText = _step;
    _timer.start();  // reinicia el ciclo de parpadeo del rombo activo
  }

  // Rombo activo: se borra solo su zona y se redibuja según el parpadeo;
  // los inactivos ya están fijos en pantalla
  if (_timer.expired(HOLD_MS)) {
    bool visibleDiamond = _timer.blinkOn(BLINK_PERIOD, BLINK_OFF_PCT);
    if ((visibleDiamond && !_visibleDiamond) || (!visibleDiamond && _visibleDiamond)) {
        int16_t acx, acy;
        diamondCenter(_step, acx, acy);
        drawDiamond(acx, acy, _visibleDiamond);
        _visibleDiamond = !_visibleDiamond;
    }
  }
}

// ========================================================
// Primer frame: clear() completo + dibujar todo (estáticos)
// ========================================================

void Legend::firstPrint() {
  const int16_t w = display.getWidth();
  int16_t middleX = Config::Screen::WIDTH / 2;
  char* title = "Move";

  // Rótulo y pad MOVE (izquierda)
  title = "Move";
  display.drawText(title, (middleX - strlen(title) * 6) / 2, SIGN_Y, TEXT_6x8);
  drawArrow(0, PAD_MOVE_X, CY - R);  // ↑
  drawArrow(1, PAD_MOVE_X + R, CY);  // →
  drawArrow(2, PAD_MOVE_X, CY + R);  // ↓
  drawArrow(3, PAD_MOVE_X - R, CY);  // ←

  // Rótulo y rombos de ACTION (derecha): las posiciones de un pad
  title = "Action";
  display.drawText(title, middleX + (middleX - strlen(title) * 6) / 2, SIGN_Y, TEXT_6x8);
  for (uint8_t step = 0; step < 4; step++) {
    int16_t cx, cy;
    diamondCenter(step, cx, cy);
    drawDiamond(cx, cy, false);
  }

  display.fillRect(0, Config::Screen::FOOT_LINE, w, 1, false);
  display.drawText(BTN[0], (Config::Screen::WIDTH - strlen(BTN[0]) * 6) / 2, Config::Screen::FOOT_TOP, TEXT_6x8);
}

// ========================================================
// Posición del rombo i (centro), según el pad direccional
// ========================================================

void Legend::diamondCenter(uint8_t i, int16_t& cx, int16_t& cy) const {
  switch (i) {
    case 0:
      cx = DIA_PAD_X;
      cy = CY - DIA_R;
      break;  // ↑ Btn1
    case 1:
      cx = DIA_PAD_X + DIA_R;
      cy = CY;
      break;  // → Btn2
    case 2:
      cx = DIA_PAD_X;
      cy = CY + DIA_R;
      break;  // ↓ Btn3
    default:
      cx = DIA_PAD_X - DIA_R;
      cy = CY;
      break;  // ← Btn4
  }
}

// ========================================================
// Rombo de 4 flechas centrado en (cx, cy)
// ========================================================

void Legend::drawPad(int16_t cx) {
  drawArrow(0, cx, CY - R);  // ↑
  drawArrow(1, cx + R, CY);  // →
  drawArrow(2, cx, CY + R);  // ↓
  drawArrow(3, cx - R, CY);  // ←
}

// ========================================================
// Flecha sólida (punta hacia afuera del rombo)
// ========================================================

void Legend::drawArrow(uint8_t dir, int16_t cx, int16_t cy) {
  const int16_t T = 4;  // altura de la punta
  const int16_t B = 5;  // media base

  switch (dir) {
    case 0:  // ↑: punta arriba
      display.fillTriangle(cx, cy - T, cx - B, cy + (T - 1), cx + B, cy + (T - 1), false);
      break;
    case 1:  // →: punta a la derecha
      display.fillTriangle(cx + T, cy, cx - (T - 1), cy - B, cx - (T - 1), cy + B, false);
      break;
    case 2:  // ↓: punta abajo
      display.fillTriangle(cx, cy + T, cx - B, cy - (T - 1), cx + B, cy - (T - 1), false);
      break;
    case 3:  // ←: punta a la izquierda
      display.fillTriangle(cx - T, cy, cx + (T - 1), cy - B, cx + (T - 1), cy + B, false);
      break;
  }
}

// ========================================================
// Rombo completo de DIA_SIZE centrado en (cx, cy)
// ========================================================

void Legend::drawDiamond(int16_t cx, int16_t cy, bool black) {
  const int16_t h = DIA_SIZE / 2;  // media altura / ancho medio

  display.fillTriangle(cx, cy - h, cx + h, cy, cx, cy + h, black);
  display.fillTriangle(cx, cy - h, cx - h, cy, cx, cy + h, black);
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
  return _done;
}

// ====================================================================================
// Fin
// ====================================================================================

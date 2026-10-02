#include "Legend.h"
#include "Globals.h"

#include <Adafruit_GFX.h>
#include <stdio.h>
#include <string.h>

// ========================================================
// Textos del pie (identificador y función de cada rombo)
// ========================================================

const char* const Legend::BTN_FUNC[4] = {
  "Btn1 / Back",
  "Btn2 / Select / Pause",
  "Btn3 / None",
  "Btn4 / None"
};

const char* const Legend::BTN_NAME[4] = { "Btn1", "Btn2", "Btn3", "Btn4" };
const char* const Legend::BTN_XXX[4] = {
  "Back", "Select / Pause", "None", "None"
};

// ========================================================
// Constructor
// ========================================================

Legend::Legend()
  : _done(false),
    _btn(0),
    _ticker(CICLE),
    _holdDiamond(true),
    _timer(),
    _visibleDiamond(true),
    _clear(true),
    _lastActive(0),
    _lastText(-1) {}

// ========================================================
// Inicialización (al entrar en la ventana)
// ========================================================

void Legend::begin() {
  _done = false;
  _btn = 0;
  _ticker.start();
  _holdDiamond = true;
  _timer.start();
  _visibleDiamond = true;
  _clear = true;
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
  if (steps) _btn = (_btn + steps) % 4;

  if (_holdDiamond && _timer.expired(Config::Legend::HOLD))
    _holdDiamond = false;

  if (!_holdDiamond)
    _blinkDiamond = _visibleDiamond ^ _timer.blinkOn(Config::Legend::PERIOD, Config::Legend::OFF);

  _nextBtn = _prevBtn != _btn;
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
  firstPrint();
  blinkDiamond();
  nextBtn();

  // if (true) return;

  // if (_prevBtn != _btn) {
  //   // El rombo que dejó de ser activo debe quedar completo. Si el cambio lo
  //   // pilló en su fase oculta del parpadeo, su zona quedó borrada y nadie la
  //   // volvería a dibujar: se restaura el rombo completo como estático.
  //   int16_t acx, acy;
  //   diamondCenter(_btn, acx, acy);
  //   dDiamond(acx, acy, false);

  //   // Texto del pie: función del rombo activo
  //   display.fillRect(0, Config::Screen::FOOT_TOP, display.getWidth(), 8, true);
  //   display.drawText(BTN_FUNC[_btn], (Config::Screen::WIDTH - strlen(BTN_FUNC[_btn]) * 6) / 2, Config::Screen::FOOT_TOP, TEXT_6x8);
  //   _prevBtn = _btn;
  //   _timer.start();  // reinicia el ciclo de parpadeo del rombo activo
  // }

  // // Rombo activo: se borra solo su zona y se redibuja según el parpadeo;
  // // los inactivos ya están fijos en pantalla
  // if (_timer.expired(HOLD)) {
  //   bool visibleDiamond = _timer.blinkOn(PERIOD, OFF);
  //   if ((visibleDiamond && !_visibleDiamond) || (!visibleDiamond && _visibleDiamond)) {
  //     int16_t acx, acy;
  //     diamondCenter(_btn, acx, acy);
  //     dDiamond(acx, acy, _visibleDiamond);
  //     _visibleDiamond = !_visibleDiamond;
  //   }
  // }
}

// ========================================================
// Primer frame: clear() completo + dibujar todo (estáticos)
// ========================================================

void Legend::firstPrint() {
  if (!_clear) return;

  // ------------------------------------------------------

  display.clear();
  _clear = false;

  // ------------------------------------------------------

  // ------------------------------------------------------

  const int16_t w = Config::Screen::WIDTH;

  char* title = "Move / Action";
  const int16_t bodyTop = Config::Screen::BODY_TOP;

  // const int16_t x1 = PAD_L_X - PAD_RADIO;
  // const int16_t x2 = PAD_L_X + PAD_RADIO;
  // const int16_t y1 = bodyTop + PAD_RADIO;
  // const int16_t y2 = bodyTop + PAD_RADIO;

  // Rótulo y pad MOVE (izquierda)
  // title = "Move";
  // display.fillRect(PAD_L_X, bodyTop+PAD_L_R, PAD_L_R, 1, false);
  // display.fillRect(PAD_L_X - (Config::Diamond::SIZE + PAD_R), 0, 1, bodyTop + size * 2 + PAD_R, false);
  // display.fillRect(0, PAD_Y, 1, bodyTop + size * 2 + PAD_R  + 1, false);
  display.fillRect(0, bodyTop - 1, w, 1, false);

  // ------------------------------------------------------

  int16_t centerX = 0;
  int16_t centerY = 0;
  const int16_t size = Config::Diamond::SIZE;

  // ------------------------------------------------------

  int16_t middleX = w / 2;

  title = "Move";
  display.drawText(title, (middleX - strlen(title) * 6) / 2, TEXT_Y, TEXT_6x8);

  // Arriba (↑)
  centerX = PAD_LEFT_X;
  centerY = PAD_Y - PAD_RADIO;
  display.fillTriangle(centerX - size, centerY, centerX, centerY - size, centerX + size, centerY, false);

  // Derecha (→)
  centerX = PAD_LEFT_X + PAD_RADIO;
  centerY = PAD_Y;
  display.fillTriangle(centerX, centerY - size, centerX + size, centerY, centerX, centerY + size, false);

  // Abajo (↓)
  centerX = PAD_LEFT_X;
  centerY = PAD_Y + PAD_RADIO;
  display.fillTriangle(centerX - size, centerY, centerX, centerY + size, centerX + size, centerY, false);

  // Izquierda (←)
  centerX = PAD_LEFT_X - PAD_RADIO;
  centerY = PAD_Y;
  display.fillTriangle(centerX, centerY - size, centerX - size, centerY, centerX, centerY + size, false);

  // ------------------------------------------------------

  // Rótulo y rombos de ACTION (derecha): las posiciones de un pad
  title = "Action";
  display.drawText(title, middleX + (middleX - strlen(title) * 6) / 2, TEXT_Y, TEXT_6x8);

  for (_btn = 0; _btn < 4; _btn++) {
    _visibleDiamond = false;
    _blinkDiamond = true;
    blinkDiamond();
  }

  _btn = 0;
  // for (uint8_t step = 0; step < 4; step++) {
  //   int16_t cx, cy;
  //   diamondCenter(step, cx, cy);
  //   drawDiamond(cx, cy, false);
  // }

  // ------------------------------------------------------

  display.fillRect(0, Config::Screen::FOOT_LINE, w, 1, false);
  _nextBtn = true;
  nextBtn();
}


void Legend::blinkDiamond() {
  if (!_blinkDiamond) return;

  int16_t centerX = 0;
  int16_t centerY = 0;
  const int16_t size = Config::Diamond::SIZE;

  switch (_btn) {
    case 0:  // Btn1 (Arriba)
      centerX = PAD_RIGHT_X;
      centerY = PAD_Y - PAD_RADIO;
      break;

    case 1:  // Btn2 (Derecha)
      centerX = PAD_RIGHT_X + PAD_RADIO;
      centerY = PAD_Y;
      break;

    case 2:  // Btn3 (Abajo)
      centerX = PAD_RIGHT_X;
      centerY = PAD_Y + PAD_RADIO;
      break;

    case 3:  // Btn4 (Izquierda)
      centerX = PAD_RIGHT_X - PAD_RADIO;
      centerY = PAD_Y;
      break;
  }

  switch (_btn) {
    case 0:
    case 2:
      display.fillTriangle(centerX - size, centerY, centerX, centerY - size, centerX + size, centerY, _visibleDiamond);
      display.fillTriangle(centerX - size, centerY, centerX, centerY + size, centerX + size, centerY, _visibleDiamond);
      break;
    case 1:
    case 3:
      display.fillTriangle(centerX, centerY - size, centerX - size, centerY, centerX, centerY + size, _visibleDiamond);
      display.fillTriangle(centerX, centerY - size, centerX + size, centerY, centerX, centerY + size, _visibleDiamond);
      break;
  }

  _blinkDiamond = false;
  _visibleDiamond = !_visibleDiamond;
}

void Legend::nextBtn() {
  if (!_nextBtn) return;

  const int16_t w = Config::Screen::WIDTH;
  const char* text = BTN_FUNC[_btn];
  display.fillRect(0, Config::Screen::FOOT_TOP, w, 8, true);
  display.drawText(text, (w - strlen(text) * 6) / 2, Config::Screen::FOOT_TOP, TEXT_6x8);

  _visibleDiamond = false;
  _blinkDiamond = true;
  blinkDiamond();

  _prevBtn = _btn;
  _timer.start();
  _holdDiamond = true;
  _nextBtn = false;
}

// ========================================================
// Posición del rombo i (centro), según el pad direccional
// ========================================================

void Legend::diamondCenter(uint8_t i, int16_t& cx, int16_t& cy) const {
  switch (i) {
    case 0:
      cx = PAD_RADIO;
      cy = CY - PAD_RADIO;
      break;  // ↑ Btn1
    case 1:
      cx = PAD_RADIO + PAD_RADIO;
      cy = CY;
      break;  // → Btn2
    case 2:
      cx = PAD_RADIO;
      cy = CY + PAD_RADIO;
      break;  // ↓ Btn3
    default:
      cx = PAD_RADIO - PAD_RADIO;
      cy = CY;
      break;  // ← Btn4
  }
}

// ========================================================
// Rombo de 4 flechas centrado en (cx, cy)
// ========================================================

void Legend::drawPad(int16_t cx) {
  drawArrow(0, cx, CY - PAD_RADIO);  // ↑
  drawArrow(1, cx + PAD_RADIO, CY);  // →
  drawArrow(2, cx, CY + PAD_RADIO);  // ↓
  drawArrow(3, cx - PAD_RADIO, CY);  // ←
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

void Legend::dDiamond(int16_t cx, int16_t cy, bool black) {
  const int16_t h = DIA_SIZE / 2;  // media altura / ancho medio

  display.fillTriangle(cx, cy - h, cx + h, cy, cx, cy + h, black);
  display.fillTriangle(cx, cy - h, cx - h, cy, cx, cy + h, black);
}

// ========================================================
// ¿El rombo activo está visible? (fijo HOLD_MS, luego parpadeo MUY rápido)
// ========================================================

bool Legend::blinkVisible() const {
  // Fijo (visible) mientras se mantiene el rombo: primero HOLD_MS
  if (!_timer.expired(HOLD)) return true;

  // Luego parpadea MUY rápido: oculto durante el OFF_PCT inicial de cada
  // BLINK_PERIOD (anclado al _timer: sin salto de fase con el reloj 64 bits)
  return _timer.blinkOn(PERIOD, OFF);
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

#include "Boot.h"

#include <Adafruit_GFX.h>

// ========================================================
// Constructor
// ========================================================

Boot::Boot(Display& display, Buttons& buttons)
  : _display(display),
    _buttons(buttons),
    _shift(0),
    _prevShift(0),
    _tickAccum(0),
    _lastMs(0),
    _startMs(0),
    _done(false),
    _redraw(true) {}

// ========================================================
// Inicialización (al entrar en la ventana)
// ========================================================

void Boot::begin() {
  _shift = 0;
  _tickAccum = 0;
  _lastMs = millis();
  _startMs = millis();
  _done = false;
  _redraw = true;
}

// ========================================================
// Actualizar (procesa eventos de botones ya leídos y avanza el desplazamiento)
// ========================================================

void Boot::update() {
  // Cualquier botón termina la animación
  for (uint8_t i = 0; i < Buttons::MAX_BUTTONS; i++) {
    if (_buttons.pressed(i)) {
      _done = true;
      return;
    }
  }

  // Duración total
  if (millis() - _startMs >= TOTAL_MS) {
    _done = true;
    return;
  }

  // Avance de 1 px cada ANIM_TICK ms (acumulador por tiempo)
  uint32_t now = millis();
  _tickAccum += now - _lastMs;
  _lastMs = now;

  while (_tickAccum >= ANIM_TICK) {
    _tickAccum -= ANIM_TICK;
    _shift++;
    if (_shift >= BAR_SPACING) _shift = 0;
  }
}

// ========================================================
// Dibujar (bandas de líneas verticales)
//
// Solo se limpia lo necesario: en el primer frame se hace un
// clear() completo; luego únicamente se borran las columnas de
// las franjas anteriores que NO coinciden con las nuevas (las
// coincidentes se mantienen) y se dibujan las franjas nuevas.
// Si el desplazamiento no cambió, no se toca nada.
// ========================================================

void Boot::print() {
  if (_redraw) {
    _display.clear();
    _redraw = false;
  } else if (_shift == _prevShift) {
    return;  // nada cambió: el resto de la pantalla se mantiene
  } else {
    eraseOldBars();
  }

  drawBars();
  _prevShift = _shift;
}

// ========================================================
// Dibujar todas las franjas en el desplazamiento actual
// ========================================================

void Boot::drawBars() {
  const int16_t w = _display.getWidth();

  // TITULO: las líneas se mueven de izquierda a derecha
  for (int16_t x = 0; x < w; x += BAR_SPACING) {
    int16_t px = (x + _shift) % w;
    drawBar(px, TITLE_TOP, TITLE_H, w);
  }

  // CUERPO: las líneas se mueven de derecha a izquierda
  for (int16_t x = 0; x < w; x += BAR_SPACING) {
    int16_t px = ((x - _shift) % w + w) % w;
    drawBar(px, BODY_TOP, BODY_H, w);
  }
}

// ========================================================
// Borrar solo lo que dejan de ocupar las franjas: cada columna
// de la franja anterior que no pertenece a la franja nueva se
// pinta de negro; el resto de la pantalla no se toca
// ========================================================

void Boot::eraseOldBars() {
  const int16_t w = _display.getWidth();

  for (int16_t x = 0; x < w; x += BAR_SPACING) {
    int16_t oldX = (x + _prevShift) % w;
    int16_t newX = (x + _shift) % w;
    eraseBarDiff(oldX, newX, TITLE_TOP, TITLE_H, w);
  }

  for (int16_t x = 0; x < w; x += BAR_SPACING) {
    int16_t oldX = ((x - _prevShift) % w + w) % w;
    int16_t newX = ((x - _shift) % w + w) % w;
    eraseBarDiff(oldX, newX, BODY_TOP, BODY_H, w);
  }
}

// ========================================================
// Borra 1 px de las columnas de la franja vieja que no están en la nueva
// ========================================================

void Boot::eraseBarDiff(int16_t oldX, int16_t newX, uint8_t top,
                        uint8_t height, int16_t width) {
  Adafruit_SSD1306& s = _display.screen();

  for (uint8_t i = 0; i < BAR_W; i++) {
    int16_t oc = (oldX + i) % width;
    bool shared = false;
    for (uint8_t j = 0; j < BAR_W; j++) {
      if (((newX + j) % width) == oc) {
        shared = true;
        break;
      }
    }
    if (!shared) s.fillRect(oc, top, 1, height, SSD1306_BLACK);
  }
}

// ========================================================
// Dibujar una línea vertical de BAR_W px (con rebalse)
// ========================================================

void Boot::drawBar(int16_t x, uint8_t top, uint8_t height, int16_t width) {
  Adafruit_SSD1306& s = _display.screen();

  if (x + BAR_W <= width) {
    s.fillRect(x, top, BAR_W, height, SSD1306_WHITE);
  } else {
    // Rebalsa por el borde derecho: se dibuja en dos partes
    s.fillRect(x, top, width - x, height, SSD1306_WHITE);
    s.fillRect(0, top, BAR_W - (width - x), height, SSD1306_WHITE);
  }
}

// ========================================================
// Salida (true = la animación terminó)
// ========================================================

bool Boot::done() const {
  return _done;
}
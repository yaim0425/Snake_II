#include "Boot.h"

#include <Adafruit_GFX.h>

// ========================================================
// Constructor
// ========================================================

Boot::Boot(Display& display, Buttons& buttons)
  : _display(display),
    _buttons(buttons),
    _shift(0),
    _tickAccum(0),
    _lastMs(0),
    _startMs(0),
    _done(false) {}

// ========================================================
// Inicialización (al entrar en la ventana)
// ========================================================

void Boot::begin() {
  _shift = 0;
  _tickAccum = 0;
  _lastMs = millis();
  _startMs = millis();
  _done = false;
}

// ========================================================
// Actualizar (lee botones y avanza el desplazamiento)
// ========================================================

void Boot::update() {
  _buttons.read();

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
// ========================================================

void Boot::print() {
  Adafruit_SSD1306& s = _display.screen();
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
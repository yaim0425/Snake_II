#include "HardwareSerial.h"
#include "Boot.h"
#include "Globals.h"

#include <Adafruit_GFX.h>

// ========================================================
// Constructor (usa los servicios globales Display/Buttons)
// ========================================================

Boot::Boot()
  : _shift(0),
    _prevShift(0),
    _ticker(ANIM_TICK),
    _total(),
    _done(false),
    _redraw(true) {}

// ========================================================
// Inicialización (al entrar en la ventana)
// ========================================================

void Boot::begin() {
  _shift = 0;
  _ticker.start();
  _total.start();
  _done = false;
  _redraw = true;
}

// ========================================================
// Actualizar (procesa eventos de botones ya leídos y avanza el desplazamiento)
// ========================================================

void Boot::update() {
  // Cualquier botón termina la animación
  for (uint8_t i = 0; i < Buttons::MAX_BUTTONS; i++) {
    if (buttons.pressed(i)) {
      _done = true;
      return;
    }
  }

  // Duración total
  if (_total.expired(TOTAL_MS)) {
    _done = true;
    return;
  }

  // Avance de 1 px cada ANIM_TICK ms (Ticker acumula por tiempo;
  // consume() devuelve los pasos completos de una vez)
  uint32_t steps = _ticker.consume();
  if (steps) _shift = (uint8_t)((_shift + steps) % BAR_SPACING);
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
    display.clear();
    drawFirstBars();
    _redraw = false;
  } else if (_shift == _prevShift) {
    return;  // nada cambió: el resto de la pantalla se mantiene
  } else {
    // eraseOldBars();
  }

  drawBars();
  _prevShift = _shift;
}

// ========================================================
// Dibujar las franjas iniciales
// ========================================================

void Boot::drawFirstBars() {
  const int16_t w = display.getWidth();

  // TITULO: las líneas se mueven de izquierda a derecha
  for (int16_t x = 0; x < w; x += BAR_SPACING) {
    int16_t px = (x + _shift) % w;
    drawBar(px, Config::Screen::HEADER_TOP, Config::Screen::HEADER_H, (int16_t)SSD1306_WHITE);
  }

  // CUERPO: las líneas se mueven de derecha a izquierda
  for (int16_t x = 0; x < w; x += BAR_SPACING) {
    int16_t px = ((x - _shift) % w + w) % w;
    drawBar(px, Config::Screen::BODY_TOP, Config::Screen::BODY_H, (int16_t)SSD1306_WHITE);
  }
}

// ========================================================
// Dibujar todas las franjas en el desplazamiento actual
// ========================================================

void Boot::drawBars() {
  const int16_t w = display.getWidth();

  // TITULO: las líneas se mueven de izquierda a derecha
  for (int16_t x = 0; x < w; x += BAR_SPACING) {
    int16_t px = (x + _shift + w + BAR_W - 1) % w;
    drawBar(px, Config::Screen::HEADER_TOP, Config::Screen::HEADER_H, (int16_t)SSD1306_WHITE);
    px = ((px - BAR_W) % w + w) % w;
    drawBar(px, Config::Screen::HEADER_TOP, Config::Screen::HEADER_H, (int16_t)SSD1306_BLACK);
  }

  // CUERPO: las líneas se mueven de derecha a izquierda
  for (int16_t x = 0; x < w; x += BAR_SPACING) {
    int16_t px = ((x - _shift) % w + w - BAR_W + 1) % w;
    drawBar(px, Config::Screen::BODY_TOP, Config::Screen::BODY_H, (int16_t)SSD1306_WHITE);
    px = (px + BAR_W) % w;
    drawBar(px, Config::Screen::BODY_TOP, Config::Screen::BODY_H, (int16_t)SSD1306_BLACK);
  }
}

// ========================================================
// Borrar solo lo que dejan de ocupar las franjas: cada columna
// de la franja anterior que no pertenece a la franja nueva se
// pinta de negro; el resto de la pantalla no se toca
// ========================================================

void Boot::eraseOldBars() {
  const int16_t w = display.getWidth();

  for (int16_t x = 0; x < w; x += BAR_SPACING) {
    int16_t oldX = (x + _prevShift) % w;
    int16_t newX = (x + _shift) % w;
    eraseBarDiff(oldX, newX, Config::Screen::HEADER_TOP, Config::Screen::HEADER_H, w);
  }

  for (int16_t x = 0; x < w; x += BAR_SPACING) {
    int16_t oldX = ((x - _prevShift) % w + w) % w;
    int16_t newX = ((x - _shift) % w + w) % w;
    eraseBarDiff(oldX, newX, Config::Screen::BODY_TOP, Config::Screen::BODY_H, w);
  }
}

// ========================================================
// Borra 1 px de las columnas de la franja vieja que no están en la nueva
// ========================================================

void Boot::eraseBarDiff(int16_t oldX, int16_t newX, uint8_t top,
                        uint8_t height, int16_t width) {
  Adafruit_SSD1306& s = display.screen();

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

void Boot::drawBar(int16_t x, uint8_t y, uint8_t height, int16_t color) {
  const int16_t w = display.getWidth();

  if (x + BAR_W <= w) {
    display.fillRect(x, y, BAR_W, height, color);
  } else {
    // Rebalsa por el borde derecho: se dibuja en dos partes
    display.fillRect(x, y, w - x, height, color);
    display.fillRect(0, y, BAR_W - (w - x), height, color);
  }
}

// ========================================================
// Salida (true = la animación terminó)
// ========================================================

bool Boot::done() const {
  return _done;
}

// ====================================================================================
// Fin
// ====================================================================================

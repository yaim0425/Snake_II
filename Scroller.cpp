#include "Scroller.h"
#include "Globals.h"

#include <string.h>

// ========================================================
// Constructor (usa la Display global)
// ========================================================

Scroller::Scroller()
  : _height(16),
    _dir(1),
    _slideX(0),
    _ticker(ANIM_TICK),
    _dirty(true) {
}

// ========================================================
// Inicialización (al entrar en la ventana: franja centrada,
// sin desplazamiento)
// ========================================================

void Scroller::begin() {
  _slideX = 0;
  _ticker.start();
  _dirty = true;
}

// ========================================================
// Componer el texto en la franja (texto centrado)
//
// Calcula el límite de caracteres según el tamaño
// (ancho / (6 * size)) y trunca silenciosamente si el
// texto excede el límite. El texto se compone en un
// canvas auxiliar y luego se extraen las columnas.
// ========================================================

void Scroller::setTexto(const char* text, uint8_t height, uint8_t size) {
  if (height > STRIP_H) height = STRIP_H;
  if (size < 1) size = 1;
  if (size > 3) size = 3;

  _height = height;

  uint8_t maxChars = (uint8_t)(display.getWidth() / (6 * size));

  char buffer[64];
  strncpy(buffer, text, maxChars);
  buffer[maxChars] = '\0';

  GFXcanvas8 canvas((uint16_t)display.getWidth(), height);

  int16_t textW = display.getTextWidth(buffer, size);
  int16_t x0 = ((int16_t)display.getWidth() - textW) / 2;

  canvas.fillScreen(0);
  canvas.setTextSize(size);
  canvas.setTextColor(1);
  canvas.setCursor(x0, 0);
  canvas.print(buffer);

  uint8_t heightBytes = height / 8;
  for (uint8_t row = 0; row < heightBytes; row++) {
    for (uint16_t col = 0; col < display.getWidth(); col++) {
      uint8_t byte = 0;
      for (uint8_t bit = 0; bit < 8; bit++) {
        uint8_t y = row * 8 + bit;
        if (y < height && canvas.getPixel(col, y) != 0) {
          byte |= (uint8_t)(1 << bit);
        }
      }
      _strip[row][col] = (int8_t)byte;
    }
  }

  _dirty = true;
}

// ========================================================
// Inicio de la transición lateral: la franja arranca
// completamente fuera de pantalla y entra desde un lado
// ========================================================

void Scroller::startSlide(int8_t dir) {
  _dir = dir;
  _slideX = (_dir > 0) ? (int16_t)display.getWidth()
                       : -(int16_t)display.getWidth();
  _ticker.start();
  _dirty = true;
}

// ========================================================
// Animación: la franja avanza 1 px por cada ANIM_TICK ms
// (Ticker acumula por tiempo, constante aunque el loop sea lento)
//
// Mientras la franja está fuera del centro hay algo nuevo que
// pintar en cada frame (aunque en este frame no haya ticks), así
// que `_dirty` se pone a 1: al terminar el vuelo (|_slideX| = 0)
// queda un último volcado que asienta la franja y, del frame
// siguiente en adelante, blit() ya no hace nada.
// ========================================================

void Scroller::animate() {
  if (_slideX == 0) return;
  _dirty = true;

  uint32_t steps = _ticker.consume();
  for (uint32_t i = 0; i < steps; i++) {
    if (_dir > 0) {
      if (_slideX > 0) _slideX--;
    } else {
      if (_slideX < 0) _slideX++;
    }
  }
}

// ========================================================
// Volcado forzado (lo llama la ventana tras su display.clear(),
// que se lleva por delante lo ya volcado)
// ========================================================

void Scroller::redraw() {
  _dirty = true;
}

// ========================================================
// Volcar la franja a la pantalla en la fila y (fondo
// blanco, texto negro). Es un no-op si no hay nada nuevo
// que volcar (dirty flag a 0): en reposo la franja ya está
// en pantalla y no hace falta repintarla en cada frame.
// ========================================================

void Scroller::blit(int16_t y) {
  if (!_dirty) return;

  Adafruit_SSD1306& s = display.screen();
  uint8_t heightBytes = _height / 8;

  for (uint16_t sx = 0; sx < display.getWidth(); sx++) {
    int16_t sc = (int16_t)sx - _slideX;
    if (sc < 0 || sc >= (int16_t)STRIP_W) continue;

    for (uint8_t row = 0; row < heightBytes; row++) {
      int8_t byte = _strip[row][sc];
      for (uint8_t bit = 0; bit < 8; bit++) {
        uint8_t py = y + row * 8 + bit;
        if (py >= display.getHeight()) break;
        bool pixel = byte & (1 << bit);
        s.drawPixel(sx, py, pixel ? SSD1306_BLACK : SSD1306_WHITE);
      }
    }
  }

  _dirty = false;
}

// ====================================================================================
// Fin
// ====================================================================================

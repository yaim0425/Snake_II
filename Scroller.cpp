#include "Scroller.h"
#include "Globals.h"

// ========================================================
// Constructor: reserva los canvas de cada banda y los de la
// tira/compositor (usa la Display global)
// ========================================================

Scroller::Scroller(uint8_t bands, const uint8_t* bandHeights)
  : _bands((bands == 0) ? 1 : bands),
    _bandHeights(new uint8_t[bands == 0 ? 1 : bands]),
    _chipBox(new GFXcanvas8*[bands == 0 ? 1 : bands]),
    _composer(display.getWidth(), STRIP_H),
    _dir(1),
    _slideX(0),
    _ticker(ANIM_TICK),
    _dirty(true),
    _blitted(0) {

  for (uint8_t i = 0; i < _bands; i++) {
    _bandHeights[i] = (bandHeights != nullptr) ? bandHeights[i] : STRIP_H;
    _chipBox[i] = new GFXcanvas8(display.getWidth(), _bandHeights[i]);
  }
}

// ========================================================
// Destructor: libera los canvas de las bandas
// ========================================================

Scroller::~Scroller() {
  for (uint8_t i = 0; i < _bands; i++) delete _chipBox[i];
  delete[] _chipBox;
  delete[] _bandHeights;
}

// ========================================================
// Inicialización (al entrar en la ventana: tira centrada,
// sin desplazamiento)
//
// La banda queda marcada para volcar: la ventana acaba de
// limpiar la pantalla, así que hay que pintarla aunque la tira
// no se mueva.
// ========================================================

void Scroller::begin() {
  _slideX = 0;
  _ticker.start();
  _dirty = true;
  _blitted = 0;
}

// ========================================================
// Composición de la tira: texto centrado en la matriz de 1 bit
// (se compone en el canvas auxiliar sin tocar las bandas
// actuales)
// ========================================================

void Scroller::compose(const char* text, uint8_t size) {
  int16_t x0 = (int16_t)((display.getWidth() -
                          display.getTextWidth(text, size)) / 2);

  _composer.fillScreen(CHIP_BG);
  _composer.setTextSize(size);
  _composer.setTextColor(CHIP_TEXT, CHIP_BG);
  _composer.setCursor(x0, 0);
  _composer.print(text);

  // Copiar a la matriz de 1 bit: 1 = glifo, 0 = fondo
  for (uint8_t r = 0; r < STRIP_H; r++) {
    for (uint8_t c = 0; c < STRIP_W; c++) {
      uint8_t bit = (uint8_t)(1 << (c % 8));
      if (_composer.getPixel(c, r) == CHIP_TEXT)
        _strip[r][c / 8] |= bit;
      else
        _strip[r][c / 8] &= (uint8_t)~bit;
    }
  }
}

// ========================================================
// Inicio de la transición lateral: la tira arranca
// completamente fuera de pantalla y entra desde un lado
// ========================================================

void Scroller::startSlide(int8_t dir) {
  _dir = dir;
  _slideX = (_dir > 0) ? (int16_t)display.getWidth()
                       : -(int16_t)display.getWidth();
  _ticker.start();
  _dirty = true;    // entra una tira nueva: hay que volcar las bandas
  _blitted = 0;
}

// ========================================================
// Animación: la tira avanza 1 px por cada ANIM_TICK ms
// (Ticker acumula por tiempo, constante aunque el loop sea lento)
//
// Mientras la tira está fuera del centro hay algo nuevo que
// pintar en cada frame (aunque en este frame no haya ticks), así
// que `_dirty` se pone a 1: al terminar el vuelo (|_slideX| = 0)
// queda un último volcado que asienta la banda y, del frame
// siguiente en adelante, blit() ya no hace nada.
// ========================================================

void Scroller::animate() {
  if (_slideX == 0) return;
  _dirty = true;

  uint32_t steps = _ticker.consume();
  for (uint32_t i = 0; i < steps; i++) {
    if (_dir > 0) {              // entra por la derecha: se mueve hacia la izquierda
      if (_slideX > 0) _slideX--;
    } else {                     // entra por la izquierda: se mueve hacia la derecha
      if (_slideX < 0) _slideX++;
    }
  }
}

// ========================================================
// Volcado forzado (lo llama la ventana tras su display.clear(),
// que se lleva por delante lo ya volcado)
// ========================================================

void Scroller::invalidate() {
  _dirty = true;
  _blitted = 0;
}

// ========================================================
// Pintar las columnas visibles de la tira sobre una banda
// persistente: cada columna de la tira (incluidos sus espacios
// de fondo) sobrescribe lo que había, así la opción anterior se
// mantiene hasta ser borrada por la nueva
// ========================================================

void Scroller::slideStrip(GFXcanvas8& chipBox, uint8_t h) {
  for (uint8_t r = 0; r < h; r++) {
    for (uint16_t sx = 0; sx < (uint16_t)display.getWidth(); sx++) {
      int16_t sc = (int16_t)sx - _slideX;   // columna de la matriz (borde izq. = _slideX)
      if (sc < 0 || sc >= (int16_t)STRIP_W) continue;

      bool glyph = _strip[r][sc / 8] & (uint8_t)(1 << (sc % 8));
      chipBox.drawPixel(sx, r, glyph ? CHIP_TEXT : CHIP_BG);
    }
  }
}

// ========================================================
// Volcado de una banda: sin borrado, la vieja queda hasta que
// la nueva la cubre. Todas las bandas usan el mismo _slideX y
// deslizan sincronizadas.
//
// Si no hay nada nuevo que volcar (`_dirty` a 0) se sale sin
// hacer nada: la banda persistente ya está en pantalla y sus
// 2048 px no hay que repintarlos en cada frame. La bandera se
// limpia recién cuando TODAS las bandas se han volcado en este
// frame (`_blitted` llega a `_bands`), porque blit() se llama una
// vez por banda y con varias (Créditos) un volcado no basta para
// darla por hecha.
// ========================================================

void Scroller::blit(uint8_t band, int16_t y, uint16_t fgColor,
                    uint16_t bgColor) {
  if (band >= _bands) return;
  if (!_dirty) return;   // en reposo: la banda ya está en pantalla

  GFXcanvas8& chipBox = *_chipBox[band];
  uint8_t h = _bandHeights[band];

  slideStrip(chipBox, h);

  Adafruit_SSD1306& s = display.screen();
  for (uint8_t r = 0; r < h; r++) {
    for (uint16_t c = 0; c < (uint16_t)display.getWidth(); c++) {
      uint8_t v = chipBox.getPixel(c, r);
      s.drawPixel(c, y + r, (v == CHIP_TEXT) ? fgColor : bgColor);
    }
  }

  if (++_blitted >= _bands) {
    _dirty = false;
    _blitted = 0;
  }
}

// ====================================================================================
// Fin
// ====================================================================================

#include "TextMatrix.h"
#include "Globals.h"

// ========================================================
// Constructor
// ========================================================

TextMatrix::TextMatrix()
  : _canvas(W, H),
    _chars(0),
    _truncated(false),
    _valid(false),
    _changed(false) {

  memset(_mat, 0, SIZE);
  _cache[0] = '\0';
}

// ========================================================
// Texto: componer solo si ha cambiado
// ========================================================

uint8_t TextMatrix::setText(const char* text) {
  if (text == nullptr) text = "";

  // Clave de caché: los MAX_CHARS + 1 primeros caracteres. El
  // último es el que decide si hay recorte, así que con él
  // basta para saber si la matriz va a cambiar.
  char key[CACHE_LEN];
  uint8_t i = 0;

  while (i < (CACHE_LEN - 1) && text[i] != '\0') {
    key[i] = text[i];
    i++;
  }
  key[i] = '\0';

  if (_valid && strcmp(_cache, key) == 0) return _chars;   // en reposo

  uint8_t chars = (i > MAX_CHARS) ? MAX_CHARS : i;
  _truncated = (i > MAX_CHARS);

  compose(key, chars);

  memcpy(_cache, key, sizeof(_cache));
  _chars = chars;
  _valid = true;
  _changed = true;

  return chars;
}

// ========================================================
// Composición: texto en el canvas y empaquetado a 1 bit
// ========================================================

void TextMatrix::compose(const char* text, uint8_t chars) {
  // Solo los `chars` que caben (el texto puede venir con uno más)
  char buf[MAX_CHARS + 1];
  memcpy(buf, text, chars);
  buf[chars] = '\0';

  // Ancho por aritmética: la fuente integrada es de anchura fija
  // (6 px por carácter), que es lo mismo que devuelve
  // display.getTextWidth()
  uint8_t w = chars * CHAR_W * TEXT_SIZE;

  _canvas.fillScreen(0);
  _canvas.setTextSize(TEXT_SIZE);
  _canvas.setTextColor(1, 0);
  _canvas.setCursor((int16_t)((W - w) / 2), 0);
  _canvas.print(buf);

  // Empaquetado: 1 byte cada 8 columnas, LSB primero (bit 0 = la
  // columna de la izquierda del byte), como la página del SSD1306
  // y las tiras del Scroller
  for (uint8_t y = 0; y < H; y++) {
    uint8_t* row = _mat + (y * BYTES_PER_ROW);

    for (uint8_t x = 0; x < W; x++) {
      uint8_t bit = (uint8_t)(1 << (x & 7));

      if (_canvas.getPixel(x, y) != 0) row[x >> 3] |= bit;
      else                           row[x >> 3] &= (uint8_t)~bit;
    }
  }
}

// ========================================================
// Recorte y bandera de repintado
// ========================================================

bool TextMatrix::truncated() const {
  return _truncated;
}

bool TextMatrix::changed() {
  bool c = _changed;
  _changed = false;
  return c;
}

// ========================================================
// Lectura
// ========================================================

bool TextMatrix::at(uint8_t x, uint8_t y) const {
  return (_mat[(y * BYTES_PER_ROW) + (x >> 3)] & (uint8_t)(1 << (x & 7))) != 0;
}

void TextMatrix::blit(int16_t y) const {
  for (uint8_t r = 0; r < H; r++) {
    for (uint8_t c = 0; c < W; c++) {
      if (at(c, r)) display.drawPixel(c, y + r, true);
    }
  }
}

// ====================================================================================
// Fin
// ====================================================================================
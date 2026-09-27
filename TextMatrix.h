#ifndef TEXTMATRIX_H
#define TEXTMATRIX_H

#include <Arduino.h>
#include <Adafruit_GFX.h>

// ========================================================
// TextMatrix — banda de 128x16 con un texto convertido a una
// matriz de 1 bit (256 B: 1 = glifo, 0 = fondo)
//
// El texto se compone con la fuente integrada de Adafruit a
// tamaño 2 (celda 12x16) en un canvas de 8 bits y luego se
// empaqueta bit a bit en la matriz.
//
// Caracteres: ASCII 32..126 (la fuente integrada no tiene ñ,
// tildes ni símbolos; si el texto trae uno, se dibuja como un
// hueco, porque el glifo está a cero).
//
// Anchura: la fuente integrada es de anchura fija (5 px de
// glifo + 1 de separación = 6), así que el ancho es
// `caracteres * 6 * TEXT_SIZE` y caben MAX_CHARS = 128 / 12 = 10
// caracteres. Lo que no cabe se recorta: setText() devuelve
// cuántos se han puesto y truncated() avisa del recorte.
//
// Caché: se comparan los MAX_CHARS + 1 primeros caracteres del
// texto (el último solo para detectar el recorte), así que si el
// texto no cambia no se recompone ni se repinta. En reposo
// changed() es false y no se toca ni el canvas ni la matriz.
//
// Gasto: el canvas de composición (128x16 = 2048 B) + la matriz
// (256 B). El texto vive en el canvas, no en RAM aparte.
// ========================================================

class TextMatrix {
public:

  // ========================================================
  // Geometría
  // ========================================================

  static constexpr uint8_t W = 128;                        // px de ancho
  static constexpr uint8_t H = 16;                         // px de alto (8 * TEXT_SIZE)
  static constexpr uint8_t BYTES_PER_ROW = W / 8;          // 16 B por fila
  static constexpr uint8_t SIZE = H * BYTES_PER_ROW;        // 256 B
  static constexpr uint8_t TEXT_SIZE = 2;                  // celda 12x16
  static constexpr uint8_t CHAR_W = 6;                     // celda de la fuente integrada
  static constexpr uint8_t CHAR_H = 8;
  static constexpr uint8_t MAX_CHARS = W / (CHAR_W * TEXT_SIZE);   // 10
  static constexpr uint8_t CACHE_LEN = MAX_CHARS + 2;      // 12 (11 chars + NUL)

  // ========================================================
  // Construcción
  // ========================================================

  TextMatrix();

  TextMatrix(const TextMatrix&) = delete;            // no copiar (canvas con RAM)
  TextMatrix& operator=(const TextMatrix&) = delete;

  // ========================================================
  // Texto
  // ========================================================

  // Compone `text` centrado en la matriz y devuelve cuántos
  // caracteres se han puesto (0..MAX_CHARS). Si no ha cambiado
  // respecto a la llamada anterior, no recompone nada y
  // devuelve el mismo valor.
  uint8_t setText(const char* text);

  // true si el texto se ha recortado (no cabía entero)
  bool truncated() const;

  // true si hay que repintar (la última setText recompuso).
  // Limpia la bandera: se lee una vez por repintado.
  bool changed();

  // ========================================================
  // Lectura y volcado
  // ========================================================

  // Píxel de la matriz (x = 0..127 de izquierda a derecha,
  // y = 0..15 de arriba abajo)
  bool at(uint8_t x, uint8_t y) const;

  // Pinta la matriz en la fila `y` de la pantalla: solo los
  // píxeles a 1 (el fondo lo pone la ventana con su clear de
  // la región). 2048 drawPixel, así que solo cuando el texto
  // ha cambiado.
  void blit(int16_t y) const;

private:

  // ========================================================
  // Estado
  // ========================================================

  GFXcanvas8 _canvas;      // composición del texto (8 bits por px)
  uint8_t _mat[SIZE];      // la matriz de 1 bit
  char _cache[CACHE_LEN];  // texto cacheado (primeros CACHE_LEN-1)
  uint8_t _chars;          // caracteres compuestos
  bool _truncated;         // el texto no cabía entero
  bool _valid;             // hay texto cacheado
  bool _changed;           // la última composición cambió la matriz

  // ========================================================
  // Internos
  // ========================================================

  // Pinta `chars` caracteres de `text` en el canvas y los
  // empaqueta en la matriz
  void compose(const char* text, uint8_t chars);
};

#endif
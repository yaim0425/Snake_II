#ifndef SPRITE_H
#define SPRITE_H

#include <Arduino.h>

// ========================================================
// Sprite — sprites de la serpiente y comida especial
// (contenido)
//
// Tabla estática con los sprites de las partes de la
// serpiente (estilo Nokia): cola, cuerpo, curvas, cabeza
// (fauces cerradas/abiertas) y panza. Cada sprite de la
// serpiente es de 4×4 px (SIZE) de 1 bit: 1 = glifo,
// 0 = fondo. También incluye el sprite de la comida especial
// (SPECIAL_FOOD, 8×4 px).
//
// Los 4×4 van EMPAQUETADOS: 1 bit por píxel, así cada
// sprite cabe en un uint16_t (16 px) en vez de 16 bytes, y
// los 27 sprites pasan de 432 a 54 bytes. El dibujo se sigue
// escribiendo legible (0/1, con `Pattern`); `pack()` lo
// aplana en tiempo de compilación, de modo que en memoria
// solo queda la tabla empaquetada (no queda una copia doble).
//
// Es un namespace de SOLO datos: no necesita instancia ni
// archivo .cpp; los sprites se leen con Sprite::pixel(part,
// x, y) y Sprite::SPECIAL_FOOD.
// ========================================================

namespace Sprite {

  // ========================================================
  // Dimensiones
  // ========================================================

  constexpr uint8_t SIZE = 4;            // ancho y alto del sprite de la serpiente (4×4 px)
  constexpr uint8_t BITS = SIZE * SIZE;  // px por sprite = bits de su uint16_t

  // ========================================================
  // Partes de la serpiente
  // ========================================================

  enum Part : uint8_t {

    TAIL_TO_UP = 0,
    TAIL_TO_RIGHT,
    TAIL_TO_DOWN,
    TAIL_TO_LEFT,

    BODY_TO_UP,
    BODY_TO_RIGHT,
    BODY_TO_DOWN,
    BODY_TO_LEFT,

    CORNER_RIGHT_UP,
    CORNER_RIGHT_DOWN,
    CORNER_LEFT_UP,
    CORNER_LEFT_DOWN,

    HEAD_UP_CLOSE,
    HEAD_RIGHT_CLOSE,
    HEAD_DOWN_CLOSE,
    HEAD_LEFT_CLOSE,

    HEAD_UP_OPEN,
    HEAD_RIGHT_OPEN,
    HEAD_DOWN_OPEN,
    HEAD_LEFT_OPEN,

    BELLY_TO_RIGHT,
    BELLY_TO_LEFT,

    BELLY_RIGHT_UP,
    BELLY_RIGHT_DOWN,
    BELLY_LEFT_UP,
    BELLY_LEFT_DOWN,

    EMPTY,

    COUNT
  };

  // ========================================================
  // Empaque: un sprite de 4×4 son 16 px de 1 bit = 16 bits,
  // o sea un uint16_t (no un byte por píxel). `Pattern` es el
  // dibujo legible de 0/1 que se escribe en el código y
  // `pack()` lo aplana en tiempo de compilación.
  //
  // Orden de los bits: fila 0,1,2,3 izquierda-derecha -> bit 15,14,...,0
  // (px (0,0) en el bit 15, px (3,3) en el bit 0), tal como se lee en `pixel()`.

  // ========================================================
  // Sprites de la serpiente (empaquetados: 1 bit por px)
  // ========================================================

  constexpr uint16_t SPRITES[COUNT] = {
    // ------------------------------------------------------
    // TAIL
    // ------------------------------------------------------
    pack(Pattern{ {0,1,1,0}, {0,1,1,0}, {0,1,0,0}, {0,1,0,0} }),  // TAIL_TO_UP
    pack(Pattern{ {0,0,0,0}, {0,0,1,1}, {1,1,1,1}, {0,0,0,0} }),  // TAIL_TO_RIGHT
    pack(Pattern{ {0,1,0,0}, {0,1,0,0}, {0,1,1,0}, {0,1,1,0} }),  // TAIL_TO_DOWN
    pack(Pattern{ {0,0,0,0}, {1,1,0,0}, {1,1,1,1}, {0,0,0,0} }),  // TAIL_TO_LEFT

    // ------------------------------------------------------
    // BODY
    // ------------------------------------------------------

    pack(Pattern{ {0,1,1,0}, {0,1,0,0}, {0,0,1,0}, {0,1,1,0} }),  // BODY_TO_UP
    pack(Pattern{ {0,0,0,0}, {1,1,0,1}, {1,0,1,1}, {0,0,0,0} }),  // BODY_TO_RIGHT
    pack(Pattern{ {0,1,1,0}, {0,0,1,0}, {0,1,0,0}, {0,1,1,0} }),  // BODY_TO_DOWN
    pack(Pattern{ {0,0,0,0}, {1,0,1,1}, {1,1,0,1}, {0,0,0,0} }),  // BODY_TO_LEFT

    // ------------------------------------------------------
    // CORNERS
    // ------------------------------------------------------

    pack(Pattern{ {0,1,1,0}, {0,1,0,1}, {0,0,1,1}, {0,0,0,0} }),  // CORNER_RIGHT_UP
    pack(Pattern{ {0,0,0,0}, {0,0,1,1}, {0,1,0,1}, {0,1,1,0} }),  // CORNER_RIGHT_DOWN
    pack(Pattern{ {0,1,1,0}, {1,0,1,0}, {1,1,0,0}, {0,0,0,0} }),  // CORNER_LEFT_UP
    pack(Pattern{ {0,0,0,0}, {1,1,0,0}, {1,0,1,0}, {0,1,1,0} }),  // CORNER_LEFT_DOWN

    // ------------------------------------------------------
    // HEAD CLOSE
    // ------------------------------------------------------

    pack(Pattern{ {0,0,0,0}, {0,1,1,0}, {0,1,1,0}, {0,1,0,1} }),  // HEAD_UP_CLOSE
    pack(Pattern{ {1,0,0,0}, {0,1,1,0}, {1,1,1,0}, {0,0,0,0} }),  // HEAD_RIGHT_CLOSE
    pack(Pattern{ {0,1,0,1}, {0,1,1,0}, {0,1,1,0}, {0,0,0,0} }),  // HEAD_DOWN_CLOSE
    pack(Pattern{ {0,0,0,1}, {0,1,1,0}, {0,1,1,1}, {0,0,0,0} }),  // HEAD_LEFT_CLOSE

    // ------------------------------------------------------
    // HEAD OPEN
    // ------------------------------------------------------

    pack(Pattern{ {0,0,0,0}, {1,0,0,1}, {0,1,1,0}, {0,1,0,1} }),  // HEAD_UP_OPEN
    pack(Pattern{ {1,0,1,0}, {0,1,0,0}, {1,1,0,0}, {0,0,1,0} }),  // HEAD_RIGHT_OPEN
    pack(Pattern{ {0,1,0,1}, {0,1,1,0}, {1,0,0,1}, {0,0,0,0} }),  // HEAD_DOWN_OPEN
    pack(Pattern{ {0,1,0,1}, {0,0,1,0}, {0,0,1,1}, {0,1,0,0} }),  // HEAD_LEFT_OPEN

    // ------------------------------------------------------
    // BELLY
    // ------------------------------------------------------

    pack(Pattern{ {0,1,1,0}, {1,1,0,1}, {1,0,1,1}, {0,1,1,0} }),  // BELLY_TO_RIGHT (compartido con BELLY_TO_UP)
    pack(Pattern{ {0,1,1,0}, {1,0,1,1}, {1,1,0,1}, {0,1,1,0} }),  // BELLY_TO_LEFT (compartido con BELLY_TO_DOWN)
    pack(Pattern{ {0,1,1,1}, {0,1,0,1}, {0,0,1,1}, {0,0,0,0} }),  // BELLY_RIGHT_UP
    pack(Pattern{ {0,0,0,0}, {0,0,1,1}, {0,1,0,1}, {0,1,1,1} }),  // BELLY_RIGHT_DOWN
    pack(Pattern{ {1,1,1,0}, {1,0,1,0}, {1,1,0,0}, {0,0,0,0} }),  // BELLY_LEFT_UP
    pack(Pattern{ {0,0,0,0}, {1,1,0,0}, {1,0,1,0}, {1,1,1,0} }),  // BELLY_LEFT_DOWN

    // ------------------------------------------------------
    // EMPTY
    // ------------------------------------------------------

    pack(Pattern{ {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0} })   // EMPTY
  };

  // ========================================================
  // Píxel de un sprite (x = columna, y = fila, desde 0).
  // Usa el empaque de 16 bits: px (0,0) -> bit 15.
  // ========================================================

  constexpr bool pixel(Part part, uint8_t x, uint8_t y) {
    return (SPRITES[(uint8_t)part] & ((uint16_t)1 << (BITS - 1 - (y * SIZE + x)))) != 0;
  }

  // ========================================================
  // Comida especial (8×4 px)
  // ========================================================

  constexpr uint8_t SPECIAL_FOOD_W = 8;   // ancho en px
  constexpr uint8_t SPECIAL_FOOD_H = 4;   // alto en px
  constexpr uint8_t SPECIAL_FOOD[SPECIAL_FOOD_H][SPECIAL_FOOD_W] = {
    { 0, 1, 0, 1, 0, 1, 0, 0 },
    { 1, 0, 1, 1, 1, 1, 1, 0 },
    { 1, 1, 1, 1, 1, 1, 1, 1 },
    { 0, 0, 1, 0, 0, 1, 0, 0 }
  };
}

#endif

// ====================================================================================
// Fin
// ====================================================================================
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
// los 27 sprites pasan de 432 a 54 bytes. Cada sprite se
// escribe como un literal binario de 16 bits, en cuatro
// grupos de 4 (una fila cada grupo, de arriba abajo) para
// que el dibujo siga siendo legible. La comida especial
// (8×4) se empaqueta por fila: 8 px = 1 byte, así que son
// 4 bytes en vez de 32, un literal binario de 8 por fila.
//
// Es un namespace de SOLO datos: no necesita instancia ni
// archivo .cpp; los sprites se leen con Sprite::pixel(part,
// x, y) y Sprite::specialPixel(x, y).
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
  // o sea un uint16_t (no un byte por píxel).
  //
  // Orden de los bits: fila 0,1,2,3 izquierda-derecha -> bit 15,14,...,0
  // (px (0,0) en el bit 15, px (3,3) en el bit 0), tal como se lee en `pixel()`.
  // Por eso cada sprite se escribe como un literal binario de 16
  // bits partido en cuatro grupos de 4: un grupo por fila, de
  // arriba abajo. 1 = glifo, 0 = fondo.

  // ========================================================
  // Sprites de la serpiente (empaquetados: 1 bit por px)
  // ========================================================

  constexpr uint16_t SPRITES[COUNT] = {
    // ------------------------------------------------------
    // TAIL
    // ------------------------------------------------------
    0b0110'0110'0100'0100,  // TAIL_TO_UP
    0b0000'0011'1111'0000,  // TAIL_TO_RIGHT
    0b0100'0100'0110'0110,  // TAIL_TO_DOWN
    0b0000'1100'1111'0000,  // TAIL_TO_LEFT

    // ------------------------------------------------------
    // BODY
    // ------------------------------------------------------

    0b0110'0100'0010'0110,  // BODY_TO_UP
    0b0000'1101'1011'0000,  // BODY_TO_RIGHT
    0b0110'0010'0100'0110,  // BODY_TO_DOWN
    0b0000'1011'1101'0000,  // BODY_TO_LEFT

    // ------------------------------------------------------
    // CORNERS
    // ------------------------------------------------------

    0b0110'0101'0011'0000,  // CORNER_RIGHT_UP
    0b0000'0011'0101'0110,  // CORNER_RIGHT_DOWN
    0b0110'1010'1100'0000,  // CORNER_LEFT_UP
    0b0000'1100'1010'0110,  // CORNER_LEFT_DOWN

    // ------------------------------------------------------
    // HEAD CLOSE
    // ------------------------------------------------------

    0b0000'0110'0110'0101,  // HEAD_UP_CLOSE
    0b1000'0110'1110'0000,  // HEAD_RIGHT_CLOSE
    0b0101'0110'0110'0000,  // HEAD_DOWN_CLOSE
    0b0001'0110'0111'0000,  // HEAD_LEFT_CLOSE

    // ------------------------------------------------------
    // HEAD OPEN
    // ------------------------------------------------------

    0b0000'1001'0110'0101,  // HEAD_UP_OPEN
    0b1010'0100'1100'0010,  // HEAD_RIGHT_OPEN
    0b0101'0110'1001'0000,  // HEAD_DOWN_OPEN
    0b0101'0010'0011'0100,  // HEAD_LEFT_OPEN

    // ------------------------------------------------------
    // BELLY
    // ------------------------------------------------------

    0b0110'1101'1011'0110,  // BELLY_TO_RIGHT (compartido con BELLY_TO_UP)
    0b0110'1011'1101'0110,  // BELLY_TO_LEFT (compartido con BELLY_TO_DOWN)
    0b0111'0101'0011'0000,  // BELLY_RIGHT_UP
    0b0000'0011'0101'0111,  // BELLY_RIGHT_DOWN
    0b1110'1010'1100'0000,  // BELLY_LEFT_UP
    0b0000'1100'1010'1110,  // BELLY_LEFT_DOWN

    // ------------------------------------------------------
    // EMPTY
    // ------------------------------------------------------

    0b0000'0000'0000'0000  // EMPTY
  };

  // ========================================================
  // Píxel de un sprite (x = columna, y = fila, desde 0).
  // Usa el empaque de 16 bits: px (0,0) -> bit 15.
  // ========================================================

  constexpr bool pixel(Part part, uint8_t x, uint8_t y) {
    return (SPRITES[(uint8_t)part] & ((uint16_t)1 << (BITS - 1 - (y * SIZE + x)))) != 0;
  }

  // ========================================================
  // Comida especial (8×4 px, empaquetado por fila: 8 px de
  // 1 bit = 8 bits = 1 byte por fila, o sea 4 bytes en vez
  // de 32). Cada fila es un literal binario de 8 px, con el
  // px (0,0) en el bit 7.
  // ========================================================

  constexpr uint8_t SPECIAL_FOOD_W = 8;   // ancho en px (8 px = 1 byte por fila)
  constexpr uint8_t SPECIAL_FOOD_H = 4;   // alto en px (4 filas = 4 bytes)

  constexpr uint8_t SPECIAL_FOOD[SPECIAL_FOOD_H] = {
    0b0101'0100,  // fila 0
    0b1011'1110,  // fila 1
    0b1111'1111,  // fila 2
    0b0010'0100   // fila 3
  };

  // Píxel de la comida especial (x = columna, y = fila): px (0,0) = bit 7
  constexpr bool specialPixel(uint8_t x, uint8_t y) {
    return (SPECIAL_FOOD[y] & ((uint8_t)0x80 >> x)) != 0;
  }
}

#endif

// ====================================================================================
// Fin
// ====================================================================================
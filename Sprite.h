#ifndef SPRITE_H
#define SPRITE_H

#include <Arduino.h>

// ========================================================
// Sprite — sprites de la serpiente, comida especial
// (contenido)
//
// Tabla estática con los sprites de las partes de la
// serpiente (estilo Nokia): cola, cuerpo, curvas, cabeza
// (fauces cerradas/abiertas) y panza. Cada sprite de la
// serpiente es de 4×4 px (SIZE) de 1 bit: 1 = glifo,
// 0 = fondo. También incluye el sprite de la comida especial
// (SPECIAL_FOOD, 8×4 px) y el logo de la pantalla de
// arranque (LOGO, 80×48 px).
//
// Los 4×4 van EMPAQUETADOS: 1 bit por píxel, así cada
// sprite cabe en un uint16_t (16 px) en vez de 16 bytes, y
// los 27 sprites pasan de 432 a 54 bytes. Cada sprite se
// escribe como un literal binario de 16 bits, en cuatro
// grupos de 4 (una fila cada grupo, de arriba abajo) para
// que el dibujo siga siendo legible. La comida especial
// (8×4) se empaqueta por fila: 8 px = 1 byte, así que son
// 4 bytes en vez de 32, un literal binario de 8 por fila.
// El logo (80×48) se empaqueta igual por filas: cada una
// son 5 palabras de 16 bits, 240 palabras en total (480
// bytes frente a 3840 si cada px ocupase un byte), y una
// línea de código por fila de px.
//
// En TODO el archivo 1 = glifo y 0 = fondo, así que las tres
// funciones de lectura devuelven true = "pinta este píxel".
//
// Es un namespace de SOLO datos: no necesita instancia ni
// archivo .cpp; los sprites se leen con Sprite::pixel(part,
// x, y), Sprite::specialPixel(x, y) y Sprite::logoPixel(x, y).
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

  // ========================================================
  // Logo del arranque (80x48 px, empaquetado por fila: cada
  // fila son 80 px de 1 bit = 5 palabras de 16 bits, o sea
  // 240 palabras = 480 bytes en vez de 3840). El px (0,0) de
  // cada palabra está en el bit 15, igual que en el resto
  // de sprites.
  //
  // OJO con la polaridad: el bitmap original venía con
  // 1 = fondo y 0 = glifo (el borde de 1 px era 81.7% de
  // unos, o sea prácticamente todo fondo). Aquí se guarda
  // YA invertido, de modo que 1 = glifo y logoPixel()
  // significa lo mismo que pixel() y specialPixel():
  // true = pintar.
  // ========================================================

  constexpr uint8_t LOGO_W = 80;                          // ancho en px (5 palabras de 16 bits)
  constexpr uint8_t LOGO_H = 48;                          // alto en px (1 palabra por fila de px)
  constexpr uint8_t LOGO_WORDS = LOGO_W / 16;            // palabras de 16 bits por fila

  constexpr uint16_t LOGO[LOGO_H * LOGO_WORDS] = {
    0b0000'0000'0000'0000, 0b0000'0000'0000'0000, 0b0000'0000'0000'0000, 0b0000'0000'0000'0000, 0b0000'0000'0000'0000,  // fila 0
    0b0000'0000'0011'1100, 0b0000'0000'0001'1100, 0b0001'1000'0011'1000, 0b0000'0001'1110'0000, 0b0000'0000'1111'0000,  // fila 1
    0b0000'0000'0111'1100, 0b0000'1100'0011'1100, 0b0011'1100'0111'1000, 0b0011'0011'1110'0000, 0b0001'1111'0001'1000,  // fila 2
    0b0000'0000'1111'1110, 0b0111'1100'0011'1100, 0b0111'1100'1111'1000, 0b0111'1011'1100'0011, 0b1111'0000'0001'1000,  // fila 3
    0b0000'0001'1111'1110, 0b0111'1100'0111'1100, 0b1111'1100'0111'1000, 0b1111'1011'1000'1110, 0b0000'0000'0001'1000,  // fila 4
    0b0000'0011'1111'1100, 0b0111'1100'0111'1000, 0b1111'1100'0111'1000, 0b1111'0111'0000'1000, 0b0000'0000'0011'1000,  // fila 5
    0b0000'0011'1111'0000, 0b0111'1110'0111'1001, 0b1111'1110'0001'1001, 0b1110'1100'0000'1000, 0b0000'1100'1111'0000,  // fila 6
    0b0000'0111'1100'0000, 0b0111'1110'0111'1001, 0b1100'1110'1101'1111, 0b1110'1100'1100'1001, 0b1001'1100'1100'0000,  // fila 7
    0b0000'0111'1000'0000, 0b0111'1111'0111'1001, 0b1100'1111'1110'1111, 0b1101'1101'1100'1111, 0b1001'0100'1100'0000,  // fila 8
    0b0000'0111'0000'0000, 0b0011'1111'0111'1001, 0b1100'0111'1110'1111, 0b1001'1111'1100'0111, 0b1001'0100'1100'0000,  // fila 9
    0b0000'1111'0001'1111, 0b0011'1011'1111'1011, 0b1100'1111'1100'1111, 0b0000'1111'1000'0000, 0b1001'0100'1100'0000,  // fila 10
    0b0000'1111'1111'1111, 0b1011'1011'1111'1011, 0b1111'1111'0001'1111, 0b1000'1111'0011'0000, 0b1001'0100'1100'0000,  // fila 11
    0b0000'1111'1111'1111, 0b1011'1001'1111'0011, 0b1111'1111'0011'1111, 0b1100'0111'0111'0000, 0b1001'0100'1111'0000,  // fila 12
    0b0000'1111'1000'1111, 0b1011'1001'1111'0111, 0b1110'0111'1011'1101, 0b1110'0011'1111'0000, 0b1001'0100'1111'1000,  // fila 13
    0b0000'0000'0001'1111, 0b1011'1000'1110'0111, 0b1000'0111'1011'1100, 0b1110'0001'1110'0000, 0b1001'0100'1000'1000,  // fila 14
    0b0000'0000'0111'1111, 0b0111'1000'0000'1111, 0b1000'0000'0001'1100, 0b0110'0001'1100'0000, 0b1001'1000'0000'1000,  // fila 15
    0b0000'0000'1111'1110, 0b0110'0000'0000'1111, 0b0000'0001'1100'1100, 0b0000'0001'0000'0001, 0b1000'0000'0001'1000,  // fila 16
    0b0000'0111'1111'1100, 0b0000'0000'0000'0000, 0b0000'0011'0010'0000, 0b0000'0000'0000'0111, 0b0000'0000'1111'1000,  // fila 17
    0b0000'0111'1111'0000, 0b0000'0000'0000'0000, 0b1100'1100'1101'1111, 0b0000'0000'0000'0100, 0b0000'0011'1110'0000,  // fila 18
    0b0000'0011'1110'0000, 0b0000'0000'0000'0001, 0b0011'0001'0110'0000, 0b1000'0000'0000'0100, 0b0001'1111'0000'0000,  // fila 19
    0b0000'0011'1000'0000, 0b0000'0000'0000'0001, 0b1100'0010'0110'0000, 0b1000'0000'0000'0100, 0b1111'1000'0000'0000,  // fila 20
    0b0000'0000'0000'0000, 0b0000'0111'1111'1011, 0b0100'0111'1100'0000, 0b0100'0000'0000'0111, 0b1110'0000'0000'0000,  // fila 21
    0b0000'0000'0000'0000, 0b0011'1010'1010'1111, 0b1000'1000'0000'0000, 0b0110'0000'0000'0011, 0b1000'0000'0000'0000,  // fila 22
    0b0000'0000'0000'0001, 0b1101'0101'0101'1000, 0b0000'0000'0000'0000, 0b0011'0000'0000'0000, 0b0000'0000'0000'0000,  // fila 23
    0b0000'0000'0000'0110, 0b1010'1010'1011'1111, 0b1111'0000'0000'1111, 0b1111'1100'0000'0000, 0b0000'0000'0000'0000,  // fila 24
    0b0000'0000'0000'1101, 0b0101'0111'1101'0000, 0b0000'1001'1111'1110, 0b1010'1010'0000'0000, 0b0000'0000'0000'0000,  // fila 25
    0b0000'0000'0001'1010, 0b1011'1110'0001'1100, 0b0000'0111'1000'1011, 0b0101'0101'0000'0000, 0b0000'0000'0000'0000,  // fila 26
    0b0000'0000'0011'0101, 0b0110'0000'0000'1011, 0b1111'1011'0000'1000, 0b1011'1111'1111'0000, 0b0000'0000'0000'0000,  // fila 27
    0b0000'0000'0010'1010, 0b1100'0000'0000'1010, 0b0110'1010'0001'1000, 0b1100'0000'0000'1100, 0b0011'0000'0000'0000,  // fila 28
    0b0000'0000'0101'0111, 0b1000'0000'0000'0110, 0b0110'0110'0011'0001, 0b0000'0000'0000'0010, 0b0100'1100'0000'0000,  // fila 29
    0b0000'0000'0111'1011, 0b1000'0000'0000'0000, 0b1100'1000'0110'0010, 0b0001'1111'1110'0001, 0b1110'0010'0000'0000,  // fila 30
    0b0000'0011'1111'0101, 0b0111'1000'0000'0001, 0b0101'0000'1100'0100, 0b0010'0000'0001'0000, 0b1011'0001'0000'0000,  // fila 31
    0b0000'0110'1010'1111, 0b1011'1100'0000'0000, 0b0010'0001'1000'1000, 0b0100'0000'0000'1000, 0b1011'1001'0000'0000,  // fila 32
    0b0000'0101'0101'1000, 0b1101'0100'0000'0000, 0b0100'0011'0001'0000, 0b1000'0111'1101'1000, 0b1111'1000'1100'0000,  // fila 33
    0b0000'0111'1110'1101, 0b1111'1110'0000'0000, 0b1000'0110'0001'0001, 0b0001'1110'0111'0000, 0b1111'1100'1010'0000,  // fila 34
    0b0000'1101'0111'0111, 0b1101'0101'1000'0001, 0b0000'1100'0010'0010, 0b0011'0001'1111'1111, 0b0101'1100'0110'0000,  // fila 35
    0b0000'1010'1011'1010, 0b1010'1010'1100'0010, 0b0001'1001'1010'0010, 0b0010'1000'0000'0000, 0b0010'1110'0100'0000,  // fila 36
    0b0000'0111'1111'0101, 0b0101'1111'1110'0100, 0b0011'0011'1010'0001, 0b0011'0100'0000'0000, 0b0101'1110'0100'0000,  // fila 37
    0b0000'0000'0001'1111, 0b1010'1010'1010'1000, 0b0110'0111'1101'0000, 0b1001'0111'0000'0011, 0b1011'1110'0100'0000,  // fila 38
    0b0000'0000'0000'0111, 0b1111'0101'0110'1000, 0b0100'1111'1100'1000, 0b0100'1000'1111'1100, 0b0101'1110'0100'0000,  // fila 39
    0b0000'0001'1111'0000, 0b1011'1111'1100'1000, 0b0101'1111'1110'0100, 0b0010'0100'0000'0000, 0b0011'1110'0100'0000,  // fila 40
    0b0000'0111'1111'1100, 0b1011'1111'0101'0000, 0b1100'0111'1111'0010, 0b1001'0011'0000'0011, 0b1101'0110'0100'0000,  // fila 41
    0b0000'1111'1111'1110, 0b1100'0101'1001'0000, 0b1011'0000'0000'0001, 0b0001'1010'1111'1100, 0b1011'0010'0100'0000,  // fila 42
    0b0000'1111'1111'1110, 0b0000'1010'0001'0000, 0b0000'1111'1111'1110, 0b0000'1010'0000'0000, 0b0110'1111'1000'0000,  // fila 43
    0b0000'0111'1111'1111, 0b1100'0000'0000'1100, 0b0000'0000'0000'0000, 0b0001'1011'1000'0011, 0b1000'0000'0010'0000,  // fila 44
    0b0000'0011'1111'1111, 0b1111'1111'1110'0110, 0b0000'0000'0000'0000, 0b0001'0000'1111'1100, 0b0011'1111'1100'0000,  // fila 45
    0b0000'0000'1111'1111, 0b1111'1111'1111'0001, 0b1100'0000'0000'0001, 0b1110'0110'0000'0001, 0b1111'1111'0000'0000,  // fila 46
    0b0000'0000'0000'0001, 0b1111'1111'1111'1100, 0b0011'1111'1111'1110, 0b0000'1111'1111'1111, 0b1000'0000'0000'0000   // fila 47
  };

  // Pixel del logo del arranque (x = columna, y = fila, desde 0).
  // px (0,0) = bit 15 de la palabra 0; px (16,0) = bit 15 de la palabra 1.
  constexpr bool logoPixel(uint8_t x, uint8_t y) {
    return (LOGO[y * LOGO_WORDS + (x / 16)] & ((uint16_t)1 << (15 - (x % 16)))) != 0;
  }
}

#endif

// ====================================================================================
// Fin
// ====================================================================================
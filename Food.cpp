#include "Food.h"
#include "Game.h"

#include "Sprite.h"

#include <Adafruit_GFX.h>

// ========================================================
// Constructor
// ========================================================

Food::Food(Display& display, uint8_t cols, uint8_t rows, uint8_t top)
  : _display(display),
    _cols(cols),
    _rows(rows),
    _top(top),
    _type(Type::NORMAL),
    _x(0),
    _y(0),
    _has(false),
    _specialTime(SPECIAL_TIME_DEFAULT) {}

// ========================================================
// Inicialización: no hay alimento
// ========================================================

void Food::begin() {
  _has = false;
  _type = Type::NORMAL;
}

// ========================================================
// Colocar un alimento en una celda libre al azar.
//
// Se cuentan las celdas libres (las que `game` no reporta
// ocupadas) y se elige la pick-ésima, sin necesidad de un
// buffer: basta con recorrer la rejilla dos veces. Si no hay
// ninguna celda libre devuelve false y queda sin alimento.
// ========================================================

bool Food::spawn(Type type, const Game& game) {
  // Primer recorrido: contar las celdas libres
  uint8_t freeCells = 0;
  for (uint8_t y = 0; y < _rows; y++) {
    for (uint8_t x = 0; x < _cols; x++) {
      if (!game.occupied(x, y)) freeCells++;
    }
  }

  if (freeCells == 0) {
    _has = false;  // tablero lleno: no hay dónde poner el alimento
    return false;
  }

  // Segundo recorrido: elegir la pick-ésima celda libre
  uint8_t pick = random(freeCells);
  uint8_t seen = 0;
  bool placed = false;

  for (uint8_t y = 0; y < _rows && !placed; y++) {
    for (uint8_t x = 0; x < _cols && !placed; x++) {
      if (!game.occupied(x, y)) {
        if (seen == pick) {
          _x = x;
          _y = y;
          placed = true;
        }
        seen++;
      }
    }
  }

  _type = type;
  _has = placed;
  return placed;
}

// ========================================================
// Quitar el alimento (no hay)
// ========================================================

void Food::clear() {
  _has = false;
}

// ========================================================
// Acceso
// ========================================================

bool Food::has() const {
  return _has;
}

Food::Type Food::type() const {
  return _type;
}

uint8_t Food::x() const {
  return _x;
}

uint8_t Food::y() const {
  return _y;
}

uint8_t Food::specialTime() const {
  return _specialTime;
}

void Food::setSpecialTime(uint8_t seconds) {
  _specialTime = seconds;
}

// ========================================================
// Dibujar según el tipo
// ========================================================

void Food::draw() const {
  if (!_has) return;
  if (_type == Type::SPECIAL) drawSpecial();
  else drawNormal();
}

// ========================================================
// Alimento normal: rombo simétrico centrado en la celda
// (como el rombo seleccionado del menú)
// ========================================================

void Food::drawNormal() const {
  Adafruit_SSD1306& s = _display.screen();
  int16_t cx = (int16_t)_x * CELL + CELL / 2;
  int16_t cy = _top + (int16_t)_y * CELL + CELL / 2;

  s.fillTriangle(cx, cy - 3, cx + 3, cy, cx, cy + 3, SSD1306_WHITE);
  s.fillTriangle(cx, cy - 3, cx - 3, cy, cx, cy + 3, SSD1306_WHITE);
}

// ========================================================
// Comida especial: sprite SPECIAL_FOOD (8×4 px de 1 bit)
// dibujado a 1 px por bit, centrado verticalmente en la
// celda (la celda mide 8 px, el sprite 4)
// ========================================================

void Food::drawSpecial() const {
  Adafruit_SSD1306& s = _display.screen();
  int16_t baseX = (int16_t)_x * CELL;
  int16_t baseY = _top + (int16_t)_y * CELL
                  + (CELL - Sprite::SPECIAL_FOOD_H) / 2;

  for (uint8_t i = 0; i < Sprite::SPECIAL_FOOD_H; i++) {
    for (uint8_t j = 0; j < Sprite::SPECIAL_FOOD_W; j++) {
      if (Sprite::SPECIAL_FOOD[i][j])
        s.drawPixel(baseX + (int16_t)j, baseY + (int16_t)i, SSD1306_WHITE);
    }
  }
}

// ====================================================================================
// Fin
// ====================================================================================
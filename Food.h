#ifndef FOOD_H
#define FOOD_H

#include <Arduino.h>

// ========================================================
// Food — alimento del tablero (normal y especial)
//
// Encapsula el estado y la lógica del alimento de la
// serpiente, extraídos de Game:
//   - NORMAL: la comida común, que se dibuja como rombo
//     simétrico centrado en la celda (igual que el rombo
//     seleccionado del menú).
//   - SPECIAL: la comida especial, que se dibuja con el
//     sprite de 8×4 px (Sprite::SPECIAL_FOOD) y tiene un
//     temporizador (`specialTime`, los segundos restantes
//     que el Header muestra a la derecha). Por ahora el
//     temporizador solo es layout (valor fijo).
//
// El alimento se coloca en una celda libre al azar. La
// ocupación de las celdas la responde el tablero
// (Game::occupied): `spawn` recibe la referencia a la
// partida, así Food no conoce el tablero de antemano.
// ========================================================

class Game;  // solo para consultarle qué celdas ocupa la serpiente

class Food {
public:

  // ========================================================
  // Tipo de alimento
  // ========================================================

  enum class Type : uint8_t {
    NORMAL = 0,   // comida común (rombo centrado en la celda)
    SPECIAL       // comida especial (sprite SPECIAL_FOOD + temporizador)
  };

  // ========================================================
  // Temporizador de la comida especial
  // ========================================================

  // Segundos iniciales de la comida especial (por ahora valor
  // fijo: solo el layout del Header)
  static constexpr uint8_t SPECIAL_TIME_DEFAULT = 60;

  // ========================================================
  // Constructor (recibe la geometría del tablero: COLS x ROWS
  // celdas de 8 px a partir de la fila `top`; dibuja con la
  // Display global — Globals.h)
  // ========================================================

  Food(uint8_t cols, uint8_t rows, uint8_t top);

  // ========================================================
  // Inicialización: no hay alimento
  // ========================================================

  void begin();

  // ========================================================
  // Colocar un alimento de un tipo en una celda libre al
  // azar. La ocupación la responde el tablero (`game.occupied`).
  // Devuelve false si no hay celdas libres (tablero lleno),
  // en cuyo caso queda sin alimento (partida ganada).
  // ========================================================

  bool spawn(Type type, const Game& game);

  // ========================================================
  // Quitar el alimento (no hay)
  // ========================================================

  void clear();

  // ========================================================
  // Acceso
  // ========================================================

  bool has() const;                     // ¿hay alimento en el tablero?
  Type type() const;                    // tipo del alimento actual
  uint8_t x() const;                    // columna (0..COLS-1)
  uint8_t y() const;                    // fila (0..ROWS-1)
  uint8_t specialTime() const;          // segundos restantes de la comida especial
  void setSpecialTime(uint8_t seconds); // actualiza el temporizador de la especial

  // ========================================================
  // Dibujar según el tipo (rombo NORMAL / sprite SPECIAL)
  // ========================================================

  void draw() const;

private:

  // ========================================================
  // Geometría
  // ========================================================

  // Píxeles por celda del tablero (celdas de 8x8 px)
  static constexpr uint8_t CELL = 8;

  // ========================================================
  // Estado
  // ========================================================

  uint8_t _cols;      // columnas del tablero
  uint8_t _rows;      // filas del tablero
  uint8_t _top;       // fila superior del tablero (Body)

  Type _type;
  uint8_t _x;
  uint8_t _y;
  bool _has;
  uint8_t _specialTime;

  // ========================================================
  // Helpers de dibujo
  // ========================================================

  void drawNormal() const;   // rombo simétrico centrado en la celda
  void drawSpecial() const;  // sprite SPECIAL_FOOD (8×4 px) centrado
};

#endif

// ====================================================================================
// Fin
// ====================================================================================
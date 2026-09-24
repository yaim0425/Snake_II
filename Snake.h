#ifndef SNAKE_H
#define SNAKE_H

#include <Arduino.h>

#include "Sprite.h"

// ========================================================
// Snake — lógica pura de la serpiente (partes)
//
// Extraída de Game (la ventana del juego): encapsula TODO lo
// que es "la serpiente" sin tocar el hardware:
//   - Buffer circular de segmentos (`Seg body[MAX_LENGTH]`):
//     cada uno guarda su posición, su dirección (`dir`, hacia
//     el segmento siguiente, más cerca de la cabeza) y su
//     sprite persistente (`part`).
//   - Dirección COMMITIDA (`_dir`) y giro pendiente (`_nextDir`):
//     un único giro PENDIENTE, sin cola ni buffer, validado
//     contra la dirección actual (sin reversa directa: 180° se
//     ignora; al girar varias veces entre dos pasos la cabeza no
//     puede volverse sobre la dirección con la que avanzará).
//   - Paso del tablero (`step`): mover la cabeza con wrap en X e
//     Y, colisión con el cuerpo (la cola se libera si no come),
//     comer/crecer y paleta de sprites (BODY/CORNER/BELLY).
//   - Sprites de las partes (cola, cuerpo, curvas, panza) y de la
//     cabeza (boca abierta/cerrada según el alimento adyacente):
//     la parte de cada segmento se deriva de la geometría de sus
//     vecinos y la dirección de la cabeza.
//
// NO depende de Display, Sound ni Food: el dibujo, los efectos y
// el alimento los coordina Game, que usa los resultados de `step`
// (MOVED/ATE/DIED) y los segmentos (`length`/`segment`) para
// volcar la serpiente a la pantalla. Al ser de solo lógica se
// puede probar en el PC sin el resto del hardware (mismo núcleo
// que se validó antes de escribir Game).
//
// Nota sobre `compactar`: la casilla que la cabeza deja se
// convierte en cuerpo con su sprite persistente, según por qué
// lado entra y sale la tubería (recto BODY_TO_<dir> o esquina
// CORNER_<horizontal>_<vertical>); al comer, la panza (BELLY,
// recta o curva) queda guardada en el segmento y viaja con el
// cuerpo hasta que la cola lo borra.
// ========================================================

class Snake {
public:

  // ========================================================
  // Configuración (geometría del tablero)
  // ========================================================

  static constexpr uint8_t COLS = 16;               // columnas del tablero (16 celdas de 8 px = 128 px)
  static constexpr uint8_t ROWS = 6;                // filas del tablero (6 celdas de 8 px = 48 px, Body)
  static constexpr uint8_t MAX_LENGTH = COLS * ROWS; // 96: a lo sumo hay tantas celdas como segmentos

  // ========================================================
  // Direcciones. El orden (UP=1, RIGHT, DOWN, LEFT) coincide
  // con el orden de los sprites por dirección
  // (TAIL_TO_UP/BODY_TO_UP/HEAD_UP_...) para indexarlos con
  // (dir - 1).
  // ========================================================

  enum class Dir : uint8_t {
    NONE = 0, UP = 1, RIGHT, DOWN, LEFT
  };

  // ========================================================
  // Resultado de un paso (para que Game coordine el resto)
  // ========================================================

  enum class Result : uint8_t {
    MOVED,   // la cabeza avanzó sin comer
    ATE,     // la cabeza avanzó y comió (creció)
    DIED     // colisión con el cuerpo: la cabeza NO avanzó
  };

  // ========================================================
  // Segmento del ring buffer
  // ========================================================

  struct Seg {
    uint8_t x;
    uint8_t y;
    Dir dir;                 // dirección hacia el siguiente segmento (más cerca de la cabeza)
    Sprite::Part part;       // sprite persistente del segmento (TAIL, BODY, CORNER o BELLY)
  };

  // ========================================================
  // Constructor (no toca hardware; el tablero se arma en reset())
  // ========================================================

  Snake();

  // ========================================================
  // Reinicio: serpiente inicial horizontal (1,2)..(4,2),
  // cabeza a la derecha
  // ========================================================

  void reset();

  // ========================================================
  // Cancela el giro pendiente (al entrar en la ventana)
  // ========================================================

  void clearPending();

  // ========================================================
  // Giro de la cabeza (MOVE). `d` queda PENDIENTE (`_nextDir`)
  // si es válido desde la dirección COMMITIDA (`_dir`): para
  // `d` contraria (180°) se ignora y devuelve `false`.
  // Devuelve `true` si el giro quedó pendiente (Game toca
  // SFX_TURN solo en ese caso).
  // ========================================================

  bool turn(Dir d);

  // ========================================================
  // Un paso del tablero: aplicar el giro pendiente, mover la
  // cabeza (wrap), detectar colisión, moverse/crecer y derivar
  // los sprites de las partes. `fx`/`fy` = celda del alimento
  // (para comer/crecer y para decidir si la cola es bloqueante).
  // Devuelve el resultado para que Game coordine (puntaje,
  // sonido, alimento, GAME OVER).
  // ========================================================

  Result step(uint8_t fx, uint8_t fy);

  // ========================================================
  // ¿Una celda está ocupada por la serpiente? (lo consulta el
  // alimento para generarse en una celda libre; Food usa
  // Game::occupied, que delega aquí)
  // ========================================================

  bool occupied(uint8_t x, uint8_t y) const;

  // ========================================================
  // Acceso para dibujar (lo consume Game)
  // ========================================================

  uint8_t length() const;                 // cantidad de segmentos
  const Seg& segment(uint8_t index) const; // segmento `index` (0 = cola, length()-1 = cabeza)
  Sprite::Part headPart(bool hasFood, uint8_t fx, uint8_t fy) const; // sprite de la cabeza (boca según el alimento adyacente)

private:

  // ========================================================
  // Métodos internos
  // ========================================================

  uint8_t slot(uint8_t index) const;             // índice del ring buffer para el segmento `index`
  Dir opposite(Dir d) const;                     // dirección opuesta (RIGHT<->LEFT, UP<->DOWN)
  Sprite::Part bodyPartFor(Dir in, Dir out) const;  // BODY recto o CORNER al girar
  Sprite::Part bellyPartFor(Dir in, Dir out) const; // BELLY recto o curvo (al comer)

  // ========================================================
  // Estado (ring buffer: los segmentos van de _tailIx a _headIx)
  // ========================================================

  Seg _body[MAX_LENGTH];
  uint8_t _headIx;
  uint8_t _tailIx;
  uint8_t _length;

  Dir _dir;                // dirección COMMITIDA de la cabeza (la que usará en el
                           // próximo paso); solo cambia en step() al aplicar el giro pendiente
  Dir _nextDir;            // único giro pendiente ("el siguiente"): se valida contra `_dir` (la
                           // cabeza no puede volver sobre la dirección con la que avanzará)
  bool _bellyPending;      // la cabeza está sobre la casilla de la comida: al dejarla se pinta BELLY
};

#endif

// ====================================================================================
// Fin
// ====================================================================================
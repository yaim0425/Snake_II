#ifndef GAME_H
#define GAME_H

#include "Display.h"
#include "Buttons.h"
#include "Food.h"
#include "Sound.h"
#include "Sprite.h"

// ========================================================
// Game — ventana del juego de la serpiente
//
// Reemplaza al placeholder InfoWindow en los estados
// NUEVO ("New") y CONTINUAR ("Continue") del Engine.
//
//   - Tablero: rejilla de COLS x ROWS celdas de 8x8 px en el
//     Body (filas 16..63). La serpiente sale por un borde y
//     reaparece por el opuesto (wrap en X y en Y, estilo
//     Nokia del juego original).
//   - Sprites: los 4x4 px de Sprite, dibujados con
//     píxel doble (2x2 px) para ocupar la celda de 8x8. Cada
//     segmento del cuerpo guarda su DIRECCIÓN y su SPRITE
//     persistente: BODY recto, CORNER al girar y BELLY al
//     comer (la panza queda en la casilla de la comida cuando
//     la cabeza la deja). El cuerpo NO se mueve: cada paso
//     agrega una parte nueva delante y elimina la última (cola);
//     solo la cabeza recalcula su sprite (boca abierta 1
//     casilla antes de la comida, cerrada al colisionar).
//   - Estados: START (conteo regresivo 3-2-1), PLAY (se mueve
//     cada _moveDelay ms según la dificultad), PAUSE y
//     GAME_OVER.
//   - Controles: MOVE deja un único giro PENDIENTE (`_nextDir`)
//     validado contra la dirección COMMITIDA (`_dir`): desde esa
//     dirección la cabeza solo tiene 3 posibilidades (seguir,
//     girar a la izquierda, girar a la derecha) y la contraria
//     (180°) se ignora; el giro pendiente se aplica en el
//     siguiente paso (sin reversa directa).
//     ACTION_RIGHT (Btn2) es "Select / Pause": en PLAY pausa
//     (panel "PAUSA") y en PAUSE/CONTINUAR reanuda (ACTION_LEFT
//     también reanuda). ACTION_UP (Btn1, "Volver") vuelve al menú
//     en cualquier momento (la partida NO se pierde mientras haya
//     puntos: el menú muestra "Continue" al volver solo si la partida
//     sigue en curso Y tiene score > 0; sin puntos se oculta);
//     en GAME_OVER cualquier ACTION vuelve al menú.
// ========================================================

class Game {
public:

  // ========================================================
  // Configuración
  // ========================================================

  static constexpr uint8_t COLS = 16;              // columnas del tablero (16 celdas de 8 px = 128 px)
  static constexpr uint8_t ROWS = 6;               // filas del tablero (6 celdas de 8 px = 48 px, Body)
  static constexpr uint8_t MAX_LENGTH = COLS * ROWS;  // 96: a lo sumo hay tantas celdas como segmentos

  static constexpr uint8_t DIFICULTAD_MIN = 1;
  static constexpr uint8_t DIFICULTAD_MAX = 25;
  static constexpr uint8_t DIFICULTAD_DEFAULT = 13;

  // ========================================================
  // Constructor (recibe Display, Buttons y Sound compartidos)
  // ========================================================

  Game(Display& display, Buttons& buttons, Sound& sound);

  // ========================================================
  // Inicialización (al entrar en la ventana)
  //
  // `newGame` = true (NUEVO): reinicia todo y arranca el
  // conteo regresivo 3-2-1. `false` (CONTINUAR): reanuda la partida
  // anterior en pausa, o arranca una nueva si no hay partida
  // en curso.
  // ========================================================

  void begin(bool newGame);

  // ========================================================
  // Dificultad del menú (1..25). Se aplica a la velocidad
  // (ms por paso) AL INSTANTE, incluso con la partida ya
  // iniciada: al cambiar el nivel, _moveDelay se recalcula en
  // caliente y la partida en curso (PLAY o PAUSE) sigue el nuevo
  // ritmo; también vale para la próxima partida nueva.
  // ========================================================

  void setDifficulty(uint8_t level);

  // ========================================================
  // Actualizar (consume los eventos de botones leídos en loop())
  // ========================================================

  void update();

  // ========================================================
  // Dibujar
  // ========================================================

  void print();

  // ========================================================
  // Salida (true = pidió volver al menú)
  // ========================================================

  bool done() const;

  // ========================================================
  // ¿La partida terminó en GAME OVER? Lo usa el Engine al salir
  // para dejar la selección del menú en "New" (Game Over) o
  // "Continue" (salida con la partida aún en curso).
  // ========================================================

  bool isGameOver() const;

  // ========================================================
  // Puntajes (el Engine sincroniza el récord con el menú)
  // ========================================================

  uint16_t score() const;
  uint16_t bestScore() const;

  // ========================================================
  // ¿Una celda está ocupada por la serpiente? Lo consulta
  // Food (al colocar el alimento en una celda libre).
  // ========================================================

  bool occupied(uint8_t x, uint8_t y) const;

private:

  // ========================================================
  // Estado del juego
  // ========================================================

  enum class State : uint8_t {
    START,       // conteo regresivo 3-2-1 (el tablero ya está dibujado)
    PLAY,        // la serpiente se mueve cada _moveDelay ms
    PAUSE,       // panel "PAUSA" sobre el tablero estático
    GAME_OVER    // panel "GAME OVER"; cualquier ACTION vuelve al menú
  };

  // Direcciones del tablero. El orden (UP=1, RIGHT, DOWN, LEFT)
  // coincide con el orden de los sprites por dirección
  // (TAIL_TO_UP/BODY_TO_UP/HEAD_UP_...) para indexarlos con
  // (dir - 1).
  enum class Dir : uint8_t {
    NONE = 0, UP = 1, RIGHT, DOWN, LEFT
  };

  struct Seg {
    uint8_t x;
    uint8_t y;
    Dir dir;                 // dirección hacia el siguiente segmento (más cerca de la cabeza)
    Sprite::Part part; // sprite persistente del segmento (TAIL, BODY, CORNER o BELLY)
  };

  // ========================================================
  // Geometría y tiempos
  // ========================================================

  static constexpr int16_t BODY_TOP = 16;          // fila superior del tablero (Body)
  static constexpr uint32_t COUNTDOWN_MS = 3000;   // duración del conteo regresivo inicial (3 s, uno por dígito)
  static constexpr uint32_t COUNT_HIDE_MS = 250;   // al final de cada dígito: el número (y su cuadro) se ocultan antes del siguiente (parpadeo)

  // ========================================================
  // Métodos internos
  // ========================================================

  // Arranque/reanudación
  void reset();
  void startPlay();

  // Lógica
  void handleTurn();              // MOVE deja el giro pendiente (sin reversa directa)
  void turn(Dir d);               // fija `_nextDir` si `d` es válido desde la dirección COMMITIDA (_dir)
  void step();                    // un paso del tablero (aplicar el giro pendiente, mover, comer, crecer, morir)
  void die();                     // colisión o tablero lleno

  // Render
  uint8_t slot(uint8_t index) const;             // índice del ring buffer para el segmento `index`
  Dir opposite(Dir d) const;                     // dirección opuesta (RIGHT<->LEFT, UP<->DOWN)
  Sprite::Part headPart() const;           // sprite de la cabeza (boca según la comida adyacente)
  Sprite::Part bodyPartFor(Dir in, Dir out) const;  // BODY recto o CORNER al girar
  Sprite::Part bellyPartFor(Dir in, Dir out) const; // BELLY recto o curvo (al comer)
  void drawSprite(Sprite::Part part, uint8_t x, uint8_t y);
  void drawSnake();
  void drawHeader();
  void drawOverlay(const char* title, bool fullWidth = false);

  // Velocidad según dificultad (1..25): cuanto más alta, más rápido
  uint16_t speedFor(uint8_t level) const;

  // ========================================================
  // Dependencias
  // ========================================================

  Display& _display;
  Buttons& _buttons;
  Sound& _sound;

  // ========================================================
  // Estado
  // ========================================================

  State _state;
  bool _exit;          // pidió volver al menú
  bool _redraw;        // primer frame tras begin(): clear() completo + estáticos
  bool _redrawHeader;  // el Header (puntaje/récord) cambió
  bool _dirtyBoard;    // hay que volcar el tablero (movimiento, comida, overlay)
  bool _overlayHidden; // el conteo está en su fase de parpadeo (cuadro + dígito ocultos)
  uint8_t _lastCount;  // último dígito del conteo mostrado (0xFF = ninguno aún: pitido al aparecer el 3)

  // Partida
  bool _hasGame;          // hay una partida en curso (para "Continue")
  uint8_t _difficulty;    // nivel del menú (1..25, default 13)
  uint16_t _moveDelay;    // ms por paso, derivado de la dificultad
  uint32_t _moveLast;     // instante del último paso
  uint32_t _startMs;      // instante de entrada a START

  // Serpiente (ring buffer: los segmentos van de _tailIx a _headIx)
  Seg _body[MAX_LENGTH];
  uint8_t _headIx;
  uint8_t _tailIx;
  uint8_t _length;

  Dir _dir;                // dirección COMMITIDA de la cabeza (la que usará en el
                           // próximo paso); solo cambia en step() al aplicar el giro pendiente
  Dir _nextDir;            // único giro pendiente ("el siguiente"): se valida contra `_dir` (la
                           // cabeza no puede volver sobre la dirección con la que avanzará)
  bool _bellyPending;      // la cabeza está sobre la casilla de la comida: al dejarla se pinta BELLY

  // Alimento y puntaje
  Food _food;          // alimento del tablero (normal / especial) en la clase Food
  uint16_t _score;
  uint16_t _bestScore;
};

#endif

// ====================================================================================
// Fin
// ====================================================================================
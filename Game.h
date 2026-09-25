#ifndef GAME_H
#define GAME_H

#include "Config.h"

#include "Food.h"
#include "Snake.h"

// ========================================================
// Game — ventana del juego de la serpiente
//
// Reemplaza al placeholder InfoWindow en los estados
// NEW ("New") y CONTINUE ("Continue") del Engine.
//
// Game COORDINA: la lógica de la serpiente (buffer circular,
// giro pendiente, colisiones, elección de sprites) vive en la
// clase Snake, que no toca Display/Sound/Food; Game decide el
// ritmo (dificultad), lee los botones, llama `snake.step()`,
// gestiona el alimento (Food), toca los efectos y dibuja el
// tablero con los segmentos que Snake expone.
//
//   - Tablero: rejilla de Snake::COLS x Snake::ROWS celdas de
//     8x8 px en el Body (filas 16..63). La serpiente sale por
//     un borde y reaparece por el opuesto (wrap en X y en Y,
//     estilo Nokia del juego original).
//   - Sprites: los 4x4 px de Sprite, dibujados con píxel doble
//     (2x2 px) para ocupar la celda de 8x8. La parte de cada
//     segmento (cola/cuerpo/curva/cabeza) la deriva Snake de la
//     geometría de sus vecinos y de la dirección de la cabeza.
//   - Estados: START (conteo regresivo 3-2-1), PLAY (se mueve
//     cada _moveDelay ms según la dificultad), PAUSE y
//     GAME_OVER.
//   - Controles: MOVE deja un único giro pendiente validado
//     contra la dirección commitida (sin reversa directa, lo
//     decide Snake::turn). ACTION_RIGHT (Btn2) es "Select /
//     Pause": en PLAY pausa (panel "PAUSA") y en
//     PAUSE/CONTINUE reanuda (ACTION_LEFT también reanuda).
//     ACTION_UP (Btn1, "Volver") vuelve al menú en cualquier
//     momento (la partida NO se pierde mientras haya puntos: el
//     menú muestra "Continue" al volver solo si la partida sigue
//     en curso Y tiene score > 0; sin puntos se oculta); en
//     GAME_OVER cualquier ACTION vuelve al menú. Al ESTABLECER
//     UN NUEVO RÉCORD (bestScore), en vez del letrero estático
//     se muestra en ciclo "GAME OVER" → "BUT" → "YOU ARE" →
//     "THE BEST" (cada letrero NEW_BEST_SIGN_MS) hasta que se
//     presiona un botón; la fanfarria (SFX_NEW_BEST) suena solo
//     la PRIMERA vez que aparece el letrero "THE BEST".
// ========================================================

class Game {
public:

  // ========================================================
  // Configuración
  // ========================================================

  // La dificultad (nivel 1..10) usa los límites compartidos de
  // Config::Difficulty (MIN_LEVEL/MAX_LEVEL/DEFAULT_LEVEL), igual que el menú.

  // ========================================================
  // Constructor (sin parámetros de servicios: usa los globales
  // Display, Buttons y Sound — Globals.h)
  // ========================================================

  Game();

  // ========================================================
  // Inicialización (al entrar en la ventana)
  //
  // `newGame` = true (NEW): reinicia todo y arranca el
  // conteo regresivo 3-2-1. `false` (CONTINUE): reanuda la
  // partida anterior en pausa, o arranca una nueva si no hay
  // partida en curso.
  // ========================================================

  void begin(bool newGame);

  // ========================================================
  // Dificultad del menú (1..10). Se aplica a la velocidad
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
  // Food (al colocar el alimento en una celda libre). Delega
  // en Snake.
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
    GAME_OVER    // panel "GAME OVER"; al batir el récord muestra el ciclo BUT/YOU ARE/THE BEST; cualquier ACTION vuelve al menú
  };

  // ========================================================
  // Geometría y tiempos
  // ========================================================
  // La fila superior del tablero es el Body: Config::Screen::BODY_TOP.

  static constexpr uint32_t COUNTDOWN_MS = 3000;   // duración del conteo regresivo inicial (3 s, uno por dígito)
  static constexpr uint32_t COUNT_HIDE_MS = 250;   // al final de cada dígito: el número (y su cuadro) se ocultan antes del siguiente (parpadeo)
  static constexpr uint32_t NEW_BEST_SIGN_MS = 1500; // duración de cada letrero del festejo de nuevo récord ("GAME OVER"/"BUT"/"YOU ARE"/"THE BEST")

  // ========================================================
  // Métodos internos
  // ========================================================

  // Arranque/reanudación
  void reset();
  void startPlay();

  // Lógica (coordinación; la serpiente la resuelve en Snake)
  void handleTurn();              // lee MOVE y llama snake.turn() (tocando SFX_TURN si aceptó)
  void step();                    // un paso: snake.step() + comida/puntaje/sonidos del resultado
  void die();                     // colisión(GAME OVER) o tablero lleno

  // Render
  void drawSprite(Sprite::Part part, uint8_t x, uint8_t y);
  void drawSnake();
  void drawHeader();
  void drawOverlay(const char* title, bool fullWidth = false);

  // Velocidad según dificultad (1..10): cuanto más alta, más rápido
  uint16_t speedFor(uint8_t level) const;

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
  uint8_t _difficulty;    // nivel del menú (1..10, default 5)
  uint16_t _moveDelay;    // ms por paso, derivado de la dificultad
  uint32_t _moveLast;     // instante del último paso
  uint32_t _startMs;      // instante de entrada a START
  uint32_t _gameOverMs;   // instante del fin de partida (arranca el festejo de récord)

  // Festejo de nuevo récord (GAME OVER con _newBest): ciclo de letreros
  // "GAME OVER"/"BUT"/"YOU ARE"/"THE BEST" hasta que se presiona un botón
  bool _newBest;    // este fin de partida estableció un nuevo récord
  uint8_t _celeSfx; // flag: 0 = la fanfarria aún no sonó en "THE BEST";
                    // al sonar pasa a 1 (suena solo la primera vez que
                    // aparece ese letrero del festejo)

  // Serpiente (lógica pura, sin Display/Sound/Food) y alimento
  Snake _snake;
  Food _food;          // alimento del tablero (normal / especial) en la clase Food
  uint16_t _score;
  uint16_t _bestScore;
};

#endif

// ====================================================================================
// Fin
// ====================================================================================
#ifndef MENU_DIFFICULTY_H
#define MENU_DIFFICULTY_H

#include "Config.h"
#include "Timer.h"

// ========================================================
// MenuDifficulty — ventana de edición de la dificultad
//
// Selector de nivel 1..10 al que se entra desde la opción
// "Difficulty" del Menu (Menu::OPT_DIFFICULTY). Es una ventana
// más del Engine, como MenuCredits: el Menu deja de editar la
// dificultad inline y se limita a devolver la opción elegida.
//
// No borra la pantalla: entra sobre el Menu ya dibujado y solo
// borra la banda de rombos (45..53), la misma franja que el
// selector inline ocupaba, para escribir en ella "< N >". Al
// salir es el Menu el que restaura esa franja con sus rombos
// (Menu::restoreDiamondBand), así que el resto de la ventana
// (título, cuadro de la opción, pie) nunca se toca.
//
// El valor vive aquí y es persistente: `difficulty()` lo lee el
// Engine al arrancar la partida, igual que antes lo leía
// Menu::difficulty(). Los límites y el nivel por defecto vienen
// de Config::Difficulty.
// ========================================================

class MenuDifficulty {
public:

  // ========================================================
  // Constructor (sin parámetros de servicios: usa los globales
  // Display, Buttons y Sound — Globals.h)
  // ========================================================

  MenuDifficulty();

  // ========================================================
  // Inicialización (no dibuja nada: solo deja el valor en
  // edición y ancla el parpadeo; el dibujo es de print())
  // ========================================================

  void begin();

  // ========================================================
  // Actualizar (consume los eventos de botones leídos en loop())
  // ========================================================

  void update();

  // ========================================================
  // Dibujar (borra la banda 45..53 y escribe el selector)
  // ========================================================

  void print();

  // ========================================================
  // Estado
  // ========================================================

  bool done() const;

  // Nivel confirmado (1..Config::Difficulty::MAX_LEVEL): el que
  // se aplica a la partida. No cambia hasta confirmar con
  // ACTION_RIGHT; ACTION_UP cancela y lo deja como estaba.
  uint8_t difficulty() const;

private:

  // ========================================================
  // Geometría
  // ========================================================

  // La banda es la de los rombos del Menu (DIA_TOP/DIA_SIZE de
  // Menu): las filas 45..53. Ojo a la última: la 53 es la línea
  // separadora del pie (Config::Screen::FOOT_LINE), así que el erase
  // se lleva por delante esa fila y el selector escribe encima
  // (46..53). Por eso Menu::restoreDiamondBand() tiene que repintar
  // también la línea al volver.
  static constexpr int16_t SEL_TOP  = 45;
  static constexpr uint8_t SEL_SIZE = 8;

  // Número centrado con ancho constante: 1 dígito lleva un
  // espacio inicial (" 5" mide lo mismo que "10", 12 px) para que
  // el centrado no se desplace al pasar de 9 a 10.
  static constexpr int16_t SEL_TEXT_TOP = SEL_TOP + 1;  // 46
  static constexpr int16_t SEL_TEXT_MID = SEL_TOP + SEL_SIZE / 2;  // 49
  static constexpr int16_t SEL_TEXT_BOT = SEL_TOP + SEL_SIZE;      // 53

  // Flechas: grosor horizontal, hueco con el número, período de
  // parpadeo (visible 75%, oculto el 25% inicial) y las dos
  // constantes de repetición al mantener.
  static constexpr int16_t ARROW_GAP            = 6;    // hueco (px) entre el número y la flecha
  static constexpr int16_t ARROW_W              = 6;    // grosor horizontal de la flecha (px)
  static constexpr uint32_t ARROW_BLINK_PERIOD  = 500;  // período del parpadeo de las flechas (ms)
  static constexpr uint8_t  ARROW_BLINK_OFF_PCT = 25;   // % del período en que la flecha está oculta
  static constexpr uint32_t HOLD_REPEAT_DELAY   = 400;  // mantener para empezar a repetir (ms)
  static constexpr uint32_t HOLD_REPEAT_TICK    = 100;  // intervalo de repetición mientras se mantiene (ms)

  // ========================================================
  // Métodos internos
  // ========================================================

  void drawSelector();

  // Repetición por mantención: true cuando hay que aplicar el
  // paso del botón. El primero es inmediato (evento pressed) y,
  // manteniéndolo presionado, los siguientes cada HOLD_REPEAT_TICK
  // ms a partir de HOLD_REPEAT_DELAY de mantención. Dos Stopwatch
  // compartidos (`_repeat` = retardo, `_repeatTick` = cadencia):
  // si se suelta y se vuelve a presionar, pressed() reinicia ambos.
  bool holdRepeat(uint8_t button);

  // ========================================================
  // Estado
  // ========================================================

  uint8_t _difficulty;   // nivel confirmado, persistente entre entradas
  uint8_t _edit;         // nivel en edición (se aplica al confirmar)
  bool _done;            // confirmó o canceló: el Engine vuelve al Menu

  Stopwatch _blink;      // ancla el parpadeo de las flechas
  Stopwatch _repeat;     // inicio de la mantención (repetición por hold)
  Stopwatch _repeatTick; // último paso de la repetición
};

#endif

// ====================================================================================
// Fin
// ====================================================================================

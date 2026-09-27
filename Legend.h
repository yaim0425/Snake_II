#ifndef LEGEND_H
#define LEGEND_H

#include <Arduino.h>
#include "Timer.h"
#include "Config.h"

// ========================================================
// Legend — panel de botones (leyenda)
//
// Muestra la disposición de los botones:
//   - Izquierda: MOVE, un rombo de 4 flechas.
//   - Derecha:   ACTION, 4 rombos de posición colocados en
//     las posiciones de un pad direccional (↑ → ↓ ←), que
//     parpadean MUY rápido uno a la vez recorriéndolos en
//     ciclo lento y automático. El texto del pie indica la
//     función del rombo activo (array BTN[]):
//       Btn1 (↑ = ACTION_UP):    "Btn1: Back"
//       Btn2 (→ = ACTION_RIGHT): "Btn2: Select / Pause"
//       Btn3 (↓ = ACTION_DOWN):  "Btn3: None"
//       Btn4 (← = ACTION_LEFT):  "Btn4: None"
//
// Se muestra al arranque (después de la animación Boot) y al
// volver al menú desde cualquier ventana. Cualquier botón la
// cierra (done() = true) y Engine pasa al menú; el sonido
// depende del botón presionado: MOVE = SFX_CLICK,
// ACTION_UP (Back) = SFX_BACK, ACTION_RIGHT (Select / Pause) =
// SFX_CONFIRM, ACTION_DOWN/ACTION_LEFT (None) = SFX_CLICK.
// ========================================================

class Legend {
public:
  // ========================================================
  // Constructor (sin parámetros: usa los servicios globales
  // Display, Buttons y Sound, declarados en Globals.h)
  // ========================================================

  Legend();

  // ========================================================
  // Inicialización (al entrar en la ventana)
  // ========================================================

  void begin();

  // ========================================================
  // Actualizar (consume eventos de botones ya leídos y avanza el ciclo de parpadeo)
  // ========================================================

  void update();

  // ========================================================
  // Dibujar (pad MOVE + rombos de ACTION + texto del pie)
  // ========================================================

  void print();

  // ========================================================
  // Salida (true = se pidió ir al menú)
  // ========================================================

  bool done() const;

private:
  // ========================================================
  // Geometría del pad MOVE (rombo de 4 flechas)
  // ========================================================

  static constexpr int16_t CY = 32;                                    // centro vertical de ambos pads
  static constexpr int16_t R = 12;                                     // radio del rombo (centro-flecha)
  static constexpr int16_t PAD_MOVE_X = Config::Screen::WIDTH * 0.25;  // centro del pad MOVE

  // Rótulo sobre el pad MOVE
  static constexpr int16_t SIGN_Y = 4;

  // ========================================================
  // Rombos de posición del pad ACTION (mitad derecha)
  // ========================================================

  static constexpr uint8_t DIA_SIZE = 8;                              // rombo completo (SIEMPRE rombo)
  static constexpr int16_t DIA_PAD_X = Config::Screen::WIDTH * 0.75;  // centro del pad de rombos
  static constexpr int16_t DIA_R = 12;                                // radio del pad (centro-rombo)

  // Texto del pie de cada rombo activo, "BtnN: función" (Btn1..Btn4)
  static const char* const BTN[4];

  // ========================================================
  // Animación del rombo activo (ciclo lento + parpadeo rápido)
  // ========================================================

  static constexpr uint32_t HOLD_MS = 900;       // visible fija antes de parpadear
  static constexpr uint32_t DWELL_MS = 2200;     // duración total por rombo (avance lento)
  static constexpr uint32_t BLINK_PERIOD = 100;  // período del parpadeo MUY rápido (ms)
  static constexpr uint8_t BLINK_OFF_PCT = 50;   // % del período en que está oculto

  // ========================================================
  // Estado
  // ========================================================

  bool _done;
  uint8_t _step;     // rombo activo (0..3): recorre Btn1 → Btn4
  Ticker _ticker;    // avance de 1 paso cada DWELL_MS
  Stopwatch _timer;  // desde que se fijó el rombo activo (hold + parpadeo)
  bool _visible;     // rombo activo visible (parpadeo)
  bool _redraw;      // primer frame tras begin(): clear() completo + estáticos
  int8_t _lastText;  // paso cuyo texto del pie está en pantalla (-1 = ninguno);
                     // al cambiar es además el paso anterior, cuyo rombo hay
                     // que dejar completo (puede haber quedado borrado)

  // ========================================================
  // Helpers de dibujo
  // ========================================================

  // Estáticos del primer frame (rótulos, pad MOVE, 4 rombos y pie); lo llama print()
  void firstPrint();

  // Flecha sólida (0 = ↑, 1 = →, 2 = ↓, 3 = ←), centrada en (cx, cy)
  void drawArrow(uint8_t dir, int16_t cx, int16_t cy);

  // Rombo de 4 flechas centrado en (cx, cy)
  void drawPad(int16_t cx);

  // Posición del rombo i (0 = ↑, 1 = →, 2 = ↓, 3 = ←)
  void diamondCenter(uint8_t i, int16_t& cx, int16_t& cy) const;

  // Rombo completo de DIA_SIZE centrado en (cx, cy)
  void drawDiamond(int16_t cx, int16_t cy);
};

#endif

// ====================================================================================
// Fin
// ====================================================================================
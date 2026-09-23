#ifndef LEGEND_H
#define LEGEND_H

#include "Display.h"
#include "Buttons.h"
#include "Sound.h"

// ========================================================
// Legend — panel de botones (leyenda)
//
// Muestra la disposición de los botones:
//   - Izquierda: MOVE, un rombo de 4 flechas.
//   - Derecha:   ACTION, 4 rombos de posición colocados en
//     las posiciones de un pad direccional (↑ → ↓ ←), que
//     parpadean MUY rápido uno a la vez recorriéndolos en
//     ciclo lento y automático. El texto del pie indica la
//     función del rombo activo:
//       Btn1 (↑ = ACTION_UP):    "Back / Pause"
//       Btn2 (→ = ACTION_RIGHT): "Select"
//       Btn3 (↓ = ACTION_DOWN):  "None"
//       Btn4 (← = ACTION_LEFT):  "None"
//
// Se muestra al arranque (después de la animación Boot) y al
// volver al menú desde cualquier ventana. Cualquier botón la
// cierra (done() = true) y Engine pasa al menú; el sonido
// depende del botón presionado: MOVE = SFX_CLICK,
// ACTION_UP (Back/Pause) = SFX_BACK, ACTION_RIGHT (Select) =
// SFX_CONFIRM, ACTION_DOWN/ACTION_LEFT (None) = SFX_CLICK.
// ========================================================

class Legend {
public:
  // ========================================================
  // Constructor
  // ========================================================

  Legend(Display& display, Buttons& buttons, Sound& sound);

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

  static constexpr int16_t CY = 32;          // centro vertical de ambos pads
  static constexpr int16_t R  = 12;          // radio del rombo (centro-flecha)
  static constexpr int16_t PAD_MOVE_X = 30;  // centro del pad MOVE

  // Rótulo sobre el pad MOVE
  static constexpr int16_t SIGN_Y = 0;

  // ========================================================
  // Rombos de posición del pad ACTION (mitad derecha)
  // ========================================================

  static constexpr uint8_t DIA_SIZE = 8;     // rombo completo (SIEMPRE rombo)
  static constexpr int16_t DIA_PAD_X = 96;   // centro del pad de rombos
  static constexpr int16_t DIA_R     = 12;   // radio del pad (centro-rombo)

  // Textos del pie: identificador y función de cada rombo (Btn1..Btn4)
  static const char* const BTN_NAME[4];
  static const char* const BTN_FUNC[4];

  // ========================================================
  // Animación del rombo activo (ciclo lento + parpadeo rápido)
  // ========================================================

  static constexpr uint32_t HOLD_MS       = 900;   // visible fija antes de parpadear
  static constexpr uint32_t DWELL_MS      = 2200;  // duración total por rombo (avance lento)
  static constexpr uint32_t BLINK_PERIOD  = 100;   // período del parpadeo MUY rápido (ms)
  static constexpr uint8_t  BLINK_OFF_PCT = 50;    // % del período en que está oculto

  // ========================================================
  // Pie del Body (mismo diseño que el menú)
  // ========================================================

  static constexpr int16_t PIE_LINE_ROW = 54;  // línea separadora
  static constexpr int16_t PIE_TOP      = 57;  // texto centrado (función del rombo activo)

  // ========================================================
  // Dependencias y estado
  // ========================================================

  Display& _display;
  Buttons& _buttons;
  Sound& _sound;

  bool _exit;
  uint8_t _selected;  // rombo activo (0..3): recorre Btn1 → Btn4
  uint32_t _setTime;  // momento en que se fijó el rombo activo
  bool _redraw;       // primer frame tras begin(): clear() completo + estáticos
  int8_t _lastActive; // último rombo cuya zona se gestionó (para restaurar el que deja de ser activo)
  int8_t _lastText;   // texto del pie que se dibujó (para borrar/redibujar solo al cambiar)

  // ========================================================
  // Helpers de dibujo
  // ========================================================

  // Flecha sólida (0 = ↑, 1 = →, 2 = ↓, 3 = ←), centrada en (cx, cy)
  void drawArrow(uint8_t dir, int16_t cx, int16_t cy);

  // Rombo de 4 flechas centrado en (cx, cy)
  void drawPad(int16_t cx);

  // Posición del rombo i (0 = ↑, 1 = →, 2 = ↓, 3 = ←)
  void diamondCenter(uint8_t i, int16_t& cx, int16_t& cy) const;

  // Rombo completo de DIA_SIZE centrado en (cx, cy); si show es false no se dibuja
  void drawDiamond(int16_t cx, int16_t cy, bool show);

  // ¿El rombo activo está visible? (fijo durante HOLD_MS, luego parpadeo rápido)
  bool blinkVisible() const;
};

#endif

// ====================================================================================
// Fin
// ====================================================================================
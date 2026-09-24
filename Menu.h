#ifndef MENU_H
#define MENU_H

#include "Display.h"
#include "Buttons.h"
#include "Sound.h"
#include "Scroller.h"

class Menu {
public:

  // ========================================================
  // Configuración
  // ========================================================

  static constexpr uint8_t MAX_OPTIONS = 8;

  enum Option : uint8_t {
    OPC_NUEVO = 0,
    OPC_CONTINUAR,
    OPC_DIFICULTAD,
    OPC_SONIDO,
    OPC_CREDITOS
  };

  // ========================================================
  // Constructor
  // ========================================================

  Menu(Display& display, Buttons& buttons, Sound& sound, uint16_t bestScore = 0,
       const char* version = "v0.1");

  // ========================================================
  // Inicialización
  // ========================================================

  void begin();

  // ========================================================
  // Opciones (cantidad variable)
  // ========================================================

  void setOptions(const char* const* texts, uint8_t count);

  // ========================================================
  // Opción "Continue" (hay partida en curso que reanudar)
  //
  // true  -> la lista incluye "Continue" (5 opciones).
  // false -> se oculta (4 opciones: New, Difficulty, Sound,
  //          Credits). Es el estado inicial: sin partida no hay
  //          nada que continuar. La selección se mantiene y se
  //          adapta a la nueva cantidad de opciones.
  // ========================================================

  void setContinueAvailable(bool available);

  // ========================================================
  // Apariencia (título del Header y pie opcional)
  // ========================================================

  void setTitle(const char* title);
  void setShowFooter(bool show);

  // ========================================================
  // Selección inicial por OPCIÓN LÓGICA (enum Option, p. ej.
  // OPC_NUEVO u OPC_CONTINUAR) y reinicio de la animación.
  // Internamente se mapea al índice de la lista visible; si la
  // opción no está visible (p. ej. "Continue" oculto) la
  // selección cae a la primera opción (New).
  // ========================================================

  void setSelected(Option option);

  // ========================================================
  // Actualizar (consume eventos de botones, navega y anima)
  // ========================================================

  void update();

  // ========================================================
  // Dibujar
  // ========================================================

  void print();

  // ========================================================
  // Accesos
  // ========================================================

  int8_t selected() const;
  void setBestScore(uint16_t value);

  // Devuelve la opción elegida si se confirma (ACTION_RIGHT), o -1
  int8_t confirm() const;

  // ========================================================
  // Edición inline de Sonido
  // ========================================================

  void beginSoundEdit();
  void endSoundEdit();
  bool isEditingSound() const;

  // ========================================================
  // Edición inline de Dificultad
  // ========================================================

  void beginDifficultyEdit();
  void endDifficultyEdit();
  bool isEditingDifficulty() const;
  uint8_t difficulty() const;

private:

  // ========================================================
  // Opciones por defecto
  // ========================================================

  // La lista que se muestra depende de si hay partida en curso:
  //   - con "Continue": DEFAULT_OPTIONS (5) opciones
  //   - sin "Continue": DEFAULT_OPTIONS - 1 (4) opciones
  static constexpr uint8_t DEFAULT_OPTIONS = 5;
  static const char* const DEFAULT_OPTION_TEXT[DEFAULT_OPTIONS];
  static const char* const NO_CONTINUE_OPTIONS[DEFAULT_OPTIONS - 1];

  const char* optionText(int8_t index) const;

  // Mapeo entre el índice de la lista y la opción lógica (enum
  // Option). Con "Continue" el índice coincide con el enum; sin
  // "Continue" el índice 1 pasa a Dificultad, el 2 a Sonido y el
  // 3 a Créditos. optionAt() devuelve la opción lógica de un índice
  // de la lista; indexOfOption() hace lo contrario y devuelve -1 si
  // la opción no está visible (p. ej. OPC_CONTINUAR sin partida).
  Option optionAt(int8_t index) const;
  int8_t indexOfOption(Option option) const;

  // ========================================================
  // Geometría del menú
  // ========================================================

  // Región del menú dentro del Body: fila 16 hasta una fila antes del pie
  static constexpr int16_t BODY_TOP = 16;

  // Cuadro de selección: fijo, ancho completo. Con el rombo activo de punta en
  // la 45: 2 filas libres (44..43) y el cuadro desde la fila 3 (42) hacia arriba
  static constexpr int16_t BOX_TOP    = 25;
  static constexpr int16_t BOX_HEIGHT = 18;

  // Posición del texto del cuadro (1 px dentro, centrado verticalmente)
  static constexpr int16_t TEXT_SEL_TOP = 26;

  // Rombos de posición: banda 45..53, apoyada en la línea separadora 54 del pie.
  static constexpr int16_t DIA_TOP  = 45;  // punta superior del rombo (rombo simétrico 45..53)
  static constexpr uint8_t DIA_SIZE = 8;   // rombo: punta 45, hombros 49, punta inferior 53

  // Parpadeo del rombo seleccionado tras mantenerlo: visible 75%, oculto 25%
  static constexpr uint32_t BLINK_HOLD    = 500;  // mantener sin navegar para parpadear
  static constexpr uint32_t BLINK_PERIOD  = 500;  // período completo del parpadeo (ms)
  static constexpr uint8_t  BLINK_OFF_PCT = 25;   // % del período en que está oculto

  // Selector de sonido (modo edición): flecha única, pegada al texto, en el
  // lado del destino (OFF muestra "OFF >": MOVE_RIGHT enciende; ON muestra
  // "< ON": MOVE_LEFT apaga). La flecha parpadea: visible 75%, oculto 25% de
  // un período de ARROW_BLINK_PERIOD ms. La palabra NO parpadea.
  static constexpr int16_t ARROW_GAP             = 6;    // hueco (px) entre el texto y la flecha
  static constexpr int16_t ARROW_W               = 6;    // grosor horizontal de la flecha (px)
  static constexpr uint32_t ARROW_BLINK_PERIOD   = 500;  // período del parpadeo de la flecha (ms)
  static constexpr uint8_t  ARROW_BLINK_OFF_PCT  = 25;   // % del período en que la flecha está oculta

  // Selector de dificultad (modo edición): número 1..10 centrado con ancho
  // constante (1 dígito se alinea a la derecha con un espacio inicial: " 5"
  // mide lo mismo que "10", 12 px, y el centrado no se desplaza) y dos
  // flechas parpadeantes a los lados ("< 5 >"). La flecha del lado en su
  // límite se oculta: en 1 no hay flecha izquierda (-1 no existe); en 10 no
  // hay derecha (+1 no existe). MOVE_LEFT -1, MOVE_RIGHT +1, con repetición
  // al mantener presionado: el primer cambio es inmediato y tras
  // HOLD_REPEAT_DELAY ms repele cada HOLD_REPEAT_TICK ms. Al mantener un
  // botón el parpadeo se detiene: solo la flecha del botón activo queda fija
  // y la contraria se oculta (señal visual de la repetición continua). Al
  // llegar al límite (1 o 10) se procesa igual que haber soltado el botón:
  // vuelve el parpadeo normal (con el límite oculto).
  static constexpr uint8_t  DIFICULTAD_MIN     = 1;
  static constexpr uint8_t  DIFICULTAD_MAX     = 10;
  static constexpr uint8_t  DIFICULTAD_DEFAULT = 5;

  static constexpr uint32_t HOLD_REPEAT_DELAY = 400;  // mantener para empezar a repetir (ms)
  static constexpr uint32_t HOLD_REPEAT_TICK  = 100;  // intervalo de repetición mientras se mantiene (ms)

  // Pie del Body: línea separadora y texto (el texto baja 1 px: 56 -> 57)
  static constexpr int16_t PIE_LINE_ROW = 54;  // línea horizontal 1 px, a 2 px sobre el pie
  static constexpr int16_t PIE_TOP      = 57;  // texto "Best"/versión (antes fila 56)

  // ========================================================
  // Métodos internos
  // ========================================================

  void navigate();
  void drawDiamonds();
  void drawSoundSelector();
  void drawDifficultySelector();

  // Repetición por mantención: true cuando hay que aplicar el paso de un botón
  // (MOVE_LEFT/MOVE_RIGHT) en el modo de edición de dificultad. El primer
  // paso es inmediato (pressed); al mantener, solo tras HOLD_REPEAT_DELAY ms
  // y luego un paso cada HOLD_REPEAT_TICK ms.
  bool holdRepeat(uint8_t button);

  // ========================================================
  // Dependencias
  // ========================================================

  Display& _display;
  Buttons& _buttons;
  Sound& _sound;

  // ========================================================
  // Estado
  // ========================================================

  uint16_t _bestScore;
  const char* _version;

  const char* _title;      // título del Header (default "Snake II")
  bool _showFooter;        // pie "Best"/versión (default true)

  uint8_t _optionCount;
  const char* const* _optionTexts;
  bool _continueAvailable;   // muestra/oculta la opción "Continue" (default: oculta)

  int8_t _selected;   // opción actual (objetivo central)
  uint32_t _holdStart;  // momento de la última selección (parpadeo del rombo)
  bool _redraw;         // primer frame tras begin(): clear() completo + estáticos

  // Edición inline de sonido
  bool _editingSound;
  bool _soundEnabled;

  // Edición inline de dificultad
  bool _editingDifficulty;
  uint8_t _difficulty;      // valor persistente (default 5; se aplica al confirmar)
  uint8_t _editDifficulty;  // valor en edición (no aplicado hasta confirmar)
  uint32_t _repeatStart;    // inicio de la mantención (repetición por hold)
  uint32_t _repeatLast;     // último paso de la repetición

  Scroller _scroller;   // scroller de 1 bit del cuadro de selección (1 banda, texto 12x16)
};

#endif

// ====================================================================================
// Fin
// ====================================================================================
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

  Menu(Display& display, Buttons& buttons, Sound& sound, uint8_t topScore = 0,
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
  // Apariencia (título del Header y pie opcional)
  // ========================================================

  void setTitle(const char* title);
  void setShowFooter(bool show);

  // ========================================================
  // Selección inicial (clamp al rango de opciones) y reinicio
  // de la animación.
  // ========================================================

  void setSelected(int8_t index);

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
  void setTopScore(uint8_t value);

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

  static constexpr uint8_t DEFAULT_OPTIONS = 5;
  static const char* const DEFAULT_OPTION_TEXT[DEFAULT_OPTIONS];

  const char* optionText(int8_t index) const;

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

  // Selector de dificultad (modo edición): número 1..25 centrado con ancho
  // constante (1 dígito se alinea a la derecha con un espacio inicial: " 5"
  // mide lo mismo que "13", 12 px, y el centrado no se desplaza) y dos
  // flechas parpadeantes a los lados ("< 13 >"). La flecha del lado en su
  // límite se oculta: en 1 no hay flecha izquierda (-1 no existe); en 25 no
  // hay derecha (+1 no existe). MOVE_LEFT -1, MOVE_RIGHT +1, con repetición
  // al mantener presionado: el primer cambio es inmediato y tras
  // HOLD_REPEAT_DELAY ms repele cada HOLD_REPEAT_TICK ms.
  static constexpr uint8_t  DIFICULTAD_MIN     = 1;
  static constexpr uint8_t  DIFICULTAD_MAX     = 25;
  static constexpr uint8_t  DIFICULTAD_DEFAULT = 13;

  static constexpr uint32_t HOLD_REPEAT_DELAY = 400;  // mantener para empezar a repetir (ms)
  static constexpr uint32_t HOLD_REPEAT_TICK  = 100;  // intervalo de repetición mientras se mantiene (ms)

  // Pie del Body: línea separadora y texto (el texto baja 1 px: 56 -> 57)
  static constexpr int16_t PIE_LINE_ROW = 54;  // línea horizontal 1 px, a 2 px sobre el pie
  static constexpr int16_t PIE_TOP      = 57;  // texto "Top"/versión (antes fila 56)

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

  uint8_t _topScore;
  const char* _version;

  const char* _title;      // título del Header (default "Snake II")
  bool _showFooter;        // pie "Top"/versión (default true)

  uint8_t _optionCount;
  const char* const* _optionTexts;

  int8_t _selected;   // opción actual (objetivo central)
  uint32_t _holdStart;  // momento de la última selección (parpadeo del rombo)
  bool _redraw;         // primer frame tras begin(): clear() completo + estáticos

  // Edición inline de sonido
  bool _editingSound;
  bool _soundEnabled;

  // Edición inline de dificultad
  bool _editingDifficulty;
  uint8_t _difficulty;      // valor persistente (default 13; se aplica al confirmar)
  uint8_t _editDifficulty;  // valor en edición (no aplicado hasta confirmar)
  uint32_t _repeatStart;    // inicio de la mantención (repetición por hold)
  uint32_t _repeatLast;     // último paso de la repetición

  Scroller _scroller;   // scroller de 1 bit del cuadro de selección (1 banda, texto 12x16)
};

#endif

// ====================================================================================
// Fin
// ====================================================================================
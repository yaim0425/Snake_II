#ifndef MENU_H
#define MENU_H

#include "Config.h"
#include "Timer.h"

#include "Scroller.h"

class Menu {
public:

  // ========================================================
  // Configuración
  // ========================================================

  enum Option : uint8_t {
    OPT_NEW = 0,
    OPT_CONTINUE,
    OPT_DIFFICULTY,
    OPT_SOUND,
    OPT_CREDITS,

    OPT_COUNT
  };

  // ========================================================
  // Constructor (sin parámetros de servicios: usa los globales
  // Display, Buttons y Sound — Globals.h)
  // ========================================================

  Menu(uint16_t bestScore = 0, const char* version = Config::Version::VERSION);

  // ========================================================
  // Inicialización
  // ========================================================

  void begin();
  void drawDiamond(int8_t selected, bool focus, bool black);
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
  // OPT_NEW u OPT_CONTINUE) y reinicio de la animación.
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
void firstPrint();

  // ========================================================
  // Accesos
  // ========================================================

  int8_t selected() const;
  void setBestScore(uint16_t value);

  // Devuelve la opción elegida si se confirma (ACTION_RIGHT), o -1
  int8_t confirm() const;

  // ========================================================
  // Rombos de posición (restauración)
  // ========================================================

  // Las ventanas MenuDifficulty y MenuSound sustituyen la banda de
  // rombos (45..53) por su selector y dejan el resto de la ventana
  // como estaba. Al salir de ellas el Engine llama a esto para que
  // el Menu vuelva a pintar esa franja. A diferencia de begin(), no
  // borra la pantalla: solo la banda.
  void restoreDiamondBand();

private:

  // ========================================================
  // Opciones por defecto
  // ========================================================
  
  static const char* const OPTION_TEXT[OPT_COUNT];

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
  // la opción no está visible (p. ej. OPT_CONTINUE sin partida).
  Option optionAt(int8_t index) const;
  int8_t indexOfOption(Option option) const;

  // ========================================================
  // Geometría del menú
  // ========================================================

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

  // Pie del Body: línea separadora y texto (el texto baja 1 px: 56 -> 57)
  static constexpr int16_t PIE_LINE_ROW = 54;  // línea horizontal 1 px, a 2 px sobre el pie
  static constexpr int16_t PIE_TOP      = 57;  // texto "Best"/versión (antes fila 56)

  // ========================================================
  // Métodos internos
  // ========================================================

  void navigate();
  void blink();

  // ========================================================
  // Estado
  // ========================================================

  uint16_t _bestScore;
  const char* _version;

  const char* _title;      // título del Header (default "Snake II")
  bool _showFooter;        // pie "Best"/versión (default true)

  uint8_t _optionCount;
  const char* const* _optionTexts;
  bool _visibleContinue;   // muestra/oculta la opción "Continue" (default: oculta)
  bool _visibleDiamond;        // rombo activo visible (parpadeo)

  int8_t _selected;   // opción actual (objetivo central)
  int8_t _lastSelected;
  Stopwatch _timer;    // desde la última selección (parpadeo del rombo)
  bool _redraw;         // primer frame tras begin(): clear() completo + estáticos
  bool _diamondsDirty;  // hay que repintar solo la banda de rombos (restoreDiamondBand)

  Scroller _scroller;   // scroller de 1 bit del cuadro de selección (1 banda, texto 12x16)
};

#endif

// ====================================================================================
// Fin
// ====================================================================================
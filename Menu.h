#ifndef MENU_H
#define MENU_H

#include "Display.h"
#include "Buttons.h"
#include <Adafruit_GFX.h>

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

  Menu(Display& display, Buttons& buttons, uint8_t topScore = 0,
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
  // Actualizar (lee botones, navega y anima)
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

  // Devuelve la opción elegida si se confirma (ACTION_LEFT), o -1
  int8_t confirm() const;

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

  // Animación (scroller de 1 bit): la tira completa (128 px de matriz) se
  // desliza UNA columna por cada ANIM_TICK ms (acumulado por tiempo, la
  // velocidad no depende de la velocidad del loop). Vuelo total ~128*ANIM_TICK
  static constexpr uint8_t  STRIP_W    = 128;  // columnas de la tira (= ancho de pantalla)
  static constexpr uint8_t  STRIP_H    = 16;   // filas de la tira (= alto del texto 12x16)
  static constexpr uint32_t ANIM_TICK  = 4;    // ms por columna de desplazamiento (~0,5 s total)

  // Rombos de posición: banda 45..53, apoyada en la línea separadora 54 del pie.
  static constexpr int16_t DIA_TOP  = 45;  // punta superior del rombo (rombo simétrico 45..53)
  static constexpr uint8_t DIA_SIZE = 8;   // rombo: punta 45, hombros 49, punta inferior 53

  // Parpadeo del rombo seleccionado tras mantenerlo: visible 75%, oculto 25%
  static constexpr uint32_t BLINK_HOLD    = 500;  // mantener sin navegar para parpadear
  static constexpr uint32_t BLINK_PERIOD  = 1000;  // período completo del parpadeo (ms)
  static constexpr uint8_t  BLINK_OFF_PCT = 25;   // % del período en que está oculto

  // Pie del Body: línea separadora y texto (el texto baja 1 px: 56 -> 57)
  static constexpr int16_t PIE_LINE_ROW = 54;  // línea horizontal 1 px, a 2 px sobre el pie
  static constexpr int16_t PIE_TOP      = 57;  // texto "Top"/versión (antes fila 56)

  // ========================================================
  // Métodos internos
  // ========================================================

  void navigate();
  void startSlide(int8_t dir);
  void animate();
  void loadOption(int8_t index);               // compone la opción centrada en la matriz de 1 bit
  void drawStrip();                            // dibuja las columnas visibles de la matriz sobre el cuadro
  int16_t textCenterX(const char* text) const;
  void drawDiamonds();

  // ========================================================
  // Dependencias
  // ========================================================

  Display& _display;
  Buttons& _buttons;

  // ========================================================
  // Estado
  // ========================================================

  uint8_t _topScore;
  const char* _version;

  uint8_t _optionCount;
  const char* const* _optionTexts;

  int8_t _selected;   // opción actual (objetivo central)
  int8_t _dir;        // +1 siguiente (entra por la derecha), -1 anterior (izquierda)
  int16_t _slideX;    // borde izquierdo de la tira en pantalla (objetivo 0 = centrada)
  uint32_t _colAcc;   // acumulador de tiempo para avanzar columnas
  uint32_t _animLast;
  uint32_t _holdStart;

  uint8_t _strip[STRIP_H][STRIP_W / 8];  // matriz 128x16 de 1 bit: 1 = glifo negro, 0 = blanco
  GFXcanvas8 _chipBox;                   // canvas de composición (128x16) para cargar la opción
};

#endif
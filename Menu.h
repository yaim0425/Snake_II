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

  // Animación lateral (carousel): distancia del salto y velocidad
  static constexpr int16_t SLIDE_DIST = 48;
  static constexpr uint32_t ANIM_TICK = 15;
  static constexpr int8_t ANIM_STEP   = 2;

  // Rombos de posición: banda 45..53, apoyada en la línea separadora 54 del pie.
  static constexpr int16_t DIA_TOP  = 45;  // punta superior del rombo (rombo simétrico 45..53)
  static constexpr uint8_t DIA_SIZE = 8;   // rombo: punta 45, hombros 49, punta inferior 53

  // Parpadeo del rombo seleccionado tras mantenerlo
  static constexpr uint32_t BLINK_HOLD   = 500;
  static constexpr uint32_t BLINK_TOGGLE = 250;

  // Pie del Body: línea separadora y texto (el texto baja 1 px: 56 -> 57)
  static constexpr int16_t PIE_LINE_ROW = 54;  // línea horizontal 1 px, a 2 px sobre el pie
  static constexpr int16_t PIE_TOP      = 57;  // texto "Top"/versión (antes fila 56)

  // ========================================================
  // Métodos internos
  // ========================================================

  void navigate();
  void startSlide(int8_t dir);
  void animate();
  int16_t textCenterX(const char* text) const;
  void paintOption(int8_t index, int16_t offX);   // dibuja la opción en el canvas
  void blitChip();                                 // vuelca el canvas a la pantalla
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
  int16_t _slideX;    // desplazamiento de la opción entrante (objetivo 0)
  uint32_t _animLast;
  uint32_t _holdStart;

  GFXcanvas8 _chipBox;  // ventana del cuadro (recorta el texto al deslizar)
};

#endif
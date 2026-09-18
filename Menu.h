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
  // Actualizar (lee botones y anima)
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

  // Cuadro de selección: fijo, ancho completo, centrado en la banda 16..55
  static constexpr int16_t BOX_TOP    = 27;
  static constexpr int16_t BOX_HEIGHT = 18;

  // Posición del texto del cuadro (1 px dentro, centrado verticalmente)
  static constexpr int16_t TEXT_SEL_TOP = 28;

  // Animación lateral (carousel): distancia del salto y velocidad
  static constexpr int16_t SLIDE_DIST = 48;
  static constexpr uint32_t ANIM_TICK = 15;
  static constexpr int8_t ANIM_STEP   = 2;

  // Rombos de posición (sobre la fila 44 del cuadro: aire en la 45 y 2 filas
  // libres 44..45 entre rombo completo y cuadro; pie libre a partir de la 54)
  static constexpr int16_t DIA_TOP  = 46;  // punta superior del rombo seleccionado (8 filas: 46..53)
  static constexpr uint8_t DIA_SIZE = 8;
  static constexpr int16_t DIA_RISE = 1;   // el seleccionado se eleva 1 px sobre la punta

  // Parpadeo del rombo seleccionado tras mantenerlo
  static constexpr uint32_t BLINK_HOLD   = 500;
  static constexpr uint32_t BLINK_TOGGLE = 250;

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
  int8_t _dir;        // +1 bajar, -1 subir (sentido del deslizamiento)
  int16_t _slideX;    // desplazamiento de la opción entrante (objetivo 0)
  uint32_t _animLast;
  uint32_t _holdStart;

  GFXcanvas8 _chipBox;  // ventana del cuadro (recorta el texto al deslizar)
};

#endif
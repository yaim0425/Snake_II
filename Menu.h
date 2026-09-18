#ifndef MENU_H
#define MENU_H

#include "Display.h"
#include "Buttons.h"

class Menu {
public:

  // ========================================================
  // Configuración
  // ========================================================

  static constexpr uint8_t OPTION_COUNT = 5;

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
  // Opciones
  // ========================================================

  static const char* const OPTION_TEXT[OPTION_COUNT];

  // ========================================================
  // Geometría del menú
  // ========================================================

  // Región del menú dentro del Body: fila 16 hasta una fila antes del pie
  static constexpr int16_t BODY_TOP    = 16;
  static constexpr int16_t MENU_BOTTOM = 55;

  // Cuadro de selección: fijo, ancho completo, centrado en la banda 16..55
  static constexpr int16_t BOX_TOP    = 27;
  static constexpr int16_t BOX_HEIGHT = 18;

  // Posición objetivo del texto seleccionado (centrado en el cuadro)
  static constexpr int16_t TEXT_SEL_TOP = 28;

  // Separación vertical entre opciones apiladas
  static constexpr int16_t OPTION_STEP = 20;

  // Dos filas libres sobre el pie: no se dibuja nada entre 40..55
  static constexpr int16_t FREE_TOP = 40;
  static constexpr int16_t PIE_TOP  = 56;

  // Velocidad de la animación (px por tick)
  static constexpr uint32_t ANIM_TICK = 15;

  // ========================================================
  // Métodos internos
  // ========================================================

  int16_t optionTarget(int8_t i) const;
  void animate();

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

  int8_t _selected;
  int16_t _optionY[OPTION_COUNT];
  uint32_t _animLast;
};

#endif
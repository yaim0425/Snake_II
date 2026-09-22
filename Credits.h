#ifndef CREDITS_H
#define CREDITS_H

#include "Display.h"
#include "Buttons.h"
#include "Sound.h"
#include "Scroller.h"

class Credits {
public:

  // ========================================================
  // Configuración
  // ========================================================

  static constexpr uint8_t NUM_ENTRIES = 3;

  // ========================================================
  // Constructor
  // ========================================================

  Credits(Display& display, Buttons& buttons, Sound& sound);

  // ========================================================
  // Inicialización (al entrar en la ventana)
  // ========================================================

  void begin();

  // ========================================================
  // Actualizar (consume eventos de botones, navega y anima)
  // ========================================================

  void update();

  // ========================================================
  // Dibujar
  // ========================================================

  void print();

  // ========================================================
  // Salida (true = ACTION_UP pidió volver al menú)
  // ========================================================

  bool done() const;

private:

  // ========================================================
  // Dependencias
  // ========================================================

  Display& _display;
  Buttons& _buttons;
  Sound& _sound;

  // ========================================================
  // Estado
  // ========================================================

  uint8_t _entry;   // entrada visible (0 = izquierda, 1 = centro, 2 = derecha)
  bool _exit;

  // ========================================================
  // Scroller de 1 bit compartido (igual que el menú): DOS bandas
  // sincronizadas (deslizan con el mismo _slideX). El rol
  // (12x16) y el nombre (6x8) se componen por separado en la
  // tira y cada banda sobrescribe columna a columna su canvas
  // persistente: la entrada anterior se mantiene hasta que la
  // nueva la cubre. El deslizamiento arranca desde el borde.
  // ========================================================

  Scroller _scroller;

  // ========================================================
  // Métodos internos
  // ========================================================

  void navigate();
  void loadEntry(uint8_t slot);  // compone rol/nombre en la tira de 1 bit

  // Compone el slot y vuelca su banda persistente sincronizada
  void drawBand(uint8_t slot, int16_t y, uint16_t fgColor, uint16_t bgColor);
};

#endif
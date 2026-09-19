#ifndef CREDITS_H
#define CREDITS_H

#include "Display.h"
#include "Buttons.h"

class Credits {
public:

  // ========================================================
  // Constructor
  // ========================================================

  Credits(Display& display, Buttons& buttons, const char* version = "v0.1");

  // ========================================================
  // Inicialización (al entrar en la ventana)
  // ========================================================

  void begin();

  // ========================================================
  // Actualizar (lee botones y eventos)
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

  Display& _display;
  Buttons& _buttons;

  const char* _version;
  bool _exit;
};

#endif
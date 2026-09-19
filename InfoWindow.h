#ifndef INFOWINDOW_H
#define INFOWINDOW_H

#include "Display.h"
#include "Buttons.h"

// Ventana genérica para opciones aún no implementadas:
// muestra un título + "En desarrollo" y vuelve al menú con ACTION_UP.
class InfoWindow {
public:

  // ========================================================
  // Constructor
  // ========================================================

  InfoWindow(Display& display, Buttons& buttons);

  // ========================================================
  // Inicialización (al entrar en la ventana)
  // ========================================================

  void begin(const char* title);

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

  const char* _title;
  bool _exit;
};

#endif
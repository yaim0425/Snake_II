#ifndef SOUNDWINDOW_H
#define SOUNDWINDOW_H

#include "Display.h"
#include "Buttons.h"
#include "Sound.h"
#include "Menu.h"

// ========================================================
// SoundWindow — opción "Sound" del menú
//
// "On" y "Off" se muestran y animan igual que las opciones
// del menú principal: un submenú (Menu embebido) con el mismo
// cuadro deslizante (scroller de 1 bit) y los rombos de
// posición, título "Sound" y sin pie.
//   - MOVE_LEFT / MOVE_RIGHT: navegan dentro del submenú
//     (SFX_CLICK, primera/última no conectadas, sin ciclo).
//   - ACTION_RIGHT: aplica la opción seleccionada al sonido
//     (SFX_CONFIRM al encender, para comprobar que hay audio).
//   - ACTION_UP: vuelve al menú.
// La selección inicial refleja el estado actual de Sound.
// ========================================================

class SoundWindow {
public:

  // ========================================================
  // Constructor
  // ========================================================

  SoundWindow(Display& display, Buttons& buttons, Sound& sound);

  // ========================================================
  // Inicialización (al entrar en la ventana)
  // ========================================================

  void begin();

  // ========================================================
  // Actualizar (consume los eventos de botones leídos en loop())
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
  Sound& _sound;

  bool _exit;
  Menu _menu;  // submenú On/Off (mismo diseño y animación que el menú principal)
};

#endif
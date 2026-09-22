#ifndef SOUNDWINDOW_H
#define SOUNDWINDOW_H

#include "Display.h"
#include "Buttons.h"
#include "Sound.h"

// ========================================================
// SoundWindow — opción "Sound" del menú
//
// "On" y "Off" se comportan como las opciones del Menú:
//   - MOVE_LEFT / MOVE_RIGHT: cambian la selección (con SFX_CLICK,
//     primera/última no conectadas, sin ciclo).
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
  Sound& _sound;

  bool _exit;
  uint8_t _selected;  // 0 = On, 1 = Off (opción resaltada, como el menú)
};

#endif
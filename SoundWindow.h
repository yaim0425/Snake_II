#ifndef SOUNDWINDOW_H
#define SOUNDWINDOW_H

#include "Display.h"
#include "Buttons.h"
#include "Sound.h"

// ========================================================
// SoundWindow — opción "Sound" del menú
//
// Activa/desactiva el sonido del juego:
//   - ACTION_RIGHT: alterna On/Off (suena el confirm si se
//     enciende, para comprobar que hay audio).
//   - ACTION_UP: vuelve al menú.
// El estado lo conserva la clase Sound (compartida).
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
};

#endif
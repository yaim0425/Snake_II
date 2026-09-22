#include "SoundWindow.h"

// ========================================================
// Opciones del submenú (On / Off, como las del menú)
// ========================================================

static const char* const ON_OFF_OPTIONS[2] = { "On", "Off" };

// ========================================================
// Constructor: prepara el submenú (título "Sound", sin pie)
// ========================================================

SoundWindow::SoundWindow(Display& display, Buttons& buttons, Sound& sound)
  : _display(display),
    _buttons(buttons),
    _sound(sound),
    _exit(false),
    _menu(display, buttons, sound) {

  // On/Off se muestran y animan como las opciones del menú principal
  _menu.setOptions(ON_OFF_OPTIONS, 2);
  _menu.setTitle("Sound");
  _menu.setShowFooter(false);
}

// ========================================================
// Inicialización (al entrar, la selección refleja el estado)
// ========================================================

void SoundWindow::begin() {
  _exit = false;
  _menu.setSelected(_sound.enabled() ? 0 : 1);  // 0 = On, 1 = Off
}

// ========================================================
// Actualizar: el submenú navega (MOVE_LEFT/MOVE_RIGHT, con
// SFX_CLICK) y anima el deslizamiento como el menú principal;
// aquí se aplica la opción (ACTION_RIGHT) y se vuelve al menú
// (ACTION_UP). Los eventos se leen dentro de menu.update().
// ========================================================

void SoundWindow::update() {
  _menu.update();

  // Volver al menú (botón común de las ventanas)
  if (_buttons.actionUpPressed()) {
    _exit = true;
    return;
  }

  // Confirmar la opción seleccionada (ACTION_RIGHT, como el menú)
  if (_buttons.actionRightPressed()) {
    int8_t sel = _menu.selected();
    _sound.setEnabled(sel == 0);

    // Al dejar el sonido encendido se oye la confirmación
    // (comprueba que hay audio)
    if (sel == 0) _sound.play(Sound::SFX_CONFIRM);
  }
}

// ========================================================
// Dibujar: se delega al submenú (mismo diseño que el menú)
// ========================================================

void SoundWindow::print() {
  _menu.print();
}

// ========================================================
// Salida
// ========================================================

bool SoundWindow::done() const {
  return _exit;
}
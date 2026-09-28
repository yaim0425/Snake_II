#include "esp32-hal.h"
#include "MenuSound.h"
#include "Globals.h"

// ========================================================
// Constructor
// ========================================================

MenuSound::MenuSound()
  : _enabled(true),
    _done(false),
    _blink() {}

// ========================================================
// Inicialización
//
// No borra la pantalla ni dibuja nada: entra sobre el Menu ya
// dibujado, así que basta con partir del valor que tiene el
// servicio Sound (lo que se cancele se descarta) y anclar el
// parpadeo. El selector lo pinta print().
// ========================================================

void MenuSound::begin() {
  _enabled = sound.enabled();
  _done = false;
  _blink.start();
}

// ========================================================
// Actualizar
//
// La tecla indicada por la flecha (la del destino) cambia el
// valor mostrado (ON/OFF) sin aplicarlo; solo se aplica al
// confirmar con ACTION_RIGHT. ACTION_UP cancela. En ambos casos
// se vuelve al Menu.
// ========================================================

void MenuSound::update() {
  if (buttons.pressed(Buttons::MOVE_LEFT) && _enabled) {
    // ON: la flecha "<" (MOVE_LEFT) apaga
    _enabled = false;
    sound.play(Sound::SFX_CLICK);
  }
  if (buttons.pressed(Buttons::MOVE_RIGHT) && !_enabled) {
    // OFF: la flecha ">" (MOVE_RIGHT) enciende
    _enabled = true;
    sound.play(Sound::SFX_CLICK);
  }

  if (buttons.pressed(Buttons::ACTION_RIGHT)) {
    // Select: aplica el valor actual
    sound.setEnabled(_enabled);
    // Al apagar no hay nada que confirmar: el sonido está muteado
    if (_enabled) sound.play(Sound::SFX_CONFIRM);
    _done = true;
  } else if (buttons.pressed(Buttons::ACTION_UP)) {
    // Back: cancela sin cambiar el estado
    sound.play(Sound::SFX_BACK);
    _done = true;
  }
}

// ========================================================
// Dibujar
//
// La franja de rombos del Menu se borra y se vuelve a pintar en
// cada frame (la flecha parpadea; la palabra no). No se toca
// ninguna otra zona: el título, el cuadro de la opción y el pie
// del Menu siguen como estaban.
// ========================================================

void MenuSound::print() {
  display.fillRect(0, SEL_TOP, display.getWidth(), SEL_SIZE + 1, true);
  drawSelector();
}

// ========================================================
// Selector de sonido
//
// Texto 6x8 centrado con el MISMO ancho en ambos estados ("ON "
// lleva un espacio final para emparejar el ancho con "OFF",
// 18 px, así el centrado no se desplaza) y UNA flecha
// parpadeante, en el lado del destino:
//   - OFF: "OFF >"  (la flecha apunta a la tecla MOVE_RIGHT,
//     que enciende el sonido)
//   - ON:  "< ON"  (la flecha apunta a la tecla MOVE_LEFT,
//     que apaga el sonido)
// La flecha está pegada a la palabra (hueco ARROW_GAP) y
// parpadea visible 75% / oculta 25% de ARROW_BLINK_PERIOD ms;
// la palabra no parpadea.
// ========================================================

void MenuSound::drawSelector() {
  // Palabra centrada. En ambos estados mide lo mismo: "ON " lleva un
  // espacio final para emparejar el ancho con "OFF" (18 px).
  const char* label = (_enabled) ? "ON " : "OFF";
  int16_t labelW = display.getTextWidth(label, TEXT_6x8);
  int16_t labelX = (display.getWidth() - labelW) / 2;

  // Parpadeo de la flecha: visible el 75% del período, oculta el
  // primer 25% (anclado al begin: sin salto de fase con el reloj
  // de 64 bits)
  if (_blink.blinkOn(ARROW_BLINK_PERIOD, ARROW_BLINK_OFF_PCT)) {
    if (_enabled) {
      // ON: flecha a la izquierda, punta hacia la izquierda ("< ON")
      int16_t base = labelX - ARROW_GAP;  // lado plano, pegado a la palabra
      display.fillTriangle(base - ARROW_W, SEL_TEXT_MID, base, SEL_TEXT_TOP, base, SEL_TEXT_BOT, true);
    } else {
      // OFF: flecha a la derecha, punta hacia la derecha ("OFF >")
      int16_t base = labelX + labelW + ARROW_GAP;  // lado plano, pegado a la palabra
      display.fillTriangle(base + ARROW_W, SEL_TEXT_MID, base, SEL_TEXT_TOP, base, SEL_TEXT_BOT, true);
    }
  }

  display.drawText(label, labelX, SEL_TEXT_TOP, TEXT_6x8);
}

// ========================================================
// Estado
// ========================================================

bool MenuSound::done() const {
  return _done;
}

// ====================================================================================
// Fin
// ====================================================================================

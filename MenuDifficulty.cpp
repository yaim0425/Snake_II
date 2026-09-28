#include "esp32-hal.h"
#include "MenuDifficulty.h"
#include "Globals.h"

#include <stdio.h>

// ========================================================
// Constructor
// ========================================================

MenuDifficulty::MenuDifficulty()
  : _difficulty(Config::Difficulty::DEFAULT_LEVEL),
    _edit(Config::Difficulty::DEFAULT_LEVEL),
    _done(false),
    _blink(),
    _repeat(),
    _repeatTick() {}

// ========================================================
// Inicialización
//
// No borra la pantalla ni dibuja nada: entra sobre el Menu ya
// dibujado, así que basta con poner el valor confirmado en
// edición y anclar el parpadeo (sin salto de fase respecto al
// reloj de 64 bits). El selector lo pinta print().
// ========================================================

void MenuDifficulty::begin() {
  _edit = _difficulty;
  _done = false;
  _blink.start();
}

// ========================================================
// Actualizar
//
// MOVE_RIGHT +1 y MOVE_LEFT -1 con repetición al mantener
// presionado (primer paso inmediato, después cada
// HOLD_REPEAT_TICK ms tras HOLD_REPEAT_DELAY de mantención). El
// valor mostrado cambia sin aplicarse; ACTION_RIGHT lo aplica y
// ACTION_UP cancela. En ambos casos se vuelve al Menu.
// ========================================================

void MenuDifficulty::update() {
  if (holdRepeat(Buttons::MOVE_RIGHT) && _edit < Config::Difficulty::MAX_LEVEL) {
    _edit++;
    sound.play(Sound::SFX_CLICK);
  }
  if (holdRepeat(Buttons::MOVE_LEFT) && _edit > Config::Difficulty::MIN_LEVEL) {
    _edit--;
    sound.play(Sound::SFX_CLICK);
  }

  if (buttons.pressed(Buttons::ACTION_RIGHT)) {
    // Select: aplica el nivel mostrado
    _difficulty = _edit;
    sound.play(Sound::SFX_CONFIRM);
    _done = true;
  } else if (buttons.pressed(Buttons::ACTION_UP)) {
    // Back: cancela sin cambiar el nivel confirmado
    sound.play(Sound::SFX_BACK);
    _done = true;
  }
}

// ========================================================
// Repetición por mantención
// ========================================================

bool MenuDifficulty::holdRepeat(uint8_t button) {
  if (buttons.pressed(button)) {
    _repeat.start();
    _repeatTick.start();
    return true;
  }

  if (buttons.state(button) && _repeat.expired(HOLD_REPEAT_DELAY) && _repeatTick.expired(HOLD_REPEAT_TICK)) {
    _repeatTick.start();
    return true;
  }

  return false;
}

// ========================================================
// Dibujar
//
// La franja de rombos del Menu se borra y se vuelve a pintar en
// cada frame (las flechas parpadean; el número no). No se toca
// ninguna otra zona: el título, el cuadro de la opción y el pie
// del Menu siguen como estaban.
// ========================================================

void MenuDifficulty::print() {
  display.fillRect(0, SEL_TOP, display.getWidth(), SEL_SIZE + 1, true);
  drawSelector();
}

// ========================================================
// Selector de dificultad
//
// Texto 6x8 con el número 1..10 centrado con ancho constante
// (1 dígito se alinea a la derecha con un espacio inicial: " 5"
// mide lo mismo que "10", 12 px, y el centrado no se desplaza) y
// DOS flechas a los lados, apuntando al exterior ("< 5 >"):
//   - flecha izquierda: MOVE_LEFT (-1), oculta en el mínimo
//   - flecha derecha:   MOVE_RIGHT (+1), oculta en el máximo
// Las flechas parpadean juntas (visible 75%, oculto 25% de
// ARROW_BLINK_PERIOD ms), pegadas al número (hueco ARROW_GAP);
// el número no parpadea.
//
// Al mantener presionado MOVE_LEFT o MOVE_RIGHT el paso se vuelve
// continuo y el dibujo lo refleja: se detiene el parpadeo, solo la
// flecha del botón activo queda fija y la contraria se oculta. Al
// llegar al límite (1 o 10) el botón de ese lado ya no puede
// avanzar y se dibuja como siempre (flecha oculta), sin marcar la
// repetición.
// ========================================================

void MenuDifficulty::drawSelector() {
  // Número centrado con ancho constante (" 5" / "10" = 12 px)
  char buf[8];
  if (_edit < 10) sprintf(buf, " %u", _edit);
  else sprintf(buf, "%u", _edit);

  int16_t labelW = display.getTextWidth(buf, TEXT_6x8);
  int16_t labelX = (display.getWidth() - labelW) / 2;

  // Al mantener presionado MOVE_LEFT (-1) o MOVE_RIGHT (+1) la repetición
  // continua queda marcada en pantalla: el parpadeo se detiene y SOLO la
  // flecha del botón activo se muestra (fija); la contraria se oculta.
  // Sin mantener, las dos flechas parpadean juntas (visible 75%, oculto
  // 25% de ARROW_BLINK_PERIOD ms) como siempre.
  bool leftHeld  = buttons.state(Buttons::MOVE_LEFT)  && _edit > Config::Difficulty::MIN_LEVEL;
  bool rightHeld = buttons.state(Buttons::MOVE_RIGHT) && _edit < Config::Difficulty::MAX_LEVEL;

  bool arrowsVisible = leftHeld || rightHeld || _blink.blinkOn(ARROW_BLINK_PERIOD, ARROW_BLINK_OFF_PCT);

  if (arrowsVisible) {
    // Flecha izquierda (-1): fija al mantener MOVE_LEFT; oculta mientras se
    // mantiene MOVE_RIGHT; sin mantener parpadea. No se dibuja en el mínimo.
    if (_edit > Config::Difficulty::MIN_LEVEL && !rightHeld) {
      int16_t base = labelX - ARROW_GAP;  // lado plano, pegado al número
      display.fillTriangle(base - ARROW_W, SEL_TEXT_MID, base, SEL_TEXT_TOP, base, SEL_TEXT_BOT, true);
    }
    // Flecha derecha (+1): fija al mantener MOVE_RIGHT; oculta mientras se
    // mantiene MOVE_LEFT; sin mantener parpadea. No se dibuja en el máximo.
    if (_edit < Config::Difficulty::MAX_LEVEL && !leftHeld) {
      int16_t base = labelX + labelW + ARROW_GAP;  // lado plano, pegado al número
      display.fillTriangle(base + ARROW_W, SEL_TEXT_MID, base, SEL_TEXT_TOP, base, SEL_TEXT_BOT, true);
    }
  }

  display.drawText(buf, labelX, SEL_TEXT_TOP, TEXT_6x8);
}

// ========================================================
// Estado
// ========================================================

bool MenuDifficulty::done() const {
  return _done;
}

uint8_t MenuDifficulty::difficulty() const {
  return _difficulty;
}

// ====================================================================================
// Fin
// ====================================================================================

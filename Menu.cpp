#include "esp32-hal.h"
#include "Menu.h"
#include "Globals.h"

#include <stdio.h>

// ========================================================
// Opciones por defecto
// ========================================================

const char* const Menu::OPTION_TEXT[OPT_COUNT] = {
  "New",
  "Continue",
  "Difficulty",
  "Sound",
  "Credits"
};

const char* const Menu::DEFAULT_OPTION_TEXT[Menu::DEFAULT_OPTIONS] = {
  "New",
  "Continue",
  "Difficulty",
  "Sound",
  "Credits"
};

const char* const Menu::NO_CONTINUE_OPTIONS[Menu::DEFAULT_OPTIONS - 1] = {
  "New",
  "Difficulty",
  "Sound",
  "Credits"
};

const char* Menu::optionText(int8_t index) const {
  return _optionTexts[index];
}

// ========================================================
// Mapeo índice <-> opción lógica (enum Option)
//
// Con "Continue" visible, el índice de la lista coincide con
// el valor del enum (New=0, Continue=1, Difficulty=2, Sound=3,
// Credits=4). Sin "Continue" la lista se compacta: el índice 1
// pasa a Dificultad, el 2 a Sonido y el 3 a Créditos; el
// OPT_CONTINUE deja de existir (indexOfOption devuelve -1).
//
// "Difficulty" y "Sound" ya no se editan inline aquí: confirm()
// las devuelve como cualquier otra opción y el Engine abre las
// ventanas MenuDifficulty y MenuSound.
// ========================================================

Menu::Option Menu::optionAt(int8_t index) const {
  if (_visibleContinue) return (Option)index;
  switch (index) {
    case 0: return OPT_NEW;
    case 1: return OPT_DIFFICULTY;
    case 2: return OPT_SOUND;
    default: return OPT_CREDITS;
  }
}

int8_t Menu::indexOfOption(Option option) const {
  if (_visibleContinue) return (int8_t)option;
  switch (option) {
    case OPT_NEW: return 0;
    case OPT_DIFFICULTY: return 1;
    case OPT_SOUND: return 2;
    case OPT_CREDITS: return 3;
    default: return -1;  // OPT_CONTINUE sin partida en curso
  }
}

// ========================================================
// Constructor
// ========================================================

Menu::Menu(uint16_t bestScore, const char* version)
  : _bestScore(bestScore),
    // _version(version),
    // _title("Snake II"),
    // _showFooter(true),
    // _optionCount(OPT_NEW),
    // _optionTexts(NO_CONTINUE_OPTIONS),
    _visibleContinue(false),
    _visibleDiamond(true),
    _selected(OPT_NEW),
    _timer(),
    _redraw(true),
    _diamondsDirty(false),
    _scroller() {}

// ========================================================
// Inicialización
// ========================================================

void Menu::begin() {
  _scroller.begin();
  _scroller.setTexto(OPTION_TEXT[_selected], 16, TEXT_12x16);
  _timer.start();
  _redraw = true;
  _visibleDiamond = true;
  // _optionCount = OPT_COUNT;
}

// ========================================================
// Opciones (cantidad variable)
// ========================================================

void Menu::setOptions(const char* const* texts, uint8_t count) {
  if (texts == nullptr) return;

  if (count < 1) count = 1;
  if (count > OPT_COUNT) count = OPT_COUNT;

  _optionTexts = texts;
  _optionCount = count;

  if (_selected >= (int8_t)count) _selected = count - 1;

  _scroller.begin();
  _timer.start();
  _scroller.setTexto(optionText(_selected), 16, TEXT_12x16);
  _redraw = true;
}

// ========================================================
// Opción "Continue" (hay partida en curso que reanudar)
//
// Cambia a la lista con/ sin "Continue" (5 o 4 opciones). La
// selección se conserva y se adapta a la nueva cantidad; si la
// opción seleccionada ya no existe (p. ej. estaba en "Continue"
// y se oculta), la navegación vuelve a apuntarla a un rango
// válido (la misión de dejar la selección en "New" la cumple
// el Engine con setSelected tras llamarnos).
// ========================================================

void Menu::setContinueAvailable(bool available) {
  if (available == _visibleContinue) return;

  _visibleContinue = available;
  setOptions(available ? DEFAULT_OPTION_TEXT : NO_CONTINUE_OPTIONS,
             available ? DEFAULT_OPTIONS : DEFAULT_OPTIONS - 1);
}

// ========================================================
// Apariencia (título del Header y pie opcional)
// ========================================================

void Menu::setTitle(const char* title) {
  _title = title;
}

void Menu::setShowFooter(bool show) {
  _showFooter = show;
}

// ========================================================
// Selección inicial (clamp al rango de opciones) y reinicio
// de la animación.
// ========================================================

void Menu::setSelected(Menu::Option option) {
  int8_t index = indexOfOption(option);
  if (index < 0) index = 0;  // la opción no está visible (p. ej. "Continue" oculto)

  _selected = index;
  _scroller.begin();
  _timer.start();
  _scroller.setTexto(optionText(_selected), 16, TEXT_12x16);
  _redraw = true;
}

// ========================================================
// Rombos de posición (restauración)
//
// La repintamos entera (y solo ella) porque la ventana anterior
// la dejó ocupada con su selector. No hace falta vaciar el resto
// de la pantalla: el título, el cuadro de la opción y el pie
// siguen siendo los del propio Menu. El Engine entra al Menu sin
// llamar a begin() (changeState(..., false)), así que el clear()
// completo no se dispara y esta bandera es la única que repinta.
// ========================================================

void Menu::restoreDiamondBand() {
  _diamondsDirty = true;
  _timer.start();  // el parpadeo del rombo activo arranca de cero
}

// ========================================================
// Actualizar (consume los eventos de botones leídos en loop())
//
// "Difficulty" y "Sound" no se editan aquí: al confirmarlas
// confirm() devuelve la opción y el Engine abre la ventana
// correspondiente (MenuDifficulty / MenuSound). El Menú solo
// navega y compone.
// ========================================================

void Menu::update() {
  navigate();
  if (true) return;  // IGNORE: no se actualiza el menú en esta versión

  _scroller.animate();
}

// ========================================================
// Navegación
// ========================================================

void Menu::navigate() {
  int8_t before = _selected;
  bool moved = false;

  // Solo MOVE_LEFT y MOVE_RIGHT (primera y última no conectadas)
  if (buttons.pressed(Buttons::MOVE_LEFT) && _selected > 0) {
    _selected--;
    if (_selected == OPT_CONTINUE && !_visibleContinue) _selected--;  // saltar "Continue" si no está visible
    moved = true;
  }

  if (buttons.pressed(Buttons::MOVE_RIGHT) && _selected < OPT_COUNT - 1) {
    _selected++;
    if (_selected == OPT_CONTINUE && !_visibleContinue) _selected++;  // saltar "Continue" si no está visible
    moved = true;
  }

  if (moved) {
    sound.play(Sound::SFX_CLICK);
    // _scroller.setTexto(optionText(_selected), 16, TEXT_12x16);
    // _scroller.startSlide((_selected > before) ? 1 : -1);
    _timer.start();
    Serial.printf("Menu: opcion %d -> %d\n", before, _selected);
  }
}

// ========================================================
// Rombos de posición
// ========================================================

void Menu::blink() {
  if (_timer.expired(BLINK_HOLD)) {
    bool visibleDiamond = _timer.blinkOn(BLINK_PERIOD, BLINK_OFF_PCT);
    if ((visibleDiamond && !_visibleDiamond) || (!visibleDiamond && _visibleDiamond)) {
      drawDiamond(_selected, true, _visibleDiamond);

      // // Rombos de posición
      // int8_t size = DIA_SIZE / 2;
      // int16_t y = DIA_TOP + DIA_SIZE;
      // int16_t x = (_selected + 1) * floor(display.getWidth() / (_optionCount + 1));

      // // Dibujar rombos de posición (triángulos) centrados en la banda inferior
      // display.fillTriangle(x - size, y - size, x, y - DIA_SIZE, x + size, y - size, _visibleDiamond);
      // display.fillTriangle(x - size, y - size, x, y - 0, x + size, y - size, _visibleDiamond);

      // Hacer intermitente el texto de la opción seleccionada
      display.fillRect(0, BOX_TOP, display.getWidth(), BOX_HEIGHT, false);
      if (!_visibleDiamond)
        display.drawTextInverted(OPTION_TEXT[_selected], (Config::Screen::WIDTH - strlen(OPTION_TEXT[_selected]) * 12) / 2, BOX_TOP + 1, TEXT_12x16);

      _visibleDiamond = !_visibleDiamond;
    }
  }
}

void Menu::drawDiamond(int8_t diamond, bool focus, bool black) {
  // Rombos de posición
  int8_t size = DIA_SIZE / 2;
  int16_t y = DIA_TOP + DIA_SIZE;
  int16_t x = (diamond + 1) * floor(display.getWidth() / (OPT_COUNT + 1));

  if (focus) {
    display.fillTriangle(x - size, y - size, x, y - DIA_SIZE, x + size, y - size, black);
    display.fillTriangle(x - size, y - size, x, y - 0, x + size, y - size, black);
  } else
    display.fillTriangle(x - size, y, x, y - size, x + size, y, black);
}

// ========================================================
// Dibujar
// ========================================================

void Menu::print() {

  // Estáticos (solo al entrar, tras el clear() completo): fondo, cuadro
  // de selección, header (título) y pie (línea + Best/versión). Se dibujan
  // UNA sola vez; ya no se borran ni se redibujan en cada frame.
  if (_redraw) {
    display.clear();
    firstPrint();
    _redraw = false;
  }

  // La ventana anterior (MenuDifficulty/MenuSound) sustituyó la banda de
  // rombos por su selector y la borró entera. Esa banda (45..53) incluye la
  // fila de la línea separadora del pie (Config::Screen::FOOT_LINE = 53),
  // así que hay que repintar las dos cosas. Solo se toca esa franja: el
  // título, el cuadro de la opción y el texto del pie siguen como estaban.
  if (_diamondsDirty) {
    display.fillRect(0, DIA_TOP, display.getWidth(), DIA_SIZE + 1, true);
    for (int8_t i = 0; i < OPT_COUNT; i++)
      drawDiamond(i, i == _selected, false);
    display.fillRect(0, Config::Screen::FOOT_LINE, display.getWidth(), 1, false);
    _visibleDiamond = true;  // el rombo activo vuelve a la fase visible
    _diamondsDirty = false;
  }

  if (_lastSelected != _selected) {
    drawDiamond(_lastSelected, true, true);
    drawDiamond(_selected, false, true);
    drawDiamond(_lastSelected, false, false);
    drawDiamond(_selected, true, false);

    // int8_t size = DIA_SIZE / 2;
    // int16_t y = DIA_TOP + DIA_SIZE;
    // int16_t ax = (_selected + 1) * floor(display.getWidth() / (_optionCount + 1));
    // int16_t bx = (_lastSelected + 1) * floor(display.getWidth() / (_optionCount + 1));

    // display.fillTriangle(bx - size, y - size, bx, y - DIA_SIZE, bx + size, y - size, true);
    // display.fillTriangle(bx - size, y - size, bx, y - 0, bx + size, y - size, true);
    // display.fillTriangle(ax - size, y, ax, y - size, ax + size, y, true);

    // display.fillTriangle(ax - size, y - size, ax, y - DIA_SIZE, ax + size, y - size, false);
    // display.fillTriangle(ax - size, y - size, ax, y - 0, ax + size, y - size, false);
    // display.fillTriangle(bx - size, y, bx, y - size, bx + size, y, false);

    display.fillRect(0, BOX_TOP, display.getWidth(), BOX_HEIGHT, false);
    display.drawTextInverted(OPTION_TEXT[_selected], (Config::Screen::WIDTH - strlen(OPTION_TEXT[_selected]) * 12) / 2, BOX_TOP + 1, TEXT_12x16);

    _lastSelected = _selected;
    _timer.start();
  }

  blink();
  if (true) return;  // IGNORE: no se actualiza el menú en esta versión

  // Dinámicos (cada frame): la banda de la opción deslizante y los rombos.
  // La banda persistente (lo que ya está en pantalla) se mantiene intacta;
  // la tira nueva desliza y sus columnas sobrescriben la banda hasta
  // reemplazarla por completo. La opción anterior permanece hasta ser borrada.
  // El scroller se salta este volcado cuando está en reposo (nada nuevo que
  // pintar: la banda ya está en la pantalla) y solo vuelca al navegar, al
  // recomponer la opción o tras el clear de arriba.
  _scroller.blit(TEXT_SEL_TOP);
}

void Menu::firstPrint() {
  // Fondo y header (título)
  display.drawText(Config::Version::NAME, (Config::Screen::WIDTH - strlen(Config::Version::NAME) * 12) / 2, Config::Screen::HEADER_TOP, TEXT_12x16);

  // Cuadro de selección (banda de la opción actual)
  display.fillRect(0, BOX_TOP, display.getWidth(), BOX_HEIGHT, false);
  display.drawTextInverted(OPTION_TEXT[_selected], (Config::Screen::WIDTH - strlen(OPTION_TEXT[_selected]) * 12) / 2, BOX_TOP + 1, TEXT_12x16);

  // Pie (línea + Best/versión)
  display.fillRect(0, Config::Screen::FOOT_LINE, Config::Screen::WIDTH, 1, false);

  // Best score
  const char* label = "Best ";
  display.drawText(label, 0, Config::Screen::FOOT_TOP, TEXT_6x8);
  char buf[6];
  sprintf(buf, "%u", (unsigned)_bestScore);
  display.drawText(buf, strlen(label) * 6, Config::Screen::FOOT_TOP, TEXT_6x8);

  // Versión
  display.drawText(Config::Version::VERSION, Config::Screen::WIDTH - strlen(Config::Version::VERSION) * 6, Config::Screen::FOOT_TOP, TEXT_6x8);

  for (uint8_t selected = 0; selected < OPT_COUNT; selected++)
    drawDiamond(selected, selected == _selected, false);

  // // Rombos de posición
  // int8_t size = DIA_SIZE / 2;
  // int16_t y = DIA_TOP + DIA_SIZE;

  // // Dibujar rombos de posición (triángulos) centrados en la banda inferior
  // for (uint8_t selected = 0; selected < _optionCount; selected++) {
  //   int16_t x = (selected + 1) * floor(display.getWidth() / (_optionCount + 1));

  //   if (selected == _selected) {
  //     display.fillTriangle(x - size, y - size, x, y - DIA_SIZE, x + size, y - size, false);
  //     display.fillTriangle(x - size, y - size, x, y - 0, x + size, y - size, false);
  //   } else
  //     display.fillTriangle(x - size, y, x, y - size, x + size, y, false);
  // }
}

// ========================================================
// Accesos
// ========================================================

int8_t Menu::selected() const {
  return _selected;
}

int8_t Menu::confirm() const {
  // Se devuelve la OPCIÓN LÓGICA (enum Option): con "Continue" oculto la
  // lista es 4 opciones y los índices ya no coinciden con el enum, así el
  // Engine compara con los mismos valores (OPT_NEW/OPT_CONTINUE/
  // OPT_DIFFICULTY/OPT_SOUND/OPT_CREDITS) y es él quien abre la ventana
  // correspondiente.
  if (buttons.pressed(Buttons::ACTION_RIGHT))
    return (int8_t)optionAt(_selected);
  return -1;
}

void Menu::setBestScore(uint16_t value) {
  if (value != _bestScore) {
    _bestScore = value;
    _redraw = true;  // el pie cambia: se redibija al volver al menú
  }
}

// ====================================================================================
// Fin
// ====================================================================================

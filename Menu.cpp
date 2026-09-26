#include "esp32-hal.h"
#include "Menu.h"
#include "Globals.h"

#include <stdio.h>

// ========================================================
// Opciones por defecto
// ========================================================

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
// ========================================================

Menu::Option Menu::optionAt(int8_t index) const {
  if (_continueAvailable) return (Option)index;
  switch (index) {
    case 0:   return OPT_NEW;
    case 1:   return OPT_DIFFICULTY;
    case 2:   return OPT_SOUND;
    default:  return OPT_CREDITS;
  }
}

int8_t Menu::indexOfOption(Option option) const {
  if (_continueAvailable) return (int8_t)option;
  switch (option) {
    case OPT_NEW:      return 0;
    case OPT_DIFFICULTY: return 1;
    case OPT_SOUND:     return 2;
    case OPT_CREDITS:   return 3;
    default:             return -1;  // OPT_CONTINUE sin partida en curso
  }
}

// ========================================================
// Constructor
// ========================================================

Menu::Menu(uint16_t bestScore, const char* version)
  : _bestScore(bestScore),
    _version(version),
    _title("Snake II"),
    _showFooter(true),
    _optionCount(DEFAULT_OPTIONS - 1),  // sin "Continue" al arrancar (no hay partida)
    _optionTexts(NO_CONTINUE_OPTIONS),
    _continueAvailable(false),
    _selected(OPT_NEW),
    _hold(),
    _redraw(true),
    _editingSound(false),
    _soundEnabled(true),
    _editingDifficulty(false),
    _difficulty(Config::Difficulty::DEFAULT_LEVEL),
    _editDifficulty(Config::Difficulty::DEFAULT_LEVEL),
    _editBlink(),
    _repeat(),
    _repeatTick(),
    _scroller(1, nullptr) {}

// ========================================================
// Inicialización
// ========================================================

void Menu::begin() {
  _scroller.begin();
  _scroller.compose(optionText(_selected), TEXT_12x16);
  _hold.start();
  _redraw = true;
  _editingSound = false;
  _editingDifficulty = false;
}

// ========================================================
// Opciones (cantidad variable)
// ========================================================

void Menu::setOptions(const char* const* texts, uint8_t count) {
  if (texts == nullptr) return;

  if (count < 1) count = 1;
  if (count > MAX_OPTIONS) count = MAX_OPTIONS;

  _optionTexts = texts;
  _optionCount = count;

  if (_selected >= (int8_t)count) _selected = count - 1;

  _scroller.begin();
  _hold.start();
  _scroller.compose(optionText(_selected), TEXT_12x16);
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
  if (available == _continueAvailable) return;

  _continueAvailable = available;
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
  _hold.start();
  _scroller.compose(optionText(_selected), TEXT_12x16);
  _redraw = true;
  _editingSound = false;
  _editingDifficulty = false;
}

// ========================================================
// Actualizar (consume los eventos de botones leídos en loop())
// ========================================================

void Menu::update() {
  if (_editingSound) {
    // En modo edición de sonido: la tecla indicada por la flecha (la del
    // destino) cambia el valor mostrado (ON/OFF) sin aplicarlo; solo se
    // aplica al confirmar (btn2).
    if (buttons.pressed(Buttons::MOVE_LEFT) && _soundEnabled) {
      // ON: la flecha "<" (MOVE_LEFT) apaga
      _soundEnabled = false;
      sound.play(Sound::SFX_CLICK);
    }
    if (buttons.pressed(Buttons::MOVE_RIGHT) && !_soundEnabled) {
      // OFF: la flecha ">" (MOVE_RIGHT) enciende
      _soundEnabled = true;
      sound.play(Sound::SFX_CLICK);
    }
    if (buttons.pressed(Buttons::ACTION_RIGHT)) {
      // btn2 (Select): aplica el valor actual y vuelve al menú
      sound.setEnabled(_soundEnabled);
      if (_soundEnabled) sound.play(Sound::SFX_CONFIRM);
      endSoundEdit();
    } else if (buttons.pressed(Buttons::ACTION_UP)) {
      // btn1 (Back): cancela sin cambiar el estado y vuelve al menú
      sound.play(Sound::SFX_BACK);
      endSoundEdit();
    }
  } else if (_editingDifficulty) {
    // En modo edición de dificultad: MOVE_RIGHT +1, MOVE_LEFT -1 con
    // repetición al mantener presionado (primer paso inmediato, después
    // repite cada HOLD_REPEAT_TICK ms tras HOLD_REPEAT_DELAY de mantención).
    // El valor mostrado cambia sin aplicarse; solo se aplica al confirmar.
    if (holdRepeat(Buttons::MOVE_RIGHT) && _editDifficulty < Config::Difficulty::MAX_LEVEL) {
      _editDifficulty++;
      sound.play(Sound::SFX_CLICK);
    }
    if (holdRepeat(Buttons::MOVE_LEFT) && _editDifficulty > Config::Difficulty::MIN_LEVEL) {
      _editDifficulty--;
      sound.play(Sound::SFX_CLICK);
    }
    if (buttons.pressed(Buttons::ACTION_RIGHT)) {
      // btn2 (Select): aplica el valor y vuelve al menú
      _difficulty = _editDifficulty;
      sound.play(Sound::SFX_CONFIRM);
      endDifficultyEdit();
    } else if (buttons.pressed(Buttons::ACTION_UP)) {
      // btn1 (Back): cancela sin cambiar el estado y vuelve al menú
      sound.play(Sound::SFX_BACK);
      endDifficultyEdit();
    }
  } else {
    navigate();
    // Al confirmar la opción "Dificultad" (btn2) se entra en modo edición
    // inline, igual que "Sound" (ver Engine).
    if (buttons.pressed(Buttons::ACTION_RIGHT) && optionAt(_selected) == OPT_DIFFICULTY) {
      sound.play(Sound::SFX_CLICK);
      beginDifficultyEdit();
      return;
    }
    if (buttons.pressed(Buttons::ACTION_RIGHT) && optionAt(_selected) == OPT_SOUND) {
      sound.play(Sound::SFX_CLICK);
      beginSoundEdit();
      return;
    }
  }
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
    moved = true;
  }

  if (buttons.pressed(Buttons::MOVE_RIGHT) && _selected < _optionCount - 1) {
    _selected++;
    moved = true;
  }

  if (moved) {
    sound.play(Sound::SFX_CLICK);
    _scroller.compose(optionText(_selected), TEXT_12x16);
    _scroller.startSlide((_selected > before) ? 1 : -1);
    _hold.start();
    Serial.printf("Menu: opcion %d -> %d\n", before, _selected);
  }
}

// ========================================================
// Edición inline de Sonido
// ========================================================

void Menu::beginSoundEdit() {
  _editingSound = true;
  _soundEnabled = sound.enabled();
  _editBlink.start();
}

void Menu::endSoundEdit() {
  _editingSound = false;
  _redraw = true;
}

bool Menu::isEditingSound() const {
  return _editingSound;
}

// ========================================================
// Edición inline de Dificultad
// ========================================================

void Menu::beginDifficultyEdit() {
  _editingDifficulty = true;
  _editDifficulty = _difficulty;
  _editBlink.start();
}

void Menu::endDifficultyEdit() {
  _editingDifficulty = false;
  _redraw = true;
}

bool Menu::isEditingDifficulty() const {
  return _editingDifficulty;
}

uint8_t Menu::difficulty() const {
  return _difficulty;
}

// ========================================================
// Repetición por mantención (dificultad)
//
// Devuelve true cuando hay que aplicar el paso del botón en
// el modo de edición de dificultad: el primero es inmediato
// (evento pressed) y, manteniéndolo presionado, los siguientes
// cada HOLD_REPEAT_TICK ms a partir de HOLD_REPEAT_DELAY de
// mantención. Dos Stopwatch compartidos (`_repeat` = retardo,
// `_repeatTick` = cadencia): si se suelta y se vuelve a
// presionar, pressed() reinicia ambos.
// ========================================================

bool Menu::holdRepeat(uint8_t button) {
  if (buttons.pressed(button)) {
    _repeat.start();
    _repeatTick.start();
    return true;
  }

  if (buttons.state(button) &&
      _repeat.expired(HOLD_REPEAT_DELAY) &&
      _repeatTick.expired(HOLD_REPEAT_TICK)) {
    _repeatTick.start();
    return true;
  }

  return false;
}

// ========================================================
// Selector de sonido (modo edición de sonido)
//
// Reemplaza a los rombos de posición mientras se edita el
// sonido. Texto 6x8 centrado en mayúsculas con el MISMO
// ancho en ambos estados ("OFF" y "ON " miden 3 chars = 18
// px, así el centrado no se desplaza) y UNA flecha
// parpadeante, en el lado del destino:
//   - OFF: "OFF >"  (la flecha apunta a la tecla MOVE_RIGHT,
//     que enciende el sonido)
//   - ON:  "< ON"  (la flecha apunta a la tecla MOVE_LEFT,
//     que apaga el sonido)
// La flecha está pegada al texto (hueco ARROW_GAP) y parpadea
// visible 75% / oculto 25% de ARROW_BLINK_PERIOD ms; la
// palabra no parpadea. Ocupa la banda 45..53.
// ========================================================

void Menu::drawSoundSelector() {
  Adafruit_SSD1306& s = display.screen();

  // Palabra centrada. En ambos estados mide lo mismo: "ON " lleva un
  // espacio final para emparejar el ancho con "OFF" (18 px).
  const char* label = (_soundEnabled) ? "ON " : "OFF";
  int16_t labelW = display.getTextWidth(label, TEXT_6x8);
  int16_t labelX = (display.getWidth() - labelW) / 2;

  const int16_t yTop = DIA_TOP + 1;           // 46
  const int16_t yMid = DIA_TOP + DIA_SIZE / 2; // 49
  const int16_t yBot = DIA_TOP + DIA_SIZE;     // 53

  // Parpadeo de la flecha: visible el 75% del período, oculta el primer 25%
  // (anclado al beginSoundEdit: sin salto de fase con el reloj de 64 bits)
  bool arrowVisible =
      _editBlink.blinkOn(ARROW_BLINK_PERIOD, ARROW_BLINK_OFF_PCT);

  if (arrowVisible) {
    if (_soundEnabled) {
      // ON: flecha a la izquierda, punta hacia la izquierda ("< ON")
      int16_t base = labelX - ARROW_GAP;      // lado plano, pegado al texto
      s.fillTriangle(base - ARROW_W, yMid, base, yTop, base, yBot,
                     SSD1306_WHITE);
    } else {
      // OFF: flecha a la derecha, punta hacia la derecha ("OFF >")
      int16_t base = labelX + labelW + ARROW_GAP;  // lado plano, pegado al texto
      s.fillTriangle(base + ARROW_W, yMid, base, yTop, base, yBot,
                     SSD1306_WHITE);
    }
  }

  display.drawText(label, labelX, yTop, TEXT_6x8);
}

// ========================================================
// Selector de dificultad (modo edición de dificultad)
//
// Reemplaza a los rombos de posición mientras se edita la
// dificultad. Texto 6x8 con el número 1..10 centrado con
// ancho constante (1 dígito se alinea a la derecha con un
// espacio inicial: " 5" mide lo mismo que "10", 12 px, y el
// centrado no se desplaza) y DOS flechas parpadeantes a los
// lados, apuntando al exterior ("< 5 >"):
//   - flecha izquierda: MOVE_LEFT (-1), oculta en el mínimo
//   - flecha derecha:   MOVE_RIGHT (+1), oculta en el máximo
// Las flechas parpadean juntas (visible 75%, oculto 25% de
// ARROW_BLINK_PERIOD ms), pegadas al texto (hueco ARROW_GAP);
// el número no parpadea. Ocupa la banda 45..53.
//
// Al mantener presionado MOVE_LEFT o MOVE_RIGHT el paso se vuelve
// continuo y el dibujo lo refleja: se detiene el parpadeo, solo la
// flecha del botón activo queda fija y la contraria se oculta.
// ========================================================

void Menu::drawDifficultySelector() {
  Adafruit_SSD1306& s = display.screen();

  // Número centrado con ancho constante ("13" / " 5" = 12 px)
  char buf[8];
  if (_editDifficulty < 10) sprintf(buf, " %u", _editDifficulty);
  else                      sprintf(buf, "%u", _editDifficulty);

  int16_t labelW = display.getTextWidth(buf, TEXT_6x8);
  int16_t labelX = (display.getWidth() - labelW) / 2;

  const int16_t yTop = DIA_TOP + 1;            // 46
  const int16_t yMid = DIA_TOP + DIA_SIZE / 2; // 49
  const int16_t yBot = DIA_TOP + DIA_SIZE;     // 53

  // Al mantener presionado MOVE_LEFT (-1) o MOVE_RIGHT (+1) la repetición
  // continua queda marcada en pantalla: el parpadeo se detiene y SOLO la
  // flecha del botón activo se muestra (fija); la contraria se oculta.
  // Sin mantener, las dos flechas parpadean juntas (visible 75%, oculto
  // 25% de ARROW_BLINK_PERIOD ms) como siempre. Al llegar al límite (1 o
  // 25) el botón de ese lado ya no puede avanzar y se procesa igual que si
  // se hubiera soltado (vuelve el parpadeo normal, con el límite oculto).
  bool leftHeld  = buttons.state(Buttons::MOVE_LEFT) &&
                   _editDifficulty > Config::Difficulty::MIN_LEVEL;
  bool rightHeld = buttons.state(Buttons::MOVE_RIGHT) &&
                   _editDifficulty < Config::Difficulty::MAX_LEVEL;

  bool arrowsVisible =
      leftHeld || rightHeld ||
      _editBlink.blinkOn(ARROW_BLINK_PERIOD, ARROW_BLINK_OFF_PCT);

  if (arrowsVisible) {
    // Flecha izquierda (-1): fija al mantener MOVE_LEFT; oculta mientras se
    // mantiene MOVE_RIGHT; sin mantener parpadea. No se dibuja en el mínimo.
    if (_editDifficulty > Config::Difficulty::MIN_LEVEL && !rightHeld) {
      int16_t base = labelX - ARROW_GAP;  // lado plano, pegado al texto
      s.fillTriangle(base - ARROW_W, yMid, base, yTop, base, yBot,
                     SSD1306_WHITE);
    }
    // Flecha derecha (+1): fija al mantener MOVE_RIGHT; oculta mientras se
    // mantiene MOVE_LEFT; sin mantener parpadea. No se dibuja en el máximo.
    if (_editDifficulty < Config::Difficulty::MAX_LEVEL && !leftHeld) {
      int16_t base = labelX + labelW + ARROW_GAP;  // lado plano, pegado al texto
      s.fillTriangle(base + ARROW_W, yMid, base, yTop, base, yBot,
                     SSD1306_WHITE);
    }
  }

  display.drawText(buf, labelX, yTop, TEXT_6x8);
}

// ========================================================
// Rombos de posición
// ========================================================

void Menu::drawDiamonds() {
  uint8_t n = _optionCount;

  for (uint8_t i = 0; i < n; i++) {
    int16_t cx = (int16_t)((i + 1) * display.getWidth()) / (n + 1);
    int16_t x = cx - DIA_SIZE / 2;

    if (i == _selected) {
      // Parpadeo: visible 75% del período, oculto 25%. Anclado al
      // _hold (última selección): la fase depende solo del tiempo
      // desde la selección, no del instante absoluto del reloj.
      if (_hold.expired(BLINK_HOLD) &&
          !_hold.blinkOn(BLINK_PERIOD, BLINK_OFF_PCT))
        continue;  // fase oculta (25%): no se dibuja

      // Rombo simétrico de 9 filas (45..53), como el alimento del juego:
      // punta superior 45, hombros 49, punta inferior 53 (visible)
      Adafruit_SSD1306& s = display.screen();
      s.fillTriangle(x + 4, DIA_TOP, x + 8, DIA_TOP + DIA_SIZE / 2,
                     x + 4, DIA_TOP + DIA_SIZE, SSD1306_WHITE);
      s.fillTriangle(x + 4, DIA_TOP, x, DIA_TOP + DIA_SIZE / 2,
                     x + 4, DIA_TOP + DIA_SIZE, SSD1306_WHITE);
    } else {
      // Solo la punta (triángulo superior), apoyada sobre la línea separadora:
      // base en la 53 (sobre la línea de la 54), vértice en la 50
      int16_t baseY = DIA_TOP + DIA_SIZE;  // 53
      int16_t y = baseY - 3;               // 50
      display.screen().fillTriangle(x + 4, y, x, baseY, x + 8, baseY,
                                     SSD1306_WHITE);
    }
  }
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

    // Cuadro de selección: fijo, de ancho completo
    display.screen().fillRect(0, BOX_TOP, display.getWidth(), BOX_HEIGHT,
                               SSD1306_WHITE);

    // Header: título
    display.drawTextAligned(_title, CENTER, TEXT_12x16, REGION_HEADER);

    // Pie del Body: línea separadora + texto (Best/versión), opcional
    // (cuando se oculta solo queda la línea que sostiene los rombos)
    display.screen().drawFastHLine(0, PIE_LINE_ROW, display.getWidth(),
                                    SSD1306_WHITE);
    if (_showFooter) {
      char buf[16];
      sprintf(buf, "Best: %u", (unsigned)_bestScore);

      TextPos tPos = display.getTextPos(buf, LEFT_DOWN, TEXT_6x8, REGION_BODY);
      display.drawText(buf, tPos.x, PIE_TOP, TEXT_6x8);

      TextPos vPos = display.getTextPos(_version, RIGHT_DOWN, TEXT_6x8, REGION_BODY);
      display.drawText(_version, vPos.x, PIE_TOP, TEXT_6x8);
    }

    _redraw = false;

    // El clear se ha llevado por delante la banda de la opción que el
    // scroller tenía volcada: hay que volver a pintarla (aunque la tira
    // esté centrada y no se mueva).
    _scroller.invalidate();
  }

  // Dinámicos (cada frame): la banda de la opción deslizante y los rombos.
  // La banda persistente (lo que ya está en pantalla) se mantiene intacta;
  // la tira nueva desliza y sus columnas sobrescriben la banda hasta
  // reemplazarla por completo. La opción anterior permanece hasta ser borrada.
  // El scroller se salta este volcado cuando está en reposo (nada nuevo que
  // pintar: la banda ya está en la pantalla) y solo vuelca al navegar, al
  // recomponer la opción o tras el clear de arriba.
  _scroller.blit(0, TEXT_SEL_TOP, SSD1306_BLACK, SSD1306_WHITE);

  if (_editingSound) {
    // Modo edición de sonido: la banda del selector (45..53) se borra y se
    // vuelve a dibujar en cada frame (la flecha parpadea; la palabra no).
    display.screen().fillRect(0, DIA_TOP, display.getWidth(),
                               DIA_SIZE + 1, SSD1306_BLACK);
    drawSoundSelector();
  } else if (_editingDifficulty) {
    // Modo edición de dificultad: la banda del selector (45..53) se borra y
    // se vuelve a dibujar en cada frame (las flechas parpadean; el número no).
    display.screen().fillRect(0, DIA_TOP, display.getWidth(),
                               DIA_SIZE + 1, SSD1306_BLACK);
    drawDifficultySelector();
  } else {
    // Solo se borra la banda de rombos (45..53), la única zona dinámica restante
    display.screen().fillRect(0, DIA_TOP, display.getWidth(),
                               DIA_SIZE + 1, SSD1306_BLACK);
    drawDiamonds();
  }
}

// ========================================================
// Accesos
// ========================================================

int8_t Menu::selected() const {
  return _selected;
}

int8_t Menu::confirm() const {
  // Las opciones "Sound" y "Dificultad" NO se devuelven para abrir otra
  // ventana: se editan inline en el propio menú (ver beginSoundEdit /
  // beginDifficultyEdit y update). Se devuelve la OPCIÓN LÓGICA (enum
  // Option): con "Continue" oculto la lista es 4 opciones y los índices
  // ya no coinciden con el enum, así el Engine compara con los mismos
  // valores (OPT_NEW/OPT_CONTINUE/OPT_CREDITS).
  if (buttons.pressed(Buttons::ACTION_RIGHT) &&
      optionAt(_selected) != OPT_SOUND &&
      optionAt(_selected) != OPT_DIFFICULTY)
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

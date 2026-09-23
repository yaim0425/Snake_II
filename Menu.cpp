#include "esp32-hal.h"
#include "Menu.h"

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

const char* Menu::optionText(int8_t index) const {
  return _optionTexts[index];
}

// ========================================================
// Constructor
// ========================================================

Menu::Menu(Display& display, Buttons& buttons, Sound& sound, uint8_t topScore,
           const char* version)
  : _display(display),
    _buttons(buttons),
    _sound(sound),
    _topScore(topScore),
    _version(version),
    _title("Snake II"),
    _showFooter(true),
    _optionCount(DEFAULT_OPTIONS),
    _optionTexts(DEFAULT_OPTION_TEXT),
    _selected(OPC_NUEVO),
    _holdStart(0),
    _redraw(true),
    _editingSound(false),
    _soundEnabled(true),
    _scroller(display, 1, nullptr) {}

// ========================================================
// Inicialización
// ========================================================

void Menu::begin() {
  _scroller.begin();
  _selected = 0;
  _scroller.compose(optionText(_selected), TEXT_12x16);
  _holdStart = millis();
  _redraw = true;
  _editingSound = false;
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
  _holdStart = millis();
  _scroller.compose(optionText(_selected), TEXT_12x16);
  _redraw = true;
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

void Menu::setSelected(int8_t index) {
  if (index < 0) index = 0;
  if (index >= (int8_t)_optionCount) index = (int8_t)_optionCount - 1;

  _selected = index;
  _scroller.begin();
  _holdStart = millis();
  _scroller.compose(optionText(_selected), TEXT_12x16);
  _redraw = true;
  _editingSound = false;
}

// ========================================================
// Actualizar (consume los eventos de botones leídos en loop())
// ========================================================

void Menu::update() {
  if (_editingSound) {
    // En modo edición de sonido: MOVE_LEFT/MOVE_RIGHT cambian el valor
    // mostrado (On/Off) sin aplicarlo; solo se aplica al confirmar (btn2).
    if (_buttons.pressed(Buttons::MOVE_LEFT) && _soundEnabled) {
      _soundEnabled = false;
      _sound.play(Sound::SFX_CLICK);
    }
    if (_buttons.pressed(Buttons::MOVE_RIGHT) && !_soundEnabled) {
      _soundEnabled = true;
      _sound.play(Sound::SFX_CLICK);
    }
    if (_buttons.actionRightPressed()) {
      // btn2 (Select): aplica el valor actual y vuelve al menú
      _sound.setEnabled(_soundEnabled);
      if (_soundEnabled) _sound.play(Sound::SFX_CONFIRM);
      endSoundEdit();
    } else if (_buttons.actionUpPressed()) {
      // btn1 (Back): cancela sin cambiar el estado y vuelve al menú
      _sound.play(Sound::SFX_BACK);
      endSoundEdit();
    }
  } else {
    navigate();
    // Al confirmar la opción "Sound" (btn2) se entra en modo edición
    // inline; ya no se abre SoundWindow (ver Engine).
    if (_buttons.actionRightPressed() && _selected == OPC_SONIDO) {
      _sound.play(Sound::SFX_CLICK);
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
  if (_buttons.pressed(Buttons::MOVE_LEFT) && _selected > 0) {
    _selected--;
    moved = true;
  }

  if (_buttons.pressed(Buttons::MOVE_RIGHT) && _selected < _optionCount - 1) {
    _selected++;
    moved = true;
  }

  if (moved) {
    _sound.play(Sound::SFX_CLICK);
    _scroller.compose(optionText(_selected), TEXT_12x16);
    _scroller.startSlide((_selected > before) ? 1 : -1);
    _holdStart = millis();
    Serial.printf("Menu: opcion %d -> %d\n", before, _selected);
  }
}

// ========================================================
// Edición inline de Sonido
// ========================================================

void Menu::beginSoundEdit() {
  _editingSound = true;
  _soundEnabled = _sound.enabled();
}

void Menu::endSoundEdit() {
  _editingSound = false;
  _redraw = true;
}

bool Menu::isEditingSound() const {
  return _editingSound;
}

// ========================================================
// Selector On/Off (modo edición de sonido)
//
// Reemplaza a los rombos de posición mientras se edita el
// sonido: "On" y "Off" en 6x8 con un solo espacio entre ambos;
// la palabra activa (el valor actual) queda centrada entre las
// flechas, que apuntan hacia adentro y son intermitentes.
// Ocupa la misma banda 45..53.
// ========================================================

void Menu::drawSoundSelector() {
  Adafruit_SSD1306& s = _display.screen();

  const int16_t yTop = DIA_TOP + 1;           // 46
  const int16_t yMid = DIA_TOP + DIA_SIZE / 2; // 49
  const int16_t yBot = DIA_TOP + DIA_SIZE;     // 53

  // Flechas intermitentes: visibles 75% del período, ocultas 25%
  bool hidden = (millis() % BLINK_PERIOD) <
                (uint32_t)BLINK_PERIOD * BLINK_OFF_PCT / 100;

  if (!hidden) {
    // Flecha izquierda: punta orientada al centro (apunta al valor actual)
    s.fillTriangle(30, yMid, 24, yTop, 24, yBot, SSD1306_WHITE);

    // Flecha derecha: punta orientada al centro (apunta al valor actual)
    s.fillTriangle(98, yMid, 104, yTop, 104, yBot, SSD1306_WHITE);
  }

  // "On" y "Off" con un solo espacio de separación. La palabra activa (On u
  // Off) queda centrada entre las flechas: el bloque se desplaza de modo que
  // el centro de la palabra activa caiga en el punto medio de las flechas (64).
  const char* label = "On Off";
  uint8_t onW   = _display.getTextWidth("On", TEXT_6x8);         // 2 chars
  uint8_t offW  = _display.getTextWidth("Off", TEXT_6x8);        // 3 chars
  uint8_t spaceW = _display.getTextWidth(" ", TEXT_6x8);         // 1 char

  // Centro de la palabra activa dentro del bloque "On Off":
  //  - On  activo: centro en onW/2                        (On va primero)
  //  - Off activo: centro en onW + spaceW + offW/2        (tras "On ")
  int16_t activeCenter = _display.getWidth() / 2;  // 64: punto medio de las flechas
  int16_t labelX;
  if (_soundEnabled) {
    labelX = activeCenter - onW / 2;
  } else {
    labelX = activeCenter - (onW + spaceW + offW / 2);
  }

  _display.drawText(label, labelX, yTop, TEXT_6x8);
}

// ========================================================
// Rombos de posición
// ========================================================

void Menu::drawDiamonds() {
  uint8_t n = _optionCount;

  for (uint8_t i = 0; i < n; i++) {
    int16_t cx = (int16_t)((i + 1) * _display.getWidth()) / (n + 1);
    int16_t x = cx - DIA_SIZE / 2;

    if (i == _selected) {
      // Parpadeo: visible 75% del período, oculto 25%
      if (millis() - _holdStart >= BLINK_HOLD &&
          (millis() % BLINK_PERIOD) < (uint32_t)BLINK_PERIOD * BLINK_OFF_PCT / 100)
        continue;  // fase oculta (25%): no se dibuja

      // Rombo simétrico de 9 filas (45..53), como el alimento del juego:
      // punta superior 45, hombros 49, punta inferior 53 (visible)
      Adafruit_SSD1306& s = _display.screen();
      s.fillTriangle(x + 4, DIA_TOP, x + 8, DIA_TOP + DIA_SIZE / 2,
                     x + 4, DIA_TOP + DIA_SIZE, SSD1306_WHITE);
      s.fillTriangle(x + 4, DIA_TOP, x, DIA_TOP + DIA_SIZE / 2,
                     x + 4, DIA_TOP + DIA_SIZE, SSD1306_WHITE);
    } else {
      // Solo la punta (triángulo superior), apoyada sobre la línea separadora:
      // base en la 53 (sobre la línea de la 54), vértice en la 50
      int16_t baseY = DIA_TOP + DIA_SIZE;  // 53
      int16_t y = baseY - 3;               // 50
      _display.screen().fillTriangle(x + 4, y, x, baseY, x + 8, baseY,
                                     SSD1306_WHITE);
    }
  }
}

// ========================================================
// Dibujar
// ========================================================

void Menu::print() {

  // Estáticos (solo al entrar, tras el clear() completo): fondo, cuadro
  // de selección, header (título) y pie (línea + Top/versión). Se dibujan
  // UNA sola vez; ya no se borran ni se redibujan en cada frame.
  if (_redraw) {
    _display.clear();

    // Cuadro de selección: fijo, de ancho completo
    _display.screen().fillRect(0, BOX_TOP, _display.getWidth(), BOX_HEIGHT,
                               SSD1306_WHITE);

    // Header: título
    _display.drawTextAligned(_title, CENTER, TEXT_12x16, REGION_HEADER);

    // Pie del Body: línea separadora + texto (Top/versión), opcional
    // (cuando se oculta solo queda la línea que sostiene los rombos)
    _display.screen().drawFastHLine(0, PIE_LINE_ROW, _display.getWidth(),
                                    SSD1306_WHITE);
    if (_showFooter) {
      char buf[16];
      sprintf(buf, "Top: %u pts", _topScore);

      TextPos tPos = _display.getTextPos(buf, LEFT_DOWN, TEXT_6x8, REGION_BODY);
      _display.drawText(buf, tPos.x, PIE_TOP, TEXT_6x8);

      TextPos vPos = _display.getTextPos(_version, RIGHT_DOWN, TEXT_6x8, REGION_BODY);
      _display.drawText(_version, vPos.x, PIE_TOP, TEXT_6x8);
    }

    _redraw = false;
  }

  // Dinámicos (cada frame): la banda de la opción deslizante y los rombos.
  // La banda persistente (lo que ya está en pantalla) se mantiene intacta;
  // la tira nueva desliza y sus columnas sobrescriben la banda hasta
  // reemplazarla por completo. La opción anterior permanece hasta ser borrada
  _scroller.blit(0, TEXT_SEL_TOP, SSD1306_BLACK, SSD1306_WHITE);

  if (_editingSound) {
    // Modo edición de sonido: se borra/redibuja la banda del selector
    // (45..53) en cada frame porque las flechas parpadean.
    _display.screen().fillRect(0, DIA_TOP, _display.getWidth(),
                               DIA_SIZE + 1, SSD1306_BLACK);
    drawSoundSelector();
  } else {
    // Solo se borra la banda de rombos (45..53), la única zona dinámica restante
    _display.screen().fillRect(0, DIA_TOP, _display.getWidth(),
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
  // La opción "Sound" NO se devuelve para abrir otra ventana: se edita
  // inline en el propio menú (ver beginSoundEdit/update). Con lo demás
  // se confirma como siempre (ACTION_RIGHT).
  if (_buttons.actionRightPressed() && _selected != OPC_SONIDO) return _selected;
  return -1;
}

void Menu::setTopScore(uint8_t value) {
  if (value != _topScore) {
    _topScore = value;
    _redraw = true;  // el pie cambia: se redibija al volver al menú
  }
}
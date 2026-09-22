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
    _scroller(display, 1, nullptr) {}

// ========================================================
// Inicialización
// ========================================================

void Menu::begin() {
  _scroller.begin();
  _holdStart = millis();
  _scroller.compose(optionText(_selected), TEXT_12x16);
  _redraw = true;
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
// de la animación. La usa SoundWindow para reflejar el estado
// del sonido al entrar (0 = On, 1 = Off).
// ========================================================

void Menu::setSelected(int8_t index) {
  if (index < 0) index = 0;
  if (index >= (int8_t)_optionCount) index = (int8_t)_optionCount - 1;

  _selected = index;
  _scroller.begin();
  _holdStart = millis();
  _scroller.compose(optionText(_selected), TEXT_12x16);
  _redraw = true;
}

// ========================================================
// Actualizar (consume los eventos de botones leídos en loop())
// ========================================================

void Menu::update() {
  navigate();
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

    // Pie: línea separadora + texto (Top/versión), opcional
    // (SoundWindow lo oculta: solo deja la línea que sostiene los rombos)
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

  // Solo se borra la banda de rombos (45..53), la única zona dinámica restante
  _display.screen().fillRect(0, DIA_TOP, _display.getWidth(),
                             DIA_SIZE + 1, SSD1306_BLACK);
  drawDiamonds();
}

// ========================================================
// Accesos
// ========================================================

int8_t Menu::selected() const {
  return _selected;
}

int8_t Menu::confirm() const {
  if (_buttons.actionRightPressed()) return _selected;
  return -1;
}

void Menu::setTopScore(uint8_t value) {
  if (value != _topScore) {
    _topScore = value;
    _redraw = true;  // el pie cambia: se redibija al volver al menú
  }
}
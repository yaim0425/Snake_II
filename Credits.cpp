#include "esp32-hal.h"
#include "Credits.h"
#include "Globals.h"

#include "Config.h"

static const char* const ROLE_NAME[Credits::NUM_ENTRIES][2] = {
  { "Dev",      "opencode.ai" },
  { "Snake II", "v0.1"        },
  { "Director", "YAIM904"     }
};

static constexpr int16_t PIE_TOP = 54;

Credits::Credits()
  : _entry(1),
    _exit(false),
    _redraw(true) {}

void Credits::begin() {
  _entry = 1;
  _scrollerRol.begin();
  _scrollerNombre.begin();
  loadEntry();
  _exit = false;
  _redraw = true;
}

void Credits::update() {
  navigate();
  _scrollerRol.animate();
  _scrollerNombre.animate();
}

void Credits::navigate() {
  uint8_t before = _entry;
  bool moved = false;

  if (buttons.pressed(Buttons::MOVE_LEFT) && _entry > 0) {
    _entry--;
    moved = true;
  }

  if (buttons.pressed(Buttons::MOVE_RIGHT) && _entry < NUM_ENTRIES - 1) {
    _entry++;
    moved = true;
  }

  if (moved) {
    sound.play(Sound::SFX_CLICK);
    loadEntry();
    int8_t dir = (_entry > before) ? 1 : -1;
    _scrollerRol.startSlide(dir);
    _scrollerNombre.startSlide(dir);
    Serial.printf("Credits: opcion %u -> %u\n", before, _entry);
  }

  if (buttons.pressed(Buttons::ACTION_UP)) _exit = true;
}

void Credits::loadEntry() {
  _scrollerRol.setTexto(ROLE_NAME[_entry][0], 16, TEXT_12x16);
  _scrollerNombre.setTexto(ROLE_NAME[_entry][1], 8, TEXT_6x8);
}

void Credits::print() {
  int16_t w = display.getWidth();
  int16_t bodyTop = (int16_t)Config::Screen::BODY_TOP;
  int16_t roleH = display.getTextHeight(TEXT_12x16);
  int16_t roleY = bodyTop + (PIE_TOP - bodyTop - roleH) / 2;

  if (_redraw) {
    display.clear();

    display.drawTextAligned("Credits", CENTER, TEXT_12x16, REGION_HEADER);

    display.screen().fillRect(0, roleY - 1, w, roleH + 2, SSD1306_WHITE);

    _redraw = false;

    _scrollerRol.redraw();
    _scrollerNombre.redraw();
  }

  _scrollerRol.blit(roleY);

  int16_t nameH = display.getTextHeight(TEXT_6x8);
  int16_t nameY = PIE_TOP + 1;
  display.screen().fillRect(0, nameY - 1, w, nameH + 2, SSD1306_WHITE);
  _scrollerNombre.blit(nameY);
}

bool Credits::done() const {
  return _exit;
}

// ====================================================================================
// Fin
// ====================================================================================

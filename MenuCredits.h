#ifndef MENU_CREDITS_H
#define MENU_CREDITS_H

#include <Arduino.h>

#include "Scroller.h"

class MenuCredits {
public:

  static constexpr uint8_t NUM_ENTRIES = 3;

  MenuCredits();

  void begin();

  void update();

  void print();

  bool done() const;

private:

  uint8_t _entry;
  bool _exit;
  bool _redraw;

  Scroller _scrollerRol;
  Scroller _scrollerNombre;

  void navigate();
  void loadEntry();
};

#endif

// ====================================================================================
// Fin
// ====================================================================================

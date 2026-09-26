#ifndef CREDITS_H
#define CREDITS_H

#include <Arduino.h>

#include "Scroller.h"

class Credits {
public:

  // ========================================================
  // Configuración
  // ========================================================

  static constexpr uint8_t NUM_ENTRIES = 3;

  // ========================================================
  // Bandas del Scroller (2 sincronizadas)
  // ========================================================

  static constexpr uint8_t NUM_SLOTS   = 2;   // bandas del Scroller
  static constexpr uint8_t SLOT_ROLE   = 0;   // rol 12x16 (banda del cuadro blanco)
  static constexpr uint8_t SLOT_NAME   = 1;   // nombre 6x8 (banda del pie)

  // ========================================================
  // Constructor (sin parámetros de servicios: usa los globales
  // Display, Buttons y Sound — Globals.h)
  // ========================================================

  Credits();

  // ========================================================
  // Inicialización (al entrar en la ventana)
  // ========================================================

  void begin();

  // ========================================================
  // Actualizar (consume eventos de botones, navega y anima)
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

  // ========================================================
  // Estado
  // ========================================================

  uint8_t _entry;   // entrada visible (0 = izquierda, 1 = centro, 2 = derecha)
  bool _exit;
  bool _redraw;     // primer frame tras begin(): clear() completo + estáticos

  // ========================================================
  // Scroller de 1 bit compartido (igual que el menú): DOS bandas
  // sincronizadas (deslizan con el mismo _slideX). El rol
  // (12x16) y el nombre (6x8) se componen por separado en la
  // tira de cada banda —cada banda del Scroller tiene la suya—
  // y cada una sobrescribe columna a columna su canvas
  // persistente: la entrada anterior se mantiene hasta que la
  // nueva la cubre. El deslizamiento arranca desde el borde.
  // ========================================================

  Scroller _scroller;

  // ========================================================
  // Métodos internos
  // ========================================================

  void navigate();
  void loadEntry();  // compone rol y nombre de la entrada actual (solo al entrar o al navegar)

  // Vuelca la banda persistente del slot (ya compuesta)
  void drawBand(uint8_t slot, int16_t y, uint16_t fgColor, uint16_t bgColor);
};

#endif

// ====================================================================================
// Fin
// ====================================================================================
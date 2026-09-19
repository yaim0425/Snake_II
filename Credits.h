#ifndef CREDITS_H
#define CREDITS_H

#include "Display.h"
#include "Buttons.h"
#include <Adafruit_GFX.h>

class Credits {
public:

  // ========================================================
  // Configuración
  // ========================================================

  static constexpr uint8_t NUM_ENTRIES = 3;

  // ========================================================
  // Constructor
  // ========================================================

  Credits(Display& display, Buttons& buttons);

  // ========================================================
  // Inicialización (al entrar en la ventana)
  // ========================================================

  void begin();

  // ========================================================
  // Actualizar (lee botones, navega y anima)
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
  // Animación lateral (espejo del menú): distancia del salto y velocidad
  // ========================================================

  static constexpr int16_t SLIDE_DIST = 48;
  static constexpr uint32_t ANIM_TICK = 15;
  static constexpr int8_t ANIM_STEP   = 2;

  // ========================================================
  // Dependencias
  // ========================================================

  Display& _display;
  Buttons& _buttons;

  // ========================================================
  // Estado
  // ========================================================

  uint8_t _entry;   // entrada visible (0 = izquierda, 1 = centro, 2 = derecha)
  uint8_t _prev;    // entrada previa (se borra desde la dirección contraria)
  int8_t _dir;      // +1 siguiente (entra por la derecha), -1 anterior (izquierda)
  int16_t _slideX;  // desplazamiento de la entrada entrante (objetivo 0)
  uint32_t _animLast;
  bool _exit;

  GFXcanvas8 _band;  // canvas de una banda (recorta el texto al deslizar)

  // ========================================================
  // Métodos internos
  // ========================================================

  void navigate();
  void startSlide(int8_t dir);
  void animate();
  void wipeOld(int16_t offX, int16_t y, uint8_t h, uint16_t color);
  void paintText(const char* text, int16_t x, uint8_t size);
  void blitBand(const char* text, int16_t x, uint8_t size, int16_t y,
                uint16_t color);
};

#endif
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
  // Animación lateral (igual que el menú): arranca desde el
  // borde (ancho completo de pantalla) y avanza 1 px por cada
  // ANIM_TICK ms, con acumulador por tiempo (constante aunque
  // el loop sea lento). Vuelo total ~128*4 ms ≈ 0,5 s.
  // ========================================================

  static constexpr uint32_t ANIM_TICK = 4;   // ms por píxel (igual que el menú)

  // ========================================================
  // Scroller de 1 bit (igual que el menú): DOS bandas
  // sincronizadas. El rol (12x16) y el nombre (6x8) se componen
  // por separado en la misma tira (usando STRIP_H = alto del
  // texto 12x16; el nombre ocupa sus 8 primeras filas) y
  // deslizan a la vez con el mismo _slideX, sobrescribiendo
  // columna a columna cada banda persistente: la entrada
  // anterior se mantiene hasta que la nueva la cubre
  // ========================================================

  static constexpr uint8_t STRIP_W = 128;  // columnas de la tira (= ancho de pantalla)
  static constexpr uint8_t STRIP_H = 16;   // filas de la tira (= alto del texto 12x16)

  // ========================================================
  // Dependencias
  // ========================================================

  Display& _display;
  Buttons& _buttons;

  // ========================================================
  // Estado
  // ========================================================

  uint8_t _entry;   // entrada visible (0 = izquierda, 1 = centro, 2 = derecha)
  int8_t _dir;      // +1 siguiente (entra por la derecha), -1 anterior (izquierda)
  int16_t _slideX;  // desplazamiento de la entrada entrante (objetivo 0)
  uint32_t _animLast;
  uint32_t _colAcc;   // acumulador de tiempo para avanzar píxeles
  bool _exit;

  uint8_t _strip[STRIP_H][STRIP_W / 8];  // matriz 128x16 de 1 bit de la entrada entrante
  GFXcanvas8 _chipBoxRole;               // banda persistente del rol (128x16): lo que está en pantalla
  GFXcanvas8 _chipBoxName;               // banda persistente del nombre (128x8)
  GFXcanvas8 _composer;                  // canvas auxiliar (128x16) para componer la entrada

  // ========================================================
  // Métodos internos
  // ========================================================

  void navigate();
  void startSlide(int8_t dir);
  void animate();
  void loadEntry(uint8_t slot);                     // compone rol/nombre en la matriz de 1 bit
  void slideStrip(GFXcanvas8& chipBox, uint8_t h);  // pinta las columnas visibles de la tira sobre la banda
  void blitBand(GFXcanvas8& chipBox, int16_t y, uint8_t h,
                uint16_t fgColor, uint16_t bgColor);  // vuelca la banda a la pantalla
  void drawBand(uint8_t slot, int16_t y, uint8_t size,
                uint16_t fgColor, uint16_t bgColor);
};

#endif
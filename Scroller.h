#ifndef SCROLLER_H
#define SCROLLER_H

#include <Arduino.h>
#include <Adafruit_GFX.h>

#include "Timer.h"

// ========================================================
// Scroller — animación "scroller de 1 bit" (compartida por
// Menu y Credits; antes estaba duplicada en ambas clases)
//
// Una tira de 128x16 px se compone centrada en una matriz de 1
// bit (1 = glifo, 0 = fondo), desliza lateralmente (1 px cada
// ANIM_TICK ms, acumulado por tiempo: constante aunque el loop
// sea lento) y sobrescribe columna a columna una banda
// persistente. Así la opción/entrada anterior se mantiene en
// pantalla hasta que la nueva la cubre (superposición al
// navegar rápido).
//
// Soporta N bandas que deslizan a la vez con el MISMO _slideX
// (Créditos: rol 12x16 + nombre 6x8 sincronizados). Cada banda
// tiene su canvas persistente (`_chipBox`), su alto y sus
// colores de volcado (fg/bg) propios; la tira se compone con
// compose() y cada banda se vuelca con blit().
//
// El volcado solo ocurre cuando hay algo nuevo que pintar: un
// dirty flag (`_dirty`) lo ponen begin(), startSlide(), el
// avance de la animación e invalidate() (la ventana lo llama
// tras su clear(), que se lleva por delante lo ya volcado), y
// lo limpia blit() cuando todas las bandas se han volcado en el
// frame. En reposo (tira centrada, sin animación y sin clear
// detrás) blit() no hace nada: la banda ya está en pantalla y
// no hace falta repintar sus 2048 px en cada frame.
// ========================================================

class Scroller {
public:

  // ========================================================
  // Constructor
  // ========================================================
  // `bands`      = cantidad de bandas que deslizan juntas
  //                (1 = menú; 2 = créditos).
  // `bandHeights`= alto en px de cada banda (si nullptr, todas
  //                usan STRIP_H = 16).
  // Usa la Display global (Globals.h) para el ancho de la tira.
  // Los canvas se reservan en el constructor (como antes).

  Scroller(uint8_t bands = 1,
           const uint8_t* bandHeights = nullptr);

  Scroller(const Scroller&) = delete;            // no copiar (reserva con new)
  Scroller& operator=(const Scroller&) = delete; // ni asignar (doble delete)

  ~Scroller();

  // ========================================================
  // Inicialización (al entrar en la ventana: tira centrada)
  // ========================================================

  void begin();

  // ========================================================
  // Composición de la tira (texto centrado en 128 px)
  // ========================================================

  void compose(const char* text, uint8_t size);

  // ========================================================
  // Animación lateral (arranca desde el borde, ±ancho)
  // ========================================================

  void startSlide(int8_t dir);

  void animate();

  // ========================================================
  // Forzar el volcado de las bandas en el próximo frame: la
  // ventana lo llama tras su `display.clear()` (el clear se
  // lleva por delante lo que ya estaba volcado, así que la
  // banda hay que volver a pintarla aunque la tira esté en
  // reposo).
  // ========================================================

  void invalidate();

  // ========================================================
  // Volcado de una banda: pinta las columnas visibles de la
  // tira sobre la banda persistente `band` y la vuelca a la
  // pantalla en la fila `y` (fila superior de la banda), con
  // el glifo en `fgColor` y el fondo en `bgColor`.
  //
  // Es un no-op si no hay nada nuevo que volcar (`_dirty` a 0):
  // en reposo la banda ya coincide con la pantalla.
  // ========================================================

  void blit(uint8_t band, int16_t y, uint16_t fgColor, uint16_t bgColor);

private:

  // ========================================================
  // Geometría y tiempos
  // ========================================================

  static constexpr uint8_t  STRIP_W   = 128;  // columnas de la tira (= ancho de pantalla)
  static constexpr uint8_t  STRIP_H   = 16;   // filas de la tira (máx.: alto del texto 12x16)
  static constexpr uint32_t ANIM_TICK = 4;    // ms por píxel de desplazamiento (~0,5 s)

  // Valores del canvas: 1 = glifo (texto), 255 = fondo/chip
  static constexpr uint8_t CHIP_TEXT = 1;
  static constexpr uint8_t CHIP_BG   = 255;

  // ========================================================
  // Bandas (una por canvas persistente)
  // ========================================================

  uint8_t _bands;
  uint8_t* _bandHeights;   // alto en px de cada banda
  GFXcanvas8** _chipBox;   // banda persistente por banda: lo que está en pantalla

  // ========================================================
  // Tira y compositor
  // ========================================================

  uint8_t _strip[STRIP_H][STRIP_W / 8];  // matriz 128x16 de 1 bit de la tira entrante
  GFXcanvas8 _composer;                  // canvas auxiliar (128x16) para componer la tira

  // ========================================================
  // Estado del deslizamiento
  // ========================================================

  int8_t _dir;        // +1 entra por la derecha (mueve hacia la izquierda), -1 al revés
  int16_t _slideX;    // borde izquierdo de la tira en pantalla (objetivo 0 = centrada)
  Ticker _ticker;     // avance de 1 px por ANIM_TICK ms (acumulador por tiempo)
  bool _dirty;        // hay algo nuevo que volcar a pantalla (o la ventana limpió detrás)
  uint8_t _blitted;   // bandas ya volcadas en el frame actual (limpia _dirty al llegar a _bands)

  // ========================================================
  // Internos
  // ========================================================

  // Pintar las columnas visibles de la tira sobre una banda
  // persistente (incluidos sus espacios de fondo)
  void slideStrip(GFXcanvas8& chipBox, uint8_t h);
};

#endif

// ====================================================================================
// Fin
// ====================================================================================
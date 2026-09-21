#ifndef BOOT_H
#define BOOT_H

#include "Display.h"
#include "Buttons.h"

// ========================================================
// Boot — animación de arranque (franjas verticales)
//
// La pantalla se divide en dos bandas completas: TITULO
// (filas 0..15) y CUERPO (filas 16..63). Cada banda se llena
// con líneas verticales de 3 px de grosor, separadas 8 px,
// que se desplazan:
//   - TITULO: de izquierda a derecha.
//   - CUERPO: de derecha a izquierda.
//
// Las líneas avanzan 1 px cada ANIM_TICK ms (con acumulador
// por tiempo, igual que el menú). Al llegar al borde derecho,
// la línea se parte en dos tramos (derecho + izquierdo) para
// no cortarse en seco. La animación dura en total TOTAL_MS;
// cualquier botón la termina antes. Al terminar, done() devuelve
// true y Engine pasa al menú.
// ========================================================

class Boot {
public:
  // ========================================================
  // Constructor
  // ========================================================

  Boot(Display& display, Buttons& buttons);

  // ========================================================
  // Inicialización (al entrar en la ventana)
  // ========================================================

  void begin();

  // ========================================================
  // Actualizar (lee botones y avanza el desplazamiento)
  // ========================================================

  void update();

  // ========================================================
  // Dibujar (bandas de líneas verticales)
  // ========================================================

  void print();

  // ========================================================
  // Salida (true = la animación terminó)
  // ========================================================

  bool done() const;

private:
  // ========================================================
  // Geometría
  // ========================================================

  // Grosor de cada línea vertical
  static constexpr uint8_t BAR_W = 3;

  // Separación entre líneas (el desplazamiento repite cada traslación)
  static constexpr uint8_t BAR_SPACING = 8;

  // Banda TITULO (banda completa 0..15)
  static constexpr uint8_t TITLE_TOP = 0;
  static constexpr uint8_t TITLE_H = 16;

  // Banda CUERPO (banda completa 16..63)
  static constexpr uint8_t BODY_TOP = 16;
  static constexpr uint8_t BODY_H = 48;

  // ========================================================
  // Tiempo
  // ========================================================

  // Avance de 1 px cada ANIM_TICK ms
  static constexpr uint32_t ANIM_TICK = 30;

  // Duración total de la animación
  static constexpr uint32_t TOTAL_MS = 4000;

  // ========================================================
  // Estado interno
  // ========================================================

  Display& _display;
  Buttons& _buttons;

  uint8_t _shift;       // desplazamiento actual (0..BAR_SPACING-1)
  uint32_t _tickAccum;  // acumulador de tiempo para el avance
  uint32_t _lastMs;     // último millis() leído
  uint32_t _startMs;    // millis() al entrar en la ventana
  bool _done;

  // ========================================================
  // Helper
  // ========================================================

  // Dibuja una línea vertical de BAR_W px (con rebalse por el borde derecho)
  void drawBar(int16_t x, uint8_t top, uint8_t height, int16_t width);
};

#endif
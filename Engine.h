#ifndef ENGINE_H
#define ENGINE_H

#include "Boot.h"
#include "Menu.h"
#include "Credits.h"
#include "Game.h"
#include "Legend.h"

// ========================================================
// Engine — despachador de ventanas
//
// POSEE las ventanas: Boot, Menu, Credits, Game y Legend son
// miembros propios (no globales, no anidadas entre sí). Cada
// una es una clase independiente con el patrón
// begin()/update()/print()/done() y usa los servicios globales
// (Display, Buttons, Sound — ver Globals.h) directamente.
//
// Solo Engine conoce el estado (State): decide qué ventana se
// ve (update()/print() despachan a la ventana activa) y, al
// cambiar de estado, llama al begin() de la ventana entrante.
// Como las ventanas son miembros y nadie más las referencia,
// la regla "una ventana nunca conoce a las demás" queda
// garantizada por el compilador.
//
// Las ventanas son instancias únicas que persisten entre
// transiciones: sus valores se conservan (a menos que su
// begin() los reinicie al entrar).
// ========================================================

class Engine {
public:

  // ========================================================
  // Constructor (no recibe nada: los servicios son globales)
  // ========================================================

  Engine();

  // ========================================================
  // Inicialización (estado inicial: Boot). Se llama desde setup().
  // ========================================================

  void begin();

  // ========================================================
  // Actualizar (solo la ventana activa + transiciones)
  // ========================================================

  void update();

  // ========================================================
  // Dibujar (dibuja solo la ventana activa)
  // ========================================================

  void print();

  // ========================================================
  // Puntaje máximo (lo conserva el menú entre sesiones)
  // ========================================================

  void setBestScore(uint16_t value);

private:

  // ========================================================
  // Estado interno: determina qué se ve y a qué ventana se despacha
  // ========================================================

  enum class State : uint8_t {
    BOOT = 0,
    MENU,
    NEW,
    CONTINUE,
    CREDITS,
    LEGEND
  };

  State _state;

  // ========================================================
  // Transición (fija el estado y llama al begin() de la ventana entrante)
  // ========================================================

  void changeState(State newState);

  // ========================================================
  // Ventanas (miembros propios: las posee Engine, ninguna es global)
  // ========================================================

  Boot _boot;
  Menu _menu;
  Credits _credits;
  Game _game;
  Legend _legend;
};

#endif

// ====================================================================================
// Fin
// ====================================================================================
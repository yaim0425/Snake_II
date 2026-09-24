#ifndef ENGINE_H
#define ENGINE_H

#include "Display.h"
#include "Buttons.h"
#include "Boot.h"
#include "Menu.h"
#include "Credits.h"
#include "Game.h"
#include "Legend.h"
#include "Sound.h"

// ========================================================
// Engine — despachador de ventanas
//
// NO anida las partes del juego: cada ventana (Menu, Credits,
// Game y Legend) es una clase independiente, construida en
// Snake_II.ino y recibida aquí por referencia, igual que
// Display y Buttons.
//
// Solo Engine conoce el estado (State): decide qué ventana se
// ve (update()/print() despachan a la ventana activa) y, al
// cambiar de estado, llama al begin() de la ventana entrante.
//
// Las ventanas son instancias únicas que persisten entre
// transiciones: sus valores se conservan (a menos que su
// begin() los reinicie al entrar).
// ========================================================

class Engine {
public:

  // ========================================================
  // Constructor (recibe las ventanas ya construidas y sin anidar)
  // ========================================================

  Engine(Display& display, Buttons& buttons, Boot& boot, Menu& menu,
         Credits& credits, Game& game, Legend& legend,
         Sound& sound);

  // ========================================================
  // Inicialización (estado inicial: menú). Se llama desde setup().
  // ========================================================

  void begin();

  // ========================================================
  // Actualizar (solo la ventana activa + transiciones)
  // ========================================================

  void update();

  // ========================================================
  // Dibujar (limpia y dibuja solo la ventana activa)
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
  // Dependencias (ventanas hermanas, comparten Display y Buttons)
  // ========================================================

  Display& _display;
  Buttons& _buttons;

  Boot& _boot;
  Menu& _menu;
  Credits& _credits;
  Game& _game;
  Legend& _legend;

  Sound& _sound;
};

#endif

// ====================================================================================
// Fin
// ====================================================================================
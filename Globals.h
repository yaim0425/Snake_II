#pragma once

#include "Display.h"
#include "Buttons.h"
#include "Sound.h"

// ========================================================
// Globals — servicios globales (hardware)
//
// Los ÚNICOS globales del proyecto: los servicios de hardware
// que usa todo el mundo (Display, Buttons, Sound).
// Cualquier clase los usa DIRECTAMENTE (display.drawText(...),
// buttons.pressed(...), sound.play(...)), sin inyectarlos por
// constructor.
//
// El Buzzer NO es un servicio global: es propiedad exclusiva
// de Sound, que lo contiene por valor y lo inicializa en su
// begin().
//
// Las ventanas (Boot, Menu, Credits, Game, Legend) NO son
// globales: viven dentro de Engine (miembros propios), para no
// romper la regla del despachador (una ventana nunca conoce a
// las demás; solo Engine las coordina).
//
// Las DEFINICIONES viven en Snake_II.ino, en el mismo orden
// en que se declaran aquí.
// ========================================================

extern Display display;
extern Buttons buttons;
extern Sound   sound;

// ====================================================================================
// Fin
// ====================================================================================
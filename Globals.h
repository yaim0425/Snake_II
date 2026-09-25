#pragma once

#include "Display.h"
#include "Buttons.h"
#include "Buzzer.h"
#include "Sound.h"

// ========================================================
// Globals — servicios globales (hardware)
//
// Los ÚNICOS globales del proyecto: los servicios de hardware
// que usa todo el mundo (Display, Buttons, Sound, Buzzer).
// Cualquier clase los usa DIRECTAMENTE (display.drawText(...),
// buttons.pressed(...), sound.play(...)), sin inyectarlos por
// constructor.
//
// Las ventanas (Boot, Menu, Credits, Game, Legend) NO son
// globales: viven dentro de Engine (miembros propios), para no
// romper la regla del despachador (una ventana nunca conoce a
// las demás; solo Engine las coordina).
//
// Las DEFINICIONES viven en Snake_II.ino, en orden de
// dependencia (buzzer antes que sound): al estar todas en la
// misma unidad de traducción se garantiza su orden de
// construcción y que nada se construya antes que su
// dependencia (p. ej. Sound(buzzer)).
// ========================================================

extern Display display;
extern Buttons buttons;
extern Buzzer  buzzer;
extern Sound   sound;

// ====================================================================================
// Fin
// ====================================================================================
#ifndef SOUND_H
#define SOUND_H

#include <Arduino.h>
#include "Buzzer.h"

// ========================================================
// Sound — sonidos del juego (contenido)
//
// Compone los efectos de sonido de Snake II como secuencias
// de tonos (Note) sobre la capa de hardware Buzzer. Todo es
// NO bloqueante: play() arranca el efecto y update()
// (llamado desde loop()) lo avanza paso a paso a medida
// que cada tono termina.
//
// Con el sonido desactivado (setEnabled(false)) play() no
// hace nada, pensado para la opción "Sound" del menú.
// ========================================================

class Sound {
public:

  // ========================================================
  // Efectos del juego
  // ========================================================

  enum Sfx : uint8_t {
    SFX_NONE = 0,
    SFX_CLICK,      // navegar por el menú (opción / rombo)
    SFX_CONFIRM,    // activar una opción del menú
    SFX_BACK,       // volver al menú desde una ventana
    SFX_EAT,        // comer el alimento (crece la serpiente)
    SFX_START,      // GO! al iniciar la partida
    SFX_LEVEL_UP,   // subir de nivel
    SFX_GAME_OVER   // muerte de la serpiente
  };

  // ========================================================
  // Constructor (recibe el Buzzer ya construido)
  // ========================================================

  Sound(Buzzer& buzzer);

  // ========================================================
  // Inicialización (silencia y reinicia la secuencia)
  // ========================================================

  void begin();

  // ========================================================
  // Sonido activado/desactivado (opción "Sound" del menú)
  // ========================================================

  void setEnabled(bool on);
  bool enabled() const;

  // ========================================================
  // Reproducir un efecto (arranca la secuencia, no bloquea)
  // ========================================================

  void play(Sfx effect);

  // ========================================================
  // Actualizar: avanza a la siguiente nota del efecto.
  // Llamar una vez por loop().
  // ========================================================

  void update();

  // ========================================================
  // Estado
  // ========================================================

  void stop();          // corta el efecto en curso
  bool playing() const; // true mientras suena un efecto

private:

  // ========================================================
  // Una nota de la secuencia (0 = silencio/luego)
  // ========================================================

  struct Note {
    uint16_t freq;
    uint16_t durMs;
  };

  // ========================================================
  // Secuencias (tablas estáticas en Sound.cpp)
  // ========================================================

  static const Note SEQ_CLICK[];
  static const Note SEQ_CONFIRM[];
  static const Note SEQ_BACK[];
  static const Note SEQ_EAT[];
  static const Note SEQ_START[];
  static const Note SEQ_LEVEL_UP[];
  static const Note SEQ_GAME_OVER[];
  static const uint8_t LEN_CLICK;
  static const uint8_t LEN_CONFIRM;
  static const uint8_t LEN_BACK;
  static const uint8_t LEN_EAT;
  static const uint8_t LEN_START;
  static const uint8_t LEN_LEVEL_UP;
  static const uint8_t LEN_GAME_OVER;

  // ========================================================
  // Estado
  // ========================================================

  Buzzer&  _buzzer;
  const Note* _seq;   // secuencia en curso (null = sin efecto)
  uint8_t  _len;      // cantidad de notas de la secuencia
  uint8_t  _step;     // nota actual
  bool     _enabled;  // sonido activado (menú "Sound")
};

#endif

// ====================================================================================
// Fin
// ====================================================================================
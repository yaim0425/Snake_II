#include "Sound.h"

// ========================================================
// Secuencias de notas de cada efecto (definición de los
// miembros estáticos SEQ_*: los tonos siguen la paleta del
// GameBuzzer original). Tablas privadas de la clase: solo las
// usa la tabla EFFECTS de más abajo.
// 0 en freq = silencio (espera de durMs).
// ========================================================

const Sound::Note Sound::SEQ_CLICK[] = {
  // Navegar por el menú (like Sound 01)
  { 1800, 35 }
};

const Sound::Note Sound::SEQ_CONFIRM[] = {
  // Activar opción (like Sound 05)
  { 700, 50 },
  { 1000, 80 }
};

const Sound::Note Sound::SEQ_BACK[] = {
  // Volver al menú (descendente)
  { 900, 50 },
  { 600, 90 }
};

const Sound::Note Sound::SEQ_EAT[] = {
  // Comer el alimento (like Sound 11 - moneda)
  { 988, 60 },
  { 1319, 100 }
};

const Sound::Note Sound::SEQ_START[] = {
  // GO! (like Sound 34)
  { 800, 60 },
  { 1100, 60 },
  { 1500, 150 }
};

const Sound::Note Sound::SEQ_LEVEL_UP[] = {
  // Subir de nivel (like Sound 43)
  { 523, 60 },
  { 659, 60 },
  { 784, 60 },
  { 1047, 120 },
  { 1319, 180 }
};

const Sound::Note Sound::SEQ_GAME_OVER[] = {
  // Muerte de la serpiente (like Sound 22)
  { 800, 100 },
  { 650, 100 },
  { 500, 150 },
  { 300, 300 }
};

const Sound::Note Sound::SEQ_TICK[] = {
  // Conteo regresivo: un pitido corto por dígito
  { 900, 40 }
};

const Sound::Note Sound::SEQ_TURN[] = {
  // Cambio de dirección: blip muy corto
  { 1319, 20 }
};

const Sound::Note Sound::SEQ_PAUSE[] = {
  // Pausar: descendente
  { 600, 50 },
  { 300, 60 }
};

const Sound::Note Sound::SEQ_RESUME[] = {
  // Reanudar: ascendente
  { 500, 50 },
  { 900, 60 }
};

const Sound::Note Sound::SEQ_NEW_BEST[] = {
  // Festejo de nuevo récord: fanfarria de victoria (marcha triunfal:
  // motivo ascendente C5-E5-G5-C6 y cierre largo en E6)
  { 523, 100 },   // C5  "ta"
  { 659, 100 },   // E5  "ra"
  { 784, 100 },   // G5  "ra"
  { 1047, 140 },  // C6  "pum"
  { 784, 100 },   // G5
  { 1047, 140 },  // C6
  { 1319, 420 }   // E6  cierre largo
};

// ========================================================
// Tabla de efectos: índice = Sfx (SFX_NONE..SFX_NEW_BEST,
// mismo orden que el enum). Cada entrada apunta a su secuencia
// y deriva el largo con sizeof en sitio (ya no hace falta
// LEN_*). SFX_NONE no tiene secuencia (no suena nada).
// ========================================================

const Sound::Seq Sound::EFFECTS[] = {
  { nullptr,             0 },                                       // SFX_NONE
  { SEQ_CLICK,     sizeof(SEQ_CLICK)       / sizeof(Sound::Note) }, // SFX_CLICK
  { SEQ_CONFIRM,   sizeof(SEQ_CONFIRM)     / sizeof(Sound::Note) }, // SFX_CONFIRM
  { SEQ_BACK,      sizeof(SEQ_BACK)        / sizeof(Sound::Note) }, // SFX_BACK
  { SEQ_EAT,       sizeof(SEQ_EAT)         / sizeof(Sound::Note) }, // SFX_EAT
  { SEQ_START,     sizeof(SEQ_START)       / sizeof(Sound::Note) }, // SFX_START
  { SEQ_LEVEL_UP,  sizeof(SEQ_LEVEL_UP)    / sizeof(Sound::Note) }, // SFX_LEVEL_UP
  { SEQ_GAME_OVER, sizeof(SEQ_GAME_OVER)   / sizeof(Sound::Note) }, // SFX_GAME_OVER
  { SEQ_TICK,      sizeof(SEQ_TICK)        / sizeof(Sound::Note) }, // SFX_TICK
  { SEQ_TURN,      sizeof(SEQ_TURN)        / sizeof(Sound::Note) }, // SFX_TURN
  { SEQ_PAUSE,     sizeof(SEQ_PAUSE)       / sizeof(Sound::Note) }, // SFX_PAUSE
  { SEQ_RESUME,    sizeof(SEQ_RESUME)      / sizeof(Sound::Note) }, // SFX_RESUME
  { SEQ_NEW_BEST,  sizeof(SEQ_NEW_BEST)    / sizeof(Sound::Note) }  // SFX_NEW_BEST
};

// ========================================================
// Constructor
// ========================================================

Sound::Sound(Buzzer& buzzer)
  : _buzzer(buzzer),
    _seq(nullptr),
    _len(0),
    _step(0),
    _enabled(true) {}

// ========================================================
// Inicialización
// ========================================================

void Sound::begin() {
  stop();
}

// ========================================================
// Sonido activado/desactivado
// ========================================================

void Sound::setEnabled(bool on) {
  _enabled = on;
  if (!on) stop();
}

bool Sound::enabled() const {
  return _enabled;
}

// ========================================================
// Reproducir un efecto (no bloqueante)
// ========================================================

void Sound::play(Sfx effect) {
  if (!_enabled) return;
  if (effect >= SFX_COUNT) effect = SFX_NONE;   // solo por defensa: los caller pasan valores del enum

  _seq = EFFECTS[effect].notes;
  _len = EFFECTS[effect].len;

  _step = 0;
  if (_len > 0) _buzzer.tone(_seq[0].freq, _seq[0].durMs);
  else          _seq = nullptr;
}

// ========================================================
// Actualizar: avanza a la siguiente nota del efecto
// ========================================================

void Sound::update() {
  // El Buzzer avanza su propia duración; cuando termina
  // la nota, busy() pasa a false y aquí se toca la siguiente.
  _buzzer.update();

  if (_seq == nullptr) return;
  if (_buzzer.busy()) return;

  _step++;

  if (_step < _len) {
    _buzzer.tone(_seq[_step].freq, _seq[_step].durMs);
  } else {
    _seq = nullptr;
  }
}

// ========================================================
// Estado
// ========================================================

void Sound::stop() {
  _seq = nullptr;
  _len = 0;
  _step = 0;
  _buzzer.stop();
}

bool Sound::playing() const {
  return _seq != nullptr;
}

// ====================================================================================
// Fin
// ====================================================================================
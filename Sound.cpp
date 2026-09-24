#include "Sound.h"

// ========================================================
// Secuencias de los efectos (tablas estáticas).
// Los tonos siguen la paleta del GameBuzzer original.
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

// ========================================================
// Largos de las secuencias (derivados de los arreglos)
// ========================================================

const uint8_t Sound::LEN_CLICK = sizeof(SEQ_CLICK) / sizeof(Note);
const uint8_t Sound::LEN_CONFIRM = sizeof(SEQ_CONFIRM) / sizeof(Note);
const uint8_t Sound::LEN_BACK = sizeof(SEQ_BACK) / sizeof(Note);
const uint8_t Sound::LEN_EAT = sizeof(SEQ_EAT) / sizeof(Note);
const uint8_t Sound::LEN_START = sizeof(SEQ_START) / sizeof(Note);
const uint8_t Sound::LEN_LEVEL_UP = sizeof(SEQ_LEVEL_UP) / sizeof(Note);
const uint8_t Sound::LEN_GAME_OVER = sizeof(SEQ_GAME_OVER) / sizeof(Note);
const uint8_t Sound::LEN_TICK = sizeof(SEQ_TICK) / sizeof(Note);
const uint8_t Sound::LEN_TURN = sizeof(SEQ_TURN) / sizeof(Note);
const uint8_t Sound::LEN_PAUSE = sizeof(SEQ_PAUSE) / sizeof(Note);
const uint8_t Sound::LEN_RESUME = sizeof(SEQ_RESUME) / sizeof(Note);

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

  switch (effect) {
    case SFX_CLICK:      _seq = SEQ_CLICK;      _len = LEN_CLICK;      break;
    case SFX_CONFIRM:    _seq = SEQ_CONFIRM;    _len = LEN_CONFIRM;    break;
    case SFX_BACK:       _seq = SEQ_BACK;       _len = LEN_BACK;       break;
    case SFX_EAT:        _seq = SEQ_EAT;        _len = LEN_EAT;        break;
    case SFX_START:      _seq = SEQ_START;      _len = LEN_START;      break;
    case SFX_LEVEL_UP:   _seq = SEQ_LEVEL_UP;   _len = LEN_LEVEL_UP;   break;
    case SFX_GAME_OVER:  _seq = SEQ_GAME_OVER;  _len = LEN_GAME_OVER;  break;
    case SFX_TICK:       _seq = SEQ_TICK;       _len = LEN_TICK;       break;
    case SFX_TURN:       _seq = SEQ_TURN;       _len = LEN_TURN;       break;
    case SFX_PAUSE:      _seq = SEQ_PAUSE;      _len = LEN_PAUSE;      break;
    case SFX_RESUME:     _seq = SEQ_RESUME;     _len = LEN_RESUME;     break;
    default:             _seq = nullptr;        _len = 0;              break;
  }

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
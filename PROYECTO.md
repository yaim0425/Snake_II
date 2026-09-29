# Snake II — ESP32-S3

Proyecto: Snake II (estilo Nokia) para placa ESP32-S3 con Arduino IDE.
Programación orientada a objetos: cada clase en su archivo `.h` y `.cpp`.

---

## 0. Reglas de trabajo con la IA

- **La IA no debe hacer nada sin una orden explícita del usuario.**
- No crear, modificar ni eliminar archivos por iniciativa propia.
- No proponer ni iniciar fases de desarrollo por su cuenta; solo actuar cuando
  el usuario lo indique.
- **La IA debe hacer un commit al modificar archivos** (cuando el usuario lo ordene
  o como parte del flujo de trabajo ya autorizado).
- **Agrupar los cambios en un solo commit por tarea**: los micro-commits por cambio
  puntual ya no se usan; todo lo relacionado con una misma tarea (código, este
  documento y, si corresponde, API) queda en un único commit.
- **La IA debe actualizar este documento (`PROYECTO.md`) antes de hacer el commit**:
  toda modificación de código debe quedar reflejada (secciones/API) y ese cambio a
  `PROYECTO.md` debe incluirse en el mismo commit.
- **La IA no compila sin permiso explícito del usuario.** El usuario compila por su
  cuenta (compilar la IA y después el usuario demora el flujo de trabajo).

### Frase para iniciar una nueva sesión

Para no empezar desde cero, **copiar y pegar tal cual** el siguiente bloque en la
nueva sesión (no hay que escribir nada más):

```
Trabaja en el proyecto Snake II (ESP32-S3, Arduino IDE) ubicado en
D:\Documents\ESP32S3\Snake_II.
```

---

## 1. Estado actual del proyecto

En desarrollo. `Snake_II.ino` es el **wiring**: define los servicios globales
(`Display`, `Buttons`, `Sound` — `Globals.h`) y crea el `Engine`, que **posee las
ventanas** (`Boot`, `Legend`, `Menu`, `MenuDifficulty`, `MenuSound`,
`MenuCredits`, `Game`) como miembros. Las
constantes compartidas viven en `Config.h` (pines, geometría, dificultad, versión).

**Arquitectura:**
- `Engine` es el despachador: posee las ventanas (no globales, no anidadas) y
  llama al `begin()` de la entrante al cambiar de estado. `loop()` hace la única
  lectura de botones del frame.
- Renderizado sin `clear()` global: cada ventana limpia solo en su primer frame y
  luego redibuja solo zonas dinámicas. `MenuDifficulty` y `MenuSound` son la
  excepción: no limpian nada, se dibujan **sobre el `Menu`** sustituyendo solo
  la banda de los rombos (45..53) y vuelven con
  `changeState(State::MENU, false)` + `Menu::restoreDiamondBand()`, así que el
  título, el cuadro de la opción y el pie nunca se repintan en el ida y vuelta.
- Flujo de arranque: `Boot` → `Legend` → `Menu`. La `Legend` solo aparece al
  arrancar; al volver al menú se pasa directo a `Menu`.

**Clases extraídas de `Game`:**
- `Food` — alimento (normal/especial): estado, spawn en celdas libres, dibujo,
  temporizador.
- `Snake` — lógica pura (sin `Display`/`Sound`/`Food`): buffer circular, giro
  pendiente, wrap, colisión, sprites. `Game` coordina ritmo, input y dibujo.
- `Sprite.h` — tabla de sprites (header-only).
- `Timer.h` / `Timer.cpp` — reloj de 64 bits (`esp_timer_get_time`), `Stopwatch` y `Ticker`.
  Migración completa desde `millis()`.

**Características del juego:**
- Dificultad en caliente: `setDifficulty` recalcula `_moveDelay` al instante.
- Puntaje = nivel de dificultad (`uint16_t` para evitar overflow).
- Festejo de nuevo récord: ciclo "GAME OVER" → "BUT" → "YOU ARE" → "THE BEST"
  con fanfarria solo en "THE BEST".

---

## 2. Hardware y pines

| Elemento  | Pin |
|-----------|-----|
| MOVE_UP   | 2   |
| MOVE_RIGHT | 1  |
| MOVE_DOWN | 42  |
| MOVE_LEFT | 41  |
| ACTION_UP | 38  |
| ACTION_RIGHT | 40 |
| ACTION_DOWN | 39  |
| ACTION_LEFT | 47  |
| Buzzer    | 14  |
| OLED SDA  | 8   |
| OLED SCL  | 9   |

- **Pantalla:** OLED SSD1306 monocromo (amarillo) 128x64, I2C, dirección `0x3C`.
- **Librerías:** Adafruit_SSD1306, Adafruit_GFX.
- **Botones:** configurados como `INPUT_PULLDOWN` en el juego original.

---

## 3. Estructura de archivos

Directorio: `D:\Documents\ESP32S3\Snake_II`

| Archivo | Contenido |
|---------|-----------|
| `Display.h` / `Display.cpp` | Clase `Display` (control del OLED). Completa. |
| `Config.h` | Constantes compartidas del proyecto (namespace `Config`, sección 19): pines (`Config::Pin`: botones, buzzer, SDA/SCL), geometría y regiones de la pantalla (`Config::Screen`: 128×64, celda 8, dirección I2C, Header/Body) y límites de la dificultad (`Config::Difficulty`: MIN_LEVEL/MAX_LEVEL/DEFAULT_LEVEL) y la versión del firmware (`Config::Version`: VERSION/RELEASE_DATE, la usa el pie del menú). Solo lo verdaderamente compartido; el resto es `static constexpr` en su clase. Sin `#define` para valores (constantes con tipo y ámbito). |
| `Globals.h` | Declara `extern` los **servicios globales**: `Display display;`, `Buttons buttons;` y `Sound sound;` (definidos en `Snake_II.ino`, sección 20). **No** declara el `Buzzer` (es interno de `Sound`). No define las ventanas: esas viven dentro de `Engine`. |
| `Buttons.h` / `Buttons.cpp` | Clase `Buttons` (lectura con debounce, `pressed`/`released`). Completa. |
| `Boot.h` / `Boot.cpp` | Clase `Boot` (animación de arranque: dos bandas completas —TITULO 0..15, CUERPO 16..63— de líneas verticales de 3 px que se desplazan en sentidos opuestos, con rebalse por el borde; dura `TOTAL_MS` y se termina con cualquier botón). Completa. |
| `Legend.h` / `Legend.cpp` | Clase `Legend` (panel de botones: pad MOVE a la izquierda con 4 flechas, 4 rombos completos de ACTION a la derecha en las posiciones de un pad que parpadean MUY rápido uno a la vez en ciclo lento —rombo fijo `HOLD_MS`, parpadeo `BLINK_PERIOD=100 ms`— y texto centrado en el pie con la función del rombo activo: Back, Select / Pause, None, None; cualquier botón la cierra con un sonido según el botón pulsado: MOVE = CLICK, ACTION_UP = BACK, ACTION_RIGHT = CONFIRM). Completa. |
| `Scroller.h` / `Scroller.cpp` | Clase `Scroller` (scroller de 1 bit: una sola banda, texto como array de `int8_t` donde cada byte = 1 columna de 8 px; fondo siempre blanco y texto negro; `setTexto()` calcula el límite de caracteres según el tamaño y trunca silenciosamente; usada por `Menu` con 1 instancia y por `MenuCredits` con 2 instancias sincronizadas). Completa. |
| `Menu.h` / `Menu.cpp` | Clase `Menu` (menú con scroller de 1 bit —1 banda del `Scroller` compartido— y rombos de posición). Completa. **Ya no edita nada**: "Difficulty" y "Sound" son ventanas propias (sección 10.3); el `Menu` solo navega y compone, y devuelve la opción confirmada para que el `Engine` abra la que toque. Añade `restoreDiamondBand()` (repinta solo la banda 45..53 y la línea del pie) para volver de esos selectores sin `clear()`. |
| `MenuDifficulty.h` / `MenuDifficulty.cpp` | Clase `MenuDifficulty` (selector de nivel 1..10, `< N >`, con repetición al mantener presionado; al mantener, solo queda fija la flecha del botón activo). Completa. Ventana hermana: vive sobre el `Menu` ya dibujado, sustituye solo la banda de rombos (45..53) y no hace `clear()`. Guarda el nivel confirmado (`difficulty()`), que el `Engine` pasa a `Game::setDifficulty`. |
| `MenuSound.h` / `MenuSound.cpp` | Clase `MenuSound` (selector On/Off con una flecha en el lado del destino, sobre la misma banda). Completa. Ventana hermana con la misma banda y el mismo regreso que `MenuDifficulty`; el valor real se aplica al global `Sound` con `setEnabled`. |
| `MenuCredits.h` / `MenuCredits.cpp` | Clase `MenuCredits` (ventana de créditos con 3 entradas navegables con transición lateral —2 bandas sincronizadas del `Scroller` compartido— y `SFX_CLICK` al navegar, vuelve al menú con `ACTION_UP`). Se llama así, y no `Credits`, para distinguirla de una hipotética ventana de créditos general: esta es la que se abre desde la opción "Credits" del `Menu`. Completa. |
| `Game.h` / `Game.cpp` | Clase `Game` (ventana del juego de la serpiente: estados NEW/CONTINUE del `Engine`). **Coordina**: dificultad/velocidad, lectura de botones (MOVE → `Snake::turn`), `snake.step()` con manejo del resultado, alimento (`Food`), puntaje, sonidos, overlays y volcado del tablero con los segmentos de `Snake` (celda de `Config::Screen::CELL`, sprites escalados con `SPRITE_SCALE`). Completa. |
| `Food.h` / `Food.cpp` | Clase `Food` (alimento del tablero, extraído de `Game`): estado (posición, presencia, tipo normal/especial), generación en celdas libres (`spawn`, que consulta la ocupación al tablero vía `Game::occupied`), dibujo del rombo (normal) o del sprite `SPECIAL_FOOD` (especial) —la celda la toma de `Config::Screen::CELL`, no declara una propia— y el temporizador de la comida especial. Completa. |
| `Snake.h` / `Snake.cpp` | Clase `Snake` (lógica pura de la serpiente, extraída de `Game`): buffer circular de segmentos, dirección commitida + giro pendiente (sin reversa directa), paso con wrap, colisión, comer/crecer y elección de sprites de las partes. **Sin `Display`/`Sound`/`Food`**: `Game` coordina el ritmo, el alimento, los sonidos y el dibujo. Completa. |
| `Engine.h` / `Engine.cpp` | Clase `Engine` (despachador de ventanas, antes `App`). **No anida las ventanas** pero las **posee como miembros** (`_boot`, `_menu`, `_menuDifficulty`, `_menuSound`, `_menuCredits`, `_game`, `_legend`): su estado interno decide qué ventana corre y cuándo cambiar (`changeState()`, que llama al `begin()` de la ventana entrante). Los `begin()` de las ventanas se lanzan desde `setup()` vía `engine.begin()`. `changeState()` admite un `beginWindow = false` para volver al menú desde `MenuDifficulty`/`MenuSound` sin repintar la página entera. Completa. |
| `Snake_II.ino` | Enlace de dependencias (wiring). Define los **servicios globales** (`display`, `buttons`, `sound`) y crea `Engine engine;` (que posee las ventanas). `Sound` se construye con el pin: `Sound sound(Config::Pin::BUZZER);` (el `Buzzer` es suyo). `setup()` llama `display.begin()`, `buttons.begin()`, `sound.begin()` (que inicializa su `Buzzer` interno) y `engine.begin()`; `loop()` hace la **única lectura de botones del frame** (`buttons.read()`) y llama `engine.update()`, `engine.print()`, `sound.update()` y `display.show()`. |
| `Buzzer.h` / `Buzzer.cpp` | Clase `Buzzer` (capa de hardware de sonido: un tono no bloqueante vía LEDC). **No es un servicio global**: la posee `Sound` por valor (sección 10.2). Completa. |
| `Sound.h` / `Sound.cpp` | Classe `Sound` (secuencias de los efectos del juego sobre su `Buzzer` interno —miembro por valor, inicializado en `begin()`—, con `setEnabled` para silenciar). Completa. |
| `Sprite.h` | Namespace `Sprite` (tabla de sprites de la serpiente, estilo Nokia: cola, cuerpo, curvas, cabeza cerrada/abierta y panza; sprites de 4×4 px + sprite de la comida especial de 8×4 px). Solo datos (header-only, sin `.cpp`). Adaptada al estilo del proyecto. |
| `Timer.h` / `Timer.cpp` | Reloj de 64 bits y cronómetros compartidos (`nowMs()`, `Stopwatch`, `Ticker`), basados en `esp_timer_get_time()` (sección 21). El header solo declara; las definiciones están en el `.cpp`, porque el `inline` en el header repetía el mismo código en cada `.cpp` que lo incluye. |
| `PROYECTO.md` | Este documento. |
| `LICENSE.md` | Licencia del proyecto: **MIT**, con el texto canónico en inglés (traducirlo haría que GitHub dejara de reconocerlo). Para cambiar el titular basta con editar la línea `Copyright (c) 2026 <nombre>` de ese archivo. |
| `THIRD_PARTY_NOTICES.md` | Aviso de atribución de las dependencias de terceros (Adafruit GFX y SSD1306, BSD-3; core Arduino-ESP32, LGPL-2.1; ESP-IDF, Apache-2.0) y constancia de que **el repositorio no distribuye código ni artwork de terceros** (los sprites de `Sprite.h` son tablas de bits propias). No es una obligación legal —las librerías se enlazan, no se distribuyen— pero se mantiene al día si cambian de versión las del gestor de Arduino. |

Nota: Arduino solo compila el `.ino` del sketch. El respaldo quedó como `.txt`
para que no interfiera en la compilación.

Nota (refactor de servicios globales): desde esta tarea las clases nuestras ya no
reciben `Display`/`Buttons`/`Sound`/`Buzzer` por constructor — las usan
directamente vía `Globals.h` (que se incluye solo en los `.cpp`, no en los
`.h`). `Sound` es la **única** clase que usa `Buzzer`, y lo **posee por valor**
(`Sound(uint8_t pin)` lo construye con el pin y `Sound::begin()` lo inicializa);
ya no quedan `Buzzer buzzer;` global ni el bind `Sound(Buzzer&)`. Se mantienen
constructor los
parámetros de configuración: `Menu(bestScore, version)`, `MenuCredits()`,
`Food(cols, rows, top)` y `Sound(pin)`
(y `Game()`/`Boot()`/
`Legend()` quedan sin parámetros). El constructor propio de cada ventana está
documentado en su sección.

---

## 4. Pantalla: geometría

- Tamaño: **128x64 píxeles**.
- Celda: **8x8 px** → rejilla de **16 columnas × 8 filas**.
- Los métodos usan coordenadas de celda o de píxel según el método.
- `cellSize` configurable por constructor (default 8).

### Fuente / tamaño de texto (Adafruit_GFX)

| Enumerador  | textSize | Dimensiones |
|-------------|----------|-------------|
| `TEXT_6x8`  | 1        | 6×8 px      |
| `TEXT_12x16` | 2       | 12×16 px    |
| `TEXT_18x24` | 3       | 18×24 px    |

---

## 5. Clase `Display` — API

Ubicación: `Display.h` / `Display.cpp`.

### 5.1 Constructor

```cpp
Display(uint8_t sda = Config::Pin::OLED_SDA, uint8_t scl = Config::Pin::OLED_SCL,
        uint8_t address = Config::Screen::ADDRESS, uint8_t width = Config::Screen::WIDTH,
        uint8_t height = Config::Screen::HEIGHT, uint8_t cellSize = Config::Screen::CELL);
```

Los valores por defecto (pines I2C, dirección y geometría) vienen de `Config`
(`Config::Pin` / `Config::Screen`); cualquier parámetro se puede sobreescribir.

La lista de inicialización del constructor sigue **el orden de declaración de los
miembros en `Display.h`** (en C++ los miembros se inicializan en orden de
declaración, no en el de la lista; un desajuste genera el warning `-Wreorder`).
Actualmente: `_screen`, `_sda`, `_scl`, `_address`, `_width`, `_height`,
`_cellSize`, `_columns`, `_rows`.

### 5.2 Enums y struct

```cpp
enum TextSize { TEXT_6x8 = 1, TEXT_12x16 = 2, TEXT_18x24 = 3 };

enum TextAlign {
  LEFT_UP, CENTER_UP, RIGHT_UP,
  CENTER_LEFT, CENTER, CENTER_RIGHT,
  LEFT_DOWN, CENTER_DOWN, RIGHT_DOWN
};

struct TextPos { int16_t x; int16_t y; };  // esquina sup-izquierda del texto
```

### 5.3 Regiones

La pantalla se divide en dos regiones para el texto. Las alineaciones (`TextAlign`)
se aplican dentro de la región elegida:

| Región | Zona |
|--------|------|
| `REGION_HEADER` | (0,0)  - (128,15)  |
| `REGION_BODY`   | (0,16) - (128,64)  |
| `REGION_FULL`   | (0,0)  - (128,64)  |

Cualquier texto puede alinearse en cualquiera de las 9 posiciones de cada región
y con cualquier tamaño (`TEXT_6x8`, `TEXT_12x16`, `TEXT_18x24`).

### 5.4 Métodos

| Método | Descripción |
|--------|-------------|
| `void begin()` | `Wire.begin(sda, scl)`, crea el OLED y lo limpia. **Idempotente:** si la pantalla ya quedó inicializada (`_screen != nullptr`) no hace nada, de modo que llamarla dos veces no reasigna el OLED ni filtra memoria (si el primer intento falló, `_screen` quedó en `nullptr` y un segundo llamado reintenta). |
| `void clear()` | Limpia el buffer de la pantalla. |
| `void show()` | Envía el buffer al OLED. |
| `void drawPixel(x, y, black=false)` | Dibuja 1 píxel (blanco o negro). |
| `void fillRect(x, y, w, h, black)` | Rectángulo relleno. Reenvío directo a `Adafruit_SSD1306::fillRect` (`int16_t, int16_t, uint16_t, uint16_t, uint16_t`), con el guard de `_screen == nullptr` del resto de métodos, pero el color ya no se propaga como `uint16_t` de la pantalla: la API de `Display` lo expresa como `bool black` (`true` = `SSD1306_BLACK`, `false` = `SSD1306_WHITE`), igual que `drawPixel`, para que las ventanas no manejen las constantes de color. |
| `void fillTriangle(x0, y0, x1, y1, x2, y2, black)` | Triángulo relleno por sus tres vértices (en cualquier orden). Reenvío directo a `Adafruit_GFX::fillTriangle` con el mismo `bool black` que `fillRect`. Quedan en `Display.cpp` los helpers estáticos `sortByY()`/`edgeAt()` de una versión anterior que rasterizaba el triángulo por filas (ordenar los vértices por `y` y cortar los dos lados activos en cada fila): ya no los usa nadie y están pendientes de borrar. |
| `TextPos getTextPos(texto, align, size=1, region=FULL)` | Devuelve x,y (esquina sup-izq) según alineación y región. |
| `void drawText(texto, x, y, size=1)` | Imprime texto en posición píxel exacta. |
| `void drawTextInverted(texto, x, y, size=1)` | Imprime texto en negro (sobre cualquier fondo) en posición exacta. |
| `void drawTextAligned(texto, align, size=1, region=FULL)` | Posición según alineación/región + imprime. |
| `void drawHighlight(texto, x, y, size=1)` | Texto resaltado (cuadro blanco + texto invertido) en posición exacta. |
| `void drawHighlightAligned(texto, align, size=1, region=FULL)` | Igual que `drawHighlight`, posicionado por alineación/región. |
| `uint8_t getTextWidth(texto, size=1)` | Ancho del texto = `strlen × 6 × size` (clamp 255). |
| `uint8_t getTextHeight(size=1)` | Alto del texto = `8 × size` (clamp 255). |
| `uint8_t getWidth()` / `getHeight()` | Ancho/alto en píxeles (128/64). |
| `uint8_t getCellSize()` | Píxeles por celda (8). |
| `uint8_t getColumns()` / `getRows()` | Columnas (16) / filas (8) de la rejilla. |
| `Adafruit_SSD1306& screen()` | Acceso directo al objeto OLED. **Referencia segura (no-opcional):** si `begin()` falló devuelve un OLED "mudo" en RAM (inicializado la primera vez) en lugar de desreferenciar `nullptr`; comprobar `isReady()` para saber si hay pantalla real. Precondición: `begin()` ya se llamó. |
| `bool isReady()` | `true` si la pantalla quedó operativa (el OLED respondió en `begin()`); `false` si la inicialización falló (pantalla ausente o sin respuesta por I2C). Tras un fallo, `screen()` sigue devolviendo una referencia válida (fallback en memoria) y el resto de métodos (`clear`, `show`, `drawText`, etc.) no hacen nada. |

### 5.5 Renderizado de texto y resaltado

- **Texto normal (`drawText`/`drawTextAligned`):** blanco sobre fondo negro, sin cuadro.
- **Resaltado (`drawHighlight`/`drawHighlightAligned`):** cuadro blanco (`fillRoundRect`,
  radio 0) con el texto invertido (negro) centrado dentro del cuadro.
- El cuadro **rebasa al texto**: `+2*size` px en X (1/2/3 px por lado) y `+1` px
  arriba y `+1` px abajo (total `+2` px en Y), centrando el texto en el cuadro.

```
size=1 -> texto 6x8      -> cuadro  (6+2)  x (8+2)  = 8x10
size=2 -> texto 12x16    -> cuadro  (12+4) x (16+2) = 16x18
size=3 -> texto 18x24    -> cuadro  (18+6) x (24+2) = 24x26
```

- No existen "botones no seleccionados": se usa texto normal o resaltado.

---

## 6. Decisiones de diseño

- **`char*` en lugar de `String`** en la clase `Display`: evita fragmentación del
  heap en bucles continuos. Las funciones de impresión aceptan `char*` (NULL-terminated).
- **Retornos `uint8_t`** para anchos/altos de texto: suficiente para la pantalla.
  Con clamp a 255 para evitar desbordes.
- **El ancho de texto usa `strlen` (bytes):** con caracteres UTF-8 (á, é, ñ) el ancho
  sería incorrecto. Actualmente se usan solo ASCII.
- **Posición de texto devuelta:** siempre la *esquina superior-izquierda* del área del texto.
- **Clase `Display` encapsula** el OLED; el juego que estaba en `Snake_II.ino` usaba
  `Adafruit_SSD1306` directamente (orden: `Screen`). Al integrar, migrar al objeto `Display`.
- **Pie de archivo "Fin":** todos los archivos fuente (`.h`/`.cpp`) terminan con el
  bloque de comentario `// ===…` + `// Fin` + `// ===…` (con una línea en blanco antes
  y sin salto de línea final), igual que `Snake_II.ino`.
- **Idioma del código: inglés.** Los identificadores son en inglés (clases, métodos,
  variables, constantes y enums; p. ej. `Menu::Option` = `OPT_NEW`/`OPT_CONTINUE`/
  `OPT_DIFFICULTY`/`OPT_SOUND`/`OPT_CREDITS`, `Config::Difficulty::MIN_LEVEL/MAX_LEVEL/DEFAULT_LEVEL` y
  los estados
  del `Engine` `NEW`/`CONTINUE`/`MENU_CREDITS`/`MENU_DIFFICULTY`/`MENU_SOUND`). Los comentarios y la documentación (`PROYECTO.md`)
  se mantienen en español (convención del proyecto).
- **Servicios globales, ventanas internas:** `Display`, `Buttons` y `Sound` son los
  únicos **globales** (`Globals.h`: `extern`, definidos en
  `Snake_II.ino`; el orden de construcción ya no importa porque cada servicio se
  inicializa en su `begin()` desde `setup()`, y así `Scroller`/`Food`
  pueden consultar `display.getWidth()` al construir `Engine`). El `Buzzer` **no**
  es global: lo contiene `Sound` por valor. Las **ventanas no son
  globales**: viven dentro de `Engine` (sección 11), así ninguna clase puede
  llamarlas por fuera del despachador.

---

## 7. Clase `Buttons` — API

Ubicación: `Buttons.h` / `Buttons.cpp`. Basada en el diseño de `GameInput` (referencia
`D:\Documents\ESP32S3\Snake_2\GameInput.{h,cpp}`).

### Pines (orden del enum)

Los pines de los botones viven en `Config::Pin::BUTTONS` (sección 19),
`Snake_II.ino` los pasa al constructor:

```cpp
Buttons buttons(Config::Pin::BUTTONS);   // en Snake_II.ino
```

### Enum

```cpp
enum Button : uint8_t {
  MOVE_UP = 0, MOVE_RIGHT, MOVE_DOWN, MOVE_LEFT,
  ACTION_UP, ACTION_RIGHT, ACTION_DOWN, ACTION_LEFT
};
```

### Métodos

| Método | Descripción |
|--------|-------------|
| `Buttons(const int8_t* pins, uint32_t buttonDelay = 30)` | Constructor, recibe los pines y el tiempo de debounce en ms. |
| `void begin()` | Configura `INPUT_PULLDOWN` y lee el estado inicial. |
| `void read()` | Leer físicamente, aplicar debounce y generar eventos. **Se llama una sola vez por `loop()`** (en `loop()`, antes de `engine.update()`); las ventanas solo consultan `state`/`pressed`/`released` sin volver a leer. |
| `bool state(index)` / `pressed(index)` / `released(index)` | **Acceso único** por botón, con los valores del enum `Button`: `state(Buttons::MOVE_LEFT)` = estado actual (mantenido), `pressed(Buttons::ACTION_RIGHT)` = evento de pulso (true solo en el ciclo en que se presiona), `released(...)` = evento de liberación. Los 24 getters con nombre (`moveUp()`/`actionRightPressed()`/`...,Released()`, etc.) se eliminaron: API única sin boilerplate. |

### Diseño del debounce (estados agrupados en bytes)

- Lee los 8 botones agrupados en bytes (`uint8_t`, 1 bit por botón,
  HIGH = presionado con `INPUT_PULLDOWN`), igual que el `estados` del
  ejemplo de referencia: `_rawButtons` (físico sin filtrar), `_buttons`
  (confirmado), `_pressed`/`_released` (eventos de 1 ciclo).
- `_lastButtons` ya no existe: los eventos Pressed/Released se detectan
  comparando el bit del botón con su estado confirmado anterior antes de
  actualizarlo (no hace falta guardar todo el estado previo).
- Verificación de bits con el helper privado estático
  `bool isSet(uint8_t estados, uint8_t boton)` → `estados & (1 << boton)`.
- Detecta el cambio físico por bit (`raw ^ _rawButtons`) y toma nota del
  instante por botón.
- Solo acepta el nuevo estado tras `_buttonDelay` ms de estabilidad.
- Almacenamiento reducido: de 4 arrays `bool[8]` (32 B) + 1 byte a 4 bytes.

### Anticonflicto MOVE

Si **2 o más botones MOVE se confirman a la vez** (2+ bits de la máscara
`MOVE_MASK` = bots 0..3), se **anula la activación de todos**: esa limpieza se
aplica al final de `read()` sobre el estado confirmado y los eventos del frame
(`_buttons`, `_pressed` y `_released` se quedan sin esos bits). Así ningún botón
MOVE queda activo mientras hay simultaneidad (el pad direccional es excluyente);
los botones `ACTION` no participan del anticonflicto (se pueden pulsar a la vez
que un MOVE). Al liberar uno, si queda un único MOVE presionado, ese vuelve a
activarse solo (su `state()/pressed()` reflejan de nuevo al botón que queda).

---

## 7. Clase `Menu` — API

Ubicación: `Menu.h` / `Menu.cpp`.

### Constructor

```cpp
Menu(uint16_t bestScore = 0, const char* version = Config::Version::VERSION);
```

Los servicios `Display`, `Buttons` y `Sound` son globales (sección 20); quedan
como parámetros solo los datos de configuración. El `Scroller` interno se
construye con `_scroller(1, nullptr)` en la lista de inicialización. El texto del
pie sale de `Config::Version::VERSION` (sección 19), así que la versión ya no es
un literal en la firma.

### Métodos

| Método | Descripción |
|--------|-------------|
| `void begin()` | Restablece el estado de la animación y del parpadeo. **No resetea la selección**: conserva la opción elegida antes de salir del menú (las ventanas son hermanas persistentes; al volver al menú se muestra la misma opción que se tenía, no siempre "New"). Pone `_redraw`, que hace el `clear()` completo + `firstPrint()`. |
| `void setOptions(textos, conteo)` | Fija la lista y la cantidad de opciones (1..`MAX_OPTIONS`=8). El menú (textos y rombos) se adapta al conteo. |
| `void setContinueAvailable(bool)` | **Muestra/oculta la opción "Continue"** según haya partida en curso que reanudar. `true` = lista de 5 opciones (New, Continue, Dificultad, Sound, Créditos); `false` = lista de 4 (New, Dificultad, Sound, Créditos). Estado inicial: `false` (al arrancar no hay partida). Quién lo decide: el `Engine` al volver del juego solo pasa `true` si la partida sigue en curso **y** tiene puntos (`!gameOver && score() > 0`). Cambia la lista interna (como `setOptions`) conservando la selección y adaptándola a la nueva cantidad. |
| `void update()` | Lee botones, navega con `MOVE_RIGHT`/`MOVE_LEFT` y anima el deslizamiento lateral; log en Serial al cambiar de opción; toca `SFX_CLICK` al navegar. Nada más: "Difficulty" y "Sound" ya no se editan aquí, el `Engine` abre sus ventanas. |
| `void print()` | Dibuja título, cuadro fijo con la opción deslizante, rombos de posición y pie (Best + versión). La banda de la opción se vuelca con `Scroller::blit`, que **en reposo es un no-op** (dirty flag del `Scroller`: solo pinta si hay algo nuevo, ver sección 14). Si vuelve de `MenuDifficulty`/`MenuSound` repinta solo la banda de rombos (ver `restoreDiamondBand`). |
| `int8_t selected()` | Índice de la opción seleccionada. |
| `int8_t confirm()` | Devuelve la opción seleccionada si se confirma con `ACTION_RIGHT` (pulse recién presionado), o `-1`. **Devuelve las cinco opciones lógicas** (incluidas `OPT_DIFFICULTY` y `OPT_SOUND`): es el `Engine` quien abre la ventana que corresponda. Es el "activar opción" del menú. |
| `void setBestScore(uint16_t)` | Actualiza el puntaje máximo mostrado. |
| `void setTitle(const char*)` | Cambia el título del Header. |
| `void setShowFooter(bool)` | Ocultar/mostrar el texto del pie ("Best"/versión); la línea de la `54` se dibuja siempre. |
| `void setSelected(Menu::Option)` | Fija la selección **por opción lógica** (enum `Option`, p. ej. `OPT_NEW` u `OPT_CONTINUE`), se mapea al índice de la lista visible y reinicia la animación (al entrar en la ventana). Si la opción no está visible ("Continue" oculto) la selección cae a `New`. |
| `void restoreDiamondBand()` | Deja marcado (`_diamondsDirty`) que hay que repintar **solo la banda de rombos** (45..53) y la línea separadora del pie. Lo llama el `Engine` al volver de `MenuDifficulty`/`MenuSound`, que dejaron esa franja ocupada con su selector. A diferencia de `begin()`, **no** provoca `clear()`: el título, el cuadro de la opción y el texto del pie se conservan tal cual. |

### Las opciones "Difficulty" y "Sound" son ventanas

Al confirmar cualquiera de las dos, el `Engine` abre `MenuDifficulty` o `MenuSound`
(ventanas hermanas, sección 11) en vez de editar el valor dentro del `Menu`. El
`Menu` solo navega y compone; el estado editable y su dibujo viven ya en sus
propias ventanas. `Menu::confirm()` por tanto devuelve también esas dos opciones.

Ambas ventanas **no borran la pantalla**: entran sobre el `Menu` ya dibujado,
sustituyen únicamente la banda de rombos (45..53) por su selector (`< N >` con
sus dos flechas, o `ON`/`OFF` con su flecha única) y, al salir, el `Engine`
devuelve el control al `Menu` **sin llamar a su `begin()`**
(`changeState(State::MENU, false)`), que solo repinta esa misma banda. El resto
de la ventana —título, cuadro de la opción y pie con "Best"— no se vuelve a
tocar en todo el ida y vuelta.

Anteriormente esta edición era *inline* dentro del propio `Menu` (con
`beginSoundEdit`/`beginDifficultyEdit`, `drawSoundSelector`/`drawDifficultySelector`
y `holdRepeat`): todo eso se eliminó al extraerlo a las ventanas.

### Opciones y enum

```cpp
enum Option : uint8_t {
  OPT_NEW = 0, OPT_CONTINUE, OPT_DIFFICULTY, OPT_SOUND, OPT_CREDITS
};
```

El enum documenta las **5 opciones lógicas**. La lista visible varía: con
`Continue` disponible (`setContinueAvailable(true)`) los índices de la lista
coinciden con el enum; sin `Continue` la lista se compacta a 4 opciones y el
índice 1 pasa a Dificultad, el 2 a Sonido y el 3 a Créditos (mapeo interno
`optionAt`/`indexOfOption`, privados). `confirm()` devuelve siempre la **opción
lógica** (enum `Option`), así el `Engine` compara con los mismos valores con o
sin "Continue". Cantidad real máxima `MAX_OPTIONS = 8`.

### Diseño del menú (scroller de 1 bit)

- **Título:** "Snake II", `TEXT_12x16`, centrado en `REGION_HEADER`. **Estático:**
  se dibuja una sola vez al entrar (tras el `clear()` completo) y ya no se limpia
  ni se redibuja por frame.
- **Pie:** "Best: X" (`LEFT_DOWN`) y versión (`RIGHT_DOWN`) en `TEXT_6x8`,
  **bajado 1 px** (fila 57, `PIE_TOP = 57`). **Estático:** se dibuja una sola vez
  al entrar (tras el `clear()` completo; se actualiza solo si `setBestScore`
  cambia). Una **línea horizontal de 1 px** de grosor,
  `drawFastHLine`, en la fila `54` (`PIE_LINE_ROW`), a **2 px sobre el pie**
  (filas libres `55..56` entre la línea y el texto). Las 2 filas sobre el pie
  quedan siempre limpias.
- **Cuadro de selección:** **fijo** y de **ancho completo** (128 px),
  `BOX_TOP = 25`, `BOX_HEIGHT = 18` (banda 25..42). **Estático:** el cuadro blanco
  se dibuja una sola vez al entrar; por frame solo se vuelca la banda de la
  opción (`26..41`) con el scroller. Con el rombo activo de punta
  en la `45`: quedan **2 filas libres** (`44..43`) sobre el rombo y el cuadro
  arranca en la **fila 3** desde la punta (`42`) hacia arriba. **No se mueve**;
  el tamaño del texto (tamaño 2) tampoco cambia.
- **Animación (scroller de 1 bit):** en la clase **`Scroller`** (ver
  sección 14, "Clase `Scroller`"), que `Menu` instancia con **1 instancia**
  (texto 12x16) y `MenuCredits` con **2 instancias sincronizadas** (rol 12x16 +
  nombre 6x8). Cada opción se compone **antes** de mostrarse en un **array de
  `int8_t`** (cada byte = 1 columna de 8 px, `1` = glifo, `0` = fondo)
  **centrada**, mediante `setTexto()` que dibuja el texto en un canvas
  auxiliar y extrae las columnas. La franja es siempre **fondo blanco y texto
  negro**. Al navegar `MOVE_RIGHT` la tira entra por la **derecha** (se mueve
  hacia la izquierda); con `MOVE_LEFT` por la **izquierda**. Arranca **fuera
  de pantalla** y avanza **una columna por cada `ANIM_TICK` ms** (`animate`,
  acumulado por tiempo; vuelo total ≈ `128 × 4 ms ≈ 0,5 s`). Si se navega a
  mitad de la animación, la banda conserva lo que había y la nueva tira se
  superpone (pueden verse varias opciones a la vez). Todas las opciones quedan
  con el **mismo ancho de 128 px** (espacios vacíos incluidos). Con la tira ya
  centrada y sin navegar, `blit()` **no repinta la banda** (dirty flag del
  `Scroller`, sección 14): queda tal cual en la pantalla.
- **Rombos de posición:** banda `45..53`, pegada a la línea separadora en la `54`
  (antes de la fila del pie, `DIA_TOP = 45`). Solo el **seleccionado** es un **rombo simétrico
  completo** de 9 filas (dibujado con dos `fillTriangle`, como el alimento del
  juego): punta superior en la `45`, hombros en la `49` y **punta inferior en la
  `53`** (visible, justo sobre la línea separadora `54`). Los **no seleccionados** son
  solo la **punta** (triángulo superior), **bajada hasta la línea separadora**:
  base en la `53` y vértice en la `50`. Reparto uniforme en el ancho
  (`cx = (i+1)·128/(n+1)`). Si se **mantiene** seleccionado sin navegar
`BLINK_HOLD = 500 ms`, el rombo **parpadea** mostrándose el **75%** de cada
  período y oculto el **25%** (período `BLINK_PERIOD = 500 ms`, fase oculta = primer
  25%). Es la **única zona dinámica** del menú: por frame se limpia solo la banda
  `45..53` (`fillRect(0, DIA_TOP, ancho, DIA_SIZE+1)`) y se redibujan los rombos.
- Primera y última opción no conectadas (navegación con límites).

---

## 8. Clase `Boot` — API

Ubicación: `Boot.h` / `Boot.cpp`.

### Constructor

```cpp
Boot();
```

Sin parámetros: usa los servicios globales `Display` y `Buttons` (sección 20).

### Métodos

| Método | Descripción |
|--------|-------------|
| `void begin()` | Reinicia la animación: desplazamiento 0, temporizadores a cero y flag de salida apagado. |
| `void update()` | Lee botones; cualquier botón la termina. Autoavance de 1 px cada `ANIM_TICK = 30 ms` (acumulador por tiempo) y duración total `TOTAL_MS = 4000 ms`. Al terminar pone `done() = true`. |
| `void print()` | Dibuja las dos bandas de líneas verticales. |
| `bool done()` | `true` cuando la animación terminó (Engine pasa al menú). |

### Animación (franjas verticales)

- **Bandas completas:** `TITULO` = filas **0..15** y `CUERPO` = filas **16..63**
  (sin márgenes, como las regiones de la pantalla).
- **Líneas:** verticales de `BAR_W = 3 px` de grosor separadas `BAR_SPACING = 8 px`,
  dibujadas con `Display::fillRect` (sección 7); ya no se accede a la pantalla cruda.
- **Reparto de las 3 columnas de cada línea** (cola = extremo por el que la línea
  entra, cabeza = extremo por el que sale):
  - `firstPrint()` pinta la línea **completa** de cada franja (un solo `fillRect`
    de `BAR_W` de ancho) con el desplazamiento 0 del primer frame.
  - `drawBars()` pinta la columna de **cabeza** (la primera de la línea según su
    dirección: la derecha en `TITULO`, la izquierda en `CUERPO`, que es la única
    columna nueva en cada avance) y borra en negro la columna que la línea deja
    libre por la cola. El resto de la línea no cambia y no se toca.
- **Rebalse:** al llegar al borde la línea se parte en dos tramos (derecho +
  izquierdo) para que no se corte en seco; como `fillRect` recorta en vez de
  envolver, `drawBar()` normaliza el módulo a `0..ancho-1` y pinta **una sola
  columna**, de modo que la columna que rebalsa va a su tramo sin duplicar el
  pintado de la columna 0.
- **Movimiento:** `_step` avanza 1 px cada `ANIM_TICK = 30 ms` (ciclo completo de
  8 px). En `TITULO` las líneas se mueven **de izquierda a derecha**; en `CUERPO`
  **de derecha a izquierda**.
- **Limpieza incremental (sin `clear()` por frame):** el primer frame hace un `clear()`
  completo y dibuja el patrón inicial (`firstPrint()`, la línea completa de cada
  franja); en cada avance de 1 px solo se pinta la columna de cabeza y se
  borra la liberada por la cola. El resto de la pantalla no se toca. `print()`
  encadena las etapas con `if` sueltos y `return` temprano: el `clear()` del
  primer frame, el caso "no cambió el desplazamiento" (`_step == _prevStep`, no se
  dibuja nada) y el avance (`drawBars()` + `_prevStep = _step`). `eraseOldBars()`/
  `eraseBarDiff()` ya no se usan (el borrado quedó dentro de `drawBars()`).
- **Duración:** `TOTAL_MS = 4000 ms` o cualquier botón, lo que ocurra primero; luego
  `Engine` entra al menú.

---

## 9. Clase `Legend` — API

Ubicación: `Legend.h` / `Legend.cpp`.

### Constructor

```cpp
Legend();
```

Sin parámetros: usa los servicios globales `Display`, `Buttons` y `Sound`
(sección 20).

### Métodos

| Método | Descripción |
|--------|-------------|
| `void begin()` | Reinicia la ventana: apaga el flag de salida, rombo activo = `Btn1`. |
| `void update()` | Lee botones y avanza el ciclo. **El sonido depende del botón presionado** (`done() = true`): `MOVE_*` (navegación) = `SFX_CLICK`, `ACTION_UP` (Back) = `SFX_BACK`, `ACTION_RIGHT` (Select / Pause) = `SFX_CONFIRM`, `ACTION_DOWN`/`ACTION_LEFT` (None) = `SFX_CLICK`. El rombo activo cambia cada `DWELL_MS`. |
| `void print()` | Dibuja el pad MOVE, los 4 rombos de ACTION y el texto centrado del pie. |
| `bool done()` | `true` cuando se pidió ir al menú. `Engine` solo cambia de estado; **la Legend ya reprodujo su sonido** (Engine no toca `SFX_BACK` en esta transición). |

### Dibujo (leyenda de botones)

- **Pad MOVE (izquierda):** rombo de 4 flechas sólidas (`drawArrow`) centrado en
  `(PAD_MOVE_X=30, CY=32)`, radio `R=12`, con el rótulo "Move" arriba.
- **Pad ACTION (derecha):** **4 rombos completos** (`drawDiamond`, SIEMPRE rombo
  simétrico de `DIA_SIZE=8`, nunca triángulos) colocados en las **posiciones de un
  pad direccional** alrededor de `(DIA_PAD_X=96, CY)` con radio `DIA_R=12`: ↑
  (Btn1), → (Btn2), ↓ (Btn3), ← (Btn4); rótulo "Action" arriba centrado sobre el
  grupo.
- **Parpadeo (uno a la vez, MUY rápido):** el rombo activo recorre `Btn1 → Btn4`
  en **ciclo lento** (avanza cada `DWELL_MS=2200` ms); queda fijo `HOLD_MS=900` ms
  y luego parpadea a alta frecuencia (`BLINK_PERIOD=100` ms, oculto el
  `BLINK_OFF_PCT=50%` inicial de cada período: ~10 Hz). Los rombos inactivos se
  quedan fijos y completos.
- **Texto del pie (centrado, mismo diseño que el menú):** línea en
  `PIE_LINE_ROW=54` y texto en `PIE_TOP=57` con la función del rombo activo:
  `Btn1` (↑ = `ACTION_UP`): **"Back"**, `Btn2` (→ = `ACTION_RIGHT`):
  **"Select / Pause"**, `Btn3` (↓ = `ACTION_DOWN`): **"None"**, `Btn4` (← =
  `ACTION_LEFT`): **"None"**.
- **Salida:** cualquier botón cierra la leyenda (`done()`) con su sonido según el
  botón. `Engine` solo la muestra al arranque (después del `Boot`); ya no se repite
  al volver al menú.
- **Renderizado (sin `clear()` por frame):** rótulos, pad MOVE y los 4 rombos de
  ACTION son **estáticos** (se dibujan una sola vez al entrar, tras el `clear()`
  completo). Por frame solo se borra/redibuja la **zona del rombo activo**
  (cuadro 9x9 alrededor de su centro, para el parpadeo) y el **texto del pie**
  (banda `54..63`) únicamente cuando cambia el rombo activo. Al cambiar el rombo
  activo, el que **deja de serlo** se restaura completo (`drawDiamond(..., true)`):
  si el cambio lo pilla en su **fase oculta de parpadeo**, su zona quedó borrada y
  nadie la volvería a dibujar (evita rombos desaparecidos).

---

## 10. Clase `Buzzer` — API (capa de hardware)

Ubicación: `Buzzer.h` / `Buzzer.cpp`. Reproduce **un solo tono a la vez**, de forma
**no bloqueante**: `tone()` enciende el tono y marca su duración; `update()` (vía
`Sound::update()`) lo apaga al agotarse el tiempo. Sin `delay()`, el juego nunca se
congela. Usa LEDC del núcleo ESP32 (Core 3.x), como el `GameBuzzer` original.

**No es un servicio global:** es propiedad exclusiva de `Sound`, que lo contiene
como **miembro por valor** (`Buzzer _buzzer;`) y lo construye con el pin recibido.
Solo `Sound` lo usa: no hay `extern Buzzer` en `Globals.h` ni instancia global en
`Snake_II.ino`, y su `begin()` lo llama `Sound::begin()`. La API es pública dentro
de `Buzzer`, pero está encapsulada por la estructura de `Sound`.

### Constructor

```cpp
Buzzer(uint8_t pin = Config::Pin::BUZZER);   // pin 14 (Config)
```

### Métodos

| Método | Descripción |
|--------|-------------|
| `bool begin()` | `ledcAttach(pin, 2000, 10)` y silencia. Devuelve si se pudo adjuntar el canal. |
| `void update()` | Apaga el tono cuando termina su duración. Llamar una vez por `loop()` (ya lo hace `Sound::update()`). |
| `void tone(freq, durMs = 0)` | Emite un tono (no bloqueante). `durMs > 0` lo apaga solo; `0` = suena hasta `stop()`. `freq = 0` = silencio (espera activa). |
| `void stop()` | Silencia el buzzer y cancela la duración pendiente. |
| `bool busy()` | `true` mientras hay un tono/duración en curso. |
| `bool attached()` | `true` si `begin()` adjuntó el canal LEDC. |

---

## 10.2 Clase `Sound` — API (sonidos del juego)

Ubicación: `Sound.h` / `Sound.cpp`. Compone los efectos de Snake II como **secuencias
de tonos** (`Note` = `{freq, durMs}`, `0` = silencio) sobre su **`Buzzer` interno**
(miembro por valor, inicializado en `Sound::begin()`). Todo es no
bloqueante: `play()` arranca el efecto y `update()` lo avanza paso a paso cuando
cada nota termina. Con el sonido desactivado `play()` no hace nada (pensado para la
opción "Sound" del menú, `setEnabled(false)`).

Las 12 secuencias `SEQ_*` son **miembros estáticos privados** declarados en `Sound.h`
y definidos en `Sound.cpp` (los tonos), y se indexan con una **única tabla
`EFFECTS[]` de `{const Note* notes, uint8_t len}`** (orden igual al enum `Sfx`,
`SFX_NONE` incluido, con secuencia vacía): reemplaza a las 12 constantes `LEN_*` (el
largo de cada secuencia se deriva con `sizeof` dentro de la tabla) y al `switch` de
`play()`, que ahora solo lee `EFFECTS[effect]`. Son detalle de la implementación: la
API pública de `Sound` no expone `Note`, `Seq` ni las tablas.

### Constructor

```cpp
explicit Sound(uint8_t pin = Config::Pin::BUZZER);
```

El `Buzzer` se **construye por valor** dentro de `Sound` (miembro `_buzzer`, primer
miembro y primero en la lista de inicialización): ya no hay `Buzzer&` ni instancia
global.

### Enum y efectos

```cpp
enum Sfx : uint8_t {
  SFX_NONE = 0, SFX_CLICK, SFX_CONFIRM, SFX_BACK,
  SFX_EAT, SFX_START, SFX_LEVEL_UP, SFX_GAME_OVER,
  SFX_TICK, SFX_TURN, SFX_PAUSE, SFX_RESUME,
  SFX_NEW_BEST, SFX_COUNT   // SFX_COUNT = tamaño de EFFECTS (tabla indexada por Sfx)
};
```

| Efecto | Uso | Secuencia (frecuencias Hz / ms) |
|--------|-----|---------------------------------|
| `SFX_CLICK` | Navegar por el menú | 1800/35 |
| `SFX_CONFIRM` | Activar una opción | 700/50, 1000/80 |
| `SFX_BACK` | Volver al menú | 900/50, 600/90 |
| `SFX_EAT` | Comer el alimento | 988/60, 1319/100 |
| `SFX_START` | GO! al iniciar | 800/60, 1100/60, 1500/150 |
| `SFX_LEVEL_UP` | Subir de nivel | 523/60, 659/60, 784/60, 1047/120, 1319/180 |
| `SFX_GAME_OVER` | Muerte de la serpiente | 800/100, 650/100, 500/150, 300/300 |
| `SFX_TICK` | Conteo regresivo 3-2-1 (un pitido por dígito) | 900/40 |
| `SFX_TURN` | Cambio de dirección de la serpiente | 1319/20 |
| `SFX_PAUSE` | Pausar la partida | 600/50, 300/60 |
| `SFX_RESUME` | Reanudar la partida | 500/50, 900/60 |
| `SFX_NEW_BEST` | Festejo de nuevo récord (suena en el letrero "THE BEST") | 523/100, 659/100, 784/100, 1047/140, 784/100, 1047/140, 1319/420 |

Los tonos siguen la paleta del `GameBuzzer` original.

### Métodos

| Método | Descripción |
|--------|-------------|
| `void begin()` | Inicializa la capa de hardware (`_buzzer.begin()`, que adjunta el canal LEDC) y después `stop()` (silencia y reinicia la secuencia). Es el único lugar que inicializa el `Buzzer`. |
| `void setEnabled(bool)` / `enabled()` | Activa/desactiva el sonido (opción "Sound"); al desactivar corta el efecto en curso. |
| `void play(Sfx)` | Arranca la secuencia del efecto (no bloqueante). |
| `void update()` | Avanza a la siguiente nota (llama `_buzzer.update()` antes). Llamar una vez por `loop()`. |
| `void stop()` | Corta el efecto en curso y silencia. |
| `bool playing()` | `true` mientras suena un efecto. |

En `Snake_II.ino`: `Sound sound(Config::Pin::BUZZER);` (instancia única del
servicio global; el `Buzzer` que contiene se inicializa con `sound.begin()` en
`setup()` y `sound.update()` se llama en `loop()`).

---

## 10.3 Ventanas del menú: `MenuDifficulty` y `MenuSound`

Las opciones **"Difficulty"** y **"Sound"** del menú son ventanas del `Engine`,
igual que `MenuCredits` (históricamente fueron una clase `SoundWindow` y luego
edición *inline* dentro del propio `Menu`; ambas formas se eliminaron). El `Menu`
las devuelve como cualquier otra opción en `confirm()` y el `Engine` abre la
ventana que corresponda (`OPT_DIFFICULTY` → `MENU_DIFFICULTY`, `OPT_SOUND` →
`MENU_SOUND`).

Ambas comparten el mismo esqueleto de ventana (`begin`/`update`/`print`/`done`),
la misma banda de dibujo (**45..53**, la de los rombos del `Menu`) y la misma
regla de renderizado: **no borran la pantalla**, solo sustituyen esa franja. Al
salir, el `Engine` llama a `Menu::restoreDiamondBand()` y vuelve al menú con
`changeState(State::MENU, false)`, que evita el `clear()` completo de
`Menu::begin()`; el `Menu` repinta la banda y la línea del pie.

### `MenuDifficulty` — nivel 1..10

| Miembro | Descripción |
|---------|-------------|
| `MenuDifficulty()` | `_difficulty = Config::Difficulty::DEFAULT_LEVEL` (5) y el valor en edición igual. |
| `void begin()` | Copia `_difficulty` a `_edit` y ancla el parpadeo. **No dibuja ni borra nada** (entra sobre el `Menu`). |
| `void update()` | `MOVE_RIGHT` +1 / `MOVE_LEFT` −1 con repetición al mantener (helper `holdRepeat`), tope en `Config::Difficulty::MIN_LEVEL`/`MAX_LEVEL`, cada paso con `SFX_CLICK`. `ACTION_RIGHT` **aplica** (`_difficulty = _edit`) y marca `done()`; `ACTION_UP` cancela. |
| `void print()` | Borra la banda 45..53 y llama a `drawSelector()` (cada frame: las flechas parpadean, el número no). |
| `bool done()` | `true` tras confirmar o cancelar. |
| `uint8_t difficulty()` | Nivel **confirmado**: lo lee el `Engine` en `changeState(NEW/CONTINUE)` para pasárselo a `Game::setDifficulty` (antes vivía en `Menu::difficulty()`). |
| `void drawSelector()` | Número 1..10 en `TEXT_6x8` centrado con **ancho constante** (`" 5"` mide lo mismo que `"10"`, 12 px) y **dos flechas** a los lados (`"< 5 >"`), **pegadas** al número (hueco `ARROW_GAP = 6` px, grosor `ARROW_W = 6`), que **parpadean juntas** (visible 75% / oculto 25% de `ARROW_BLINK_PERIOD = 500` ms). En el **límite** la flecha de ese lado se oculta. **Al mantener** `MOVE_LEFT`/`MOVE_RIGHT` el parpadeo se detiene: solo queda fija la flecha del botón activo y la contraria se oculta; al llegar al límite se dibuja como siempre (sin marcar la repetición). |
| `bool holdRepeat(uint8_t)` | Primer paso inmediato (`pressed`); mantenido, un paso cada `HOLD_REPEAT_TICK = 100` ms tras `HOLD_REPEAT_DELAY = 400` ms. |
| `_blink`, `_repeat`, `_repeatTick` | `Stopwatch`: ancla del parpadeo, retardo y cadencia de la repetición. |

### `MenuSound` — On/Off

| Miembro | Descripción |
|---------|-------------|
| `MenuSound()` | `_enabled = true`, `done()` en `false`. |
| `void begin()` | `_enabled = sound.enabled()` (parte del valor real) y ancla el parpadeo. **No dibuja ni borra nada.** |
| `void update()` | Solo actúa la tecla del lado de la flecha: `MOVE_LEFT` apaga si está ON y `MOVE_RIGHT` enciende si está OFF (cada uno con `SFX_CLICK`). `ACTION_RIGHT` **aplica** (`sound.setEnabled(_enabled)`, con `SFX_CONFIRM` solo si queda encendido) y marca `done()`; `ACTION_UP` cancela. |
| `void print()` | Borra la banda 45..53 y llama a `drawSelector()` (cada frame: la flecha parpadea, la palabra no). |
| `bool done()` | `true` tras confirmar o cancelar. |
| `void drawSelector()` | `"ON "` o `"OFF"` en `TEXT_6x8` centrado (ambos miden 18 px, así el centrado no se desplaza) y **una sola flecha** pegada a la palabra, en el **lado del destino**: `"OFF >"` (apunta a `MOVE_RIGHT`, que enciende) y `"< ON"` (apunta a `MOVE_LEFT`, que apaga). Parpadea visible 75% / oculto 25% de `ARROW_BLINK_PERIOD = 500` ms; la palabra no. |
| `_blink` | `Stopwatch`: ancla del parpadeo de la flecha. |

El valor del sonido **no es propio de la ventana**: el estado real vive en el
servicio global `Sound` (`setEnabled`/`enabled`), así que `MenuSound` solo lleva
el valor en edición y no necesita recordarlo entre entradas (a diferencia de
`MenuDifficulty`, que sí guarda el nivel confirmado).

Las constantes de geometría y parpadeo de ambas ventanas (`SEL_TOP = 45`,
`SEL_SIZE = 8`, `ARROW_*`, `HOLD_REPEAT_*`) son `static constexpr` de cada clase,
siguiendo la convención de `Config.h` (allí solo va lo compartido de verdad); la
banda 45..53 es la misma que los rombos del `Menu`, y su erase incluye la fila de
la línea separadora del pie (`Config::Screen::FOOT_LINE = 53`), que por eso
`Menu::restoreDiamondBand()` vuelve a pintar.

---

## 11. Clase `Engine` — despachador de ventanas

Separada del `.ino` en `Engine.h` / `Engine.cpp` (antes `App`). **No anida las
ventanas** pero las **posee como miembros**: `Boot`, `Legend`, `Menu`, `MenuCredits`,
`Game` son clases independientes (hermanas) declaradas como miembros `_boot`,
`_menu`, `_menuDifficulty`, `_menuSound`, `_menuCredits`, `_game`, `_legend`. No son
globales ni reciben las
ventanas por referencia.

### Responsabilidad

`Engine` es el **despachador puro**: su **estado interno** (`enum class State`)
decide qué ventana corre y cuándo cambiar (`changeState()`, que llama al `begin()`
de la ventana entrante). `loop()` no participa en las transiciones: solo llama a
`engine.update()` y `engine.print()`. `setup()` inicia el hardware y llama a
`engine.begin()` (primera transición → `boot.begin()`).

### Constructor

```cpp
Engine();
```

Sin parámetros: los servicios (`Display`, `Buttons`, `Sound`) los consume como
globales (sección 20) y las ventanas son sus propios miembros. En `Snake_II.ino`
los globales se definen **antes** de `Engine engine;`, así los constructores de
los miembros (p. ej. `Scroller`/`Food` que consultan `display.getWidth()`) ven
los globales ya construidos (misma TU, orden de definición).

### Reglas de esta arquitectura

1. **Todas las clases se inician en `setup()`** y se usan en `loop()`/`Engine`.
   Los constructores son livianos (los de las ventanas ya no guardan
   referencias: usan los servicios globales); el trabajo real va
   en `begin()`/`update()`. Los `begin()` de las ventanas los llama `changeState()`
   al entrar (la primera se lanza dentro de `setup()` vía `engine.begin()`).
2. **Las ventanas NO se anidan entre sí, el `Engine` las posee:** cada ventana es
   una clase propia con el patrón `begin()/update()/print()/done()` (no código
   inline dentro de `Engine`), y `Engine` solo las despacha. Desde el refactor de
   servicios globales son **miembros del `Engine`** (no globales): ninguna otra
   clase puede invocarlas.
3. **El estado determina qué se ve** (solo `Engine` conoce `State`) **y los valores
   se conservan**: las ventanas son instancias persistentes (hermanas, no se
   recrean), así sus miembros sobreviven entre transiciones; el `begin()` solo
   reinicia lo que se requiere al entrar.
4. **La lectura de botones está centralizada:** el hardware se lee **una sola vez
   por frame** en `loop()` (`buttons.read()`, principio de responsabilidad única).
   Ninguna ventana llama a `read()` en su `update()`: todas consumen los eventos
   `pressed`/`released` de esa misma lectura, así es seguro que varias partes del
   sistema (ventana activa + HUD futuro, etc.) compartan el estado del mismo frame
   sin que ninguno "se coma" los eventos de un ciclo.
5. **Las ventanas "capa" no limpian la pantalla:** `MenuDifficulty` y `MenuSound`
   no son ventanas de página completa, sino capas sobre el `Menu` ya dibujado.
   Por eso su `begin()` no dibuja nada (su `print()` borra y repinta cada frame
   solo la banda 45..53), y por eso el `Engine` vuelve de ellas con
   `menu.restoreDiamondBand()` + `changeState(State::MENU, false)` —el `false`
   salta el `Menu::begin()`, que sí provocaría el `clear()` de página completa.
   Ninguna otra ventana usa ese camino.

### Estados internos

```cpp
enum class State : uint8_t {
  BOOT = 0, MENU, NEW, CONTINUE, MENU_CREDITS, MENU_DIFFICULTY, MENU_SOUND, LEGEND
};
```

| Estado | Ventana | Notas |
|--------|---------|-------|
| `BOOT` | `Boot` | Animación de arranque (franjas). Al terminar (`done()`) pasa a `LEGEND`. Cualquier botón la termina. |
| `LEGEND` | `Legend` | Panel de botones: pad MOVE con 4 flechas + 4 rombos completos de ACTION en las posiciones de un pad que parpadean MUY rápido uno a la vez (ciclo lento) con la función del rombo activo centrada en el pie. Cualquier botón la cierra → menú (suena el efecto según el botón —CLICK/BACK/CONFIRM—; `Engine` no añade `SFX_BACK`). Solo se muestra tras el arranque. |
| `MENU` | `Menu` | Confirma con `ACTION_RIGHT` (`confirm()`). |
| `NEW` | `Game` | Nueva partida: `setDifficulty(_menuDifficulty.difficulty())` + `begin(true)`. Arranca con el conteo regresivo 3-2-1 (un `SFX_TICK` por dígito). Al salir (`done()`) suena `SFX_BACK`, el `Engine` sincroniza el récord (`menu.setBestScore(game.bestScore())`), **oculta/muestra "Continue" al volver** (`menu.setContinueAvailable(resumable)`, donde `resumable = !game.isGameOver() && game.score() > 0`: partida en curso **y** con puntos), deja la selección del menú en `Continue` si `resumable`, o en `New` en caso contrario (`menu.setSelected(...)`) y pasa a `MENU`. |
| `CONTINUE` | `Game` | Reanudar la partida anterior (`begin(false)`): queda en pausa y se retoma con `ACTION_RIGHT` (Btn2, "Select / Pause") o `ACTION_LEFT`; si no hay partida en curso arranca una nueva. Al salir (`done()`) igual que `NEW`. |
| `MENU_DIFFICULTY` | `MenuDifficulty` | Selector de nivel 1..10 **sobre el `Menu` ya dibujado**: sustituye solo la banda de rombos (45..53) por `< N >`, sin `clear()`. `MOVE_LEFT`/`MOVE_RIGHT` editan con repetición (`SFX_CLICK` por paso), `ACTION_RIGHT` aplica y `ACTION_UP` cancela. Al salir (`done()`) el `Engine` llama `menu.restoreDiamondBand()` y hace `changeState(MENU, false)`: **no** vuelve a llamar `Menu::begin()` (que haría `clear()`), solo repinta esa banda. El nivel queda en `MenuDifficulty::_difficulty` (sección 10.3). |
| `MENU_SOUND` | `MenuSound` | Selector On/Off **sobre el `Menu` ya dibujado**, misma banda y mismo regreso que `MENU_DIFFICULTY`. `MOVE_LEFT` apaga / `MOVE_RIGHT` enciende (`SFX_CLICK`), `ACTION_RIGHT` aplica con `sound.setEnabled()` (y `SFX_CONFIRM` solo si queda encendido), `ACTION_UP` cancela. El valor real vive en el servicio `Sound`, no en la ventana. |
| `MENU_CREDITS` | `MenuCredits` | 3 entradas navegables con `MOVE_LEFT`/`MOVE_RIGHT` y transición lateral (rol tamaño 2 **seleccionado con cuadro de borde a borde** y centrado en el alto restante del Body; nombre tamaño 1 plano en el pie). La transición usa el **mismo `Scroller` que el menú** pero con **2 bandas sincronizadas** (`BAND_HEIGHTS = {16, 8}` = altos de rol 12x16 y nombre 6x8): cada banda tiene su **propia tira** de 128x16 (el rol en `SLOT_ROLE`, el nombre en `SLOT_NAME`), que se compone **solo al entrar y al navegar** (`loadEntry` → `Scroller::compose`); `drawBand(slot, y, fg, bg)` vuelca su **canvas persistente** con sus colores y la tira se sobrescribe **columna a columna** con sus fondos, así la entrada anterior se mantiene hasta que la nueva la cubre (superposición al navegar rápido). El deslizamiento **arranca desde el borde** (`startSlide`, fuera de escena) y avanza **1 px cada 4 ms con acumulador por tiempo** (igual que el menú, ≈0,5 s). Al navegar suena `SFX_CLICK` y al salir (`done()`) suena `SFX_BACK` (lo toca el `Engine`) y pasa directo a `MENU`. |

### Métodos

| Método | Descripción |
|--------|-------------|
| `void begin()` | Primera transición: entra al test de píxeles (`changeState(State::BOOT)`). Se llama desde `setup()`. |
| `void update()` | Lee/actualiza la ventana activa y gestiona las transiciones de estado. Reproduce los efectos del sonido: `SFX_CONFIRM` al confirmar una opción del menú y `SFX_BACK` al volver a `MENU` desde cualquier ventana (excepto desde `Legend`, que toca su propio sonido según el botón, y desde `MENU_DIFFICULTY`/`MENU_SOUND`, que **no** los añaden: cada selector toca su propio `SFX_CONFIRM` al aplicar o `SFX_BACK` al cancelar). |
| `void print()` | Despacha el dibujo a la ventana activa. **Ya no limpia la
  pantalla (`display.clear()`)**: cada ventana la usa solo en su primer frame tras
  `begin()` y luego limpia/redibuja solo sus zonas dinámicas (sección 13). |
| `void setBestScore(uint16_t)` | Reenvía al menú para conservar el puntaje máximo entre sesiones. |
| `void changeState(State, bool beginWindow = true)` | Transición común: guarda el estado y llama `begin()` de la ventana destino. `beginWindow = false` **solo** omite ese `begin()`, y es exclusivamente para volver de `MENU_DIFFICULTY`/`MENU_SOUND`: el menú sigue en pantalla y solo hay que repintar la banda de rombos (`menu.restoreDiamondBand()`), sin el `clear()` de `Menu::begin()`. El resto del cableado de la transición (p. ej. `setDifficulty` al entrar en `Game`) se aplica siempre. No reproduce sonidos: cada ventana toca el suyo al confirmar o cancelar. |

### Patrón de ventana

Toda ventana implementa:

| Método | Descripción |
|--------|-------------|
| `begin()` | Restablece la ventana al entrar. `Engine` lo llama solo en `changeState()`. |
| `update()` | Maneja los eventos de botones (leídos una sola vez por `loop()` en `buttons.read()`; la ventana no llama a `read()`). |
| `print()` | Dibuja la ventana. Al entrar hace un `clear()` completo en el primer frame y luego solo limpia/redibuja sus zonas dinámicas. |
| `done()` | `true` cuando la ventana pide volver al menú. |

### Reglas del despachador

1. Solo `Engine` cambia de estado (nadie más conoce `State`).
2. Una ventana nunca cambia de estado ni conoce a las demás: expone `done()`.
3. `ACTION_UP` es el botón común "volver al menú" en todas las ventanas.
4. `ACTION_RIGHT` activa la opción del menú (su `confirm()`).
5. `begin()` de cada ventana se llama desde `changeState()`, nunca desde `loop()`.

---

## 12. Cómo compilar/probar

- IDE: Arduino IDE, placa `ESP32-S3 (Dev Module)` (verificar puerto).
- Librerías: Adafruit GFX + Adafruit_SSD1306.
- **IntelliSense en VS Code** (solo está instalada la extensión Microsoft C/C++, sin
  extensión de Arduino ni PlatformIO): el `<Arduino.h>` marcado como error lo
  resuelven las rutas del core, y **dónde van depende de cómo se abra el
  proyecto**:
  - El proyecto se abre con el workspace **`D:\Documents\ESP32S3\ESP32S3.code-workspace`**
    (carpeta raíz `.`, es decir `D:\Documents\ESP32S3`, que contiene `Snake_II` y
    `blink`). En un `.code-workspace` **no** se lee el `c_cpp_properties.json` de
    una subcarpeta: la configuración va en el bloque `settings` del propio
    `.code-workspace` con claves `C_Cpp.default.*`
    (`compilerPath`, `includePath`, `defines`, `cppStandard`, `cStandard`,
    `intelliSenseMode`). Ese archivo está **fuera del repo**, así que no se versiona.
  - Si se abre la carpeta `Snake_II` sola (File > Open Folder), entonces sí manda
    `.vscode/c_cpp_properties.json` (una configuración con nombre,
    `ESP32-S3 Dev Module (arduino-esp32 3.3.12)`, equivalente a las
    `C_Cpp.default.*` del workspace). Es la copia que sí queda en el repo.
  - Contenido de las rutas:
  - `compilerPath` al toolchain real del core
    (`packages/esp32/tools/esp-x32/2601/bin/xtensa-esp-elf-g++.exe`); de ahí la
    extensión saca sola los includes del sistema (libstdc++, newlib, xtensa).
  - Rutas del core: `cores/esp32` (Arduino.h), `variants/esp32s3` (pins_arduino.h),
    `libraries/Wire/src` y `libraries/SPI/src` (los incluye `Adafruit_SSD1306.h`).
  - SDK de ESP-IDF: `tools/esp32s3-libs/3.3.12/qio_qspi/include` (el `sdkconfig.h`
    que define `CONFIG_XTAL_FREQ`, `CONFIG_MMU_PAGE_SIZE`...) y
    `tools/esp32s3-libs/3.3.12/include/**` recursivo, que es la forma de
    reproducir en VS Code los ~200 `-iwithprefixbefore` que pasa el core.
  - Librerías del sketchbook `D:\Documents\Arduino\libraries`: `Adafruit_GFX_Library`,
    `Adafruit_SSD1306` y `Adafruit_BusIO` (esta última la necesita
    `Adafruit_GFX.h`, no `Adafruit_SSD1306.h`).
  - `cppStandard: c++20` (el core compila con `-std=gnu++2a`) y los `-D` del
    recipe: `F_CPU`, `ARDUINO`, `ARDUINO_ARCH_ESP32`, `ARDUINO_ESP32S3_DEV`,
    `ARDUINO_BOARD`, `ARDUINO_VARIANT`, `ARDUINO_USB_CDC_ON_BOOT=0`, `ESP_PLATFORM`...
    **Sin `ESP32=ESP32` no arranca:** viene de `build.extra_flags` del
    `platform.txt` y es el que hace que `Adafruit_SPIDevice.h` tome su rama de
    ESP32; sin él cae en el `typedef BitOrder BusIOBitOrder` y el core 3.3.12 ya
    no define ese tipo (ahora usa `SPI_MSBFIRST`).
  - En el workspace, `${workspaceFolder}` es `D:\Documents\ESP32S3`, así que la
    raíz del proyecto va como `${workspaceFolder}/Snake_II`.
  - `Snake_II/.vscode/settings.json` deja el `.aider-venv` (y su caché) fuera de
    la indexación y de las búsquedas; sin eso el workspace se indexa entero.
  - Rutas absolutas: al actualizar el core (o el toolchain) hay que revisar los
    `3.3.12` y `2601`, y hay que tocarlas **en los dos sitios**.
- **Warnings de compilación C++ activados** en la máquina de desarrollo vía
  `platform.local.txt` del core ESP32 (`compiler.cpp.extra_flags=-Wall -Wreorder`)
  para que desajustes como el orden de inicialización de miembros salten a la vista
  en la compilación (no entra en el repo: se configura a nivel del paquete del core).
- La demo actual (`Snake_II.ino`) usa `Display`, `Buttons` y el sonido integrado:
  el menú arranca **sin la opción "Continue"** (no hay partida en curso) y solo
  aparece al volver del juego con la partida viva **y con puntos** (score > 0);
  al navegar el menú y los
  créditos suena `SFX_CLICK`, al confirmar `SFX_CONFIRM`,
  al volver al menú `SFX_BACK`, la `Legend` suena según el botón pulsado (MOVE =
  CLICK, ACTION_UP = BACK, ACTION_RIGHT = CONFIRM) y en la opción "Sound" el
  **On/Off se edita en su propia ventana** `MenuSound` (selector con flechas en
  la banda de los rombos; `MOVE_LEFT`/`MOVE_RIGHT` cambian el valor,
  `ACTION_RIGHT` lo aplica y `ACTION_UP` cancela). En la opción "Dificultad" el
  **nivel 1..10 se edita en `MenuDifficulty`** (selector `< N >` con dos flechas
  parpadeantes que se
  ocultan en los límites; `MOVE_RIGHT` +1, `MOVE_LEFT` -1 con repetición al
  mantener presionado —al mantener, el parpadeo se detiene y solo queda fija la
  flecha del botón activo, la contraria se oculta; al llegar al límite se
  procesa igual que haber soltado el botón (vuelve el parpadeo normal)—,
  `ACTION_RIGHT` lo aplica y
  `ACTION_UP` cancela). En el juego (`Game`): `SFX_TICK` en cada dígito del
  conteo 3-2-1, `SFX_START` (jingle GO!) al arrancar la partida después del "1",
  `SFX_TURN` al girar, `SFX_PAUSE`/`SFX_RESUME` al pausar/reanudar,
  `SFX_EAT` al comer, `SFX_GAME_OVER` al morir y `SFX_BACK` al volver al menú.
- Con SDA=8 y SCL=9, dirección 0x3C.

---

## 13. Esquema de renderizado y limpieza (sin `clear()` global)

El juego ya no limpia la pantalla completa en cada frame: `Engine::print()` **no**
llama a `display.clear()`, lo decide cada ventana.

### Reglas

1. **Clear completo solo al entrar:** cada ventana hace `display.clear()` en su
   **primer `print()`** después de su `begin()` (flag `_redraw` puesto en
   `begin()` y apagado tras ese primer dibujo). Ese primer clear elimina la resaca
   de la ventana anterior y deja el fondo listo. Los métodos que reinician la
   animación (`Menu::begin/setOptions/setSelected/setContinueAvailable`,
   `setBestScore` si cambia)
   también activan ese flag.
2. **Estáticos una sola vez:** títulos, pies, línea separadora, cuadro de
   selección, rótulos y pads se dibujan en ese primer frame y **ya no se vuelven a
   dibujar**; persisten en el buffer (que `display.show()` vuelca completo cada
   frame).
3. **Dinámicos por frame, borrando solo lo necesario:**

   | Ventana | Estáticos (una vez) | Dinámicos por frame |
   |---------|---------------------|---------------------|
   | `Boot` | — (primer frame: clear completo + `firstPrint()`, la línea completa de `BAR_W` columnas de cada franja) | Por franja: su columna de cabeza en blanco y, en negro, la que deja libre por la cola (`drawBars()`, con el módulo normalizado en `drawBar` para el rebalse); si no cambió el desplazamiento (`_step == _prevStep`) no se dibuja nada |
    | `Menu` | Cuadro blanco (25..42), título, pie (línea 54 + texto) | Banda de la opción (26..41) con `Scroller::blit` —**solo cuando el scroller tiene algo nuevo que volcar** (navegar, recomponer o tras el `clear`; en reposo se salta, sección 14)— + rombos (banda 45..53); y además la banda 45..53 + la línea separadora del pie cuando `restoreDiamondBand()` deja `_diamondsDirty` (regreso de `MenuDifficulty`/`MenuSound`, sin `clear()`) |
    | `MenuDifficulty` | — (entra sobre el `Menu` ya dibujado, no borra nada) | Selector `< N >`: borra y redibuja **cada frame** la banda 45..53 (número centrado estático con ancho constante + dos flechas laterales que parpadean juntas, ocultas en su límite; al mantener un botón el parpadeo se detiene y solo queda fija la flecha del botón activo, ocultándose la contraria; al llegar al límite se procesa igual que haber soltado el botón, volviendo el parpadeo normal) |
    | `MenuSound` | — (entra sobre el `Menu` ya dibujado, no borra nada) | Selector ON/OFF: borra y redibuja **cada frame** la banda 45..53 (palabra centrada estática + flecha única en el lado del destino, que parpadea) |
    | `MenuCredits` | Título + cuadro blanco del rol | Bandas rol/nombre (`Scroller`, 2 bandas sincronizadas; se vuelcan juntas en el mismo frame y solo si hay algo nuevo que pintar, sección 14) |
   | `Legend` | Rótulos, pad MOVE y los 4 rombos fijos | Zona del rombo activo (cuadro 9x9, parpadeo) + texto del pie (banda 54..63) solo si cambia el rombo; al cambiar, se restaura completo el rombo que deja de ser activo (evita que quede borrado si el cambio lo pilló en su fase oculta) |
   | `Game` | Primer frame: clear completo + Header (puntaje 12x16 izq., segundos restantes de la comida especial 12x16 der.) y alimento y serpiente | Header solo si cambia el puntaje o `_food.specialTime()` (banda 0..15); tablero (Body 16..63) solo si `_dirtyBoard` (movimiento, comida nueva, transición de estado): borra el Body, redibuja alimento + serpiente; overlay "3-2-1"/"PAUSA"/"GAME OVER"/festejo de récord (texto invertido sobre banda blanca: cuadro centrado para el conteo, de lado a lado para PAUSA, GAME OVER y los letreros del festejo "BUT"/"YOU ARE"/"THE BEST") en cada frame según el estado —en el conteo, al final de cada dígito el número y su cuadro se ocultan (`COUNT_HIDE_MS`), marcando `_dirtyBoard` una sola vez para restaurar el tablero —; al morir superando el récord, el "GAME OVER" es un ciclo "GAME OVER" → "BUT" → "YOU ARE" → "THE BEST" (`NEW_BEST_SIGN_MS` cada uno) que se repite hasta que se presiona un botón, y el `SFX_NEW_BEST` suena solo la primera vez que aparece el letrero "THE BEST" |

4. Las ventanas `MenuDifficulty` y `MenuSound` **no hacen `clear()`**: son
   capas sobre el `Menu` ya dibujado. Comparten la banda dinámica de los rombos
   (45..53, que incluye la fila de `Config::Screen::FOOT_LINE = 53`): desde su
   primer frame la borran y la redibujan **cada frame** con su selector
   (`< N >` o ON/OFF, con las flechas parpadeantes). Todo lo demás del `Menu`
   (título, cuadro de la opción, pie con "Best") queda intacto detrás. Al
   salir, el `Engine` llama `menu.restoreDiamondBand()` y vuelve con
   `changeState(State::MENU, false)`, que **no** ejecuta `Menu::begin()`: el
   `Menu` marca `_diamondsDirty` y en su siguiente `print()` repinta solo la
   banda 45..53 y la línea del pie (los rombos vuelven). Por eso el erase de los
   selectores tiene que incluir la fila de la línea, que de otro modo quedaría
   borrada. Los flags `_redrawSound` y `_redrawDifficulty` ya no existen.

---

## 14. Clase `Scroller` — API (scroller de 1 bit)

Encapsula la animación "scroller de 1 bit": compone un texto en un **array de
`int8_t`** donde cada byte representa **1 columna de 8 píxeles** (bit 0 = fila 0,
bit 7 = fila 7) y lo desliza lateralmente. **Una sola banda** por instancia.
La franja es siempre **fondo blanco y texto negro** (sin parámetros de color).

- **Constantes:** `STRIP_W = Config::Screen::WIDTH` (128), `STRIP_H = 24` (máx.
  altura de texto 18x24), `ANIM_TICK = 4` ms por píxel (vuelo ≈ 0,5 s).
- **Constructor:** `Scroller()` — sin parámetros. No reserva memoria dinámica
  (el array `_strip` es un miembro por valor), así que la copia está permitida
  aunque se mantiene bloqueada por coherencia con el resto del proyecto.
- **API:**
  - `begin()` — reposiciona el deslizamiento (objetivo 0) y marca `_dirty`.
  - `setTexto(const char* text, uint8_t height, uint8_t size)` — compone el
    texto centrado en un array de `int8_t` (cada byte = 1 columna de 8 px).
    Calcula el **límite de caracteres** según el tamaño (`ancho / (6 * size)`)
    y **trunca silenciosamente** si el texto excede el límite. El texto se
    compone en un canvas auxiliar (`GFXcanvas8`) y luego se extraen las
    columnas. Marca `_dirty`.
  - `startSlide(int8_t dir)` — `+1` entra por la derecha, `-1` por la izquierda;
    arranca desde el borde (`_slideX = ±ancho`) y marca `_dirty`.
  - `animate()` — avanza 1 px por `ANIM_TICK` ms (acumulador por tiempo); a
    llamar en `update()`. Mientras `_slideX != 0` pone `_dirty`: en cada frame
    hay algo nuevo que pintar, y al llegar a 0 queda un **último volcado** que
    asienta la banda (del frame siguiente en adelante `blit()` no hace nada).
  - `redraw()` — fuerza el volcado en el próximo frame: la ventana la llama
    **después de su `display.clear()`**, que se lleva por delante lo ya volcado.
  - `blit(int16_t y)` — vuelca la franja a la pantalla en la fila `y`, con
    **fondo blanco y texto negro** (sin parámetros de color). A llamar en
    `print()`. **Es un no-op si no hay nada nuevo que volcar** (`_dirty` a 0).
- **Dirty flag (reposo):** con el flag `_dirty` el `blit()` se salta cuando la
  franja está en reposo, que es el caso normal. Lo ponen `begin()`, `setTexto()`,
  `startSlide()`, `animate()` (mientras haya desplazamiento) y `redraw()`; lo
  limpia `blit()` al volcar.
- **Usos:** `Menu` = 1 instancia (16 px, `blit(TEXT_SEL_TOP)`); `MenuCredits` = 2
  instancias sincronizadas (`_scrollerRol` 16 px + `_scrollerNombre` 8 px,
  ambas reciben `startSlide()` y `animate()` en el mismo frame). En reposo
  `blit()` es un no-op (dirty flag de `Scroller`).
- **Detalle de diseño:** `setTexto()` usa `display.getTextWidth()` para el
  centrado y `display.getWidth()` para el límite de caracteres. El array
  `_strip[STRIP_H / 8][STRIP_W]` almacena las columnas: `_strip[row][col]` es
  un byte con los 8 píxeles de esa columna en ese grupo de filas. `Boot`
  conserva su propia animación (`ANIM_TICK = 30`, bandas **completas** que
  avanzan solas por borde) — **no** usa `Scroller`.

---

## 15. Namespace `Sprite` — API (sprites de la serpiente y comida especial)

Ubicación: `Sprite.h`. Tabla estática con los sprites de las partes de la
serpiente (estilo Nokia) y el sprite de la **comida especial**. Es un
**namespace de solo datos** (no una clase): no necesita instancia
ni archivo `.cpp`; los sprites se leen con `Sprite::pixel(part, x, y)` y
`Sprite::specialPixel(x, y)`.

**Empaquetado a 1 bit por píxel.** Ninguna tabla guarda un byte por píxel: cada
sprite se escribe **directamente como un literal binario** del tamaño exacto de sus
px (16 bits = un `uint16_t` para los 4×4, 8 bits = un `uint8_t` por fila para la
comida especial), así que en memoria solo queda la versión empaquetada y no hay
ningún paso de conversión ni tabla intermedia.

| Tabla | Antes | Ahora | Bytes |
|-------|-------|-------|-------|
| Sprites de la serpiente (4×4) | `uint8_t SPRITES[COUNT][4][4]` | `uint16_t SPRITES[COUNT]` (16 px = 16 bits) | 432 → **54** |
| Comida especial (8×4) | `uint8_t SPECIAL_FOOD[4][8]` | `uint8_t SPECIAL_FOOD[4]` (1 byte por fila: 8 px = 8 bits) | 32 → **4** |

Los literales binarios llevan separadores `'` para que las filas del dibujo se sigan
leyendo: en los 4×4, cuatro grupos de 4 bits (uno por fila, de arriba abajo); en la
comida especial, dos grupos de 4 por fila. El px `(0,0)` (esquina superior izquierda)
es el bit más significativo y el último px, el bit 0; el acceso a un píxel concreto lo
hacen las funciones `constexpr` `pixel()` / `specialPixel()` (el juego nunca indexa
los bits a mano).

```cpp
constexpr uint16_t SPRITES[COUNT] = {
  0b0110'0110'0100'0100,  // TAIL_TO_UP
  0b0000'0011'1111'0000,  // TAIL_TO_RIGHT
  ...
};

```cpp
constexpr uint8_t SIZE = 4;                 // sprite de 4×4 px
constexpr uint8_t BITS = SIZE * SIZE;       // px por sprite = bits de su uint16_t

enum Part : uint8_t {
  TAIL_TO_UP = 0, TAIL_TO_RIGHT, TAIL_TO_DOWN, TAIL_TO_LEFT,   // cola
  BODY_TO_UP,    BODY_TO_RIGHT,  BODY_TO_DOWN,   BODY_TO_LEFT, // cuerpo
  CORNER_RIGHT_UP, CORNER_RIGHT_DOWN, CORNER_LEFT_UP, CORNER_LEFT_DOWN, // curvas
  HEAD_UP_CLOSE, HEAD_RIGHT_CLOSE, HEAD_DOWN_CLOSE, HEAD_LEFT_CLOSE,     // cabeza: fauces cerradas
  HEAD_UP_OPEN,  HEAD_RIGHT_OPEN,  HEAD_DOWN_OPEN,  HEAD_LEFT_OPEN,      // cabeza: fauces abiertas
  BELLY_TO_RIGHT, BELLY_TO_LEFT,                  // panza recta (1 sprite por par de direcciones)
  BELLY_RIGHT_UP, BELLY_RIGHT_DOWN, BELLY_LEFT_UP, BELLY_LEFT_DOWN,      // panza curva
  EMPTY,
  COUNT
};
```

### Miembros

| Miembro | Contenido |
|---------|-----------|
| `SIZE` | Lado del sprite en píxeles (4). |
| `BITS` | Píxeles por sprite de la serpiente (16) = bits de su `uint16_t`. |
| `SPRITES[COUNT]` | Tabla de sprites **empaquetados** (1 bit por píxel) indexada por `Part`: cada sprite es un literal binario de 16 bits, en cuatro grupos de 4 (una fila por grupo). Rango: 0..3 cola, 4..7 cuerpo, 8..11 curvas, 12..15 cabeza cerrada, 16..19 cabeza abierta, 20..25 panza, 26 `EMPTY`, 27 `COUNT`. |
| `pixel(Part, x, y)` | `constexpr`: píxel del sprite (`x` = columna, `y` = fila) desde el `uint16_t` empaquetado. |
| `EMPTY` | Sprite vacío (todo fondo). |
| `COUNT` | Cantidad de sprites de la tabla. |
| `SPECIAL_FOOD_W` / `SPECIAL_FOOD_H` | Ancho (8) y alto (4) en píxeles del sprite de la comida especial. |
| `SPECIAL_FOOD[SPECIAL_FOOD_H]` | Sprite de 1 bit de la **comida especial** (8×4 px) **empaquetado por fila**: un literal binario de 8 px por fila (px `(0,0)` = bit 7) = 4 bytes en total, tabla aparte por no encajar en los 4×4 de la serpiente. |
| `specialPixel(x, y)` | `constexpr`: píxel de la comida especial (`x` = columna, `y` = fila). |

La panza recta comparte sprite por par de direcciones: `BELLY_TO_RIGHT` =
`BELLY_TO_UP` y `BELLY_TO_LEFT` = `BELLY_TO_DOWN`.

`Sprite.h` (antes `SnakeSprites.h`) forma parte del respaldo del juego original
adaptado al estilo
del proyecto (pie `// Fin`, cabecera descriptiva, comentarios de los sprites
corregidos). Los consume `Snake` (la parte de cada segmento
–cola/cuerpo/curva/panza/cabeza– se deriva de la geometría de sus vecinos y de
la dirección de la cabeza, ver sección 18) y `Game` (los dibuja en el tablero,
ver sección 16); `Food` usa `SPECIAL_FOOD` (ver sección 17).

---

## 16. Clase `Game` — API (ventana del juego)

Ubicación: `Game.h` / `Game.cpp`. Ventana del juego de la serpiente
(estado `NEW`/`CONTINUE` del `Engine`), reemplaza al placeholder `InfoWindow`
(eliminado). **`Game` coordina**: la lógica de la serpiente (buffer circular,
giro pendiente, colisiones, elección de sprites) vive en la clase `Snake`
(sección 18), que **no toca `Display`/`Sound`/`Food`**. `Game` decide el ritmo
(dificultad), lee los botones y traduce MOVE a `Snake::Dir`, llama
`snake.step()` y maneja el resultado (`Result` MOVED/ATE/DIED) con puntaje,
sonidos y regeneración del alimento, y dibuja el tablero volcando los segmentos
que `Snake` expone (`length`/`segment`/`headPart`).

### Constructor

```cpp
Game();
```

Sin parámetros: consume los servicios globales (`Display`, `Buttons`, `Sound`,
sección 20). Los miembros `_food`, `Snake _snake;` y `Menu`-independientes se
construyen solos en su lista de inicialización.

### Constantes

| Constante | Valor | Significado |
|-----------|-------|-------------|
| `Config::Difficulty::MIN_LEVEL` / `MAX_LEVEL` / `DEFAULT_LEVEL` | 1 / 10 / 5 | Nivel de dificultad acotado (mismo rango y fuente única que el menú; ya no se duplica en `Game`). |
| `SPRITE_SCALE` | 2 | Px por lado de cada píxel del sprite al dibujarlo: los sprites de `Sprite::SIZE` (4×4) se dibujan como bloques 2×2 para llenar la celda (`Sprite::SIZE * SPRITE_SCALE = Config::Screen::CELL` = 8). Antes era el `2` literal de `drawSprite`. |
| `COUNTDOWN_MS` | 3000 | Duración del conteo regresivo inicial (3 s, un dígito por segundo: 3-2-1). |
| `COUNT_HIDE_MS` | 250 | Fase de parpadeo al final de cada dígito: el número (y su cuadro) se ocultan antes de que aparezca el siguiente. |

La geometría del tablero (`COLS`=16, `ROWS`=6, `MAX_LENGTH`=96, rejilla de 16×6
celdas en el Body de 128×48) vive en `Snake` (sección 18); `Game` la
usa vía `Snake::COLS`/`Snake::ROWS` (p. ej. para construir `Food`). **El tamaño de
la celda no se declara en `Game`**: se toma de `Config::Screen::CELL` (8 px), la
fuente única que también usa `Food` al dibujar el alimento; lo propio de `Game` es
solo el factor de escala del sprite (`SPRITE_SCALE`).

### Métodos

| Método | Descripción |
|--------|-------------|
| `void begin(bool newGame)` | `true` = nueva partida (reinicia todo y arranca el conteo regresivo 3-2-1). `false` = reanudar la partida anterior en pausa; si no hay partida en curso arranca una nueva. Conserva el récord (`_bestScore`) entre partidas. Llama `snake.clearPending()` (ningún giro pendiente al entrar). |
| `void setDifficulty(uint8_t level)` | Nivel 1..10 (clamp). **Se aplica EN CALIENTE, también con la partida iniciada**: recalcula `_moveDelay` al instante, por lo que una partida en curso (PLAY o PAUSE) sigue el nuevo ritmo al cambiar el nivel desde el menú; también vale para la próxima partida nueva (`reset()` la vuelve a derivar). |
| `void update()` | Estado `START`: pre-gira con MOVE (giro pendiente), entra a `PLAY` con `ACTION_RIGHT` o al agotarse `COUNTDOWN_MS` (al arrancar la partida suena `SFX_START`, el jingle GO! después del "1"). `PLAY`: gira con MOVE (sin reversa directa, queda un único giro pendiente que se aplica en el siguiente paso), avanza un paso cada `_moveDelay` ms y `ACTION_RIGHT` (Btn2, "Select / Pause") pausa. `PAUSE`: reanuda con `ACTION_RIGHT` (o `ACTION_LEFT`). `GAME_OVER`: cualquier ACTION vuelve al menú. `ACTION_UP` (Btn1, "Volver") sale en cualquier estado menos `GAME_OVER`. |
| `void print()` | Renderizado por zonas (ver sección 13). |
| `bool done()` | `true` al pedir volver al menú. |
| `bool isGameOver()` | `true` si al salir (`done()`) la partida terminó en `GAME OVER`; lo usa el `Engine` (junto con `score() > 0`) para dejar la selección del menú en `New` (Game Over o sin puntos) o `Continue` (partida en curso con puntos). |
| `uint16_t score()` / `bestScore()` | Puntaje actual / récord (el récord solo se actualiza al terminar en GAME OVER, ver "Comer"/"Colisión"). El `Engine` sincroniza `bestScore()` con el menú al salir. |
| `bool occupied(uint8_t x, uint8_t y)` | ¿Una celda está ocupada por la serpiente? Lo consulta `Food` (al colocar el alimento en una celda libre). **Delega en `Snake::occupied`.** |

### Reglas del juego (qué coordina Game)

- **Movimiento:** la cabeza avanza 1 celda por paso con **wrap en X y en Y**
  (sale por un borde, aparece por el opuesto, estilo Nokia). La velocidad
  (`_moveDelay` ms por paso) es lineal con la dificultad con **pasos alternados
  de 101/102 ms** (102 en los niveles 1, 4 y 7): `1000 - (nivel-1)*101 -
  (nivel+1)/3`. Los 9 saltos suman 912 ms, de **1000 ms en el nivel 1 a 88 ms
  en el nivel 10** (secuencia 1000, 898, 797, 696, 594, 493, 392, 290, 189, 88;
  101,33 ms por nivel no es entero, por eso se alternan 6 saltos de 101 y 3 de
  102). La fórmula vive en `Game::speedFor`.
- **Paso (`Game::step`):** llama `snake.step(_food.x(), _food.y())` y maneja el
  resultado de `Snake::Result`:
  - `MOVED` → solo repintar (`_dirtyBoard`).
  - `ATE` → **puntaje suma el nivel de dificultad actual** (`_score += _difficulty`,
    no +1 fijo; al poder cambiar la dificultad en caliente, vale la del momento de
    comer), suena `SFX_EAT`, regenera el alimento
    (`_food.spawn(Food::Type::NORMAL, *this)`; si no hay celdas libres → tablero
    lleno → `die()`, la partida **se gana**/termina).
  - `DIED` → `die()`: `GAME_OVER` (`SFX_GAME_OVER`), `_hasGame = false`.
- **Comer/crecer, colisión, giro, sprites:** los resuelve `Snake` (sección 18);
  `Game` solo consume el resultado y repinta.
- **Alimento:** rombo simétrico centrado en la celda (dos `fillTriangle`), como
  el rombo del menú; lo dibuja `Food` (sección 17). La **semilla del generador aleatorio** se
  fija en `Game::reset()` (lo consume `Food::spawn`) con `randomSeed(esp_random()
  ^ (uint32_t)nowMs())`: `esp_random()` es el RNG de hardware del ESP32 (entropía
  real, no predecible como `micros()`) y se combina con el reloj de 64 bits.
- **Colisión con el cuerpo:** al mover, la celda destino es ilegal si coincide
  con el cuerpo **salvo la celda de la cola cuando NO come** (la cola se libera
  ese paso, como en el Nokia original; la cola es bloqueante solo cuando come);
  la resuelve `Snake::step` (ver sección 18). Si colisiona: `GAME_OVER`
  (`SFX_GAME_OVER`), informa `SFX_BACK` al volver y `_hasGame = false` (un
  `Continue` posterior arranca de nuevo). **El récord (`_bestScore`) se
  verifica/actualiza solo aquí, en el GAME OVER** (o al terminar el tablero
  lleno, que también pasa por `die()`): partidas abandonadas con `ACTION_UP`
  no cuentan. Si el puntaje **supera el récord**, se activa el
**festejo de nuevo récord**: en vez del letrero estático, la secuencia
   "GAME OVER" → "BUT" → "YOU ARE" → "THE BEST" se repite en ciclo
   (`NEW_BEST_SIGN_MS` por letrero, `_newBest`/`_gameOverMs`/`_celeSfx`) hasta que
se presiona un botón, y la fanfarria (`SFX_NEW_BEST`) suena solo la primera vez
  que aparece el letrero "THE BEST" (YOU ARE ya no la dispara; el "GAME OVER" ya sonó en `die()` con `SFX_GAME_OVER`).
- **Pausa y salida:** `ACTION_RIGHT` (Btn2, "Select / Pause") durante `PLAY`
  pausa la partida (panel "PAUSA", suena `SFX_PAUSE`); en `PAUSE` retoma con el
  mismo botón o con `ACTION_LEFT` (suena `SFX_RESUME`). `ACTION_UP` (Btn1, "Volver")
  durante la partida vuelve al menú
  **sin perderla** (`_hasGame` mantiene el tablero; `Continue` la reanuda en
  pausa).
  `GAME_OVER` deja `_hasGame = false`. Al salir, el `Engine` deja la selección
  del menú en **`Continue`** si la partida siguió en curso **y con puntos** (sale
  con `ACTION_UP` tras haber comido) o en **`New`** si terminó en `GAME OVER` o la
  partida no tiene puntos (`resumable = !game.isGameOver() && game.score() > 0`;
  lo aplica con `menu.setSelected(...)` antes de pasar a `MENU`); además la
  opción **"Continue" en el menú se oculta cuando no hay partida que reanudar o
  cuando la partida no tiene puntos**
  (`menu.setContinueAvailable(resumable)`): al arrancar, tras un
  `GAME OVER` o al salir sin puntos la lista queda con 4 opciones (New, Dificultad, Sound, Créditos).
- **Header:** puntaje en `TEXT_12x16` (izq., NO se mueve) y segundos restantes
  de la **comida especial** en `TEXT_12x16` (der., `_food.specialTime()`, por
  ahora valor fijo 60 solo para el layout). Se redibuja solo cuando cambian.
  Ya no muestra el récord `HI` (el "Best: N" queda solo en el menú).
- **Overlays:** al iniciar el **conteo regresivo 3-2-1** (un dígito por segundo,
  `COUNTDOWN_MS/3` ms por dígito, texto centrado) y "PAUSA" / "GAME OVER" como
  **banda blanca de lado a lado** (todo el ancho del Body). `drawOverlay(title,
  fullWidth)` dibuja **cuadro centrado** alrededor del texto (`fullWidth = false`,
  el conteo) o **banda de borde a borde** (`fullWidth = true`, PAUSA y GAME OVER).
  La banda sobresale del texto **2 px por arriba y 0 px por abajo** y está
  **centrada como rectángulo a lo alto del Body** (16..63), no el texto: `y` se
  deriva del alto de la banda (`bandH = h + 2`), y el texto queda 2 px dentro;
  siempre `fillRoundRect` blanco + texto invertido negro `TEXT_12x16`
  (`drawTextInverted`) centrado en el rectángulo. **Festejo de nuevo récord:** si al
  morir se supera el "Best", el "GAME OVER" pasa a un **ciclo** de los cuatro letreros
  ("GAME OVER" → "BUT" → "YOU ARE" → "THE BEST", banda de borde a borde, `NEW_BEST_SIGN_MS`
  cada uno) que se repite hasta que se presiona un botón; el `SFX_NEW_BEST` (festejo)
  suena solo la **primera** vez que aparece el letrero "THE BEST" (YOU ARE ya no la dispara). **Parpadeo del conteo:** al
  final de cada dígito (los últimos `COUNT_HIDE_MS = 250` ms de su segundo) el
  número **y su cuadro desaparecen** antes de que aparezca el siguiente: el cambio
  es como un parpadeo. Al ocultarlo se marca `_dirtyBoard` una sola vez (restaura
  el tablero debajo del cuadro; flag `_overlayHidden`, reiniciado en `reset()`).
  **Pitido por dígito:** cada dígito suena `SFX_TICK` al aparecer (flag
  `_lastCount`, reiniciado en `reset()`; el jingle `SFX_START` ya no suena en
  `reset()`: se reserva para el arranque real de la partida). Al pasar de
  `START` a `PLAY` —por `ACTION_RIGHT` o al agotarse el conteo— suena
  `SFX_START` (GO!), justo después del "1", para iniciar la partida.
  Al volver a `PLAY` se marca
  `_dirtyBoard` (borra el overlay bajo el tablero).

### Validación en host (MinGW)

Antes de escribir el código se simuló en el PC la lógica núcleo (misma aritmética
de ring buffer, colisiones y selección de sprites): comer/crecer (cola se
mantiene, score, longitud), longitud estable sin comida (la cola avanza),
colisión real detectada (la cabeza no avanza), la cabeza **puede** ocupar la
celda de la cola (anillo casi cerrado), wrap horizontal y vertical, y cargo de
integridad de 400 pasos con giros (celdas únicas + adyacencia sin romper).
Ese núcleo ahora vive en `Snake` (sección 18), que por no depender de
`Display`/`Buttons`/`Sound`/`Food` se puede compilar y probar en el PC
directamente (sin stubs de esas clases).

---

## 17. Clase `Food` — API (alimento del tablero)

Ubicación: `Food.h` / `Food.cpp`. Encapsula el estado y la lógica del alimento
de la serpiente, extraídos de `Game` (antes `_food`/`_hasFood`/`_specialTime` y
los métodos `spawnFood()`/`drawFood()` vivían en `Game`). Es un componente del
juego (no una ventana): no tiene `begin()`/`update()` propios sino que es usado
por `Game` (miembro `_food`).

### Constructor

```cpp
Food(uint8_t cols, uint8_t rows, uint8_t top);
```

Recibe la geometría del tablero (dibuja con la `Display` global, sección 20):
rejilla de
`cols`×`rows` celdas de `Config::Screen::CELL` px a partir de la fila `top` (el
Body). `Game` la
construye así: `_food(Snake::COLS, Snake::ROWS, Config::Screen::BODY_TOP)` (las
constantes del tablero viven en `Snake`).

**El tamaño de la celda no se declara en `Food`:** el viejo `static constexpr
uint8_t CELL = 8` privado duplicaba `Config::Screen::CELL`; ahora ambas clases
(`Food` y `Game`) toman la celda de `Config` (sección 19), que es la fuente
única. `Food.cpp` incluye `Config.h`.

### Enum y constantes

```cpp
enum class Type : uint8_t { NORMAL = 0, SPECIAL };
static constexpr uint8_t SPECIAL_TIME_DEFAULT = 60;   // segundos iniciales de la especial (layout)
```

| Tipo | Significado |
|------|-------------|
| `NORMAL` | Comida común, dibujada como rombo simétrico centrado en la celda (como el rombo seleccionado del menú). |
| `SPECIAL` | Comida especial, dibujada con el sprite `Sprite::SPECIAL_FOOD` (8×4 px, centrado verticalmente en la celda) y con temporizador (`specialTime`, los segundos que el Header muestra a la derecha). Por ahora el temporizador es solo layout (valor fijo). |

### Métodos

| Método | Descripción |
|--------|-------------|
| `void begin()` | No hay alimento. |
| `bool spawn(Type, const Game&)` | Coloca un alimento del tipo dado en una **celda libre al azar**. La ocupación la responde el tablero (`Game::occupied`, que quedó público para esto): se cuentan las libres y se elige la `pick`-ésima (dos recorridos, sin buffer). Devuelve `false` si no hay celdas libres (tablero lleno → partida ganada), en cuyo caso queda sin alimento. |
| `void clear()` | Quita el alimento (no hay). |
| `bool has()` | `true` si hay alimento en el tablero. |
| `Type type()` | Tipo del alimento actual. |
| `uint8_t x()` / `y()` | Columna (0..`COLS`-1) / fila (0..`ROWS`-1). |
| `uint8_t specialTime()` | Segundos restantes de la comida especial. |
| `void setSpecialTime(uint8_t s)` | Actualiza el temporizador de la especial. |
| `void draw() const` | Dibuja según el tipo: `NORMAL` → rombo (`drawNormal`, dos `fillTriangle`); `SPECIAL` → sprite (`drawSpecial`, píxel a píxel del sprite de 1 bit). Si no hay alimento no dibuja nada. |

`Game` usa `_food` así: lo genera en `reset()` y al comer (`_food.spawn(Food::Type::NORMAL, *this)`; si devuelve `false` se muere), consulta `_food.has()/_food.x()/_food.y()` para detectar comida y boca abierta, dibuja `_food.draw()` al volcar el tablero y muestra `_food.specialTime()` en el Header. La construye en su lista de inicialización:
`_food(Snake::COLS, Snake::ROWS, Config::Screen::BODY_TOP)` (las constantes del tablero viven en `Snake`, sección 18).

---

## 18. Clase `Snake` — API (lógica pura de la serpiente)

Ubicación: `Snake.h` / `Snake.cpp`. Encapsula toda la lógica de la serpiente,
extraída de `Game` (refactor de la rama principal del proyecto; antes vivía en
las secciones Insecto/16 de este README). Es un componente del juego (no una
ventana): **no depende de `Display`, `Sound` ni `Food`** —solo de `Arduino.h`
(`delay`, `random`) y de `Sprite.h` (los enums de partes/conexiones, para elegir
los sprites que `Game` dibujará)—, de modo que se puede compilar y probar en el
PC sin el resto del proyecto.

### Qué hace y qué no hace

`Snake` mantiene el **buffer circular** de segmentos, la dirección commitida, el
giro pendiente (sin buffer ni reversa directa), el avance con wrap, la detección
de colisión, comer/crecer y la elección del sprite de cada parte. `Game` es quien
**coordina**: le pasa el alimento en cada paso, aplica puntaje/sonidos con el
resultado, regenera la comida y dibuja. `Game` también mantiene la **dificultad
(el ritmo)** y la **velocidad** aquí no intervienen: `Snake::step` se llama una
vez por paso y `Game` controla la cadencia.

### Constructor

```cpp
Snake();
```

Deja la serpiente en estado vacío (longitud 0, dirección `NONE`); hay que llamar
`reset()` antes de usarla.

### Constantes

| Constante | Valor | Significado |
|-----------|-------|-------------|
| `COLS`, `ROWS` | 16, 6 | Tablero: rejilla de 16×6 celdas de 8 px en el Body (128×48). Antes vivían en `Game`; `Game` y `Food` las usan vía `Snake::COLS`/`Snake::ROWS`. |
| `MAX_LENGTH` | 96 | Cantidad máxima de segmentos (una celda por segmento; 16×6 celdas caben, y nunca se repite celda viva). |
| `Dir::NONE` / `UP` / `RIGHT` / `DOWN` / `LEFT` | 0 / 1 / 2 / 3 / 4 | Direcciones. El orden (UP=1..LEFT=4) coincide con el orden de los sprites por dirección (`dir-1`). `NONE` = sin dirección (estado vacío). |
| `Result::MOVED` / `ATE` / `DIED` | — | Resultado de `step`: avanzó sin comer / comió (crece, la cola NO avanza ese paso) / colisión (muerte, sin moverse). |

### Métodos

| Método | Descripción |
|--------|-------------|
| `void reset()` | Inicializa la serpiente: células `(1,2)..(4,2)`, cabeza a la derecha, longitud 4, giro pendiente `NONE`. |
| `void clearPending()` | Descarta el giro pendiente (`_nextDir = NONE`). Lo llama `Game::begin()` al entrar a la ventana (START) para no reanudar con un giro acumulado del menú. |
| `bool turn(Dir d)` | Maniobra con **único giro pendiente, sin buffer ni reversa directa**. Se evalúa **siempre** desde la dirección actual de la cabeza (`_dir`, la COMMITIDA, la que usará en el próximo paso): desde ella solo hay 3 posibilidades —seguir, giro a la izquierda, giro a la derecha— y la contraria (180°) se **ignora**. Devuelve `true` si el giro quedó pendiente (el último válido pisa al anterior) para que `Game` toque `SFX_TURN` solo al aceptar. Así, al girar varias veces entre dos pasos la cabeza no puede volverse sobre la dirección con la que avanzará realmente y no se genera un GAME OVER espurio por una reversa falsa del último MOVE. |
| `Result step(uint8_t fx, uint8_t fy)` | **Avanza un paso** (aplica el giro pendiente si lo hay). Coordenadas `(fx, fy)` del alimento (las pasa `Game` desde `_food`). Devuelve `Result`; `Game` interpreta: `ATE` → crece (+1 segmento, la cola NO avanza ese paso porque come), `DIED` → no se mueve (colisión). |
| `bool occupied(uint8_t x, uint8_t y)` | `true` si la celda `(x,y)` está ocupada por algún segmento. Lo consulta `Food` (al colocar el alimento) a través de `Game::occupied`, que delega aquí. |
| `uint8_t length()` | Longitud actual (cantidad de segmentos vivos). |
| `const Seg& segment(uint8_t i)` | Segmento `i` (0 = cola, `length()-1` = cabeza) para que `Game` lo dibuje. |
| `Sprite::Part headPart(bool hasFood, uint8_t fx, uint8_t fy)` | Sprite de la cabeza según la dirección commitida: `HEAD_<dir>_OPEN` **una casilla antes** de llegar al alimento (la comida está en la próxima celda según `_dir`) y `HEAD_<dir>_CLOSE` al colisionar con él (o al no haber comida). Lo consulta `Game` cada frame. |

### Internos (segmentos y buffer)

```cpp
enum class Dir : uint8_t { NONE = 0, UP = 1, RIGHT, DOWN, LEFT };
enum class Result : uint8_t { MOVED, ATE, DIED };
struct Seg {
    uint8_t x, y;
    Dir dir;            // hacia el segmento siguiente (más cerca de la cabeza)
    Sprite::Part part;  // sprite persistente
};
Seg _body[MAX_LENGTH];
uint8_t _headIx, _tailIx;   // índices circular (cabeza/cola)
uint8_t _length;
Dir _dir, _nextDir;         // commitida + giro pendiente
// montaje del sprite del cuerpo (boca arriba/abajo/izquierda/derecha), panza y cola
Sprite::Part bodyPartFor(Dir in, Dir out);
Sprite::Part bellyPartFor(Dir in, Dir out);
```

- **Ring buffer:** cola en `_tailIx`, cabeza en `_headIx`. Cada paso **agrega una
  parte nueva** (la cabeza) y **elimina la última** (la cola) salvo al comer (la
  cola NO avanza ese paso). El nuevo segmento se escribe en `slot()` = `_headIx+1`
  módulo `MAX_LENGTH`, y la cabeza nueva pasa a apuntar a ese slot.
- **El cuerpo NO se mueve:** la casilla que la cabeza deja se convierte en cuerpo
  con su `part` persistente (`bodyPartFor`/`bellyPartFor`), según por qué lado
  entra y sale la tubería: recta `BODY_TO_<dir>` (`in == out`) o esquina
  `CORNER_<horizontal>_<vertical>`/`BELLY_RIGHT_UP`.. al girar (`in != out`).
  La panza recta comparte sprite por par de direcciones: `BELLY_TO_RIGHT` =
  `BELLY_TO_UP` y `BELLY_TO_LEFT` = `BELLY_TO_DOWN`.
- **Comer (`bodyPartFor` en modo `EAT`):** con `in == out` se toma la panza recta
  de la dirección combinada `in`, y con giro (comer y girar a la vez) la panza
  curva `BELLY_<horizontal>_<vertical>`.
- **Colisión (`step`):** la celda destino es ilegal si coincide con el cuerpo
  **salvo la celda de la cola cuando NO come** (la cola se libera ese paso, como
  en el Nokia original; la cola es bloqueante solo cuando come). El giro pendiente
  se evalúa con una **copia local** (`dir`) y `_dir` (la COMMITIDA) solo se
  actualiza si el destino resulta legal, por lo que al morir la cabeza conserva la
  orientación real de su último movimiento (su sprite se dibuja según `_dir`).
- **Wrap:** `x = (x + 1) % COLS` etc.: sale por un borde, aparece por el opuesto.

### Validación en host

Por no depender de `Display`/`Buttons`/`Sound`/`Food`, `Snake` se puede compilar
y probar en el PC (MinGW) sin stubs: los casos del núcleo original (comer/crecer,
longitud estable sin comida, colisión real sin avanzar, cabeza sobre la celda de
la cola, wrap X/Y, cargo de integridad con giros) se conservan y pasan mirando a
esta clase directamente.

---

## 19. `Config.h` — constantes compartidas (namespace `Config`)

Ubicación: `Config.h`. **Un solo archivo pequeño para lo verdaderamente
compartido**: constantes que usan varias clases o que son propias del
hardware/placa. No es un cajón de sastre: lo específico de una clase se queda
como `static constexpr` dentro de ella (p. ej. `ANIM_TICK` en `Scroller`,
`NEW_BEST_SIGN_MS` en `Game`, `ARROW_BLINK_PERIOD` en `Menu`).

- **Sin `#define` para valores:** las constantes llevan tipo y viven en
  namespaces, de modo que no contaminan el ámbito global ni chocan con nombres
  de librerías (Adafruit / core ESP32). `#define` queda solo para lo que necesita
  el preprocesador (`#ifdef DEBUG`). Header: `#pragma once` + `#include <Arduino.h>`.
- **`constexpr` no ocupa RAM** y compila sin problemas en Arduino IDE con el core
  de ESP32 (C++17).

```cpp
namespace Config {

namespace Pin {
  constexpr int8_t BUTTONS[8] = { 2, 1, 42, 41, 38, 40, 39, 47 }; // orden Buttons::Button
  constexpr uint8_t BUZZER   = 14;  // zumbador
  constexpr uint8_t OLED_SDA = 8;   // I2C: datos
  constexpr uint8_t OLED_SCL = 9;   // I2C: reloj
}

namespace Screen {
  constexpr uint8_t WIDTH = 128, HEIGHT = 64, CELL = 8, ADDRESS = 0x3C;
  constexpr uint8_t HEADER_TOP = 0, HEADER_H = 16;  // Header 0..15
  constexpr uint8_t BODY_TOP = 16, BODY_H = 48;     // Body 16..63
  constexpr uint8_t FOOT_LINE = HEIGHT - (8 + 2 + 1);   // línea separadora del pie (53)
  constexpr uint8_t FOOT_TOP = HEIGHT - 8, FOOT_H = 8;  // texto del pie (56..63)
}

namespace Difficulty {
  constexpr uint8_t MIN_LEVEL     = 1;
  constexpr uint8_t MAX_LEVEL     = 10;
  constexpr uint8_t DEFAULT_LEVEL = 5;
}

namespace Version {
  constexpr char* VERSION      = "v1.0.0";
  constexpr char* RELEASE_DATE = "2026/12/31";
}
}
```

Estilo del archivo: los namespaces internos (`Pin`, `Screen`, `Difficulty`,
`Version`) van en columna 0, sin indentar respecto de `namespace Config {`.

### Qué contiene y quién lo usa

| Namespace | Constantes | Las usan |
|-----------|------------|----------|
| `Config::Pin` | `BUTTONS`, `BUZZER`, `OLED_SDA`, `OLED_SCL` | `Snake_II.ino` (pasa `Config::Pin::BUTTONS` a `Buttons` y `Config::Pin::BUZZER` a `Sound`, que construye su `Buzzer`), `Buzzer` (pin por defecto), `Display` (pines I2C por defecto) |
| `Config::Screen` | `WIDTH`/`HEIGHT`/`CELL`/`ADDRESS` y regiones `HEADER_*`/`BODY_*`/`FOOT_*` | `Display` (defaults del constructor y `regionBounds`), `Boot` (bandas TITULO=Header/CUERPO=Body), `Game` (tablero en el Body, celda de los sprites y `BODY_TOP` al volcar el tablero), `Food` (celda del alimento: centro del rombo y sprite especial), `MenuCredits` (rol del Body) |
| `Config::Difficulty` | `MIN_LEVEL`/`MAX_LEVEL`/`DEFAULT_LEVEL` | `MenuDifficulty` (selector de dificultad) y `Game` (`setDifficulty`/velocidad): antes duplicadas en ambas clases. El sufijo `_LEVEL` y `DEFAULT_LEVEL` evitan la macro `DEFAULT` del core ESP32 (`Arduino.h`). |
| `Config::Version` | `VERSION`/`RELEASE_DATE` | `Menu` (valor por defecto del parámetro `version` de su constructor, que es el texto del pie; antes el literal `"v0.1"` estaba en la firma). `RELEASE_DATE` todavía no lo usa nadie: queda para la pantalla de créditos o el pie. |

Las regiones `HEADER_TOP/H` y `BODY_TOP/H` reemplazan las constantes repetidas
`BODY_TOP`/`BODY_H`/`TITLE_TOP`/`TITLE_H` de `Menu`, `Game`, `Boot` y `MenuCredits`.
Los `static constexpr` de esas clases se eliminaron; solo `Boot` conserva sus
constantes propias (`BAR_W`, `BAR_SPACING`, `ANIM_TICK`, `TOTAL_MS`), que son de
su animación.

La banda del pie tiene su geometría en `Config::Screen`: `FOOT_LINE` = 53 para la
línea separadora, y `FOOT_TOP` = 56 con `FOOT_H` = 8 para el texto, todo derivado
de `HEIGHT` igual que `HEADER_TOP/H` y `BODY_TOP/H`. Antes eran `FOOT_TOP` =
`HEIGHT - 9` y `FOOT_H` = 7. En el estado actual del repo **todavía no las usa
ninguna ventana**: el pie de `Menu` sigue pintado sobre su propia banda (54..63) y
el `Legend` que ya las consume (línea y texto con `drawText`) sigue sin commitear.

`CELL` sigue la misma regla: es la **única** medida de px por celda. Reemplaza al
`static constexpr uint8_t CELL = 8` que `Food` declaraba por su cuenta y a los `8`
literales de `Game::drawSprite`. Lo que **no** es compartido (el factor de escala
con que un sprite de 4×4 llena la celda) queda como constante propia con nombre en
la clase que la usa: `Game::SPRITE_SCALE = 2` (`Sprite::SIZE * SPRITE_SCALE ==
Config::Screen::CELL`).

---

## 20. `Globals.h` — servicios globales

Ubicación: `Globals.h`. Declara con `extern` los **servicios de hardware** que
comparten todas las clases nuestras. Desde este refactor las clases ya no reciben
los servicios por constructor (ver secciones 3, 6 y 11): el único lugar donde se
**instancian** es `Snake_II.ino`.

```cpp
// Globals.h
#pragma once

#include "Display.h"
#include "Buttons.h"
#include "Sound.h"

extern Display display;
extern Buttons buttons;
extern Sound   sound;
```

### Reglas

1. **Se instancian solo en `Snake_II.ino`**, y cada uno se inicializa en su
   `begin()` desde `setup()` (por eso el **orden de definición ya no importa**:
   el `Engine`, declarado al final, se construye sobre globales que solo
   necesitan su constructor —`Scroller` (`display.getWidth()`) y `Food` son
   seguros— y el trabajo real va en los `begin()`). **No** se usa
   `static order/fiasco` ni factories.
2. **Solo las clases de servicio son globales.** Las **ventanas** (`Boot`,
   `Legend`, `Menu`, `MenuCredits`, `Game`) NO: son miembros del `Engine` (sección
   11) y por eso no aparecen aquí. El **`Buzzer` tampoco**: es una pieza de
   hardware interna de `Sound` (miembro por valor), no un servicio global.
3. **`Globals.h` se incluye solo desde los `.cpp`** (las cabeceras no lo
   incluyen): evita acoplar los `.h` al global y mantiene el orden de includes
   predecible. Un `.cpp` que usa un servicio global debe incluir `Globals.h`.
   `Sound.h` sí incluye `Buzzer.h`, porque su miembro es un `Buzzer` por valor.

---

## 21. `Timer.h` — reloj de 64 bits y cronómetros

Ubicación: `Timer.h` / `Timer.cpp`. Antes `Timer.h` era header-only; ahora las
definiciones están en el `.cpp` (el patrón `.h`/`.cpp` del resto del proyecto).
Motivo: `Stopwatch` y `Ticker` los usan casi todas las ventanas y, como métodos
`inline` dentro de la clase, el compilador repetía el mismo código en cada
unidad de traducción. **Nada se declara `inline` en el header**: una función
`inline` definida en otro `.cpp` no genera símbolo y daría *undefined
reference*.

Ante los límites de `millis()`
(32 bits: da la vuelta cada ~49,7 días, y `elapsed % period` sobre un instante
absoluto salta de fase una vez por giro), se centraliza el tiempo en el **reloj de
64 bits del ESP32**: `esp_timer_get_time()` (microsegundos desde el arranque, no
envuelve en ~292.000 años). Con 64 bits las restas (`ahora - inicio`) y los
módulos son seguros sin pensar en el desbordamiento.

**`esp_timer.h` vive solo en el `.cpp`** (único consumidor: `nowMs()`).
`<Arduino.h>` **se mantiene en el header** a propósito: además de los tipos
enteros, otras unidades de traducción lo heredan de ahí y no lo incluyen
(`Buttons.cpp` usa `pinMode`/`digitalRead`, `Game.cpp` usa `Serial`). Quitarlo
exigiría añadirlo explícitamente en esos `.cpp`: es deuda conocida, no un descuido.

### API

```cpp
uint64_t nowMs();        // instante actual en milisegundos (64 bits)

class Stopwatch {
  void start();                        // reinicia (llamar en el begin() de la ventana)
  uint64_t elapsed() const;            // ms desde start()
  bool expired(uint32_t ms) const;     // ¿ya pasaron `ms`?
  bool blinkOn(period, offPct) const;  // ¿fase visible? (oculto el primer offPct%)
};

class Ticker {
  explicit Ticker(uint32_t period);    // pasos de `period` ms
  void start();                        // reinicia el acumulador
  uint32_t consume();                  // pasos completos desde la última consulta
};                                     // (conserva el residuo)
```

### Reglas de uso

1. **Medir con restas, nunca contra un instante absoluto:** `nowMs() - inicio`,
   que con enteros sin signo de 64 bits da bien aunque el reloj se mueva.
2. **`Stopwatch` para plazos** (una fecha en la que algo vence): `TOTAL_MS` en
   `Boot`, `COUNTDOWN_MS`/`_moveDelay`/`NEW_BEST_SIGN_MS` en `Game`,
   `BLINK_HOLD` en `Menu`, `DWELL_MS`/`HOLD_MS` en `Legend` y `_durationMs` en
   `Buzzer`. `start()` se llama en el `begin()` de la ventana o al fijar la zona
   que parpadea: así el tiempo inactivo no entra.
3. **`Ticker` para pasos periódicos:** `ANIM_TICK` en `Boot` y `Scroller`
   (`consume()` devuelve los pasos de una vez; el `while` del acumulador
   desapareció). 
4. **`blinkOn(period, offPct)`** reemplaza al `millis() % período` absoluto de
   los parpadeos (`Menu`: rombo; `MenuDifficulty`/`MenuSound`: flechas de sus
   selectores; `Legend`: parpadeo del rombo activo). El `Stopwatch` se ancla al
   entrar en la ventana (`begin()`), así la fase del
   parpadeo no salta nunca.

### Quién lo usa

| Clase | Reloj/cronómetro | Cambio |
|-------|------------------|--------|
| `Boot` | `Ticker _ticker` (`ANIM_TICK`) + `Stopwatch _total` (`TOTAL_MS`) | El avance de 1 px y el plazo total pasan de `millis()` a 64 bits; el acumulador `while` se reduce a `_step = (_step + _ticker.consume()) % BAR_SPACING`. |
| `Menu` | `Stopwatch _hold` (rombo) | El parpadeo con `millis() % período` absoluto pasa a `blinkOn` anclado. |
| `MenuDifficulty` | `Stopwatch _blink` (flechas), `Stopwatch _repeat`/`_repeatTick` (repetición por mantensión) | Extraído de `Menu` con el selector: `blinkOn` anclado y `holdRepeat` con `expired()` en vez de restas sobre `millis()`. |
| `MenuSound` | `Stopwatch _blink` (flecha) | Extraído de `Menu` con el selector: `blinkOn` anclado. |
| `Legend` | `Stopwatch _timer` (`DWELL_MS`/`HOLD_MS`/`BLINK_PERIOD`) | El ciclo y el parpadeo del rombo activo usan `blinkOn` anclado al cambio de rombo. |
| `Scroller` | `Ticker _ticker` (`ANIM_TICK`) | El acumulador `_colAcc`/`_animLast` pasa a `consume()` (misma cadencia, sin `while`). |
| `Game` | `nowMs()` en `_moveLast`/`_startMs`/`_gameOverMs` (uint64_t) | Plazos y módulos del conteo/festejo sobre el reloj de 64 bits (mismo comportamiento; sin techo de 49,7 días). |
| `Buzzer` | `_startMs` (uint64_t) + `nowMs()` | El fin de la duración se compara con resta de 64 bits. |
| `Buttons` | `_buttonLast` (uint64_t) + `nowMs()` | El debounce se mide sobre el reloj de 64 bits. |

`Sound` no tiene reloj propio: delega las duraciones en `Buzzer`. `Snake` sigue
siendo lógica pura (solo `Arduino.h`: `delay`/`random`) y no usa el reloj.


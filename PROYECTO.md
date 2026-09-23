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

### Frase para iniciar una nueva sesión

Para no empezar desde cero, **copiar y pegar tal cual** el siguiente bloque en la
nueva sesión (no hay que escribir nada más):

```
Trabaja en el proyecto Snake II (ESP32-S3, Arduino IDE) ubicado en
D:\Documents\ESP32S3\Snake_II.
```

---

## 1. Estado actual del proyecto

En desarrollo por partes. La clase `Display` está completa. `Snake_II.ino` es el
**enlace de dependencias (wiring)**: construye TODAS las clases y las inicia en
`setup()`.

Arquitectura:
- **Ventanas hermanas (no anidadas):** `Boot`, `Legend`, `Menu`, `Credits` y
  `Game` son clases independientes, instancias únicas creadas
  en `Snake_II.ino` (como `Display`, `Buttons`, `Buzzer` y `Sound`) y compartidas
  por referencia. Sus valores persisten entre transiciones.
- **`Engine` = despachador puro:** recibe las ventanas por referencia y NO las
  anida. Su estado interno decide qué ventana se ve; al cambiar de estado llama al
  `begin()` de la ventana entrante. `setup()` llama `display.begin()`,
  `buttons.begin()` y `engine.begin()`; `loop()` hace la **única lectura de botones
  del frame** (`buttons.read()`, antes de `engine.update()`) y luego llama
  `engine.update()`, `engine.print()`, `sound.update()` y `display.show()`.
- **Renderizado sin `clear()` global:** `Engine::print()` ya **no** limpia la
  pantalla. Cada ventana hace `display.clear()` **solo en su primer frame** tras
  su `begin()` y luego no vuelve a borrar lo estático: dibuja su fondo una sola
  vez y por frame solo borra/redibuja sus zonas dinámicas (ver sección 13).
- Al iniciar se muestra la animación de arranque (`Boot`, franjas verticales),
  luego el panel de botones (`Legend`, pad MOVE con flechas + 4 rombos de ACTION
  que parpadean uno a la vez) y
  después el menú inicial con las opciones `New` y `Continue` (llevan a la ventana
  `Juego`), `Sound` y `Dificultad` (**se editan inline en el propio `Menu`**:
  al confirmar con `ACTION_RIGHT` aparece un selector en la banda de los rombos —
  On/Off para `Sound`, nivel 1..25 para `Dificultad`—, se navega con
  `MOVE_LEFT`/`MOVE_RIGHT` y se aplica
  con `ACTION_RIGHT`; `ACTION_UP` cancela) y los créditos. La `Legend` solo se muestra al arranque;
  al volver al menú desde cualquier ventana se pasa directo a `Menu` (ya no se
  repite la leyenda). El código del juego original no se mantiene como archivos de
  respaldo en el repo: queda en el historial de Git (`git show <commit>:Snake_II.ino`,
  `git show <commit>:GameBuzzer.h`).

La clase `Engine` (despachador de ventanas, antes `App`) está separada del `.ino`
en `Engine.h` / `Engine.cpp`, y recibe las ventanas **sin anidarlas**.

También se incorporó `SnakeSprites.h` (adaptada al estilo del proyecto, sección
15): la tabla de sprites de la serpiente del juego original, lista para que la
lógica del juego la consuma cuando exista.

Fases pendientes: la lógica de la serpiente ya está integrada en la ventana
`Juego` (estados `NUEVO`/`CONTINUAR` del `Engine`): movimiento con wrap,
sprites del contenido, comida, colisiones, dificultad (velocidad), pausa y
game over. Queda como mejora opcional conservar el récord (`bestScore`) entre
reinicios de la placa (p. ej. con EEPROM); hoy el récord vive solo en la sesión.

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
| `Buttons.h` / `Buttons.cpp` | Clase `Buttons` (lectura con debounce, `pressed`/`released`). Completa. |
| `Boot.h` / `Boot.cpp` | Clase `Boot` (animación de arranque: dos bandas completas —TITULO 0..15, CUERPO 16..63— de líneas verticales de 3 px que se desplazan en sentidos opuestos, con rebalse por el borde; dura `TOTAL_MS` y se termina con cualquier botón). Completa. |
| `Legend.h` / `Legend.cpp` | Clase `Legend` (panel de botones: pad MOVE a la izquierda con 4 flechas, 4 rombos completos de ACTION a la derecha en las posiciones de un pad que parpadean MUY rápido uno a la vez en ciclo lento —rombo fijo `HOLD_MS`, parpadeo `BLINK_PERIOD=100 ms`— y texto centrado en el pie con la función del rombo activo: Back/Pause, Select, None, None; cualquier botón la cierra con un sonido según el botón pulsado: MOVE = CLICK, ACTION_UP = BACK, ACTION_RIGHT = CONFIRM). Completa. |
| `Scroller.h` / `Scroller.cpp` | Clase `Scroller` (scroller de 1 bit compartido: compone una tira 128x16 y desliza lateralmente N bandas sincronizadas con el mismo desplazamiento; cada banda tiene su canvas persistente y sus colores de frente/fondo; usada por `Menu` con 1 banda y por `Credits` con 2). Completa. |
| `Menu.h` / `Menu.cpp` | Clase `Menu` (menú con scroller de 1 bit —1 banda del `Scroller` compartido— y rombos de posición). Completa. **Incluye la edición inline de la opción "Sound"** (selector On/Off en la banda de los rombos) **y de la opción "Dificultad"** (selector `< N >`, nivel 1..25, con repetición al mantener presionado; al mantener, solo queda fija la flecha del botón activo). |
| `Credits.h` / `Credits.cpp` | Clase `Credits` (ventana de créditos con 3 entradas navegables con transición lateral —2 bandas sincronizadas del `Scroller` compartido— y `SFX_CLICK` al navegar, vuelve al menú con `ACTION_UP`). Completa. |
| `Game.h` / `Game.cpp` | Clase `Game` (ventana del juego de la serpiente: estados NUEVO/CONTINUAR del `Engine`). Completa. |
| `Engine.h` / `Engine.cpp` | Clase `Engine` (despachador de ventanas, antes `App`). **No anida las ventanas**: las recibe por referencia y su estado interno decide qué ventana corre y cuándo cambiar (`changeState()`, que llama al `begin()` de la ventana entrante). Todos los `begin()` se lanzan desde `setup()`. Completa. |
| `Snake_II.ino` | Enlace de dependencias (wiring). Construye TODAS las clases: `Display`, `Buttons` y las ventanas hermanas `Boot`/`Legend`/`Menu`/`Credits`/`Game` (compartidas por referencia, valores conservados). Crea `Engine` con esas referencias; `setup()` llama `display.begin()`, `buttons.begin()` y `engine.begin()`; `loop()` hace la **única lectura de botones del frame** (`buttons.read()`) y llama `engine.update()`, `engine.print()`, `sound.update()` y `display.show()`. |
| `Buzzer.h` / `Buzzer.cpp` | Clase `Buzzer` (capa de hardware de sonido: un tono no bloqueante vía LEDC). Completa. |
| `Sound.h` / `Sound.cpp` | Clase `Sound` (secuencias de los efectos del juego sobre `Buzzer`, con `setEnabled` para silenciar). Completa. |
| `SnakeSprites.h` | Clase `SnakeSprites` (tabla de sprites de la serpiente, estilo Nokia: cola, cuerpo, curvas, cabeza cerrada/abierta y panza; sprites de 4×4 px). Solo datos (header-only, sin `.cpp`). Adaptada al estilo del proyecto. |
| `PROYECTO.md` | Este documento. |

Nota: Arduino solo compila el `.ino` del sketch. El respaldo quedó como `.txt`
para que no interfiera en la compilación.

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
Display(uint8_t sda = 8, uint8_t scl = 9, uint8_t address = 0x3C,
        uint8_t width = 128, uint8_t height = 64, uint8_t cellSize = 8);
```

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
| `void begin()` | `Wire.begin(sda, scl)`, crea el OLED y lo limpia. |
| `void clear()` | Limpia el buffer de la pantalla. |
| `void show()` | Envía el buffer al OLED. |
| `void drawPixel(x, y, black=false)` | Dibuja 1 píxel (blanco o negro). |
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

---

## 7. Clase `Buttons` — API

Ubicación: `Buttons.h` / `Buttons.cpp`. Basada en el diseño de `GameInput` (referencia
`D:\Documents\ESP32S3\Snake_2\GameInput.{h,cpp}`).

### Pines (orden del enum)

```cpp
const int8_t BUTTON_PINS[Buttons::MAX_BUTTONS] = {
  02, 01, 42, 41,  // MOVE_UP, MOVE_RIGHT, MOVE_DOWN, MOVE_LEFT
  38, 40, 39, 47   // ACTION_UP, ACTION_RIGHT, ACTION_DOWN, ACTION_LEFT
};
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
| `bool state(index)` / `pressed(index)` / `released(index)` | Acceso por índice (0-7) útil para ciclos genéricos. |
| `moveUp() / moveRight() / moveDown() / moveLeft()` | Estado actual (mantenido). |
| `actionUp() / actionRight() / actionDown() / actionLeft()` | Estado actual (mantenido). |
| `...,Pressed()` | Evento de pulso (true solo en el ciclo en que se presiona). |
| `...,Released()` | Evento de liberación (true solo en el ciclo en que se suelta). |

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
Menu(Display& display, Buttons& buttons, Sound& sound, uint8_t bestScore = 0, const char* version = "v0.1");
```

### Métodos

| Método | Descripción |
|--------|-------------|
| `void begin()` | Restablece el estado de la animación, del parpadeo y del modo de edición de sonido. **No resetea la selección**: conserva la opción elegida antes de salir del menú (las ventanas son hermanas persistentes; al volver al menú se muestra la misma opción que se tenía, no siempre "New"). |
| `void setOptions(textos, conteo)` | Fija la lista y la cantidad de opciones (1..`MAX_OPTIONS`=8). El menú (textos y rombos) se adapta al conteo. |
| `void update()` | Lee botones, navega con `MOVE_RIGHT`/`MOVE_LEFT` y anima el deslizamiento lateral; log en Serial al cambiar de opción; toca `SFX_CLICK` al navegar. En las opciones "Sound" y "Dificultad" gestiona el **modo de edición inline** (ver abajo). |
| `void print()` | Dibuja título, cuadro fijo con la opción deslizante, rombos de posición y pie (Best + versión). En modo de edición de sonido dibuja el **selector On/Off** en la banda de los rombos; en modo de edición de dificultad, el **selector `< N >`** (nivel 1..25). |
| `int8_t selected()` | Índice de la opción seleccionada. |
| `int8_t confirm()` | Devuelve la opción seleccionada si se confirma con `ACTION_RIGHT` (pulse recién presionado), o `-1`. **`OPC_SONIDO` y `OPC_DIFICULTAD` nunca se devuelven**: esas opciones se editan inline (ver abajo). Es el "activar opción" del resto del menú. |
| `void setBestScore(uint8_t)` | Actualiza el puntaje máximo mostrado. |
| `void setTitle(const char*)` | Cambia el título del Header. |
| `void setShowFooter(bool)` | Ocultar/mostrar el texto del pie ("Best"/versión); la línea de la `54` se dibuja siempre. |
| `void setSelected(int8_t)` | Fija la selección (clamp al rango) y reinicia la animación (al entrar en la ventana). |
| `void beginSoundEdit()` | Activa el modo de edición de sonido inline (borra los rombos y dibuja el selector On/Off). |
| `bool isEditingSound()` | `true` mientras el menú está en el modo de edición de sonido. |
| `void beginDifficultyEdit()` | Activa el modo de edición de dificultad inline (borra los rombos y dibuja el selector `< N >`). |
| `void endDifficultyEdit()` | Sale del modo de edición de dificultad (el menú se repinta: vuelven los rombos). |
| `bool isEditingDifficulty()` | `true` mientras el menú está en el modo de edición de dificultad. |
| `uint8_t difficulty()` | Nivel de dificultad persistente (1..25, default 13). |

### Modo de edición de sonido (inline, en el propio `Menu`)

Al confirmar la opción **"Sound"** con `ACTION_RIGHT` (btn2) **ya no se abre una
ventana separada** (la clase `SoundWindow` fue eliminada): el menú entra en modo
de edición inline.

- En la **banda de los rombos (45..53)** se dibuja el selector con
  **mayúsculas**: texto **"OFF"** o **"ON "** en `TEXT_6x8` centrado (con el
  espacio final de "ON " ambos estados miden lo mismo, 18 px, y el centrado no
  se desplaza) y **una sola flecha** (triángulo `fillTriangle`), **pegada al
  texto** (hueco `ARROW_GAP = 6` px), que **parpadea** (visible 75% / oculto
  25% de un período de `ARROW_BLINK_PERIOD = 500` ms); la palabra no parpadea.
  La flecha marca el **lado del destino** (la tecla que cambia el estado):
  `"OFF >"` cuando el sonido está OFF (presionar `MOVE_RIGHT` enciende) y
  `"< ON"` cuando está ON (presionar `MOVE_LEFT` apaga). El selector se
  redibuja **cada frame** (banda móvil): al entrar, al cambiar el valor o por
  el parpadeo de la flecha.
- `MOVE_LEFT`/`MOVE_RIGHT`: cambia el valor mostrado ON/OFF **sin aplicarlo**
  (toca `SFX_CLICK`). Solo cambia la tecla indicada por la flecha (`MOVE_LEFT`
  apaga si está ON, `MOVE_RIGHT` enciende si está OFF); la otra no hace nada.
- `ACTION_RIGHT` (btn2): **aplica** el valor (`Sound::setEnabled`) y vuelve al
  menú (`SFX_CONFIRM` al encender). El texto del selector desaparece.
- `ACTION_UP` (btn1): **cancela** sin cambiar el estado (toca `SFX_BACK`) y vuelve
  al menú. El texto del selector desaparece.
- `Sound` se guarda como miembro `_soundEnabled` (no es una vista previa global);
  el selector se redibuja en cada `print()` (la flecha parpadea; el texto, no).
  `OPC_SONIDO` no se entrega
  a `Engine::confirm()` (devuelve `-1`), por lo que el `Engine` permanece en `MENU`.

### Modo de edición de dificultad (inline, en el propio `Menu`)

Al confirmar la opción **"Dificultad"** con `ACTION_RIGHT` (btn2) el menú entra en
modo de edición inline, igual que "Sound" (no se abre ninguna ventana).

- En la **banda de los rombos (45..53)** se dibuja el selector con el **nivel
  1..25** en `TEXT_6x8` centrado con **ancho constante** (1 dígito se alinea a la
  derecha con un espacio inicial: `" 5"` mide lo mismo que `"13"`, 12 px, y el
  centrado no se desplaza) y **dos flechas** (`fillTriangle`) que **parpadean
  juntas** a los lados del texto (`"< 13 >"`), visible 75% / oculto 25% de
  `ARROW_BLINK_PERIOD = 500` ms; el número no parpadea. En el **límite** la
  flecha de ese lado se oculta: en `1` no hay flecha izquierda (-1 no existe) y
  en `25` no hay derecha (+1 no existe). **Al mantener presionado**
  `MOVE_LEFT`/`MOVE_RIGHT` (paso continuo) el parpadeo se **detiene**: solo la
  flecha del botón activo queda **fija** y la contraria se oculta (señal visual
  de la repetición). **Al llegar al límite (1 o 25)** el botón de ese lado ya no
  puede avanzar y se procesa **igual que haber soltado el botón**: vuelve el
  parpadeo normal, con la flecha del límite oculta.
- `MOVE_RIGHT` = **+1**, `MOVE_LEFT` = **-1** (clamp entre `DIFICULTAD_MIN = 1`
  y `DIFICULTAD_MAX = 25`, cada paso toca `SFX_CLICK`). **Repetición al
  mantener presionado:** el primer paso es inmediato (`pressed`) y, manteniendo
  el botón, tras `HOLD_REPEAT_DELAY = 400` ms se repite +1/-1 cada
  `HOLD_REPEAT_TICK = 100` ms (helper `holdRepeat`). El valor mostrado cambia
  **sin aplicarlo**; solo se aplica al confirmar.
- `ACTION_RIGHT` (btn2): **aplica** el valor (`_difficulty`, visible con
  `difficulty()`) y vuelve al menú (`SFX_CONFIRM`).
- `ACTION_UP` (btn1): **cancela** sin cambiar el valor guardado (`SFX_BACK`).
- Valor por defecto `DIFICULTAD_DEFAULT = 13`, conservado en el miembro
  persistente `_difficulty`; `beginDifficultyEdit()` copia a `_editDifficulty`
  (el valor en edición). `OPC_DIFICULTAD` no se entrega a `Engine::confirm()`
  (devuelve `-1`), por lo que el `Engine` permanece en `MENU`.

### Opciones y enum

```cpp
enum Option : uint8_t {
  OPC_NUEVO = 0, OPC_CONTINUAR, OPC_DIFICULTAD, OPC_SONIDO, OPC_CREDITOS
};
```

El enum documenta los índices de las 5 opciones por defecto. La cantidad real es
variable (`setOptions`), con `MAX_OPTIONS = 8`.

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
- **Animación (scroller de 1 bit):** en la clase compartida **`Scroller`** (ver
  sección 14, "Clase `Scroller`"), que `Menu` instancia con **1 banda** (rol único
  128×16, texto 12x16) y `Credits` con **2 bandas sincronizadas**. Cada opción se
  compone **antes** de mostrarse en una **matriz de 128×16 de 1 bit**
  (`_strip[16][16]`, `1` = glifo, `0` = fondo) **centrada**, mediante el canvas
  auxiliar `_composer` (128×16) que dibuja el texto (`compose`). La **banda del
  cuadro** es un **canvas persistente** (`_chipBox[band]`, 128×alto): conserva lo
  que está en pantalla entre frames, así la opción anterior **se mantiene hasta
  ser borrada** por la nueva (`slideStrip` sobrescribe columna a columna la banda
  con la tira entrante, incluidos sus fondos; `blit` la vuelca a la pantalla con
  sus colores). Al navegar con `MOVE_RIGHT` la tira entra por la **derecha** (se
  mueve a la izquierda); con `MOVE_LEFT` por la **izquierda**. Arranca **fuera
  de pantalla** y avanza **una columna por cada `ANIM_TICK` ms** (`animate`,
  acumulado por tiempo; vuelo total ≈ `128 × 4 ms ≈ 0,5 s`). Si se navega a
  mitad de la animación, la banda conserva lo que había y la nueva tira se
  superpone (pueden verse varias opciones a la vez). Todas las opciones quedan
  con el **mismo ancho de 128 px** (espacios vacíos incluidos).
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
Boot(Display& display, Buttons& buttons);
```

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
  dibujadas con `fillRect` directo sobre el OLED.
- **Rebalse:** al llegar al borde derecho la línea se parte en dos tramos (`drawBar`:
  tramo derecho + tramo por la izquierda) para que no se corte en seco.
- **Movimiento:** `_shift` avanza 1 px cada `ANIM_TICK = 30 ms` (ciclo completo de
  8 px). En `TITULO` las líneas se mueven **de izquierda a derecha**; en `CUERPO`
  **de derecha a izquierda**.
- **Limpieza incremental (sin `clear()` por frame):** en el primer frame se hace
  un `clear()` completo; luego, al avanzar, se borran **solo las columnas que cada
  franja deja de ocupar** (las columnas que coinciden con la posición nueva se
  mantienen) y se dibujan las franjas en su nueva posición; el resto de la pantalla
  no se toca. Si el desplazamiento no cambió, no se dibuja nada.
- **Duración:** `TOTAL_MS = 4000 ms` o cualquier botón, lo que ocurra primero; luego
  `Engine` entra al menú.

---

## 9. Clase `Legend` — API

Ubicación: `Legend.h` / `Legend.cpp`.

### Constructor

```cpp
Legend(Display& display, Buttons& buttons, Sound& sound);
```

### Métodos

| Método | Descripción |
|--------|-------------|
| `void begin()` | Reinicia la ventana: apaga el flag de salida, rombo activo = `Btn1`. |
| `void update()` | Lee botones y avanza el ciclo. **El sonido depende del botón presionado** (`done() = true`): `MOVE_*` (navegación) = `SFX_CLICK`, `ACTION_UP` (Back/Pause) = `SFX_BACK`, `ACTION_RIGHT` (Select) = `SFX_CONFIRM`, `ACTION_DOWN`/`ACTION_LEFT` (None) = `SFX_CLICK`. El rombo activo cambia cada `DWELL_MS`. |
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
  `Btn1` (↑ = `ACTION_UP`): **"Back / Pause"**, `Btn2` (→ = `ACTION_RIGHT`):
  **"Select"**, `Btn3` (↓ = `ACTION_DOWN`): **"None"**, `Btn4` (← =
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

### Constructor

```cpp
Buzzer(uint8_t pin = 14);
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
de tonos** (`Note` = `{freq, durMs}`, `0` = silencio) sobre `Buzzer`. Todo es no
bloqueante: `play()` arranca el efecto y `update()` lo avanza paso a paso cuando
cada nota termina. Con el sonido desactivado `play()` no hace nada (pensado para la
opción "Sound" del menú, `setEnabled(false)`).

### Constructor

```cpp
Sound(Buzzer& buzzer);
```

### Enum y efectos

```cpp
enum Sfx : uint8_t {
  SFX_NONE = 0, SFX_CLICK, SFX_CONFIRM, SFX_BACK,
  SFX_EAT, SFX_START, SFX_LEVEL_UP, SFX_GAME_OVER
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

Los tonos siguen la paleta del `GameBuzzer` original.

### Métodos

| Método | Descripción |
|--------|-------------|
| `void begin()` | Silencia y reinicia la secuencia. |
| `void setEnabled(bool)` / `enabled()` | Activa/desactiva el sonido (opción "Sound"); al desactivar corta el efecto en curso. |
| `void play(Sfx)` | Arranca la secuencia del efecto (no bloqueante). |
| `void update()` | Avanza a la siguiente nota (llama `_buzzer.update()` antes). Llamar una vez por `loop()`. |
| `void stop()` | Corta el efecto en curso y silencia. |
| `bool playing()` | `true` mientras suena un efecto. |

En `Snake_II.ino`: `Buzzer buzzer;` y `Sound sound(buzzer);` (instancias únicas,
`buzzer.begin()` y `sound.begin()` en `setup()`, `sound.update()` en `loop()`).

---

## 10.3 Opción "Sound" (borrada la clase `SoundWindow`)

**Eliminada.** La clase `SoundWindow` (y sus archivos `SoundWindow.h`/`.cpp`)
fue eliminada del proyecto: la opción "Sound" ya no abre una ventana separada.
El On/Off se edita **inline en el propio `Menu`** (modo de edición de sonido,
ver sección 7): selector ON/OFF en la banda de los rombos con una flecha
parpadeante en el lado del destino,
`MOVE_LEFT`/`MOVE_RIGHT` cambian el valor, `ACTION_RIGHT` (btn2) lo aplica y
vuelve al menú, `ACTION_UP` (btn1) cancela y vuelve al menú. `Engine` ya no tiene
el estado `SONIDO` ni recibe `SoundWindow`.

---

## 11. Clase `Engine` — despachador de ventanas

Separada del `.ino` en `Engine.h` / `Engine.cpp` (antes `App`). **No anida las
ventanas**: `Boot`, `Legend`, `Menu`, `Credits`, `Game`
son clases independientes, instancias únicas creadas en `Snake_II.ino` y pasadas a
`Engine` por referencia, igual que `Display` y `Buttons`.

### Responsabilidad

`Engine` es el **despachador puro**: su **estado interno** (`enum class State`)
decide qué ventana corre y cuándo cambiar (`changeState()`, que llama al `begin()`
de la ventana entrante). `loop()` no participa en las transiciones: solo llama a
`engine.update()` y `engine.print()`. `setup()` inicia el hardware y llama a
`engine.begin()` (primera transición → `boot.begin()`).

### Constructor

```cpp
Engine(Display& display, Buttons& buttons, Boot& boot, Menu& menu,
       Credits& credits, Game& game, Legend& legend,
       Sound& sound);
```

### Reglas de esta arquitectura

1. **Todas las clases se inician en `setup()`** y se usan en `loop()`/`Engine`.
   Los constructores son livianos (solo guardan referencias); el trabajo real va
   en `begin()`/`update()`. Los `begin()` de las ventanas los llama `changeState()`
   al entrar (la primera se lanza dentro de `setup()` vía `engine.begin()`).
2. **No se anidan las partes del juego:** `Juego` NO va dentro de `Menu` ni de
   `Engine`. Cada ventana es una clase propia con el patrón
   `begin()/update()/print()/done()`; `Engine` solo las despacha.
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

### Estados internos

```cpp
enum class State : uint8_t {
  BOOT = 0, MENU, NUEVO, CONTINUAR, CREDITOS, LEGEND
};
```

| Estado | Ventana | Notas |
|--------|---------|-------|
| `BOOT` | `Boot` | Animación de arranque (franjas). Al terminar (`done()`) pasa a `LEGEND`. Cualquier botón la termina. |
| `LEGEND` | `Legend` | Panel de botones: pad MOVE con 4 flechas + 4 rombos completos de ACTION en las posiciones de un pad que parpadean MUY rápido uno a la vez (ciclo lento) con la función del rombo activo centrada en el pie. Cualquier botón la cierra → menú (suena el efecto según el botón —CLICK/BACK/CONFIRM—; `Engine` no añade `SFX_BACK`). Solo se muestra tras el arranque. |
| `MENU` | `Menu` | Confirma con `ACTION_RIGHT` (`confirm()`). |
| `NUEVO` | `Game` | Nueva partida: `setDifficulty(menu.difficulty())` + `begin(true)`. Arranca con la cuenta regresiva "GO !" (`SFX_START`). Al salir (`done()`) suena `SFX_BACK`, el `Engine` sincroniza el récord (`menu.setBestScore(game.bestScore())`) y pasa a `MENU`. |
| `CONTINUAR` | `Game` | Reanudar la partida anterior (`begin(false)`): queda en pausa y se retoma con `ACTION_RIGHT`/`ACTION_LEFT`; si no hay partida en curso arranca una nueva. Al salir (`done()`) igual que `NUEVO`. |
| `CREDITOS` | `Credits` | 3 entradas navegables con `MOVE_LEFT`/`MOVE_RIGHT` y transición lateral (rol tamaño 2 **seleccionado con cuadro de borde a borde** y centrado en el alto restante del Body; nombre tamaño 1 plano en el pie). La transición usa el **mismo `Scroller` compartido que el menú** pero con **2 bandas sincronizadas** (`BAND_HEIGHTS = {16, 8}` = altos de rol 12x16 y nombre 6x8): rol y nombre se componen por separado en la misma tira (`loadEntry` → `compose`) y deslizan a la vez con el **mismo `_slideX`** interno del `Scroller` (aparecen al mismo tiempo). `drawBand(slot, y, fg, bg)` compone el slot y vuelca su **banda persistente** (`_chipBox[slot]`) con sus colores; la tira la sobrescribe **columna a columna** con sus fondos, así la entrada anterior se mantiene hasta que la nueva la cubre (superposición al navegar rápido). El deslizamiento **arranca desde el borde** (`startSlide`, fuera de escena) y avanza **1 px cada 4 ms con acumulador por tiempo** (igual que el menú, ≈0,5 s). Al navegar suena `SFX_CLICK` y al salir (`done()`) suena `SFX_BACK` (lo toca el `Engine`) y pasa directo a `MENU`. |

### Métodos

| Método | Descripción |
|--------|-------------|
| `void begin()` | Primera transición: entra al test de píxeles (`changeState(State::BOOT)`). Se llama desde `setup()`. |
| `void update()` | Lee/actualiza la ventana activa y gestiona las transiciones de estado. Reproduce los efectos del sonido: `SFX_CONFIRM` al confirmar una opción del menú y `SFX_BACK` al volver a `MENU` desde cualquier ventana (excepto desde `Legend`, que toca su propio sonido según el botón). |
| `void print()` | Despacha el dibujo a la ventana activa. **Ya no limpia la
  pantalla (`display.clear()`)**: cada ventana la usa solo en su primer frame tras
  `begin()` y luego limpia/redibuja solo sus zonas dinámicas (sección 13). |
| `void setBestScore(uint8_t)` | Reenvía al menú para conservar el puntaje máximo entre sesiones. |

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
- **Warnings de compilación C++ activados** en la máquina de desarrollo vía
  `platform.local.txt` del core ESP32 (`compiler.cpp.extra_flags=-Wall -Wreorder`)
  para que desajustes como el orden de inicialización de miembros salten a la vista
  en la compilación (no entra en el repo: se configura a nivel del paquete del core).
- La demo actual (`Snake_II.ino`) usa `Display`, `Buttons` y el sonido integrado:
  al navegar el menú y los créditos suena `SFX_CLICK`, al confirmar `SFX_CONFIRM`,
  al volver al menú `SFX_BACK`, la `Legend` suena según el botón pulsado (MOVE =
  CLICK, ACTION_UP = BACK, ACTION_RIGHT = CONFIRM) y en la opción "Sound" el
  **On/Off se edita inline en el propio menú** (selector con flechas en la banda
  de los rombos; `MOVE_LEFT`/`MOVE_RIGHT` cambian el valor, `ACTION_RIGHT` lo
  aplica y `ACTION_UP` cancela). En la opción "Dificultad" el **nivel 1..25
  también se edita inline** (selector `< N >` con dos flechas parpadeantes que se
  ocultan en los límites; `MOVE_RIGHT` +1, `MOVE_LEFT` -1 con repetición al
  mantener presionado —al mantener, el parpadeo se detiene y solo queda fija la
  flecha del botón activo, la contraria se oculta; al llegar al límite se
  procesa igual que haber soltado el botón (vuelve el parpadeo normal)—,
  `ACTION_RIGHT` lo aplica y
  `ACTION_UP` cancela). En el juego (`Game`): `SFX_START` al iniciar (GO !),
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
   animación (`Menu::begin/setOptions/setSelected`, `setBestScore` si cambia)
   también activan ese flag.
2. **Estáticos una sola vez:** títulos, pies, línea separadora, cuadro de
   selección, rótulos y pads se dibujan en ese primer frame y **ya no se vuelven a
   dibujar**; persisten en el buffer (que `display.show()` vuelca completo cada
   frame).
3. **Dinámicos por frame, borrando solo lo necesario:**

   | Ventana | Estáticos (una vez) | Dinámicos por frame |
   |---------|---------------------|---------------------|
   | `Boot` | — (primer frame: clear completo) | Franjas: se borran **solo las columnas que cada franja deja de ocupar** (las coincidentes se mantienen) y se dibujan las nuevas; si no cambió el desplazamiento no se dibuja nada. |
   | `Menu` | Cuadro blanco (25..42), título, pie (línea 54 + texto) | Banda de la opción (26..41) con `Scroller::blit` + rombos (banda 45..53); en el modo de edición de sonido, en vez de rombos se borra/redibuja **cada frame** la misma banda 45..53 con el selector ON/OFF (palabra centrada estática + flecha única, lado del destino, que parpadea); en el modo de edición de dificultad, el selector `< N >` (número centrado estático con ancho constante + dos flechas laterales que parpadean juntas, ocultas en su límite; al mantener un botón el parpadeo se detiene y solo queda fija la flecha del botón activo, ocultándose la contraria; al llegar al límite se procesa igual que haber soltado el botón, volviendo el parpadeo normal) |
   | `Credits` | Título + cuadro blanco del rol | Bandas rol/nombre (`Scroller`, 2 bandas sincronizadas) |
   | `Legend` | Rótulos, pad MOVE y los 4 rombos fijos | Zona del rombo activo (cuadro 9x9, parpadeo) + texto del pie (banda 54..63) solo si cambia el rombo; al cambiar, se restaura completo el rombo que deja de ser activo (evita que quede borrado si el cambio lo pilló en su fase oculta) |
   | `Game` | Primer frame: clear completo + Header (puntaje 12x16 izq., segundos restantes de la comida especial 12x16 der.) y alimento y serpiente | Header solo si cambia el puntaje o `_specialTime` (banda 0..15); tablero (Body 16..63) solo si `_dirtyBoard` (movimiento, comida nueva, transición de estado): borra el Body, redibuja alimento + serpiente; overlay "GO !"/"PAUSA"/"GAME OVER" (cuadro blanco centrado + texto invertido) en cada frame según el estado |

4. Los modos de edición del `Menu` ("Sound" y "Dificultad") comparten la banda
   dinámica de los rombos (45..53): al entrar (`beginSoundEdit()`/
   `beginDifficultyEdit()`) se borra y se dibuja el selector (ON/OFF o `< N >`
   con el nivel 1..25) y, como las flechas parpadean, la banda se borra/redibuja
   en **cada frame**; al salir (`_redraw = true`) el menú se repinta completo
   (vuelven los rombos). El flag `_redrawSound` ya no existe.

---

## 14. Clase `Scroller` — API (scroller de 1 bit compartido)

Extraída de la lógica duplicada de `Menu` y `Credits` (tarea 4). Encapsula la
animación "scroller de 1 bit": compone un texto en una **tira de 128×16 de 1
bit** y la desliza lateralmente sobre **N bandas sincronizadas** (todas usan el
**mismo `_slideX`** → aparecen a la vez, mismo offset). Cada banda tiene su
**canvas persistente** (`GFXcanvas8`), su **alto** (`bandHeights[i]`, por defecto
`STRIP_H = 16`) y sus **colores** de frente/fondo aplicados al volcar.

- **Constantes:** `STRIP_W = 128` (ancho de pantalla), `STRIP_H = 16` (máx.
  altura de texto 12x16), `ANIM_TICK = 4` ms por píxel (vuelo ≈ 0,5 s),
  `CHIP_TEXT = 1` / `CHIP_BG = 255`.
- **Constructor:** `Scroller(Display& display, uint8_t bands = 1,
  const uint8_t* bandHeights = nullptr)`. Asigna en heap los arrays de altos y
  de canvas de banda (**NOTA:** no hay constructor por defecto de `GFXcanvas8`
  en Adafruit_GFX, y la copia implícita es peligrosa; por eso `_chipBox` es
  `GFXcanvas8**`). `~Scroller()` libera los canvas y los arrays. La lista de
  inicialización respeta el **orden de declaración** (regla de la tarea 3).
- **API:**
  - `begin()` — reposiciona el deslizamiento (objetivo 0, sin borrar bandas).
  - `compose(const char* text, uint8_t size)` — dibuja el texto centrado en la
    tira (canvas auxiliar `_composer` 128×16 → matriz `_strip[16][16]`).
  - `startSlide(int8_t dir)` — `+1` entra por la derecha, `-1` por la izquierda;
    arranca desde el borde (`_slideX = ±ancho`).
  - `animate()` — avanza 1 px por `ANIM_TICK` ms (acumulador por tiempo, llama a
    `slideStrip` según `_slideX`); a llamar en `update()`.
  - `blit(uint8_t band, int16_t y, uint16_t fgColor, uint16_t bgColor)` —
    sobrescribe la banda persistente con la tira entrante (columna a columna,
    fondos incluidos) y la vuelca a la pantalla en la fila `y`, con `fgColor`
    para los glifos y `bgColor` para el fondo. A llamar en `print()`.
- **Usos:** `Menu` = 1 banda (16 px, `blit(0, TEXT_SEL_TOP, NEGRO, BLANCO)`);
  `Credits` = 2 bandas (`BAND_HEIGHTS = {16, 8}`, rol 12x16 / nombre 6x8, cada
  `drawBand(slot, y, fg, bg)` compone y vuelca su banda).
- **Detalle de diseño:** `compose` usa `_display.getTextWidth()` (ya no se
  duplica la lógica de centrado); `blit` recibe el índice de banda y los
  colores (el sketch original pedía `blit(Display&, int16_t y, uint8_t h)` pero
  hacía falta la banda y los colores). `Boot` conserva su propia animación
  (`ANIM_TICK = 30`, bandas **completas** que avanzan solas por borde) — **no**
  usa `Scroller`.

---

## 15. Clase `SnakeSprites` — API (sprites de la serpiente)

Ubicación: `SnakeSprites.h`. Tabla estática con los sprites de las partes de la
serpiente (estilo Nokia). Es una clase de **solo datos**: no necesita instancia
ni archivo `.cpp`; los sprites se leen con `SnakeSprites::SPRITES[Part]`.

```cpp
static constexpr uint8_t SIZE = 4;          // sprite de 4×4 px

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
| `SPRITES[COUNT][SIZE][SIZE]` | Tabla de sprites de 1 bit (`1` = glifo, `0` = fondo) indexada por `Part`. Rango: 0..3 cola, 4..7 cuerpo, 8..11 curvas, 12..15 cabeza cerrada, 16..19 cabeza abierta, 20..25 panza, 26 `EMPTY`, 27 `COUNT`. |
| `EMPTY` | Sprite vacío (todo fondo). |
| `COUNT` | Cantidad de sprites de la tabla. |

La panza recta comparte sprite por par de direcciones: `BELLY_TO_RIGHT` =
`BELLY_TO_UP` y `BELLY_TO_LEFT` = `BELLY_TO_DOWN`.

`SnakeSprites.h` forma parte del respaldo del juego original adaptado al estilo
del proyecto (pie `// Fin`, cabecera descriptiva, comentarios de los sprites
corregidos). Los consume la ventana `Game` (ver sección 16): la parte de cada
segmento (cola/cuerpo/curva/cabeza) se deriva en cada frame de la geometría de
sus vecinos y de la dirección de la cabeza.

---

## 16. Clase `Game` — API (ventana del juego)

Ubicación: `Game.h` / `Game.cpp`. Ventana del juego de la serpiente
(estado `NUEVO`/`CONTINUAR` del `Engine`), reemplaza al placeholder `InfoWindow`
(eliminado). Siguió el diseño validado en un simulacro en host (MinGW) de la
lógica núcleo (selección de sprites, wrap, comida, colisiones) antes de escribirse.

### Constructor

```cpp
Game(Display& display, Buttons& buttons, Sound& sound);
```

### Constantes

| Constante | Valor | Significado |
|-----------|-------|-------------|
| `COLS`, `ROWS` | 16, 6 | Tablero: rejilla de 16×6 celdas de 8 px en el Body (128×48). |
| `MAX_LENGTH` | 96 | Cantidad máxima de segmentos (una celda por segmento). |
| `DIFICULTAD_MIN` / `MAX` / `DEFAULT` | 1 / 25 / 13 | Nivel de dificultad acotado (mismo rango que el menú). |
| `GO_MS` | 1200 | Duración de la cuenta regresiva inicial ("GO !"). |

### Métodos

| Método | Descripción |
|--------|-------------|
| `void begin(bool newGame)` | `true` = nueva partida (reinicia todo y arranca la cuenta regresiva). `false` = reanudar la partida anterior en pausa; si no hay partida en curso arranca una nueva. Conserva el récord (`_bestScore`) entre partidas. |
| `void setDifficulty(uint8_t level)` | Nivel 1..25 (clamp). Se aplica a la velocidad cuando ARRANCA una partida (no a las reanudadas). |
| `void update()` | Estado `START`: pre-gira con MOVE, entra a `PLAY` con `ACTION_RIGHT` o al agotarse `GO_MS`. `PLAY`: gira con MOVE (sin reversa directa) y avanza un paso cada `_moveDelay` ms. `PAUSE`: reanuda con `ACTION_RIGHT`/`ACTION_LEFT`. `GAME_OVER`: cualquier ACTION vuelve al menú. `ACTION_UP` (común "volver al menú") sale en cualquier estado menos `GAME_OVER`. |
| `void print()` | Renderizado por zonas (ver sección 13). |
| `bool done()` | `true` al pedir volver al menú. |
| `uint8_t score()` / `bestScore()` | Puntaje actual / récord. El `Engine` sincroniza `bestScore()` con el menú al salir. |

### Reglas del juego

- **Movimiento:** la cabeza avanza 1 celda por paso con **wrap en X y en Y**
  (sale por un borde, aparece por el opuesto, estilo Nokia). La velocidad
  (`_moveDelay` ms por paso) es lineal con la dificultad: `1000 - (nivel-1)*38`
  (nivel 1 → 1000 ms, nivel 25 → 88 ms).
- **No hay reversa directa:** girar hacia la dirección contraria se ignora
  (los botones MOVE son excluyentes entre sí por el anticonflicto de `Buttons`).
- **Serpiente:** buffer circular `Seg body[MAX_LENGTH]` (cola en `_tailIx`,
  cabeza en `_headIx`). Cada `Seg` guarda su **posición**, su **dirección** (`dir`,
  hacia el segmento siguiente, más cerca de la cabeza) y su **sprite persistente**
  (`part`). Inicial: células `(1,2)..(4,2)`, cabeza a la derecha.
- **El cuerpo NO se mueve:** cada paso se **agrega una parte nueva** (la cabeza)
  y se **elimina la última** (la cola). La casilla que la cabeza deja se convierte
  en cuerpo con su sprite persistente, según por qué lado entra y sale la tubería:
  recto `BODY_TO_<dir>` (`in == out`) o esquina `CORNER_<horizontal>_<vertical>`
  al girar (`in != out`). El nombre de la esquina indica los dos **lados** de la
  celda que conecta (el lado por el que entra la tubería, opuesto a la dirección
  de llegada `in`, y el lado por el que sale, `out`): p. ej. iba a la izquierda y
  sube, o bajaba y cruza a la derecha, conectan el lado derecho con el superior
  → `CORNER_RIGHT_UP`.
- **Comer:** al tocar el alimento (`SFX_EAT`): crece (+1 segmento, la cola NO
  avanza ese paso, puntaje +1). La cabeza queda **sobre la casilla del alimento**
  y, al dejarla en el siguiente paso, esa casilla se dibuja como **`BELLY`**
  (panza recta o curva según el giro) que queda guardada en el segmento y viaja
  con el cuerpo hasta que la cola lo borra. El alimento se regenera en una
  **celda libre al azar**. Si no hay celdas libres (tablero lleno) la partida
  **se gana** (termina). Dibujado como **rombo** simétrico centrado en la celda
  (dos `fillTriangle`), como el rombo del menú.
- **Colisión con el cuerpo:** al mover, la celda destino es ilegal si coincide
  con el cuerpo **salvo la celda de la cola cuando NO come** (la cola se libera
  ese paso, como en el Nokia original; la cola es bloqueante solo cuando come).
  Si colisiona: `GAME_OVER` (`SFX_GAME_OVER`), informa `SFX_BACK` al volver y
  `_hasGame = false` (un `Continue` posterior arranca de nuevo).
- **Pausa y salida:** `ACTION_UP` durante la partida vuelve al menú **sin
  perderla** (`_hasGame` mantiene el tablero; `Continue` la reanuda en pausa).
  `GAME_OVER` deja `_hasGame = false`.
- **Sprites (cuerpo persistente):** cada segmento del cuerpo guarda su
  `part` (sprite fijo): cola `TAIL_TO_<dir>` (su `dir` guardada), cuerpo recto
  `BODY_TO_<dir>`, curva `CORNER_<horizontal>_<vertical>` (índice 8..11
  calculado) y panza `BELLY` (recta `BELLY_TO_RIGHT`/`TO_LEFT` o curva
  `BELLY_RIGHT_UP`..). Solo la **cabeza** se calcula en cada frame
  (`headPart()`): `HEAD_<dir>_OPEN` **una casilla antes** de llegar al alimento
  (la comida está en la próxima celda según `_dir`) y `HEAD_<dir>_CLOSE`
  al colisionar con él. El orden del enum `Dir` (UP=1..LEFT=4) coincide con el
  orden de los sprites por dirección (`dir-1`). `drawSprite` dibuja cada píxel
  del sprite 4×4 como un bloque 2×2 (completa la celda de 8×8). La cabeza se
  dibuja al final (queda encima).
- **Header:** puntaje en `TEXT_12x16` (izq., NO se mueve) y segundos restantes
  de la **comida especial** en `TEXT_12x16` (der., variable `_specialTime`, por
  ahora valor fijo 60 solo para el layout). Se redibuja solo cuando cambian.
  Ya no muestra el récord `HI` (el "Best: N" queda solo en el menú).
- **Overlays:** "GO !" (cuenta regresiva), "PAUSA" y "GAME OVER" = cuadro blanco
  (`fillRoundRect`) centrado en el Body + texto invertido negro `TEXT_12x16`
  (`drawTextInverted`). Al volver a `PLAY` se marca `_dirtyBoard` (borra el
  overlay bajo el tablero).

### Validación en host (MinGW)

Antes de escribir el código se simuló en el PC la lógica núcleo (misma aritmética
de ring buffer, colisiones y selección de sprites): comer/crecer (cola se
mantiene, score, longitud), longitud estable sin comida (la cola avanza),
colisión real detectada (la cabeza no avanza), la cabeza **puede** ocupar la
celda de la cola (anillo casi cerrado), wrap horizontal y vertical, y cargo de
integridad de 400 pasos con giros (celdas únicas + adyacencia sin romper).
`Game.cpp` además se compiló en host con stubs de `Display`/`Buttons`/`Sound`/
`Adafruit_SSD1306` reproduciendo las firmas reales (0 errores).
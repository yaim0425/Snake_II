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
- **Ventanas hermanas (no anidadas):** `Boot`, `Legend`, `Menu`, `Credits` e
  `InfoWindow` son clases independientes, instancias únicas creadas en
  `Snake_II.ino` (como `Display` y `Buttons`) y compartidas por referencia. Sus
  valores persisten entre transiciones.
- **`Engine` = despachador puro:** recibe las ventanas por referencia y NO las
  anida. Su estado interno decide qué ventana se ve; al cambiar de estado llama al
  `begin()` de la ventana entrante. `setup()` llama `display.begin()`,
  `buttons.begin()` y `engine.begin()`; `loop()` solo llama `engine.update()` y
  `engine.print()`.
- Al iniciar se muestra la animación de arranque (`Boot`, franjas verticales),
  luego el panel de botones (`Legend`, flechas "Move"/"Action" y Btn1..Btn4) y
  después el menú inicial, los placeholders "En desarrollo" (Nuevo, Continuar,
  Dificultad, Sonido) y los créditos. La `Legend` también se muestra al volver al
  menú desde cualquier ventana. El código del juego original está respaldado (no
  restaurado) en `Snake_II_juego_backup.txt`.

La clase `Engine` (despachador de ventanas, antes `App`) está separada del `.ino`
en `Engine.h` / `Engine.cpp`, y recibe las ventanas **sin anidarlas**.

Fases pendientes: integración de la clase `Display` en el juego, botones/pulsadores
(`Buttons` ya integrado), buzzer, menú, lógica de la serpiente.

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
| `Legend.h` / `Legend.cpp` | Clase `Legend` (panel de botones: dos pad direccionales —MOVE a la izquierda, ACTION a la derecha con etiquetas Btn1..Btn4—, se muestra tras el arranque y al volver al menú; cualquier botón la cierra). Completa. |
| `Menu.h` / `Menu.cpp` | Clase `Menu` (menú con scroller de 1 bit y rombos de posición). Completa. |
| `Credits.h` / `Credits.cpp` | Clase `Credits` (ventana de créditos con 3 entradas navegables con transición lateral, vuelve al menú con `ACTION_UP`). Completa. |
| `InfoWindow.h` / `InfoWindow.cpp` | Ventana genérica "En desarrollo" (Nuevo, Continuar, Dificultad, Sonido). Completa. |
| `Engine.h` / `Engine.cpp` | Clase `Engine` (despachador de ventanas, antes `App`). **No anida las ventanas**: las recibe por referencia y su estado interno decide qué ventana corre y cuándo cambiar (`changeState()`, que llama al `begin()` de la ventana entrante). Todos los `begin()` se lanzan desde `setup()`. Completa. |
| `Snake_II.ino` | Enlace de dependencias (wiring). Construye TODAS las clases: `Display`, `Buttons` y las ventanas hermanas `Boot`/`Legend`/`Menu`/`Credits`/`InfoWindow` (compartidas por referencia, valores conservados). Crea `Engine` con esas referencias; `setup()` llama `display.begin()`, `buttons.begin()` y `engine.begin()`; `loop()` llama `engine.update()` y `engine.print()`. |
| `Snake_II_juego_backup.txt` | Respaldo del código del juego (Snake_II.ino original). |
| `GameBuzzer.h` | Clase del buzzer del juego original (sin cambios). |
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
| `Adafruit_SSD1306& screen()` | Acceso directo al objeto OLED. |

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
| `void read()` | Leer físicamente, aplicar debounce y generar eventos. Llamar una vez por `loop()`. |
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

---

## 7. Clase `Menu` — API

Ubicación: `Menu.h` / `Menu.cpp`.

### Constructor

```cpp
Menu(Display& display, Buttons& buttons, uint8_t topScore = 0, const char* version = "v0.1");
```

### Métodos

| Método | Descripción |
|--------|-------------|
| `void begin()` | Restablece el estado de la animación y del parpadeo. |
| `void setOptions(textos, conteo)` | Fija la lista y la cantidad de opciones (1..`MAX_OPTIONS`=8). El menú (textos y rombos) se adapta al conteo. |
| `void update()` | Lee botones, navega con `MOVE_RIGHT`/`MOVE_LEFT` y anima el deslizamiento lateral; log en Serial al cambiar de opción. |
| `void print()` | Dibuja título, cuadro fijo con la opción deslizante, rombos de posición y pie (Top + versión). |
| `int8_t selected()` | Índice de la opción seleccionada. |
| `int8_t confirm()` | Devuelve la opción seleccionada si se confirma con `ACTION_RIGHT` (pulse recién presionado), o `-1`. Es el "activar opción" del menú. |
| `void setTopScore(uint8_t)` | Actualiza el puntaje máximo mostrado. |

### Opciones y enum

```cpp
enum Option : uint8_t {
  OPC_NUEVO = 0, OPC_CONTINUAR, OPC_DIFICULTAD, OPC_SONIDO, OPC_CREDITOS
};
```

El enum documenta los índices de las 5 opciones por defecto. La cantidad real es
variable (`setOptions`), con `MAX_OPTIONS = 8`.

### Diseño del menú (scroller de 1 bit)

- **Título:** "Snake II", `TEXT_12x16`, centrado en `REGION_HEADER`. Se dibuja al
  inicio, luego se limpia la banda (0..16) con negro y se redibuja.
- **Pie:** "Top: X pts" (`LEFT_DOWN`) y versión (`RIGHT_DOWN`) en `TEXT_6x8`,
  **bajado 1 px** (fila 57, `PIE_TOP = 57`). Se limpian con negro las filas
  `54..63` antes de dibujar. Una **línea horizontal de 1 px** de grosor,
  `drawFastHLine`, en la fila `54` (`PIE_LINE_ROW`), a **2 px sobre el pie**
  (filas libres `55..56` entre la línea y el texto). Las 2 filas sobre el pie
  quedan siempre limpias.
- **Cuadro de selección:** **fijo** y de **ancho completo** (128 px),
  `BOX_TOP = 25`, `BOX_HEIGHT = 18` (banda 25..42). Con el rombo activo de punta
  en la `45`: quedan **2 filas libres** (`44..43`) sobre el rombo y el cuadro
  arranca en la **fila 3** desde la punta (`42`) hacia arriba. **No se mueve**;
  el tamaño del texto (tamaño 2) tampoco cambia.
- **Animación (scroller de 1 bit):** cada opción se compone **antes** de
  mostrarse en una **matriz de 128×16 de 1 bit** (`_strip[16][16]`, `1` = glifo
  negro, `0` = espacio blanco) **centrada**, mediante el canvas auxiliar
  `_composer` (128×16) que dibuja el texto (`loadOption`). La **banda del
  cuadro** es un **canvas persistente** (`_chipBox`, 128×16): conserva lo que
  está en pantalla entre frames, así la opción anterior **se mantiene hasta ser
  borrada** por la nueva (`slideStrip` sobrescribe columna a columna la banda
  con la tira entrante, incluidos sus espacios blancos; `blitBand` la vuelca a
  la pantalla). Al navegar con `MOVE_RIGHT` la tira entra por la **derecha** (se
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
  período y oculto el **25%** (período `BLINK_PERIOD = 1000 ms`, fase oculta = primer
  25%). Antes de dibujarlos se limpia con negro la banda
  `45..53` (`fillRect(0, DIA_TOP, ancho, DIA_SIZE+1)`).
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
- **Duración:** `TOTAL_MS = 4000 ms` o cualquier botón, lo que ocurra primero; luego
  `Engine` entra al menú.

---

## 9. Clase `Legend` — API

Ubicación: `Legend.h` / `Legend.cpp`.

### Constructor

```cpp
Legend(Display& display, Buttons& buttons);
```

### Métodos

| Método | Descripción |
|--------|-------------|
| `void begin()` | Reinicia la ventana: apaga el flag de salida. |
| `void update()` | Lee botones; cualquier botón pone `done() = true`. |
| `void print()` | Dibuja los dos pad direccionales y los rótulos. |
| `bool done()` | `true` cuando se pidió ir al menú (Engine pasa al menú). |

### Dibujo (leyenda de botones)

- **Pad MOVE (izquierda):** rombo de 4 flechas sólidas (`drawArrow`) centrado en
  `(PAD_MOVE_X=30, CY=32)`, radio `R=12`, con el rótulo "Move" arriba.
- **Pad ACTION (derecha):** rombo de 4 flechas centrado en `(PAD_ACTION_X=82,
  CY=32)` con el rótulo "Action" arriba y una **columna de etiquetas** al lado
  derecho (`LABEL_X=104`): `Btn1` (↑ = `ACTION_UP`), `Btn2` (→ = `ACTION_RIGHT`),
  `Btn3` (↓ = `ACTION_DOWN`), `Btn4` (← = `ACTION_LEFT`).
- **Salida:** cualquier botón cierra la leyenda (`done()`). `Engine` la muestra al
  arranque (después del `Boot`) y al volver al menú desde cualquier ventana.

---

## 11. Clase `Engine` — despachador de ventanas

Separada del `.ino` en `Engine.h` / `Engine.cpp` (antes `App`). **No anida las
ventanas**: `Boot`, `Legend`, `Menu`, `Credits`, `InfoWindow` (y el futuro `Juego`)
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
       Credits& credits, InfoWindow& info, Legend& legend);
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

### Estados internos

```cpp
enum class State : uint8_t {
  BOOT = 0, MENU, NUEVO, CONTINUAR, DIFICULTAD, SONIDO, CREDITOS, LEGEND
};
```

| Estado | Ventana | Notas |
|--------|---------|-------|
| `BOOT` | `Boot` | Animación de arranque (franjas). Al terminar (`done()`) pasa a `LEGEND`. Cualquier botón la termina. |
| `LEGEND` | `Legend` | Panel de botones (flechas Move/Action + Btn1..Btn4). Cualquier botón la cierra → menú. Se muestra tras el arranque y al volver al menú desde cualquier ventana. |
| `MENU` | `Menu` | Confirma con `ACTION_RIGHT` (`confirm()`). |
| `NUEVO`, `CONTINUAR`, `DIFICULTAD`, `SONIDO` | `InfoWindow` | Placeholder "En desarrollo" (tamaño 1); se reemplazarán por `Juego`/`Config` reales. Al salir (`done()`) pasa a `LEGEND`. |
| `CREDITOS` | `Credits` | 3 entradas navegables con `MOVE_LEFT`/`MOVE_RIGHT` y transición lateral (rol tamaño 2 **seleccionado con cuadro de borde a borde** y centrado en el alto restante del Body; nombre tamaño 1 plano en el pie). La transición usa el **mismo scroller de 1 bit que el menú** con **dos bandas sincronizadas**: rol (128×16) y nombre (128×8) se componen por separado (`loadEntry`, canvas `_composer` 128×16 → matriz `_strip[16][16]`) y deslizan a la vez con el **mismo `_slideX`** (aparecen al mismo tiempo). Cada banda es un **canvas persistente** (`_chipBoxRole`/`_chipBoxName`) que la tira sobrescribe **columna a columna** con sus espacios de fondo, así la entrada anterior se mantiene hasta que la nueva la cubre (superposición al navegar rápido). La animación **arranca desde el borde**: `startSlide` pone `_slideX = ±ancho` (fuera de escena) y avanza **1 px cada 4 ms con acumulador por tiempo** (`ANIM_TICK = 4`, como el menú, ≈0,5 s y constante aunque el loop sea lento). Al salir (`done()`) pasa a `LEGEND`. |

### Métodos

| Método | Descripción |
|--------|-------------|
| `void begin()` | Primera transición: entra al test de píxeles (`changeState(State::BOOT)`). Se llama desde `setup()`. |
| `void update()` | Lee/actualiza la ventana activa y gestiona las transiciones de estado. |
| `void print()` | Limpia (`display.clear()`) y dibuja solo la ventana activa. |
| `void setTopScore(uint8_t)` | Reenvía al menú para conservar el puntaje máximo entre sesiones. |

### Patrón de ventana

Toda ventana implementa:

| Método | Descripción |
|--------|-------------|
| `begin()` | Restablece la ventana al entrar. `Engine` lo llama solo en `changeState()`. |
| `update()` | Lee botones (`_buttons.read()`) y maneja sus eventos. |
| `print()` | Dibuja la ventana (Engine hace `display.clear()` antes de cada `print()`). |
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
- La demo actual (`Snake_II.ino`) usa solo `Display` y `Buttons` (la `Boot` usa
  botones solo para saltar patrones; el buzzer no se usa todavía).
- Con SDA=8 y SCL=9, dirección 0x3C.
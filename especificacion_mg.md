# MetaGráfica V3 — Especificación del Lenguaje

> **Estado:** Borrador de trabajo. Las secciones marcadas con ⚠️ están abiertas a decisión.

---

## Decisiones de diseño ya tomadas

| Tema | Decisión |
|---|---|
| Variables | Asignación directa `x = 5.5`, sin `SET` |
| Colores | Nombres modernos: `"blue"`, `"#4080FF"` |
| Argumentos | Mixtos: posicionales primero, nombrados después (como Python) |
| Compatibilidad V1 | Traductor Python, no modo dual en el compilador |
| Structs | Con parámetros nativos: `struct Nombre(args) { }` |
| Loops | `for x = 0 to 10 step 1 { }` |
| Nombres | Comandos descriptivos en minúsculas: `polyline`, `circle`, `rectangle`, `bezier`, `text` |
| Caso (case) | **Todo en minúsculas**; `snake_case` para compuestos inevitables (`world_window`, `tick_size`). Se **recomienda Capitalizar** los nombres de structs del usuario (`Flecha`, `Tick`) para distinguirlas de los comandos |
| Paths | Secuencias de pares `x y`; primitivas multi-punto aplican la operación en cada punto |
| Transformaciones | Sentencias de estado `translate`/`rotate`/`scale`/`shear`, con ámbito de bloque `{ }`, ver §7/§11 |
| Posición de pluma | `moveto(x,y)` fija el punto actual; `step(...)` define el avance entre elementos, ver §12 |
| Condicionales | `if cond { } else { }` con comparadores y `and`/`or`, ver §6.1 |
| Recursión | Las structs pueden autoinvocarse; paro con `if`, límite con `max_depth`, ver §8.1 |
| Aspect ratio | Espacio isométrico por construcción: escala uniforme (*meet*) + margen; deformación solo con `stretch=true`, ver §3.1 |
| Unidad física | El punto tipográfico (`pt` = 1/72"): grosor, fuente y `dash` se miden en pt; salida vectorial independiente de la resolución (no es "72 DPI"), ver §3.2 |

---

## 1. Estructura Léxica

- **Identificadores (`ID`):** `[a-zA-Z_][a-zA-Z0-9_]*`
- **Números (`NUMBER`):** `[0-9]+(\.[0-9]+)?` (punto flotante)
- **Cadenas (`STRING`):** texto entre comillas dobles `"..."`
- **Comentarios:** desde `%` hasta fin de línea
- **Delimitadores:** `{ }` para bloques de geometría, `( )` para argumentos, `,` entre parámetros
- **Paths nombrados:** `&nombre` referencia a un path definido previamente

**Convención de nombres.** Todos los comandos y palabras clave del lenguaje van en minúsculas (`polyline`, `place`, `transform`, `for`). Los nombres compuestos inevitables usan `snake_case` (`world_window`, `tick_size`). El compilador distingue mayúsculas y minúsculas, por lo que se recomienda **Capitalizar los nombres de structs definidas por el usuario** (`Flecha`, `Tick`): así un lector distingue de un vistazo lo que es del lenguaje (minúscula) de lo que es del usuario (Mayúscula inicial).

---

## 2. Gramática Central (EBNF)

```ebnf
Program     ::= Statement*
Statement   ::= ConfigStmt
              | Assignment
              | PathDef
              | StructDef
              | ForStmt
              | IfStmt
              | StateStmt
              | Block
              | CompoundStmt
              | RepeatStmt
              | PenStmt
              | Generator
              | Placement
              | Primitive
              | TextStmt
              | Import

ConfigStmt  ::= "display_size" NUMBER NUMBER
              | "world_window" NUMBER NUMBER NUMBER NUMBER (ID "=" ArgValue)*

Assignment  ::= ID "=" RHS
RHS         ::= Expression | STRING | Tuple | Array   % una variable puede ser número, cadena, tupla o lista
PathDef     ::= "path" ID "=" PathExpr

StructDef   ::= "struct" ID "(" [ParamList] ")" "{" Statement* "}"
ForStmt     ::= "for" ID "=" Expression "to" Expression ["step" Expression] "{" Statement* "}"
IfStmt      ::= "if" Condition "{" Statement* "}" [ "else" "{" Statement* "}" ]
StateStmt   ::= AttrName ArgValue+            % sentencia de estado, ámbito léxico (§7)
AttrName    ::= "color" | "fill" | "line_width" | "dash" | "cap" | "join"
              | "hatch" | "hatch_gap" | "font" | "font_size" | "align" | "valign"
              | "translate" | "rotate" | "scale" | "shear"
Block       ::= "{" Statement* "}"            % bloque anónimo: apila/restaura estado (§7.1)
Import      ::= "include" STRING

CompoundStmt  ::= "compound" "(" [ArgList] ")" "{" (Primitive | Placement)* "}"
RepeatStmt    ::= "repeat" "(" ID "," ArgList ")"
PenStmt       ::= "moveto" "(" Expression "," Expression ")"
              | "step"   "(" ArgList ")"
Generator     ::= "ticks"   "(" ArgList ")"
              | "numbers" "(" ArgList ")"
              | "axis"    "(" ArgList ")" "{" PointList "}"
              | "grid"    "(" ArgList ")" "{" PointList "}"

Primitive   ::= PrimName "(" [ArgList] ")" "{" PointList "}"
TextStmt    ::= "text" "(" STRING ["," ArgList] ")" [ "{" PointList "}" ]

Placement   ::= "place" "(" ID [ "," ArgList ] ")" "{" Locus "}"
              | "fit"   "(" ID ")" "{" PointList "}"
Locus       ::= PointList | "&" ID

PathExpr    ::= "{" PointList "}"                % path literal
              | "bezier"  "{" PointList "}"
              | "smooth"  "{" PointList "}"
              | "trail"   "(" ArgList ")"
              | "tile"    "(" "&" ID "," "times" "=" NUMBER ")"
              | "concat"  "(" "&" ID ("," "&" ID)+ [ "," "join" "=" ("true"|"false") ] ")"
              | "reverse" "(" "&" ID ")"
              | "normalize" "(" "&" ID ")"
              | "fitrect" "(" "&" ID ")" "{" PointList "}"

PrimName    ::= "polyline" | "polygon" | "circle" | "rectangle"
              | "arc" | "ellipse" | "dot" | "marker" | "polybar" | "sine" | "bezier"
              | "smooth"        % sine y smooth son también expresiones de path (§9)

ParamList   ::= Param ("," Param)*
Param       ::= ID ["=" Expression]              % parámetros con default, p. ej. size=0.2
ArgList     ::= Arg ("," Arg)*
Arg         ::= Expression | ID "=" ArgValue     % posicionales primero, nombrados después
ArgValue    ::= Expression | STRING | Tuple | Array | TransformList
Tuple       ::= "(" Expression "," Expression ")"
Array       ::= "[" [ ArrayElem ("," ArrayElem)* ] "]"   % p. ej. [10,2,1,2] o ["red","blue"]
ArrayElem   ::= Expression | STRING | Tuple
TransformList ::= TransformCall TransformCall*   % composición: rotate(30) scale(0.95)
TransformCall ::= ("translate" | "rotate" | "scale" | "shear") "(" ExpressionList ")"
PointList   ::= Subpath (";" Subpath)*        % ';' separa subtrayectos disjuntos
Subpath     ::= (Coord Coord)+                % pares (x y)
Coord       ::= Term                          % ¡Term, no Expression! Un '-' inicia
                                              % coordenada (unario); +/- binario en una
                                              % coordenada exige paréntesis: (x+0.1)

Condition   ::= Comparison (("and" | "or") Comparison)*
Comparison  ::= Expression RelOp Expression
RelOp       ::= "<" | ">" | "<=" | ">=" | "==" | "!="

Expression  ::= Term (("+" | "-") Term)*
Term        ::= Unary (("*" | "/") Unary)*         % módulo = función mod(a,b) ('%' es comentario)
Unary       ::= "-" Unary | Power
Power       ::= Atom ["^" Unary]                    % ^ = potencia, asociativa a la derecha
Atom        ::= NUMBER | ID | Index | "&" ID | FuncCall | "(" Expression ")"
Index       ::= ID "[" Expression "]"            % indexado de lista, base 0
FuncCall    ::= ID "(" [ExpressionList] ")"
ExpressionList ::= Expression ("," Expression)*
```

---

## 3. Espacio de trabajo

```text
display_size 12 8        % dimensiones físicas del canvas en cm
world_window 0 24 0 16   % coordenadas de usuario: xmin xmax ymin ymax
```

Las coordenadas en todos los comandos están en unidades de usuario (`world_window`). El compilador aplica la transformación a las unidades de salida.

### 3.1 Aspect ratio: espacio isométrico por construcción

V3 mapea la `world_window` al `display_size` con un **único factor de escala uniforme**, el mayor que hace caber la ventana completa en el canvas (semántica *meet* de `preserveAspectRatio` en SVG). Una unidad de usuario mide lo mismo en x que en y, **siempre**.

- **Proporciones distintas** → la ventana se centra en el canvas y el sobrante queda como margen. El anclaje se controla con `align=` (`"center"` default; `"left"`, `"right"`, `"bottom"`, `"top"`, combinables como `"bottom-left"`).
- **Deformación solo explícita** — `stretch=true` reproduce el estiramiento por eje de V1, para quien lo quiera deliberadamente:

```text
world_window 0 24 0 16                    % isométrico, centrado (default)
world_window 0 24 0 16 align="bottom"     % isométrico, anclado abajo
world_window 0 24 0 16 stretch=true       % estira por eje, como V1
```

**Consecuencia central del diseño.** Con el espacio isométrico, círculos, arcos, rotaciones, shear, structs y placements funcionan sin compensación alguna: la garantía de no-deformación es del sistema de coordenadas, no de cada primitiva. Desaparecen del motor los ajustes de V1 (radio de círculo normalizado solo con el eje y, structs colocadas a escala `min(dvx,dvy)`, anti-escalas por `ratio` en los placements), que eran compensaciones parciales de la normalización por eje al cuadrado unitario.

*(V1: el parser normalizaba cada eje por separado a [0,1]² y el backend estiraba con `scale(dvx,dvy)`; el manual pedía como "buena práctica" que `WW` y `$D` tuvieran la misma proporción.)*

### 3.2 Unidad física: el punto tipográfico

MetaGráfica maneja dos clases de cantidad y conviene no confundirlas:

- **Cantidades de mundo** — posiciones y tamaños geométricos expresados en unidades de
  `world_window` (§3.1): coordenadas de puntos, radios de `circle`/`arc`, dimensiones de
  `rectangle`, tamaño al que se ajusta una struct. Escalan con la ventana.
- **Cantidades físicas** — se miden en **puntos tipográficos** (`pt` = 1/72 de pulgada),
  la unidad nativa de PostScript y PDF: el `display_size` (dado en cm y convertido a
  pt), el **grosor de línea** (`line_width`, §4.10), el **tamaño de fuente** (`font_size`, §4.8) y
  las longitudes de un patrón `dash`. **No** escalan con la ventana; conservan su tamaño
  físico independientemente de `world_window`.

El punto es la **única unidad física** del lenguaje. El sistema de coordenadas de salida
es la retícula de puntos (72 unidades por pulgada); los tres backends inyectan la escala
cm→pt (`display_size`) en su matriz de dibujo, y SVG declara además su lienzo en `pt`
—con `viewBox` de extensión igual al número de puntos— para no caer en el default de
96 px/pulgada de CSS. Así una línea `line_width=1` mide 1/72" ≈ 0.35 mm y una fuente `font_size=8`
mide 8 pt en cualquier backend.

**Esto no supone una resolución de 72 DPI.** La salida es vectorial y, por tanto,
independiente de la resolución: el 72 es la *definición* del punto, no una densidad de
píxeles. Una densidad real (DPI) solo interviene al **rasterizar** a un formato de mapa
de bits o al definir un patrón en píxeles de dispositivo; para EPS, PDF y SVG
vectoriales el DPI nunca entra en juego.

---

## 4. Primitivas gráficas

Los atributos de estilo se pasan como argumentos nombrados. Los argumentos posicionales son los parámetros geométricos propios de cada primitiva. El bloque `{ }` contiene la lista de puntos.

**Regla de posición.** La magnitud que define la forma va **posicional y primero** —el radio de `circle`/`dot`, el par `rx, ry` (en ese orden) de `ellipse`, la cadena de `text`— y su nombre es opcional (`circle(2)` ≡ `circle(r=2)`). Los calificadores que no se leen solos como números sueltos van **nombrados** (`from=`/`to=` de `arc`, `hatch_gap=`), para no volverlos crípticos.

**Regla de coordenadas.** Las coordenadas de posición van **siempre en el bloque `{ }`**, nunca en los paréntesis. La regla no tiene excepciones, incluido el texto: `text("etiqueta") { x y }` coloca la cadena igual que `circle(r) { x y }` coloca un círculo (§4.8). El paréntesis lleva *contenido y atributos* (qué se dibuja y con qué estilo); el bloque lleva *dónde* (uno o varios puntos). De esa ortogonalidad sale gratis la siembra múltiple: `circle(0.2) { 0 0  5 5 }` traza dos círculos y `text("×") { 0 0  5 5 }` estampa la misma etiqueta en dos puntos.

**Subtrayectos (`;`).** Dentro del bloque de una primitiva de trazo (`polyline`, `polygon`, `bezier`, `smooth`), un `;` separa **subtrayectos disjuntos**: `polyline { a b ; c d }` dibuja dos trazos independientes con el mismo estilo, sin conectarlos —evita repetir la primitiva cuando varios trazos comparten estilo. En `polygon` cada subtrayecto se cierra y rellena por separado; para un relleno **combinado con huecos** (regla par-impar) se usa `compound` (§9.4), no `;`.

**Coordenadas con expresiones.** Una coordenada del bloque puede ser una expresión, pero de nivel **`Term`** (§2): número, variable, `func(...)`, `a*b`, `a^b`, con `-` unario inicial. Como las coordenadas van **separadas por espacio**, un `-` **siempre inicia una coordenada nueva** (negada), no una resta binaria: `{ -0.2 -0.5 }` son dos puntos, no `-0.7`. Para **sumar o restar** dentro de una coordenada, se usan paréntesis: `{ (x+0.1) y }`.

> ⚠️ **Las dos reglas interactúan.** Un identificador desnudo seguido de `(` es una **llamada a función**, aun con espacio en medio: al parentizar una coordenada puedes romper la de al lado, porque `{ 1/u  (u*u-u) }` se lee `1/u(u*u-u)` — una llamada, no dos coordenadas. *Regla práctica:* en un bloque con aritmética, **o parentizas todas las coordenadas, o ninguna**; nunca dejes una coordenada que termine en identificador desnudo junto a otra que abra con `(`.
>
> El compilador cubre las espaldas: un bloque con un número **impar** de coordenadas es **error de compilación** (`polyline recibió un número impar de coordenadas (1); van en pares x y`), y cada subtrayecto separado por `;` se valida por su cuenta. Es justo lo que delata el caso de arriba, donde las dos coordenadas colapsan en una.

**Argumentos de estilo comunes (todos opcionales y nombrados):**
- `color="blue"` / `color="#4080FF"` — color de trazo
- `fill="red"` / `fill="#RRGGBB"` — color de relleno (activa relleno sólido)
- `line_width=2.5` — grosor de línea en puntos (default `1.0`; ver §4.10)
- `dash="dashed"` — patrón de línea (ver §4.10)
- `cap="round"` / `join="round"` — extremos y uniones del trazo (ver §4.10)
- `hatch=45` — relleno con trama en lugar de sólido (ver §4.11)

#### Color, gris y contorno

- **Color** se especifica con nombre (`"blue"`, `"red"`, `"cyan"`, …) o hex (`"#RRGGBB"`).
- **Gris** es solo un color. Tres formas: un nombre (`"gray"`, `"lightgray"`), un hex neutro (`"#808080"`), o la función `gray(x)` con `x` de **0 (negro) a 1 (blanco)** —la convención de `setgray` de PostScript— cuando se necesita una escala fina sin memorizar hex:

```text
rectangle(fill=gray(0.9)) { 0 0  1 1 } % gris claro (90% blanco)
rectangle(fill=gray(0.2)) { 0 0  1 1 } % gris oscuro
```

  No hay un comando de gris separado. *(V1: `LGRAY`/`FGRAY g` → `color`/`fill` con el gris equivalente; como `FGRAY g` medía el % de negro, `FGRAY g` → `gray(1 - g/100)`.)*
- **Contorno de una forma rellena.** Por defecto una primitiva con `fill=` no dibuja su borde. Si además se da `color=`, se traza el contorno en ese color (convención tipo SVG):

```text
polygon(fill="cyan")                { 0 0  5 0  5 5  0 5 } % solo relleno, sin borde
polygon(fill="cyan", color="black") { 0 0  5 0  5 5  0 5 } % relleno + borde negro
```

*(V1: un valor negativo en `FILL`/`FCOLOR`/`FGRAY` o el prefijo `-` en un color, p. ej. `"-red"`, activaba el contorno. En V3 el contorno es simplemente la presencia de `color=`.)*

### 4.1 polyline — polilínea abierta

```text
polyline(color="black", line_width=1) {
    0 0   5 3   10 0
}
```

Con `closed=true` la polilínea **cierra su trayecto** (une el último punto con el primero) sin repetir la coordenada inicial. A diferencia de repetir el punto a mano, el cierre es real para los backends (`closepath`), de modo que la costura es una esquina limpia y no un traslape. Es el modo de dibujar un **contorno cerrado sin relleno** (un `polygon` siempre rellena, §4.2). El default es `closed=false`.

```text
polyline(closed=true) { 0 0   5 0   5 5   0 5 } % cuadrilátero cerrado, solo contorno
```

### 4.2 polygon — polígono cerrado

```text
polygon(fill="cyan") {
    0 0   5 0   5 5   0 5
}
```

### 4.3 circle — círculo; múltiples centros = múltiples círculos

```text
circle(2)               { 5 5 }           % un círculo (r posicional; `r=2` también válido)
circle(0.3)             { 1 1  3 1  5 1 } % tres círculos
circle(1, fill="red")   { 0 0  10 0 }     % dos círculos rellenos
```

El bloque `{ }` contiene pares `x y` de centros. Por cada par se dibuja un círculo.

### 4.4 rectangle

```text
rectangle(fill="gray") { 1 1  9 7 }  % esquina inferior-izquierda, esquina superior-derecha
```

Se define por dos esquinas opuestas y es una **forma transformable** (como el `<rect>` de SVG): bajo rotación, shear o escala anisótropa se dibuja transformando **las cuatro esquinas**, produciendo el cuadrilátero girado/deformado correcto —no una caja realineada a los ejes. Bajo traslación y escala uniforme queda el rectángulo de siempre. El orden de las esquinas se respeta: una esquina invertida **refleja** el contenido, igual que en `fit` (§10.2).

*(Implementado en los tres backends: `SVGDisplay`, `EPSDisplay::rect` y `PDFDisplay::rect` emiten el path de 4 esquinas transformadas —trazo, relleno y clip del tramado §4.11.)*

### 4.5 arc

```text
arc(3, from=0, to=180) { 5 5 }   % semicírculo superior, centro (5,5); r posicional, ángulos nombrados
arc(rx=4, ry=2, from=0, to=360) { 5 5 }  % arco elíptico completo
arc(1, from=190, to=530) { 0 0 } % barrido de 340° CCW desde 190° (V1 CR 1 340 190)
```

`from`/`to` son ángulos en **grados** y el arco va de `from` a `to` con barrido (con signo) `to − from`: **positivo = CCW**, negativo = CW. Por eso `to` puede **pasar de 360** (o quedar por debajo de `from`) para expresar barridos largos o el sentido, sin un parámetro `sweep` aparte.

### 4.6 dot y marker — marcadores de tamaño físico

Ambos estampan un símbolo en cada posición del bloque. Son **marcadores de tamaño físico**: su **posición** la transforma el sistema de coordenadas vigente, pero su **tamaño** (el radio `r`) es una cantidad **física, en puntos (pt)** —como `line_width` y `font_size`, §3.2— y **no** lo afectan la ventana ni las transformaciones, a diferencia del radio de `circle`, que es cantidad de mundo. Por eso son el primitivo correcto para marcar datos dentro de un marco `stretch` (§13.7): los símbolos caen donde deben y no se deforman por la anisotropía del marco.

**`marker(r, shape="…")`** es la primitiva: un símbolo de radio físico `r`, que cabe en un círculo de ese radio para que las formas queden comparables entre sí.

**`dot(r)`** es su **atajo** para el caso común, el disco: `dot(r)` ≡ `marker(r)`. **No lleva `shape=`** —un `dot` que dibujara un cuadrado sería un nombre mentiroso—; si quieres otra forma, la primitiva es `marker`.

```text
dot(4) { 2 3  4 3 }                                % disco relleno (atajo)
marker(3, shape="circle", color="black") { … }     % círculos abiertos (fig2-3)
marker(4, shape="cross") { … }                     % cruces +
marker(6, shape="Punta", marker_orient="auto") { … }  % un struct del usuario, sobre la tangente
```

**Formas** (`shape=`):

| `shape` | Símbolo |
|---|---|
| `"circle"` (default, = `dot`) | ○ / ● círculo |
| `"square"` | □ cuadro |
| `"diamond"` | ◇ rombo |
| `"cross"` | + |
| `"x"` | × |
| `"triangle"` | ▶ triángulo relleno apuntando a +x (el `triangle_right` de matplotlib) |
| `"arrow"` | ⟩ flecha de arpón (la del libro) — la única que se orienta sola |
| `"circle-dot"` | ⊙ anillo + punto central |
| *un nombre de struct* | lo que dibuje el struct |

**No hay forma `"disk"`, y es a propósito: la forma y el relleno son ejes independientes** (§4). El disco es `circle` **relleno**, que es el default; `color=` sin `fill=` da el círculo **abierto**. Duplicar el catálogo por relleno (disk/circle, square-lleno/square-vacío…) sería redundante. `cross`, `x` y `arrow` son siempre trazo, por geometría no cerrada (el arpón es un lazo dibujado a contorno), y ahí manda `color=`.

**`triangle` vs `arrow` — forma y papel son ejes distintos (act. 2026-07-17).** `triangle` es el triángulo relleno *estático*: un símbolo de datos como cuadro/rombo, con orientación **fija** por default. `arrow` es el arpón de contorno con lengüetas —la flecha que el libro usa a manos llenas— y es la **única** forma que se orienta a la **tangente por default** (`marker_orient="auto"`), porque su papel es *direccional* (marca flujo/sentido), no ser un símbolo puntual. Cualquiera de las dos se puede forzar al otro modo con `marker_orient="fixed"|"auto"`. Antes `arrow` nombraba al triángulo; al entrar el arpón built-in (que retira el `include "arrow.mg"` de las figuras que lo usan como marcador; ver `fig4-1.mg`), el triángulo pasó a llamarse `triangle` y `arrow` quedó para el arpón. El arpón es la geometría de `Arrowr` (× 2/3), con la **punta en el ancla** (no centrado en bbox, a diferencia de los símbolos simétricos): así la punta cae exactamente en el punto —el extremo de la línea en un `marker_end`— y al orientarse a la tangente rota alrededor de la punta, que queda fija. Su tamaño es **físico** en pt (inmune a la ventana), a diferencia del viejo struct de `arrow.mg`, cuyo tamaño iba por `scale` (cantidad de mundo).

**`circle-dot` (⊙, añadido 2026-07-17, con `fig1.mg`)** es la excepción a "la forma y el
relleno son ejes independientes": es una forma COMPUESTA con relleno mixto por construcción
(anillo abierto + disco relleno), no una casilla más del catálogo relleno/vacío. No pasa por
`MarkerShape`/subpaths como el resto (esos son polígonos rectos en caja unitaria; un anillo
necesita un arco real, y una sola forma no puede tener dos subtrayectos con relleno distinto):
se resuelve como DOS `dot()` reales superpuestos —anillo (`setFilled(false)`) y punto central al
30% del radio (`setFilled(true)`)—, igual que `circle` se resuelve como un arco real y no como
polígono. Proporción tomada de la digitalización de Fig. 1 (`plan_fig1.md`): anillo ~23px, punto
central ~6px de diámetro.

**Orientación (`marker_orient=`).** Tres modos, un solo argumento:
- `"auto"` — sigue la **tangente** local de la curva (default del arpón `arrow`).
- `"fixed"` — como se dibuja: el símbolo apunta a **+x** (default del resto).
- **un número en grados** — ángulo **fijo**, rota la forma esa cantidad. Así `triangle`
  apunta arriba con `marker_orient=90` (▲), a la izquierda con `180` (◀), abajo con `270`
  (▼); y sirve para cualquier forma (`square` a `45` = ◆). El ángulo es relativo al marco
  del mundo (bajo `stretch`/rotación de marco lo acompaña), no al papel; reusa la misma
  rotación que la tangente, así que es consistente en los tres backends (respeta el flip
  de SVG). No hay familia `triangle-up/down/left/right` a la matplotlib: una casilla por
  dirección duplicaría el catálogo cuando un ángulo lo cubre todo.

**Un struct como marcador.** Donde se pide una forma se acepta también **el nombre de un struct** del usuario: `marker(6, shape="Punta")`. La geometría se extrae del struct y se estampa con el mismo tamaño físico. La orientación por defecto es **fija** —un struct no se sabe "flecha", a diferencia del builtin `arrow`— y se cambia con `marker_orient="auto"` (tangente) o un ángulo en grados. Es el mismo catálogo, y las mismas reglas, que los atributos `marker_start`/`marker_mid`/`marker_end` de `polyline`.

**Orientación en `polyline` (`marker_orient` + por extremo).** Mismos valores que en `arc`: ausente = **por forma** (`arrow`→tangente, el resto→`fixed`); `"auto"` = tangente local; `"fixed"` = +x; **`"reverse"`** = tangente + 180° (la flecha apunta hacia atrás en la curva); un número = ángulo absoluto en grados. `marker_orient` es **global**; **`marker_start_orient`/`marker_end_orient`** lo sobreescriben por extremo — p. ej. `marker_start="arrow", marker_end="arrow", marker_start_orient="reverse"` deja las dos cabezas apuntando **hacia afuera** (la cota de toda la vida). `reverse` funciona también con `marker`=todos (cada instancia invierte su tangente local), porque se resuelve en draw-time sobre la dirección.

**Desplazar el marcador de extremo sobre su segmento (`marker_start_shift`/`marker_end_shift`).** Por default el marcador de `polyline` cae en el **vértice** del extremo. Para una **cota** (flecha adentro de la línea, no en la punta) `marker_end_shift=` lo corre a lo largo del último segmento, en `[0,1]`: **1** (default) = en el vértice; **0** = en el vértice adyacente interior; intermedio = interpolación. `marker_start_shift=` hace lo análogo sobre el primer segmento. El marcador conserva la orientación de ese segmento (o lo que fije `marker_orient`, incluido `reverse` y los overrides por extremo). Es el `shift` de `LNST` (V1), ahora como atributo del marcador; aplica solo a `start`/`end` (`mid` y `marker`=todos no tienen "segmento de extremo").

> **Los dos `marker` no son lo mismo, y no colisionan.** `marker(shape=…)` es la **primitiva**: estampa un símbolo por punto. `polyline(marker=…)` es el **atributo** que decora los vértices de una curva. Ahí el argumento se llama `marker` porque nombra *qué* se pone; en la primitiva se llama `shape` porque la primitiva ya *es* el marcador.

**Marcadores en `arc` (§4.5).** Un `arc` acepta `marker_start`/`marker_end` (y `marker` = ambos extremos): `arc(2, from=20, to=160, marker_end="arrow")` = **flecha curva** (punta en el extremo del arco, orientada a su tangente). A diferencia de `polyline` —donde el path SON los vértices y el marcador se dispersa sobre ellos— el path de un `arc` son **centros**; el punto del marcador y su orientación se **derivan de los parámetros** del arco (`centro + r·(cos θ, sin θ)` en `from`/`to`, tangente = perpendicular al radio, en el sentido del barrido). Por eso solo `start`/`end` tienen sentido (un arco no tiene vértices interiores). `marker_orient` ajusta el sentido: ausente o `"auto"` = tangente del barrido (default); **`"reverse"`** = la flecha apunta al revés (tangente + 180°, sin mover el extremo ni invertir `from`/`to`); `"fixed"` = +x; un número = ángulo absoluto en grados. `marker_orient` es **global** (afecta ambos extremos); **`marker_start_orient`/`marker_end_orient`** lo sobreescriben por extremo, para invertir **solo uno** — p. ej. `marker="arrow", marker_start_orient="reverse"` deja las dos cabezas apuntando hacia afuera (flecha curva **bidireccional**). El resto de primitivas de forma cerrada (`polygon`, `circle`, `bezier`) aún no llevan marcadores —esperan una figura que los pida.

El radio `r` es **físico**: no se deforma bajo `stretch` ni bajo transformaciones —solo la posición se transforma.

*(V1: `DOT` (físico) y los marcadores con prefijo `MK`, inconclusos —ver `plan_marcadores.md`. La familia de **atributos** `marker_start`/`marker_mid`/`marker_end`, que decora los vértices de una curva en vez de dispersar puntos sueltos, comparte estas mismas formas y este render físico.)*

### 4.7 bezier — curva de Bézier cúbica

```text
bezier { p0x p0y  c1x c1y  c2x c2y  p1x p1y }
```

Los puntos son: posición inicial, control 1, control 2, posición final. Segmentos adicionales concatenan la curva.

### 4.8 text — texto en posición

La cadena y los atributos van en los paréntesis; la posición va en el bloque, como en cualquier primitiva (§4, regla de coordenadas):

```text
text("Texto normal") { x y }
text("Título", color="red", font_size=14) { x y }
text("Nota", font="italic", size=10) { x y }               % font= cara; size = alias de font_size
text("$\alpha + \beta = \gamma$", align="center", valign="middle") { x y }
text("etiqueta") { 0 0  5 5  10 0 }   % la misma cadena en varios puntos
```

Los atributos por-primitiva de `text()` son `font_size` (alias `size`), `font` (cara: `"roman"`/`"italic"`/`"bold"`/`"sanserif"`/`"courier"`…, §14.3), `color`, `align` y `valign`; todos combinan entre sí y con el markup interno del string. El argumento `align` (horizontal) puede ser `"left"` (default), `"center"`, `"right"`; `valign` (vertical) puede ser `"baseline"` (default), `"top"`, `"middle"`, `"bottom"`. Son ortogonales: se combinan y se fijan por separado, también como estado (§7.3). La forma sin bloque (o con bloque vacío), `text("...")`, dibuja en la posición de pluma (§12.1). El markup interno del string se documenta en §14.

### 4.9 ellipse — elipse completa

```text
ellipse(4, 2) { 5 5 }              % elipse centrada en (5,5); rx, ry posicionales (x, y)
ellipse(4, 2) { 2 2  8 2 }         % múltiples centros = múltiples elipses
```

`ellipse` es una conveniencia para la elipse cerrada completa; equivale a `arc(rx, ry, from=0, to=360)`. Para arcos elípticos parciales se usa `arc` con `from`/`to` (§4.5). *(V1: `EL rx ry`.)*

### 4.10 Estilo de línea (`line_width`, `dash`, `cap`, `join`)

V3 abandona los índices cerrados de V1 (`LWIDTH n`, `LPATRN n`) por atributos con nombre
compatibles de forma nativa con PostScript, PDF y SVG. Los cuatro son opcionales y se
pasan a cualquier primitiva geométrica (o se fijan como estado de bloque, §7).

#### Grosor (`line_width`)

- Valor de punto flotante en **puntos tipográficos** (`pt` = 1/72 de pulgada), el
  estándar universal de la industria editorial. `line_width=1.5` son 1.5 pt.
- Si se omite, el grosor por defecto es **`1.0` pt**.
- **No existe** el comportamiento *hairline* dependiente del dispositivo (el `line_width = 0`
  del PostScript clásico, que dibuja el trazo más fino que el aparato permita). Para una
  línea ultrafina se especifica un valor real y explícito, p. ej. `line_width=0.1`.

```text
polyline(line_width=1.5) { 0 0  10 10 }
circle(line_width=0.25)  { 5 5 }          % línea fina (≈ el LWIDTH 1 de V1)
```

*(V1: el grosor iba en unidades de 0.2 pt —`LWIDTH 1` = 0.2 pt, `LWIDTH 5` = 1.0 pt— y
`LWIDTH 0` producía el hairline del dispositivo. El traductor convierte
`LWIDTH n` → `line_width = n × 0.2` y, por la prohibición del hairline, `LWIDTH 0` → `line_width=0.1`.)*

#### Patrón (`dash`)

El argumento `dash` es "inteligente": acepta un **nombre de alias** o un **arreglo
explícito**.

**A. Alias predefinidos.** Reproducen los patrones clásicos de V1. Las longitudes son
trazo/hueco alternos, en puntos:

| `dash` | Trazo/hueco (pt) | Equivale a V1 |
|---|---|---|
| `"solid"` (default) | — (línea continua) | `LPATRN 0` y `LPATRN 1` |
| `"dashed"` | `4 2` | `LPATRN 2` |
| `"dotted"` | `2 1.6` | `LPATRN 3` |
| `"dashdot"` | `4 2 1 2` | `LPATRN 4` |
| `"dashdotdot"` | `4 2 2 2 2 2` | `LPATRN 5` |

```text
polyline(dash="dashed") { 0 0  10 0 }
```

> **Nota sobre V1.** El motor de V1 fundía los patrones 0 y 1 en la misma línea continua
> (el "Pattern 1" de `line_patterns.mg` era indistinguible del 0), por eso ambos mapean a
> `"solid"` y no existe un sexto alias. Tampoco existía "long-dash".

**B. Arreglo explícito** (estilo `stroke-dasharray` de SVG). Para control total, `dash`
acepta un arreglo de números que alternan longitud de trazo y de hueco, **multiplicados
por el `line_width` actual** para conservar la proporción al cambiar el grosor:

```text
% trazo largo 10, hueco 2, punto 1, hueco 2  (× line_width)
polyline(dash=[10, 2, 1, 2]) { 0 0  10 10 }
```

Los tres backends lo soportan: EPS emite `[ … ] 0 setdash`, SVG `stroke-dasharray="…"`,
PDF `HPDF_Page_SetDash(...)`.

#### Extremos y uniones (`cap`, `join`)

Aunque V1 no los exponía, el motor PostScript/PDF/SVG subyacente siempre los ha
soportado. Son clave para gráficos de calidad profesional, sobre todo en líneas gruesas.

- **`cap`** — forma del extremo de un trazo abierto:

  | `cap` | Efecto |
  |---|---|
  | `"butt"` (default) | corte exacto en el nodo |
  | `"round"` | punta redondeada (ideal para `dash="dotted"`) |
  | `"square"` | punta cuadrada que sobresale medio grosor del nodo |

- **`join`** — forma de la esquina donde se unen dos segmentos (polilíneas, polígonos):

  | `join` | Efecto |
  |---|---|
  | `"miter"` (default) | esquina afilada en punta |
  | `"round"` | esquina redondeada |
  | `"bevel"` | esquina biselada (recortada) |

```text
polyline(line_width=5.0, cap="round", join="round") { 0 0  5 5  10 0 }
```

*(V1 no tenía equivalente; los trazos usaban los defaults `butt`/`miter`. El traductor
no genera `cap`/`join` salvo que se pidan explícitamente.)*

### 4.11 Patrones de relleno (`hatch`)

> ⚠️ **No hay transparencia (`alpha`), y meterla no es un feature chico.** Lo pide material real
> (`figure_02` rellena sus histogramas con `alpha=0.5`), pero **EPS no tiene alpha nativo**;
> PDF y SVG sí. Añadirla rompería la paridad de los tres backends que la Capa 3 del harness
> protege, o exigiría simular la mezcla en EPS (calcular el color compuesto contra un fondo
> conocido — falso en cuanto haya solapamiento). Es una decisión de arquitectura, no un
> atributo más: tomarla a propósito. La trama (`hatch`) cubre el caso de "dejar ver lo de
> abajo" sin salirse del modelo opaco.

En vez de relleno sólido, una primitiva cerrada puede rellenarse con una **trama** de líneas paralelas. `hatch` está **sobrecargado**, exactamente en paralelo a `dash` (§4.10, que acepta un alias de cadena o un índice):

- **Número** = ángulo (en grados) de **una** familia de líneas, **libre** (cualquier valor, no restringido a una tabla).
- **Cadena** = **estilo nombrado** (catálogo mínimo, à la Asymptote — se amplía solo cuando el corpus lo exija):
  - `"hatch"` → una familia a **45°**.
  - `"hatchback"` → una familia a **135°** (diagonal invertida).
  - `"crosshatch"` → **dos** familias, 45°+135° (rejilla).

`hatch_gap` — separación perpendicular entre líneas en **puntos**; **opcional** (default 4 pt). Es una cantidad física (pt), no escala con `world_window` (§3.2), porque el tramado se barre en el espacio de dispositivo ya transformado a pt (`EPSDisplay::rect`/`useFillPattern` aplican la matriz antes de emitir; SVG teja un `<pattern>` en userSpace de pt). Las tres densidades de V1 (`FPATRN`) equivalían a `hatch_gap` de **4, 2 y 1 pt**; ese mapeo sigue funcionando pero `hatch_gap` ya no está limitado a esos tres valores.

No hay más: la trama es una forma de **relleno**, así que sus líneas toman el **color de relleno** (`fill`, §4; negro por default si no se fijó). Su **grosor** sale del estado de trazo (`line_width`, §4.10). No existe un `hatch_color` ni un `hatch_width` aparte.

**Contorno de la región.** Como en el modelo de estado (§7) *siempre* hay un color de trazo vigente, la presencia de `color=` no puede servir de disparador del contorno ahí. El contorno se controla así:

- **Registro por-primitiva** (§4): dar `color=` junto al relleno traza el borde en ese color, como en cualquier forma rellena.
- **Registro de estado**: la sentencia `outlinefill` activa el contorneo de los rellenos del bloque (`outlinefill` o `outlinefill true` = on; `outlinefill false` = off). El borde se traza con el `color`/`line_width` vigentes.

**Dos registros** (la distinción general de §7.5):

- **Atributo** (en paréntesis, nombrado) — aplica a una sola primitiva:

```text
polygon(hatch=30)                        { 0 0  5 0  5 5  0 5 } % una familia, ángulo libre 30°
polygon(hatch="hatch", hatch_gap=3)      { 0 0  5 0  5 5  0 5 } % 45°, paso 3 pt
polygon(hatch="hatchback")               { 0 0  5 0  5 5  0 5 } % 135°
polygon(hatch="crosshatch", hatch_gap=2) { 0 0  5 0  5 5  0 5 } % rejilla 45°+135°, paso 2 pt
```

- **Sentencia de estado** (posicional, rige hasta el fin del bloque, §7) — nombre o ángulo, y el paso opcional caben en una sola línea, igual que `translate dx dy`:

```text
fill "cyan"           % color de las líneas de la trama
hatch "crosshatch"    % estilo nombrado; el paso toma el default
hatch 30 2            % ángulo libre 30°, paso 2 pt
outlinefill           % además, contornea la región (color/line_width vigentes)
polygon { 0 0  5 0  5 5  0 5 }   % trama cyan + contorno
polygon { 6 0  9 0  9 5  6 5 }
```

  El segundo valor posicional de `hatch` es el paso; equivale a fijarlo aparte con `hatch_gap 4`.

*(V1: `FCOLOR c` + `FPATRN` → `fill "c"` + `hatch a`; el `FCOLOR` era el color de relleno y la trama lo hereda. `FPATRN n` codificaba ángulo y densidad en el índice `n`, restringido a 4 ángulos × 3 densidades; el `n` negativo que añadía el contorno se reemplaza por `color=` (por-primitiva) o `outlinefill` (estado). El índice sigue vivo solo como entrada del front-end V1 congelado.)*

### 4.12 polybar — barras (histogramas)

Dibuja una **barra** (rectángulo) por cada punto del bloque, desde una base común (0) hasta la altura dada. Cada punto es el **centro superior** de su barra. Pensada para histogramas y series de datos.

```text
polybar(width=0.8, fill="gray") { 1 40  2 55  3 30  4 70 }
```

- `width` — ancho geométrico de la barra en unidades de mundo, centrada sobre la x. **Obligatorio.**
- `dir="horizontal"` — las barras crecen en x desde 0; entonces la x del bloque es la **longitud** y la y la **posición** vertical. Default: vertical.
- Relleno, contorno y estilo siguen §4 (`fill`, `color`, `line_width`, `dash`, `hatch`), como cualquier forma cerrada.

**Barras superpuestas** (dos series sobre las mismas x) = dos `polybar` con distinto `width`/`fill`/`dash`:

```text
{
    line_width 1.5
    polybar(width=0.8, fill="gray")  { 1 40  2 55  3 30  4 70 }
    polybar(width=0.4, fill="black") { 1 35  2 60  3 25  4 65 }
}
```

Internamente `polybar` expande cada punto a un `rectangle` (§4.4) usando el `rect()` del motor, así que **no requiere lógica nueva en los backends**, y las barras heredan que `rectangle` es transformable. *(V0: se usó en el primer artículo publicado —los histogramas de espectro, `docs/first_article.pdf` figs. 4–6; `examples/fig_polybar.mg` reproduce la fig. 5.)*

**Barras contiguas: no se tapan.** `rect()` rellena dentro de un `gsave` y traza **fuera**, barra por barra: la barra *k+1* cubre el borde de la *k* con su relleno pero acto seguido lo retraza. Requiere contorno (`outlinefill`, o `color=` junto al relleno); con un `fill` sólido pelado las barras se fusionan en una silueta.

**Limitación — `width` en eje logarítmico.** `width` es un miembro de la primitiva, no una coordenada del path, así que el mapeador puntual de `plot` (§13.7, ruta log) **no lo alcanza**: en un eje x logarítmico todas las barras salen del mismo ancho físico en vez de seguir el mapeo. En eje lineal no hay problema (la matriz envolvente escala el `rect` completo). Sin resolver; ningún ejemplo lo pide todavía.

### 4.13 sine — segmento senoidal

Dibuja una **onda senoidal** a lo largo del segmento base dado en el bloque, oscilando perpendicular a él. Pensada para funciones de onda, señales y curvas periódicas suaves, sin ensamblar aproximaciones bezier a mano.

```text
sine(half_cycles=3, amplitude=1.2) { 0 5  10 5 }   % 3 jorobas de seno sobre la base
```

- `half_cycles` — número de medios ciclos (jorobas) a lo largo de la base. **Obligatorio.** `sin(nπx)` ⇒ `half_cycles=n`. Puede ser **fraccionario** (onda parcial).
- `amplitude` — amplitud perpendicular, en unidades de mundo. **Obligatorio.**
- `phase=deg` — desfase inicial en grados (default `0`; `phase=90` = **coseno**, la onda arranca en el máximo). Es un eje independiente de `squared` (la fase desplaza, `squared` eleva al cuadrado).
- `squared=true` — dibuja el **cuadrado** de la onda, `sin²` (una densidad `|ψ|²`): las jorobas quedan todas positivas, con forma de coseno levantado. Default `false`.
- La base puede ser cualquier segmento (horizontal, vertical o inclinado); la onda oscila perpendicular. Estilo (`color`, `line_width`, `dash`) como cualquier trazo.

Internamente `sine` se aproxima con curvas bezier (las de `bzsinepaths`). Los desfases y extensiones **múltiplos de 90°** (cuarto de ciclo) caen en frontera de segmento, así que se arman con piezas enteras (el coseno, `phase=90`, no recorta nada); un `phase` o `half_cycles` **no alineado a 90°** recorta el spline con de Casteljau —se implementa cuando el corpus lo pida. El caso `squared` usa la aproximación de coseno levantado (el `&cos2pi` del V1). Todo invisible: el autor solo da jorobas, amplitud y, si acaso, fase.

`sine` también puede usarse como **expresión de path** del álgebra §9 (`path p = sine(...) { base }`): la misma onda, como datos concatenables. Con `phase=90`/`270` cada llamada es un medio ciclo de coseno que baja/sube entre extremos con pendiente cero — la pieza natural para funciones de onda con envolvente por tramo (`concat` de piezas con `amplitude` distinta; caso guía: fig16-9).

*(V1: se ensamblaba a mano con `BZ &sinpi`/`&sin2pi`/`&cos2pi` + `RPPT` (tile) + la pila de matrices de path `IDPT`/`TLPT`/`SCPT`. `sine` colapsa todo eso. Caso guía: fig4-10, los niveles del pozo infinito.)*

---

## 5. Variables y expresiones

Ámbito léxico. Las variables definidas fuera de un bloque son globales al archivo. El
modelo completo de alcance (lectura, escritura y el matiz de las structs) se detalla en
§5.3.

```text
r_base = 5.5
margen = 2 * 0.5
n = 8

circle(r = r_base + 1) { margen margen }
circle(r = r_base / n) { 0 0 }
```

Los **operadores y funciones** disponibles se detallan en §5.2; las **listas**, en §5.1.

### 5.1 Listas

Una variable puede contener una **lista** de valores —números, cadenas o tuplas— con la misma sintaxis `[…]` de `dash` (§4.10). Se indexa con `lista[i]` (**base 0**) y `len(lista)` da su longitud. Es lo que permite recorrer una tabla de valores con un `for` (§6):

```text
fills = ["black", "blue", "brown", "cyan", "green", "red", "yellow"]

for i = 0 to len(fills)-1 {
    x = i * 0.1
    rectangle(fill = fills[i]) { x 0  x+0.1 0.1 }
}
```

El elemento extraído **conserva su tipo**: `fills[i]` es una cadena y sirve donde iría `"red"`; una lista de números sirve donde iría un número. Las listas son valores de primera clase (se asignan y se pasan como argumento), pero **no participan en la aritmética**: solo se indexan y se consulta su `len`.

### 5.2 Operadores y funciones

**Operadores aritméticos**, de menor a mayor precedencia:

| Operadores | Papel |
|---|---|
| `+`  `-` | suma / resta |
| `*`  `/` | producto / división |
| `^` | **potencia**, asociativa a la derecha (`2^3^2` = `2^(3^2)`) |
| `-` (unario) | negación; más débil que `^`, de modo que `-2^2` = `-(2^2)` = `-4` y `2^-1` = `0.5` |

Los paréntesis agrupan como de costumbre. `^` es un operador simple. **El módulo es la función `mod(a, b)`, no un operador `%`**: `%` es el carácter de **comentario** de MetaGráfica, así que no puede reusarse como operador.

**Concatenación de cadenas.** Con al menos un operando de cadena, `+` **concatena** en vez de sumar; un número se convierte a texto en formato mínimo (`%g`: entero sin punto, decimales solo si los hay). Sirve para armar etiquetas a partir de variables sin una `text` por elemento:

```text
names = ["solid", "dashed", "dotted"]
text("Pattern " + names[i] + " (#" + i + ")") { 1 y }   % "Pattern dashed (#1)"
text("x = " + (3*2.5)) { 1 5 }                           % "x = 7.5"
```

(Para un número con un número de decimales fijo dentro de una serie, `numbers` con `prefix`/`suffix` (§13.2) sigue siendo la vía; `+` da el formato mínimo por default.)

**Funciones y constante:** `sin(x)`, `cos(x)`, `tan(x)`, `sqrt(x)`, `abs(x)`, `atan2(y, x)`, `mod(a, b)`, `exp(x)`, `ln(x)`, `len(lista)`, `str(x[, decimals])`, y `pi`. El conjunto se mantiene deliberadamente pequeño; se ampliará (redondeo, `min`/`max`…) **solo cuando el corpus lo exija** —para no cargar el lenguaje con funciones especulativas.

`exp`/`ln` entraron por esa vía (2026-07-18), con un caso concreto detrás: un potencial de Morse, `D(1-exp(-a(r-re)))²`, no es escribible sin ellas —por eso la curva de `fig16-9.mg` está digitalizada, no por preferencia— y con ellas sus puntos de retorno salen en **forma cerrada**, `re - ln(1 ∓ sqrt(E/D))/a`, en vez de medirse sobre la curva ya dibujada. Ver `examples/franck_condon.mg`. `ln` de argumento no positivo es un error **fatal** (`evalError`), no un `-inf` que se propaga: en la práctica delata un modelo mal parametrizado —dar `D`, `we` y `xe` como independientes cuando la relación de Morse `D = we/(4·xe)` las liga— antes de que produzca una figura sutilmente incorrecta.

**Funciones definidas por el usuario: no hay.** La única abstracción es la struct (§8), que produce **gráficos, no valores**, así que una fórmula escalar que se repite —la energía de un nivel, una conversión de unidades— se reescribe en cada uso. Cuando lo que varía por iteración es un **path**, un `for` sí lo absorbe: la invocación de struct acepta una **expresión de path inline** (`Nivel(concat(&plana, sine(half_cycles=v+2, …), &plana))`), y es la vía para no declarar N paths casi iguales. Las listas (§5.1) **no admiten paths**, así que un `for` no puede indexarlos.

`str(x)` convierte un número a cadena en formato mínimo (entero sin punto, decimales solo si los hay), igual que la conversión implícita al concatenar con `+`; `str(x, decimals)` fija el número de decimales (`str(0.2*i, 2)` → `"0.40"`). Útil para armar etiquetas con texto+número (`text("Width " + str(0.2*i, 2))`); complementa el `decimals` de `numbers` (§13.2) cuando el número va suelto en una cadena, no en una serie de rótulos.

### 5.3 Alcance de variables (lectura, escritura y bloques)

Los bloques (`{ }`, `for`, `if`, cuerpo de struct) anidan **ámbitos**. Para variables el
modelo es el de C/JavaScript —los bloques **comparten** las variables del entorno que los
contiene; solo un nombre nuevo crea una variable propia—:

- **Lectura:** un nombre se busca en el ámbito actual y, si no está, se sube por los
  ámbitos que lo contienen. Un bloque **ve** las variables de afuera.
- **Escritura:** si la variable **ya existe** en un ámbito exterior, la asignación
  **modifica esa**, no crea una copia; si **no existe** en ningún ancestro, se crea local
  al bloque. MetaGráfica no tiene declaración explícita (`let`/`var`): la asignación es el
  único mecanismo, y esta es la semántica intuitiva.

La consecuencia útil: una variable con un **valor por defecto** afuera puede ajustarse
dentro de una rama `if`/`else` y **conservar** ese cambio al salir. No hace falta un `else`
para cubrir el caso restante:

```text
w = 8.9                       % valor por defecto
if i < 3 or i > 6    { w = 1.0 }   % modifica la w de afuera
else if i == 3       { w = 5.7 }
polyline { 0.76 y  w y }      % usa el w resultante (1.0, 5.7 u 8.9)
```

**Importante — variables vs. estado gráfico.** El alcance de las *variables* (esta
sección) es distinto del alcance del *estado gráfico* (`color`, `line_width`, `dash`,
transformaciones; §7.1). El **estado** se aísla por bloque con `gsave`/`grestore`: un
cambio dentro de un bloque **no** se filtra afuera. Una **variable**, en cambio, sí puede
alcanzar hacia afuera al escribirse (regla de arriba). Son dos modelos deliberadamente
distintos: el estado se restaura, los datos persisten.

**Variables dentro de structs (nuevo, aún por explorar).** El cuerpo de una struct es hijo
del ámbito **global**, no del llamador (§8, ámbito léxico). Con la regla de escritura, si
una struct asigna a un nombre que **ya es una variable global** del documento, la
**modifica**; si el nombre es nuevo, queda local a la invocación (y así las structs
recursivas son seguras: cada llamada tiene sus parámetros y sus locales). Esto significa
que una struct *puede* mutar globales —igual que un bloque cualquiera—. Es un modelo
simple y de un solo mecanismo; queda como **terreno por explorar** si conviene que el borde
de una struct sea además una *barrera de escritura* (locales por defecto, estilo función),
lo que exigiría distinguir el ámbito de struct del de bloque. Por ahora: un solo modelo.

---

## 6. Control de flujo

```text
for i = 0 to 9 {
    circle(0.3) { i * 1.0  5 }
}

for angle = 0 to 360 step 45 {
    dot(0.2) { 5 + 3*cos(angle)  5 + 3*sin(angle) }
}
```

- Las variables del `for` son locales al bloque y restauran el valor previo al salir.
- Loops anidados están soportados.
- El `step` puede ser negativo para contar hacia atrás.

### 6.1 if — condicionales

```text
if r > 2 {
    circle(r=r) { 5 5 }
} else {
    dot(r=0.2)  { 5 5 }
}
```

- Comparadores: `<`, `>`, `<=`, `>=`, `==`, `!=`; combinables con `and` / `or`, y negables
  con `not`. Precedencia, de menor a mayor: `or` < `and` < `not` < comparación — la de los
  lenguajes con operadores en palabra, así que `not a > b` es `not (a > b)`. `not` es
  prefijo y encadenable (`not not x`). Cero es falso; cualquier otro valor, verdadero.
- El bloque tiene ámbito léxico, como `for` y los bloques `{ }` (§7.1); la rama `else` es opcional.
- Una variable que ya existe afuera puede asignarse en una rama y **conservar** el cambio al salir (§5.3): permite fijar un valor por defecto y sobrescribirlo solo en ciertos casos, sin necesitar un `else`.
- Su uso principal es la condición de paro en structs recursivas (§8.1); también permite variantes de una struct según sus parámetros.

---

## 7. Estado, atributos y bloques

MetaGráfica mantiene un **estado gráfico** (color, grosor, relleno, tamaño de texto, transformación local…). En V3 el estado se fija con **sentencias**: una palabra clave seguida de su valor, sin paréntesis ni `=`, exactamente como `display_size` o `world_window` (§3). Una sentencia de estado aplica a **todo lo que le sigue** hasta que se cambie o hasta el final del bloque que la contiene.

```text
color "blue"
line_width 2
polyline { 0 0  10 0 }        % azul, line_width 2
polyline { 0 5  10 5 }        % azul, line_width 2 (el estado persiste)
circle(r=1) { 5 2.5 }         % azul, line_width 2
```

Esto recupera la sencillez imperativa de V1 (`LCOLOR`, `LWIDTH`, `FILL`, `TLLC`… eran sentencias) con una diferencia decisiva: **el estado tiene alcance léxico** (§7.1), así que no se fuga de forma global —que era la principal fragilidad de V1.

### 7.1 Bloques y alcance

Una llave `{ … }` en posición de sentencia abre un **bloque anónimo**: un ámbito que apila el estado gráfico al entrar y lo restaura al salir (un `gsave`/`grestore`). Los cambios de estado dentro del bloque **no se filtran** afuera. (Esto rige el **estado gráfico**; el alcance de las **variables** es otro —una asignación sí puede alcanzar hacia afuera—, ver §5.3.)

```text
color "black"
polyline { 0 0  10 0 }        % negro

{                             % ← bloque: acota el estado
    color "red"
    line_width 3
    polyline { 0 2  10 2 }    % rojo, line_width 3
    circle(r=1) { 5 2.5 }     % rojo, line_width 3
}

polyline { 0 4  10 4 }        % negro otra vez: el bloque restauró el estado
```

Es el mismo mecanismo con que `world_window` se redefine dentro de un bloque (§16) y con que las structs aíslan su estado (§8): un solo modelo de ámbito para todo.

### 7.2 Cambios a media secuencia

Como el estado son sentencias, puede cambiar **entre** primitivas dentro del mismo bloque —algo que un encabezado de atributos no permite sin anidar:

```text
{
    color "red"
    polyline { 0 0  10 0 }    % rojo
    color "blue"
    polyline { 0 2  10 2 }    % azul
}
```

### 7.3 Atributos de estado

Todos usan la forma `palabra valor(es)`. Los nombres **coinciden** con los argumentos por primitiva (§4), así que `color "red"` fija lo mismo que `polyline(color="red")`.

| Sentencia | Efecto | Sección |
|---|---|---|
| `color "c"` / `color "#RRGGBB"` / `color "none"` | color de trazo (`"none"` = sin trazo) | §4 |
| `fill "c"` / `fill "none"` | color de relleno sólido (`"none"` = sin relleno) | §4 |
| `line_width w` | grosor de línea en pt | §4.10 |
| `dash "dashed"` / `dash [10,2]` | patrón de línea | §4.10 |
| `cap "round"` / `join "round"` | extremos / uniones de trazo | §4.10 |
| `hatch a [g]` / `hatch "estilo" [g]` | relleno con trama: ángulo libre `a` o estilo nombrado (`"hatch"`/`"hatchback"`/`"crosshatch"`), paso `g` opcional; o `hatch_gap g` aparte | §4.11 |
| `font "italic"` | fuente base del texto (`"roman"`, `"italic"`, `"bold"`, …, §14.3) | §14.3 |
| `font_size h` | tamaño de texto en pt (mismo keyword en documento y en bloque) | §14 |
| `align "center"` / `valign "middle"` | alineación horizontal / vertical de texto | §4.8 |
| `translate dx dy` / `rotate a` / `scale s` / `shear hx hy` | transformación local | §11 |

`font_size` no es un keyword aparte del de documento (§3): es el **mismo**, y el `font_size` de documento es simplemente el del ámbito más externo. Igual que `world_window`, se redefine en un bloque anidado y se restaura al salir (§16). Un solo tamaño de texto, absoluto en pt.

### 7.4 Precedencia

De mayor a menor:

```text
argumento explícito en la primitiva  >  estado del bloque interno  >  estado del bloque externo
```

```text
{
    color "blue"
    line_width 2
    polyline { 0 0  10 0 }                  % azul, line_width 2
    {
        line_width 4
        polyline { 0 1  10 1 }              % azul (heredado), line_width 4 (redefinido)
        polyline(color="red") { 0 2  10 2 } % rojo (explícito gana), line_width 4
    }
}
```

### 7.5 Argumentos por primitiva: para casos puntuales

La forma con paréntesis (`polyline(color="green") { … }`, §4) sigue disponible y es la más cómoda cuando un atributo aplica a **una sola** primitiva. La distinción es clara:

- **paréntesis** → atributo adjunto a *esa* llamada;
- **sentencia** → estado que rige *hasta el fin del bloque*.

> **V3 elimina el bloque `with(...)`.** La sentencia de estado + el bloque anónimo lo sustituyen por completo: más expresivo (cambios a media secuencia, §7.2), consistente con `display_size` y `world_window` (global y anidado, §16), y una sola forma que aprender.

---

## 8. Structs

Una struct encapsula un bloque reutilizable. No dibuja nada hasta que se invoca. Recibe parámetros como una función. Hereda el sistema de coordenadas del punto de invocación. Por convención, los nombres de structs se **Capitalizan** para distinguirlos de los comandos (§1).

```text
struct Flecha(lx, ly, size=0.2) {
    polyline { 0 0  lx ly }
    polygon(fill="black") {
        lx-size*cos(atan2(ly,lx)+0.3)  ly-size*sin(atan2(ly,lx)+0.3)
        lx  ly
        lx-size*cos(atan2(ly,lx)-0.3)  ly-size*sin(atan2(ly,lx)-0.3)
    }
}

Flecha(3, 0)              % flecha horizontal
Flecha(0, 2, size=0.3)    % flecha vertical más grande
```

**Modificadores de colocación (`at`, `rotate`, `scale`).** Además de sus parámetros propios, toda invocación acepta tres argumentos nombrados opcionales:

- `at=(x, y)` — coloca el origen local de la struct en ese punto (dentro del sistema de coordenadas vigente, §11).
- `rotate=deg` — giro de la instancia en grados (positivo = antihorario), alrededor de su origen local.
- `scale=s` o `scale=(sx, sy)` — tamaño de la struct, uniforme o por eje. Es la misma `scale` de `place`/`repeat` (la matriz de estructura MTST, §11).

```text
Cuadro(scale=(0.2, 0.125), at=(0.1, 0.75))       % colocada y dimensionada
Flecha(3, 0, at=(5, 5), rotate=30)               % girada 30° y llevada a (5,5)
```

Se aplican en **orden canónico fijo `T·R·S`**: la struct se **escala** y **gira** en su marco local y luego se **traslada** a `at`. Por eso **el orden en que se escriben los modificadores no altera el resultado** —lo fija la spec, no el texto—, y equivalen exactamente a envolver la llamada en el bloque

```text
{ translate 5 5   rotate 30   Flecha(3, 0) }     % T·R, post-multiplicado (§11.1)
```

`at`/`rotate`/`scale` son, pues, azúcar del caso común "coloca una copia aquí, con este giro y tamaño". Cuando la composición **dependa del orden** de un modo distinto al canónico (escalar *después* de trasladar, cizallar, etc.), se usan las sentencias de transformación (§11.1) o `transform=` (§17), donde el orden de escritura *es* la composición. Los tres son **nombres reservados** de la invocación: una struct no puede declarar parámetros propios así llamados; como los parámetros declarados son posicionales, no chocan (`Flecha(3, 0, rotate=30)`).

### 8.1 Recursión

Una struct puede invocarse a sí misma, lo que habilita fractales, árboles y patrones autosimilares. Con parámetros propios y ámbito léxico la recursión es segura: cada invocación recibe sus argumentos y no depende de estado global. La condición de paro se expresa con `if` (§6.1):

```text
struct Rama(longitud, grosor) {
    if longitud >= 1 {                    % condición de paro
        polyline(line_width=grosor) { 0 0  0 longitud }
        {
            translate 0 longitud
            rotate 30
            Rama(longitud * 0.7, grosor * 0.7)
        }
        {
            translate 0 longitud
            rotate -30
            Rama(longitud * 0.7, grosor * 0.7)
        }
    }
}

Rama(5, 2)
```

Como red de seguridad, `max_depth n` (§18) limita la profundidad de expansión. Nótese que `exit` (§18) **no** sirve como condición de paro: detiene el parseo del archivo, no la expansión de una struct. *(V1: "una estructura puede contenerse a sí misma, permitiendo sorprendentes efectos" —artículo de 1991—, con `MAXDEEP n` como límite; era frágil porque la pluma y las matrices eran estado global.)*

### 8.2 Parámetros de tipo path

Un parámetro puede declararse de tipo **path** con el sigilo `&`, para que la struct reciba un path completo (no un número) y lo dibuje con él:

```text
struct Nivel(&onda, w = path_width(&onda)) {
    world_window 0 w -2 2
    polyline { 0 0  w 0 }
    bezier(&onda)
}

Nivel(&pw0)                          % w sale del propio path (default)
Nivel(&pw0, 6)                       % w explícito, pisa el default
fit(Nivel(&pw0), stretch=true) { 1.84615 0.44169  3.46615 1.48169 }
```

- El parámetro se liga en el ámbito del **llamador**, no en el de la struct: un path pasado como argumento resuelve sus propias referencias internas (p. ej. una variable usada dentro de un `sine(amplitude=...)`) en el sitio donde se escribió la llamada, no en el cuerpo que lo recibe.
- Un parámetro path **ensombrece** un path global del mismo nombre dentro del cuerpo — deliberado, no un accidente de resolución.
- El `world_window` local del cuerpo (§16) **puede** usar un parámetro de la struct (arriba, `world_window 0 w -2 2`).
- Pasar un número donde se espera un path (o viceversa), o invocar sin el argumento path requerido, es error de compilación con mensaje claro.
- **`fit` acepta la misma invocación** (§10.2): `fit(Struct(args), ...) { rect }`. `place`/`repeat` todavía no (nadie lo ha necesitado).

**`path_width(&p)`** — reduce un path a su extensión en x (`max x − min x`). Es el primer miembro construido de una familia reservada de accesores path→escalar (nombres **provisionales**, se refinan cuando estén todos):

```text
path_width(&p)              % construido — extensión en x
path_height(&p)             % reservado
path_x_bounds(&p, at_y=E)   % reservado
```

Exacto cuando el path es monótono en x (los extremos son entonces los puntos inicial y final, que sí están sobre la curva); sobre una Bézier genuinamente curva en esa zona, opera sobre el polígono de **control**, no sobre la curva — misma advertencia que `path_x_bounds_at_y` (motor, `splines.cpp`), el otro miembro de la familia.

---

## 9. Paths como objetos de datos

Los paths son objetos nombrados que representan trayectorias geométricas. Se usan en placements (§10) y como splines para dibujo.

```text
% Definición
path sinpi = bezier { 0 0  0.41 1.335  0.59 1.335  1 0 }

% Operaciones de álgebra de paths
path sin2pi  = tile(&sinpi, times=2)         % concatenar consigo mismo
path rev     = reverse(&sinpi)               % invertir dirección
path norm    = normalize(&sinpi)             % normalizar al rango [0,1]
path fitted  = fitrect(&sinpi) { 1 1  9 7 }  % escalar al rectángulo dado
path wave    = concat(&sinpi, &rev)          % unir dos paths

% Generadores como expresión de path: sine (§4.13) y smooth (§9.2)
path fall    = sine(half_cycles=1, phase=90, amplitude=1) { 0 0  1 0 }

% Dibujar un path nombrado
bezier(&sinpi)
```

> **Estado (2026-07-18):** implementados literales, `&ref`, `transpose`/`flip_x`/
> `flip_y`/`reverse`, `concat` **variádico y sin auto-reversión** (la semántica de
> abajo), y `sine`/`smooth` como generadores (caso guía: fig16-9, funciones de onda
> por medios ciclos de coseno con envolvente). `tile`/`normalize`/`fitrect` siguen
> reservados. `path_width(&p)` (§8.2) es una operación hermana: reduce un path a un
> escalar en vez de a otro path — sirve al default de un parámetro de struct
> (`w = path_width(&onda)`), no se usa suelta como las de arriba.

> Las dos operaciones "ajustar a rectángulo" tienen nombres distintos según su operando: `fitrect(&path)` produce un path (álgebra, esta sección); `fit(Struct)` (§10.2) coloca una struct. Igual con la repetición: `tile(&path)` concatena un path; `repeat(Struct)` (§17) repite una struct.

**Empalme en `concat` y `tile` (semántica de unión).** Al construir un path a partir de varios, cada operando subsiguiente se **traslada** para que su primer punto continúe desde el último punto del path acumulado. Este empalme automático es lo que permite armar una curva larga a partir de segmentos definidos en un dominio canónico (p. ej. los medios periodos seno/coseno de una biblioteca de curvas).

- **`join=true` (default).** Traslada cada pieza para pegar su inicio al final de la anterior. Es continuidad **C0** (de posición): la **tangente no se ajusta**, así que para uniones suaves los segmentos deben estar diseñados para encontrarse (como las Bézier de seno/coseno) o se usa `smooth` (§9.2) sobre los nodos combinados. `tile` siempre empalma así (repetir sin empalmar solo produciría copias superpuestas).
- **`join=false`.** Concatena las coordenadas **tal cual**, sin trasladar. Útil cuando los paths ya están posicionados en coordenadas absolutas y solo se quiere unir sus listas de puntos.
- **Sin auto-reversión.** El empalme pega el *inicio* de cada operando al final del anterior. Si un segmento debe recorrerse al revés, se envuelve explícitamente en `reverse(&seg)` (evita heurísticas de dirección frágiles).
- **Variádico.** `concat(&a, &b, &c, …)` une la secuencia en orden. Para tamaños distintos por segmento, se transforma cada operando antes (p. ej. con `fitrect`); `concat` solo une, no escala.

**Acumulación en un lazo (`path +=`).** `concat` es variádico pero de aridad fija en el fuente; para construir una curva cuyo **número de piezas depende de una variable** (un `for`), se acumula con `+=`:

```text
path w = &plana                       % semilla (o el primer '+=' la crea)
for k = 1 to n {
    amp = ...                          % cada pieza con su propio parámetro
    path w += sine(half_cycles=1, phase=ph, amplitude=amp) { 0 0  1 0 }
}
path w += &plana
```

- **`+=` evalúa YA** (al ejecutar la sentencia), a diferencia de `path w = <expr>`, que es **diferido** (guarda el árbol y lo evalúa al dibujar). Por eso `+=` puede leer variables del lazo: lee el path actual, le concatena la pieza en el ámbito vigente y **congela** el resultado. El empalme es el de `concat` (C0: traslada cada pieza al final de la anterior).
- Si el nombre **no existe** aún, `+=` lo **crea** con la pieza (permite sembrar sin un `=` previo). Un `path w = <expr>` dentro del `for` **re-siembra** en cada vuelta (la declaración es re-ejecutable).
- Es lo que permite curvas paramétricas pieza-a-pieza sin declarar N paths ni encadenar un `concat` de longitud fija: p. ej. las funciones de onda con **envolvente WKB** (amplitud por lóbulo) de `examples/franck_condon.mg`. *(Implementado 2026-07-19.)*

*(V1: `CTPT`/`RPPT` con la matriz `PT` y `SCPT` entre appends. La traslación de empalme la hacía el motor —`concat_paths`— pero asumía que el frente de cada segmento estaba en x=0 (precondición no documentada, corregida 2026-07-06 para alinear en ambos ejes). La continuidad C1 en las uniones queda como posible extensión futura, p. ej. `join="smooth"`.)*

### 9.1 spline — RETIRADA (2026-07-20)

`spline` estuvo reservada aquí con tres modos (`nodes=n`, `mode="bezier"`,
`mode="conic"`). **No se construye**, y sus dos huecos históricos se cierran.

**`spline` es `smooth` con otro nombre.** Catmull-Rom *pasa por* sus puntos de
control, así que la distinción que esta sección afirmaba —"en `spline` los puntos
son de control, en `smooth` son nodos"— no existe en la geometría. Las dos rutas del
motor (`splines_to_bezier` y `path_to_bezier`, ambas en `splines.cpp`) calculan la
misma curva; `smooth` (§9.2) es la mejor de las dos, porque recupera los extremos por
reflexión en vez de descartarlos. Un segundo nombre para la misma curva es justo el
problema de nomenclatura que §13.0 corrigió en los ejes.

**Las cónicas no se soportan, y no se pierde nada.** Una Bézier cuadrática es un
subconjunto *exacto* de la cúbica por elevación de grado (`c1 = q0 + ⅔(q−q0)`,
`c2 = q2 + ⅔(q−q2)`): cualquier cónica se emite sin pérdida por la ruta que ya existe.
Sus dos ventajas de 1988 —evaluación barata en Pascal/ensamblador y cónicas
*racionales* para círculos exactos— ya no aplican: la salida es EPS/PDF/SVG (los tres
hablan cúbica nativa) y MG tiene arcos y elipses reales (§4.3).

*(Historia: `$S 1` figura en el comentario de `Parser.cpp` pero **nunca se implementó**
—el `switch` solo atiende `n==0` y `n>1`—, así que las cónicas no sobrevivieron al paso
a EPS de 1991. Ni `SP` ni `$S` aparecen una sola vez en el corpus V1 congelado: en 25
años y tres libros publicados el comando no se usó nunca. La sección se conserva
numerada para no mover las referencias a §9.2 y siguientes.)*

**Lo que sí quedó pendiente de aquí** es el muestreo (`nodes=n`), que no es una forma de
*trazar* una curva sino de convertirla en *datos*: tiene clientes reales en `place` sobre
path (§10.1), `path_width` (§8.2) y `path_x_bounds_at_y`, que hoy operan sobre el polígono
de control y no sobre la curva. Su lugar sería una operación `sample(&p, n)` del álgebra
§9, no un modo de `spline`. Espera a la figura que lo pida.

### 9.2 smooth — Bézier suave a través de puntos

```text
path s = smooth { 0 0  2 3  5 1  8 4 }
```

Ajusta segmentos Bézier que pasan **exactamente** por los puntos dados, calculando las tangentes automáticamente: los puntos son *nodos* por los que la curva pasa, no puntos de control que la atraigan. Es la **única** forma de interpolación suave del lenguaje —`spline` se retiró por ser la misma curva con otro nombre (§9.1). La parametrización es **centrípeta** (Yuksel et al.), la que garantiza no formar cúspides ni autointersecciones. *(V1: `GNBZPATH name path`.)*

**Dos formas, como `sine` (§4.13).** `smooth` es a la vez expresión de path (arriba, para componer con `concat`/`fit`/`reverse`) y **primitiva de dibujo**:

```text
smooth { 0 0  2 3  5 1  8 4 }                      % dibuja
smooth(color="red", line_width=0.8) { 0 0  2 3 … } % con atributos §7.5
smooth(&nodos)                                     % suaviza un path del álgebra §9
```

**`smooth` y `bezier` son primitivas hermanas, y la diferencia es de quién calcula las tangentes.** En `bezier` el bloque son puntos de **control**: el autor las pone, y por eso puede hacer cosas que `smooth` no —un pico deliberado, tangentes discontinuas, tiradores asimétricos—; es estrictamente más expresiva. En `smooth` el bloque son **nodos** y el compilador emite los controles, garantizando a cambio que la curva pase por todos y empalme sin picos. Un `;` abre subtrayecto (§4) y cada uno se suaviza por separado.

*(Antes de 2026-07-20 solo existía la forma de expresión, así que dibujar exigía `bezier(smooth { … })` — que filtraba al documento el detalle de que `smooth` produce puntos de control. La forma primitiva la pidió el port de la Fig. del prob. ilustr. 3.2 de IMQ, que la necesitaba cuatro veces.)*

**Implementado (2026-07-18)** como expresión de path (§9). El motor (`path_to_bezier`) consume primer y último punto como ayudas de tangente; `smooth` extiende los extremos por **reflexión** (`p0′ = 2·p0 − p1`) para que la curva pase también por ellos con tangente natural — el dup manual que exigía `GNBZPATH` ya no hace falta (y duplicar daría distancia cero → NaN en la parametrización).

### 9.3 trail — path por avance

```text
path diagonal = trail(start=(0,0), count=8, translate=(1, 0.5))
```

Genera un path repitiendo un punto inicial y aplicando una transformación en cada paso. Se documenta junto con los generadores en §13.4. *(V1: `GNPATH n x y name`.)*

### 9.4 Path compuesto (compound)

Combina varias primitivas (líneas, arcos, curvas) en un **único contorno**, que puede rellenarse o trazarse como una sola región. Sin esto, primitivas heterogéneas (p. ej. una línea y un arco) no comparten path y no se pueden rellenar juntas.

```text
compound(fill="cyan") {
    polyline { 0 0  4 0 }
    arc(r=2, from=0, to=180) { 2 0 }
}
```

Las primitivas dentro del bloque no se cierran individualmente; sus extremos se unen en un solo path, que se rellena/traza según los atributos del `compound`. *(V1: `OPPT` … `CLPT`.)*

**Regla de relleno (`fill_rule`).** Un path compuesto puede encerrar subregiones (una dona, texto hueco). El argumento `fill_rule` decide qué queda dentro: `"non-zero"` (default, cuenta orientaciones de los subpaths) o `"even-odd"` (alterna dentro/fuera en cada cruce). Ambas reglas son nativas de PostScript y SVG, así que el backend solo las declara — el trabajo topológico lo hace el intérprete de salida.

```text
compound(fill="orange", fill_rule="even-odd") {
    circle(r=3)   { 5 5 }
    circle(r=1.5) { 5 5 }    % agujero: una dona
}
```

*(V1: el relleno topológico `FL` de 1991 —flood fill del área cerrada que contiene cada punto— se reemplaza por reglas de relleno vectoriales.)*

---

## 10. Placements — loops implícitos sobre locus geométricos

Esta es la característica distintiva de MetaGráfica. Un *placement* repite una struct a lo largo de un locus geométrico (línea, arco, path), rotando cada instancia para alinearla con la tangente local. Hay un solo comando, `place`, cuyo tipo de locus se infiere de la geometría del bloque; y `fit`, que ajusta una struct a un rectángulo.

### 10.1 place — a lo largo de un locus

`place(Struct, ...)` coloca la struct repetida a lo largo del locus definido en el bloque, rotando cada instancia con la tangente local. **El tipo de locus se infiere:**

- **2 puntos** → línea recta.
- argumentos de arco (`r`, `from`, `to`) → **arco** circular.
- **3+ puntos** o una referencia **`&path`** → trayectoria (path).
- override explícito con `along="line" | "arc" | "path"`.

```text
% Línea: marcas uniformes entre dos puntos, eje X local en la dirección de la línea
place(Tick, scale=0.3) { 0 0  10 0 }

% Arco: radio 5, barrido 90°, desde 0°, centro (5,5)
place(Tick, scale=0.3, r=5, from=0, to=90) { 5 5 }

% Path: en cada punto de la trayectoria, según la tangente
place(Tick, scale=0.2) { &sinpi }
place(Tick, scale=0.2) { 0.1 0.2  0.3 0.5  0.7 0.8 }
```

**Argumentos nombrados:**
- `scale` — tamaño de la struct (default 1.0).
- `shift` — desplazamiento del inicio como fracción del intervalo (default 0).
- `count` — número de instancias repartidas uniformemente sobre el locus. Alternativa a `gap`: dar `count` fija `gap = longitud/count`. *(línea/arco)*
- `gap` — espaciado entre instancias (default = escala). *(solo línea)*
- `both_sides` — colocar también en el lado opuesto (default false).
- `r`, `from`, `to` — radio y ángulos inicial/final en grados, **igual que `arc` (§4.5)**: el locus va de `from` a `to` (barrido `to−from`, positivo = CCW). *(solo arco)*

> ⚠️ **Abierto:** sobre un path, `place` instancia en cada punto del path. Falta definir el espaciado uniforme por longitud de arco (extender `gap=` a paths), útil para marcas cada *n* unidades a lo largo de una curva. Ver §19.

**V1:** `LNST` (línea), `ARCST` (arco), `DPST` / `&name path` (path).

> **`place` es para loci con extensión** (2+ puntos, arco o path): repite y orienta cada instancia según la tangente. Para colocar **una sola** instancia en un punto no se usa `place` —no hay locus ni tangente— sino la invocación directa con `at`/`scale` (§8). El *identifier placement* de V1 (`cuadro .1 .75`) traduce a `Cuadro(at=(.1, .75))`, no a `place` (que corresponde a `LNST`/`ARCST`/`DPST`).

### 10.2 fit — ajustar una struct a un rectángulo

No repite: transforma el sistema de coordenadas de la struct para que ocupe el rectángulo dado.

```text
fit(Panel) { 0 0  10 8 }                % escala uniforme, contenido centrado
fit(Panel, stretch=true) { 0 0  10 8 }  % deforma para llenar el rectángulo exacto
```

Por default preserva la proporción de la struct: escala uniforme al mayor tamaño que cabe en el rectángulo, con el contenido centrado (misma semántica *meet* de §3.1). Con `stretch=true` se deforma para ocupar el rectángulo exacto, que a veces es lo deseado.

**El orden de las esquinas importa: refleja.** El bloque `{ p1 p2 }` mapea la caja de la struct a esas dos esquinas *en ese orden*; si un eje queda decreciente —p. ej. `{ .5 0  0 1 }`, con la x de `p1` mayor que la de `p2`— el contenido se **refleja** en ese eje. Es un idiom compacto para espejar un panel: `curvas3.mg` lo usa para armar un patrón de difracción simétrico a partir de media curva. *(Motor: `StructureRectangle::draw` vía `Matrix::to_rectangle`, que admite escala negativa.)*

**Invocación paramétrica.** Si la struct tiene parámetros, `fit` acepta la misma sintaxis de invocación que una llamada directa: `fit(Struct(args), stretch=…) { rect }` (§8.2) — típicamente para pasar un parámetro path (`fit(Nivel(&pw0), stretch=true) { … }`). Los modificadores de colocación `at=`/`rotate=`/`scale=` (§8) **no** se aceptan dentro de esa invocación —competirían con la matriz que arma el propio `fit`— y son error de compilación si se escriben ahí.

**V1:** `PWST` / `PVPT` (siempre deformaban al rectángulo; el orden de esquinas ya reflejaba).

### Cuadro comparativo V1 → V3

| V1 | V3 | Semántica |
|---|---|---|
| `LNST sc [sh [n [gap]]] p1 p2` | `place(s, scale, shift, count\|gap) { p1 p2 }` | Struct a lo largo de línea (V1 `n` → `count`) |
| `ARCST sc r da ai [sh [n]] p` | `place(s, scale, r, from, to) { c }` | Struct a lo largo de arco (V1 `da`=barrido → `to = ai+da`) |
| `DPST name` / `&name path` | `place(s, scale) { &path }` | Struct a lo largo de path |
| `PWST p1 p2` / `PVPT name p1 p2` | `fit(s, stretch=true) { p1 p2 }` | Struct ajustada a rectángulo (V1 deformaba; el default V3 preserva proporción) |
| `RPPT name n` | `path x = tile(&name, times=n)` | Repetir path (álgebra) |
| `CTPT name` + ... `CLPT` | `path x = concat(...)` | Concatenar paths |

> **Resuelto:** el repetidor V1 `RPST n` se mapea a `repeat(...)` (§17), que avanza la posición con `step` (§12) y acumula una transformación sobre la struct. Cuando no hay acumulación, un `for` loop es equivalente.

---

## 11. Transformaciones y sistemas de coordenadas

MetaGráfica transforma cada coordenada a través de una pila de matrices homogéneas 3×3. El motor mantiene **cinco matrices** con roles distintos. La mayoría se manejan de forma implícita; V3 expone explícitamente la transformación local mediante las sentencias de estado `translate`/`rotate`/`scale`/`shear` (§7/§11.1).

| Matriz | Rol | Cómo se controla en V3 |
|---|---|---|
| **Local** (MTLC) | Transformación local a un bloque | sentencias `translate`/`rotate`/`scale`/`shear` (§11.1) |
| **Structure** (MTST) | Rotación/escala de una struct colocada | placements (automático, §10) o `transform` alrededor de la llamada (§11.2) |
| **Plume** (MTPP) | Avance de la pluma entre elementos generados | `step(...)` (§12) |
| **Path** (MTPT) | Se aplica al reutilizar un path nombrado | álgebra de paths (§9), normalmente implícito |
| **Repeat** (MTRS) | Transformación acumulada en repeticiones de struct | `repeat(...)` (§17) |

### 11.1 Transformaciones locales

Las transformaciones locales se fijan con **sentencias de estado** (§7), acotadas por el bloque que las contiene:

- `translate dx dy` — desplazamiento en unidades de usuario.
- `rotate deg` — rotación en grados (positivo = antihorario).
- `scale s` o `scale sx sy` — escala uniforme, o por eje.
- `shear hx hy` — cizalladura.

```text
{
    translate 5 5
    rotate 30
    rectangle(fill="cyan") { 0 0  3 2 }   % rectángulo llevado a (5,5) y rotado 30°
}
```

**Orden de composición:** cada sentencia **post-multiplica** la matriz local, así que las transformaciones se componen **en el orden en que se escriben** (igual que las secuencias `TLLC`/`RTLC`/`SCLC` de V1, y que la clase `Matrix` del motor). Arriba, `translate 5 5` seguido de `rotate 30` da **T·R**: la geometría se rota y luego se lleva a (5,5); escribir `rotate 30` antes de `translate 5 5` daría **R·T**, otro resultado. El autor controla el orden con la secuencia.

**Alcance (scope):** léxico al bloque; se restaura al salir (`gsave`/`grestore`), como todo el estado (§7.1). Los bloques anidados **componen** con el del padre.

**Equivalencia V1 → V3:**

| V1 | V3 |
|---|---|
| `TLLC dx dy` | `translate dx dy` |
| `RTLC a` | `rotate a` |
| `SCLC sx sy` | `scale sx sy` |
| `SHLC hx hy` | `shear hx hy` |
| `IDLC` | (cierre del bloque `{ }`) |

> Las palabras `translate`/`rotate`/`scale`/`shear` aparecen en dos papeles: como **sentencia de estado** (`rotate 30`, sin paréntesis, esta sección) y como **constructor de valor** de matriz dentro del argumento `transform=` de `repeat` (`rotate(30) scale(0.95)`, con paréntesis, §17). El primero es imperativo y scoped; el segundo es una matriz que se pasa como dato.

### 11.2 Transformaciones sobre structs (MTST)

Los placements (§10) ya fijan la matriz de estructura automáticamente para alinear cada instancia con la tangente local. Para transformar manualmente una struct al invocarla, basta encerrar la llamada en un bloque cuyo estado incluya las transformaciones deseadas, que componen sobre la matriz de estructura:

```text
{
    rotate 45
    scale 1.5
    Flecha(3, 0)        % flecha rotada y escalada
}
```

No se necesita sintaxis adicional: MTST se deriva de la composición léxica de las transformaciones activas en el punto de invocación.

> **Dos regímenes, sin conflicto.** Para el caso común —una instancia colocada, girada y dimensionada— la invocación acepta los modificadores `at`/`rotate`/`scale` (§8), que son azúcar de este bloque con **orden canónico fijo `T·R·S`** (su orden de escritura es irrelevante). El bloque explícito de arriba, en cambio, respeta el **orden de las sentencias** (§11.1) y es el que se usa cuando la composición **depende del orden** (recuérdese que las matrices no conmutan: `T·R ≠ R·T`).

---

## 12. Posición de pluma y dibujo relativo

> ⚠️ **No implementado en V3 (verificado 2026-07-15).** `moveto`/`step` no existen en el parser (`moveto(0,0)` falla como "struct no definida") y un bloque de posición vacío no dibuja: `text("x") { }` no estampa nada, en vez de usar el punto actual. Los generadores §13 y `repeat` §17 se hicieron **autocontenidos** (`at=`/`advance=`) y ningún ejemplo del corpus lo necesita, así que la sección describe un diseño **pendiente**, no el estado del compilador. Mientras tanto, la posición va siempre explícita en el bloque `{ }`.

Además de las coordenadas absolutas, MetaGráfica mantiene una **posición de pluma** (el *punto actual*). Sirve para (a) secuencias de texto, (b) los generadores (§13) y (c) el dibujo incremental.

- **`moveto(x, y)`** — fija la posición de pluma en coordenadas de usuario. *(V1: `XYPP x y`.)*
- **`step(translate=(dx,dy), rotate=deg, scale=(sx,sy))`** — define la **transformación de avance**: cómo se mueve la pluma de un elemento al siguiente en los generadores y en el texto secuencial. *(V1: la matriz `MTPP`, p. ej. `TLPP dx dy`, `RTPP a`.)*

La transformación de `step` se **acumula**: en cada avance se aplica al punto actual. Como puede incluir rotación y escala, permite disposiciones radiales o en espiral, no solo lineales.

> El comando de pluma `step(...)` (una llamada) no debe confundirse con la palabra clave `step` del `for` loop (§6, infija) ni con el argumento `step=` de `axis` (§13.5, intervalo de valor). El contexto los distingue.

### 12.1 Texto secuencial

`text("...")` (sin bloque) dibuja en la posición de pluma y la avanza un `step`. Es la forma relativa de `text("...") { x y }`, que es absoluto. *(V1: `DT`.)*

```text
moveto(0, 0)
step(translate=(0, -0.6))
text { "Primera línea" }
text { "Segunda línea" }     % colocada un step más abajo
text { "Tercera línea" }
```

### 12.2 Relación con los generadores

Los generadores (§13) toman `moveto` como punto de partida y `step` como avance entre elementos. Definir la pluma y el avance una vez evita repetir coordenadas en cada marca, número o repetición.

> **Principio de diseño.** `moveto`/`step` son el mecanismo de *bajo nivel* heredado del motor (la matriz de pluma). Para las tareas comunes —ejes y retículas— se recomiendan las construcciones declarativas `axis` y `grid` (§13.5–§13.6), que esconden por completo la pluma. Además, los generadores (§13) aceptan `at=(x,y)` y `advance=(dx,dy)` en la propia llamada, evitando el trío `moveto`/`step`/generador cuando basta un avance por traslación. La meta de V3 es que el modelo de pluma sea un detalle de implementación, no la API que el usuario maneja a diario.

---

## 13. Generadores y ejes

Los generadores producen series de elementos repetidos a partir de la posición de pluma (§12) y la transformación de avance `step`. Son la base para construir ejes, retículas y escalas sin escribir cada coordenada a mano.

**Dos niveles.** `ticks`, `numbers` y `trail` (§13.1–§13.4) son los generadores *de bajo nivel*: exponen la pluma y el `step` directamente, y dan control total. Por encima de ellos, `axis` y `grid` (§13.5–§13.6) son construcciones *declarativas* que orquestan la pluma internamente: un humano describe el rango y la decoración, no la mecánica de avance. **Para ejes y retículas comunes, usa `axis`/`grid`; recurre a los generadores de bajo nivel solo para casos no estándar.**

### 13.0 Anatomía de una gráfica: nomenclatura

Esta sección fija **un nombre por cada parte de una gráfica**, y los reserva aunque la
parte todavía no exista. Es normativa: el resto de §13 y §13.7 la usan.

```text
                        Absorción del CO₂                    ← title  (encabezado del plot)
        a_B
         ↑                                        ┌────────────────┐
     1.0 ┤        ▟█████▙                         │ ▨▨  522 cm     │ ← legend
         │      ▟█████████▙                       │ □□  261 cm     │   (entries)
     0.5 ┤    ▟█████████████▙                     └────────────────┘
         │  ▟█████████████████▙
     0.0 ┼──┬────┬────┬────┬────┬──→  λ(µm)       ← label  (nombre del EJE + unidades)
         ▲  12   14   16   18   20  ▲
         │  └──────────┬─────────┘  └── axis      (la línea)
         │        tick labels
         └── ticks (las marcas)
```

| elemento | matplotlib | asymptote | **MG** |
|---|---|---|---|
| encabezado del plot | `title` / `suptitle` | — | **`plot(title=)`** ⚠️ reservado |
| nombre del eje (+ unidades) | `xlabel` / `ylabel` | `Label` | **`axis(label=)`** |
| los números/cadenas de las marcas | tick labels | `ticks` | **`axis(tick_labels=)`** |
| las marcas (malla regular) | ticks | `ticks` | `axis(ticks=)`, `tick_size=` |
| **valor notable** (umbral, retorno, nivel) | `axvline`/`axhline` | — | **`plot { rule(…) }`** ⚠️ reservado (§13.8) |
| la línea del eje | spine | axis | `axis`, `extend=` |
| retícula | grid | `grid` | `grid=`, `grid()` |
| leyenda | `legend` | `legend` | **`legend`** (§13.9; forma explícita implementada, `rule` pendiente) |
| tabla inserta | `table` | — | **`table`** ⚠️ reservado (§13.10) |

**Marcas y valores notables son cosas distintas** (§13.8): la malla se lee, el valor notable
señala. No los metas en el mismo canal — `ticks_at=[…]` es el antipatrón, con evidencia.

#### Las tres reglas

**1. Las unidades van DENTRO de la cadena del label. No hay `units=`.** Es lo que hacen
matplotlib, asymptote, gnuplot y ggplot, y no por pereza: separarlas obliga a la
herramienta a fijar una política tipográfica que **no es universal** — el SI pide
`λ / µm`, los libros de física escriben `λ(µm)`, otras disciplinas `[µm]`. Cualquier
`units=` tendría que elegir una y estorbar a las demás. La cadena la escribe el autor:
`label="$\lambda(\mu/rm)$"`.

**2. `title` es el encabezado del PLOT, no el nombre del eje.** Es el uso universal del
término, y el nombre queda reservado para cuando `plot` lo implemente. El nombre del eje
es `label` — lo que matplotlib llama `xlabel`/`ylabel`.

**3. `tick_labels` cubre sus tres modos con un argumento**, como ya hace `grid=`:

```text
tick_labels=true            % (default) automáticos: números formateados por decimals/strip_zero
tick_labels=false           % sin rótulos (eje esquemático)
tick_labels=["Ene","Feb"]   % explícitos, uno por marca, en orden  ⚠️ reservado
```

El tercer modo es la razón por la que **no** se llama `numbers=`: los rótulos de marca no
siempre son números. `numbers()` (§13.2) sigue siendo el generador numérico de bajo nivel;
son niveles distintos y no colisionan.

#### Estilo: dos familias, un prefijo cada una

`label_*` estiliza el **nombre del eje**; `tick_label_*`, los **rótulos de marca**. El
prefijo `tick_*` ya designa lo que vive sobre las marcas (`tick_size`), así que
`tick_label_*` extiende la convención en vez de inventar otra.

| nombre del eje | rótulos de marca |
|---|---|
| `label`, `label_at`, `label_gap`, `label_font`, `label_size` | `tick_labels`, `tick_label_gap`, `tick_label_align`, `tick_label_valign`, `tick_label_font`, `tick_label_size` |

`label_at=` (`"center"` default / `"start"` / `"end"`) elige entre las **dos convenciones
reales**, ambas presentes en el corpus: centrado a lo largo del eje (matplotlib/asymptote,
lo que hace fig2-3) o al extremo (estilo de libro, lo que quieren fig4-5, fig6-4 y
fig_polybar). El default es `"center"`: es lo que espera quien llega de matplotlib.

> ⚠️ **Colisión de nombre a resolver:** `rule` (§13.8) también quiere un `label_at=`, pero con
> otro juego de valores (`"axis"`/`"legend"`). Mismo nombre, dominios distintos — defendible
> (son objetos distintos, como `align`), pero es un olor. Decidir antes de implementar.

> ✅ **Renombre HECHO (2026-07-16).** Antes, el compilador usaba `axis(title=)` para el nombre
> del eje y `axis(labels=)` + `label_*` para los rótulos de marca: los dos nombres que cualquiera
> alcanza primero significaban **otra cosa** que en el resto del ecosistema, y `title` ocupaba el
> nombre del encabezado del plot. Se hizo ahora porque V3 no tiene usuarios externos y el churn
> era ~20 usos en 4 archivos; dentro de veinte figuras, no.
>
> **Los nombres viejos fallan en compilación** apuntando al nuevo (`checkRenamedAxisArgs`,
> `parserv3.cpp`): `title`, `title_font`, `title_size`, `title_gap`, `labels`, `label_align`,
> `label_valign`. Así no se aceptan dos vocabularios en silencio.
>
> ⚠️ **La trampa, que sigue viva para quien lea código viejo:** `label_font`/`label_size`/
> `label_gap` son válidos **antes y después con distinto significado** (antes = rótulos de marca;
> ahora = nombre del eje). No pueden fallar, y el parser no puede verlo. **La red golden fue la
> única guardia**: el renombre es *puro* —no cambia comportamiento— así que se verificó exigiendo
> que los 57 goldens quedaran **byte-idénticos** (`ok=57`, sin re-bendecir). Cualquier sitio mal
> portado habría movido un golden. Es la técnica a repetir en el próximo renombre.

### 13.1 ticks — marcas de eje

```text
moveto(0, 0)
step(translate=(2, 0))
ticks(6, mark=(0, 0.3))

% forma compacta equivalente: posición y avance en la propia llamada
ticks(6, mark=(0, 0.3), at=(0, 0), advance=(2, 0))
```

Genera `count` marcas; cada marca es un segmento dado por el vector `mark=(dx,dy)`, dibujado en la posición de pluma, que avanza con `step` en cada repetición. *(V1: `TICKS n dx dy`, con la pluma y `MTPP`.)*

- `count` — número de marcas (posicional).
- `mark=(dx,dy)` — vector que define longitud y dirección de cada marca.
- `at=(x,y)` / `advance=(dx,dy)` — opcionales: fijan la posición inicial y el avance en la propia llamada, en lugar de `moveto`/`step`. `advance` es la traslación de posición entre marcas (equivale a `step(translate=…)`).

### 13.2 numbers — etiquetas numéricas

```text
moveto(0, -0.5)
step(translate=(2, 0))
numbers(from=0, by=2, count=6, decimals=0)   % 0  2  4  6  8  10

% forma compacta equivalente (posición y avance en la llamada):
numbers(from=0, by=2, count=6, decimals=0, at=(0, -0.5), advance=(2, 0))

% con decoración textual literal (envuelve al número ya formateado por decimals):
numbers(from=1, by=1, count=5, decimals=0, prefix="Pattern ")   % Pattern 1 … Pattern 5
numbers(from=10, by=10, count=5, decimals=0, suffix=" K")       % 10 K  20 K … 50 K
```

Genera `count` etiquetas numéricas, empezando en `from` e incrementando `by` en cada paso, con `decimals` decimales, en el estilo de texto actual (§14), posicionadas en la pluma y avanzando con `step`. Opcionalmente, `at=(x,y)` y `advance=(dx,dy)` fijan la posición inicial y el avance en la propia llamada, sin `moveto`/`step`. *(V1: `GNNUM i0 inc n decimals`.)*

Los argumentos opcionales `prefix` y `suffix` son cadenas **literales** que se anteponen y posponen al número ya formateado por `decimals` (`prefix="Pattern "` → `Pattern 3`; `suffix=" K"` → `20 K`). No son un lenguaje de formato: el número lo sigue gobernando `decimals`, y prefix/suffix solo lo envuelven —ejes ortogonales que componen sin conflicto. Con ellos, una etiqueta que *contiene* un número (un rótulo `Pattern n`, una marca con unidades) deja de necesitar una `text` por elemento. `axis` (§13.5) los propaga a sus `numbers` internos, para rotular marcas con unidades.

> Nota: hay dos ejes independientes. `by` es el incremento del **valor numérico** impreso; `advance` (o `step`) es la traslación de la **posición** entre etiquetas. Se usa `advance` —no `step`— en la llamada, precisamente para no confundirlos.

### 13.3 Eje completo, a bajo nivel (ejemplo)

Combinando una polilínea con `ticks` y `numbers` se obtiene un eje X de 0 a 10 con marcas y etiquetas cada 2 unidades. Este es el desglose explícito; en la práctica se prefiere `axis` (§13.5), que produce exactamente esto:

```text
polyline { 0 0  10 0 }                       % línea del eje

moveto(0, 0)
step(translate=(2, 0))
ticks(6, mark=(0, -0.3))                     % marcas hacia abajo

moveto(0, -0.7)
step(translate=(2, 0))
numbers(from=0, by=2, count=6, decimals=0)   % etiquetas
```

### 13.4 trail — generar un path por avance

```text
path diagonal = trail(start=(0,0), count=8, translate=(1, 0.5))
```

Genera un path nombrado con `count` puntos, empezando en `start` y aplicando la transformación dada (`translate`, `rotate`, `scale`) en cada paso. *(V1: `GNPATH n x y name`, con la matriz `MTPT`.)* Útil para crear trayectorias regulares reutilizables en placements (§10).

> Nota: `GNBZPATH` (ajustar segmentos Bézier a través de puntos arbitrarios) corresponde a `smooth` en el álgebra de paths (§9.2).

### 13.5 axis — eje declarativo

`axis` describe un eje completo (línea + marcas + rótulos + nombre del eje opcional) en una sola sentencia. El bloque `{ }` da los dos extremos físicos del eje; los argumentos describen el rango numérico y la decoración. Internamente es un `place` a lo largo de una línea (§10.1) que coloca marcas y etiquetas calculadas desde el rango — la pluma y el `step` se orquestan solos.

> **Nomenclatura: §13.0.** `label` es el **nombre del eje** (el `xlabel`/`ylabel` de matplotlib),
> **no** el encabezado del plot — ese nombre (`title`) está reservado para `plot`. Los rótulos de
> marca son `tick_labels`/`tick_label_*`. Los nombres viejos (`title=`, `labels=`, `label_align=`…)
> **fallan en compilación** apuntando al nuevo; ver la trampa del renombre en §13.0.

```text
% Eje X: de (0,0) a (10,0), valores de 0 a 10, marca y etiqueta cada 2
axis(from=0, to=10, step=2, ticks="out", tick_labels=true, label="x / cm") { 0 0  10 0 }

% Eje Y: el mismo constructo, distinto par de extremos
axis(from=0, to=8, step=1, ticks="out", tick_labels=true) { 0 0  0 8 }
```

**Argumentos:**

| Arg | Default | Significado |
|---|---|---|
| `from`, `to` | — | valores numéricos en el extremo inicial y final |
| `step` | — | intervalo de valor entre marcas/etiquetas |
| `start` | `from` | valor de la **primera** marca/etiqueta; sirve para omitir el origen (`start=0.1`) |
| `ticks` | `"out"` | dirección de las marcas: `"out"`, `"in"`, `"both"`, `"none"`, `"grid"` (relativas a la línea; `"grid"` las alarga a través del campo, §13.6) |
| `tick_size` | pequeño | longitud de las marcas, **cantidad física en pt** (§3.2) |
| `tick_labels` | `true` | mostrar los rótulos de marca (§13.0). `false` = eje esquemático |
| `decimals` | `0` | decimales en las etiquetas |
| `tick_label_gap` | `4` | distancia de los rótulos de marca a la línea, **cantidad física en pt** |
| `field` | ventana | (solo `ticks="grid"`) largo perpendicular del barrido; default = rango de la ventana |
| `label` | — | **nombre del eje** (texto, admite markup matemático §14); rotado en ejes verticales (§19) |
| `label_at` | `"center"` | posición del nombre **a lo largo** del eje: `"center"` (matplotlib/asymptote, fig2-3) o `"start"`/`"end"` (estilo de libro: el texto arranca/termina en el extremo, fig4-5) |
| `label_font`, `label_size` | ambiente | fuente/tamaño del **nombre del eje**, para cuando difieren de los rótulos de marca |
| `label_gap` | según `label_at` | distancia física (pt) del eje a la **línea base** de su nombre. Default: `tick_label_gap+20` centrado (libra la fila de rótulos de marca) / `tick_label_gap` al extremo (no la libra: va más allá a lo largo del eje) |
| `tick_label_font`, `tick_label_size` | ambiente | fuente/tamaño de los **rótulos de marca** (hermanos de `label_font`/`label_size`) |
| `tick_label_align`, `tick_label_valign` | auto por lado | override de la alineación de los rótulos de marca (`"left"`/`"center"`/`"right"`; `"baseline"`/`"top"`/`"middle"`/`"bottom"`) |
| `line_width`, `color` | ambiente | estilo de la **línea del eje y sus marcas**, acotado al eje (desacopla el eje del estado del contenido, necesario dentro de `plot` §13.7) |
| `extend` | `0` | alarga **solo la línea** más allá de sus extremos, en unidades del eje. Escalar = ambos lados; lista `(lo hi)` = antes de `p1` / después de `p2`. Reproduce el "sobrante estilo libro" |
| `scale` | `"linear"` | `"log"`: eje logarítmico. `from`/`to` son **valores** (no exponentes, `>0`); marcas mayores en cada `step` décadas (entero); rótulos `10^n` (n=0 → `"1"`) |
| `minor` | `false` | (solo `scale="log"`) marcas menores sin rótulo en `2·10ⁿ…9·10ⁿ` |
| `strip_zero` | `false` | (ejes lineales) quita el cero inicial de las etiquetas: `"0.30"→".30"`, `"-0.30"→"-.30"` |

> **`tick_size` y `tick_label_gap` son físicos (pt), no de mundo.** Como `line_width`,
> `font_size` y los marcadores (§4.6), no los deforma la ventana ni el `stretch`: en un
> marco de datos anisótropo (§13.7) las marcas y etiquetas quedan a la misma distancia
> física en ambos ejes, sin compensar la escala por eje. El lado ("out") se deriva de la
> tangente de la línea: eje ~horizontal → abajo, ~vertical → izquierda.
>
> **`ticks="grid"`** convierte las marcas del eje en líneas que barren el campo, a los
> mismos valores del `step`: el efecto de la fig 2.3 original (donde la retícula eran
> "ticks largos") sin un `grid` aparte. El largo perpendicular se hereda de la ventana, o
> se fija con `field=`.

Las **marcas y sus rótulos** van en `start, start+step, …` hasta `to` (`start` default = `from`). Los **rótulos de marca** heredan el estado de texto vigente (`font`, `font_size`, `align`, §7.3) y admiten override propio (`tick_label_font`/`tick_label_size`), igual que el **nombre del eje** (`label_font`/`label_size`), porque suelen diferir —p. ej. números en itálica y nombre en romana, como en fig2-3.

#### Abierto: ¿el `label` cuelga de la LÍNEA o de la CAJA?

**Hoy cuelga de la línea del eje.** matplotlib hace lo contrario: `set_ylabel` es un método del
*Axes* (la caja), no del spine — si centras el spine izquierdo, el `ylabel` **se queda en el borde
de la caja**. Las dos convenciones coinciden siempre que el eje corra sobre el borde, que es el
caso de todo el corpus salvo un panel; **divergen solo con `base=`** (la trampa de §13.7).

La medición que lo dejó ver, para no rederivarla (fig4-5, tres paneles; `V(x)` es la ordenada):

| panel | eje y en x= | `V(x)` en x= | Δ vs **eje** | Δ vs **borde de caja** |
|---|---|---|---|---|
| a | **15.5** (interior, `base=0`) | 1.0 | **−14.5** | −4.5 |
| b | 39.8 | 34.5 | −5.3 | −5.3 |
| c | 80.0 | 74.5 | −5.5 | −5.5 |

Respecto del eje es incoherente; respecto de la caja, coherente: **el libro ancla la ordenada a la
caja**. Y en la *misma figura* ancla la abscisa a la **línea** — el rótulo `x` del panel c está en
y=24.3, pegado a su eje interior (y=25), no al fondo de la caja (y=10). No es descuido: el extremo
derecho de un eje x interior es buen lugar; el tope de un eje y interior cae **sobre la curva**.

**No se cambió, y la razón no es conservadurismo: el anclaje a la caja no rescata su propio caso.**
Con `label_at="end"` anclado a la caja, los tres `V(x)` quedarían en la esquina superior de *su*
caja (y = 37, 38.5, 38.5); el libro los quiere a y=**35, los tres** — a una altura común, que es
decisión de página y ningún anclaje da. Seguirían a mano. Súmale que haría falta un
`label_rotate=false` sin más usuarios.

**Lo decide la primera figura con un eje interior cuyo rótulo ESTORBE de verdad** (regla del
proyecto: no especular sin ejemplo). Si ese día se elige la caja, revisar que no mueva fig2-3 /
fig6-4 / fig_polybar, donde eje y borde coinciden y no debe cambiar un píxel.

La orientación de marcas y etiquetas se deriva de la dirección de la línea (la lógica de tangente del motor de placements), así que un eje horizontal, vertical o inclinado se escriben igual, cambiando solo los extremos del bloque. Aquí `step` es el **intervalo de valor** (como en el `for` loop, §6), no el `step` de avance de pluma (§12).

*(V1: combinación manual de `TICKS` + `GNNUM` + `LNST`/`PL`; ver el desglose de bajo nivel en §13.3.)*

> **Estado de implementación (act. 2026-07-15).** Casi toda la "propuesta para estudiar"
> original ya está en la tabla de arriba, verificada contra fig2-3 y fig6-4:
>
> **Implementado:** `scale="log"` (con `minor`), `strip_zero`, `extend`, `ticks="in"`,
> auto-alineación de rótulos de marca por lado (con override `tick_label_align`/`tick_label_valign`),
> `line_width`/`color`/`tick_label_font`/`tick_label_size` por-eje, `label_gap` y nombre de eje centrado/rotado.
>
> **Renombre a §13.0 HECHO (2026-07-16)**, verificado por goldens byte-idénticos (`ok=57`).
> **`label_at` HECHO (2026-07-16)**: absorbe los 3 rótulos `x` de fig4-5 (antes `text()` a mano).
> Los rótulos de ordenada de fig4-5 (`V(x)`) NO se absorben y no por el centrado: van a una
> altura común en los 3 paneles (decisión de página) y en a) el eje V es interior — son
> mobiliario de página, no el nombre del eje.
>
> **Pendiente (Fase 3, `plan_plot.md`):**
> - `step` **automático** si se omite (familia 1/2/5·10ᵏ, ~5–8 marcas) y `decimals` **derivado
>   del paso** — quitan los dos parámetros más tediosos del caso común.
> - `format="%.1f"` / notación científica — formato de etiqueta más allá de `decimals`/`strip_zero`
>   (con validación estricta del especificador).
> - `at=v` — posición del eje cruzado (dibujar el eje X a la altura del cero).
> - **`tick_labels=[…]`** — rótulos de marca no numéricos (§13.0, modo 3).
>
> **Diferido:** `edge="bottom"|"top"|"left"|"right"` con `from`/`to` heredados del marco — **no**
> se implementa suelto: requiere §16 (ventanas anidadas), que no existe. Se pliega dentro de
> `plot` (§13.7), donde `box=` cumple ese papel y los ejes se piden con `xaxis`/`yaxis`.

### 13.6 grid — retícula

`grid` dibuja una retícula sobre el rectángulo dado por el bloque (esquina inferior-izquierda, esquina superior-derecha), con líneas verticales y horizontales a intervalos regulares.

```text
grid(xstep=2, ystep=1, color="lightgray") { 0 0  10 8 }
grid(xstep=2, ystep=2, dash="dotted")      { 0 0  10 8 } % retícula punteada
```

**Argumentos:** `xstep` / `ystep` (intervalos en unidades de usuario), más los atributos de estilo comunes (`color`, `line_width`, `dash`). Útil como fondo de una gráfica antes de trazar datos y ejes.

> **Pendiente — retícula por eje.** El `grid=` de `plot` (§13.7) barre **los dos** ejes; no hay
> forma de pedir solo las horizontales. Es lo más común en gráficas de barras: `figure_02` usa
> `ax.grid(axis='y', linestyle='--', alpha=0.7)` en sus cinco paneles — retícula **solo en y**,
> punteada. Falta el selector (`grid="y"`, o `grid=` por-eje en `xaxis`/`yaxis`). Aquí, en el
> `grid()` suelto, se obtiene omitiendo un `step`.

### 13.7 Graficar datos: el marco de datos

> **Estado: `plot { }` IMPLEMENTADO** (2026-07-15, ver "Azúcar `plot { }`" al final de esta
> sección y `plan_plot.md` Fase 4; fig2-3 y fig6-4 portadas). **Corrección importante:** las
> **ventanas anidadas (§16) NO existen** en V3, así que el marco de datos **no** se realiza con
> una `world_window` anidada como sugiere el ejemplo conceptual de abajo. Se realiza con el
> mecanismo de `fit` (§10.2): matriz envolvente datos→caja si es lineal, o mapeo puntual de
> coordenadas si algún eje es logarítmico (log no es afín → ninguna matriz lo expresa). Los
> marcadores de datos son **físicos** (`dot`/`marker`, §4.6), inmunes al `stretch`. El ejemplo
> con `world_window` anidada de abajo se mantiene como **ilustración del concepto**, no como la
> sintaxis real (que es `plot(...)`).

**El problema.** En V1, para meter una curva en una caja de ejes el autor **pre-escala los
datos a mano**. En fig2-3 la temperatura relativa va de 0 a 1.0, pero el autor la multiplicó
×10 para ocupar la caja física `0..10` y luego etiquetó `0.1..1.0` por separado (`GNNUM 0.1
0.1 10 1`). Ese doble trabajo —escalar la geometría y rotular el rango real— es exactamente lo
que un eje declarativo debe eliminar.

**La idea: reutilizar la ventana anidada (§16) como *marco de datos*.** Un plot no necesita un
mecanismo nuevo: un bloque con su propia `world_window` **en unidades de datos** ya define el
mapeo datos→caja física. Dentro de ese bloque, la curva y los puntos se dan **en coordenadas de
datos** (sin ×10), y los ejes decoran los bordes heredando el rango de la ventana.

Dos particularidades de un plot frente a una figura geométrica:

1. **Es anisótropo por naturaleza.** Los dos ejes miden magnitudes sin relación (temperatura vs.
   capacidad), así que el marco usa `stretch=true` (escala independiente por eje, §3.1/§16), no
   la escala isométrica *meet* del resto del lenguaje. Un marco de datos **activa `stretch` por
   default**.
2. **El rango de datos ≠ el rango de dibujo.** La ventana de datos puede tener un margen respecto
   a la caja física (para que la curva no toque el marco), como el `WW -1 11 -1 7` de fig2-3.

**fig2-3 con marco de datos** (compárese con las ~40 instrucciones estilo ensamblador del V1):

```text
display_size 12 8
world_window -1 11  -1 7            % lienzo físico exterior

{                                   % ── marco de datos ──
    world_window 0 1  0 6  stretch=true   % (xmin xmax ymin ymax) EN DATOS

    % Curva: en coordenadas de datos, sin pre-escalar ×10
    line_width 0.8                                  % V1 LWIDTH 4 → 4×0.2 = 0.8 pt
    dash "dashed"
    % Nota: concatenar estos dos segmentos para que sea una sola curva
    polyline { 0 0  0.076 0 }                       % tramo recto inicial
    bezier   { 0.076 0  0.266 0.025  0.179 5.07  0.998 5.45 }

    % Puntos experimentales (en datos). dot es marcador FÍSICO (§4.6): la posición
    % la transforma el marco, el tamaño no → redondos pese al stretch.
    dot(0.1, color="black") { 0.160 0.73  0.194 1.14  0.210 1.36  0.944 5.55 }

    % Ejes: rango heredado de la ventana; solo se da la decoración.
    % 'edge' elige el borde y omite el bloque de extremos (§13.5, propuesta).
    axis(edge="bottom", step=0.1, title="relative temperature")
    axis(edge="left",   step=1,   title="heat capacity")
}
```

Aquí `axis` no repite `from`/`to`: los toma de la ventana del marco (`0..1` en x, `0..6` en y).
La temperatura relativa se grafica **tal cual** (0.160, 0.194, …), y las etiquetas salen `0.1,
0.2, …, 1.0` solas — el ×10 desaparece.

**Marcadores en un marco `stretch`.** Como el marco es anisótropo por definición, los tamaños
que son **cantidad de mundo** se deforman: un `circle(0.1)` saldría elíptico, y una struct
colocada con `place`/`scale` también (su tamaño es de mundo). La regla es: dentro de un marco de
datos, **la posición va en datos pero el tamaño de los marcadores y del texto se queda físico**.
El primitivo `dot` (§4.6) ya cumple esto por construcción (posición transformada, tamaño en
dispositivo), así que es el marcador correcto para puntos de datos; los símbolos de la familia de
marcadores (`plan_marcadores.md`) deben heredar esta misma propiedad física, no el escalado de
mundo de `place`.

**Relación con `fit` (§10.2).** El marco de datos y `fit` son el **mismo mecanismo** a distinta
granularidad: `fit(Curva, stretch=true) { caja }` hace con una **struct reutilizable** (que trae
su propia ventana en datos) lo que la ventana anidada hace *inline*. Para una curva de un solo
uso, la ventana inline es más ligera; para una curva reutilizable o un panel repetido (fig4-10),
`fit` sobre una struct es lo natural.

### Azúcar `plot { }` — IMPLEMENTADO (2026-07-15)

`plot` empaqueta el marco de datos en un constructo tipo `compound`: **un solo bloque** de
contenido (sentencias en unidades de datos), los rangos como argumentos, la caja física como
`box=`, y los ejes con `xaxis`/`yaxis` dentro del bloque. `plot` **transforma las coordenadas de
su contenido** (mecanismo de `fit`, no ventana anidada); los ejes se interceptan y se dibujan en
coords exteriores heredando `from/to/scale`.

```text
plot(x=(0.30,0.50), y=(1e-15,1e5), yscale="log",
     box=(0.6,0.7, 5.79,5.4), grid=gray(0.5)) {
    polyline { 0.30 3e4  0.50 2e-15 }        % recta de ajuste, EN DATOS
    fill "black"
    dot(1.5) { 0.31 8e2  0.33 1.1e1 }        % puntos EN DATOS (marcador físico)
    text("$Po{^292}$") { 0.315 2e3 }         % anotación anclada a DATOS (cae gratis)
    xaxis(line_width=0.2, step=0.05, strip_zero=true, title="$E^{-1/2}$ (MeV)$^{-1/2}$")
    yaxis(line_width=0.2, minor=true, title="…")
}
```

**Argumentos de `plot`:**

| Arg | Default | Significado |
|---|---|---|
| `x`, `y` | `(0,1)` | rango de datos `(from,to)` de cada eje (tupla) |
| `xscale`, `yscale` | `"linear"` | `"log"` para eje logarítmico (usa el mapeo puntual) |
| `box` | ventana vigente | rectángulo físico `(x0,y0, x1,y1)` en world coords al que se mapean los datos |
| `grid` | — | retícula de fondo: `true`=gris default, un color, o ausente=sin retícula. Se auto-alinea con las marcas: toma `step` **y `start`** de `xaxis`/`yaxis` |

Dentro del bloque, `xaxis`/`yaxis` son `axis` (§13.5) con el lado y el rango predefinidos por
`plot` — aceptan toda la decoración de §13.5 (`step`, `title`, `strip_zero`, `line_width`, …) pero
**no** llevan bloque `{ }` ni `from`/`to`. El contenido admite cualquier sentencia (polyline,
bezier, dot, text con ancla en datos, control de flujo); `box=` omitido cubre "el plot es toda la
figura", `box=` explícito cubre paneles (fig4-10).

**`base=v` — dónde cruza el eje.** Por default cada eje corre sobre el borde de `box` (el de
abajo para `xaxis`, el izquierdo para `yaxis`). `base=v` lo mueve al valor `v` **en unidades de
datos del eje perpendicular**: `xaxis(base=0)` pone la abscisa sobre `y=0` aunque el rango baje a
negativos, y `yaxis(base=0)` centra la ordenada en el origen. Es lo que pide la figura de
potenciales de libro (fig4-5: eje `V` centrado en el oscilador, eje `x` sobre `V=0` en el
potencial efectivo). `v` fuera del rango deja el eje fuera de la caja, que es legítimo (no hay
clipping); en un eje log `base=` exige `v>0`. La **retícula** de `grid=` no se mueve: barre la
caja completa, cruce donde cruce el eje.

> ⚠️ **Trampa: `base=` se lleva el rótulo del eje con él.** El `label` cuelga de la **línea** del
> eje (§13.5), no de la caja, así que `yaxis(base=0, label="V")` manda el nombre del eje **al
> centro del panel, encima de los datos**, sin avisar. No es hipotético: es por lo que fig4-5 pone
> sus tres `V(x)` a mano en coords de ventana. Con `base=` y `label` juntos, revisa dónde cayó.
> Los rótulos de MARCA no tienen el problema (van pegados a sus marcas, que sí siguen al eje a
> propósito), ni el `label` de un eje sobre el borde de la caja, que es el caso normal.

Un eje sin marcas ni rótulos —la línea desnuda de una figura esquemática— es
`xaxis(ticks="none", labels=false)`.

**Limitaciones (con error claro):** bajo escala **log**, colocar structs (`invoke`/`place`/`fit`
de struct/`repeat`) da error ("aún no soportado": su matriz de draw-time no compone con el mapeo
no afín); `grid()`/`ticks()`/`axis()` **pelados** en el contenido también (usa `grid=` o
`xaxis`/`yaxis`); rango de datos ≤0 en un eje log. No hay clipping a la caja (el contenido puede
rebasarla, como la recta de ajuste de fig6-4). El `text()` de título horizontal-arriba (estilo
libro, como `λ⁻¹(s)` de fig6-4) va manual, porque `yaxis(title=)` rota el título a lo largo del eje;
lo mismo el título **al extremo** del eje (`V(x)`, `x` de fig4-5), porque `title=` lo centra.

### 13.8 Marcas: malla regular y valores notables (`rule`) — reservado

**Las marcas de un eje son DOS conceptos distintos, y `from`/`to`/`step` solo expresa uno.**

| | **malla regular** | **valor notable** |
|---|---|---|
| para qué | *leer* coordenadas del eje | *señalar un hecho* de los datos |
| de dónde sale | del rango (aritmética) | de la física (un umbral, un retorno, un nivel) |
| cuántos | muchos, uniformes | pocos, arbitrarios |
| qué rótulo lleva | el número, autoformateado | un **nombre** (`x₁`) o prosa (`ΔT₁ < 0 Confirmed ash`) |
| estilo | uniforme | propio (rojo punteado), a veces con línea guía y punto |

**El corpus ya lo demostraba y la spec no lo veía.** `fig4-5` es una figura de **100% valores
notables y 0% malla**: sus siete `axis` van con `ticks="none", labels=false` —existen solo para
dibujar la raya— y los puntos de retorno `x₁`, `x₂`, `x'₁` se construyen a mano, cada uno con
tres sentencias sueltas más un `text()` colocado a ojo. `fig4-10` hace lo mismo en el otro eje
(`dash "dashed"  Niveles()` = los niveles de energía E₁…E₄). Al otro extremo, `fig2-3`, `fig6-4`
y `fig_polybar` son 100% malla y cero notables. **La mitad del corpus reconstruye a mano un
concepto que el lenguaje no tiene.**

> ⚠️ **Por eso `ticks_at=[…]` (una lista de posiciones colgada de `axis`) es el movimiento
> equivocado:** vuelve a fundir los dos conceptos en un solo canal. La evidencia está en
> `figure_02` (matplotlib), que hace exactamente eso —inyecta los umbrales en `set_xticks()`—
> y paga el precio: rótulos encimados (`0.0  1.0` pegados en el panel a) y **código de borrado
> de marcas cercanas** para que sobrevivan. Eso no es el modelo funcionando, es el modelo
> peleando.
>
> 💡 **Y ahí está el premio del corte.** Ese borrado de colisiones es, palabra por palabra, lo
> que el autor escribió a mano en Python (`ticks_limpios = [t for t in ticks_actuales if
> all(abs(t - p) > distancia_segura for p in puntos_a_marcar)]`). matplotlib **no puede**
> hacerlo por él porque no distingue malla de notable: para él las dos son `ticks`. Un lenguaje
> que sí los distingue puede resolver la colisión solo —suprimir el rótulo de malla vecino a un
> `rule`— porque sabe cuál cede. **El corte no solo evita el antipatrón: absorbe el código que
> el antipatrón obliga a escribir.**

**`rule` — el valor notable, hijo del `plot`** (decidido 2026-07-16):

```text
plot(x=(-0.95,0.95), y=(-0.1,0.85), box=(5.5,10, 25.5,37)) {
    polyline { a1 Ea  a2 Ea }                                  % nivel de energía
    rule(x=a1, to=Ea, label="$x_1$", dash="dashed", marker=dot(2.5))
    xaxis(ticks="none", tick_labels=false)
}
```

- **`x=v` / `y=v`** — el valor, en unidades de datos; dice de qué eje es hijo.
- **`to=v`** — extensión. Default: toda la caja (el `axvline` de matplotlib). `to=Ea` = de la
  línea del eje hasta `Ea` (la línea de cortesía de fig4-5, que va de la curva al eje).
- **`label=`** + **`label_at=`** — `"axis"` (default) pone el nombre donde iría el rótulo de
  marca de ese valor; `"legend"` lo manda a la leyenda (§13.9). Es la única diferencia entre
  el `x₁` de fig4-5 y el `ΔT₁ < 0 Confirmed ash` de figure_02: **el mismo objeto, distinto
  destino del nombre** — corto al eje, prosa a la leyenda.
- **estilo propio** (`color`, `dash`, `line_width`) y **`marker=`** opcional (el `dot` de fig4-5).

**Es hijo del `plot`, no del `axis`.** El eje no sabe dónde está la curva, así que no puede dar
la extensión `to=Ea`; y la leyenda es del plot, no del eje. **Costo aceptado de esa elección:**
el rótulo de un `rule` **no** hereda gratis el acomodo automático del eje (auto-align por lado,
`tick_label_gap`) — con `label_at="axis"` el plot tiene que preguntarle al eje dónde pondría el
rótulo de ese valor. Es la misma cuenta que ya hace para sus marcas; es acoplamiento, no
maquinaria nueva.

**Nombre provisional.** `rule` es la palabra tipográfica para una línea, y encaja con el caso de
figure_02 (cruza el panel entero). Encaja peor con fig4-5, donde no cruza nada: es un valor
anotado con una línea corta de cortesía. Sin resolver.

**Abierto — un `rule` puede querer DOS rótulos.** En `figure_02` los umbrales llevan a la vez su
**número en el eje** (`0.0`, `1.0`, para leer *dónde* caen) y su **prosa en la leyenda** (`ΔT₁ < 0
Confirmed ash`, para saber *qué significan*). No son el mismo texto ni van al mismo sitio, así que
`label=` + `label_at=` no basta: harían falta dos canales (¿`tick=true` para el número, `label=`
para la prosa?). fig4-5 no lo pide (su `x₁` es un solo rótulo). Decidir con más figuras.

### 13.9 legend

Hay **dos fuentes** de entradas, y son distintas:

**1. Los `rule` (§13.8) — automática y limpia, sin implementar todavía.** En `figure_02` hay
correspondencia **1:1 exacta** entre entradas de leyenda y `axvline`: la leyenda no describe
series, **describe valores notables**. Un `rule` con `label_at="legend"` se autocolecciona sin
ambigüedad — lleva su nombre y su estilo, y la muestra ES su línea. Espera a que `rule` exista.

**2. Las series — explícita, IMPLEMENTADA (2026-07-17, con `fig1.mg`).** Aquí no hay
correspondencia que autocoleccionar: las dos series de `fig_polybar` son **tres** `polybar`
(trama sin contorno → blanco opaco → contorno solo, §4.12). Un colector ingenuo daría tres
entradas para dos series, y la muestra de "522 cm" —que es trama *y* contorno— no es ninguno de
los tres items por separado. El autor declara la entrada y su muestra:

```text
plot(...) {
  ...
  legend(at="top-right", margin=10, sample_width=20, gap=5, font_size=8) {
    entry("Adem (1967)/Garduño and Adem (1988)")  { polyline { 0 0.5  1 0.5 } }
    entry("Houghton (1971)/Holton (1979)")         { marker(3, shape="circle", color="black") { 0.5 0.5 } }
    entry("Peixoto and Oort (1984)/Holton (1979)") { marker(size=4, shape="cross") { 0.5 0.5 } }
  }
}
```

> 💡 **Evidencia de campo contra la autocolección universal.** El propio `figure_02.py` **tiene
> disponible** la autocolección de matplotlib (`axvline(…, label=…)` + `ax.legend()`) y **la
> esquiva**: construye proxies `Line2D([0],[0], color='red', ls='--', lw=1.0, label=…)` —líneas
> *sin datos*, que existen solo para ser la muestra— y repite el estilo, que ya estaba en el
> `axvline`. Motivo: `sns.histplot` inyectaría su propia entrada, y el autor quiere mandar sobre
> cuáles aparecen y en qué orden. La "repetición de estilo" que parecía el defecto de la leyenda
> explícita es un precio que los usuarios de matplotlib **ya pagan por elección**.

**`legend` es hijo de `plot`**, como `xaxis`/`yaxis` (§13.7): se ejecuta en coordenadas
**exteriores** (la caja física), nunca mapeada por datos — su tamaño físico no se deforma con
`plot log` ni con el stretch, y se dibuja AL FINAL (encima de contenido y ejes).

**La muestra de cada `entry` es un bloque arbitrario**, no un enum `sample="line"/"marker"`: es
la caja unitaria 0..1 que se ajusta con el `fitMatrix` MEET-centrado de `fit`/colocación de
struct (`stretch=false`) al rectángulo `sample_width`×`sample_height` de esa fila — preserva
forma, así que un `marker(shape="circle")` sale círculo, no elipse, aunque la fila no sea
cuadrada.

**`at=`** (`"top-right"`/`"top-left"`/`"bottom-right"`/`"bottom-left"`) ancla una esquina de la
caja del plot, con `margin=` (pt) de inset. El lado (`"left"`/`"right"`) fija el borde de la
**columna de muestras**, no el del texto: el compilador **no puede medir el ancho de una
cadena en parse-time** — los tres backends lo calculan en DRAW-TIME con su propio mecanismo
(EPS `stringwidth`, PDF `HPDF_Page_TextWidth`, SVG `text-anchor`), y ninguno lo expone antes.
`"...-left"` arranca la muestra en el margen y el texto **crece a la derecha** (`align="left"`,
natural); `"...-right"` **termina** la muestra en el margen y el texto **crece a la izquierda**
desde ahí (`align="right"`, nativo del backend) — así ningún caso necesita medir texto.

Estilo: `margin`/`sample_width`/`sample_height`/`gap`/`row_gap`/`font_size`, todos **físicos**
(pt), como `tick_size`/`label_gap` (§13.5). Sin marco/fondo por ahora (`fig1.mg` no lo
usa); `border=`/`fill=` se añaden cuando una figura los pida (mismo criterio que el resto del
lenguaje: la abstracción espera a la figura que la necesita).

### 13.10 table — reservado (sin diseñar)

El recuadro Mean/SD/Min/Max de `figure_02` **se parece a una leyenda y no lo es**: es
`ax.table()`, otro constructo. Nombre reservado para no dejar que se lo trague `legend`. Su
contenido es **dato derivado** (`data.mean()`, `data.std()`), lo que abre una pregunta de
alcance sin responder: ¿MG calcula estadísticas, o el `.mg` recibe los números ya hechos? El
resto del lenguaje dice lo segundo (`polybar` recibe bins, no datos crudos).

---

## 14. Texto matemático

El texto es procesado por un módulo independiente (`text_parser`) que se hereda **sin cambios** de V1. El compilador V3 extrae el string del bloque `{ "..." }` y lo pasa a `parse_text()`, que produce una `TextLine` con los chunks ya descompuestos por fuente, tamaño y nivel de script.

### 14.1 Markup del string

El contenido del string soporta el siguiente markup, procesado en parse time:

> 🐞 **Trampa resuelta (2026-07-16) — `FN_NOFACE` no puede recibir un bit de estilo.** El modelo
> de caras compone por OR de bits (`FN_ITALIC=1`, `FN_BOLD=2`), pero el centinela "hereda la cara
> ambiente" es **`FN_NOFACE = -1`**, que ya tiene *todos* los bits prendidos: `-1 | FN_ITALIC` es
> `-1`, así que **`/i` y `/b` se tragaban en silencio** sobre una cara heredada. Solo afecta a los
> modificadores de BIT; `/r`, `/s`, `/c`, `/$` **asignan** una cara y nunca lo tuvieron.
>
> Con `FN_NOFACE` solo llegan los textos que heredan cara: los rótulos de `axis`/`numbers`/`grid`
> (§13). Por eso se escondió tanto — `fig6-4` usa `/i` en su nombre de eje y funciona, porque va
> **dentro de `$…$`** y el modo math *asigna* `FN_TIMES_ITALIC` antes; `fig4-5` lo usa pelado y
> salía romano. **Fix** (`change_font_face`, `text_parser.cpp`): un modificador de bit sobre
> `FN_NOFACE` resuelve contra la cara por default — exactamente lo que ya hacía un `text()` normal,
> que hornea `FN_DEFAULT`. Así `axis(label="/ix")` sale itálico igual que `text("/ix")`.

#### Multilínea — especificado, sin implementar

Un `text()` es hoy **un solo renglón**: `text_parser.cpp` no rompe línea ni tiene interlínea.

**Diseño (decidido 2026-07-16, pendiente de implementar):** **`/n`** rompe el renglón. No es un
escape del lexer —las cadenas de MG **no** tienen escapes tipo C (§14.1)—: llega a `parse_text`
como los dos caracteres que es, igual que `/i` o `\alpha`. El motor lo resuelve emitiendo **un
`Text`/`TextLine` por renglón**, desplazados en vertical; la interlínea sale de `font_size`. Sin
motor de layout: es apilar renglones, no componer párrafos.

> ⚠️ **`\n` NO puede usarse, y conviene saber por qué** (medido 2026-07-16, antes de
> implementar). `\` **consume todo lo alfabético que sigue** —así se leen `\alpha` y `\nabla`—,
> así que `text("uno\ndos")` no ve `\n` + `dos`: ve el símbolo **`ndos`**, y hoy avisa
> `symbol name unknown ndos`. Tratar `\n` como caso especial rompería `\nabla` (quedaría salto
> de línea + `abla`), y condicionarlo a "solo si no sigue letra" haría que `text("uno\n dos")`
> funcionara y `text("uno\ndos")` no — una trampa peor que no tenerlo. `/` no tiene el problema:
> consume **un** carácter, y `/n` hoy no significa nada (se imprime literal), así que el hueco
> está libre y es inequívoco.

Aplica a **todo texto**, no solo a `axis`: `text()`, `label=` y la futura `legend` lo heredan.

Lo pide el nombre de eje de `figure_02`, con las unidades en su propio renglón —
`'$\Delta T_1$\n(BT 10.3 - 12.3 $\mu$m)'` — lo que de paso **confirma** la regla de §13.0: las
unidades van dentro de la cadena del label, no en un `units=`. Ninguna figura del corpus lo pide
todavía, así que **no bloquea beta** (§22.7).

#### Modo matemático

```text
$...$    activa la fuente CMMI (Computer Modern Math Italic) y el modo matemático
```

Dentro de `$...$` operan los sub/superíndices `_`/`^` (ver abajo). **Fuera** de `$...$` esos caracteres son **literales**, de modo que prosa como `line_width` o `x^y` no se convierte en subíndice/superíndice inesperado; para un script hay que entrar a modo matemático.

```text
text("$\alpha + \beta = \gamma$") { 5 3 }
text("Radio $r = 2\pi / \lambda$") { 2 7 }
```

#### Símbolos griegos y matemáticos

```text
\nombre    inserta el símbolo correspondiente; selecciona la fuente automáticamente
```

Letras griegas minúsculas: `\alpha \beta \gamma \delta \epsilon \zeta \eta \theta \iota \kappa \lambda \mu \nu \xi \pi \rho \sigma \tau \upsilon \phi \chi \psi \omega`

Variantes: `\varepsilon \vartheta \varpi \varrho \varsigma \varphi`

Letras griegas mayúsculas: `\Gamma \Delta \Theta \Lambda \Xi \Pi \Sigma \Upsilon \Phi \Psi \Omega`

Operadores y símbolos: `\infty \partial \nabla \int \sum \prod \pm \times \div \cdot \leq \geq \neq \approx \equiv \sim \in \subset \supset \rightarrow \leftarrow \Rightarrow \Leftarrow \forall \exists`

Funciones: `\sin \cos \tan \cot \sec \csc` (se renderizan en fuente romana recta)

#### Sub y superíndices

Solo activos **dentro de modo matemático** (`$...$`); fuera, `_` y `^` son literales.

```text
^x       superíndice de un carácter
^{...}   superíndice de un grupo
_x       subíndice de un carácter
_{...}   subíndice de un grupo
```

```text
text("$x^2 + y^2 = r^2$") { 3 5 }
text("$E_n = -13.6 / n^2$ eV") { 3 3 }
text("$\psi_{nlm}(r,\theta,\phi)$") { 3 1 }
```

Los scripts reducen el tamaño de fuente al 70% y son anidables: `$x^{2^n}$`.

#### Cambios de fuente en línea

El prefijo `/` seguido de un código cambia la fuente para el resto del string (o hasta el siguiente cambio). Se aplican dentro o fuera de modo matemático.

| Código | Fuente |
|---|---|
| `/r` | Times Roman |
| `/i` o `/e` | Times Italic |
| `/b` | negrita (combina con la fuente actual) |
| `/s` | Sans-serif (Helvetica) |
| ~~`/g`~~ | **RETIRADO** (2026-07-20) — era Symbol con el griego en las posiciones de las letras |
| `/c` o `/t` | Courier |
| `/$` | CMMI (math italic) |

**`/g` se retiró** al migrar los símbolos a Latin Modern Math (P1, 2026-07-20). Existía de
cuando Symbol era la única fuente disponible: fijaba esa cara y mandaba **bytes crudos**,
contando con su codificación completa —griego en las posiciones de las letras, y de paso sus
dígitos y signos—. La vía canónica del griego es TeX, `\lambda`, que **nombra** el glifo en
vez de codificarlo en una posición de byte, y funciona igual en los tres backends. Al quitarlo,
`FN_SYMBOL` solo se alcanza por `\comando` y el font Symbol desaparece del lenguaje. Un `/g`
en un documento viejo cae al aviso `font face style unknown`.

```text
text("/bTítulo en negrita") { 2 5 }
text("Normal, /itálica, /rnormal otra vez") { 2 3 }
```

#### Agrupación

`{...}` delimita el alcance de sub/superíndices:

```text
text("$e^{i\pi} + 1 = 0$") { 3 5 }
text("$\sum_{n=0}^{\infty} a_n x^n$") { 3 3 }
```

### 14.2 Arquitectura: por qué no cambia en V3

`parse_text()` es una función pura: toma un string UTF-8 y una fuente base, devuelve un `GraphicsItem` (`Text` o `TextLine`). No depende del parser del `.mg` ni del Display. En V3:

- El parser V3 extrae el string de `text("...") { ... }`
- Lo pasa a `parse_text()` con la fuente base del contexto
- Obtiene un `GraphicsItem` listo para dibujar con cualquier Display

**Lo único que cambia** entre V1 y V3 es cómo se especifica la fuente base y el tamaño desde el archivo `.mg` (antes: `TSTYLE`, `TXSI`; ahora: argumentos de `text`).

### 14.3 Argumentos de `text`

```text
text("...") { x y }                              % posición en el bloque; fuente y tamaño del contexto
text("...", font_size=12) { x y }                % tamaño en puntos
text("...", font="italic") { x y }               % fuente base: "roman", "italic", "bold",
                                                 %   "italic-bold", "sanserif", "courier"
text("...", align="center", color="red") { x y } % alineación horizontal y color
text("...", valign="middle") { x y }             % alineación vertical
```

La fuente base determina el punto de partida del markup interno; los `/b`, `/i`, etc. dentro del string son relativos a ella.

La alineación vertical se controla con `valign`: `"baseline"` (default), `"top"`, `"middle"`, `"bottom"`. Es ortogonal a `align` (horizontal), así que ambos se combinan y se fijan por separado, también como estado (§7.3).

---

### 14.4 Texto: repertorio de la fuente, no Latin-1

**El techo del texto corrido es el repertorio de glifos de la fuente, no una
codificación.** Una base-14 de PostScript trae ~315 glifos (Times-Roman): además
de Latin-1, tiene las comillas tipográficas, las rayas, los puntos suspensivos,
viñetas, dagas, ‰, ™, €, œ… Hasta el 2026-07-20 todo eso se **descartaba con
aviso**, no porque faltara el glifo sino porque `ISOLatin1Encoding` no sabía
nombrarlo.

**Resuelto** dándoles posición propia. Los 27 caracteres de esa lista viajan en
**ranuras 1..27** (controles C0, que en texto corrido no aparecen nunca; la 0 se
deja libre porque es el terminador de las cadenas C), y **cada backend traduce la
ranura a lo suyo**, que es justo lo que este apartado pedía:

| backend | traducción |
|---|---|
| EPS | nombre de glifo, en un `/MGTextEncoding` derivado de `ISOLatin1Encoding` |
| PDF | su byte en `WinAnsiEncoding` (CP1252 tiene 24 de los 27) |
| SVG | el codepoint Unicode, en UTF-8 nativo |

La tabla vive en **un solo sitio** (`kExtraTextGlyphs`, `text_parser.cpp`) con el
codepoint, el nombre PostScript y el byte CP1252; EPS genera su `/Encoding` desde
ella. Los anchos de avance salen de los **AFM** de las base-14, así que la
alineación y el avance son exactos, no estimados. Cobertura: `examples/texto.mg`.

**Lo que sigue sin caber es lo que la fuente NO tiene:** griego en texto corrido
(en modo math sí, §14), cirílico, CJK. Eso no es cuestión de codificación —
exigiría **embeber una fuente de texto Unicode** en EPS, como ya se hace con
Latin Modern Math para el math (§14.2). Es una decisión aparte, y de otro tamaño:
el subset math son 27 KB; una LM Roman completa, cientos.

*(Los símbolos matemáticos nunca pasaron por aquí: se escriben `\comando` y se
resuelven vía `map_symbol`/`map_tex_cmmi` → LM Math.)*

## 15. Importar archivos

```text
include "bzsinepaths"    % carga bzsinepaths.mg
include "estilos"
```

**V1:** `INPUT nombre`

Las structs y paths definidos en el archivo incluido quedan disponibles en el archivo que lo importa.

**Alcance (scope).** Las structs y paths son **globales**: una struct definida en un archivo incluido es visible en el archivo padre y en cualquier `include` posterior. Reglas:

- El `include` debe **preceder** al uso de las structs o paths que define.
- **No se permite redefinir** una struct con un nombre ya existente (es un error).
- No hay namespaces por archivo: los nombres comparten un espacio global, así que conviene prefijar los nombres de bibliotecas reutilizables (p. ej. `EjesMarca`, `FlechaSimple`).

---

## 16. Coordenadas locales y ventanas anidadas

Además de la `world_window` global (§3), una ventana de coordenadas puede **redefinirse dentro de un bloque o una struct**, estableciendo un sistema de coordenadas local que se apila y se restaura al salir.

```text
world_window 0 24 0 16            % ventana global

struct Detalle() {
    world_window 0 100 0 100      % coordenadas locales dentro de la struct
    circle(r=10) { 50 50 }        % usa el sistema local (0..100)
}
```

La ventana local rige hasta el final del bloque; al salir se recupera la ventana previa. Esto permite que una struct reutilizable trabaje en su propio rango de coordenadas cómodo, independiente de dónde se la coloque. *(V1: `WW` admite anidamiento; el parser apila y restaura la ventana.)*

La ventana anidada se mapea **isométricamente** a su región, igual que la global (§3.1): declarar una ventana local nunca deforma el contenido de la struct. `stretch=true` y `align=` aplican igual que en la ventana global.

> Relación con §11: la `world_window` remapea las coordenadas de usuario al cuadrado unitario interno; las transformaciones `transform` (§11) operan **después** de ese remapeo.

> Si el bloque es el cuerpo de una struct con parámetros, la `world_window` local puede referirse a ellos (§8.2): `struct Nivel(&onda, w) { world_window 0 w -2 2  … }`.

---

## 17. Repetición de estructuras (repeat)

`repeat` coloca la struct indicada `count` veces, avanzando la posición con `step` (§12) y, opcionalmente, **acumulando** una transformación sobre la struct en cada repetición. Esto es lo que permite espirales, patrones de crecimiento y disposiciones cuasi-fractales que un `for` loop expresa de forma menos compacta.

```text
repeat(Hoja, count=8, transform=scale(0.85), at=(0, 0), advance=(1.5, 0))
% 8 instancias de Hoja, cada una 0.85× de la anterior, avanzando 1.5 en x
```

- `count` — número de repeticiones (nombrado).
- `scale=s` / `scale=(sx,sy)` y `rotate=deg` — tamaño y giro **fijos** de cada instancia (matriz de estructura MTST), idénticos a los modificadores de `place` y de la invocación directa (§8/§10): se aplican **por igual** a todas las copias, **no** se acumulan.
- `at=(x,y)` / `advance=(dx,dy)` — posición inicial y avance entre instancias, en la propia llamada (misma convención que los generadores, §13.2): evitan el par `moveto`/`step` previo y hacen cada `repeat` **autocontenido** —sin estado de pluma compartido que se filtre entre repeticiones. `advance` es una **traslación pura**; para un avance de pluma que además rote o escale (disposiciones radiales/espirales) se usa el comando `step(...)` (§12). Sin `at`/`advance` (ni `moveto`/`step`) se parte de la pluma vigente sin avanzar.
- `transform=` — transformación acumulada sobre la struct: la instancia *k* recibe `transform` elevada a la *k* (composición sucesiva). Equivale a la matriz `MTRS` de V1. Acepta una composición de llamadas — p. ej. `transform=rotate(30) scale(0.95)` — que se multiplica como una sola matriz antes de acumularse:

```text
% 12 pétalos, cada uno rotado 30° y al 95% del tamaño del anterior
repeat(Petalo, count=12, transform=rotate(30) scale(0.95))
```

*(V1: `RPST n`, con `MTPP` para la posición y `MTRS` para transformar la struct.)*

> **`rotate=` (fijo) vs. `transform=rotate(...)` (acumulado)** — operaciones distintas, no redundantes. `rotate=30` inclina **cada** copia 30° por igual (giro constante, MTST); `transform=rotate(30)` gira la copia *k* en 30°·*k* —cada una respecto a la anterior—, que es lo que produce abanicos y espirales (MTRS). El arreglo radial necesita el acumulado; `rotate=` no lo sustituye. Lo mismo aplica a `scale=` (tamaño fijo) frente a `transform=scale(...)` (escala acumulada).

> `repeat(Struct)` (repite una struct, esta sección) no debe confundirse con `tile(&path)` (concatena un path, §9). Operan sobre cosas distintas y por eso tienen nombres distintos.

> Cuándo usar `repeat` vs. `for`: sin acumulación, un `for` loop que coloca la struct es equivalente. La forma que acumula transformación (cada instancia relativa a la anterior) es la que justifica el constructo dedicado.

---

## 18. Controles del compilador y misceláneos

- **Espacio y tamaño base** — `display_size`, `world_window` (§3). *(V1: `$D`, `WW`; el `$S` de modo de spline se retiró, §9.1.)*
- **Terminar el parseo** — `exit` detiene el procesamiento del archivo en ese punto; el resto se ignora. Útil para depurar. Es un control de parse-time: **no** sirve como condición de paro de una recursión (para eso, `if`, §8.1). *(V1: `EXIT`.)*
- **Salida implícita** — el nombre del archivo de salida se deriva del de entrada (`figura.mg` → `figura.eps`). El backend se selecciona por la extensión del archivo de salida (`.eps`, `.pdf`, `.svg`). *(V1: igual.)*
- **Caracteres acentuados** — para usar caracteres no-ASCII (á, ñ, …) con las fuentes PostScript estándar, V3 activa automáticamente la recodificación del vector de codificación cuando el texto los contiene. *(V1: bandera interna `reencode`.)*
- **Profundidad de recursión** — `max_depth n` fija el límite de expansión para structs recursivas (§8.1). Con la recursión ya especificada deja de ser un campo reservado: el motor debe aplicarlo, con un default razonable (p. ej. 32). *(V1: `MAXDEEP n`.)*

---

## 19. Lo que falta definir (pendientes)

| Tema | Estado | Notas |
|---|---|---|
| **Transformaciones locales** | ✓ Resuelto | Definidas en §7/§11.1: sentencias `translate`/`rotate`/`scale`/`shear` con ámbito de bloque (matriz MTLC), componen en orden de escritura, y se aplican sobre structs (MTST) |
| **Atributos heredados vs. explícitos** | ✓ Resuelto | Semántica de estado y bloques en §7: argumento explícito en la primitiva > estado del bloque interno > estado del bloque externo |
| **`both_sides` en placements** | ⚠️ Abierto | El parámetro V1 `sc < 0` activa ambos lados de forma implícita. En V3 es `both_sides=true`, pero el significado geométrico exacto (perpendicular vs. especular) necesita documentarse |
| **Texto: fuentes y estilos** | ✓ Resuelto | El markup interno del string (`/b`, `/i`, `$...$`, `\alpha`, `^{}`, `_{}`) lo maneja `parse_text()` sin cambios. Los argumentos de `text` cubren fuente base, tamaño y color (ver §14) |
| **Texto: alineación vertical** | ✓ Resuelto | Definido en §4.8/§7.3: `valign` (`"baseline"` default, `"top"`, `"middle"`, `"bottom"`), ortogonal a `align` (horizontal) y fijable como estado |
| **Alcance (scope) de structs** | ✓ Resuelto | Definido en §15: nombres globales, no redefinibles, `include` antes del uso |
| **Manejo de errores** | ◑ Parcial | El compilador ya reporta `archivo:línea`. Falta la columna |
| **Ejes y marcas (TICKS)** | ✓ Resuelto | Definido en §13: `ticks`, `numbers`, `trail` y el declarativo `axis` |
| **Patrones de relleno (FPATRN)** | ✓ Resuelto | Definido en §4.11: `hatch=` (ángulo) y `hatch_gap=` (paso) |
| **Color en escala de gris (LGRAY/FGRAY) y outline-fill** | ✓ Resuelto | Definido en §4: gris = color; contorno de relleno = presencia de `color=` |
| **`ellipse` como primitiva** | ✓ Resuelto | Definido en §4.9: `ellipse(rx, ry)` (conveniencia de `arc(rx, ry, from=0, to=360)`) |
| **Estilo de línea (LWIDTH/LPATRN)** | ✓ Resuelto | Definido en §4.10: `line_width=` (pt), `dash=` (alias o arreglo `[…]`), `cap=`, `join=` |
| **Output implícito** | ✓ Resuelto | Definido en §18: derivado del nombre de entrada; backend por bandera de CLI |
| **Repetición de struct (RPST)** | ✓ Resuelto | Definido en §17: `repeat(...)` |
| **Condicionales (`if`) y recursión de structs** | ✓ Resuelto | Definido en §6.1 (if, comparadores, `and`/`or`) y §8.1 (recursión con paro por `if`, límite `max_depth` §18) |
| **Regla de relleno en `compound`** | ✓ Resuelto | Definido en §9.4: `fill_rule="non-zero"` (default) / `"even-odd"` |
| **Aspect ratio / no-deformación** | ✓ Resuelto | Definido en §3.1: escala uniforme (*meet*) + margen con `align=`; `stretch=true` para deformar; `fit` preserva proporción por default (§10.2); ventanas anidadas isométricas (§16) |
| **Texto bajo `transform`** | ⚠️ Abierto | Deseado: que el texto rote/escale con los `transform` activos (EPS lo permite). Falta definir si se transforma solo el punto de anclaje o también los glifos |
| **`rectangle` bajo rotación/shear** | ✓ Resuelto | `rectangle` es forma transformable como SVG (§4.4): se transforman las 4 esquinas. **Implementado** en los tres backends —`EPSDisplay::rect` y `PDFDisplay::rect` emiten el path de 4 esquinas (trazo/relleno + clip de tramado); SVG ya lo hacía. Verificado: rotado coincide con SVG, no-rotado idéntico; goldens EPS re-bendecidos (ok=18) |
| **Espaciado uniforme en `place` sobre path** | ⚠️ Abierto | Extender `gap=` a paths: instancias cada *n* unidades de longitud de arco, no solo en los puntos del path (§10.1) |
| **Splines cuadráticos (cónicas)** | ✓ Resuelto | **Retiradas** (2026-07-20, §9.1): la cuadrática es subconjunto exacto de la cúbica por elevación de grado, así que no hay nada que ganar en la salida; los tres backends hablan cúbica nativa y los círculos exactos ya los da `arc`/`ellipse` (§4.3). `$S 1` nunca se implementó en V1 y no se usó en 25 años |
| **`spline` como comando** | ✓ Resuelto | **Retirado** (2026-07-20, §9.1): era la misma curva que `smooth` (§9.2) con otro nombre. Del modo `nodes=n` sobrevive la idea de un `sample(&p, n)` del álgebra §9 —muestrear es producir *datos*, no *tinta*—, pendiente de una figura que lo pida |
| **Graficar datos: azúcar `plot { }`** | ✅ Hecho | `plot(x=, y=, box=, xscale=, yscale=, grid=) { contenido + xaxis/yaxis }` (§13.7, 2026-07-15): lineal (matriz envolvente) + log (mapeo puntual); ejes que heredan rango, con `base=` para el cruce; fig2-3, fig6-4 y fig4-5 (3 paneles) portadas. `axis` maduro (`scale="log"`, `minor`, `strip_zero`, `extend`, estilo por-eje). Pendiente Fase 3: `step`/`decimals` automáticos, `format`, `at=v`; `title_at=` (título al extremo del eje). `edge=` suelto diferido (necesita §16) |
| **Modelo de iteración unificado (`for` vs `repeat` vs generadores)** | ◇ Prospectivo | Los tres constructos dedicados están justificados por un núcleo *irreducible* a un `for` + pluma: `repeat` con `transform` **acumulado** (§17), `place` por **longitud de arco** sobre path (§10), y el **formateo numérico** de `numbers`/`ticks` (`by`/`decimals`, §13). Su parte *reducible* (el `repeat` sin acumulación, el mero recorrido de `ticks`) podría especificarse como azúcar sobre `for`. **fig4-10 y rpstest ya traducidas confirmaron los tres constructos**; el "reset de pluma" (G3 de `rpstest`) se disolvió con `repeat` autocontenido (`at`/`advance` en la llamada, §17). La unificación como azúcar sobre `for` queda como refinamiento opcional, no bloqueante |
| **Tamaño de texto relativo (tipo TeX)** | ◇ Prospectivo | `font_size` es absoluto (pt), un solo keyword en documento y bloque (§7.3). Un segundo eje `text_size` como **factor** relativo a la base (efectivo = `font_size × text_size`) se pospone hasta que existan calificadores nombrados que lo aprovechen; el corpus usa tamaños absolutos sin ratios redondos |
| **Resolución geométrica (estilo MetaPost)** | ◇ Prospectivo | Describir relaciones en vez de calcular coordenadas: intersección de dos líneas, punto a una fracción de un path, punto medio. Convertiría "dibujar" en "describir"; es el siguiente salto de expresividad tras `axis`/`grid` |

---

## 20. Estrategia de transición V1 → V3

El compilador V3 **no** mantendrá compatibilidad con la sintaxis V1. La migración es mediante un traductor Python (`mg1to2.py`) que convierte archivos V1 al nuevo formato.

### Cobertura esperada del traductor (>90% automático):

```python
# Reglas de traducción mecánica
OPST name → struct Name() {
CLST       → }
MKST name  → Name()          # uso sin parámetros
PL { ... } → polyline { ... }
CR r : ... } → circle(r=r) { ... }
BR p1 p2   → rectangle { p1 p2 }
LWIDTH w   → line_width <w*0.2>  # sentencia de estado (§7); LWIDTH 0 → line_width 0.1
LPATRN n   → dash "solid"|"dashed"|"dotted"|"dashdot"|"dashdotdot"   # §4.10
LCOLOR c   → color "c"        # sentencia de estado
FCOLOR c   → fill "c"
FGRAY g    → fill gray(1 - g/100)   # % de negro → intensidad 0..1 (§4)
FILL/NOFILL→ fill "c" / fill "none"
TALIGN n   → align "left"|"center"|"right"
THEIGHT h  → font_size h
TLLC dx dy → translate dx dy  # transformaciones también son sentencias (§11.1)
RTLC a     → rotate a
SCLC sx sy → scale sx sy
INPUT f    → include "f"
LNST ...   → place(...) { p1 p2 }
ARCST ...  → place(..., r, from, to) { c }
```

> **El nuevo modelo de estado (§7) simplifica el traductor:** los comandos de estado de V1 (`LCOLOR`, `LWIDTH`, `FILL`, `TLLC`…) son sentencias que aplican hasta cambiarse, y mapean **1:1** a las sentencias de estado de V3 —sin la lógica de "acumular sobre la siguiente primitiva". El único cuidado es el **ámbito**: donde V1 abría/cerraba estado con estructuras o al reasignar, el traductor puede envolver en un bloque `{ }` para acotar el efecto y reproducir el alcance original.

> Nota: el traductor capitaliza los nombres de structs (`OPST flecha` → `struct Flecha`) para seguir la convención de §1.

### Casos que requieren revisión manual:
- Uso de `MKST` + matrices MTST (los parámetros se transforman en parámetros de struct)
- `RPST n` con `SCST`/`TLST` (requiere modelado como `for` loop)
- Texto con markup complejo (`TSTYLE`, `TXSI`, subíndices/superíndices)
- Tamaños físicos bajo la política isométrica (§3.1): en V1 el radio de `CR` se medía en unidades del eje y, las structs colocadas escalaban a `min(dvx,dvy)` y `DOT` usaba puntos tipográficos. Si el archivo V1 tenía `WW` y `$D` con proporciones distintas, el traductor debe convertir estos valores para conservar el tamaño físico del original.

---

## 21. Cobertura V1 → V3 (auditoría de keywords)

Auditoría completa de cada palabra clave del léxico V1 (`keyword_map` en `MGLexer::init_tables()`) frente a la especificación V3. Estados: ✓ cubierto · ◑ parcial · ✗ se elimina.

### Primitivas y atributos

| V1 | V3 | Estado | Sección |
|---|---|---|---|
| `PL` | `polyline` | ✓ | §4.1 |
| `PG` | `polygon` | ✓ | §4.2 |
| `CR` | `circle` | ✓ | §4.3 |
| `BR` | `rectangle` | ✓ | §4.4 |
| `EL` | `ellipse(rx,ry)` | ✓ | §4.9 |
| `DOT` | `dot` | ✓ | §4.6 |
| `BZ` | `bezier` | ✓ | §4.7 |
| `SP` | `smooth` | ✓ | §9.2 (`spline` retirada, §9.1) |
| `LWIDTH` | `line_width=` (pt; `w×0.2`, `0`→`0.1`) | ✓ | §4.10 |
| `LPATRN` | `dash=` (alias; `1`→`"solid"` como `0`) | ✓ | §4.10 |
| `LSTYLE` | — | ✗ | bug V1 (mapea a `line_width`), se descarta |
| — | `cap=` / `join=` (nuevos en V3) | ✓ | §4.10 |
| `LCOLOR` | `color=` | ✓ | §4 |
| `FCOLOR` | `fill=` | ✓ | §4 |
| `FILL` / `NOFILL` | `fill=` (presencia/ausencia) | ✓ | §4 |
| `LGRAY` / `FGRAY` | gris en `color=`/`fill=` (`gray(x)`, `x` 0..1) | ✓ | §4 |
| `FPATRN` | `hatch=` / `hatch_gap=` | ✓ | §4.11 |
| `TSTYLE` | `font=` | ✓ | §14.3 |
| `THEIGHT` / `TSIZE` | `font_size=` | ✓ | §4.8 |
| `TALIGN` | `align=` | ✓ | §4.8 |

### Estructuras y placements

| V1 | V3 | Estado | Sección |
|---|---|---|---|
| `OPST` … `CLST` | `struct Nombre(...) { }` | ✓ | §8 |
| `MKST` | `Nombre(...)` (invocación) | ✓ | §8 |
| `LNST` | `place(...) { p1 p2 }` (línea) | ✓ | §10.1 |
| `ARCST` | `place(..., r, from, to) { c }` (arco) | ✓ | §10.1 |
| `DPST` / `&name path` | `place(...) { &path }` (path) | ✓ | §10.1 |
| `PWST` | `fit(...)` | ✓ | §10.2 |
| `RPST` | `repeat(...)` | ✓ | §17 |

### Paths

| V1 | V3 | Estado | Sección |
|---|---|---|---|
| `&name path` | `path name = ...` | ✓ | §9 |
| `RPPT` | `tile()` | ✓ | §9 |
| `CTPT` … `CLPT` | `concat()` | ✓ | §9 |
| `INVPT` | `reverse()` | ✓ | §9 |
| `NORMPT` | `normalize()` | ✓ | §9 |
| `PWPT` | `fitrect()` | ✓ | §9 |
| `GNPATH` | `trail()` | ✓ | §9.3 / §13.4 |
| `GNBZPATH` | `smooth()` | ✓ | §9.2 |
| `OPPT` … `CLPT` | `compound { }` | ✓ | §9.4 |

### Transformaciones, pluma y generadores

| V1 | V3 | Estado | Sección |
|---|---|---|---|
| `TLLC`/`RTLC`/`SCLC`/`SHLC` | `translate`/`rotate`/`scale`/`shear` (sentencias) | ✓ | §7 / §11 |
| `IDLC` | (cierre del bloque `{ }`) | ✓ | §7.1 |
| `··ST` (MTST) | `transform` sobre la llamada | ✓ | §11.2 |
| `··PP` (MTPP) | `step(...)` | ✓ | §12 |
| `··PT` (MTPT) | álgebra de paths | ✓ | §9 |
| `··RS` (MTRS) | `repeat(transform=...)` | ✓ | §17 |
| `XYPP` | `moveto` | ✓ | §12 |
| `DT` | `text("s")` (en la pluma) | ✓ | §12.1 |
| `XYDT` | `text("s") { x y }` | ✓ | §4.8 |
| `TICKS` | `ticks` / `axis` | ✓ | §13.1 / §13.5 |
| `GNNUM` | `numbers` | ✓ | §13.2 |

### Controles y obsoletos

| V1 | V3 | Estado | Sección |
|---|---|---|---|
| `$D` | `display_size` | ✓ | §3 |
| `$P` | tamaño base de texto | ✓ | §3 / §14 |
| `$S` | — (retirado; `smooth` no tiene modos) | ✓ | §9.1 |
| `WW` | `world_window` (global y anidado) | ✓ | §3 / §16 |
| `INPUT` | `include` | ✓ | §15 |
| `EXIT` | `exit` | ✓ | §18 |
| `MAXDEEP` | `max_depth` | ✓ | §18 / §8.1 |
| `INTXT` / `XYTXT` | — | ✗ | obsoletos, se eliminan |
| `MKMR` / `MR` | — | ✗ | internos/obsoletos, revisar al traducir |

---

## 22. Continuidad del motor y hoja de ruta de ingeniería V3

Esta sección consolida las conclusiones de la auditoría de backend (2026-06/07) que
siguen siendo relevantes hacia adelante. **Sustituye a `PENDIENTES.md`**, que era el
registro de esa auditoría: al abrir la rama de V3, ese archivo puede retirarse — lo
histórico (ítems ya resueltos) no necesita viajar, y lo que sí importa vive aquí y en
§19 (decisiones de diseño abiertas).

### 22.1 Qué se reemplaza y qué continúa

V1 no es una sola pieza; son tres capas con destinos distintos:

| Capa | Qué es | Destino en V3 |
|---|---|---|
| **Front-end de gramática** | Léxico + parser de comandos de dos letras (`PL`, `CR`, `OPST`, `MKST`, matrices `TL`/`ID`/`CP`…) | **Se congela y se reemplaza.** La gramática V3 (§1–§18) es nueva; la migración de fuentes V1 es vía el traductor `mg1to2.py` (§20), no un modo dual en el compilador. No vale la pena invertir en esta capa. |
| **Motor / backend compartido** | `Display` y sus tres backends (EPS/PDF/SVG), `Matrix`, `primitives`, `splines`, y el módulo de texto (heredado sin cambios, §14.2) | **Continúa.** El compilador V3 estrena gramática pero **emite a través de esta misma maquinaria.** Toda inversión aquí es inversión en V3. |
| **Traductor V1→V3** | `mg1to2.py` | **Nuevo.** Es donde el conocimiento de la sintaxis V1 se preserva de forma ejecutable (§20–§21). |

La consecuencia práctica: la distinción útil no es "V1 vs V3" sino **"front-end
congelado" vs "motor compartido que perdura"**. Endurecer el motor no es mantenimiento
de V1 legado; es preparar el sustrato de V3.

### 22.2 Estado del motor: punto de partida para V3

La auditoría dejó el motor en un estado deliberadamente más robusto y profesional que
el que describía el artículo original de V1. Lo relevante para construir V3 encima:

- **Coordenadas en `double` de punta a punta.** `Matrix` y toda la tubería
  (`Display`, los tres backends, `point` y demás primitivas, parser, texto, splines)
  pasaron de `float` a `double`. Las composiciones largas de transformaciones ya no
  acumulan error de precisión y `deg2rad` (que siempre fue `double`) deja de truncarse
  en cada rotación. Para calidad de publicación, importa.
- **Máquina de estado de matrices unificada en la clase base.** El estado compartido
  (`mt`, `mtst`, las pilas) y su contabilidad (`pushMatrix`/`popMatrix`/`init_matrix`,
  la rama `MTST` de las transformaciones) viven en `Display`; cada backend solo
  implementa los *hooks* `device*` de su rama nativa (`MTLC`). Los tres backends son
  simétricos y ninguno reimplementa la aritmética de la pila.
- **EPS válido.** El emisor de `init_matrix` producía `defaultmatrix` a secas
  (PostScript inválido: `stackunderflow`); ahora emite `initmatrix`, que es el
  operador correcto para resetear la CTM.

### 22.3 Refactor de motor diferido: extraer `DeviceBackend` puro

Queda una mejora de arquitectura **no urgente** pero recomendada cuando V3 empiece a
exigirle a los backends (recursión, `transform` anidados, o un cuarto formato de
salida). Hoy `Display` mezcla dos responsabilidades:

- **(A)** la máquina de estado semántica de MetaGráfica (pila de transformaciones,
  estado de dibujo, resolución de structs) — idéntica entre backends; y
- **(B)** las primitivas de salida — genuinamente específicas de cada formato.

La Etapa 1 de esa separación ya está hecha (A subió a `Display`). La **Etapa 2** es
extraer un tipo `DeviceBackend` puro y sin estado (~12 métodos: ciclo de vida,
agrupamiento, transform nativo, y las primitivas `path`/`arc`/`dot`/`text`). Un backend
nuevo implementaría solo eso y **estructuralmente no podría corromper la pila de
transformaciones** — ni siquiera tendría acceso a ella. El peor bug de la sesión de
`SVGDisplay` fue exactamente eso, y con este diseño sería imposible de escribir.
Emprenderla solo si se prevé ese cuarto backend o se quiere testeabilidad (mock/null
backend); no es bloqueante.

### 22.4 El puente isométrico (§3.1)

La política de aspect ratio de V3 —espacio isométrico por construcción, escala uniforme
*meet* + margen (§3.1)— era el cambio que más tocaba al motor compartido, y **ya está
implementado** (2026-07-05). Con él **desaparecieron las compensaciones por `getRatio()`**
que vivían en `src/structure.cpp` (los `stpos.x *= ratio` y análogos), y con ellas la causa
raíz del defecto de V1: la normalización anisotrópica a [0,1]² que deformaba en formato
apaisado. Fue trabajo sobre el backend, no sobre la gramática.

### 22.5 Red de regresión

El arnés de regresión *golden-file* sobre `examples/` —el primer paso de mayor palanca al
abrir la rama V3— **ya está construido** (`test/run.sh`): captura la salida de referencia y
tras cada cambio compara por bytes `.eps` y `.svg` (`diff -q`) y visualmente el `.pdf` (varía
por timestamps internos). Estado actual: **ok=18, fail=0**. Con esa red, tocar el backend dejó
de ser una apuesta manual —fue lo que permitió verificar y re-bendecir con confianza, por
ejemplo, el `rectangle` transformable (§4.4).

### 22.6 Orden de trabajo recomendado para la rama V3

**Hecho:**
- **Arnés de regresión** golden-file (§22.5): `test/run.sh` sobre `examples/v1/` (ok=18).
- **Puente isométrico** (§3.1 / §22.4) implementado en el motor.
- **Diseño de la gramática V3** (§1–§18) **cerrado**: el corpus completo se tradujo a mano a
  `examples/v3/` (fixtures y oráculos de traducción), y cada traducción destapó y resolvió los
  huecos —de ahí salieron `line_width`, coords en bloque, listas+`for`, `at/rotate/scale`,
  `place` (`count`/`from`/`to`), `dot`/`marker`, `polybar`, `sine`, `compound`, subtrayectos `;`,
  `fit` con reflejo, etc.
- **`rectangle` transformable** implementado en los tres backends (§4.4 / §19).

**Siguiente (el bloque grande):**
1. **Parser V3** (§1–§18) — el cuello de botella actual; el diseño ya está maduro y validado por
   el corpus traducido.
2. **Primitivas nuevas en el motor**: `sine` (generar las bezier internas), `polybar` (ya con
   clase + `draw`; falta lexer + parser), y la escotilla de tamaño **físico** del marcador
   colocado bajo `stretch` (`plan_marcadores.md`).
3. **`mg1to2.py`** (§20) — tras el parser; el corpus de prueba ya está listo como par
   `examples/v1/` ↔ `examples/v3/`.
4. **Refactor `DeviceBackend`** (§22.3) — oportunista, cuando el manejo de matrices estorbe.

### 22.7 Qué falta para salir de beta

`MG_VERSION` es **3.0.0-beta**. La palabra dice dos cosas a la vez: el lenguaje **todavía
puede cambiar** (nombres y argumentos no están congelados) y **faltan piezas** de esta
spec. Lo que sí está se ejercita con el corpus en cada cambio y ha compuesto libros.

**Las tres condiciones** (decididas 2026-07-16). No hay más:

1. **Congelar la gramática.** Es lo que hace que la beta sea beta. Solo en la última
   sesión se movieron `axis(title=)`→`label=`, `labels=`→`tick_labels=` y
   `dot(marker=)`→`marker(shape=)`. Congelar significa que un `.mg` que compila hoy
   seguirá compilando, y que un renombre pasa a costar una migración de verdad. (Los
   nombres viejos fallan en compilación apuntando al nuevo, nunca en silencio; esa
   escalera se retira al congelar.)
2. **Cerrar lo aparcado de `plot`**: `rule` (§13.8), `legend` (§13.9) y `table` (§13.10),
   hoy reservados y sin construir. Cada uno espera **figuras que lo pidan** — es la regla
   del proyecto: no especular sin ejemplo.
3. **Texto en UTF-8** (§14.4): quitar la conversión a Latin-1, que es una restricción de
   EPS impuesta a los tres backends antes de que exista un Display.

**Lo que NO bloquea beta**, aunque importe:

- **`mg1to2.py`** (§20). Del material V1 solo queda el corpus del libro de QM, y las
  figuras más difíciles ya se tradujeron —y se *transformaron*— a mano. El traductor
  sigue siendo importante para seguir importando figuras, pero no es condición.
- **Texto multilínea** (§14.1). Especificado, sin implementar; ninguna figura del corpus
  lo pide todavía.
- **Pseudo-3D** (`plan_pseudo3d.md`). Tema aparte y con poco avance. **MG no se va a
  convertir en 3D**: la meta es *simular* volumen con proyección oblicua (`shear`), por
  composición de primitivas 2D, sin z-buffer ni cámara. Ver `lib/pseudo3d.mg` y
  `examples/simulate3d/`.


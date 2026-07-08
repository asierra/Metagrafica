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
              | "spline"  ["(" ArgList ")"] "{" PointList "}"
              | "smooth"  "{" PointList "}"
              | "trail"   "(" ArgList ")"
              | "tile"    "(" "&" ID "," "times" "=" NUMBER ")"
              | "concat"  "(" "&" ID ("," "&" ID)+ [ "," "join" "=" ("true"|"false") ] ")"
              | "reverse" "(" "&" ID ")"
              | "normalize" "(" "&" ID ")"
              | "fitrect" "(" "&" ID ")" "{" PointList "}"

PrimName    ::= "polyline" | "polygon" | "circle" | "rectangle"
              | "arc" | "ellipse" | "dot" | "marker" | "polybar" | "sine" | "bezier"

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

**Subtrayectos (`;`).** Dentro del bloque de una primitiva de trazo (`polyline`, `polygon`, `bezier`, `spline`), un `;` separa **subtrayectos disjuntos**: `polyline { a b ; c d }` dibuja dos trazos independientes con el mismo estilo, sin conectarlos —evita repetir la primitiva cuando varios trazos comparten estilo. En `polygon` cada subtrayecto se cierra y rellena por separado; para un relleno **combinado con huecos** (regla par-impar) se usa `compound` (§9.4), no `;`.

**Coordenadas con expresiones.** Una coordenada del bloque puede ser una expresión, pero de nivel **`Term`** (§2): número, variable, `func(...)`, `a*b`, `a^b`, con `-` unario inicial. Como las coordenadas van **separadas por espacio**, un `-` **siempre inicia una coordenada nueva** (negada), no una resta binaria: `{ -0.2 -0.5 }` son dos puntos, no `-0.7`. Para **sumar o restar** dentro de una coordenada, se usan paréntesis: `{ (x+0.1) y }`.

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

**`dot(r)`** es el caso base: un disco de radio físico `r`. Por defecto relleno; con `color=` y sin `fill=`, un círculo **abierto** (convención §4). Se conserva por brevedad y compatibilidad.

**`marker(r, shape="…")`** generaliza `dot` a una familia de formas, todas de radio físico `r` (el símbolo cabe en un círculo de radio `r`, para que queden comparables). `dot(r)` ≡ `marker(r, shape="disk")`.

```text
dot(0.15) { 2 3  4 3 }                             % disco relleno (atajo)
marker(0.1, shape="circle", color="black") { … }  % círculos abiertos (fig2-3)
marker(0.12, shape="cross") { … }                  % cruces +           (fig1)
marker(0.12, shape="circle-dot") { … }             % círculo con punto ⊙ (fig1)
```

**Formas:**

| `shape` | Símbolo | Relleno/contorno |
|---|---|---|
| `"disk"` (= `dot`) | ● disco | relleno (o abierto con `color=` sin `fill=`) |
| `"circle"` | ○ círculo | abierto/relleno según §4 |
| `"square"` | □ cuadro | abierto/relleno según §4 |
| `"diamond"` | ◇ rombo | abierto/relleno según §4 |
| `"cross"` | + | solo trazo |
| `"x"` | × | solo trazo |
| `"circle-dot"` | ⊙ círculo con punto | compuesto: contorno + punto central relleno |

Color y relleno/contorno siguen §4 (`fill=` rellena; `color=` fija el trazo; en las formas de solo trazo —`cross`, `x`— manda `color=`). El radio `r` es **físico**: no se deforma bajo `stretch` ni bajo transformaciones —solo la posición se transforma.

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
text("$\alpha + \beta = \gamma$", align="center", valign="middle") { x y }
text("etiqueta") { 0 0  5 5  10 0 }   % la misma cadena en varios puntos
```

El argumento `align` (horizontal) puede ser `"left"` (default), `"center"`, `"right"`; `valign` (vertical) puede ser `"baseline"` (default), `"top"`, `"middle"`, `"bottom"`. Son ortogonales: se combinan y se fijan por separado, también como estado (§7.3). La forma sin bloque (o con bloque vacío), `text("...")`, dibuja en la posición de pluma (§12.1). El markup interno del string se documenta en §14.

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

En vez de relleno sólido, una primitiva cerrada puede rellenarse con una **trama** de líneas paralelas. La trama tiene exactamente **dos parámetros**:

- `hatch` — ángulo de la trama en grados; **obligatorio**, restringido a los cuatro valores que provee el motor: `0`, `45`, `90` o `135`.
- `hatch_gap` — separación entre líneas en **puntos**; **opcional** (default según el motor). Es una cantidad física (pt), no escala con `world_window` (§3.2), porque el tramado se barre en el espacio de dispositivo ya transformado a pt (`EPSDisplay::rect` aplica la matriz antes de emitir; SVG teja un `<pattern>` en userSpace de pt). Por eso las tres densidades de V1 (`FPATRN`) equivalen a `hatch_gap` de **4, 2 y 1 pt**, mapeo 1:1 sin calibración.

No hay más: la trama son **trazos**, así que su **color** y su **grosor** salen del estado de trazo vigente (`color` y `line_width`, §4.10) —no existe un `hatch_color` ni un `hatch_width` aparte. Si se da `color=`, además se dibuja el contorno de la región en ese mismo color (§4); sin `color=`, la trama usa el color de trazo del bloque (negro por default).

**Dos registros** (la distinción general de §7.5):

- **Atributo** (en paréntesis, nombrado) — aplica a una sola primitiva:

```text
polygon(hatch=45)             { 0 0  5 0  5 5  0 5 } % trama diagonal a 45°
polygon(hatch=0, hatch_gap=3) { 0 0  5 0  5 5  0 5 } % trama horizontal, paso 3 pt
```

- **Sentencia de estado** (posicional, rige hasta el fin del bloque, §7) — el ángulo y el paso opcional caben en una sola línea, igual que `translate dx dy`:

```text
hatch 45          % solo ángulo; el paso toma el default
hatch 45 4        % ángulo 45, paso 4 pt
polygon { 0 0  5 0  5 5  0 5 }   % ambos con esa trama
polygon { 6 0  9 0  9 5  6 5 }
```

  El segundo valor posicional es el paso; equivale a fijarlo aparte con `hatch_gap 4`. Como el ángulo está limitado a cuatro valores, el paso nunca resulta ambiguo.

*(V1: `FCOLOR c` + `FPATRN` → `color "c"` + `hatch a`; el `FCOLOR` de la trama es el color de trazo, no un relleno sólido. `FPATRN n` codificaba ángulo y densidad de forma combinada en el índice `n`; `n` negativo añadía el contorno.)*

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

Internamente `polybar` expande cada punto a un `rectangle` (§4.4) usando el `rect()` del motor, así que **no requiere lógica nueva en los backends**, y las barras heredan que `rectangle` es transformable. *(V0: se usó en el primer artículo publicado —los histogramas de espectro, `docs/first_article.pdf` figs. 4–6.)*

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

*(V1: se ensamblaba a mano con `BZ &sinpi`/`&sin2pi`/`&cos2pi` + `RPPT` (tile) + la pila de matrices de path `IDPT`/`TLPT`/`SCPT`. `sine` colapsa todo eso. Caso guía: fig4-10, los niveles del pozo infinito.)*

---

## 5. Variables y expresiones

Ámbito léxico. Las variables definidas fuera de un bloque son globales al archivo.

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

**Funciones y constante:** `sin(x)`, `cos(x)`, `tan(x)`, `sqrt(x)`, `abs(x)`, `atan2(y, x)`, `mod(a, b)`, `len(lista)`, y `pi`. El conjunto se mantiene deliberadamente pequeño; se ampliará (redondeo, `min`/`max`, transcendentales como `exp`/`ln`…) **solo cuando el corpus lo exija** —para no cargar el lenguaje con funciones especulativas.

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

- Comparadores: `<`, `>`, `<=`, `>=`, `==`, `!=`; combinables con `and` / `or`.
- El bloque tiene ámbito léxico, como `for` y los bloques `{ }` (§7.1); la rama `else` es opcional.
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

Una llave `{ … }` en posición de sentencia abre un **bloque anónimo**: un ámbito que apila el estado gráfico al entrar y lo restaura al salir (un `gsave`/`grestore`). Los cambios de estado dentro del bloque **no se filtran** afuera.

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
| `hatch a [g]` | relleno con trama: ángulo `a` (obligatorio) y paso `g` (opcional); o `hatch_gap g` aparte | §4.11 |
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

% Dibujar un path nombrado
bezier(&sinpi)
```

> Las dos operaciones "ajustar a rectángulo" tienen nombres distintos según su operando: `fitrect(&path)` produce un path (álgebra, esta sección); `fit(Struct)` (§10.2) coloca una struct. Igual con la repetición: `tile(&path)` concatena un path; `repeat(Struct)` (§17) repite una struct.

**Empalme en `concat` y `tile` (semántica de unión).** Al construir un path a partir de varios, cada operando subsiguiente se **traslada** para que su primer punto continúe desde el último punto del path acumulado. Este empalme automático es lo que permite armar una curva larga a partir de segmentos definidos en un dominio canónico (p. ej. los medios periodos seno/coseno de una biblioteca de curvas).

- **`join=true` (default).** Traslada cada pieza para pegar su inicio al final de la anterior. Es continuidad **C0** (de posición): la **tangente no se ajusta**, así que para uniones suaves los segmentos deben estar diseñados para encontrarse (como las Bézier de seno/coseno) o se usa `smooth`/`spline` (§9.1–9.2) sobre los nodos combinados. `tile` siempre empalma así (repetir sin empalmar solo produciría copias superpuestas).
- **`join=false`.** Concatena las coordenadas **tal cual**, sin trasladar. Útil cuando los paths ya están posicionados en coordenadas absolutas y solo se quiere unir sus listas de puntos.
- **Sin auto-reversión.** El empalme pega el *inicio* de cada operando al final del anterior. Si un segmento debe recorrerse al revés, se envuelve explícitamente en `reverse(&seg)` (evita heurísticas de dirección frágiles).
- **Variádico.** `concat(&a, &b, &c, …)` une la secuencia en orden. Para tamaños distintos por segmento, se transforma cada operando antes (p. ej. con `fitrect`); `concat` solo une, no escala.

*(V1: `CTPT`/`RPPT` con la matriz `PT` y `SCPT` entre appends. La traslación de empalme la hacía el motor —`concat_paths`— pero asumía que el frente de cada segmento estaba en x=0 (precondición no documentada, corregida 2026-07-06 para alinear en ambos ejes). La continuidad C1 en las uniones queda como posible extensión futura, p. ej. `join="smooth"`.)*

### 9.1 spline con control de nodos

`spline` interpola una curva suave a través de los puntos de control. Su comportamiento se ajusta con argumentos nombrados:

```text
path c = spline { 0 0  2 3  5 1  8 4 }              % Catmull-Rom centrípeto (default)
path c = spline(nodes=8) { 0 0  2 3  5 1  8 4 }     % 8 nodos interpolados por segmento
path c = spline(mode="bezier") { 0 0  2 3  5 1  8 4 } % convierte a curva Bézier equivalente
```

- Default: spline cúbico Catmull-Rom centrípeto.
- `nodes=n` (n>1): puntos interpolados por segmento; más nodos = curva más suave, más datos.
- `mode="bezier"`: convierte los puntos de control a una curva Bézier (menos datos, recomendado para la salida). *(V1: `$S 0`.)*
- `mode="conic"`: reservado, aún no soportado. *(V1: `$S 1`.)*

### 9.2 smooth — Bézier suave a través de puntos

```text
path s = smooth { 0 0  2 3  5 1  8 4 }
```

Ajusta segmentos Bézier que pasan **exactamente** por los puntos dados, calculando las tangentes automáticamente. A diferencia de `spline` (donde los puntos son de *control*), aquí los puntos son *nodos* por los que la curva pasa. *(V1: `GNBZPATH name path`.)*

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

`axis` describe un eje completo (línea + marcas + etiquetas + título opcional) en una sola sentencia. El bloque `{ }` da los dos extremos físicos del eje; los argumentos describen el rango numérico y la decoración. Internamente es un `place` a lo largo de una línea (§10.1) que coloca marcas y etiquetas calculadas desde el rango — la pluma y el `step` se orquestan solos.

```text
% Eje X: de (0,0) a (10,0), valores de 0 a 10, marca y etiqueta cada 2
axis(from=0, to=10, step=2, ticks="out", labels=true, title="x / cm") { 0 0  10 0 }

% Eje Y: el mismo constructo, distinto par de extremos
axis(from=0, to=8, step=1, ticks="out", labels=true) { 0 0  0 8 }
```

**Argumentos:**

| Arg | Default | Significado |
|---|---|---|
| `from`, `to` | — | valores numéricos en el extremo inicial y final |
| `step` | — | intervalo de valor entre marcas/etiquetas |
| `start` | `from` | valor de la **primera** marca/etiqueta; sirve para omitir el origen (`start=0.1`) |
| `ticks` | `"out"` | dirección de las marcas: `"out"`, `"in"`, `"both"`, `"none"` (relativas a la línea) |
| `tick_size` | pequeño | longitud de las marcas en unidades de usuario |
| `labels` | `true` | mostrar etiquetas numéricas |
| `decimals` | `0` | decimales en las etiquetas |
| `label_gap` | auto | distancia de las etiquetas a la línea |
| `title` | — | rótulo del eje (texto, admite markup matemático §14) |
| `title_font`, `title_size` | ambiente | fuente/tamaño del título, para cuando difieren de las etiquetas |

Las **marcas y etiquetas** van en `start, start+step, …` hasta `to` (`start` default = `from`). Las **etiquetas** heredan el estado de texto vigente (`font`, `font_size`, `align`, §7.3); solo el **título** admite override propio (`title_font`/`title_size`), porque suele diferir —p. ej. números en itálica y título en romana, como en fig2-3.

La orientación de marcas y etiquetas se deriva de la dirección de la línea (la lógica de tangente del motor de placements), así que un eje horizontal, vertical o inclinado se escriben igual, cambiando solo los extremos del bloque. Aquí `step` es el **intervalo de valor** (como en el `for` loop, §6), no el `step` de avance de pluma (§12).

*(V1: combinación manual de `TICKS` + `GNNUM` + `LNST`/`PL`; ver el desglose de bajo nivel en §13.3.)*

> ⚠️ **Propuesta para estudiar (obligatorios / optativos).** La meta es que el caso común
> pida el mínimo. Distinción propuesta:
>
> **Obligatorio:**
> - El **anclaje geométrico**: los dos extremos físicos del bloque `{ p1 p2 }` — *salvo* que el
>   eje esté dentro de un marco de datos (§13.7), donde el borde se elige con `edge="bottom" |
>   "top" | "left" | "right"` y el bloque se omite.
> - El **rango numérico** `from`/`to` — *salvo* dentro de un marco de datos, donde se **heredan**
>   de la ventana (§16) y no se repiten.
>
> **Optativo (con default útil):**
> - `step` **automático** si se omite: elegir un paso "redondo" (de la familia 1·10ᵏ, 2·10ᵏ,
>   5·10ᵏ) que produzca ~5–8 marcas en el rango, al estilo de un localizador de ejes. Esto quita
>   el parámetro más tedioso del caso común.
> - `decimals` **derivado del paso** si se omite (un paso de 0.1 ⇒ 1 decimal).
> - `ticks="out"`, `labels=true`, `tick_size`, `label_gap`, `title` — como en la tabla.
>
> **Candidatos nuevos a considerar:**
> - `minor=n` — subdivisiones menores (marcas sin etiqueta) entre marcas mayores.
> - `at=v` — posición del eje cruzado (p. ej. dibujar el eje X a la altura del cero, `at=0`).
> - `scale="log"` — eje logarítmico (marcas y etiquetas en décadas).
> - `format="%.1f"` / notación científica — formato de etiqueta más allá de `decimals`.

### 13.6 grid — retícula

`grid` dibuja una retícula sobre el rectángulo dado por el bloque (esquina inferior-izquierda, esquina superior-derecha), con líneas verticales y horizontales a intervalos regulares.

```text
grid(xstep=2, ystep=1, color="lightgray") { 0 0  10 8 }
grid(xstep=2, ystep=2, dash="dotted")      { 0 0  10 8 } % retícula punteada
```

**Argumentos:** `xstep` / `ystep` (intervalos en unidades de usuario), más los atributos de estilo comunes (`color`, `line_width`, `dash`). Útil como fondo de una gráfica antes de trazar datos y ejes.

### 13.7 Graficar datos: el marco de datos

> **Estado: núcleo resuelto** (realizado en `examples/v3/fig2-3.mg`). Para un plot que es toda la
> figura basta la `world_window` de nivel superior **en unidades de datos** con `stretch=true`;
> **no** hace falta ventana anidada —esa se reserva para **paneles** dentro de una figura mayor
> (fig4-10). Los marcadores de datos son **físicos** (`dot`/`marker`, §4.6), inmunes al `stretch`.
> Lo único **abierto** es el azúcar opcional `plot{ }` (al final de esta sección). El ejemplo de
> abajo ilustra el concepto con ventana anidada; la realización *standalone* de fig2-3 la omite.

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

**Azúcar opcional `plot { }` (a decidir).** Si el patrón "ventana de datos + dos ejes" es
frecuente, un constructo `plot` podría empaquetarlo: la caja física en el bloque, los rangos como
argumentos, `stretch` por default, y ejes con una línea:

```text
plot(x=(0,1), y=(0,6)) { -1 11  -1 7 } {   % rangos de datos ; caja física
    xaxis(step=0.1, title="relative temperature")
    yaxis(step=1,   title="heat capacity")
    bezier   { ... }        % en datos
    dot(0.1) { ... }        % en datos
}
```

Queda abierto si el doble bloque (rango + caja / contenido) es aceptable o si conviene que `plot`
tome solo el contenido y derive la caja de la ventana vigente. **Recomendación:** especificar
primero el marco de datos con ventana anidada (no cuesta nada, reutiliza §16), y decidir el
azúcar `plot` al traducir fig2-3 y fig4-10 (§22.6), cuando se vea cuánta repetición ahorra.

---

## 14. Texto matemático

El texto es procesado por un módulo independiente (`text_parser`) que se hereda **sin cambios** de V1. El compilador V3 extrae el string del bloque `{ "..." }` y lo pasa a `parse_text()`, que produce una `TextLine` con los chunks ya descompuestos por fuente, tamaño y nivel de script.

### 14.1 Markup del string

El contenido del string soporta el siguiente markup, procesado en parse time:

#### Modo matemático

```text
$...$    activa la fuente CMMI (Computer Modern Math Italic) para el bloque
```

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
| `/g` | Symbol (letras griegas en posición estándar) |
| `/c` o `/t` | Courier |
| `/$` | CMMI (math italic) |

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

- **Espacio y tamaño base** — `display_size`, `world_window` (§3) y `spline(mode=...)` (§9.1). *(V1: `$D`, `WW`, `$S`.)*
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
| **Splines cuadráticos (cónicas)** | ⚠️ Abierto | `spline(mode="conic")` reservado en §9.1; V1 lo contemplaba (`$S 1`). Decidir si se soporta nativo (QuadTo en SVG) o por conversión a cúbico |
| **Graficar datos: azúcar `plot { }`** | ◑ Parcial | **Núcleo resuelto** (§13.7, realizado en `examples/v3/fig2-3.mg`): marco de datos = `world_window` en datos + `stretch`, marcadores físicos (`dot`/`marker`), y `axis` con `start`/`title_font` (§13.5). Queda **solo**: el azúcar `plot { }`, y las propuestas de §13.5 aún sin adoptar (`edge=`, `step` automático, `minor`/`at`/`scale="log"`/`format`) |
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
| `SP` | `spline` | ✓ | §9.1 |
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
| `$S` | `spline(mode=, nodes=)` | ✓ | §9.1 |
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


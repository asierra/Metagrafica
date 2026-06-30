# MetaGráfica V2 — Especificación del Lenguaje

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
| Nombres | Descriptivos: `PolyLine`, `Circle`, `Rectangle`, `Bezier`, `TextAt` |
| Paths | Secuencias de pares `x y`; primitivas multi-punto aplican la operación en cada punto |

---

## 1. Estructura Léxica

- **Identificadores (`ID`):** `[a-zA-Z_][a-zA-Z0-9_]*`
- **Números (`NUMBER`):** `[0-9]+(\.[0-9]+)?` (punto flotante)
- **Cadenas (`STRING`):** texto entre comillas dobles `"..."`
- **Comentarios:** desde `%` hasta fin de línea
- **Delimitadores:** `{ }` para bloques de geometría, `( )` para argumentos, `,` entre parámetros
- **Paths nombrados:** `&nombre` referencia a un path definido previamente

---

## 2. Gramática Central (EBNF)

```ebnf
Program     ::= Statement*
Statement   ::= ConfigStmt
              | Assignment
              | PathDef
              | StructDef
              | ForStmt
              | WithStmt
              | Placement
              | Primitive
              | Import

ConfigStmt  ::= "DisplaySize" NUMBER NUMBER
              | "WorldWindow" NUMBER NUMBER NUMBER NUMBER

Assignment  ::= ID "=" Expression
PathDef     ::= "path" ID "=" PathExpr

StructDef   ::= "struct" ID "(" [ParamList] ")" "{" Statement* "}"
ForStmt     ::= "for" ID "=" Expression "to" Expression ["step" Expression] "{" Statement* "}"
WithStmt    ::= "with" "(" ArgList ")" "{" Statement* "}"
Import      ::= "include" STRING

Primitive   ::= PrimName "(" [ArgList] ")" "{" PointList "}"
              | "TextAt" "(" ArgList ")" "{" STRING "}"

Placement   ::= PlaceName "(" ID "," [ArgList] ")" "{" PointList "}"

PathExpr    ::= "Bezier"  "{" PointList "}"
              | "Spline"  "{" PointList "}"
              | "repeat"  "(" "&" ID "," "times" "=" NUMBER ")"
              | "concat"  "(" "&" ID "," "&" ID ")"
              | "reverse" "(" "&" ID ")"
              | "normalize" "(" "&" ID ")"
              | "fitRect" "(" "&" ID ")" "{" PointList "}"

PrimName    ::= "PolyLine" | "Polygon" | "Circle" | "Rectangle"
              | "Arc" | "Dot" | "Bezier"

PlaceName   ::= "PlaceOnLine" | "PlaceOnArc" | "PlaceOnPath" | "FitRect"

ParamList   ::= ID ("," ID)*
ArgList     ::= Arg ("," Arg)*
Arg         ::= Expression | ID "=" Expression   % posicionales primero, nombrados después
PointList   ::= (Expression Expression)*

Expression  ::= Term (("+" | "-") Term)*
Term        ::= Factor (("*" | "/") Factor)*
Factor      ::= NUMBER | ID | "&" ID | FuncCall | "(" Expression ")" | "-" Factor
FuncCall    ::= ID "(" [ExpressionList] ")"
ExpressionList ::= Expression ("," Expression)*
```

---

## 3. Espacio de trabajo

```text
DisplaySize 12 8        % dimensiones físicas del canvas en cm
WorldWindow 0 24 0 16   % coordenadas de usuario: xmin xmax ymin ymax
```

Las coordenadas en todos los comandos están en unidades de usuario (WorldWindow). El compilador aplica la transformación a las unidades de salida.

---

## 4. Primitivas gráficas

Los atributos de estilo se pasan como argumentos nombrados. Los argumentos posicionales son los parámetros geométricos propios de cada primitiva. El bloque `{ }` contiene la lista de puntos.

**Argumentos de estilo comunes (todos opcionales y nombrados):**
- `color="blue"` / `color="#4080FF"` — color de trazo
- `fill="red"` / `fill="#RRGGBB"` — color de relleno (activa relleno sólido)
- `width=2.5` — grosor de línea
- `dash=2` — estilo de línea (1=sólido, 2=guiones, 3=puntos, 4=guión-punto, 5=guión-punto-punto)

### 4.1 PolyLine — polilínea abierta

```text
PolyLine(color="black", width=1) {
    0 0   5 3   10 0
}
```

### 4.2 Polygon — polígono cerrado

```text
Polygon(fill="cyan") {
    0 0   5 0   5 5   0 5
}
```

### 4.3 Circle — círculo; múltiples centros = múltiples círculos

```text
Circle(r=2)             { 5 5 }           % un círculo
Circle(r=0.3)           { 1 1  3 1  5 1 } % tres círculos
Circle(r=1, fill="red") { 0 0  10 0 }     % dos círculos rellenos
```

El bloque `{ }` contiene pares `x y` de centros. Por cada par se dibuja un círculo.

### 4.4 Rectangle

```text
Rectangle(fill="gray") { 1 1  9 7 }  % esquina inferior-izquierda, esquina superior-derecha
```

### 4.5 Arc

```text
Arc(r=3, from=0, to=180) { 5 5 }   % semicírculo superior, centro (5,5)
Arc(rx=4, ry=2, from=0, to=360) { 5 5 }  % elipse
```

### 4.6 Dot — punto sólido relleno

```text
Dot(r=0.15) { 2 3  4 3  6 3 }   % tres puntos
```

### 4.7 Bezier — curva de Bézier cúbica

```text
Bezier { p0x p0y  c1x c1y  c2x c2y  p1x p1y }
```

Los puntos son: posición inicial, control 1, control 2, posición final. Segmentos adicionales concatenan la curva.

### 4.8 TextAt — texto en posición

```text
TextAt(x, y) { "Texto normal" }
TextAt(x, y, color="red", size=14) { "Título" }
TextAt(x, y, align="center") { "$\alpha + \beta = \gamma$" }
```

El argumento `align` puede ser `"left"` (default), `"center"`, `"right"`.

---

## 5. Variables y expresiones

Ámbito léxico. Las variables definidas fuera de un bloque son globales al archivo.

```text
r_base = 5.5
margen = 2 * 0.5
n = 8

Circle(r = r_base + 1) { margen margen }
Circle(r = r_base / n) { 0 0 }
```

**Funciones matemáticas disponibles:** `sin(x)`, `cos(x)`, `tan(x)`, `sqrt(x)`, `abs(x)`, `atan2(y, x)`, `pi` (constante).

---

## 6. Loops

```text
for i = 0 to 9 {
    Circle(0.3) { i * 1.0  5 }
}

for angle = 0 to 360 step 45 {
    Dot(0.2) { 5 + 3*cos(angle)  5 + 3*sin(angle) }
}
```

- Las variables del `for` son locales al bloque y restauran el valor previo al salir.
- Loops anidados están soportados.
- El `step` puede ser negativo para contar hacia atrás.

---

## 7. Atributos con bloque `with`

Para aplicar atributos de estilo a un grupo de primitivas sin repetirlos en cada una:

```text
with(color="blue", width=2) {
    PolyLine { 0 0  10 0 }
    PolyLine { 0 5  10 5 }
    Circle(r=1) { 5 2.5 }
}
```

Los atributos en `with` se propagan a todas las primitivas del bloque. Un atributo explícito en una primitiva individual tiene precedencia sobre el `with`.

---

## 8. Structs

Una struct encapsula un bloque reutilizable. No dibuja nada hasta que se invoca. Recibe parámetros como una función. Hereda el sistema de coordenadas del punto de invocación.

```text
struct Flecha(lx, ly, size=0.2) {
    PolyLine { 0 0  lx ly }
    Polygon(fill="black") {
        lx-size*cos(atan2(ly,lx)+0.3)  ly-size*sin(atan2(ly,lx)+0.3)
        lx  ly
        lx-size*cos(atan2(ly,lx)-0.3)  ly-size*sin(atan2(ly,lx)-0.3)
    }
}

Flecha(3, 0)              % flecha horizontal
Flecha(0, 2, size=0.3)    % flecha vertical más grande
```

---

## 9. Paths como objetos de datos

Los paths son objetos nombrados que representan trayectorias geométricas. Se usan en placements (§10) y como splines para dibujo.

```text
% Definición
path sinpi = Bezier { 0 0  0.41 1.335  0.59 1.335  1 0 }

% Operaciones de álgebra de paths
path sin2pi  = repeat(&sinpi, times=2)       % concatenar consigo mismo
path rev     = reverse(&sinpi)               % invertir dirección
path norm    = normalize(&sinpi)             % normalizar al rango [0,1]
path fitted  = fitRect(&sinpi) { 1 1  9 7 } % escalar al rectángulo dado
path wave    = concat(&sinpi, &rev)          % unir dos paths

% Dibujar un path nombrado
Bezier(&sinpi)
```

---

## 10. Placements — loops implícitos sobre locus geométricos

Esta es la característica distintiva de MetaGráfica. Un *placement* repite una struct a lo largo de un locus geométrico (línea, arco, path), rotando cada instancia para alinearla con la tangente local.

### 10.1 PlaceOnLine — a lo largo de una línea

Coloca la struct en intervalos uniformes entre dos puntos, rotando cada instancia para que el eje X local apunte en la dirección de la línea.

```text
PlaceOnLine(tick, scale=0.3) { 0 0  10 0 }

% Parámetros nombrados:
% scale    — tamaño de la struct (default 1.0)
% shift    — fracción del intervalo para desplazar el inicio (default 0)
% gap      — espaciado entre instancias (default = escala)
% bothSides — true: colocar también en el lado opuesto (default false)
```

**V1:** `LNST scale [shift [n [gap]]] x1 y1  x2 y2`

### 10.2 PlaceOnArc — a lo largo de un arco

Coloca la struct a lo largo de un arco circular, rotando cada instancia en la dirección radial.

```text
PlaceOnArc(tick, scale=0.3, r=5, sweep=90, from=0) { 5 5 }

% Parámetros nombrados:
% scale    — tamaño de la struct
% r        — radio del arco
% sweep    — barrido angular total en grados
% from     — ángulo inicial en grados (0 = eje X positivo)
% shift    — desplazamiento del inicio como fracción del intervalo
% bothSides — colocar en ambos lados del arco
```

**V1:** `ARCST scale r sweep from [shift [n]] cx cy`

### 10.3 PlaceOnPath — a lo largo de un path Bézier

Coloca la struct en cada punto del path, rotando según la tangente local.

```text
PlaceOnPath(tick, scale=0.2) { &sinpi }
PlaceOnPath(tick, scale=0.2) { 0.1 0.2  0.3 0.5  0.7 0.8 }
```

**V1:** `DPST name` (usa el path actual) / `&name path`

### 10.4 FitRect — escalar struct a un rectángulo

No repite: transforma el sistema de coordenadas de la struct para que ocupe exactamente el rectángulo dado.

```text
FitRect(grid) { 0 0  10 8 }
```

**V1:** `PWST` / `PVPT`

### Cuadro comparativo V1 → V2

| V1 | V2 | Semántica |
|---|---|---|
| `LNST sc [sh [n [gap]]] p1 p2` | `PlaceOnLine(s, scale, shift, gap)` | Struct a lo largo de línea |
| `ARCST sc r da ai [sh [n]] p` | `PlaceOnArc(s, scale, r, sweep, from)` | Struct a lo largo de arco |
| `DPST name` / `&name path` | `PlaceOnPath(s, scale)` | Struct a lo largo de path |
| `PWST p1 p2` / `PVPT name p1 p2` | `FitRect(s)` | Struct ajustada a rectángulo |
| `RPPT name n` | `path x = repeat(&name, times=n)` | Repetir path (álgebra) |
| `CTPT name` + ... `CLPT` | `path x = concat(...)` | Concatenar paths |

> ⚠️ **Pendiente de decisión:** ¿El repetidor V1 `RPST n` (colocar struct actual n veces avanzando MTPP) necesita equivalente directo en V2, o se cubre con un `for` loop?

---

## 11. Texto matemático

El texto es procesado por un módulo independiente (`text_parser`) que se hereda **sin cambios** de V1. El compilador V2 extrae el string del bloque `{ "..." }` y lo pasa a `parse_text()`, que produce una `TextLine` con los chunks ya descompuestos por fuente, tamaño y nivel de script.

### 11.1 Markup del string

El contenido del string soporta el siguiente markup, procesado en parse time:

#### Modo matemático

```text
$...$    activa la fuente CMMI (Computer Modern Math Italic) para el bloque
```

```text
TextAt(5, 3) { "$\alpha + \beta = \gamma$" }
TextAt(2, 7) { "Radio $r = 2\pi / \lambda$" }
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
TextAt(3, 5) { "$x^2 + y^2 = r^2$" }
TextAt(3, 3) { "$E_n = -13.6 / n^2$ eV" }
TextAt(3, 1) { "$\psi_{nlm}(r,\theta,\phi)$" }
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
TextAt(2, 5) { "/bTítulo en negrita" }
TextAt(2, 3) { "Normal, /itálica, /rnormal otra vez" }
```

#### Agrupación

`{...}` delimita el alcance de sub/superíndices:

```text
TextAt(3, 5) { "$e^{i\pi} + 1 = 0$" }
TextAt(3, 3) { "$\sum_{n=0}^{\infty} a_n x^n$" }
```

### 11.2 Arquitectura: por qué no cambia en V2

`parse_text()` es una función pura: toma un string UTF-8 y una fuente base, devuelve un `GraphicsItem` (`Text` o `TextLine`). No depende del parser del `.mg` ni del Display. En V2:

- El parser V2 extrae el string de `TextAt(...) { "..." }`
- Lo pasa a `parse_text()` con la fuente base del contexto
- Obtiene un `GraphicsItem` listo para dibujar con cualquier Display

**Lo único que cambia** entre V1 y V2 es cómo se especifica la fuente base y el tamaño desde el archivo `.mg` (antes: `TSTYLE`, `TXSI`; ahora: argumentos de `TextAt`).

### 11.3 Argumentos de TextAt

```text
TextAt(x, y)                              % posición, fuente y tamaño del contexto
TextAt(x, y, size=12)                     % tamaño en puntos
TextAt(x, y, font="italic")               % fuente base: "roman", "italic", "bold",
                                          %   "italic-bold", "sanserif", "courier"
TextAt(x, y, align="center", color="red") % alineación y color
```

La fuente base determina el punto de partida del markup interno; los `/b`, `/i`, etc. dentro del string son relativos a ella.

> ⚠️ **Pendiente:** alineación vertical (baseline / centro / caja). Por ahora se asume baseline.

---

## 12. Importar archivos

```text
include "bzsinepaths"    % carga bzsinepaths.mg
include "estilos"
```

**V1:** `INPUT nombre`

Las structs y paths definidos en el archivo incluido quedan disponibles en el archivo que lo importa.

---

## 13. Lo que falta definir (pendientes)

| Tema | Estado | Notas |
|---|---|---|
| **Transformaciones locales** | ⚠️ Abierto | V1 tiene TLST, SCST, ROST sobre matrices MTLC/MTST. V2 necesita sintaxis. Propuesta: `transform(translate=..., rotate=..., scale=...) { }` como bloque, o como argumento de struct call |
| **Atributos heredados vs. explícitos** | ⚠️ Abierto | ¿Un `with` interno sobreescribe al externo o se combina? Necesita semántica de cascada clara |
| **bothSides en placements** | ⚠️ Abierto | El parámetro V1 `sc < 0` activa ambos lados de forma implícita. En V2 es más claro como `bothSides=true`, pero el significado geométrico exacto (perpendicular vs. especular) necesita documentarse |
| **Texto: fuentes y estilos** | ✓ Resuelto | El markup interno del string (`/b`, `/i`, `$...$`, `\alpha`, `^{}`, `_{}`) lo maneja `parse_text()` sin cambios. Los argumentos de `TextAt` cubren fuente base, tamaño y color (ver §11) |
| **Texto: alineación vertical** | ⚠️ Abierto | ¿Baseline? ¿Centro vertical? ¿Caja del texto? Por ahora se asume baseline |
| **Alcance (scope) de structs** | ⚠️ Abierto | ¿Una struct definida en un `include` es visible en el archivo padre? ¿Se puede redefinir? |
| **Manejo de errores** | ⚠️ Abierto | El compilador V2 debe reportar archivo:línea:columna en todos los errores |
| **Ejes y marcas (TICKS)** | ⚠️ Abierto | V1 tiene `TICKS n spacing offset` para generar marcas de eje con loops automáticos. Podría ser un `PlaceOnLine` especializado o un `for` loop |
| **Línea de puntos y flecha** | ⚠️ Abierto | Patrones de línea más complejos (dash patterns) — ¿van como `dash=[4,2,1,2]`? |
| **Output implícito** | ⚠️ Abierto | En V1 el nombre del output se deriva del input. ¿Cambia en V2? |
| **PlaceOnPath con repetición (RPST)** | ⚠️ Abierto | Ver §10 |

---

## 14. Estrategia de transición V1 → V2

El compilador V2 **no** mantendrá compatibilidad con la sintaxis V1. La migración es mediante un traductor Python (`mg1to2.py`) que convierte archivos V1 al nuevo formato.

### Cobertura esperada del traductor (>90% automático):

```python
# Reglas de traducción mecánica
OPST name → struct name() {
CLST       → }
MKST name  → name()          # uso sin parámetros
PL { ... } → PolyLine { ... }
CR r : ... } → Circle(r=r) { ... }
BR p1 p2   → Rectangle { p1 p2 }
LWIDTH w   → (acumular: width=w en siguiente primitiva)
LCOLOR c   → (acumular: color="c" en siguiente primitiva)
FGRAY g    → (acumular: fill=gray_to_hex(g) en siguiente primitiva)
INPUT f    → include "f"
LNST ...   → PlaceOnLine(...)
ARCST ...  → PlaceOnArc(...)
```

### Casos que requieren revisión manual:
- Uso de `MKST` + matrices MTST (los parámetros se transforman en parámetros de struct)
- `RPST n` con `SCST`/`TLST` (requiere modelado como `for` loop)
- Texto con markup complejo (`TSTYLE`, `TXSI`, subíndices/superíndices)

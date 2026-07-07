# Marcadores y Puntas de Flecha

> **Estado:** Borrador de trabajo con auditoría del motor (2026-07-06). Ver §A (lo
> ya implementado) antes de tocar el plan. Los ejemplos con sintaxis `nombre(...)`
> son gramática V3 (§1–§18 de `especificacion_mg.md`), todavía **no** implementada.

La colocación de símbolos en los vértices de una ruta (útil para gráficas de
dispersión, series de tiempo o vectores) se maneja a través de atributos de
marcador. En V1 había soporte parcial para markers (palabras reservadas con
prefijo `MK`) y para colocación de structs sobre loci (`LNST`/`ARCST`/`DPST`).

Podemos inspirarnos en el estándar SVG (`marker-start`, `marker-mid`,
`marker-end`), **pero sin duplicar** el mecanismo distintivo que ya existe:
`place(Struct)` (spec §10) coloca una struct repetida a lo largo de un locus
rotándola con la tangente local. Los `marker_*` de este documento deben quedar
como **azúcar sintáctica y marcadores nativos** montados sobre esa maquinaria, no
como un sistema paralelo.

---

## A. Auditoría del estado actual del motor (2026-07-06)

### A.1 Lo que YA está implementado (infraestructura V1 viva en `main`)

| Capacidad | Dónde | Notas |
|---|---|---|
| Marcar una struct por nombre | `MKST` → `Parser.cpp:468` (`YMKST`) | Fija el puntero `st` usado por los placements siguientes |
| Struct a lo largo de **línea** con rotación tangente | `LNST` → `StructureLine::draw_side` (`structure.cpp:60-86`) | `rt = atan2(dy,dx)`, `mtln.rotate(rt)` → **`orient="auto"` ya resuelto para líneas** |
| Struct a lo largo de **arco** con rotación tangente | `ARCST` → `StructureArc::draw_side` (`structure.cpp:104-138`) | Rota tangente ±90°; soporta `shift`, `both_sides` |
| Struct a lo largo de **path** | `DPST`/`RPST` → `StructurePath::draw` (`structure.cpp:33-47`) | **Solo `translate`+`scale`, NO rota** ← hueco principal |
| Struct ajustada a rectángulo | `PWST`/`PVST` → `StructureRectangle::draw` (`structure.cpp:49-57`) | `fit` en V3 |
| Parámetros de colocación | `scale` (`sc`), `shift`, `gap`, `n` (repeticiones), `both_sides` (`sc<0`) | Ya parseados en `Parser.cpp` (YLNST/YARCST/YRPST) |
| Structs personalizadas como marcador | vía `MKST`/`&name` + placement | Ya soportado; el §B.B del plan original ya funciona en V1 |

**Conclusión:** el caso "flecha orientada al final de una línea" y "struct rotada
sobre un arco" **ya se pueden hacer hoy** con `MKST`+`LNST`/`ARCST`. Lo que falta
es (1) rotación tangente sobre *paths/curvas*, (2) marcadores nativos por nombre y
(3) la ergonomía de adjuntarlos como atributo de la primitiva que dibuja.

### A.2 Modelo de atributos: máquina de estados por flujo

Los atributos **no** son bolsas por primitiva. Son `GraphicsItem`s de tipo
`Attribute` (`primitives.h:240`) con un `AttributeType` (`AT_LWIDTH`, `AT_LCOLOR`,
`AT_FCOLOR`, `AT_FPATRN`…) que se emiten al flujo y el `Display` los aplica como
estado. Consecuencia para el plan: `marker_end="arrow"` **no** puede ser un campo
de `Polyline`; debe modelarse como estado de marcador que la primitiva de ruta
consulta al dibujarse, o bien como una expansión en el parser a placements
(ver §D, dos estrategias).

### A.3 Tokens V1 vestigiales

`MKMR` (`YOPS`) y `MR` (`YOP, GI_NULL`, sin primitiva asociada) siguen en
`keyword_map` (`mgpp.l:44,46`) sin uso real. Al diseñar la gramática V3 conviene
**retirarlos** (mueren con el front-end V1) o reciclar la idea de `MR` (marker) en
el nuevo comando. No invertir en ellos.

### A.4 Lo que NO existe todavía

- Marcadores estándar nativos por cadena (`"arrow"`, `"circle"`, `"square"`,
  `"diamond"`, `"cross"`, `"x"`).
- Atributos `marker_start` / `marker_mid` / `marker_end` / `marker` (shorthand).
- Rotación tangente sobre `StructurePath` (paths y curvas Bézier).
- La sintaxis de invocación V3 `polyline(...)` (gramática §1–§18 sin arrancar).
- Marcadores nativos en `<defs>` de SVG.

---

## B. Diseño propuesto (V3)

### B.1 Atributos de posición

Cualquier primitiva que genere una ruta (`polyline`, `polygon`, `spline`,
`bezier`, `path`) acepta:

- `marker_start` — solo el primer vértice (colas de vector).
- `marker_mid` — todos los vértices salvo primero y último (puntos de una serie).
- `marker_end` — solo el último vértice (puntas de flecha).
- `marker` — atajo: aplica el mismo a start, mid y end.

Semántica de cascada: siguen la regla de §7 de la spec (primitiva > `with`
interno > `with` externo), igual que `color`/`width`.

> **Refinamiento (2026-07-07, al traducir fig2-3 y revisar fig1 del artículo original).**
> El **tamaño** de un marcador es **cantidad física** (unidades de dispositivo), inmune a la
> ventana y a las transformaciones; solo su **posición** se transforma. Es obligatorio para
> graficar datos: un marco de datos usa `stretch` (spec §13.7) y ahí todo tamaño de *mundo* se
> deforma —`circle` saldría elíptico, y una struct colocada con `place`/`scale` también, porque su
> tamaño es de mundo—. Consecuencia: aunque los marcadores estándar se resuelvan como structs
> internas, su **render de tamaño NO puede ir por el escalado de mundo de `place`**; debe ser
> físico, como el de `dot` (spec §4.6; verificado: centro transformado, tamaño en dispositivo).
> `place`/`StructureLine` aportan **posición y orientación (tangente)**, no tamaño. `dot` es el
> caso base físico (dispersión de puntos); la familia `marker` es su versión rica (formas +
> start/mid/end) y comparte ese render físico. Evidencia de corpus: fig1 del artículo original
> dispersa **puntos-en-círculo (⊙) y cruces (+)** sobre una curva; fig2-3, círculos abiertos.

### B.2 Tipos de marcadores

**A. Marcadores estándar (cadenas).** Nativos, tamaño base proporcional al
`width` de la línea; heredan `color` y `fill`:

`"circle"`, `"square"`, `"diamond"`, `"cross"` (+), `"x"`, `"arrow"`.

```text
% Vector (0,0)→(10,10) con flecha al final
polyline(width=2, color="red", marker_end="arrow") { 0 0  10 10 }

% Serie con punto en cada vértice
polyline(marker="circle") { 0 0  5 8  10 2  15 9 }
```

Implementación: los estándar se definen **como structs internas del compilador**
(no como código especial por backend). Es decir, `"arrow"` es una struct
predefinida en la biblioteca base (equivalente a la `Flecha` de §8 de la spec).
Así el marcador estándar y el personalizado recorren el **mismo camino** de
`place`/`StructureLine`, y los tres backends no necesitan lógica nueva de dibujo,
solo la de colocación que ya existe. Se resuelven por nombre: primero se busca en
la tabla de estándar, si no, se asume struct del usuario.

Draft verificado de los 6 (fuente completa en `examples/markers.mg`), centrados en
el origen y sin color:

```text
OPST MkCircle    FILL CR 1 : 0 0 } CLST                                % círculo
OPST MkSquare    FILL PL -1 -1  1 -1  1 1  -1 1  -1 -1 } CLST          % cuadrado
OPST MkDiamond   FILL PL 0 -1  1 0  0 1  -1 0  0 -1 } CLST             % rombo
OPST MkCross     PL -1 0  1 0 } PL 0 -1  0 1 } CLST                    % cruz +
OPST MkX         PL -1 -1  1 1 } PL -1 1  1 -1 } CLST                  % equis x
OPST MkArrow     FILL PL -1 -0.7  1 0  -1 0.7  -1 -0.7 } CLST          % flecha +x
```

**B. Marcadores personalizados (structs).** Ya soportado conceptualmente en V1:
cualquier struct del usuario sirve como marcador pasando su nombre.

```text
struct MiPunto() {
    circle(fill="blue", width=0.5) { 0 0 }
    polyline(color="white") { -1 0  1 0 }
}
polyline(marker_mid="MiPunto") { ... }
```

### B.3 Orientación (`marker_orient`)

- `"auto"` (default para `"arrow"`): rota con la tangente local. **Ya
  implementado** para líneas (`StructureLine`) y arcos (`StructureArc`); falta
  para paths/curvas (ver §C.1).
- `"fixed"` (default para `"circle"`, `"square"`…): mantiene la orientación
  original. Correcto para dispersión.

El default de `orient` depende del marcador (arrow→auto, resto→fixed) pero es
sobreescribible.

```text
bezier(marker_mid="arrow", marker_orient="auto") { ... }
```

### B.4 Color del marcador (`marker_color` / `marker_fill`) — DECIDIDO

**Pregunta:** ¿y si el marcador debe ser de un color y la curva de otro (p. ej.
puntos negros sobre una senoide roja)?

**Respuesta (verificada empíricamente 2026-07-06):** ya funciona por construcción.
`Display::structure()` (`Display.cpp:19-24`) dibuja cada struct en un **ámbito
aislado de estado gráfico** (apila `dspstate` antes y lo restaura después,
reforzado por el `gsave/grestore` que emiten `StructureLine`/`StructurePath`). Por
tanto:

- Un marcador que **no** fija color → hereda el color vigente de la curva.
- Un marcador que fija su color adentro → dibuja en ese color y **no lo filtra** de
  vuelta a la curva.

Prueba: colocar un struct con `LCOLOR blue` interno sobre una línea `LCOLOR red`
emite `1 0 0` (línea), `0 0 1` (marcador) y restaura `1 0 0` para el siguiente
trazo, sin fuga. Es independiente del backend (EPS/SVG/PDF).

**Decisiones de diseño:**

1. Las structs `Mk*` **builtin no fijan color**: dibujan con el estado vigente
   (ver §C.2 y `examples/markers.mg`). Así son neutrales y reutilizables.
2. Se añaden los canales **`marker_color`** (trazo) y **`marker_fill`** (relleno),
   más `marker_gray` si aplica. Semántica de cascada:
   - sin `marker_color` → el marcador **hereda** el `color`/`fill` de la curva
     (default cómodo, puntos que combinan con la línea);
   - con `marker_color="black"` → el parser emite `LCOLOR/FCOLOR` del marcador
     **justo antes** de la colocación aislada; el scoping garantiza que la curva
     queda intacta.
3. Como los marcadores sólidos usan `fillcolor` y los de trazo usan `linecolor`,
   `marker_color` debe fijar **ambos** (emitir `LCOLOR` y `FCOLOR`) para que un
   mismo valor funcione en cualquier marcador; `marker_fill` permite separarlos
   cuando se quiera relleno ≠ contorno.

```text
% curva roja, puntos negros — el motor los mantiene separados solo
polyline(color="red", marker="circle", marker_color="black") { ... }
```

> Más flexible que SVG, donde por default los `<marker>` **no** heredan el color de
> la línea (requieren `context-stroke`). Aquí el default es heredar y el override es
> explícito; ambos son triviales gracias al scoping de structs.

---

## C. Plan de implementación en el MOTOR

### C.1 Rotación tangente sobre paths/curvas — IMPLEMENTADO (2026-07-06)

`StructurePath::draw` antes no rotaba. Ya calcula la tangente por vértice y rota
cada instancia (`structure.cpp`):

- Vértice interior `i`: diferencia central `path[i+1] − path[i-1]`.
- Primer vértice: `path[1] − path[0]`. Último: `path[n-1] − path[n-2]`.
- Composición en orden **T·R·S**: `translate(p)`, `rotate(atan2(d.y,d.x))` (solo si
  `orient`), `scale`. Mismo patrón que `StructureLine::draw_side`.

`StructurePath` ganó `setOrient(bool)` + miembro `orient` (default `false`, marcador
"derecho"). Cableado en los 4 sitios de construcción (DPST, RPST ×2, identificador).

**Disparador temporal V1:** el control `$O 1` / `$O 0` (en `Parser::parseDef`) fija
`orient_next`, que cada `StructurePath` lee al construirse. Es **provisional** para
probar §C.1 en aislamiento; morirá con la capa de atributos V3 (`marker_orient`).

**Verificación:** `MkArrow` colocado sobre `0 2  3 5  6 5  9 2` con `$O 1` produce
flechas a 45°, 26.6°, −26.6°, −45° (coincide con las tangentes exactas); con `$O 0`
todas a 0°, byte-idéntico al comportamiento previo. Funciona en EPS/SVG/PDF (la
lógica vive en `structure.cpp`, independiente del backend). Ningún ejemplo existente
regresó.

**Pendiente (refinamiento):** para curvas Bézier reales la tangente por cuerda es
una **aproximación**; la exacta es la derivada del segmento en el nodo (usar los
puntos de control adyacentes vía `splines.cpp`). Suficiente para polilíneas y series
de datos; se refina después. Con esto, `place(Struct){ &curva }` ya orienta —lo que
la spec §10.1 promete— cubriendo el caso "flechas que siguen la curva".

### C.2 Marcadores estándar como structs predefinidas — DECIDIDO

**Decisión:** los 6 marcadores estándar se definen **en el propio lenguaje** (no
como árbol de `GraphicsItem` en C++) y se **embeben como fuente `.mg` compilada al
arranque** dentro del binario.

- *Por qué embebido y no C++ a mano:* construir polilíneas como `GraphicsItem` en
  C++ es verboso y propenso a error, y duplica lo que el parser ya hace. La fuente
  `.mg` es inspeccionable, editable y recorre exactamente el mismo camino
  (`place`/`StructureLine`) que un marcador de usuario.
- *Por qué embebido y no solo un archivo en disco:* deben existir aunque no haya
  instalación. Implementación: empujar un buffer `std::istringstream` con la fuente
  (el lexer ya toma `std::istream*` y apila buffers vía `yypush_buffer_state`, igual
  que `INPUT`) antes del archivo del usuario, registrando las structs en
  `structure_map`.
- **Draft listo y verificado:** `examples/markers.mg` (los 6, centrados en el
  origen, sin color). Todos renderizan sin error en el motor actual; será la fuente
  a embeber.
- **Escalado proporcional al `width`:** el `scale` del placement se deriva del
  ancho de línea vigente (`AT_LWIDTH`) por un factor configurable.
- **Color:** las structs builtin **no** fijan color; el color se controla con
  `marker_color`/`marker_fill` o se hereda de la curva (ver §B.4). Corrige la nota
  previa de que "heredan color y fill... sale gratis": eso es el default, pero el
  override de color es una necesidad real y se resuelve en §B.4.

**Complementario — ruta de búsqueda para `INPUT`.** `openFile` (`mgpp.l:106`) abre
el nombre tal cual con `ifstream`, sin ruta de búsqueda: `INPUT arrow`/`INPUT
markers` solo funcionan desde el directorio del archivo. Añadir un fallback
(`$MG_LIB` → directorio del ejecutable → default de instalación) arregla las
bibliotecas de usuario compartidas. Es **ortogonal** a los builtin (esos van
embebidos), pero conviene hacerlo a la vez.

> **Nota de nombres (V1 vs V3):** el lexer V1 define identificadores como
> `{LETRA}{DIGILET}+`, **sin guión bajo**, así que `__marker_*` no compila hoy. El
> draft usa el prefijo capitalizado `Mk` (`MkCircle`, `MkArrow`…). En V3 el usuario
> invoca por cadena (`"circle"`) y el resolvedor mapea a estas structs; si V3 admite
> `snake_case` en identificadores (la spec §1 lo usa para keywords), reconsiderar el
> prefijo interno. Retirar además `MKMR`/`MR` vestigiales (§A.3).

**Biblioteca embebida hermana — segmentos de curva (bzsinepaths).** El mismo
mecanismo de embebido de §C.2 debería hospedar una **biblioteca builtin de
segmentos de curva**: los paths Bézier de seno/coseno de `examples/v1/bzsinepaths.mg`
(`&sinpi`, `&cospi`, `&sin2pi`, …), aproximaciones reutilizables sobre intervalos
estándar. Es el análogo en *paths* de los marcadores estándar (structs): el usuario
obtiene curvas matemáticas sin `INPUT`, y con `concat` arma ondas largas. En V3 son
objetos `path` (§9); embeberlos = registrarlos en `listmap` al arranque, igual que
los `Mk*` en `structure_map`.
>
> **Estado de `concat` (verificado 2026-07-06):** `concat_paths` (`splines.cpp`)
> funcionaba solo si el frente de cada segmento estaba en x=0 (precondición no
> documentada); se corrigió la alineación en x (era asimétrica con la y). La
> semántica de empalme ya está especificada en §9 de la spec (`join=true` default =
> C0 por traslación, `join=false` = crudo, sin auto-reversión, variádico). Pendiente
> para V3: continuidad C1 (tangente) en los empalmes —hoy solo C0—, p. ej.
> `join="smooth"`.

### C.3 Emisión: dos estrategias (decisión abierta ⚠️)

**Estrategia 1 — Expansión en el parser (recomendada).** Al parsear una primitiva
de ruta con `marker_*`, el parser, tras emitir la ruta, emite placements
equivalentes (`StructureLine`/`StructurePath` con el struct del marcador) sobre los
vértices correspondientes. Ventajas: cero código nuevo en los backends; reutiliza
toda la maquinaria de `structure.cpp`; SVG/EPS/PDF ya saben dibujar structs
colocadas. Desventaja: el SVG generado no usa `<marker>` nativo (más verboso pero
correcto).

**Estrategia 2 — Marcadores nativos por backend.** `Display` gana estado de
marcador (`setMarker(pos, name, orient)`) y cada backend decide: SVG emite
`<defs><marker>` + atributos `marker-start/mid/end` (trivial, delega al
navegador); EPS/PDF hacen el `save/translate/rotate/structure/restore` por vértice
(el §"Impacto" original). Ventaja: SVG idiomático y compacto. Desventaja: triplica
la lógica y rompe la simetría con `place`.

**Recomendación:** empezar con la Estrategia 1 (funciona en los tres backends con
el motor actual y es consistente con `place`). Considerar la Estrategia 2 solo
para SVG como optimización posterior, detrás de la misma interfaz.

### C.4 Impacto por backend (si se toma la Estrategia 2)

- **SVGDisplay:** define el marcador en `<defs>` y usa `marker-start/mid/end`;
  el navegador hace la rotación (`orient="auto"`) y el escalado
  (`markerUnits="strokeWidth"`). Casi trivial.
- **EPSDisplay / PDFDisplay:** PostScript/PDF no tienen marcadores nativos. Por
  cada vértice: `save()`, `translate(x,y)`, `rotate(tangente)` (si `auto`), llamar
  a la struct del marcador, `restore()`. Es exactamente lo que `StructureLine::
  draw_side` ya hace — de ahí que la Estrategia 1 sea preferible.

---

## D. Plan en la GRAMÁTICA (V3)

1. **Reservar los nombres estándar** de marcador como cadenas reconocidas por el
   resolvedor de structs (no como keywords del lexer): `"arrow"`, `"circle"`,
   `"square"`, `"diamond"`, `"cross"`, `"x"`. Prefijar sus structs internas (`Mk*`
   hoy; `__marker_*` solo si V3 admite guión bajo, ver §C.2) para no chocar con el
   espacio global de nombres del usuario (§15).
2. **Añadir los argumentos** `marker_start`, `marker_mid`, `marker_end`, `marker`,
   `marker_orient` a la lista de atributos válidos de las primitivas de ruta, en el
   mismo mecanismo de parseo de `color=`/`width=` (cuando exista la gramática V3
   de invocación `nombre(args) { locus }`).
3. **Retirar** `MKMR` y `MR` del `keyword_map` (mueren con el front-end V1) o
   documentar su reciclaje. Registrar en `mg1to2.py` la traducción
   `MKST X` + `LNST … ` → `place(X, …)` (spec §10, tabla de §10.3) y, cuando el
   V1 usara markers de prefijo `MK`, mapear al `marker_*` equivalente.
4. **Relación con `place`:** documentar que `polyline(marker_end="arrow"){p1 p2}`
   es azúcar de `polyline{p1 p2}` + `place(Arrow, shift=1){p1 p2}`. Mantener una
   sola semántica de rotación tangente compartida.

---

## E. Orden de trabajo sugerido

0. **§C.2 — draft de los 6 marcadores: ✓ HECHO.** `examples/markers.mg`, centrados
   en el origen, sin color; los 6 renderizan sin error. Falta embeberlos al arranque
   y el resolvedor por cadena.
1. **§C.1 — tangente en `StructurePath`: ✓ HECHO.** Rota con la tangente si
   `orient`; disparador temporal `$O`. Pendiente: derivada exacta de Bézier.
2. **§C.2 (resto)** — embeber `markers.mg` como fuente compilada al arranque +
   escalado por `width`; de paso, ruta de búsqueda de `INPUT` (`$MG_LIB`).
3. **§D** — cablear los atributos `marker_*` (incluidos `marker_color`/
   `marker_fill`, §B.4) en la gramática V3 (depende de que la gramática de
   invocación exista; ver roadmap §22.6).
4. **§C.3 Estrategia 1** — expansión en el parser; validar en los tres backends con
   un ejemplo nuevo (candidato: una serie de dispersión + un vector con flecha),
   añadido a `EXAMPLES` en `test/run.sh` y bendecido como golden.
5. (Opcional) **§C.3 Estrategia 2** para SVG nativo.

Casos guía a cubrir: (a) línea/curva que termina en flecha orientada;
(b) marcadores fijos en cada punto de una ruta (dispersión). Ambos deben verse
idénticos en EPS, SVG y PDF salvo el `<marker>` nativo de SVG.

> **Decisión de secuenciación (2026-07-06): PAUSA hasta la gramática V3.** Los
> pasos 2–5 (embeber `markers.mg`, ruta `INPUT`, atributos `marker_*`, emisión) se
> **posponen** hasta que arranque la gramática V3 (§1–§18). Razón: todo el corpus
> en sintaxis V1 —incluidos `examples/markers.mg` y `examples/markers-demo.mg`— se
> vuelve obsoleto en cuanto empiece la implementación de V3; invertir ahora en más
> infraestructura de marcadores sobre V1 es trabajo desechable. Lo ya hecho que
> **sobrevive**: §C.1 (rotación tangente en `StructurePath`, código de motor
> agnóstico de la gramática) y este plan. Lo **provisional** (`markers.mg`,
> `markers-demo.mg`, disparador `$O`) fue solo validación de diseño y se reescribe
> con V3.

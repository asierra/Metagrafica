# Plan: patrones de relleno (tramado) — modelo nombrado, ángulo libre, crosshatch

Evolución del sistema de tramado de área en los tres backends (EPS, PDF, SVG):
de la representación por **índice entero** (legado FPATRN de V1, restringida) al
modelo **`hatch` sobrecargado** (nombre o número), con **ángulo/gap libres** y
**estilos nombrados** (`hatch`/`hatchback`/`crosshatch`), en paralelo a cómo
`dash` acepta alias.

Estado del documento: **plan aprobado, listo para delegar.** La base index-based
está terminada (ver §1); lo nuevo es §3 (fases 1–4). Decisiones cerradas por
Alejandro (2026-07-09): sobrecargar `hatch`, relajar el ángulo, catálogo estilo
Asymptote.

---

## 1. Estado actual (punto de partida — TERMINADO)

El tramado ya funciona en **los tres backends** como **rayado de una sola familia**,
representado por un **índice entero** `dspstate.fillpattern` (0 = sólido):

- **Modelo común** en `DisplayState` (`include/Display.h`): `fill` (bool),
  `fillcolor`/`fillgray`, **`fillpattern` (int)**, `outlinefill` (bool). El bbox del
  path (`xmin..ymax` + `set_limits`/`adjust_limits`) vive en la base `Display` y lo
  alimentan los tres backends.
- **Resolución del índice**: `Display::patternFor(int idx)` (`Display.h:~223`)
  devuelve un `FillPattern` = `std::vector<HatchLine>` a partir de una tabla
  **aritmética restringida** (ángulo ∈ {0,45,90,135}, gap ∈ {4,2,1} pt → idx 1..10;
  el 1:1 con `FPATRN` de V1). `HatchLine{angle, gap, ox, oy, dashes}` ya admite
  **ángulo y gap arbitrarios** y `dashes`; `FillPattern` admite **varias familias**.
- **Render por backend** (los tres **iteran** `patternFor(...)`):
  - **EPS** — `useFillPattern()` (`EPSDisplay.cpp:378`) emite una llamada
    `hatch<ángulo>` por familia; el prólogo `ps_hatchers` (`EPSDisplay.cpp:~49`)
    tiene **4 procedimientos fijos** `hatch0/hatch45/hatch90/hatch135`, cada uno
    `gsave→clip→líneas paralelas sobre el bbox→stroke→grestore`.
  - **SVG** — `ensureHatchPattern(int idx)` (`SVGDisplay.cpp:94`) emite
    perezosamente un `<pattern id="mgpat_<idx>_<color>" patternUnits="userSpaceOnUse"
    patternTransform="rotate(angle-90)">` con **una sola** `<line>` horizontal
    (`fp[0]`), dedup por id en `emitted_patterns`. **Solo una familia** (el propio
    código lo anota como pendiente).
  - **PDF** — `hatchCurrentPath()` (`PDFDisplay.cpp:185`) itera `fp`, usa el operador
    `W` (clip sin consumir path) + dibuja las líneas sobre el bbox; libharu no expone
    tiling patterns, por eso el mismo truco clip+líneas que EPS. `ensurePatternGSave()`
    abre el GSave en PAGE_DESCRIPTION (libharu prohíbe GSave en PATH_OBJECT).
- **Parser** (`src/parserv3.cpp`): `emitHatch(angle, gap, out)` llama
  `hatchIndex(angle, gap)` → índice FPATRN, y emite `Attribute(AT_FPATRN, idx)` +
  `GS_FILL`. Lo usan el atributo por-primitiva (`emitPrimStyle`, `hatch=`/`hatch_gap=`)
  y la sentencia de estado (`StateStmt::exec`, `hatch <áng> [gap]`).

**El cuello de botella**: la representación es un **`int`**. `Attribute` solo carga
`int value` (+ `double fvalue`), así que no puede transportar un `FillPattern` ni una
tupla (estilo, ángulo, gap). Y `patternFor` restringe ángulo/gap a la tabla FPATRN.
Todo el plan nuevo consiste en **sustituir esa representación entera por un
`FillPattern`** que viaje por el pipeline, dejando intacta la capa de render (que ya
sabe dibujar N familias a ángulo arbitrario).

*(Nota: el front-end V1 está congelado en `v1-legacy`; `patternFor`/`hatchIndex`/tabla
FPATRN quedan muertos para V3 y se pueden borrar en la Fase 4.)*

---

## 2. Objetivo (modelo nuevo)

`hatch` **sobrecargado**, exactamente en paralelo a `dash` (que acepta alias o
—en la spec— arreglo explícito):

- **Número** = ángulo de **una** familia: `hatch=30` (ahora **libre**, sin restringir
  a {0,45,90,135}).
- **Cadena** = **estilo nombrado**:
  - `"hatch"`      → una familia a **45°**.
  - `"hatchback"`  → una familia a **135°** (−45°).
  - `"crosshatch"` → **dos** familias (45° + 135°).
- **`hatch_gap`** = separación en pt (**libre**, sin restringir a {4,2,1}).
- **Color** de las líneas = `fill`; **grosor** = `line_width` (§4.11, ya decidido).

Formas (idénticas a `dash`): atributo por-primitiva y sentencia de estado.

```text
polygon(hatch="crosshatch", hatch_gap=3) { … }   % rejilla, paso 3 pt
polygon(hatch="hatchback")               { … }   % diagonal inversa (135°)
polygon(hatch=30)                        { … }   % una familia a 30° (ángulo libre)
hatch "crosshatch"                                % estado
hatch 30 2                                        % estado: ángulo 30, gap 2
```

Catálogo mínimo (hatch/hatchback/crosshatch, à la Asymptote); ampliar
(hlines/vlines/ladrillo/punteado) solo cuando el corpus lo exija.

---

## 3. Plan de implementación (fases delegables)

Cada fase es **verificable de forma independiente** (build limpio + los 14 ejemplos
a EPS/SVG/PDF + `gs -dNOPAUSE -dBATCH -sDEVICE=nullpage` sin error). El harness
golden está en pausa (migración examples/v3→examples/), así que la verificación es
por render + GS + inspección dirigida del SVG/PDF, no golden.

### Fase 1 — Plomería: mover la representación de `int` a `FillPattern` (SIN cambio de salida)

Objetivo: que el patrón viaje como `FillPattern` y se guarde en `dspstate`, con la
salida **byte-equivalente** a hoy (misma tabla restringida por ahora).

1. **`dspstate`** (`include/Display.h`): añadir `FillPattern hatch;`. Mantener
   `int fillpattern` **solo como bandera** ("hay trama" = `fillpattern != 0`) o
   sustituir sus usos por `!dspstate.hatch.empty()` (decidir en implementación;
   preferible la bandera para tocar menos).
2. **`Display::setHatch(const FillPattern&)`** (nuevo): fija `dspstate.hatch` y la
   bandera. (Análogo a `setLineStyle`, pero recibe el patrón ya construido, no un
   índice.)
3. **Nuevo `GraphicsItem` `HatchAttr`** (`include/primitives.h` + `src/primitives.cpp`):
   subclase que carga un `FillPattern` y en `draw(Display&)` llama `g.setHatch(fp)`.
   Necesario porque `Attribute` (int/float) no puede cargar el payload. Añadir
   `GI_HATCHATTR` al enum (al final, no desplazar ordinales) si el despacho lo pide;
   `HatchAttr` implementa su propio `draw`, no pasa por `Attribute::draw`.
4. **Parser** (`emitHatch` en `parserv3.cpp`): en vez de `Attribute(AT_FPATRN, idx)`,
   construir el `FillPattern` (por ahora, la misma familia 45°/gap que hoy vía
   `hatchIndex`→tabla, o directamente `{HatchLine{angle, gap}}`) y emitir un
   `HatchAttr(fp)` + `GS_FILL`. `emitPrimStyle` y `StateStmt::exec` no cambian de
   forma (siguen llamando a `emitHatch`).
5. **Backends**: `useFillPattern`(EPS)/`ensureHatchPattern`(SVG)/`hatchCurrentPath`(PDF)
   leen `dspstate.hatch` en lugar de `patternFor(dspstate.fillpattern)`. (Mínimo:
   sustituir la fuente del `for (const HatchLine& …)`.)
6. **VERIFICAR**: `fill_styles.mg` y `primitives.mg` renderizan **igual que antes**
   (mismo tramado, mismos ángulos/densidades) en los 3 backends. Es un swap de
   representación; la geometría no cambia.

### Fase 2 — Ángulo libre (una familia)

Objetivo: `hatch=<ángulo cualquiera>` renderiza correctamente en los 3 backends.

7. **EPS — punto más delicado.** Reemplazar los 4 procs fijos `hatch0/45/90/135` del
   prólogo (`ps_hatchers`) por **un `hatch` genérico parametrizado por ángulo**:
   `... gap xmin ymin xmax ymax angle hatch` → `gsave` → `clip` → rotar el sistema
   (`angle rotate`) → barrer líneas paralelas que cubran el bbox rotado → `stroke` →
   `grestore`. Cuidar que el barrido cubra el bbox tras la rotación (usar la diagonal
   como extensión, como ya hace PDF). `useFillPattern()` pasa el ángulo de cada
   `HatchLine`.
8. **SVG** (`ensureHatchPattern`): llavear el `id` por **(ángulo, gap, color)** en vez
   de `idx` (línea `sprintf(idbuf,"mgpat_%d_%s",...)`) — p. ej. `mgpat_<angʹ>_<gapʹ>_<color>`
   redondeando a enteros/entero-milésimas para el id. El `patternTransform="rotate(angle-90)"`
   ya sirve para **una** familia a cualquier ángulo; solo hay que dejar de derivar el
   ángulo de la tabla.
9. **PDF** (`hatchCurrentPath`): verificar que la matemática de dirección de línea
   acepte ángulo arbitrario (probablemente ya, porque parte del ángulo del `HatchLine`
   y clip+líneas es genérico). Ajustar si asume múltiplos de 45°.
10. **VERIFICAR**: `polygon(hatch=30, hatch_gap=3)` a 30° en EPS (GS ok + ángulo
    visible), SVG (`rotate` en el `<pattern>`), PDF (líneas a 30°). Comparar los tres.

### Fase 3 — Estilos nombrados + crosshatch (multi-familia)

Objetivo: `hatch="hatch"|"hatchback"|"crosshatch"` (y estado equivalente).

11. **Parser** (`emitHatch` / lectura del valor de `hatch`): distinguir **cadena vs
    número**. Cadena → construir el `FillPattern` del alias:
    `"hatch"`→`{HatchLine{45,gap}}`, `"hatchback"`→`{HatchLine{135,gap}}`,
    `"crosshatch"`→`{HatchLine{45,gap}, HatchLine{135,gap}}`. Número → una familia a ese
    ángulo (Fase 2). En `emitPrimStyle`, `hatch` puede llegar STRING o NUMBER (ver
    `Value::type`). En `StateStmt::exec` (que ya trata `hatch` posicional) aceptar que
    el primer arg sea cadena (`hatch "crosshatch"`) además de número (`hatch 30 2`).
12. **SVG** (`ensureHatchPattern`): emitir **una `<line>` por familia** dentro del
    tile. El truco de `patternTransform` único **no** sirve para 2 ángulos distintos:
    para crosshatch, construir el tile con las 2 familias dibujadas **a su ángulo real**
    (calcular extremos de cada línea dentro del tile cuadrado de lado `gap`), sin
    `patternTransform`, o emitir dos `<pattern>` superpuestos. Preferible: un tile con
    ambas familias. **EPS y PDF ya iteran `for (h : fp)`** → multi-familia sale casi
    gratis ahí (cada familia = una llamada `hatch`/un barrido).
13. **VERIFICAR**: `hatch="crosshatch"` y `"hatchback"` en los 3 backends; que
    crosshatch se vea como rejilla en SVG (no una sola dirección).

### Fase 4 — Cierre

14. **Spec §4.11** (`especificacion_mg.md`): reescribir para el modelo nombrado +
    ángulo/gap libres; documentar el paralelo con `dash` (nombre o número); jubilar la
    tabla FPATRN y la restricción de ángulos. Mantener la nota de que color=fill y
    grosor=line_width.
15. **`fill_styles.mg`**: añadir una fila mostrando `hatch`/`hatchback`/`crosshatch`
    (y quizá un ángulo libre), rotulada con la concatenación/`str` ya disponibles.
16. **Limpieza**: borrar `patternFor`/`hatchIndex`/`AT_FPATRN`/tabla FPATRN si quedaron
    muertos (V1 congelado en v1-legacy). Actualizar la memoria del proyecto
    (`project_mgv3_next.md`): marcar "hatch libre" (pendiente #2) como resuelto.

### Riesgos / notas para quien implemente
- **EPS prólogo (paso 7)** es el mayor riesgo: pasar de 4 procs fijos a 1 genérico con
  `rotate`. Aislable; verificar con GS y comparando ángulos contra SVG/PDF.
- **SVG crosshatch (paso 12)**: el tile con 2 familias es geometría acotada pero no
  trivial; cuidar el "wrap" de las líneas en los bordes del tile para que la rejilla
  sea continua.
- **PDF** es de verificación por vista (timestamps varían); basta con que no emita
  errores libharu (INVALID_GMODE) nuevos y que las líneas salgan al ángulo pedido.
  *(Preexistente: `fig2-3.pdf` emite ~30 INVALID_GMODE ajenos a patrones; no regresión.)*
- Fase 1 **no cambia la salida** — es la red de seguridad; si algo se ve distinto ahí,
  parar y revisar el swap antes de seguir.

---

## 4. Referencias (contexto, condensado)

Convención semántica del rayado técnico: **ISO 128-50** (líneas finas continuas,
preferente 45°, separación proporcional; partes adyacentes se distinguen por
ángulo/separación) — es la lógica que el sistema imita; **ASME/ANSI Y14.2**
(*section lining*), **DIN 201**. Formato concreto: el **`.pat` de AutoCAD** modela
un patrón como lista de familias `ángulo, ox,oy, Δx,Δy [,dashes]` — exactamente la
estructura de `HatchLine`/`FillPattern` que ya usa el motor (por eso crosshatch y
punteado ya son representables). Para los **nombres user-facing** se eligió el juego
minimalista de **Asymptote** (`hatch`/`hatchback`/`crosshatch`), no los índices
ANSI31…, por coherencia con la simpleza de los alias de `dash`.

---

## 5. (Parqueado) Propiedad global de resolución (dpi)

Idea separada de Alejandro, **no** en el camino crítico de patrones. El pipeline
vectorial (EPS/PDF/SVG) es resolución-independiente (`cm→pt` exacto); **dpi es un
concepto de trama**, así que la separación de patrones debe expresarse en **longitud
física** (mm) o **densidad** (líneas/cm), no en dpi —hoy el `hatch_gap` ya está en pt
(longitud física), solo falta —si se quiere— exponerlo en unidades de usuario. dpi
pasaría a primera clase el día que exista un **backend de trama** (PNG), y como
*default* de la tolerancia de planitud de splines (`splines.cpp` usa un `intervals`
fijo; lo correcto sería subdividir por tolerancia de cuerda ≈ longitud/tol). Decisión
de diseño pendiente; retomar cuando se aborde el backend de trama o los splines.

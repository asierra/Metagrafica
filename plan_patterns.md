# Plan: rellenos de área (patrones de tramado) y resolución

Diseño de una solución general para los rellenos de área en los tres backends
(EPS, PDF, SVG), y discusión de una propiedad global de **resolución (dpi)** que
afecta también al proyecto de splines.

Estado del documento: **en progreso**. Pasos 0 y 1 hechos (ver §4.5); dpi
descartado por ahora (§5).

---

## 1. Cómo están hechos los rellenos hoy

El modelo de relleno vive en `DisplayState` (`include/Display.h`) y es común a
los tres backends:

| Campo | Significado |
|---|---|
| `fill` (bool) | ¿se rellena el área? |
| `fillcolor` / `fillgray` | color sólido de relleno |
| `fillpattern` (int, 0 = sólido) | índice de patrón de tramado |
| `outlinefill` (bool) | además de rellenar, dibujar el contorno |
| `max_fillpattern` | tope por backend |

El parser normaliza el índice con `value % (max_fillpattern+1)`, así que si un
backend declara `max_fillpattern = 0`, cualquier patrón cae a relleno sólido.

### EPSDisplay (único completo)

1. **Sólido** (`fillpattern == 0`): `rectfill` para rectángulos, `closepath fill`
   para paths arbitrarios (`EPSDisplay.cpp`, `rect()` y `stroke()`).
2. **Tramado** (`fillpattern` 1–10): `useFillPattern()` (`EPSDisplay.cpp:377`)
   deriva de forma puramente aritmética:
   - ángulo = `((pattern-1)*45) % 180` → cicla 0/45/90/135
   - separación = `4/(1+(pattern-1)/4)` (en **puntos**) → se aprieta cada 4 patrones

   y emite `hatchN`. En el prólogo PostScript (`ps_hatchers`, `EPSDisplay.cpp:63`)
   cada `hatchN` hace: `gsave → clip` (recorta al path actual) `→ dibuja líneas
   paralelas` que barren el *bounding box* del path `→ stroke → grestore`.
   El bbox se rastrea a mano con `set_limits`/`adjust_limits` en cada primitiva
   (`xmin..ymax`, `EPSDisplay.h:75`).

   Resultado: 10 patrones = 4 ángulos × ~3 densidades, **solo rayado
   unidireccional** (ni cruzado, ni puntos).

### PDFDisplay

Sólido funciona nativo (`HPDF_Page_Fill` / `HPDF_Page_FillStroke`,
`PDFDisplay.cpp:230`). Patrones **desactivados**: `max_fillpattern = 0`.

### SVGDisplay

Sólido funciona (`fill="#rrggbb"`, `getStyleAttributes()`). Patrones **no
implementados**, y además **no** fija `max_fillpattern`, así que hereda el `10`
del constructor base ⇒ si el `.mg` pide un patrón, SVG lo ignora y rellena sólido
en silencio. **Bug latente** a corregir junto con esto.

---

## 2. Qué ofrece cada lenguaje de salida

| | Relleno sólido | Patrones / tramado |
|---|---|---|
| **EPS** | `fill` | `clip` + líneas (ya hecho) |
| **PDF (libharu)** | `HPDF_Page_Fill` ✅ | libharu **no** expone tiling patterns (`/PatternType 1`) en su API pública. Solo queda: `HPDF_Page_Clip()` + dibujar las líneas — **el mismo truco que EPS** |
| **SVG** | `fill` ✅ | **`<pattern>` nativo** en `<defs>`, referenciado con `fill="url(#hatchN)"`. El navegador teja y recorta solo. Es lo idiomático y lo mejor de los tres |

Conclusiones:
- **PDF no tiene atajo nativo usable** (libharu se queda corto) ⇒ reutiliza la
  geometría de EPS (clip + líneas).
- **SVG sí tiene mecanismo nativo** (`<pattern>`), mucho más limpio que clip+líneas.

---

## 3. Referencias reconocidas de dibujo técnico

Dos capas: la **convención semántica** (qué material/región significa cada
tramado) y el **formato concreto** (cómo se define un patrón).

### Convención semántica
- **ISO 128-50** (*Basic conventions for representing areas on cuts and
  sections*): rayado con línea fina continua, preferentemente a **45°**, con
  separación proporcional al tamaño del área; **partes adyacentes se distinguen
  cambiando ángulo o separación**. Es justo la lógica que imita `useFillPattern`.
- **ASME/ANSI Y14.2** (*Line Conventions and Lettering*): define el *section
  lining*; símbolo genérico = 45° equiespaciado, con símbolos por material.
- **DIN 201 / DIN ISO 128**: versión alemana.
- **Architectural Graphic Standards** (Ramsey & Sleeper, AIA): símbolos de
  materiales en arquitectura (hormigón, ladrillo, madera…).
- **FGDC Geologic Map Symbolization** (USGS): ~700 patrones litológicos
  normalizados. Excesivo para el caso de MG, mencionado por completitud.

### Formato concreto (lo adoptable para MG)
- **Formato `.pat` de AutoCAD**: estándar *de facto* de la industria; gramática de
  texto simple y documentada. Cada patrón es una lista de familias de líneas:

  ```
  *ANSI31, ANSI Iron/Brick/Stone masonry
  45, 0,0, 0,.125
  ```
  Por cada línea: `ángulo, x-origen,y-origen, Δx,Δy [, guion1,guion2...]`. Con eso
  se expresan rayado simple, cruzado (dos entradas), punteado (con dashes), etc.
  AutoCAD trae `ANSI31–ANSI38` (materiales) y un juego `ISO*` alineado con ISO 128.

---

## 4. Solución general propuesta

Separar *qué es un patrón* (independiente del dispositivo) de *cómo se pinta*
(por backend). Encaja con el refactor "Etapa 1" ya planeado (subir estado común a
la base `Display`).

### 4.1 Descriptor de patrón estilo `.pat` en la base `Display`

En vez de la fórmula aritmética actual, modelar el patrón como una lista de
familias de líneas (estructura `.pat`):

```cpp
struct HatchLine {
  float angle;          // grados
  float ox, oy;         // origen de la familia
  float dx, dy;         // desplazamiento entre líneas paralelas
  std::vector<float> dashes;  // vacío = línea continua; >0 = punteado/guiones
};
using FillPattern = std::vector<HatchLine>;   // >1 entrada ⇒ cruzado/combinado

FillPattern Display::patternFor(int idx);     // tabla común a los 3 backends
```

Ventajas:
1. Cubre rayado simple, **cruzado** y **punteado** con la misma estructura.
2. Traducible sin pérdida a los tres backends (ver 4.3).
3. Da una tabla con nombres reconocidos (ANSI31, etc.) en vez de índices sin
   significado.
4. Un solo lugar donde ampliar el catálogo.

### 4.2 Subir el bounding box a la base

`xmin/xmax/ymin/ymax` + `set_limits`/`adjust_limits` son geometría pura de
dispositivo; moverlos a `Display` para que PDF los reutilice. EPS ya los tiene;
SVG no los necesita (el `<pattern>` teja y recorta solo). Va en la misma línea
que la Etapa 1 del rediseño de `Display`.

### 4.3 `fillWithPattern(const FillPattern&)` por backend

- **EPS**: dejar el prólogo `hatch*` + clip. Cada `HatchLine` → un `hatch` con su
  ángulo/gap. (Retrocompatibilidad: ver 4.4.)
- **PDF**: `GSave → construir path → Clip → EndPath → dibujar las líneas de cada
  familia dentro del bbox → GRestore`. Copia directa de la geometría de EPS;
  libharu soporta `HPDF_Page_Clip`.
- **SVG**: emitir perezosamente un `<pattern id="...">` en `<defs>` la primera vez
  que se usa cada combinación (una `<line>` por familia) y poner
  `fill="url(#...)"` en la figura. No necesita bbox ni clip.

### 4.4 Mapeo de los 10 patrones actuales (EPS idéntico)

Los 10 patrones actuales se expresan como entradas de **una sola** `HatchLine`.
`angle(p) = ((p-1)*45) % 180` y `gap(p) = 4/(1 + (p-1)/4)` — **ojo: el código
original usa división ENTERA** (`4/(1+…)` con operandos int), verificado en el
golden `examples/fill_styles.eps`:

| idx | ángulo | gap (pt) |
|----|-------|---------|
| 0 | — | — (sólido) |
| 1 | 0   | 4 |
| 2 | 45  | 4 |
| 3 | 90  | 4 |
| 4 | 135 | 4 |
| 5 | 0   | 2 |
| 6 | 45  | 2 |
| 7 | 90  | 2 |
| 8 | 135 | 2 |
| 9 | 0   | 1 |
| 10| 45  | 1 |

`patternFor()` debe reproducir esa aritmética entera literal para que EPS quede
byte-idéntico.

**Verificación:** capturar `.eps` de referencia de los ejemplos con patrones antes
de tocar nada, y comprobar `diff` byte a byte tras extraer `patternFor`/bbox a la
base. La salida EPS **no debe cambiar**.

### 4.5 Orden de implementación recomendado

0. ✅ **HECHO** — Capturada salida de referencia (`.eps`) de `fill_styles.mg`;
   salida determinista comprobada.
1. ✅ **HECHO** — `HatchLine`/`FillPattern` + `patternFor()` y el bbox
   (`set_limits`/`adjust_limits`, `xmin..ymax`) subidos a la base `Display`;
   `EPSDisplay::useFillPattern()` reescrito para consumir `patternFor()`.
   Verificado: **todos los `.eps` de examples byte-idénticos**.
2. ✅ **HECHO — SVG**: patrones con `<pattern patternUnits="userSpaceOnUse"
   patternTransform="rotate(angle-90)">` + una `<line>` horizontal, emitido
   perezosamente en `<defs>` (dedup por índice+color vía `emitted_patterns`).
   Una sola construcción cubre los 4 ángulos (el `-90` y el signo salen del flip
   global `scale(1,-1)`). Funciona en cualquier forma (elipse/círculo/polígono/
   rect) y respeta `outlinefill`. Verificado visualmente contra EPS con
   `fill_styles.mg` y `primitives.mg` (Inkscape). `max_fillpattern` ya valía 10
   (heredado); el bug real era que se ignoraba el patrón, ya corregido.
   BUG PREEXISTENTE corregido de paso: `SVGDisplay::deviceTranslate` emitía el
   translate en unidades normalizadas en vez de puntos (faltaba `*dvx,*dvy` como
   en EPSDisplay), así que las figuras trasladadas con TLLC caían encima de las
   originales (en `primitives` el círculo/elipse/rect/polígono rellenos tapaban
   sus contornos negros). Ahora coincide con EPS.
3. ✅ **HECHO — PDF**: `max_fillpattern` 0→10. Patrones vía clip+líneas (libharu
   no expone patrones de mosaico): `ensurePatternGSave()` abre el GSave en
   PAGE_DESCRIPTION antes del path (libharu prohíbe GSave en PATH_OBJECT y q/Q no
   preservan el path); `hatchCurrentPath()` usa el operador `W` (no consume el
   path): `W S` = contorno + recorte (outlinefill), `W n` = solo recorte, y luego
   dibuja las líneas sobre el bbox (diagonal completa, centradas) que el recorte
   trima a la forma. Bbox subido a la base en el Paso 1; PDF ahora llama
   set_limits/adjust_limits en moveto/lineto/curveto/rect/arc y resetea en
   setOpenPath. Verificado vs EPS (fill_styles, primitives): patrones, densidades,
   outlinefill y recorte en curvas coinciden.
   NOTA: `fig2-3.pdf` emite 30 errores libharu (INVALID_GMODE) — PREEXISTENTE
   (mismo conteo antes de estos cambios, verificado con git stash), ajeno a
   patrones; no usa FPATRN.
4. ⏭️ **SIGUIENTE (opcional)** — ampliar el catálogo: rayado cruzado y punteado,
   aprovechando que `HatchLine` ya lleva `dashes`/`ox,oy` y `FillPattern` admite
   varias familias. Los tres backends ya consumen `patternFor()`.

---

## 5. Discusión: propiedad global de resolución (dpi)

Idea de Alejandro: añadir a MetaGráfica una propiedad `dpi` (como ya hay
dimensiones en cm) para manejar densidades de separación y también la subdivisión
de splines.

### 5.1 El fondo es correcto, el nombre merece matiz

El pipeline actual (EPS/PDF/SVG) es **vectorial y resolución-independiente**:
`cm → puntos` es exacto (`cm_to_point = 28.346…`), no hay "puntos por pulgada"
en ningún lado. **dpi es un concepto de trama** (píxeles por pulgada). En salida
vectorial no hay píxeles, así que:

- Para **separación de patrones** la cantidad natural y consistente con las
  dimensiones en cm es una **longitud física** (p. ej. 0.5 mm entre líneas) o una
  **densidad** (líneas/cm). Es lo que hace el formato `.pat` (deltas en unidades
  de dibujo). Usar dpi aquí sería la abstracción equivocada: implicaría una
  rejilla de trama inexistente. *(Nota: hoy el gap está en puntos, ya es longitud
  física, solo falta exponerlo en unidades del usuario.)*

- Para **subdivisión de splines** (`splines()` en `splines.cpp` usa un `intervals`
  **fijo** por tramo → demasiados puntos en curvas chicas, pocos en grandes) lo
  correcto es una **tolerancia de planitud** (máxima desviación de la cuerda,
  también una longitud). Nótese que `splines_to_bezier` + `curveto` **no necesita
  subdividir**: el dispositivo rasteriza la curva nativa. La subdivisión solo hace
  falta cuando hay que producir una polilínea.

### 5.2 Dónde sí es dpi la cantidad correcta

dpi pasa a ser de primera clase el día que exista un **backend de trama**
(PNG/bitmap): ahí dpi = píxeles por pulgada es exactamente lo que se necesita.
También es un *handle* cómodo para responder "¿qué detalle es imperceptible?":
1 px a dpi dado da un tamaño mínimo de rasgo, útil como **default** para la
tolerancia de planitud de splines y para densidades por omisión.

### 5.3 Recomendación

Añadir un global de **resolución de referencia (dpi)**, pero con rol acotado:

1. Fuente de **defaults**: tolerancia de planitud de splines (≈ 1 px @ dpi) y
   densidad de patrones por omisión.
2. Parámetro **obligatorio** cuando exista un backend de trama.
3. **No** ser la unidad en que el usuario expresa separaciones: eso queda en
   longitud física (mm) o densidad (líneas/cm), consistente con las dimensiones
   en cm y con el formato `.pat`.

Así el pipeline vectorial sigue siendo resolución-independiente (una virtud) y el
dpi queda donde de verdad aporta. Decisión de diseño pendiente de Alejandro.

### 5.4 Enganche concreto con splines (independiente de patrones)

Sustituir el `intervals` fijo de `splines()` por un cálculo adaptativo:
`n_segmentos ≈ longitud_del_tramo / tolerancia`, donde `tolerancia` sale de la
resolución de referencia. Además, quitar el `printf("spline new points…")` de
depuración en `splines.cpp:77`.

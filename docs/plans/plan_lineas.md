# Estilos y Patrones de Línea — Plan de implementación

Como parte de la profesionalización de MetaGráfica, antes de pasar a V2 abandonamos el
sistema de índices cerrados para líneas (`LWIDTH`, `LPATRN`) en favor de un modelo
estándar de la industria, compatible de forma nativa con PostScript, PDF y SVG: los
estilos se describen con **grosor en puntos**, **patrón de guiones como arreglo**, y
**extremos/uniones** (`cap`/`join`).

Este documento separa el trabajo en **dos fases**:

- **Fase 1 (este repositorio, motor V1).** Refactor interno del `Display` y sus
  backends para que el modelo de datos ya sea el de V2, **sin tocar la gramática**:
  los ejemplos actuales (`LWIDTH`/`LPATRN`) deben seguir generando exactamente la misma
  salida. Es preparación pura.
- **Fase 2 (V2, gramática).** Los atributos con nombre (`width=`, `dash=`, `cap=`,
  `join=`), los alias de `dash`, los arreglos personalizados y la política de unidades.
  Su especificación vive en `especificacion_mg.md` §4.10; aquí solo se resume.

> **La Fase 1 no cambia la sintaxis `.mg`.** No se añade ningún token al lexer ni regla
> al parser. Todo el cambio ocurre bajo `setLineWidth`/`setLineStyle` y en el
> `DisplayState`.

---

## 1. Estado actual del motor (auditoría)

| Aspecto | V1 hoy | Dónde |
|---|---|---|
| Grosor | `LWIDTH n` → `AT_LWIDTH` → `setLineWidth(n)` emite `n * 0.2` pt. Default sin `LWIDTH`: 1.0 pt (default del dispositivo en PS/libharu; SVG lo replica a mano). `LWIDTH 0` = hairline dependiente del dispositivo. | `primitives.cpp` (AT_LWIDTH), `EPSDisplay::setLineWidth`, `PDFDisplay::setLineWidth`, `SVGDisplay::strokeWidth` |
| Patrón | `LPATRN n` → `AT_LSTYLE` → `setLineStyle(int)`. Cada backend traduce el índice a un arreglo **codificado a mano** en un `switch`. | `EPSDisplay::setLineStyle`, `PDFDisplay::setLineStyle`, `SVGDisplay` (`getStyleAttributes`) |
| Extremos/uniones | No existen. Se usan los defaults del motor (butt/miter). | — |
| Almacenamiento | `DisplayState { int line_style; int line_width; }` | `Display.h` |

### Tres discrepancias detectadas (a resolver en Fase 1)

1. **`LPATRN 1` renderiza sólido, no "dashed".** En EPS y PDF el `switch` funde
   `case 0: case 1:` → línea continua. Es decir, el patrón 1 de V1 es indistinguible
   del 0. (Ver `line_patterns.mg`, que rotula "Pattern 1" sobre una línea continua.)
   La tabla de alias V2 debe reflejar esto: solo hay **cinco** conductas distintas
   (0/1 sólido, 2 dashed, 3 dotted, 4 dash-dot, 5 dash-dot-dot).

2. **El patrón 3 (dotted) difiere entre backends:** EPS y SVG usan `[2 1.6]`, pero PDF
   usa `[2 2]`. Al centralizar el mapeo (§3) se unifica a `[2 1.6]` (el de EPS/SVG,
   tomado como canónico). Es un cambio cosmético menor en el `dotted` de los PDF; se
   documenta como corrección de consistencia.

3. **`LWIDTH 0` (hairline) se usa en ejemplos activos** (`fig2-3`, `fig6-1`,
   `primitives`, `line_patterns`). En Fase 1 **debe conservarse** el comportamiento
   actual. La prohibición del hairline es una decisión de V2 (el traductor convierte
   `LWIDTH 0` → `width=0.1`), no del motor V1.

---

## 2. Modelo interno objetivo (compartido V1 y V2)

Fuente única de verdad en `DisplayState`, que tanto la ruta V1 (índices) como la futura
ruta V2 (atributos con nombre) alimentan:

```cpp
struct DisplayState {
    // ...
    float line_width_pt = 1.0f;      // grosor en PUNTOS (no en unidades de 0.2 pt)
    bool  line_width_set = false;    // ¿se fijó explícitamente? (distingue default 1.0 de hairline 0)
    std::vector<float> dash_array;   // vacío = continua; longitudes on/off en pt
    int   line_cap  = 0;             // 0 butt, 1 round, 2 square
    int   line_join = 0;             // 0 miter, 1 round, 2 bevel
    // (int line_style / int line_width quedan obsoletos y se retiran)
};
```

- **Grosor en puntos.** El valor almacenado es ya el de V2 (`width=` en pt). La ruta V1
  hace la conversión `n * 0.2` al entrar; los backends emiten `line_width_pt` tal cual.
- **`dash_array` como representación única.** Reemplaza a `int line_style`. Los backends
  ya no llevan tablas propias: emiten el arreglo genéricamente. La correspondencia
  índice/alias → arreglo vive en **un solo lugar** (helper en la clase base `Display`).
- **`cap`/`join` dormidos en Fase 1.** El estado y los setters existen, pero ninguna
  ruta V1 los invoca, así que los defaults (butt/miter, idénticos a lo actual) hacen que
  la salida no cambie. Quedan listos para que V2 los conecte.

### Fuente única de verdad para el patrón

```cpp
// Display (clase base). Único lugar donde vive la correspondencia índice → arreglo.
std::vector<float> Display::dashArrayForIndex(int i) {
    switch (i) {
        case 2:  return {4, 2};
        case 3:  return {2, 1.6f};        // unifica: EPS/SVG usaban esto; PDF usaba {2,2}
        case 4:  return {4, 2, 1, 2};
        case 5:  return {4, 2, 2, 2, 2, 2};
        default: return {};               // 0 y 1 → continua
    }
}
```

---

## 3. Fase 1 — Refactor del motor (gramática V1 intacta)

Objetivo verificable: la misma salida **renderizada** en los ocho ejemplos (EPS, PDF y
SVG), salvo la unificación intencional del `dotted` en PDF (discrepancia #2). *No* se
busca salida byte-idéntica: el `dash` pasa de literales escritos a mano a un bucle sobre
floats, así que el texto EPS/SVG cambiará de forma (p. ej. `[ 4 2 ]` → `[4 2]`) aunque el
resultado sea visualmente igual. La comparación es por imagen (§3.6).

**Archivos a tocar:** `include/Display.h`, `include/EPSDisplay.h`, `include/PDFDisplay.h`,
`include/SVGDisplay.h`, `src/EPSDisplay.cpp`, `src/PDFDisplay.cpp`, `src/SVGDisplay.cpp`.
`src/primitives.cpp` solo en la ruta V1 (§3.5). **No** se toca el lexer ni el parser.

### 3.1 `DisplayState` y clase base `Display` (`include/Display.h`)

**En `DisplayState`:**
- Añadir los campos de §2 (`line_width_pt`, `line_width_set`, `dash_array`, `line_cap`,
  `line_join`).
- Retirar `int line_style` y `int line_width`. Usos a eliminar/migrar (grep para
  confirmar): sus inicializaciones en el ctor de `DisplayState`, las declaraciones de
  campo, `Display::setLineStyle`, y en `SVGDisplay` los accesos a `dspstate.line_style`
  (`getStyleAttributes`, `setLineStyle`) y `dspstate.line_width` (`setLineWidth`,
  `strokeWidth`). El miembro local `SVGDisplay::line_width_set` (en `SVGDisplay.h`) se
  **sube a `DisplayState`** y se elimina el duplicado.

**En la clase base `Display`** (diseño de hooks; el estado se fija en la base, la emisión
la hace cada backend):
```cpp
// helper: única fuente de verdad índice → arreglo (§2, no virtual)
std::vector<float> dashArrayForIndex(int i);

// setters que fijan estado y disparan el hook de emisión
virtual void setLineStyle(int i) { dspstate.dash_array = dashArrayForIndex(i); applyDash(); }
virtual void setLineCap (int c)  { dspstate.line_cap  = c; applyLineCap();  }
virtual void setLineJoin(int j)  { dspstate.line_join = j; applyLineJoin(); }

// hooks de emisión: no-op por defecto (SVG los deja así y lee el estado al dibujar);
// EPS y PDF los sobre-escriben para emitir de inmediato.
virtual void applyDash()     {}
virtual void applyLineCap()  {}
virtual void applyLineJoin() {}
```
`setLineWidth(float)` sigue siendo virtual puro (cada backend lo implementa). Los
overrides actuales de `setLineStyle` en EPS/PDF **se eliminan** (los reemplazan
`applyDash`); SVG también deja de sobre-escribir `setLineStyle`.

> **Modo gráfico (PDF).** `setLineStyle`/`applyDash` se invocan desde `Attribute::draw`,
> es decir entre primitivas, en modo PAGE_DESCRIPTION — igual que hoy. `HPDF_Page_SetDash`
> es válido ahí, así que no reaparece el problema de GMODE.

### 3.2 EPSDisplay (`EPSDisplay.h` / `.cpp`)
- `setLineWidth(float pt)` → `fprintf(file, "%g setlinewidth\n", pt);` (ya no `*0.2`; la
  escala la hace la ruta V1). Guardar también en `dspstate.line_width_pt`. No usa
  `line_width_set` (mantiene la conducta actual "emitir solo al llamar"; el default 1.0 lo
  pone el dispositivo).
- Eliminar el override de `setLineStyle`; implementar `applyDash()` que recorre
  `dspstate.dash_array` y emite `[ … ] 0 setdash` con `%g` por elemento (o `[] 0 setdash`
  si el arreglo está vacío).
- Implementar `applyLineCap`/`applyLineJoin` → `%d setlinecap` / `%d setlinejoin`. (En V1
  nadie los llama, así que no se emiten y la salida no cambia.)
- Declarar los tres `apply*` como `override` en `EPSDisplay.h`; quitar la declaración de
  `setLineStyle`.

### 3.3 PDFDisplay (`PDFDisplay.h` / `.cpp`)
- `setLineWidth(float pt)` → `HPDF_Page_SetLineWidth(page, pt);` (sin `*0.2`); guardar en
  `dspstate.line_width_pt`.
- Eliminar el override de `setLineStyle`; implementar `applyDash()` → copiar
  `dspstate.dash_array` a un buffer `HPDF_REAL[]` y `HPDF_Page_SetDash(page, buf, n, 0)`,
  o `HPDF_Page_SetDash(page, nullptr, 0, 0)` si está vacío. **Esto corrige la discrepancia
  #2** (el dotted pasa de `{2,2}` a `{2,1.6}`).
- Implementar `applyLineCap`/`applyLineJoin` → `HPDF_Page_SetLineCap` /
  `HPDF_Page_SetLineJoin`.
- Actualizar las declaraciones en `PDFDisplay.h` como en EPS.

### 3.4 SVGDisplay (`SVGDisplay.h` / `.cpp`)
- Eliminar el override `setLineStyle` (lo maneja la base) y el miembro local
  `line_width_set` de `SVGDisplay.h` (ahora vive en `DisplayState`).
- `setLineWidth(float pt)` → guarda `dspstate.line_width_pt` y marca
  `dspstate.line_width_set = true`. `strokeWidth()` lee esos campos con la lógica actual
  (no fijado → 1.0 pt; fijado a 0 → mínimo visible 0.5). Desaparece el `*0.2`.
- `getStyleAttributes()` construye `stroke-dasharray` recorriendo `dspstate.dash_array`
  (en vez del `switch` sobre `line_style`); arreglo vacío → omitir el atributo.
- Añadir `stroke-linecap`/`stroke-linejoin` en `getStyleAttributes()` solo cuando
  `dspstate.line_cap`/`line_join` ≠ 0 (para no cambiar la salida en V1). No necesita
  overrides de `applyDash`/`applyLineCap`/`applyLineJoin`: lee el estado al dibujar.

### 3.5 Ruta V1 (conversión de unidades) — `src/primitives.cpp`
El único punto que traduce el "dialecto" V1 al modelo en puntos:

```cpp
case AT_LWIDTH:                      // LWIDTH n  (n en unidades de 0.2 pt)
    g.setLineWidth(value * 0.2f);    // → line_width_pt en puntos
    break;
case AT_LSTYLE:                      // LPATRN n
    g.setLineStyle(value);           // el índice se resuelve a dash_array en la base
    break;
```

`setLineWidth` recibe ya puntos; los backends no vuelven a escalar. `LWIDTH 0` sigue
llegando como `0.0` y cada backend aplica su regla de hairline actual (sin cambios).

### 3.6 Verificación de Fase 1

La comparación es **por imagen**, no por diff de texto (el formato numérico del `dash`
cambia; ver §3).

0. **Antes de editar**, desde el `HEAD` actual: generar los 8 ejemplos en los tres
   backends y rasterizar a PNG de referencia (`pdftoppm`/`gs` para PDF/EPS, `rsvg`/`gs`
   para SVG), p. ej. a `ref/`.
1. Aplicar el refactor. `make clean && make`.
2. Regenerar los 8 ejemplos y rasterizar igual, a `out/`.
3. Comparar `ref/` vs `out/` (diff de imágenes, p. ej. `compare -metric AE` de
   ImageMagick):
   - **EPS y SVG**: idénticos en los 8 ejemplos.
   - **PDF**: idéntico salvo las líneas `dotted` (`LPATRN 3`, presente en
     `line_patterns.mg`), que ahora usan `{2,1.6}` en vez de `{2,2}`. **Es el cambio
     esperado**, no una regresión: el `dotted` del PDF queda igual al de EPS/SVG.
4. Revisar a ojo `line_patterns` (patrones y anchos) y `fig2-3`/`fig6-1` (que combinan
   `LWIDTH 0`, `LPATRN 2` y texto) en los tres backends.

---

## 4. Fase 2 — Gramática V2 (detalle en `especificacion_mg.md` §4.10)

Con el motor ya en el modelo objetivo, V2 solo añade la capa de gramática/parser:

- Atributos con nombre en cualquier primitiva: `width=` (float, pt, default 1.0),
  `dash=` (alias string **o** arreglo `[…]`), `cap=`, `join=`.
- `dash` como arreglo personalizado (`dash=[10,2,1,2]`), escalado por el `width` actual;
  se resuelve al mismo `dash_array` del motor (§2), así que los tres backends ya lo
  soportan al terminar la Fase 1.
- Política de unidades: puntos tipográficos, default 1.0 pt, sin hairline dependiente del
  dispositivo.
- Reglas del traductor `mg1to2` (`LWIDTH`, `LPATRN`, `LWIDTH 0`) y auditoría de keywords.

La redacción completa y normativa de todo lo anterior está en `especificacion_mg.md`
§4 (intro de atributos), §4.10 (estilo de línea), §20 y §21 (transición y auditoría).

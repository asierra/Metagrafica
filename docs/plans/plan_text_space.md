# Plan: tipografía math — medición precisa de `Text` + espaciado automático

**Requisito de fondo (Alejandro, 2026-07-23):** *«los objetos `Text` deben dar sus medidas
precisas»*. De ahí cuelga todo: el centrado, `fit`, y —sobre todo— `\frac`, que dimensiona la
fracción con `TextLine::width()`. Si `Text` no mide lo que dibuja, `\frac` sale torcido por
construcción. Este plan es **fundación de `plan_frac.md`**, no al revés.

Nace de un problema visible (`gravitacion_orbita`: `$F = G m_1 m_2$` con espacios de más) que al
investigar resultó ser **dos problemas con una causa común**.

## Los dos problemas (una causa)

- **(A) Medición imprecisa.** Un run de math (`FN_TEX_CMMI`) lleva **una cara** en su
  `TextState`, pero al dibujarse los tres backends lo **parten por glifo**: griego (∈
  `cmmiUnicode()`) → LM Math; no-griego (`E`, dígitos, `=`, espacio) → **Times-Italic**. En
  cambio `text_width` mide **todo el run con `cmmi_metrics_map`**. La medida ≠ el render.
- **(B) Espacios literales.** Los espacios del fuente en `$…$` se imprimen (caen al `default`
  de `parse_text`), en vez de que el espaciado lo pongan las reglas de la fórmula (TeX).

**Causa común:** los runs no son **átomos homogéneos** (una cara, una clase).

## Hallazgos del spike (2026-07-23) — corrigieron el diagnóstico

1. **El criterio de partición es UNO solo e idéntico en los tres backends:** membresía en
   `cmmiUnicode()` (griego vs no-griego). Los comentarios de `EPSDisplay::text`,
   `PDFDisplay::text` y `SVGDisplay::text` dicen «igual criterio». El respaldo no-griego es
   **Times-Italic** (EPS/PDF lo ponen explícito; SVG vía `svgFontAttrs(face)`).
2. **Sobrestimé la magnitud anoche.** El respaldo es Times-**Italic** (`=` mide 675), no
   Times-**Roman** (564). Errores reales por glifo: `=` 778(cmmi) vs 675(ital) = **+15%**,
   espacio 332 vs 250 = **+33%**, dígitos coinciden. Es real pero ~la mitad de lo que dije.
3. **El arreglo de medición es LOCALIZADO y diminuto:** ~15 líneas en `text_width` (partir el
   run por `cmmiUnicode` y medir cada trozo con su mapa —`cmmi_metrics_map` el griego,
   `serifitalic_metrics_map` el resto—). `text.cpp` alcanza `cmmiUnicode()` con
   `#include "text_parser.h"`. **Prototipado: churnea EXACTAMENTE un golden, `sines.svg`.**
   (sines.eps/pdf quedan `ok`; los otros 10 ejemplos con math no se mueven.)
4. **Por qué tan contenido, y a quién le importa de verdad:** EPS/PDF **centran con operadores
   de fuente** (`cshow`/nativo), NO con `text_width`, así que la imprecisión **solo mordía al
   SVG**. El beneficiario real de medir bien es **`\frac`**, que usa `text_width` en los tres
   backends para dimensionar la fracción. O sea: (A) no es un bug grande del texto existente,
   es el **prerrequisito de un `\frac` correcto**.
5. **NO hay que mover la partición al parser** (era el supuesto #2, el que daba miedo). Se
   arregla `text_width` para que CUADRE con la partición que los backends ya hacen, dejándola
   donde está. La simplificación de los backends (partir una sola vez) queda **fuera de
   alcance**: opcional, no requerida para la precisión.

## Plan re-escalado: DOS partes

### Parte A — medición precisa (fundación de `\frac`). ✅ **HECHA 2026-07-23.**
- `text_width` (`src/text.cpp`): rama `FN_TEX_CMMI` que parte por `cmmiUnicode` y mide cada
  trozo con su mapa (`cmmi_metrics_map` el griego, `serifitalic_metrics_map` el no-griego —la
  MISMA partición que los backends—). `text.cpp` alcanza `cmmiUnicode()` con
  `#include "text_parser.h"`.
- El cambio de `fmmap[ch]` por `find()` + fallback (500/em) **NO agregó churn** (medido
  aparte): el corpus no medía con ceros mudos → endurecimiento limpio, se quedó.
- **Churn: solo `sines.svg`** (+ su `docs/img`), como predijo el spike. Es una corrección: la
  math queda bien posicionada (verificado el render). Re-bendecir con `capture` + `images`.
- Salió en pocas horas, como se estimó. **Con esto la fundación que `\frac` necesita está.**

### Parte B — espaciado automático estilo TeX. ✅ **HECHA 2026-07-23.**
Ejecutada tras cerrar el pase de decisiones con Alejandro (las tres, como se
recomendaba): **puro TeX** (consumir todos los espacios del fuente), **anular** el
espaciado dentro de sub/superíndices, y **regla TeX** para el unario. Implementación:
- `text_parser.cpp`: enum `MathClass` + `mathClassOfByte`/`mathClassOfName` +
  `mathGlue` (tabla TeX thin=3/18, med=4/18, thick=5/18) + `mathAtomSpace` (reclasa el
  unario, anula en índices, suma los overrides). El espacio viaja como `pre_space` (em)
  en `TextState`; `mathSeal()` sella el run previo para que dos entradillas no se
  fusionen. Un `case ' '` consume los espacios del fuente en math.
- `text.h`/`text.cpp`: `TextState::pre_space` (en `operator==`); `TextLine::width()` lo
  suma y `TextLine::draw()` lo aplica (rmoveto). `textflush()` lo resetea tras hornearlo.
- Overrides `\,` `\;` `\!` `\quad` (aditivos, en el `case '\\'`).
- **Churn: 6 ejemplos** (quickstart, sines, franck_condon, texto, fig6-4, fig_polybar),
  los 3 backends, **verificados a ojo uno por uno** (rasterizados vs. el golden limpio):
  todos mejoran a tipografía TeX-correcta. `fig1` quedó byte-idéntico (el unario hizo
  `-U_A` igual que antes). **c3fail=0** (paridad intacta) y **psfail=0** en todo. Smoke
  test aparte de overrides/unario/binario/consumo de espacios: correcto.

Lo que resta como diseño previo (histórico), para referencia:
1. Al descargar cada trozo en `parse_text`, clasificarlo (Ord / Rel / Bin / Op(función) /
   Open / Close / Punct). La info casi está: los símbolos entran por `\name` con código, `=`
   `<` `>` son caracteres conocidos, `math_functions[]` ya lista `sin`/`cos`/…
2. Insertar el espacio entre átomos según el par de clases, guardado como **`pre_space` (em)**
   en cada `Text`. **`TextLine::width()` lo SUMA y `TextLine::draw()` lo aplica**
   (`rmoveto(pre_space·fs, 0)` antes del run) → la medida sigue precisa CON el espaciado.
3. **Ignorar los espacios del fuente** (consumirlos sin imprimir).

**Tabla (subconjunto de TeX):** `\thinmuskip=3/18 em`, `\medmuskip=4/18`, `\thickmuskip=5/18`.
- alrededor de **Rel** (`= < > ≈ ≤ ≥ ≠ →`): thick (5/18)
- alrededor de **Bin** (`+ − × ÷ ± ·`): medium (4/18)
- después de **función**: thin antes del argumento
- después de **Punct** (`, ;`): thin
- Ord–Ord (letras/dígitos): 0

**Decisiones abiertas de la Parte B (el pase que falta antes de ejecutarla):**
- **`−` unario vs binario** (`a−b` Bin, `−a` Ord): regla de TeX —un `+/−` al inicio, o tras
  Bin/Rel/Open/Punct, es Ord—. Barato una vez que hay clases.
- **Espaciado dentro de sub/superíndices** (el `_`/`^` es una máquina aparte, tspush/tspop):
  cómo se espacia un `=` dentro de un `^{…}`. Sin resolver.
- **Overrides explícitos:** `\,` `\;` `\quad` `\!` como `pre_space` fijo, para lo que la tabla
  no acierte.
- **Churn**: las ~12 fórmulas del corpus cambian a su forma bien espaciada. Enumerar y
  verificar que cada una mejora antes de re-bendecir.

## Orden y dependencias

- **Parte A primero** (barata, lista) → deja la medición precisa que `\frac` necesita.
- **`\frac`** (`plan_frac.md`) puede ir después de A, o esperar a B para heredar el espaciado.
- **Parte B** (espaciado) es independiente de `\frac` y mejora TODO el corpus math; pero es la
  más cara y necesita el pase de decisiones de arriba antes de ejecutarse.

## Veredicto de madurez

- **Parte A: ✅ HECHA 2026-07-23** (churn = `sines.svg`, una corrección; `find()` sin churn extra).
- **Parte B: ✅ HECHA 2026-07-23** (churn = 6 ejemplos verificados; decisiones puro-TeX /
  anular-índices / regla-unario, confirmadas con Alejandro). Con esto el espaciado math del
  corpus queda TeX-correcto. Lo que queda para `gravitacion_orbita` es solo `\frac`
  inline (`plan_frac.md`), que ya hereda la medición (Parte A) y el espaciado (Parte B).

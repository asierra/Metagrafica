# Auditoría de `src/text_parser.cpp` (+ text.h / text.cpp)

Auditoría de solo lectura. El state machine de `parse_text` opera sobre **estado global
de archivo** (`accum`, `text`, `text_line`, `text_state`, `tstack`) con un patrón
`tspush`/`tspop` que NO está protegido contra entrada malformada. Varios de los hallazgos
graves comparten esa raíz.

Convenciones: [VERIFICADO] = reproducido con un render; [probable] = por lectura.

> **Estado de resolución (2026-07-10).** Atendida en dos commits:
> - **`c177a60`** — crashes por entrada malformada: C1, A2, A3 **+ C1b** (un 4º camino
>   hallado al probar: `$`/`$$` solo-delimitadores → `text_line` nulo en el retorno).
> - **`3615f13`** — medias/bajas: M4, M5, B7, B8.
> - **No tocados por decisión:** B10 (la igualdad de `double` ES la semántica de fusión
>   de chunks), M6/`v_end` (solo comentado; depende del wraparound de `npos`).
> - **Pendientes (menores / futuro):** B9, B11, B12, y el refactor de fondo (reentrancia).
>
> Verificado tras los arreglos: build limpio, corpus 15/15 EPS/SVG/PDF, malformados sin
> crash, math+símbolos correctos, cambio de fuente en math (`$\alpha /r x$`) cierra bien.

---

## CRÍTICO

### C1. `tspop()` sin guardia → segfault con llaves/`$` desbalanceados  [VERIFICADO] — ✅ RESUELTO (c177a60)
`text_parser.cpp:302-307`. `tspop()` hace `text_state = tstack.top(); tstack.pop();` sin
comprobar `tstack.empty()`. `top()`/`pop()` sobre un `std::stack` vacío es **UB**. Cualquier
entrada con un `}` (o cierre) sin su apertura llega a `tspop()` con la pila vacía.
- **Disparo:** `text("a}b")` → **Segmentation fault** (EXIT 139). También `text("}")`,
  y en general cualquier `}` de más.
- **Arreglo aplicado:** `if (tstack.empty()) return;` al inicio de `tspop()`.

### C1b. Retorno de `parse_text` deshace `text_line` nulo → segfault  [VERIFICADO] — ✅ RESUELTO (c177a60)
(No estaba en la auditoría original; salió al probar los arreglos.) Contenido de solo
delimitadores sin ningún chunk (`$`, `$$`) deja `text_line == nullptr`; el retorno hace
`text_line->length()` → **segfault**.
- **Disparo:** `text("$")`, `text("$$")` → EXIT 139.
- **Arreglo aplicado:** guardia `if (!text_line) return std::move(text);` antes del retorno.

---

## ALTO

### A2. Estado global NO se resetea entre llamadas → corrupción cruzada  [VERIFICADO] — ✅ RESUELTO (c177a60)
`parse_text` reseteaba `text`, `text_line`, `text_state`, pero **NO `tstack`** (ni `accum`).
Como `tstack` es global de archivo, cualquier push colgado de una llamada previa (ver A3)
sobrevive a la siguiente y la corrompe.
- **Disparo:** `text("$")` (deja 1 push colgado) seguido de `text("hola}mundo")`: el `}`
  pop-ea el **TextState de la cadena anterior** → estado de fuente corrupto (EXIT 0). Es la
  otra cara de C1: una fuga previa CONVIERTE el crash en corrupción silenciosa.
- **Arreglo aplicado:** al entrar a `parse_text`, `accum.clear()` y vaciar `tstack`. (El
  refactor ideal —estado en locales, reentrante— queda como pendiente de fondo.)

### A3. Sub/superíndice vacío o al final deja `tspush` sin `tspop`  [VERIFICADO] — ✅ RESUELTO (c177a60)
En modo matemático, `_`/`^` hace `textflush(); tspush(); it++`. Si sigue fin de cadena o un
delimitador de `TOKENLIM` (p.ej. el `$` de cierre), el `else` hace `break` **saltándose el
`tspop()`**. También en el `break` de comando desconocido. Push sin pop.
- **Disparo:** `text("$x^$")`, `text("$a_")`.
- **Arreglo aplicado:** `tspop()` antes de esos dos `break` del bloque `_`/`^`.

---

## MEDIO

### M4. Detección de `$` por `font_face`, no por contador → rompe con cambio de fuente — ✅ RESUELTO (3615f13)
Abrir/cerrar `$` se decidía con `text_state.font_face!=FN_TEX_CMMI`. Si dentro de `$…$` se
cambia de fuente (`/r`, `/i`, o un símbolo `FN_SYMBOL`), el `$` de cierre veía `font_face`
!= cmmi y lo trataba como **apertura** nueva; `math_mode` quedaba desincronizado.
- **Disparo:** `text("$\alpha /r x$")` — el `$` final reabría en vez de cerrar.
- **Arreglo aplicado:** el case `$` abre/cierra según la bandera `math_mode` (la verdad),
  no según `font_face`.

### M5. `UTF8toISO8859_1`: `codepoint` sin inicializar + drop silencioso >255 — ✅ RESUELTO (3615f13)
`unsigned int codepoint;` sin init: si el primer byte es una continuación mal formada
(0x80–0xBF), se lee basura → UB. Además, todo codepoint >255 se descarta (limitación
Latin-1: un `α` literal se pierde; hay que usar `\alpha`).
- **Arreglo aplicado:** `unsigned int codepoint = 0;` + comentario documentando el descarte
  >255 (limitación del pipeline ISO-8859-1; los símbolos math van por `\comando`, no por ahí).

### M6. `int v_end` vs `size_t npos` — funciona por casualidad — ⚠ NO TOCADO (solo comentado, 3615f13)
`int v_end` recibe `npos` (`size_t`) → −1; luego `v_end==string::npos` compara por wraparound.
Correcto hoy, frágil. Hay `it = v_end - 1` con `it` int, así que cambiar el tipo no es trivial;
se dejó con un comentario notando la dependencia. Pendiente si se quiere endurecer.

---

## BAJO

### B7. `map_symbol` con ~50 entradas DUPLICADAS — ✅ RESUELTO (3615f13)
El griego y varios símbolos aparecían **dos-tres veces**. `std::map` descartaba los
duplicados (inocuo) pero confundía. **Arreglo:** deduplicado (213→107 entradas, 0 conflictos
de valor; todos los símbolos preservados).

### B8. `math_structs[]` es código muerto  [VERIFICADO] — ✅ RESUELTO (3615f13)
Definido, sin uso. **Decisión de Alejandro:** comentarlo (no borrarlo) con nota de intención
(reservado para `frac`/`int`/`prod`/`sum` futuros, que requerirían una estructura de
composición de texto; probablemente obsoleto si a futuro se embeben fragmentos TeX).

### B9. Lecturas de `input[iend]` (el `\0`) por diseño — ⚠ PENDIENTE
`input[it+1]` en `/` final y `input[it]` tras `it++` con `_`/`^` final. `operator[](size())`
devuelve el `\0` (válido desde C++11), así que no crashea, pero depende del detalle. Chequeos
`it+1 < iend` explícitos aclararían la intención. Menor.

### B10. `TextState::operator==` compara `font_size` (double) con `==` — ⚠ NO TOCADO (por decisión)
`text.h`. Igualdad exacta de `double`. **Se dejó a propósito:** ES la semántica de "mismo
estado → fusiona chunks" de `textflush`; cambiarla a tolerancia podría alterar la fusión.
Funciona porque los tamaños se derivan determinísticamente.

### B11. Duplicidad de lookup y dos entradas a cmmi — ⚠ PENDIENTE
`get_symbol_code`/`change_font_face`: `find()` seguido de `map[]` (doble búsqueda). Y hay DOS
caminos a `FN_TEX_CMMI`: `$` (conmuta `math_mode`) y `/$` (vía `change_font_face`, NO toca
`math_mode`) → `_`/`^` funcionan tras `$` pero no tras `/$`. Inconsistencia a unificar o
documentar. Menor.

### B12. Código comentado / muerto — ⚠ PENDIENTE
Varios `//printf(...)` de depuración; `//it = v_end-1;`; `//fprintf` en `get_symbol_code`;
`fn_relsz_script2` comentado en text.h. Limpieza cosmética.

---

## Recomendación de fondo — ⚠ PENDIENTE (futuro)
Los hallazgos graves (C1/C1b/A2/A3) son la **misma enfermedad**: máquina de estados con
globals mutables y `tspush`/`tspop` no balanceados. Los arreglos aplicados los blindan caso
por caso. El **refactor ideal** (no urgente): mover todo el estado (`accum`, `text`,
`text_line`, `text_state`, `tstack`) a una struct local de `parse_text` (o una clase
`TextParser`), volviéndolo **reentrante** y eliminando de raíz las fugas entre llamadas.
Ninguno toca la semántica de la salida correcta; solo blindan los caminos malformados.

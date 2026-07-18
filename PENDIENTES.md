# PENDIENTES — MetaGráfica V3

> **Qué es esto.** Tablero de continuación: el estado vivo de lo que falta, para
> retomar en cualquier máquina. **No duplica detalle** — cada ítem apunta a su fuente
> autoritativa (`especificacion_mg.md` §19/§22, los `plan_*.md`, `CLAUDE.md`). Si un
> ítem y su fuente se contradicen, gana la fuente; actualiza aquí al cerrarlo.
>
> Reemplaza a los antiguos `PENDIENTES.md` (auditoría de backend V1, retirada en
> `4b9b4d4`) y `ROADMAP.md`, ya superados. Act. **2026-07-17**.
>
> **Filosofía del proyecto:** dirigido por demanda. Casi todo lo de abajo tiene *cero
> presión del corpus*; no se construye sin una figura que lo pida (evita especular).
> Build/test: `make` + `bash test/run.sh check` → **ok=48 … c3fail=0** (3 compuertas).

---

## 🎯 Las tres condiciones para el 1.0 (§22.7 — lista canónica, decidida con Alejandro)

Es lo único que bloquea salir de beta. **No hay más.**

1. **Congelar la gramática.** Declarar estable lo que hay; no añadir sintaxis nueva.
   Implica cerrar (2) primero. Es una decisión, no código.
2. **Cerrar lo aparcado de `plot`:**
   - [ ] **`rule` (§13.8)** — valores notables (líneas de referencia). Hijo del `plot`,
     ya decidido en diseño. **Desbloquea la forma AUTOMÁTICA de `legend`** (hoy solo la
     explícita, cerrada en `df652d9`). Figura esperando: `figure_02.pdf`. → mayor palanca.
   - [ ] **`table` (§13.10)** — reservado, **sin diseñar** todavía.
3. **Migrar el texto a UTF-8** (§14.4) — deuda de Latin-1, condición para salir de beta.
   Orden recomendado (del commit `ec8e695`): **P1 de `plan_lmmath.md`** (símbolos
   `map_symbol`→LM Math) primero; quitar Latin-1 después "es casi una consecuencia".
   La técnica ya está en el árbol (`font_lmmath_eps.h` embebe LM Math Type42 en EPS).

---

## 📌 Importa, pero NO bloquea 1.0

- [ ] **Texto multilínea §14.1** (`/n` rompe renglón) — especificado, sin implementar.
      Va `/n`, no `\n` (razón medida en `ec8e695`). El motor emite un `Text`/`TextLine`
      por renglón con interlínea derivada de `font_size`.
- [ ] **Math P2** (`plan_lmmath.md`): latino en modo math → itálica LM Math (hoy Times-Italic).
- [ ] **Plot Fase 3** (`plan_plot.md`): localizador automático `step`/`decimals`
      (1/2/5·10ᵏ), `format=` con validación, `at=v`, **`title_at=`** (título al extremo
      del eje — hoy obliga a `text()` manuales en fig4-5/fig6-4).
- [ ] **Pseudo-3D** (`plan_pseudo3d.md`): refinamientos acotados (`hatch` como parámetro
      de `plano`, refactor de `fig10-2v3`). MG **no** se vuelve 3D — simula volumen con
      shear por composición 2D.

---

## 🔧 Abiertos en spec §19 (definición o bajo costo; cero presión del corpus)

- [ ] **`spline`/`smooth` §9** — motor `splines.cpp` **listo**, barato; falta figura.
- [ ] **`marker_start/mid/end` en polygon/bezier** — en pausa, falta ejemplo
      (`plan_marcadores.md`). En polyline/arc ya está.
- [ ] **`spline(mode="conic")`** — cónicas (V1 `$S 1`); nativo (QuadTo SVG) vs conversión.
- [ ] **`place` por longitud de arco** — extender `gap=` a instancias cada *n* de arco.
- [ ] **Retícula por eje §13.6** y **`alpha` §4.11** (EPS sin nativo → decisión de arquitectura).
- [ ] **`both_sides` geométrico exacto** (perpendicular vs. especular) — documentar.
- [ ] **Texto bajo `transform`** — rotación de glifos YA implementada (§19, 2026-07-11);
      falta *definir en spec* si transforma solo el ancla o los glifos.
- [ ] **Ventanas anidadas §16** — desbloquea `axis(edge=)` suelto y paneles reales.
- [ ] **`transpose` §9** (`plan_transpose.md`, reposando) — falta 2º ejemplo + orientación.
- [ ] **Error: falta la columna** (§19; hoy reporta `archivo:línea`).

---

## 🪶 Deuda técnica — code-review follow-ups (NO bloqueantes, `plan_plot.md`)

- [ ] **#2** — bajo escala log solo se valida el RANGO (`x=`/`y=`), no los puntos ≤0 del
      contenido → dato ≤0 da coords NaN/inf mudas (`parserv3.cpp` `mapAxis`).
- [ ] **#5** — el detector "línea rellena" de la Capa 3 depende del orden de atributos del
      SVG (`d="…" fill="…"`); si SVGDisplay reordena, el gate deja de cazar en silencio.
- [ ] **#6** — `parsePlot` sobrescribe un 2º `xaxis`/`yaxis` sin avisar.

---

## ✅ Cerrado recientemente (contexto, no re-litigar)

- **Traductor `mg1to2.py`** (2026-07-17) — CERRADO y commiteado. Los 14 fixtures traducen
  y compilan (`bash test/run_translator.sh check` → ok=14); fig4-10 resuelto (canal `mtpt`
  como matriz afín + `RPPT` + `&ref` des-normalizado); decisión del radio de `dot` zanjada
  (`dot(diam/2)`, fiel). Se reabre por-constructo solo si material V1 real lo pide.
  Detalle: `plan_mg1to2.md` §11.
- **`legend` explícito §13.9 + `circle-dot` ⊙ §4.6** (`df652d9`); **polybar** (§4.12);
  **nomenclatura §13 + renombre + `label_at`**; **`plot` §13.7** (lineal+log+grid+`base=`).

> Roadmap de motor/continuidad e ítems ya resueltos: `especificacion_mg.md` §22.
> Decisiones de diseño (abiertas y cerradas): §19. Historial por tema: los `plan_*.md`
> (en `docs/plans/`; se citan por nombre desnudo en todo el árbol).

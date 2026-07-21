# PENDIENTES — MetaGráfica V3

> **Qué es esto.** Tablero de continuación: el estado vivo de lo que falta, para
> retomar en cualquier máquina. **No duplica detalle** — cada ítem apunta a su fuente
> autoritativa (`especificacion_mg.md` §19/§22, los `plan_*.md`, `CLAUDE.md`). Si un
> ítem y su fuente se contradicen, gana la fuente; actualiza aquí al cerrarlo.
>
> Reemplaza a los antiguos `PENDIENTES.md` (auditoría de backend V1, retirada en
> `4b9b4d4`) y `ROADMAP.md`, ya superados. Act. **2026-07-19**.
>
> **Filosofía del proyecto:** dirigido por demanda. Casi todo lo de abajo tiene *cero
> presión del corpus*; no se construye sin una figura que lo pida (evita especular).
> Build/test: `make` + `bash test/run.sh check` → **ok=54 … c3fail=0** (3 compuertas).

---

## 🎯 Las tres condiciones para el 1.0 (§22.7 — lista canónica, decidida con Alejandro)

Es lo único que bloquea salir de beta. **No hay más.**

1. **Congelar la gramática.** Declarar estable lo que hay; no añadir sintaxis nueva.
   Implica cerrar (2) primero. Es una decisión, no código.
2. **Cerrar lo aparcado de `plot`:**
   - [x] ~~**`rule` (§13.8)**~~ — CERRADO 2026-07-21 con `figure_02` (5 paneles, local por
     confidencialidad de los datos). Parser puro, cero motor y cero backends; log gratis
     por reuso del mapper de `plot`. **Desbloqueó la forma AUTOMÁTICA de `legend`**, que
     entró en la misma tanda junto con los anclajes `center-left`/`center-right`.
     Diferidos a propósito (nadie los pide): `marker=`, el **borrado de colisiones** entre
     malla y notable, y los **dos rótulos a la vez** (número al eje + prosa a la leyenda).
   - [ ] **`table` (§13.10)** — reservado, **sin diseñar** todavía. Sigue siendo lo único
     que bloquea el punto 2. `figure_02` genera sus Mean/SD/Min/Max pero no los dibuja.
3. **Texto fuera de Latin-1** (§14.4) — ✅ **HECHO 2026-07-20**. Resultó que el techo
   no era la codificación sino el **repertorio de la fuente**: las base-14 SÍ tienen
   comillas tipográficas, rayas, puntos suspensivos, ‰, ™, €… y se descartaban solo
   porque `ISOLatin1Encoding` no los nombraba. Ahora viajan en ranuras y cada backend
   traduce (EPS `/MGTextEncoding`, PDF CP1252, SVG UTF-8). Junto con P1 y P2 de
   `plan_lmmath.md`, cierra la 3ª condición.
   - [ ] **Queda, y es decisión aparte:** lo que la fuente NO tiene (griego en texto
     corrido, cirílico, CJK) exige **embeber una fuente de texto Unicode** en EPS. El
     subset math son 27 KB; una LM Roman completa, cientos. Nadie lo pide todavía.

---

## 📌 Importa, pero NO bloquea 1.0

- [ ] **Texto multilínea §14.1** (`/n` rompe renglón) — especificado, sin implementar.
      Va `/n`, no `\n` (razón medida en `ec8e695`). El motor emite un `Text`/`TextLine`
      por renglón con interlínea derivada de `font_size`.
- [x] ~~**Math P1 y P2**~~ — CERRADOS 2026-07-20: símbolos y latino de math a LM Math;
      el font Symbol y el markup `/g` desaparecen. Dígitos, operadores y puntuación
      de `$…$` entraron el mismo día (rectos, como en TeX; el `-` es el signo menos
      U+2212). Una fórmula ya no mezcla tipografías.
- [ ] **Plot Fase 3** (`plan_plot.md`): localizador automático `step`/`decimals`
      (1/2/5·10ᵏ), `format=` con validación, `at=v`, **`title_at=`** (título al extremo
      del eje — hoy obliga a `text()` manuales en fig4-4/fig6-4).
- [ ] **Pseudo-3D** (`plan_pseudo3d.md`): refinamientos acotados (`hatch` como parámetro
      de `plano`, refactor de `fig10-2v3`). MG **no** se vuelve 3D — simula volumen con
      shear por composición 2D.
- [ ] **Sección "figura paramétrica" en el README** (ambos idiomas) — `franck_condon.mg`
      prueba lo único que la portada afirma y **hoy nadie demuestra**: «a figure is source
      code, so it can be *parameterized* and regenerated». Versionar y hacer diff se ven
      solas; parametrizar no (`fig2-5` es magnífica, pero con coordenadas puestas a mano).
      Das `a`, `re`, `we`, `xe`, `Te` y caen la curva de Morse, `D=we/(4·xe)`, los niveles,
      los retornos, la envolvente WKB y `vmax`. **El gancho:** la vertical de Franck-Condon
      aterriza en v'≈6 *sin que nadie la coloque ahí* — sale del desplazamiento `re1`→`re2`.
      Eso es lo que ninguna herramienta de dibujo puede hacer.
      **Lo que falta decidir (el trabajo real):** qué parámetro se varía y generar el **par
      de renders comparativos** que responda "¿qué pasa si cambio un número?"; el texto es
      lo fácil. `quickstart.mg` se queda simple **a propósito** — es un quickstart.
      *No lleva `plan_*.md`: cero motor, cero compuertas; el plan duraría más que el trabajo.*

---

## 🔧 Abiertos en spec §19 (definición o bajo costo; cero presión del corpus)

- [x] ~~**Figura que ejercite `smooth` §9.2**~~ — CERRADO 2026-07-20: `examples/turning_points.mg`
      lo usa dos veces, y `smooth` ganó forma de primitiva.
      (`spline` y las cónicas se **retiraron** el mismo día, ver §9.1.)
- [ ] **`marker_start/mid/end` en polygon/bezier** — en pausa, falta ejemplo
      (`plan_marcadores.md`). En polyline/arc ya está.
- [ ] **`sample(&p, n)` §9** — devolver n puntos SOBRE la curva, no los que la definen:
      lo único que valía del `nodes=` de la `spline` retirada (muestrear es producir
      *datos*, no tinta). Volvería exactas a `path_width` (§8.2) y `path_x_bounds_at_y`,
      que hoy leen el polígono de CONTROL —en una bézier los puntos interiores son
      tiradores y no están sobre la curva— y por eso llevan advertencia. Falta figura.
- [ ] 🐞 **`place(Struct) { &path }` NO está implementado** (hallado 2026-07-20). §10.1 lo
      documenta («**3+ puntos** o una referencia **`&path`**») con ejemplo
      `place(Tick, scale=0.2) { &sinpi }`, pero `parsePlace` solo parsea coordenadas y
      nunca mira `T_AMP` → error de sintaxis. Divergencia spec/implementación: decidir si
      se construye o se retira de la spec. (Independiente de `sample`.)
- [ ] **`place` por longitud de arco** — extender `gap=` a instancias cada *n* de arco.
- [ ] **Retícula por eje §13.6** y **`alpha` §4.11** (EPS sin nativo → decisión de arquitectura).
      Los dos los pide `figure_02` y los dos se esquivaron ahí: la retícula se omitió, y el
      `alpha=0.5` sobre steelblue se sustituyó por el **color ya mezclado contra blanco**
      (`#A3C1DA`) — exacto, porque las barras no se solapan. Ese truco tapa el caso "relleno
      translúcido sobre fondo liso", que es la mayoría; lo que no cubre es el solapamiento.
- [ ] **Borrado de colisiones malla↔notable (§13.8)** — el premio del corte que la spec
      promete y que `rule` todavía no cobra: suprimir el rótulo de marca vecino a un `rule`
      con `label_at="axis"`. Necesita que el eje conozca los `rule` antes de emitir rótulos.
      Falta la figura que lo pida (`figure_02` manda sus nombres a la leyenda).
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
- [x] ~~**Barrer los `compound` con arcos**~~ — HECHO 2026-07-20. Barrido de los 19
      ejemplos comparando EPS y PDF **en la misma rejilla de píxeles** (clave: el
      `%%BoundingBox` del EPS es ENTERO y la página PDF no, así que recortar y reescalar
      mete ~1 px de error acumulado y todo parece divergir). Encontró un 2º bug de la
      misma familia (cuerda de cierre trazada por `outlinefill` en PDF, ver `franck_condon`).
      Tras arreglarlo, `fig2-1` y `fig4-1` quedan en 0.00% y el resto por debajo del 5%,
      con diferencias SIMÉTRICAS = métrica de texto, no estructura.
- [ ] **Fase de los guiones EPS vs PDF** — en `primitives` las formas discontinuas
      arrancan la secuencia de `dash` en fase ligeramente distinta entre los dos backends
      (396 px de diferencia, solo en los trazos punteados). Cosmético; nadie lo ha pedido.
- [ ] **Fase de la TRAMA: SVG no coincide con EPS/PDF** (hallado 2026-07-20 en el barrido
      a tres bandas). EPS y PDF barren las líneas desde el CENTRO de la caja del bbox;
      SVG tesela un `<pattern>` desde el ORIGEN del espacio de usuario. En el polígono de
      `primitives` las líneas de uno y otro se **intercalan** —media separación de
      desfase—; en el rectángulo coinciden de casualidad. La verificación de 2026-07-09
      («EPS/SVG/PDF idénticos») comprobó el ÁNGULO, no el origen. Cosmético mientras nadie
      compare dos formatos de la misma figura lado a lado.

> **Barrido a tres bandas (2026-07-20).** Los 19 ejemplos, EPS/PDF/SVG desde el golden.
> Sin bugs estructurales nuevos. Todo lo que queda es (a) posición de glifos —métricas de
> avance distintas por backend, ≤5%, se ve como franja roja/azul a los lados del MISMO
> glifo—, (b) la fase de trama de arriba y (c) la de guiones. Dos trampas de método, por
> si se repite: **misma rejilla de píxeles** (el `%%BoundingBox` de EPS es ENTERO y la
> página PDF no → recortar y reescalar mete ~1 px de error acumulado y todo parece
> divergir), y **contar cualquier tinta no blanca**: `gs` endurece las hairlines a 1 px
> sólido mientras inkscape las antialiasea a gris claro, así que con un umbral duro el
> SVG «pierde» todas las líneas finas (31% de divergencia inventada en `fig2-1`).

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

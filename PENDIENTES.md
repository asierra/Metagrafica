# PENDIENTES — MetaGráfica V3

> **Qué es esto.** Tablero de continuación: el estado vivo de lo que falta, para
> retomar en cualquier máquina. **No duplica detalle** — cada ítem apunta a su fuente
> autoritativa (`especificacion_mg.md` §19/§22, los `plan_*.md`, `CLAUDE.md`). Si un
> ítem y su fuente se contradicen, gana la fuente; actualiza aquí al cerrarlo.
>
> Reemplaza a los antiguos `PENDIENTES.md` (auditoría de backend V1, retirada en
> `4b9b4d4`) y `ROADMAP.md`, ya superados. Act. **2026-07-21**.
>
> **Filosofía del proyecto:** dirigido por demanda. Casi todo lo de abajo tiene *cero
> presión del corpus*; no se construye sin una figura que lo pida (evita especular).
> Build/test: `make` + `bash test/run.sh check` → **ok=57 … imgfail=0** (4 compuertas).
> Traductor: `bash test/run_translator.sh check` → **ok=14** (`tools/mg1to2.py`).

---

## 🎯 Las cinco condiciones para el 1.0 (§22.7 — lista canónica, decidida con Alejandro)

Es lo único que bloquea salir de beta. **No hay más.** ⚠️ **Es una SECUENCIA, no una lista
de pendientes:** el orden real de ejecución es **2 → 5 (borrador) → 4 → 1 → 5 (final)**, y
(3) ya está. Los números son estables (se citan desde otros documentos); lo que no sigue el
orden de la lista es la ejecución.

1. **Congelar la gramática.** Declarar estable lo que hay; no añadir sintaxis nueva.
   Implica cerrar (4) y el borrador de (5) primero. Es una decisión, no código.
2. ✅ **Cerrar lo aparcado de `plot` — HECHO 2026-07-21** (`rule`, `legend` automática y
   `table`; los tres, trabajo de parser sin tocar motor ni backends):
   - [x] ~~**`rule` (§13.8)**~~ — CERRADO 2026-07-21 con `figure_02` (5 paneles, local por
     confidencialidad de los datos). Parser puro, cero motor y cero backends; log gratis
     por reuso del mapper de `plot`. **Desbloqueó la forma AUTOMÁTICA de `legend`**, que
     entró en la misma tanda junto con los anclajes `center-left`/`center-right`.
     Diferidos a propósito (nadie los pide): `marker=`, el **borrado de colisiones** entre
     malla y notable, y los **dos rótulos a la vez** (número al eje + prosa a la leyenda).
   - [x] ~~**`table` (§13.10)**~~ — CERRADO 2026-07-21, con los cinco recuadros
     Mean/SD/Min/Max de `figure_02`. Parser puro, cero motor y cero backends. **No es
     hija exclusiva de `plot`** (a diferencia de `rule`): una tabla no depende del mapeo
     de datos, solo de un rectángulo, así que `at=` está sobrecargado —esquina nombrada
     dentro de un plot, punto `(x,y)` fuera—. Y **sí lleva marco**, que la leyenda no
     pudo tener: declarar `col_widths=` en pt es justo lo que hace calculable la caja
     sin medir texto.
   - ✅ **Con eso el punto 2 queda COMPLETO.**
3. **Texto fuera de Latin-1** (§14.4) — ✅ **HECHO 2026-07-20**. Resultó que el techo
   no era la codificación sino el **repertorio de la fuente**: las base-14 SÍ tienen
   comillas tipográficas, rayas, puntos suspensivos, ‰, ™, €… y se descartaban solo
   porque `ISOLatin1Encoding` no los nombraba. Ahora viajan en ranuras y cada backend
   traduce (EPS `/MGTextEncoding`, PDF CP1252, SVG UTF-8). Junto con P1 y P2 de
   `plan_lmmath.md`, cierra la 3ª condición.
   - [ ] **Queda, y es decisión aparte:** lo que la fuente NO tiene (griego en texto
     corrido, cirílico, CJK) exige **embeber una fuente de texto Unicode** en EPS. El
     subset math son 27 KB; una LM Roman completa, cientos. Nadie lo pide todavía.
4. **Uso real por gente que no es el autor** (añadida 2026-07-21). Las tres primeras
   preguntan *¿está construido?*; esta pregunta *¿está bien?*. Un periodo de figuras
   escritas por otras personas, con las mejoras que motiven, **antes** de congelar.
   - **Va antes que (1) porque lo que compra la palabra «beta» es el permiso para
     romper.** Hoy un renombre cuesta un `sed`; después de 1.0 cuesta una migración y un
     número mayor. El criterio no es que pase tiempo (tiempo sin usuarios no prueba nada)
     sino tener evidencia de que el permiso ya no hace falta.
   - **Va después de (2)** porque con huecos la retroalimentación es «le falta X», no «X
     es incómodo», que es la única que sirve para decidir nombres.
   - 🔎 **La prueba de que hace falta ya está en el repo:** el renombre de §13 no lo cazó
     el corpus ni ninguna de las compuertas — lo cazó comparar con matplotlib. Salió
     gratis *solo* porque no había usuarios. El autor no puede hacerse esa prueba solo:
     ya sabe cómo se llaman las cosas.
   - **Salida propuesta (por evidencia, no por calendario):** que alguien de fuera escriba
     figuras no triviales desde cero y las figuras nuevas **dejen de mover la gramática**.
     Es la regla del proyecto («no se construye sin una figura que lo pida») aplicada a los
     nombres. Opcional al final: marcar `3.0.0-rc.1` antes del `3.0.0`.
   - ⚠️ **Depende del borrador de (5):** sin referencia, lo que devuelven los usuarios es
     sobre *descubribilidad* («no encontré cómo hacer X»), no sobre *ergonomía* («lo
     encontré y es incómodo»), que es la que sirve para decidir nombres.
5. **Una referencia de usuario** (añadida 2026-07-21; estaba solo en el README y sube a la
   lista canónica). Hoy hay tres documentos y **ninguno es el que un usuario necesita**: el
   README es una portada, `man mg` documenta el **binario** y no el lenguaje, y esta spec es
   **prospectiva** (describe lo que aún no existe y razona diseño). Falta el documento que
   describe, completo y sin historia, **lo que hay**.
   - **Es la única que CRUZA el congelamiento**, y por eso aparece dos veces en el orden:
     **borrador antes de (1)**, **final después**. La final va después por lo obvio: una
     referencia escrita contra una gramática que aún se mueve se reescribe con cada
     renombre. El borrador va antes por dos razones, y ninguna es de cortesía:
     1. **(4) la necesita.** No se le puede pedir a nadie de fuera que escriba figuras si
        no tiene qué leer; sin referencia lo que devuelven es sobre descubribilidad, no
        sobre ergonomía.
     2. 💡 **Escribirla ES una revisión de diseño.** Documentar cada constructo de forma
        sistemática obliga a ponerlos uno al lado del otro — exactamente el ejercicio que
        destapó el renombre de §13. Es previsible que levante inconsistencias, y conviene
        que lo haga **mientras todavía son baratas**.
   - 🔎 **Evidencia de campo REFORZADA (2026-07-21): el propio autor tropezó** armando
     `tiro_parabolico`. Cuatro veces, y son exactamente los temas que la referencia debe
     aclarar (ya añadidos como trampas a `docs/referencia.md`): (1) **datos fuera de
     `world_window`** —tres veces, con síntomas que parecen bugs del motor («EPS negro»,
     «solo 3 puntos»)—; (2) **«no lineal ≠ log»** al querer una malla por los puntos; (3)
     **`&trayecto` va primero** (`dot(2,&p)` falla); (4) la semántica de **`+=` que suelda
     relativo**. Si el autor se confunde, un recién llegado se estrella — es la validación
     más fuerte del requisito, y de que escribir la referencia AFLORA la fricción (cond. 4)
     antes de congelar.
   - 🤖 **Requisito de diseño: que pegarla en el contexto vuelva competente a un agente.**
     Lo que hace competente a un agente frío y lo que hace competente a un recién llegado
     son **casi el mismo requisito** —completa, autocontenida, guiada por ejemplos y
     explícita en las trampas—, así que no es un objetivo aparte: es un argumento más para
     el mismo documento, y una vara concreta para saber si está terminado.
     - **Por qué importa, y no es moda.** Un agente que entrega un SVG te deja aceptarlo o
       abrir Illustrator; uno que entrega un `.mg` te deja **cambiar un número**. El
       diferenciador no es «MG es amigable para la IA» sino que **el resultado sigue siendo
       editable después de que el agente termina**. Es la afirmación que el proyecto ya
       hace —una figura es código fuente— extendida a un colaborador nuevo.
     - **Evidencia de campo (sesiones del 2026-07-21):** el agente escribió `figure_02.mg`
       y `local/karl.mg` y el autor los editó directamente encima (tamaño de los `dot`,
       color de las ondas, categorías, etiquetas de las cercas). Y el caso que ningún
       formato de salida iguala: cambiar `xe1` de `0.028` a `0.045` reacomodó pozo,
       niveles, retornos y ondas de forma coherente.
     - 💡 **Los tropiezos del agente son datos de ergonomía.** Escribiendo esas figuras se
       tropezó en cuatro sitios, tres de superficie del lenguaje: indexar un literal de
       lista, `outlinefill` como supuesto atributo por-primitiva, los paréntesis de la
       Lección 7 en bloques de coordenadas, y `polybar(&p, width=…)` —que resultó ser una
       restricción real y se quitó—. **Donde tropieza un agente tropieza una persona**, y
       sale mucho más rápido: es una forma barata de aflorar la fricción de (4) antes de
       congelar, aunque NO la sustituye.
     - ⚠️ **Cautela sobre esa evidencia:** el agente tenía la spec entera, el `CLAUDE.md` y
       el corpus en contexto. Uno frío con solo el README lo haría bastante peor — lo que
       **no debilita el argumento, lo es**: prueba que MG es tratable *con buena
       documentación*, que es justo lo que falta.
     - 🚫 **Y una decisión de redacción: «amigable para IA» NO va al README.** Envejece
       mal, no es verificable y en dos años sonará fechado. Lo que dura y se comprueba:
       *la figura sigue siendo texto editable, la escriba quien la escriba*. Quien quiera
       leer ahí la ventaja con agentes, la leerá.

---

## 📌 Importa, pero NO bloquea 1.0

- [x] ~~**Texto multilínea §14.1**~~ — CERRADO 2026-07-21. `/n` rompe renglón; el motor
      gana `TextBlock` (`GI_TEXTBLOCK`), que apila renglones ya construidos con interlínea
      derivada de `font_size`. **En el motor y no en el parser** porque el tamaño de fuente
      solo existe en draw-time; resolverlo en parse-time habría sido el cuarto bug de la
      familia `FN_NOFACE`. `valign` aplica al BLOQUE. Cero cambios en los tres backends,
      cero churn (ok=57). `TextStruct` sigue reservado para composición 2-D o LaTeX.
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

- [~] 💡 **La familia de operaciones sobre paths §9 — α+β IMPLEMENTADO 2026-07-21;
      reducciones y cobertura PENDIENTES.** El hallazgo de comparar con CeTZ y MetaPost
      (`local/karl.mg`). La lección de MetaPost NO es copiar una operación: es que **el path
      es un tipo algebraico coherente** y su vocabulario se compone. MG lo empezó
      (concat/reverse/flip/transpose componen) y se detuvo en tres funciones ad-hoc
      (`path_width`, `path_x_min_at_y`, `path_x_max_at_y`).
      - 🎯 **La pieza-palanca es evaluar el path en un parámetro `t`.** De ella cuelgan:
        `point_at(&p,t)` (punto exacto), las reducciones dejando de aproximar, la "ancla" de
        CeTZ (`point_at(&p,0.5)`, sin sistema de nombres), e `intersectionpoint` (del que
        `path_x_*_at_y` es el caso especial — por eso están sin uso).
      - ✅ **DECISIONES (revisar antes de tocar código, pero cerradas):**
        - **Nombres CORTOS, sin prefijo `path_`** (el `&p` ya dice que es un path):
          `sample(&p, n)`, `point_at(&p, t)`, `angle_at(&p, t)`.
        - **DOS nombres, no sobrecarga:** `sample(&p, n)` (n = nº de puntos, path-valuado)
          y `point_at(&p, t)` (t∈[0,1], punto-valuado) son operaciones distintas con retorno
          distinto; NO se funden sobrecargando el 2º arg (¿`3` es «3 puntos» o «en t=3»?).
        - 🔑 **α+β con un flag `curve=`, NO γ (tipos en el path).** Medido 2026-07-21: un
          path-valor es NEUTRO —el mismo `&sine` se dibuja entrecortado con `polyline` y
          suave con `bezier`; la interpretación la pone la PRIMITIVA, no el valor—. Así que
          el path no puede saber si es vértices o controles: **el flag carga esa
          interpretación**, no el tipo del valor. γ rompería ese invariante (habría que
          redefinir `polyline(&curva)`), y es el cambio más grande; descartado.
          - `curve=false` (DEFAULT): puntos como VÉRTICES → interp/cruce lineal. Exacto en
            polilíneas; sobre una bézier toca la ENVOLVENTE. **Es lo que hace HOY
            `path_x_bounds_at_y`** (`splines.cpp`: cruza pares consecutivos con recta), así
            que α = comportamiento actual → congelable con CERO churn.
          - `curve=true`: puntos como CONTROLES bézier (3k+1) → evalúa la cúbica. Toca la
            CURVA real. **Vuelve EXACTAS las reducciones** —el defecto de la envolvente que
            se señaló en pegaso NO era anticipatorio: β lo arregla—.
          - El MISMO `curve=` en toda la familia: `sample`/`point_at`/`angle_at` y las
            reducciones. Un solo modelo mental.
        - **`path_x_*_at_y` se REDEFINEN**, no se retiran: pasan a ser
          `path_x_min_at_y(&p, y, curve=false)` — el caso `curve=true` es lo que les faltaba.
      - ✅ **HECHO 2026-07-21:** `sample(&p, n [, curve=b])` (path-valuado),
        `point_at(&p, t [, curve=b])` (devuelve `[x,y]`), `angle_at(&p, t [, curve=b])`
        (número). Geometría en `splines.cpp` (`bezier_point` + `arc_table`/`path_point`/
        `path_sample`/`path_angle`, longitud de arco por teselación a `SUB=24` tramos/segmento);
        cableado en `parserv3.cpp`. `curve=` acepta nombrado o posicional. Verificado: los rojos
        (`curve=true`) sobre la curva, los verdes (default) sobre la envolvente. Cero motor
        nuevo en backends, golden `ok=60` (aditivo). ⚠️ **SIN cobertura en el corpus** —función
        de lenguaje sin figura que la ejercite = se puede romper en silencio; cerrar con un
        ejemplo (candidato: la demo curva-vs-envolvente).
      - ⏳ **DIFERIDO a otra tanda:** añadir el flag `curve=` a las reducciones existentes
        (`path_x_min_at_y`/`path_x_max_at_y`/`path_width`) para que puedan tocar la curva. Hoy
        siguen tocando la envolvente. **`path_width` CONSERVA su prefijo `path_`** (a diferencia
        de los cortos `sample`/`point_at`): `width` colisiona con el atributo (`polybar(width=)`,
        `sample_width=`…), y el prefijo lo desambigua.
      - 🔧 **Qué falta en `splines.cpp` (revisado 2026-07-21):**
        - `bezier_point(p0,c1,c2,p1,t)` — Bernstein cúbico, ~6 líneas (la evaluación en `t`
          YA existe en `splines()` pero en base Catmull-Rom, `point p = c0+u*(c1+u*(c2+c3*u))`).
        - `path_point(path, t)` con `t` GLOBAL: **la longitud de arco NO es opcional** (spike
          2026-07-21). En segmentos DESIGUALES —el caso de casa— `t=0.5` naive cae a **2.9
          unidades** del medio geométrico, y *sobre la curva* (error plausible). Costo:
          teselar+acumular, ~40 líneas, reutilizando la teselación que MG ya hace para
          dibujar. De una POLILÍNEA (α, curve=false) la longitud de arco es trivial (sumar
          segmentos, sin teselar).
      - ⚖️ **Tensión filosófica ZANJADA por escrito:** `point_at` *interroga una curva*, y
        `docs/calcular_en_vez_de_medir.md` predica *derivar de la fórmula, no medir del
        dibujo*. No se contradicen —leer un punto EXACTO de una curva DERIVADA sigue siendo
        cálculo, no ojo—. La operación es legítima; su lugar es una curva con modelo detrás,
        no un trazo a mano.
      - **Pendiente ORTOGONAL** (no de α/β): el punto `[x,y]` que devuelve `point_at` solo
        entra por `at=` de una STRUCT (`Mk(at=point_at(...))`). Hallado al armar
        `path_sample` (2026-07-21): las **primitivas no tienen `at=`** —`marker(shape="arrow",
        at=…)` COMPILA pero no dibuja (0 arcos), porque `at=` es colocación de structs— y el
        **bloque `{ }` no desempaqueta la lista**. Así que hoy, para colocar una primitiva en un
        punto muestreado, hay que envolverla en una struct (es lo que hace `path_sample.mg` con
        la flecha). Cerrar el hueco = enseñar a `{ }` a aceptar un punto `[x,y]` (misma solución
        que `&path` en bloques), lo que además dejaría escribir `marker(...) { point_at(...) }`.
- [ ] **Tangente declarada en un nodo (`{dir 30}`, `tension`) §9** — el término medio que
      MG no tiene: `bezier` te hace poner los tiradores y `smooth` los deriva todos.
      MetaPost deja decir «pasa por este nodo **saliendo a 30°**», que es justo lo que se
      quiere cuando una curva debe empalmar con una recta en un ángulo dado —
      `franck_condon` lo resuelve hoy eligiendo `phase=90/270` a mano. Falta figura que lo
      exija, pero la candidata está cerca.
- [ ] **Algoritmo de Hobby para `smooth` §9.2** — es el que usa el `..` de MetaPost y la
      referencia de «curva agradable»; MG usa Catmull-Rom centrípeto (parametrización
      corregida el 2026-07-20). No urge y no está roto: es la comparación honesta si
      alguna vez se cuestiona la calidad de `smooth`.
- [ ] **Simplificación de curvas: de muchos puntos a un spline sencillo.** Dado un conjunto
      grande de puntos —los **69 digitalizados por curva** de `fig4-4` son el caso de
      casa— reducirlo a pocos segmentos bézier. Candidato: **SimpliPoly** (Chuon, Guha,
      Janecek & Song, *Int. J. Comput. Geom. Appl.* 21(4), 2011; PDF en el Dropbox del
      autor). Aproxima trozos con bézier, estima la **curvatura** en los vértices a partir
      de esas aproximaciones, y simplifica guiado por curvatura — mejor que Douglas-Peucker
      «en aplicaciones donde hay que preservar rasgos locales», que es exactamente el caso
      de una curva de física (la rodilla de `1/r`, el mínimo del potencial efectivo).
      - **Decidir si es método del lenguaje o herramienta aparte** (`tools/`, junto a
        `hist2mg.py`). Inclinación: herramienta — reducir puntos es *producir datos*, no
        tinta, la misma frontera que separó `polybar` de `hist2mg.py` y la que argumenta
        `docs/calcular_en_vez_de_medir.md`. **No urge.**
- [ ] **Mezclar recto y curvo en UNA trayectoria §9** (`z1 -- z2 .. z3` de MetaPost). Hoy
      son primitivas distintas (`polyline` / `smooth` / `bezier`) y se unen con `concat`,
      que funciona pero con más ceremonia.
      - ⚠️ **La limitación NO es de EPS**, que era la sospecha: PostScript, PDF y SVG
        permiten `lineto` y `curveto` en el mismo path, y `Display` ya expone los dos. La
        limitación es de **la representación de MG**: `Polyline::draw` consume el path de
        **tres en tres** cuando el tipo es `GI_BEZIER`, así que todo segmento es curva.
        Arreglarlo pide un tipo por segmento en el path — más hondo de lo que parece, pero
        **sin tocar los backends**, que es la clase de cambio que este proyecto prefiere.
      - El rodeo de V1 sigue siendo válido y **es exacto, no aproximado**:
        `&straightline 0 0  .3333 0  .6666 0  1 0` (`examples/v1/bzsinepaths.mg`) — una
        cúbica con los controles a 1/3 y 2/3 ES la recta.

> 🚫 **Decidido NO construir: el solucionador de ecuaciones de MetaPost.** `z3 = z1 +
> whatever*dir(60) = z2 + whatever*dir(-50)` declara relaciones y despeja el sistema. Es la
> alternativa coherente más seria que existe a lo que hace MG, y es **la filosofía
> contraria**: declarar la relación en vez de derivar la fórmula
> (`docs/calcular_en_vez_de_medir.md`). Se descarta por tres razones: pide un subsistema de
> álgebra lineal con incógnitas y unificación; cambia el modelo mental de **todo** el
> lenguaje, no de una construcción; y MG ya eligió por escrito el camino opuesto. Queda
> anotado como **decisión y no como omisión**, porque va a volver a aparecer.

- [ ] 🐞 **`&path` solo se reconoce como PRIMER argumento de una primitiva** (hallado
      2026-07-21 con `parabolic`). `dot(&p, size=2)` y `marker(&p, shape="x")` funcionan;
      `dot(2, &p)` (radio primero) y `dot(...) { &p }` (path en el bloque de coordenadas)
      NO. Dos asperezas: (a) `startsPathExpr` solo mira el primer arg, así que el path debe ir
      antes del radio —lo que uno escribe primero es `dot(2, &p)` y falla—; (b) el bloque
      `{ }` no acepta `&path`, la misma divergencia que `place(Struct) { &path }` de §10.1. El
      mensaje («se esperaba una expresión, se encontró `&`») **no orienta** a poner el path
      primero. Dato de ergonomía de superficie: resolver o mejorar el mensaje antes de congelar.
- [ ] 🐞 **`smooth(&p)` funciona como PRIMITIVA pero no como EXPRESIÓN de path** (matiz
      afinado 2026-07-21 al suavizar `parabolic`). `smooth(&parabola)` que DIBUJA compila y
      suaviza bien (6 curveto, pasa por los nodos); pero `polyline(smooth(&parabola))` y
      `path s = smooth(&parabola)` fallan con «se esperaba '{' de los puntos de smooth». Es
      decir: `parsePathExpr` acepta `smooth { pts }` literal pero no `smooth(&otro)`, mientras
      que la forma-primitiva sí toma `&p`. Sus hermanas de álgebra `flip_x`/`reverse`/`concat`
      toman `&p` en las DOS posiciones, así que la asimetría se nota. La spec §9.2 lista
      `smooth(&p)` como expresión válida → divergencia. Barato (la maquinaria existe): unificar
      para que `smooth` acepte `&p` en ambas posiciones, o retirar la forma-expresión de §9.2.
- [ ] 🐞 **Un literal de lista no se puede indexar**: `[10,20,30][1]` es error de sintaxis
      («se esperaba un comando… pero se encontró `[`»), y también dentro de un bloque de
      coordenadas. Hay que pasar por una variable (`xs = [10,20,30]` y luego `xs[i]`), que
      sí funciona, igual que el índice fuera de rango, que da error claro. Puede haber una
      razón real (ambigüedad con el bloque de coordenadas), pero **el mensaje no orienta**:
      no nombra las listas. O se permite, o el error lo dice.

- [x] ~~**Figura que ejercite `smooth` §9.2**~~ — CERRADO 2026-07-20: `examples/turning_points.mg`
      lo usa dos veces, y `smooth` ganó forma de primitiva.
      (`spline` y las cónicas se **retiraron** el mismo día, ver §9.1.)
- [x] ~~🐞 **`docs/img/*.svg` no tiene compuerta**~~ — CERRADO 2026-07-21, mismo día que se
      halló. Es la **4ª compuerta** (`imgfail`): `check` compara cada `docs/img/X.svg` contra
      lo que el compilador produce hoy. Un ejemplo entra por tener ahí un `.svg` con su
      nombre —la presencia del archivo ES la declaración, no hay lista que mantener—, y
      `capture` **NO** los regenera (`test/golden` es borrador local; `docs/img` es salida
      publicada, y bendecirla debe ser un commit consciente): para eso está el modo
      `images`. Verificada reintroduciendo el archivo rancio **real** de `e9198c0`.
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
- [x] ~~**Retícula por eje §13.6**~~ — CERRADO 2026-07-21: `xaxis(grid=…)`/`yaxis(grid=…)`
      (+ `grid_dash=`), con el `grid=` de `plot` como atajo para ambos y el eje ganando sobre
      él. **No** se hizo `grid="y"`: `grid=` ya está sobrecargado con color, así que la "y" se
      habría leído como color desconocido → malla negra en los dos ejes. En la misma tanda,
      `plot(frame=true)`, que reusa el `box=` en vez de obligar a un `rectangle` por panel.
- [ ] **`alpha` §4.11** (EPS sin nativo → decisión de arquitectura). Lo pide `figure_02` y ahí
      se esquivó: el `alpha=0.5` sobre steelblue se sustituyó por el **color ya mezclado contra
      blanco** (`#A3C1DA`) — exacto, porque las barras no se solapan. Ese truco tapa el caso
      "relleno translúcido sobre fondo liso", que es la mayoría; no cubre el solapamiento.
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

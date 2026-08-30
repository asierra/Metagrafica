# PENDIENTES — MetaGráfica V3

> **Qué es esto.** Tablero de continuación: el estado vivo de lo que falta, para
> retomar en cualquier máquina. **No duplica detalle** — cada ítem apunta a su fuente
> autoritativa (`especificacion_mg.md` §19/§22, los `plan_*.md`, `CLAUDE.md`). Si un
> ítem y su fuente se contradicen, gana la fuente; actualiza aquí al cerrarlo.
>
> Reemplaza a los antiguos `PENDIENTES.md` (auditoría de backend V1, retirada en
> `4b9b4d4`) y `ROADMAP.md`, ya superados. Act. **2026-08-30**.
>
> También reemplaza a **`ideas.txt`** (borrador fundacional de V3, borrado el 2026-07-22).
> Se repasaron sus 18 puntos contra el código: 14 están superados —varios muy por encima de
> lo que el documento imaginaba (el álgebra de paths §9, `axis`/`plot`, structs
> parametrizadas)— y dos más quedaron **resueltos por decisión**, no por omisión: las
> splines cónicas se retiraron (§9.1) y la normalización «de 0 a 1» del espacio se revirtió
> a propósito (§3.1, isométrico por construcción). Lo único que **no** estaba en ningún
> tablero son los tres ítems marcados 📥 abajo (`max_depth`, `exit`, inventario de loops).
> La reescritura del parser con «herramientas modernas» que proponía su último párrafo se
> da por **decidida en contra por los hechos**: el descenso recursivo a mano aguantó §13
> entero y reescribirlo pelea de frente con la condición 1.
>
> **Filosofía del proyecto:** dirigido por demanda. Casi todo lo de abajo tiene *cero
> presión del corpus*; no se construye sin una figura que lo pida (evita especular).
> Build/test: `make` + `bash test/run.sh check` → **ok=96 … docfail=0 citafail=0 humofail=0**
> (32 ejemplos × 3 backends; **diez** compuertas,
> razonadas una por una en `CLAUDE.md`; la 5ª son 65 pruebas NEGATIVAS en `test/errors/`,
> ampliadas el 2026-07-28 a los diagnósticos NO fatales con `EXPECT_WARN`/`EXPECT_NO_WARN`;
> la 6ª vigila que `docs/galeria.html` no quede rancia — la publica GitHub Pages y lleva
> incrustado el código de cada ejemplo, así que **editar un comentario la desactualiza** y
> ninguna de las otras puede verlo; la 7ª, que `docs/reference.md` no sea una traducción
> VIEJA; la 8ª, nueva el 2026-07-29, compila los bloques ```octave de la referencia en los
> dos idiomas: **la documentación también puede mentir**, y a un modelo de lenguaje le miente
> con éxito; la 9ª, del 2026-08-05, comprueba que lo que la documentación **cita** siga siendo
> lo que el archivo dice —fallo disjunto del anterior: una cita rancia COMPILA—; y la 10ª
> corre las herramientas de `tools/` que ningún `check` ejercita). La Capa 3 tiene **cuatro**
> invariantes: la 3ª, del 2026-07-27, es la
> paridad **geométrica** de arcos entre los tres backends (`tools/arcparity.py`) — la
> única sin escapatoria por bendición, porque no compara contra un golden sino un backend
> contra otro (ver `plan_anisotropia.md`); la 4ª, del 2026-07-28, cuenta los **rellenos
> degradados** en los tres formatos.
> Traductor: `bash test/run_translator.sh check` → **ok=14** (`tools/mg1to2.py`).

---

## 🎯 Las cinco condiciones para el 1.0 (§22.7 — lista canónica, decidida con Alejandro)

Es lo único que bloquea salir de beta. **No hay más.** ⚠️ **Es una SECUENCIA, no una lista
de pendientes:** el orden real de ejecución es **2 → 5 (borrador) → 4 → 1 → 5 (final)**, y
(3) ya está. Los números son estables (se citan desde otros documentos); lo que no sigue el
orden de la lista es la ejecución.

1. **Congelar la gramática.** Declarar estable lo que hay; no añadir sintaxis nueva.
   Implica cerrar (4) y el borrador de (5) primero. Es una decisión, no código.
   - [x] ~~📥 **Inventario de los loops que ya existen**~~ — **HECHO**: está escrito en
     `docs/referencia.md` §7, «Todas las formas de repetir», y **llegó a un veredicto**, que
     era el punto: hay **un solo lazo general** (`for`); el **bloque de coordenadas ya es un
     lazo** y `place` con 3+ puntos es exactamente ese mismo lazo para structs (una idea con
     dos nombres, y el nombre aparte existe porque una struct no es una primitiva); lo que
     justifica a `repeat` es la **acumulación** de la transformación —sin acumular, un `for`
     se lee mejor—; y `sample` **no itera**, produce datos. O sea: no son seis conceptos,
     son tres. Con eso, la condición 1 ya no está bloqueada por este ítem. Texto original:
     (de `ideas.txt`, punto nunca cerrado:
     «antes de proponer sintaxis nueva, hacer inventario de los loops que ya existen
     implícitamente»). Hoy son más que entonces: `for` (§6), `repeat` (§17, el que **acumula**
     transformación), `place` (§10.1, distribuir a lo largo de un locus), `numbers`/`ticks`
     (§13, generadores), `polybar` (una barra por punto) y `sample` (§9). **Va aquí y no en
     «importa pero no bloquea»** porque es exactamente el ejercicio de poner los constructos
     uno al lado del otro que hay que hacer ANTES de congelar —el mismo que destapó el
     renombre de §13—: la pregunta es si seis formas de iterar son seis conceptos distintos o
     tres con nombres inconsistentes. **Cero código**: es una revisión, y su salida natural es
     una sección de la referencia (5). No añadir sintaxis nueva de loops sin este inventario.
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
   - ⏳ **Cobertura: `rule` y `table` NO los compila NINGÚN ejemplo del corpus**
     (verificado 2026-08-03). `rule` tiene una línea suelta en un bloque de
     `docs/referencia.md` que sí pasa por `docfail`; **`table` no tiene ni eso**. Su
     único cliente es `figure_02`, que vive en `local/` por confidencialidad. Es el
     mismo agujero que tenía `lib/pseudo3d.mg` hasta el 2026-08-03.
     📌 **NO se le construye un ejemplo público a propósito** (decidido con Alejandro
     el 2026-08-03): el artículo está en **segunda revisión**, y al publicarse la
     figura deja de ser confidencial y puede entrar al corpus tal cual, que es el
     cliente REAL. Un ejemplo inventado solo para la compuerta sería peor prueba y
     habría que retirarlo después.
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
   - [x] ~~🐞📥 **`max_depth` (§18): una recursión sin paro SEGFAULTEA**~~ — **CERRADO
     2026-07-22**, el mismo día que se halló (repasando `ideas.txt`, cuyo punto era
     «recursión **controlada**»). Era el **único** modo de falla del compilador que moría
     por señal en vez de por `evalError`: `struct r(n) { … r(n+1) }` daba **Segmentation
     fault (exit 139)** y `max_depth 8` respondía *«sentencia de estado desconocida»*.
     - **Implementación:** `g_maxDepth` (default **32**, el que sugiere §18) + `g_structDepth`,
       y un helper **`execStructBody`** por el que pasan los **cinco** sitios que expanden un
       cuerpo de struct (`InvokeStmt`, `RepeatStmt`, `FitStmt`, `buildStructure` y el volcado
       temporal de plot-log). Ese punto único es lo que importa: verificado que las **cuatro**
       vías de invocación —invoke, `fit(Struct(...))`, `place`, `repeat`— quedan cubiertas;
       con la guarda puesta en `InvokeStmt` sola, tres de ellas seguían muriendo.
     - `max_depth n` entra como **control de documento** (`isConfig`, junto a `display_size`/
       `world_window`) y **no** como sentencia de estado: no es estado gráfico —no se acota por
       bloque ni lo restaura `gsave`/`grestore`—, es un tope del compilador. `max_depth 0` es
       `evalError`.
     - **Cuenta ANIDAMIENTO, no invocaciones:** verificado que 40 invocaciones planas de la
       misma struct no gastan profundidad. Era el riesgo obvio de un contador global.
     - **Cero churn:** los 63 goldens previos byte-idénticos.
     - ✅ **Cobertura resuelta el mismo día:** la guarda no la puede ejercitar ninguna figura
       (`fractal_tree` para en `if`), y eso destapó que el harness **no tenía dimensión de
       error**. Ahora la cubren dos fixtures de la 5ª compuerta (`max_depth_excedido`,
       `max_depth_cero`), y el caso del segfault es lo que motivó exigir **`exit == 1`
       exacto** en vez de «≠ 0».
     - **Va en esta condición, no en «importa pero no bloquea»:** es el **único** modo de
       falla del compilador que acaba en crash en vez de en `evalError` —la política del
       proyecto es que un documento roto aborte con mensaje, no que tumbe el binario— y
       le toca justo a quien estrena el constructo más llamativo del lenguaje. El primer
       fractal que alguien de fuera escriba mal no puede contestar con un volcado.
     - **Costo bajo:** un contador con save/restore en `InvokeStmt` (mismo patrón que
       `g_plotLogContext`) + `max_depth` como sentencia de estado, y `evalError` al
       excederlo nombrando la struct. **No implementar todavía** (decisión de Alejandro,
       2026-07-22).
     - ✅ **YA TIENE SU FIGURA** (2026-07-22): `examples/fractal_tree.mg`, reconstruido del
       Apéndice 1 del artículo de V0 (*Ciencias* 21, 1991). Deja de ser una guarda
       defensiva especulativa: **el listado V0 impreso NO TIENE condición de paro** —no
       podía tenerla, V0 no tenía condicionales— así que el límite de profundidad era lo
       ÚNICO que detenía ese árbol. Era infraestructura de carga. Con eso, el ítem cumple
       la regla del proyecto («no se construye sin una figura que lo pida»).
     - 🔎 **Dato de archivo:** `MAXDEEP` sobrevive como palabra en el léxico de V1
       (`mgpp.l:43`, hoy solo en la rama `v1-legacy`) pero `parseDef` no tiene caso
       para ella → **se ignora**, igual
       que el `$S 1` de las splines cónicas. Murió en la misma transición a EPS de 1991.
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
   - 🔎 **PRIMER DATO REAL (2026-07-27):** Alejandro le dio a un modelo la referencia y una
     imagen, **sin manera de ejecutar `mg`**, y le pidió la figura. Salió `geo/espectro.mg`,
     que compila y se parece. Dos hallazgos que **el autor no podía ver**: (a) para poner una
     punta de flecha escribió un `marker` suelto en la coordenada del extremo, porque
     `marker_end` aparecía **una sola vez** en la referencia y de pasada, dentro del párrafo
     de `marker_at` — nunca como *la* manera de ponerle una flecha a una línea, que es lo que
     hace todo el corpus. **Arreglado el 2026-07-28** en los dos idiomas, con tabla de los
     atributos y el aviso de que la flecha se orienta sola. (b) Aproximó los gradientes con
     franjas planas porque **el lenguaje no tiene gradientes** → `plan_gradientes.md`.
     📌 Un fallo de descubribilidad y un hueco de capacidad, los dos de una sola figura
     intentada por alguien de fuera. Es exactamente lo que esta condición pide recoger.
   - 📣 **Dónde y cómo buscar esa gente: `plan_promocion.md`** (2026-07-28). La
     infraestructura ya no estorba —binarios para los tres sistemas, galería y referencia
     bilingües, canal de issues—, así que lo que falta es gente escribiendo figuras. El plan
     ordena los foros por rendimiento **para esta condición**, no por alcance: el taller con
     estudiantes primero (produce la evidencia directamente), luego TUGboat —MetaPost es el
     pariente más cercano—, las revistas en español, y un Show HN de costo cero. Descarta
     SIGGRAPH y StackOverflow con su razón.
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

## ✅ Cerrado — arcos, anisotropía y `orbita_polar` (abierto y cerrado el 2026-07-27)

Hilo **completo**: los tres bugs de rotación de arcos/elipses arreglados, la figura terminada
y en el corpus, las dos decisiones de semántica tomadas y escritas. `docs/referencia.md`
estuvo congelada mientras duró (decisión de Alejandro: se actualiza al cerrar, no durante) y
recibió lo destapado: recortar un arco por una condición geométrica calculada en el `.mg`,
`asin`/`acos` vía `atan2`, arcos y elipses bajo transformación, `rotate` gira el plano y no la
figura, los mapas de `lib/`, y la regla de la ruta log. Fuentes: `plan_orbita_polar.md` y
`plan_anisotropia.md` (ambos cerrados; el segundo se conserva por «La firma» y «Cómo cazar
más»); bitácora 2026-07-27, (bis), (ter) y sus dos addenda.

- [x] ~~**Ocultar la mitad trasera de la órbita**~~ — HECHO 2026-07-27, sin tocar el
      compilador: la forma cerrada se evalúa en el propio `.mg`. Cada órbita esconde una
      mitad DISTINTA (decisión de modelado, verificada mirando el render), y los cortes
      caen sobre el limbo por construcción, porque el disco del mapa es un `circle(1)`
      escalado por el mismo `R` y con el mismo centro que la ecuación.
- [x] ~~**`orbita_polar` entra al corpus**~~ — HECHO 2026-07-27: `ok=72` (24 ejemplos),
      `docs/img/orbita_polar.svg` y tarjeta 23 de la galería. Con eso `arc(rx,ry)`,
      `marker_at` y `place(rx/ry, at=)` dejan de estar sin pruebas. Encabezado a la
      convención de 2026-07-23 y comentarios limpiados de arqueología.
- [x] ~~**Segundo satélite**~~ — añadido en la órbita de atrás a `at=[270]`, el extremo
      inferior del eje mayor, que cae a 2° de la dirección en que se proyecta Bolivia en
      la vista del mapa (lat 30, lon −55). La tangente ahí es horizontal, así que queda
      acostado, en contrapunto al de arriba.
- [x] ~~**Dos decisiones de semántica** de la familia anisótropa~~ — TOMADAS 2026-07-27,
      cero código. (A) En la ruta **log** de `plot`, **se mapean posiciones, no formas**: los
      tamaños quedan en coordenadas de la página. Es la generalización de lo que esa ruta ya
      hacía con `line_width` y `dot`; se descartaron linealizar el radio (precisión falsa) y
      rechazarlo como error (prohíbe algo que sí significa algo). El costo —`circle(0.5)`
      mide distinto según el eje— lo avisa la **documentación, no el compilador**. (B) El
      «out» de las marcas de eje es perpendicular en el **papel**, corolario de que
      `tick_size` ya es físico; sin cambio de código, porque nada lo alcanza. Escritas en
      `especificacion_mg.md` §13.7, `docs/referencia.md` §11 y `plan_anisotropia.md`.
- [x] ~~**`examples/test_sat.mg`**~~ — ya no está en el árbol; su cobertura vive en
      `rpstest`.
- [ ] *(opcional)* Constantes de **Mortensen** en `arc_bezier` (~5× menos deriva radial) y
      subir la precisión de impresión del SVG. Ambas con contraindicación anotada en
      `plan_orbita_polar.md`; ninguna es defecto.
      - 🔎 **Dato nuevo (2026-08-01), del mismo vecindario:** el nº de segmentos de Bézier de un
        arco es `ceil(|barrido| / 90)`, así que un arco de **exactamente 180°** cae en la
        frontera y **el ruido de coma flotante decide** si le tocan 2 o 3. Se vio al cambiar
        `atan2(...) * 180/pi` por `deg(...)` en `angulo_solido`: el barrido pasó de
        `180.00000000000003` a `180` exacto y un meridiano perdió su tercer segmento —una hebra
        **sub-píxel**, medida: 471 px de diferencia, todos de antialiasing en esa curva—. El
        resultado nuevo es el que la regla pretende; el tercer segmento era un accidente que
        además daba (por casualidad) algo más de precisión. Si algún día se toca `arc_bezier`,
        éste es el argumento de que la frontera merece un epsilon o un redondeo explícito.

## 📌 Importa, pero NO bloquea 1.0

- [ ] 🚧 **COMPUERTA PROPUESTA: paridad de CARA tipográfica entre backends** (2026-08-19).
      La invariante (a) de la Capa 3 cuenta operaciones de texto —`EPS(show) == SVG(<tspan>)
      == PDF(Tj)`— y con eso caza el rótulo **en blanco**; lo que **no** compara es con qué
      CARA se dibujó cada una. Un backend puede sacar el mismo texto, en el mismo sitio, en
      otra fuente, y las diez compuertas siguen verdes.
      - 🔎 **No es hipotético: es el hueco donde vivió el leak de la cara ambiente** (bitácora
        2026-08-19). Un run `$…$` dejaba LM Math puesta para el resto del documento; **SVG
        salía bien —por accidente, no tiene `dev_face`— y EPS/PDF mal**. Estaba **publicado**:
        la leyenda de `quickstart` en itálica y la marca «1» del eje log de `fig6-4`. El golden
        lo bendecía (byte-estable), la Capa 3 no lo veía y `imgfail` tampoco, porque el SVG
        —el único formato que `docs/img` publica— era justo el que estaba bien.
      - **Forma:** misma que `tools/arcparity.py`, que ya resuelve este problema para la
        geometría de arcos: normalizar y comparar backend contra backend. ⚠️ Y por eso hereda
        su mejor propiedad — **no hay nada que bendecir**: no compara contra un golden, así que
        `capture` no puede apagarla.
      - **El trabajo real es la normalización**, no la comparación: los tres nombran la cara de
        forma distinta (`/Times-Italic findfont` en EPS, `font-family`/`font-style`/
        `font-weight` en SVG, el nombre del recurso de fuente en PDF). Hay que mapearlas a un
        vocabulario común y comparar la SECUENCIA, no un conteo.
      - ⚠️ **Verificar que nace ROJA sobre el bug**: `git stash` del arreglo del 2026-08-19 (o
        quitar `restoreAmbientFace`) debe ponerla a fallar en `quickstart` y `fig6-4`. Una
        compuerta que no se prueba contra el fallo que dice cazar no está verificada — es la
        disciplina con la que entraron las diez.
- [ ] 🚧 **COMPUERTA PROPUESTA: los MENSAJES que cita la documentación** (propuesta 2026-07-28,
      medida el 2026-08-19). §14 y §15 de la referencia **citan mensajes del compilador
      textualmente**, y eso es un acoplamiento que ninguna de las diez vigila.
      - 🔎 **Se vio en vivo el 2026-07-28**: se escribió la entrada citando un mensaje y el
        mensaje se mejoró horas después, dejándola rancia al instante. Y la sesión del
        2026-08-19 es el mismo argumento en negativo: se estuvo trabajando **alrededor de**
        `ln: argumento no positivo` toda la noche, y si se hubiera cambiado el texto nada lo
        habría dicho.
      - **Es pariente de `citafail` pero en otro eje, y no se pueden fusionar:** aquélla
        comprueba que lo que la doc cita de un **`.mg` del árbol** siga estando ahí; ésta, que
        lo que la doc cita del **compilador** siga siendo lo que el compilador dice. La primera
        lee archivos, la segunda lee la red de pruebas negativas.
      - **Costo bajo y nace VERDE** (medido hoy): §14/§15 citan **dos** mensajes —`ln:
        argumento no positivo` y `la figura sale EN BLANCO`— y los **dos** ya tienen fixture
        (`test/errors/ln_no_positivo.mg`, `lienzo_en_blanco.mg`). Extraer los fragmentos
        entrecomillados y exigir que cada uno aparezca en algún `% EXPECT*:` de `test/errors/`.
      - 📌 **Entra verde a propósito, y es la mejor forma de entrar:** empieza a vigilar sin
        pedir trabajo, y la primera vez que suene será por una regresión de verdad. Para
        verificarla, cambiar a mano un mensaje citado y comprobar que falla.

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
- [~] **Tipografía math — DOS planes, lo pide `examples/gravitacion_orbita`.**
      **`plan_text_space.md`** es la FUNDACIÓN: **Parte A — medición precisa** de `Text` (hoy un
      run math mide todo con `cmmi_metrics_map` pero se dibuja partido cmmi/Times-Italic; spike
      2026-07-23: arreglo LOCALIZADO en `text_width` de ~15 líneas, churn = **solo `sines.svg`**
      —EPS/PDF centran con operadores de fuente, la imprecisión solo mordía al SVG—, **lista
      para ejecutar**); **Parte B — espaciado automático** estilo TeX (ignorar el espacio del
      fuente e insertar por clase de átomo; diseñada, falta el pase de unario/subíndice/churn).
      **`plan_frac.md`** — composición 2-D de fracciones: SPIKE hecho, **committeado en `main`
      como base WIP** (dormante, `ok=66`; EPS/PDF centran, SVG con bug acotado, inline sin
      hacer). ⚠️ **`\frac` DEPENDE de la Parte A** (usa `TextLine::width()` para dimensionar la
      fracción). Orden: Parte A → (`\frac` y/o Parte B). Detalle en ambos planes.
- [ ] 🪢 **Llaves y delimitadores extensibles** — `plan_llaves.md` (abierto 2026-08-06; lo pidió
      una figura de curso a la que hubo que sustituirle la llave por una cota de doble flecha).
      **(1) ✅ HECHO el 2026-08-06 — el ESCAPE que faltaba.** `\` + carácter no alfabético = ese
      carácter literal (`\{ \} \$ \\ \/`), y con él se cierra el `text("5 m/s")` que salía
      **`5 m`** porque `/s` es cambio de cara — junto con `J/g`, `cal/g`, `1/e`… todos en
      silencio. Se añadió además un aviso para el cambio de cara al final de la cadena, la
      única forma sin falsos positivos (medido: la heurística amplia los tiene, `$\mu/rm$` de
      fig2-5). 4 fixtures nuevos y cobertura de glifos en `examples/texto.mg`.
      **(2)** `{`/`}` **no están en el subset** de LM Math (medido: 237 codepoints, no 186 como
      dice su cabecera). ⚠️ **Ya no bloquea nada**: se midió que los tres backends caen a
      Times-Italic para un byte ausente de `cmmiUnicode()` y que `text_width` lo mide con la
      misma tabla, así que `\{` en math sale y cuadra. Queda como mejora tipográfica —hoy la
      llave es de Times junto a paréntesis de LM Math— y arrastra el que ⚠️ **el paso de subset
      no es reproducible hoy**: `plan_lmmath.md` dice «1 comando» pero no hay script en
      `tools/` ni regla en el Makefile. **(3) ✅ HECHO el 2026-08-06 — la primitiva `brace`.** Se dibuja en el motor
      (`include/brace.h`, geometría única para los tres backends) y se arma EN DISPOSITIVO, así
      que bajo `scale 4 1` sale idéntica en vez de estirarse 4× (medido). Vano en mundo,
      `depth` en pt, costado por el orden de los puntos. Cliente en el corpus:
      `examples/reflectancia_vegetacion.mg`. Queda **solo el delimitador de FÓRMULA** (§4.2 del
      plan), que necesita `penCurve` y que ninguna figura ha pedido. Razonamiento original: la llave alta: **dibujarla**, no tomarla de la
      fuente (el subset no trae las piezas extensibles ni la tabla `MATH` que declara su
      ensamblado, así que la fuente daría el arte y no el comportamiento; y estirar un glifo o
      una struct deforma los ganchos = familia `plan_anisotropia.md`). Orden en §6 del plan.
- [ ] 📏 **Métricas de texto en el lenguaje** — `plan_metricas_texto.md` (abierto 2026-08-08).
      ⚠️ **SIN CLIENTE, y abierto a sabiendas:** ninguna figura del corpus lo pide hoy, y el
      propio plan dice que **no se implemente hasta tenerlo** —una builtin sin figura que la
      use no la compila ninguna de las diez compuertas, igual que una `lib/*.mg` sin ejemplo—.
      El `.mg` no tiene con qué preguntar cuánto mide una cadena: las 17 builtins de
      `include/ast.h:273-352` son aritmética + `len`/`str`/`gray`/`xyz`, y ninguna toca texto.
      **Lo barato:** el motor YA mide bien (`text_width`, `TextLine::width`, `vExtent` — es lo
      que sostiene `\frac`) y ⚠️ **`parse_text` YA corre en tiempo de EVALUACIÓN** (siete sitios
      de `parserv3.cpp` dentro de `exec`), o sea que el camino cadena→objeto medible ya existe
      donde haría falta. No es trabajo tipográfico; es plomería y semántica.
      **Lo caro, y las dos cosas que hay que decidir ANTES de escribir código:**
      **(1) el CUÁNDO** — el estado tipográfico ambiente se resuelve en tiempo de DIBUJO
      (`FN_NOFACE`, `font_size` relativo), así que un `textwidth` en evaluación no sabe la cara;
      medido: `text_width` cae a `serif_metrics_map` por `default:`, correcto por casualidad
      bajo Times y mudo bajo negrita o sans. El plan recomienda la forma **explícita** y
      descarta el estado en la sombra (= segunda máquina de estados que se desincroniza, la
      familia de `plan_anisotropia.md`, sin compuerta que la cace).
      **(2) las UNIDADES** — medido el 2026-08-08: el texto **NO escala** con la colocación de
      una struct (misma struct a `scale=1` y `scale=3` → `font-size="12"` en las dos), y el
      cuerpo de una struct tiene la ventana normalizada al cuadrado unitario, así que ⚠️ **ahí
      la respuesta en unidades de mundo NO ESTÁ DEFINIDA** — y es el caso central, no un rincón.
      Conclusión: `textwidth` devuelve **pt**, y hace falta una conversión pt→mundo explícita.
      Es la misma pared de `plan_llaves.md` §0 un piso más arriba (vano en mundo, `depth` en pt).
      📌 De paso destapó que el idioma de conversión que la referencia §7 ENSEÑA
      (`retiro = 5/72*2.54 / (12/10)`) es **frágil**: supone que el eje x amarra la escala, y la
      real es *meet* `min(W/wdx, H/wdy)` — si amarra el otro eje, la cuenta queda mal y nada
      avisa. Es un argumento para el plan **independiente de medir texto**.
      **Clientes plausibles, en orden de cercanía:** `legend`/`table` (ya calculan anchos por
      dentro, `col_widths`/`sample_width`; exponerlos cierra el círculo) → recuadros y
      anotaciones → diagramas de cajas y flechas. ⚠️ El tercero es el que abrió el plan y el
      más honesto de descartar: le falta mucho más que medir texto (ruteo de aristas,
      colocación de etiquetas), y **medir texto no acerca a MG a ser PlantUML, ni es el
      objetivo**. Origen en el apéndice del plan.
- [ ] 🔤 **La fuente math del SVG no tiene lista de respaldo, y en un visor que no cargue el
      `@font-face` cae a SANS** (hallado 2026-07-31, lo notó Alejandro: «el pi del SVG no se ve
      tan bonito como el del EPS»). El archivo está BIEN: `SVGDisplay` embebe Latin Modern Math
      como TTF base64 en un `@font-face`, sus dos subtablas cmap (fmt 4 y fmt 12) coinciden
      glifo a glifo, y renderizado por un navegador los trazos son **idénticos** a los de EPS y
      PDF (medido con Chrome sobre `$A = \pi r^2 \rho$`). Lo que falla es el visor.
      - **Medido:** Chrome y **Firefox** lo honran, y también en el modo `<img src="…svg">`
        —que es el de un README, y NO es el mismo modo de render que abrir el `.svg` suelto—.
        Los que caen a sans son **`rsvg-convert` e Inkscape**, o sea herramientas de línea de
        comandos, no la ruta de publicación.
      - **Lo que lo empeora, y es lo accionable:** `src/SVGDisplay.cpp` declara la math como
        `fam = "'MGMath'"` **a secas**, mientras el texto normal sí lleva cadena completa
        (`'Times New Roman', Times, 'Liberation Serif', 'Nimbus Roman', serif`). Sin respaldo,
        el visor usa **su** default —típicamente sans— que es el peor sustituto posible para
        matemáticas. Con `'MGMath', serif` el caso degradado se parecería bastante.
      - **Exposición: BAJA, y hay que decirlo porque la primera lectura fue alarmista.**
        16 de los 28 `docs/img/*.svg` publicados dependen de ese `@font-face`, pero la ruta de
        publicación —README y galería de Pages, ambos vistos en un navegador— **sí lo carga**.
        No hay evidencia de que la portada esté enseñando tipografía equivocada. Queda como
        mejora de robustez, no como bug de salida.
      - **Costo:** una línea en `SVGDisplay.cpp`; mueve los goldens SVG con math y obliga a
        regenerar `docs/img` (`test/run.sh images`). Por tocar salida publicada a cambio de un
        caso degradado que solo se ve en herramientas de línea de comandos, **probablemente no
        vale la pena todavía** — se anota para cuando toque otro cambio en `SVGDisplay`.
      - 💡 **Lo que sí deja como método:** para VER un SVG de MetaGráfica hay que usar un
        navegador. `rsvg-convert` e Inkscape mienten sobre la tipografía math, y es fácil
        diagnosticar como bug del compilador lo que es sustitución del visor.
- [x] ~~🐞 **Un arco de BARRIDO CERO tumba el PDF entero**~~ — **CERRADO 2026-08-01.**
      `arc_bezier` (`src/PDFDisplay.cpp`) salía por su `if (sweep == 0.0) return;` **antes** de
      emitir el `MoveTo`: el path quedaba vacío y el `Stroke` de quien llama reventaba. El punto
      ahora se emite y la salida es después, así que el arco degenerado **no traza nada pero sí
      deja la pluma en su inicio** — que es lo que ya hacían los otros dos (PostScript `arc` con
      `start==end` añade el punto; SVG omite el `A` de extremos idénticos y conserva el `M`/`L`).
      Verificado en los tres caminos del constructor de paths: suelto, primer trazo de un
      `compound` (abre con `MoveTo`) y en medio de uno (se une con `LineTo`); los tres backends
      salen con la misma estructura.
      - **La prueba está en `test/errors/arco_barrido_cero.mg`**, y para que pudiera verlo hubo
        que **ampliar la compuerta**: el harness de errores compilaba **solo a SVG**, o sea que
        no podía ver un bug que vive en PDF. Ahora los fixtures `EXPECT_NO_WARN` —y solo ésos:
        los fatales abortan antes de que el backend importe— se compilan a los **TRES**. Es el
        cambio que le da sentido al marcador: si la afirmación es «esto es legítimo y compila
        limpio», tiene que serlo en los tres, y un backend que aborta donde los otros toleran es
        exactamente la clase de fallo que ninguna otra compuerta alcanza. Verificado
        reintroduciendo el bug: `errfail=1` señalando `[pdf]`, con los 78 goldens en verde.
      - **La guarda `if tc > 0.001` de `angulo_solido.mg` se quitó**, que era el punto: SVG salió
        **byte-idéntico** y EPS/PDF difieren solo en los cinco pares `gsave`/`grestore` que
        abría el propio `if`. Cero cambio de dibujo, y la figura ya no pierde el PDF al subir la
        elevación (probada a 42°, donde un paralelo queda entero detrás).

      Lo hallado el 2026-07-31 escribiendo `angulo_solido.mg`, para memoria. Repro mínimo, tres
      líneas: `arc(1, from=30, to=30) { 2 2 }` → **EPS ok, SVG ok, PDF aborta** con
      `Error de libharu 0x1051` (`HPDF_PAGE_INVALID_GMODE`) y exit 1: no se genera archivo.
      - **Es exactamente el sweep 0**, no «sweeps raros»: `from=0 to=360` (círculo completo),
        `from=0 to=0.5` y `from=10 to=370` compilan en los tres. Solo `to == from` falla.
      - **Por qué importa aunque suene exótico:** un arco de barrido cero es la salida
        NATURAL de recortar un arco por visibilidad —`orbita_polar` y `angulo_solido` calculan
        `from`/`to` con trigonometría—, y significa «no se ve nada», que es un resultado
        legítimo, no un error del autor. En `angulo_solido` aparece solo con mover la cámara:
        con elevación 42° el paralelo de −60° queda entero detrás y la figura deja de tener
        PDF. Hoy hay que esquivarlo con un `if`.
      - **Ninguna compuerta lo caza**: ningún ejemplo del corpus produce un arco degenerado,
        y como el PDF no llega a escribirse no hay golden que comparar. La paridad de Capa 3
        tampoco, porque no hay tres salidas que confrontar.
      - **Arreglo probable:** que `PDFDisplay` omita el arco cuando el barrido es 0 (o lo
        emita como un `moveto` sin trazo), igual que hacen EPS y SVG de hecho. Más una prueba
        negativa/positiva en `test/errors/` que fije la conducta elegida en los TRES.
- [x] ~~📥 **`scale` con DOS FACTORES VARIABLES a media línea: la regla existe, funciona, y no
      está documentada**~~ — **DOCUMENTADA 2026-08-01**, §9 en los dos idiomas (`scale sx (sy)`,
      por qué la ambigüedad es real y qué error engañoso da si faltan los paréntesis), con
      `reference.md` re-sellado. El motor no se tocó. Queda sin hacer lo opcional: la prueba
      negativa que fije el mensaje. Lo hallado, para memoria: `scale sx sy`
      seguido de otra sentencia en el mismo renglón **no toma `sy`**: `parserv3.cpp` solo lo
      acepta como segundo factor si el identificador **termina la sentencia**, y la salida
      inequívoca —escrita en el comentario del código— es **`scale sx (sy)`**, porque ninguna
      sentencia empieza con `(`. La desambiguación es CORRECTA y está bien razonada (un
      identificador suelto puede ser un comando o una struct; la alternativa fue el bug de
      descarte silencioso de 2026-07-22). Lo que falta es que se sepa:
      - **`docs/referencia.md` §9 solo enseña `scale 2 1`**, con literales, que es justo el
        caso que NO tiene el problema. Un lector no tiene de dónde deducir la regla.
      - **El diagnóstico apunta al token equivocado.** `{ scale sx sy   shear kk 0 … }` no dice
        nada de `scale` ni de `sy`: dice **«variable no definida: shear»**, porque `sy` queda en
        el flujo y arranca una sentencia nueva que se traga lo que sigue. Quien lo lea buscará
        el error en `shear`, que está bien escrito.
      - **Costo:** un párrafo en §9 (en los dos idiomas, más re-sellar `reference.md`) y, si se
        quiere, una prueba negativa con `% EXPECT:` que fije el mensaje. El motor no se toca.
      - ⚠️ Cuenta para la **condición 4**: es fricción que el autor no ve porque ya sabe
        esquivarla —escribir cada transformación en su renglón—, y para el lector es un error
        que señala una línea correcta. Misma clase que el hallazgo de `marker_end` del
        2026-07-27.
- [x] ~~🐞 **`sine` se TRAGA EN SILENCIO todos sus atributos por-primitiva**~~ — **CERRADO
      2026-08-01, opción (a):** `sine` los HONRA, como todas las demás. `parseSine` ya no arma
      una `SineStmt` (clase borrada) sino un **`PrimStmt` de nombre `"sine"`** cuyo `pathArg` es
      la onda (`PathSine`, que ya existía para el álgebra §9), repartiendo los nombrados —la
      geometría al generador, el resto a `PrimStmt`—. Con eso hereda de una vez el estilo con su
      alcance `gsave`/`grestore`, el `closed=` (que es lo que vuelve rellenable una onda) y la
      validación `isKnownPrimAttr`, que ahora caza también un `half_cicles=` mal escrito.
      **Cero churn** en los 81 goldens: sin atributos, la ruta emite exactamente lo de antes.
      Cobertura: `sines.mg` dibuja ahora una onda rellena, una morada gruesa y una discontinua
      (que el atributo SURTA EFECTO solo lo puede fijar un golden), y
      `test/errors/sine_atributo_desconocido.mg` fija el rechazo del typo. Lo hallado, para
      memoria: `sine(…, fill="red")` compilaba y salía `fill="none"`.
      - **Es exactamente la clase de bug que el proyecto ya cerró DOS veces** («las primitivas
        tragan argumentos nombrados desconocidos en silencio», 2026-07-22): el typo parece puesto
        y no hace nada. `sine` se salvó de aquella pasada porque **tiene su propio parser**
        (`parseSine`), no `PrimStmt`, y por eso no heredó `isKnownPrimAttr`.
      - **Dos salidas, y es DECISIÓN DE SEMÁNTICA, no un parche obvio:** (a) que `sine` honre los
        atributos por-primitiva como todas las demás —lo que un lector espera, y §7.5 de la
        referencia parece prometer—, o (b) que RECHACE los que no entiende. Sospecho que la
        buena es (a) para los de estilo, porque `bezier(&p, fill=)` **sí** los honra y tener dos
        conductas para la misma curva es lo confuso.
      - ⚠️ **Ninguna compuerta lo ve** y por partida doble: la salida es byte-estable (el atributo
        no llega a la salida) y ningún ejemplo del corpus le pone atributos a un `sine`.
- [ ] 📥 **`polygon(&p)` sobre una curva GENERADA da una cometa, sin avisar** (hallado el mismo
      día). `path p = sine(…)` devuelve puntos de **control de Bézier**, y `polygon` los toma como
      vértices: el relleno sale con esquinas rectas en vez del lóbulo. Lo correcto es
      `bezier(&p, fill=)`, que los interpreta como lo que son y cierra el relleno por la cuerda.
      - **La conducta es defendible** —`polygon` significa polígono— pero el trapiezo es fácil:
        §13 de la referencia dice que dentro de un `plane3d` funciona «un `polygon` relleno», que
        es cierto y **engañoso** cuando el trayecto viene de un generador. Cuesta una línea en
        §10 y otra en §13: *para rellenar una curva generada va `bezier(&p, fill=)`*.
      - 🔎 Queda la duda de si `polygon(&p)` debería **avisar** cuando el trayecto trae aritmética
        3k+1 de Bézier. Sería un aviso con falsos positivos (un trayecto de 4 puntos es legítimo
        como polígono), así que probablemente no; se anota para no re-litigarlo.
- [ ] 📐 **`fig2-7b` NO entra al corpus, y la razón está MEDIDA** (2026-08-01, decisión de
      Alejandro). Se pulió la versión afín —proporciones, láminas, cristal, rótulos— y quedó
      mejor; lo que la deja fuera es una sola cosa, y es la que un motor afín no puede dar.
      **No re-diagnosticar: aquí están los números.**
      - La pantalla del original es un **TRAPECIO**, no un paralelogramo. Con las dos aristas
        verticales bien localizadas: lado lejano 415 px contra 602 del cercano → **razón
        0.689** (un paralelogramo daría 1.000), y las aristas superior e inferior a −28.54° y
        +28.80°, o sea que **convergen**.
      - 📌 **El punto de fuga cae en (353, 366) y el cristal ocupa x 300..439, y 320..449**: el
        punto de fuga está DENTRO del cristal, a la altura exacta del centro de la pantalla
        (368). La pantalla está construida como la ve **el punto de dispersión** — la
        perspectiva tiene sentido físico, es el cono de difracción abriéndose.
      - ⚠️ **El original mezcla dos geometrías proyectivas**: el cristal es un paralelepípedo
        afín y la pantalla es perspectiva de un punto. Es el hallazgo de la Fase C un nivel más
        abajo (allí eran dos CÁMARAS distintas, 73.3° contra 35.0°). **Ninguna escena coherente
        puede reproducirlo**, y eso es propiedad de la fuente, no límite de MG.
      - ⚠️ Mi primera medición dio 0.845 y **estaba mal**: identifiqué las esquinas con extremos
        de `x±y` y capturaron otra tinta. El número bueno es 0.689.
      - Lo pulido se conserva en `local/simulate3d/fig2-7b-v3.mg`. Si algún día entra, entra por
        la figura, no por cobertura.
- [x] ~~🐞 **`font "italic"` como SENTENCIA es un no-op MUDO**~~ — **CERRADO 2026-08-19**, y
      resultó que el no-op era el **síntoma**, no el bug. Eran DOS defectos con una causa
      común: **`dspstate.fontFace` hacía dos trabajos a la vez** —la cara AMBIENTE del
      documento (lógica, que salva/restaura `gsave`/`grestore`) y la cara que SELECCIONA cada
      trozo de texto—, y el segundo pisaba al primero.
      - 🐞 **El defecto de fondo, que nadie sabía que existía: un run `$…$` dejaba su cara
        puesta para el RESTO DEL DOCUMENTO.** Repro de cinco líneas, en el binario de
        `ad77078` sin tocar: un `text("$x$")` antes de un `plot` teñía el eje entero —marcas
        `0`/`5`/`10` **y** rótulo— de LM Math itálica. En el corpus lo sufrían la leyenda de
        **`quickstart`** (entradas en itálica; es la figura de portada) y la marca «1» del eje
        log de **`fig6-4`**, más el TAMAÑO de los rótulos de `numbers` en `fill_styles`
        (salían a 9 con el ambiente en 6: saltarse `setFontFace` se salta el refresco de
        tamaño).
      - ⚠️ **Y hacía DISCREPAR a los backends**: SVG salía bien y EPS/PDF mal. SVG se salvaba
        **por accidente** —no tiene `dev_face`, así que leía el `FN_NOFACE` con que
        `setRelFontSize` invalida la cara y su `svgFontAttrs` lo mapea a serif—. La Capa 3 no
        podía verlo: su invariante (a) cuenta `<tspan>`, no compara CARAS.
      - **Arreglo, en dos piezas:** (1) `Display::restoreAmbientFace` + `Text::draw` devuelve
        la ambiente al terminar el trozo —logical-only, sin re-emitir al dispositivo— y
        resuelve `FN_NOFACE` por una cadena explícita *trozo → línea → documento →
        Times-Roman*; (2) recién entonces `TextStmt` hornea `FN_NOFACE` en vez de
        `FN_DEFAULT`, que es lo que hace que `font` alcance a `text()`.
      - 📌 **El `fail=30 c3fail=2 imgfail=9` que este ítem daba por peligroso era TODO el
        leak.** Arreglado (1), el paso (2) sale con **cero churn**. El churn de (1) fueron 7
        goldens en 5 ejemplos, y los cuatro que movieron dibujo se verificaron uno por uno con
        `tools/ver.sh --diff`: los cuatro son **correcciones** (los otros tres, 0 px:
        reordenamiento de `setfont` respecto a un `rmoveto`, que no dependen entre sí).
      - 🔒 **Cobertura: `examples/texto.mg`, tres líneas al final.** Es la ÚNICA red, y por la
        razón de siempre: cerrar (2) no movió un golden, así que perderlo tampoco lo movería.
        Verificado reintroduciendo los dos bugs por separado — (2) da `fail=3` en los tres
        backends; (1) da `fail=21 c3fail=2 imgfail=11`.
      - 💡 **La referencia ya documentaba lo correcto**: §7.5 lista `font` entre las sentencias
        de estado, junto a `color` y `line_width`. No hubo que corregir doc ni re-sellar
        `reference.md` — el arreglo puso al compilador de acuerdo con su propia documentación.
- [ ] 🕳️ **`prisma` y `lamina` (de `lib/pseudo3d.mg`) SIN CLIENTE en el corpus** (anotado
      2026-08-01; **acotado el 2026-08-19** — decía «cuatro piezas» y ya son dos). `cono` y
      `cilindro` los cerró **`seccion_eficaz`** el 2026-08-03; siguen sin compilar en `check`
      `prisma` y `lamina`, committeadas, instaladas por `make install` y documentadas en §12 y
      §13 de la referencia. Es la misma situación que metió a `elevacion_solar` al corpus por
      `lib/fulldisk_map.mg`.
      - **Cobertura parcial:** el bloque ```octave de §12 las invoca, así que `docfail`
        verifica que **parsean y evalúan**. Lo que no verifica es el DIBUJO — para eso hace
        falta un golden, o sea una figura.
      - **Las candidatas siguen siendo dos, y una ya se descartó:** `fig2-7b` **no entra**
        (ver el ítem de arriba, con la medición), y queda `fig18-5`, que usaría `cilindro`
        —ya cubierto, así que hoy entraría por la figura y no por cobertura—. Ninguna es
        urgente. `prisma` acepta orientación (`ex`/`ey`/`ez`) desde el 2026-08-01, y esa
        ampliación tampoco la compila nadie.
      - ⚠️ **Y una limitación que salió al extraer:** una struct de MG **no puede devolver lo que
        calculó**. `irradiancia` necesita los puntos de tangencia para colocar el rótulo de φ, así
        que si llamara a `cono` tendría que recalcularlos — por eso se quedó con la receta inline
        y es el ejemplo al que §13 manda para VER la derivación. La biblioteca sirve a quien solo
        quiere la forma.
- [ ] 🔬 **Experimento propuesto y NO corrido: ¿puede un modelo menor escribir una figura con
      la GALERÍA?** (afinado 2026-08-01; la idea es del 2026-07-28, cuando dos modelos con la
      referencia y con la bitácora eligieron herramientas distintas y salió la condición 4).
      - **Sujeto sugerido: `push_broom`** (`richards_1-7`), porque tras el recuento de cobertura
        es la figura que **no necesita nada nuevo**: plano de la escena + rayos sueltos +
        anotación, las tres en la tabla de decisión de §13 y demostradas en las tres tarjetas
        pseudo-3D, con sus trampas marcadas.
      - **Dar:** `docs/gallery.html`, `docs/referencia.md`, la imagen objetivo, y un `bin/mg`
        que pueda EJECUTAR. **No dar:** bitácora, `docs/plans/`, ni sesiones previas.
      - 🔑 **Lo que mide no es «¿compila?» sino: ¿usa `view3d`/`plane3d`/`xyz()`, o se calcula la
        proyección a mano con senos y cosenos y coloca los puntos ya proyectados?** Ése es el
        análogo exacto del fallo de la condición 4 (elegir `world_window` en vez de `plot`), y es
        lo que diría si la tabla de §13 **funciona** o solo está escrita.
      - ⚠️ La variable que probablemente decide más que el modelo es si puede **ejecutar el
        compilador**: sin eso cae donde cayeron los dos del 2026-07-28. Con eso, el aviso de
        lienzo en blanco y los mensajes de error hacen de guía.
      - Lo que **no** se espera de ningún modelo: elegir cámara y encuadre a nivel de corpus. Eso
        pidió medir e iterar incluso pudiendo renderizar.
- [x] ~~🐞 **Las asignaciones del CUERPO de una struct escriben en el ámbito del
      LLAMADOR**~~ — **ARREGLADO el 2026-08-03**, el mismo día que se encontró. Decisión de
      Alejandro: es un bug, y todo lo asignado dentro de una struct debe ser local.
      - **Cómo:** `Scope` gana una bandera `barrier` que pone `execStructBody` (el único
        punto por el que pasan los cinco sitios que expanden un cuerpo), y la asignación usa
        `findAssignable` en vez de `find`: sube por la cadena igual que una lectura pero se
        detiene al salir del cuerpo de una struct. **Las lecturas la siguen cruzando** —una
        struct ve las variables del archivo, §8— y dentro de un `if`/`for` la asignación
        sigue subiendo, que es lo que hace que `if … { w = 3 }` altere el `w` de fuera.
      - **Cero churn:** `ok=93 fail=0` sin re-bendecir nada, y traductor `ok=14`. Ninguno de
        los 31 ejemplos dependía de la fuga, tal como predijo Alejandro.
      - ⚠️ **Y por eso mismo NINGUNA COMPUERTA PUEDE CAZAR SU REGRESO:** si arreglarlo no
        movió un golden, perderlo tampoco lo movería. La red es `examples/seccion_eficaz.mg`,
        cuyas variables **colisionan a propósito** con las de `lib/pseudo3d.mg`. Verificado
        reintroduciendo el bug: falla en los tres backends. **No renombrar esas variables.**
- [ ] 📥 **`exit` (§18) NO está implementado** (hallado 2026-07-22 repasando `ideas.txt`).
      `exit` da hoy un error de sintaxis («se esperaba una expresión… se encontró un fin de
      línea»): cae al catch-all de sentencia de estado, que exige un argumento. §18 lo
      especifica: «detiene el procesamiento del archivo en ese punto; el resto se ignora».
      **Es una herramienta de autor, y por eso vale aunque nadie la pida desde una figura
      terminada** (Alejandro, 2026-07-22): construyendo una figura por partes, deja **probar
      por etapas** —compilar hasta donde llevas y ver ese estado— sin comentar el resto del
      archivo ni mantener copias. Con el tiempo de compilación de MG, es el depurador que el
      lenguaje no tiene. *(V1: `EXIT`.)*
      - Ojo con el nombre en el mensaje de error: §18 avisa que **no** es condición de paro de
        una recursión (para eso `if`, §8.1); conviene que el error lo diga si alguien lo usa
        dentro de una struct.
- [ ] **Plot Fase 3** (`plan_plot.md`): localizador automático `step`/`decimals`
      (1/2/5·10ᵏ), `format=` con validación, `at=v`, **`title_at=`** (título al extremo
      del eje — hoy obliga a `text()` manuales en fig4-4/fig6-4).
- [ ] **Pseudo-3D — plan REESCRITO el 2026-07-28** (`plan_pseudo3d.md`), y ahora **con
      clientes**: varias figuras del libro de mecánica cuántica y las del curso de
      Percepción Remota (Fig. I.2, II.10, II.11). MG sigue **sin volverse 3D** —sin
      z-buffer, sin superficies ocultas, sin iluminación—, pero la biblioteca actual llegó a
      su techo: da **piezas** con la proyección horneada, y esas figuras piden un **espacio
      compartido**. La malla del suelo de Fig. II.10 son decenas de líneas con la misma
      proyección, y sus rayos unen un punto en el aire con una celda del suelo: un segmento
      que **no pertenece a ninguna pieza**.
      **Es la primera vez que se justifica tocar el motor aquí**, y con el límite que el
      propio plan exigía: MG no tiene funciones de usuario —una struct dibuja, no devuelve—
      así que una función que proyecte `(x,y,z)` no puede vivir en un `.mg`. El diseño son
      dos piezas: `view3d(...)` (cámara, sentencia de estado con alcance) y `xyz(x,y,z)`
      (punto proyectado). **La gramática no se toca**: un bloque de coordenadas ya acepta
      términos que valen un punto, como `point_at`. ⚠️ Añade sintaxis antes de congelar
      (condición 1), así que los nombres se deciden con el cuidado de §13; el plan deja
      cuatro decisiones abiertas.
- [ ] 🎨 **Gradientes de relleno** (`plan_gradientes.md`, abierto 2026-07-28) — MG no los
      tiene y es una ausencia notoria: cualquier sistema de gráficos 2-D los soporta. Cliente
      real: la figura del espectro electromagnético del curso de Percepción Remota, donde la
      banda **es** un gradiente continuo. Forma propuesta: como `hatch`, que ya resolvió una
      vez «un relleno que no es un color plano» (`gradient=[colores]`, `gradient_angle=`).
      ⚠️ **La viabilidad no es simétrica y condiciona el diseño**: SVG nativo, EPS por `shfill`
      de PostScript nivel 3, y **libharu solo implementa el sombreado tipo 4** (malla de
      triángulos) — el lineal sale por ahí, el **radial no**, y por eso se difiere. La cuarta
      invariante de la Capa 3 (rellenos degradados en los tres formatos) va **en el mismo
      commit** que la característica: un gradiente es justo lo que un backend omite en
      silencio.
- [ ] 📥 **Editor web / galería** (`plan_interactivo.md`) — último punto vivo del `TODO` de
      2024, retirado el 2026-07-22 (`ec0c3d1`); los otros cuatro están cerrados.
      **El editor queda CONDICIONADO a la condición 4**, no descartado y no abierto sin
      fecha: su propuesta de valor —bajar la barrera de entrada— **no se puede evaluar sin
      gente de fuera**, así que decidirla hoy en cualquier sentido sería especular. Es la
      regla del proyecto aplicada al empaquetado: hace falta un **usuario** que lo pida.
      Decisión de Alejandro (2026-07-22): conservar la idea justamente para eso.
      - 🔎 **El dato que hay que releer cuando reaparezca:** la barrera medida NO es
        instalar. Ocho tropiezos documentados (cuatro del autor en `tiro_parabolico`, cuatro
        del agente en `figure_02`/`karl.mg`) y **ninguno** fue «no pude compilar el
        binario». Lo que los quita es la referencia (condición 5), no una interfaz.
      - ⚠️ **Si se construye, va DESPUÉS de congelar** (condición 1): el resaltado de
        sintaxis es una segunda implementación de la superficie del lenguaje, y mantenerla
        contra una gramática en movimiento es comprar trabajo duplicado.
      - **La GALERÍA no está condicionada y es barata:** HTML estático con cada
        `examples/*.mg` junto a su `.svg`. No necesita WASM, ni servidor, ni gramática de
        editor; los `.svg` ya se generan con `mg` y ya tienen compuerta (`docs/img`,
        `imgfail`). Ataca el cuello de botella real —ver a qué se parece una figura es lo
        que hace que alguien quiera intentarlo— y sirve a las condiciones 4 y 5.
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

- [x] ~~🐞 **La invocación de struct NO comprueba la aridad de los parámetros
      escalares**~~ — **CERRADO 2026-07-22**, el mismo día que lo destapó la 5ª compuerta
      (un fixture esperaba error y el documento compiló). Era la peor variante de la
      familia, la que produce una figura **plausible**: sobre `struct S(a,b)`, `S(1)`
      dejaba **`b = 0`** y dibujaba a (1,0) en vez de (1,3), con código 0 y sin una
      palabra.
      - **Cuatro casos, el mismo defecto** —un argumento que no liga con nada, o un
        parámetro que no recibe nada—, cerrados juntos en `bindStructParams`, que es el
        punto por el que pasan la invocación directa y la de `fit`: parámetro sin
        argumento y sin default; demasiados posicionales; **argumento nombrado
        desconocido** (un typo en el nombre se ignoraba); y el mismo parámetro dado por
        posición **y** por nombre, donde antes ganaba el posicional callando.
      - `at=`/`rotate=`/`scale=` quedan exentos por ser modificadores de **colocación**
        (§8) y no parámetros — los consume `InvokeStmt`, y dentro de `fit` ya eran error
        antes de llegar aquí.
      - **Cero churn:** los 66 goldens byte-idénticos, o sea que ningún ejemplo del corpus
        se apoyaba en el cero silencioso ni en argumentos que no ligaban.
      - Cubierto por cinco fixtures (`aridad_faltante`, `aridad_sobrante`,
        `arg_nombrado_desconocido`, `arg_duplicado`, `aridad_en_fit`), y verificado
        reintroduciendo el `Value(0)`: la compuerta lo caza por las dos vías y el golden
        se queda en `ok=66 fail=0`, ciego.
- [x] ~~🐞 **`scale` DESCARTA su segundo argumento si es una variable**~~ — **CERRADO
      2026-07-22.** `scale s s` quedaba uniforme con el primer factor y el segundo se caía
      al flujo como si fuera una sentencia. Era el único transform con el defecto
      (`translate`/`shear` usan aridad fija 2); `scale` es el único de aridad VARIABLE, y
      eso chocaba de frente con «varias sentencias por línea».
      - **No se parcheó, se decidió, porque un identificador suelto NO se puede
        desambiguar**: en `scale s  Flecha()` o `scale s  color "red"` lo que sigue es otra
        sentencia. La regla nueva es una **deducción sintáctica**: un identificador seguido
        de FIN DE SENTENCIA (newline/`;`/`}`/EOF) no puede *ser* una sentencia —las de
        estado piden argumento y una invocación pide `(`— luego se escribió como segundo
        factor, y se toma. Así `scale sx sy` funciona sin romper ninguno de los dos
        patrones legítimos.
      - **Una excepción, con nombre:** `outlinefill` es la única sentencia de estado de
        cero argumentos (§4.11), o sea el único identificador que aparece solo y sí es una
        sentencia. Está excluido explícitamente.
      - ⚠️ **`scale s (q)` NO es la salida** —fue mi primer intento—: choca con el footgun
        global de que un identificador seguido de `(` se parsea como llamada (`s(q)`).
      - Cero churn (`ok=66`). Cubierto por un fixture **indirecto**
        (`scale_segundo_factor`): el arreglo es un cambio POSITIVO, así que lo que se pinea
        es *cuál* error sale — si el 2º factor se toma, falla por la variable; si se
        descarta, revienta como sentencia con otro mensaje.
- [x] ~~🐞 **Las primitivas TRAGAN argumentos nombrados desconocidos EN SILENCIO**~~ —
      **CERRADO 2026-07-22.** `marker(rotate=90)`, `dot(tamano=5)`, `polyline(colour="red")`
      compilaban sin hacer nada y sin avisar: `emitPrimStyle` devolvía si reconoció el
      nombre y `PrimStmt` descartaba el retorno. Ahora `PrimStmt::exec` valida contra la
      lista de los 24 atributos reconocidos (estilo + los propios de cada forma) y
      `evalError` ante uno fuera.
      - La lista es **común a todas las primitivas**, no por forma: `circle(closed=true)`
        sigue pasando. Cierra el caso que duele —un nombre que no existe en NINGUNA— sin
        arriesgar falsos positivos; afinar por primitiva es una vuelta posterior, cuando
        exista la referencia que diga qué acepta cada una.
      - 💡 **Y el corpus cazó mi lista incompleta al primer intento:** `fig2-5` usa
        `marker_start_orient=`, que sí existe y está en la spec, pero se pasa a un helper
        (`parseSpec`/`parseOrient`) en vez de por `named.count()`, así que no salía al
        extraer los nombres por grep. **Una lista blanca sacada de los accesos DIRECTOS
        está incompleta por construcción.**
      - Resuelve de paso el `rotate=` de `marker`: en vez de ignorarse, ahora dice que no
        existe y remite implícitamente a `marker_orient=`. Cero churn (`ok=66`), dos
        fixtures (`prim_attr_desconocido`, `marker_rotate`).
- [ ] 🐞 **`place` ignora en silencio los argumentos que no aplican a SU locus** (hallado
      2026-07-22 escribiendo el inventario de loops de la referencia). `place` tiene tres
      loci —**3+ puntos**, **línea de 2 puntos** y **arco** (`r=`)— y cada uno atiende un
      subconjunto distinto de `count`/`gap`/`shift`/`both_sides`/`from`/`to`. Lo que sobra
      no avisa. Dos casos medidos:
      - `gap=` con **3 o más puntos**: byte-idéntico a no ponerlo. ⚠️ **El ejemplo de la
        propia referencia lo traía** (`place(Cuadro, gap=0.5) { 0 0  3 0  3 3 }`), o sea que
        el silencio ya había producido documentación equivocada. Corregido de paso.
      - `count=` sobre el locus **arco**: se ignora; el arco coloca un ejemplar (o dos con
        `both_sides`), no `count`.
      No es el mismo arreglo que el de las primitivas —aquí el nombre **sí existe**, solo que
      no en esa forma—, así que la validación es por locus, no por lista. Antes de congelar.
      💡 Y hay una pregunta de diseño detrás: **`place` son cuatro construcciones bajo un
      nombre** (sembrar en puntos dados, repartir N entre dos, línea guía con algo encima,
      y lo mismo sobre un arco). Las dos primeras son colocación; las dos últimas **dibujan
      el locus**, que es otra cosa. Candidato para el inventario de la condición 1.
- [ ] **Portabilidad a Windows — revisada el 2026-07-22, sin verificar en máquina.** El
      **código** es portable por construcción (cero cabeceras POSIX en fuentes propias,
      ningún `fork`/`popen`/`system`; el único `unistd.h` es andamiaje de flex con su guarda
      `YY_NO_UNISTD_H`; el lexer generado está en git, así que flex no hace falta; libharu
      abre el PDF con `"wb"`). Lo POSIX es el **build**: `SHELL=/bin/sh`, `mkdir -p`,
      `rm -rf`, `install`, `-Wl,--gc-sections` y `CXX = clang++` fijo → compila con
      **MSYS2/MinGW o Git Bash**, no con `cmd.exe` + MSVC. Precedente: en 1999 el usuario
      principal estaba en Windows y se resolvió con MinGW; el port a macOS de 2024 no pidió
      ni un cambio, solo recompilar con GNU.
      - ✅ **Cerrado en esa revisión** (sin tocar el binario ni quitar `-lz`): EPS y SVG
        pasan a `fopen(…, "wb")` —en Unix es lo mismo, en Windows evita que CRLF vuelva la
        salida distinta y produzca **66 FAIL en la red golden que no son regresiones**—, y
        `pandoc` deja de bloquear el `make` por default (si no está, compila el binario y
        avisa).
      - ⏳ **Falta y no se puede hacer desde aquí:** compilarlo de verdad en Windows. Sin
        eso no se anuncia (decisión de Alejandro; para el release se publicará un **.exe**).
        Importa para la **condición 4**: si un invitado llega con Windows, el primer `make`
        que falle se lleva por delante la retroalimentación que se está buscando.
      - Pendiente menor por verificar: si MinGW deja el binario como `bin/mg` sin `.exe`. El
        harness (`test/run.sh`) pide bash + `gs` + `diff` + `mktemp`.
- [x] ~~🐞 **Los GENERADORES y los constructos de `plot` siguen tragando nombres
      desconocidos**~~ — **CERRADO 2026-07-23**, y salió como estaba previsto: se hizo
      *después* de la referencia, y la referencia es la que hizo verificables las listas.
      Once constructos validados (`axis`/`xaxis`/`yaxis`, `numbers`, `ticks`, `grid`,
      `plot`, `rule`, `legend`, `table`, `place`, `repeat`, `fit`) contra una lista propia
      cada uno, con `evalError` que nombra el constructo y el argumento.
      - 🔑 **Las listas se armaron cruzando TRES fuentes, y las tres hicieron falta:**
        (1) `docs/referencia.md` §11, que enumera qué acepta cada constructo; (2) un
        barrido de los accesos a `named` **con cualquier nombre de variable** —`PlotStmt`
        lee `m.find("grid")`, no `named.find`, así que el barrido ingenuo lo perdía—; y
        (3) los nombres que un constructo **INYECTA** en otro: `plot` escribe
        `from`/`to`/`scale`/`field`/`color`/`line_width` en el `AxisStmt` **del usuario**
        y luego lo ejecuta, así que la lista de `axis` es la unión. Con la lista sacada
        solo de lo que `AxisStmt` lee, `xaxis(color="red")` habría dejado de compilar.
        Es la lección de `marker_start_orient=` confirmada por segunda vez.
      - **`fit` necesitó otro arreglo:** es el único que descarta en **parse-time** (su
        `named` no llega a `exec`), así que la compuerta genérica no lo alcanzaba.
      - 💡 **Y la lista de `place` salió MAL en el primer intento, por la misma clase de
        error que la lección anterior pero al revés:** el barrido delimitaba cada `Stmt`
        hasta el *siguiente* `struct`, y entre `PlaceStmt` y el siguiente vive el helper de
        `sine`, así que le atribuyó `amplitude`/`half_cycles`/`phase`/`squared`. Lo destapó
        una prueba de **comportamiento**, no de lectura de código: `place(P, count=6,
        half_cycles=2, amplitude=1)` daba coordenadas **byte-idénticas** a `place(P,
        count=6)`. O sea que se aceptaban y no hacían nada — el bug que la compuerta
        cierra, colado dentro de la lista blanca que lo cierra. **Leer el código dice qué
        nombres aparecen; solo ejecutarlo dice cuáles hacen algo.**
      - ⚠️ **ORDEN en `axis`:** `checkRenamedAxisArgs` va ANTES que la lista genérica. Los
        nombres viejos (`title=`, `labels=`…) tampoco están en la lista, así que con el
        orden inverso el mensaje que dice *a qué se renombraron* —el que hace barata la
        migración— quedaba tapado por un «no existe» genérico. **Lo cazó la 5ª compuerta**,
        no el corpus.
      - **Cero churn** (`ok=66`): ningún ejemplo se apoyaba en un nombre fuera de lista.
        Cinco fixtures nuevos (`gen_axis_desconocido`, `gen_plot_desconocido`,
        `gen_legend_desconocido`, `fit_arg_desconocido`, `place_arg_desconocido`),
        `err_ok=31`→**36**. Verificado quitando el chequeo de `plot`: el documento
        **compila en silencio** y el golden sigue en `ok=66`, ciego.
      - ⏳ **Queda de la misma familia:** afinar por-constructo lo que hoy es una lista
        por-constructo pero generosa, y el ítem de `place` de abajo —que es otro problema:
        ahí el nombre **sí existe**, solo que no en ese locus.
- [ ] **`rotate=` en `marker` (decidido 2026-07-21: se queda como está, revisar antes de
      congelar).** La orientación de un marcador se pide con `marker_orient=<ángulo>`, NO con
      el `rotate=` que giran structs y otras primitivas — `marker(rotate=…)` se ignora (caso
      del silencio de arriba). **Decisión de Alejandro:** los marcadores son distintos de las
      structs y varios ni rotan (cruz/x/círculo son simétricos), así que `rotate=` universal
      no encaja; `marker_orient=` es el nombre correcto. En el radar por si al avanzar hace
      falta un alias, pero **no** ahora.

- [x] ~~**`crosshatch` con ángulo → rejilla recta directa**~~ — **CERRADO 2026-07-23** (pedido
      por Alejandro armando `lib/satellite.mg` para ilustraciones de clase). Se resolvió como
      **`hatch_angle`** —un compañero de `hatch_gap`— y **NO** como el `crosshatch=<ángulo>` que
      proponía este ítem, que era otra sobrecarga: `hatch` dice **qué** trama, `hatch_angle` **a
      qué** ángulo, `hatch_gap` **a qué** paso. Tres perillas ortogonales; idea de Alejandro,
      mejor que la mía.
      - **`hatch_angle=0` endereza el `crosshatch`** a 0°+90°; el default (45°) queda intacto.
        En un estilo simple fija el ángulo de las líneas. El `hatch=<número>` sobrecargado sigue
        siendo el atajo de la familia simple.
      - **Backends:** EPS/PDF **cero cambios** (ya iteraban sobre los ángulos de cada familia).
        Solo SVG: su emisor de familia doble estaba cableado a 45° (dos diagonales √2). Se
        generalizó a **rejilla cuadrada girada** (`patternTransform="rotate(θ)"`) — como el
        `crosshatch` es siempre ortogonal `{θ,θ+90}`, eso lo cubre a CUALQUIER ángulo, así que
        salió también el "nivel 3" que se había estimado difícil. El 45/135 por defecto conserva
        su emisión histórica byte-idéntica (guardia explícita) → **cero churn** (verificado con
        binario pre-cambio: EPS/SVG/PDF idénticos con nombre de archivo fijo).
      - `hatch_angle` entró a `isKnownPrimAttr` (la 5ª compuerta lo valida contra typos).
      - Docs: spec §4.11 y `docs/referencia.md` actualizadas.
      - ✅ **Cobertura CERRADA el mismo día:** `fill_styles.mg` (lámina de referencia de
        tramas) muestra ahora el `crosshatch` oblicuo en dos densidades y su tercer hueco es la
        **rejilla recta** (`hatch_angle=0`, rotulada "cross 0°") → la rama SVG girada ya tiene
        golden que la vigila. Se regeneraron sus artefactos publicados (`docs/img/fill_styles.svg`
        + galería) por `images`; los otros 23 `docs/img` quedaron byte-idénticos, confirmando
        que el motor fue cero-churn. `ok=66`, todas las compuertas en 0.
      - 🔎 **Nota de ergonomía anexa (NO es bug), ya cerrada en la referencia:** el color de las
        líneas de un tramado sale de **`fill=`**, no de `color=` (`fillColorHex()`,
        `SVGDisplay.cpp:107`) — la trama ES el relleno; `color=` contornea el borde. Documentado
        en spec §4.11 y `docs/referencia.md`.

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
      - ✅ **HECHO 2026-07-21 — el bloque `{ }` acepta un punto `[x,y]`.** `PrimStmt::evalPath`
        pasó a máquina de estados escalar-o-punto: un término que evalúa a lista de 2 aporta el
        par entero. Así `marker(...) { point_at(&c,t) }`, `polyline { 0 0  (p)  5 5 }` (mezcla) y
        literales `dot { [3,4] }` componen sin envolver en struct. La paridad se validaba en
        parse-time (`checkCoordPairs`) pero un punto en variable no se distingue de un número ahí,
        así que se movió a eval-time (flag `allowsPoints` que difiere; bloques sin puntos
        —smooth/place/literal— conservan el chequeo estricto con línea:columna). **Único costo:**
        el error de coordenada impar en una primitiva ya no trae línea:columna (sí nombra la
        primitiva). Golden `ok=63` intacto. `path_sample.mg` muestra el marker suelto además de
        la struct. Queda aparte `&path` DENTRO del bloque (splice), que es redundante con
        `polyline(&p)` salvo para mezclar, y `place(Struct) { &path }` de §10.1.
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
      - 🆕 **2026-07-29: lo volvió a cazar la 9ª compuerta, y desde el otro lado.** La forma
        `path suave2 = smooth(&nodos)` estaba **en `docs/referencia.md` §10 como ejemplo**, y el
        ⚠️ de ese párrafo la declaraba «la» manera de partir de un trayecto que ya tienes, «igual
        que en el resto del álgebra», añadiendo «vale lo mismo para `bezier`, `polyline` y las
        demás». Medido: cierto para las que CONSUMEN un trayecto, falso para los generadores
        —`sine(&p, …)` tampoco—, que era justo el párrafo al que estaba pegado. La referencia se
        corrigió en los dos idiomas para describir lo que el compilador hace hoy, incluida la
        limitación en voz alta. **Eso abarata la opción «retirar»**: ya no hay documentación
        pública que prometa la forma-expresión, solo la spec §9.2. La decisión sigue siendo de
        Alejandro. Ver `docs/bitacora.md` 2026-07-29 (quater).
- [ ] 🐞 **`world_window` en unidades de DATOS no avisa, y arruina la figura en silencio**
      (medido 2026-07-29). Escribir `world_window 0.4 2.5 0 100` porque los datos van de 0.4 a
      2.5 y de 0 a 100 produce una figura ilegible —el motor es isométrico, el eje grande aplasta
      al otro—, y con un `plot` encima el `box=` cae fuera de la ventana. **Compila limpio**, así
      que ni el golden ni la Capa 3 ni el bucle «compila y lee el error» lo ven: es exactamente
      el perfil del aviso de lienzo en blanco (2026-07-28). Cerrar = aviso NO fatal cuando el
      `box=` de un `plot` queda mayormente fuera de su ventana, o cuando la proporción de la
      ventana delata unidades de datos. Con `EXPECT_WARN` y su `EXPECT_NO_WARN` (el caso legítimo
      de `fig6-4`, que sí mezcla ventana de página con datos en el plot).
      - 📊 **Evidencia, no conjetura:** es la única falla que sobrevivió a las tres variantes del
        Modelfile del agente (`docs/bitacora.md` 2026-07-29 quinquies). Es la regla más explícita
        de ese SYSTEM, con su ejemplo correcto al lado, y los tres brazos la ignoraron; arreglando
        **solo** eso, el resto de lo que escribieron estaba bien. Y le sirve igual a un humano: es
        el tropiezo nº 1 de §14 de la referencia, que hoy solo se cuenta, no se detecta.
- [ ] 🐞 **`carácter inesperado 'X'` no orienta**: dice dónde y qué, no el arreglo (medido
      2026-07-29). El caso concreto es `#` usado como comentario —lo natural para quien viene de
      Python o shell—, que da `Error léxico en 4:15: carácter inesperado '#'` sin nombrar nunca
      el `%`. Con el mensaje actual, un agente al que se le realimenta el error **no logra
      corregirse ni en dos vueltas**; sus hermanos con más contexto sí (bitácora 2026-07-29
      quinquies). Barato: añadir la pista para los caracteres de comentario de otros lenguajes
      (`#`, `//`, `;`) — «¿comentario? en MG empiezan con `%`». Misma familia que los demás
      «el mensaje no orienta» de esta sección y que la mejora del bloque huérfano del 2026-07-28: el mensaje que nombra la
      corrección vale el doble del que solo localiza el fallo.
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
- [ ] 🐞 **Error: falta el ARCHIVO, no la columna — el ítem estaba INVERTIDO** (medido
      2026-07-22 al sembrar la 5ª compuerta). §19 y este tablero decían «ya reporta
      `archivo:línea`, falta la columna»; lo que sale es `Error de sintaxis en 2:1:` — o
      sea **línea:columna, sin archivo**. Y es más grave que lo que se creía pendiente:
      con `include` (§15) un mensaje sin nombre de archivo **no dice en cuál de los
      archivos está el error**, y es requisito para cualquier editor o IDE. La columna,
      que se daba por faltante, lleva tiempo funcionando (los fixtures
      `bezier_conteo`, `rule_suelto`, `comando_mal_escrito` y `literal_path_impar` la
      verifican con `EXPECT_AT`).

---

## 🪶 Deuda técnica — code-review follow-ups (NO bloqueantes, `plan_plot.md`)

- [ ] **#2** — bajo escala log solo se valida el RANGO (`x=`/`y=`), no los puntos ≤0 del
      contenido → dato ≤0 da coords NaN/inf mudas (`parserv3.cpp` `mapAxis`).
- [ ] **#5** — el detector "línea rellena" de la Capa 3 depende del orden de atributos del
      SVG (`d="…" fill="…"`); si SVGDisplay reordena, el gate deja de cazar en silencio.
- [ ] **#6** — `parsePlot` sobrescribe un 2º `xaxis`/`yaxis` sin avisar.
- [ ] **Los sub/superíndices SIN LLAVES escanean `\comando` por su cuenta** (2026-08-30).
      La rama de `_`/`^` de `text_parser.cpp` es una **segunda implementación, más pobre**,
      del `case '\\'`. La forma CON llaves no la necesita —hace `tspush()` y vuelve al bucle
      principal, así que la atiende el escáner bueno—, y de ahí la consecuencia: **todo lo
      que se le añada al marcado nace roto ahí, y en silencio**. Así nacieron rotos el
      escape E1 (`$x^\{$` se comía los dos caracteres *y* descuadraba el resto de la
      cadena), el espaciado explícito (`$x^\,y$` dibujaba una COMA), las seis funciones
      (`$x^\sin$` avisaba «unknown sin» mientras `$\sin x$` salía bien) y el símbolo
      desconocido, que fue el único que alguien reportó. Los cuatro se **parcharon a mano**
      ese día, uno por uno; la duplicación que los produce sigue en pie, y `\frac`/`\hat`
      ahí solo se **diagnostican**, no funcionan.
      **Arreglo de fondo:** que `^`/`_` sin llaves NO escanee — que consuma *un átomo*
      delegando en el bucle principal, que es lo que hace TeX y lo que la forma con llaves
      ya hace; los tres comandos con grupo saldrían gratis. Toca la máquina de
      `tspush`/`tspop` del bucle, de lo menos cubierto del archivo: su red son el golden de
      `examples/texto.mg` (que fija los GLIFOS) y los tres fixtures
      `test/errors/indice_*.mg` + `simbolo_desconocido_en_indice.mg` (que fijan stderr).
      *Cero presión mientras el parche aguante; anotado para no rediagnosticarlo desde cero.*
- [ ] **El ORDEN de los trabajos en `release.yml` esconde la compuerta que importa** (2026-08-04).
      `smoke-macos` y `smoke-windows` comparan la salida **byte a byte entre plataformas**, que es
      la única red contra la familia «fórmula que debe dar cero y da 1e-15» —ningún golden puede
      verla, porque el golden se genera en UNA plataforma—. Pero dependen de `build`, y `build`
      corre primero las ocho compuertas locales: si una falla, los dos smoke **ni se ejecutan**.
      Eso pasó con el ángulo de la elipse alineada: lo cazó `imgfail` al correr por primera vez en
      macOS, o sea la compuerta equivocada por accidente, y de la diseñada para esto no se supo
      nada. ⚠️ Peor de lo que parece: si el fallo local hubiera sido en Linux, `build` habría
      muerto ahí y **la divergencia habría seguido invisible**. Arreglo probable: que los smoke
      dependan solo de que el binario ENLACE, no de que pasen las compuertas locales. *Cero
      presión hasta el próximo release; anotado para no redescubrirlo.*
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

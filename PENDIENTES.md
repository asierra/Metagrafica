# PENDIENTES — MetaGráfica V3

> **Qué es esto.** Tablero de continuación: el estado vivo de lo que falta, para
> retomar en cualquier máquina. **No duplica detalle** — cada ítem apunta a su fuente
> autoritativa (`especificacion_mg.md` §19/§22, los `plan_*.md`, `CLAUDE.md`). Si un
> ítem y su fuente se contradicen, gana la fuente; actualiza aquí al cerrarlo.
>
> Reemplaza a los antiguos `PENDIENTES.md` (auditoría de backend V1, retirada en
> `4b9b4d4`) y `ROADMAP.md`, ya superados. Act. **2026-07-22**.
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
> Build/test: `make` + `bash test/run.sh check` → **ok=66 … errfail=0** (5 compuertas,
> la 5ª son 30 pruebas NEGATIVAS en `test/errors/`).
> Traductor: `bash test/run_translator.sh check` → **ok=14** (`tools/mg1to2.py`).

---

## 🎯 Las cinco condiciones para el 1.0 (§22.7 — lista canónica, decidida con Alejandro)

Es lo único que bloquea salir de beta. **No hay más.** ⚠️ **Es una SECUENCIA, no una lista
de pendientes:** el orden real de ejecución es **2 → 5 (borrador) → 4 → 1 → 5 (final)**, y
(3) ya está. Los números son estables (se citan desde otros documentos); lo que no sigue el
orden de la lista es la ejecución.

1. **Congelar la gramática.** Declarar estable lo que hay; no añadir sintaxis nueva.
   Implica cerrar (4) y el borrador de (5) primero. Es una decisión, no código.
   - [ ] 📥 **Inventario de los loops que ya existen** (de `ideas.txt`, punto nunca cerrado:
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
       (`src/mgpp.l:43`) pero `parseDef` no tiene caso para ella → **se ignora**, igual
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
- [ ] **Pseudo-3D** (`plan_pseudo3d.md`): refinamientos acotados (`hatch` como parámetro
      de `plano`, refactor de `fig10-2v3`). MG **no** se vuelve 3D — simula volumen con
      shear por composición 2D.
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

- [ ] 🐞 **Las primitivas TRAGAN argumentos nombrados desconocidos EN SILENCIO** (hallado
      2026-07-21). `marker(rotate=90)`, `dot(tamano=5)`, `polyline(colour="red")` (typo
      clásico), `marker(disparate=5)` — todos COMPILAN sin avisar y no hacen nada. Un typo en
      un atributo simplemente se ignora. Es la misma clase de silencio que se cazó en las
      sentencias de estado (`colour 0.5` → error, `emitStyleAttr` devuelve `bool`), pero el
      atributo POR-PRIMITIVA **descarta ese retorno** (`emitPrimStyle`, `parserv3.cpp:475`).
      Cerrar = juntar los nombres reconocidos (estilo + los propios de cada primitiva:
      `shape`/`size`/`width`/`marker_orient`/`closed`/`from`/`to`/`hatch_gap`…) y `evalError`
      ante uno fuera de lista. **Antes de congelar** (un typo silencioso es peor cuanto más
      madura la gramática). Ver también el `rotate=` de abajo, que es un caso de esto.
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
- [ ] 🐞 **Los GENERADORES y los constructos de `plot` siguen tragando nombres
      desconocidos** (medido 2026-07-22 al cerrar el de `text`). `axis(disparate=1)`,
      `numbers(disparate=1)` y `grid(disparate=1)` compilan y no dicen nada; es
      previsible que `plot`/`legend`/`table`/`rule`/`place`/`repeat`/`fit` estén igual.
      Es el mismo arreglo de una línea que recibieron las primitivas y `text`, pero
      **la superficie es mucho mayor y el riesgo real no es el código sino la LISTA**:
      cerrando el de primitivas, el corpus cazó al primer intento que la mía estaba
      incompleta (`marker_start_orient=`, que existe y está en la spec, se pasa a un
      helper y no salía por grep). Una lista blanca sacada de los accesos DIRECTOS está
      incompleta por construcción, y aquí cada generador tiene ~15 nombres. Conviene
      hacerlo **cuando se escriba la referencia** (condición 5), que es justo el
      ejercicio de enumerar qué acepta cada constructo — la lista sale de ahí verificada
      en vez de por grep.
- [ ] **`rotate=` en `marker` (decidido 2026-07-21: se queda como está, revisar antes de
      congelar).** La orientación de un marcador se pide con `marker_orient=<ángulo>`, NO con
      el `rotate=` que giran structs y otras primitivas — `marker(rotate=…)` se ignora (caso
      del silencio de arriba). **Decisión de Alejandro:** los marcadores son distintos de las
      structs y varios ni rotan (cruz/x/círculo son simétricos), así que `rotate=` universal
      no encaja; `marker_orient=` es el nombre correcto. En el radar por si al avanzar hace
      falta un alias, pero **no** ahora.

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

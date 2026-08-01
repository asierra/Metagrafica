# Plan para simulación pseudo-3D (2.5D) en MG V3

> **Actualizado el 2026-07-31.** La revisión del 2026-07-28 derivaba todo su diseño de **tres
> clientes abstractos** del libro de Percepción Remota (Fig. I.2, II.11, II.10) que nadie había
> visto, y de ahí sacaba una sola necesidad —malla de suelo + rayos—, una sola adición al motor
> y un orden de fases que la seguía. Ahora hay **nueve figuras concretas** (`local/simulate3d/meta/`)
> y **no piden todas lo mismo**: dentro de una misma figura la pantalla y el prisma son
> estrategias distintas, dos figuras de libros distintos son la misma, y en varias más de la
> mitad del dibujo es anotación 2-D que ya se sabe hacer. Lo que cambia: entra la **§2 nueva**
> (vocabulario de estrategias + tabla figura×estrategia), se cierran las cuatro decisiones que
> §7 dejaba abiertas, y **son dos las adiciones al motor, no una**.
>
> De la revisión anterior sigue en pie todo lo demás: las fases 0-2 del plan de 2026-07-12
> están hechas (`lib/pseudo3d.mg`, `fig10-2v3`, `fig2-7b-v3` compilan hoy) y son el punto de
> partida, no trabajo perdido.

Objetivo: dar soporte fiel a la ilustración científica 2.5D —planos que receden, prismas,
pantallas inclinadas, mallas de terreno, esferas reticuladas— manteniendo la filosofía 2D del
lenguaje: espacio isométrico por construcción (§3.1), ortogonalidad forma/posición, y la regla
sagrada `Subpath ::= (Coord Coord)+` **intacta**.

🚧 **Y el límite del objetivo, dicho de una vez para no volver a discutirlo:** todo esto es
**simulación** de 3-D hecha en 2-D. Figuras que se *ven* tridimensionales porque su geometría
se calculó en el espacio, no una escena tridimensional. Si algún día hace falta 3-D de verdad
—superficies ocultas, iluminación, cámara libre, mallas densas— eso es **Blender** u otra
herramienta, y **MG no la va a sustituir**. El valor de MG aquí es que la figura sale de un
`.mg` de treinta líneas que se lee, se versiona y se recompila; no que compita con un motor de
render.

---

## 1. Qué falla en el diseño actual

`lib/pseudo3d.mg` ofrece `plano(w, h, k)` y `prisma(w, h, d, a, f)`: structs que **hornean la
proyección dentro de cada forma**. Funciona —reproduce `fig10-2` y `fig2-7b`— y tiene tres
consecuencias que solo se ven cuando la figura crece:

1. **Cada pieza tiene su propia idea de la proyección.** Nada garantiza que la pantalla, el
   cristal y las láminas de `fig2-7b` estén en el mismo mundo: coinciden porque se colocaron a
   mano hasta que se vieron bien.
2. **Las posiciones se calculan de cabeza.** Todo se ubica con `at=` en 2-D — el mismo «medir a
   ojo» que el proyecto combate en el resto del corpus.
3. **Mover la cámara no es una operación.** Para ver el montaje desde más arriba hay que
   recalibrar cada pieza. La figura no sabe que es 3-D: sabe que *parece* 3-D.

Ninguna es un defecto del código. Son el techo de haber resuelto **formas** cuando el problema
es un **espacio**.

---

## 2. Las figuras y lo que pide cada una

La regla del proyecto es no construir sin una figura que lo pida. Estas son las nueve, en
`local/simulate3d/meta/`:

| archivo | figura |
|---|---|
| `fig10-2.png` | planos paralelos que receden, con cotas (IMQ) |
| `fig2-7b.png` | difracción de electrones: láminas, cristal, pantalla, anillo (IMQ) |
| `fig18-5.png` | sección eficaz: triada x/y/z, blanco, detector cilíndrico, dΩ (IMQ) |
| `lira_II-1_senal_u_onda_electromagnetica.png` | E y B senoidales en planos perpendiculares |
| `lira_II-4_proyeccion_del_angulo_solido.png` | esfera de meridianos y paralelos |
| `lira_II-7_esquema_de_la_ley_de_la_irradiancia.png` | cono de ángulo sólido sobre una superficie |
| `richards_1-6_image_formation_by_mechanical_line_scann.png` | escáner de línea: suelo, rayos, rejillas |
| `richards_1-7_image_formation_by_push_broom_scanning.png` | push-broom: suelo, abanico de rayos |
| `waves.png` | senoide rellena sobre una malla de suelo |

*(Las Fig. I.2 / II.11 / II.10 del libro de PR que citaba la revisión anterior siguen siendo
clientes plausibles, pero **no hay imagen a la vista** y por tanto no gobiernan el diseño: lo
que pedían —malla de suelo y rayos— lo piden ya `richards_1-6` y `1-7`, que sí se pueden
verificar.)*

### 2.1 Seis estrategias, porque no es una sola cosa

| | estrategia | mecanismo | ¿motor nuevo? |
|---|---|---|---|
| **A** | **Plano de la escena** | `plane3d(at=, u=, v=)`: un dibujo 2-D corriente vive en un plano del espacio. Un `circle` sobre él sale como la **elipse exacta** (§3.2). | `plane3d` |
| **B** | **Puntos y rayos sueltos** | `xyz(x,y,z)`: segmentos y mallas cuyos extremos **no pertenecen a ninguna pieza** — un rayo del sensor a una celda del suelo. | `xyz()` |
| **C** | **Curva y relleno en un plano** | A + los generadores de siempre: dentro de un `plane3d`, `sine`/`smooth`/`bezier`/`polygon` funcionan sin enterarse (§4.4). | ninguno |
| **D** | **Sólido de caras planas** | N polígonos de A, pintados **de atrás a adelante**. Es lo que hace `prisma` hoy. | ninguno (`lib/`) |
| **E** | **Silueta de revolución** | cono/cilindro = dos círculos de A + sus **dos tangentes comunes**. | geometría nueva → **diferida, §5 Fase F** |
| **F** | **Anotación en el papel** | arcos de ángulo, cotas, flechas, rótulos, contornos a mano alzada. | **ya existe** |

📌 **F merece su renglón aunque exista, y por dos razones.** Primera: es **más de la mitad** de
`richards_1-6`, `richards_1-7` y `fig18-5` —el sensor, las rejillas de salida, las flechas
curvas, los rótulos—, así que reconocerla es lo que mantiene chico el alcance de todo lo demás.
Segunda: **no se sube a la escena**. Un arco que marca φ se dibuja en el papel, porque el
ángulo entre dos direcciones proyectadas **no** es la proyección del ángulo 3-D (familia
`plan_anisotropia.md`); en las figuras publicadas ese arco es decorativo, y pretender
calcularlo sería entrar en la familia por la puerta grande.

### 2.2 La tabla

| figura | A | B | C | D | E | F | lo que la hace particular |
|---|:-:|:-:|:-:|:-:|:-:|:-:|---|
| `fig10-2` | ● | ● | | | | ● | Planos **xy** a distintas z. Las cotas y las líneas de enlace punteadas van **sin proyectar**. Ya compila (`fig10-2v3.mg`, con `shear`). |
| `fig2-7b` | ● | ● | | ● | | ● | **Cuatro estrategias en una figura**, y por eso es el mejor caso de integración. Pantalla y láminas = A; **anillo = círculo sobre la pantalla** → elipse exacta (hoy `ellipse(0.6, 1.3)`, medida contra el `.png`); **cristal = D**; haz y rayo difractado = B, y **P tiene que caer sobre el anillo**. |
| `lira_II-1` (onda EM) | ● | | ● | | | ● | E en el plano **xy**, B en el **xz**, propagación en x. Los peines punteados y sus flechas **viven cada uno en el plano de su onda** → son 2-D, no necesitan `xyz()`. |
| `waves` | ● | | ● | | | ● | Misma estrategia que `lira_II-1`, más dos cosas: **relleno partido en los cruces por cero** (dos colores, un `polygon` por medio ciclo) y **orden de pintado** — las medias ondas tapan la malla y se tapan entre sí. |
| `lira_II-4` (ángulo sólido) | ● | ● | | | | ● | **A en bruto**: ~14 círculos de la escena = 14 elipses exactas, en dos `for`. Meridiano de longitud λ = plano `u=[cos λ,0,sin λ] v=[0,1,0]`; paralelo de latitud β = plano `at=[0,R·sin β,0] u=[1,0,0] v=[0,0,1]` con radio `R·cos β`. Sin oclusión en el original. Radios ρ y r punteados = B. |
| `richards_1-6` (line scan) | ● | ● | | | | ● | Suelo = plano **xz**; celda resaltada = rectángulo del suelo; rayos IFOV/FOV = B. **Sensor, rejillas, flechas curvas y rótulos son F.** |
| `richards_1-7` (push broom) | ● | ● | | | | ● | **Misma estrategia que `1-6`** + abanico de rayos en un `for`. Una sola figura de prueba cubre las dos. |
| `lira_II-7` (irradiancia) | ● | ● | | | **●** | ● | dA y dΩ = discos de la escena; ΔP = parche plano; **la superficie irregular es un `smooth` del papel (F), no 3-D**. Falta **E** para la silueta del cono. |
| `fig18-5` (sección eficaz) | ● | ● | | | **●** | ● | Triada de ejes = tres segmentos B + rótulos (la forma **mínima** del `box_axis` diferido); blanco y dΩ = A; **detector cilíndrico = E**. ⚠️ Los rótulos `x`/`y`/`z` **son contenido de la figura**, no el marco de la escena: con la convención de §4.1 no coinciden, y confundirlos costaría una figura mal armada. |

### 2.3 Lo que la tabla hace visible

- **`lira_II-1` ≡ `waves`** → C. Idéntica estrategia; `waves` solo añade relleno y orden de
  pintado.
- **`richards_1-6` ≡ `richards_1-7`** → B. Una figura de prueba cubre las dos.
- **`lira_II-7` ≡ `fig18-5`** → E, y por eso **se difieren juntas**: comparten la única pieza
  que pide geometría nueva.
- **Dentro de `fig2-7b`: pantalla = A, cristal = D.** Son estrategias distintas dentro de un
  mismo dibujo — que es precisamente lo que el plan anterior no tenía dónde registrar.
- **A y F aparecen en las nueve.** Eso, y no la malla de suelo, es lo que fija el orden de las
  fases (§5).

⚠️ **Antes de reintentar `fig18-5`:** su original V1, `local/simulate3d/fig19-5.mg`, abre con
`%% OJO Las matrices de transformacion trabajan mal`. Esa figura ya había topado con esto en
1998; conviene mirar qué le fallaba antes de rehacerla, no después.

---

## 3. El límite concreto que justifica tocar el motor

El plan de 2026-07-12 puso la barra, y conviene citarla porque se cumple exactamente:

> *«Solo se consideraría subir algo a builtin si un límite concreto del lenguaje lo impide, y se
> anotaría aquí antes de hacerlo.»*

### 3.1 El límite: MG no tiene funciones de usuario

Tiene `struct`, y una struct **dibuja**, no **devuelve**. Así que una función que proyecte
`(x,y,z)` a un punto **no puede escribirse en un `.mg`**. Es la pieza que obliga a bajar al
compilador, y es pequeña: `xyz()`.

✅ **Y lo que NO hace falta cambiar, verificado el 2026-07-28:** un bloque de coordenadas **ya
acepta términos que valen un punto** (una lista de dos), que es lo que hace `point_at(&p, t)` y
lo que ejercita `examples/path_sample.mg`. Por tanto la gramática **no se toca** y la regla
sagrada `Subpath ::= (Coord Coord)+` queda intacta.

### 3.2 Y una segunda pieza, que resultó ser la más barata de las dos

**Una elipse del motor ya *es* un círculo proyectado.** `Matrix::ellipse_frame` (`matrix.cpp`)
entrega centro + **semidiámetros conjugados** `u, v`, con `P(t) = C + u·cos t + v·sin t`. Esa
es, literalmente, la forma cerrada de la proyección ortográfica de un círculo del espacio: `u`
y `v` son las proyecciones de la base del plano, escaladas por el radio. Y los tres backends
**ya consumen esa forma** — es lo que quedó de la reconstrucción de arcos y elipses del
2026-07-27.

📌 **Consecuencia:** una sentencia `plane3d` que empuje la matriz del plano hace que un
`circle(r)` sobre un plano de la escena salga como **la elipse exacta**, con **cero cambios en
los backends**. Se implementa como un `Stmt` que reusa `OPMPUSH`/`Transform` igual que ya hace
la sentencia de transformación (`parserv3.cpp`), más `g_flags.using_ellipse = true` como hace
`shear`. No hace falta una op nueva de `Matrix` ni tocar `matrix.h`.

Es la pieza con **más clientes de todo el plan (9 de 9 figuras)** y la más barata. Sin ella,
cada elipse se sigue midiendo a ojo, como hoy en `fig2-7b-v3.mg`. Con ella, el anillo de
difracción, los catorce círculos de `lira_II-4`, los discos dA y dΩ y el parche ΔP salen de la
geometría y no de la regla.

**Corolario:** bajo `plane3d`, el `from`/`to` de un `arc` sigue siendo **ángulo del plano**, no
de la página. O sea que recortar un círculo de la escena —dibujar solo la mitad que se ve— es
el mismo truco cerrado que ya usa `orbita_polar`, sin medir nada sobre el dibujo.

---

## 4. Diseño

Dos sentencias y una función; todo lo demás sigue igual.

### 4.1 Convención de ejes: `z` es la PROFUNDIDAD

**x a la derecha, y arriba, z hacia el observador.** El plano del papel es **xy**.

Se eligió así (2026-07-31) porque hace que **`view3d(azimuth=0, elevation=0)` sea la
identidad**: la vista frontal de `fig10-2` y `fig2-7b` —donde la cara que importa conserva su
forma real— es el caso por default, sin caso especial. El suelo de `richards_1-6/1-7` y de
`waves` es entonces el plano **xz**, y una altura es `y`.

⚠️ Es la primera pregunta que hará quien lo use, así que va **en `docs/referencia.md`** cuando
se documente (§5, Fase D). Y ojo con `fig18-5`: los rótulos `x`/`y`/`z` de esa figura son de la
física que ilustra, **no** de este marco.

### 4.2 `view3d` — la cámara, como sentencia de estado con alcance

Igual que `translate`/`rotate`: vale desde donde aparece hasta el fin del bloque. **No** como
atributo por-primitiva (no duplicar el concepto, misma decisión que tomó el plan anterior).

<!-- ilustrativo: sintaxis que aún NO existe -->
```octave
view3d(azimuth=35, elevation=25)                     % axonométrica (ortográfica)
view3d(type="oblique", angle=45, foreshorten=0.5)    % caballera/gabinete
```

**Dos proyecciones, porque el corpus tiene las dos.** Con la convención de §4.1:

- **Axonométrica ortográfica** — la de las figuras de PR y de `lira_II-4`. Acimut θ (giro sobre
  la vertical) y elevación φ (cámara levantada sobre el plano horizontal):

      X = x·cos θ + z·sin θ
      Y = y·cos φ + (x·sin θ − z·cos θ)·sin φ

  Comprobaciones que tienen que salir: **θ=φ=0 ⇒ identidad**; θ=0, φ=90° ⇒ vista en planta
  (`X=x`, `Y=−z`); θ=90°, φ=0 ⇒ vista desde +x (`X=z`, `Y=y`).

- **Oblicua (caballera/gabinete)** — la de `fig10-2` y `fig2-7b`: la cara frontal conserva su
  forma real y la profundidad recede a un ángulo `angle` con factor `foreshorten` (1 =
  caballera, 0.5 = gabinete). Lo lejano es `z < 0` y recede hacia `angle`:

      X = x − z·f·cos(angle)
      Y = y − z·f·sin(angle)

  El plano xy **siempre** conserva su forma, sea cual sea `angle`. Es lo que `lib/pseudo3d.mg`
  hace hoy con `shear`.

**Una sola sentencia con `type=`, no dos** (decisión 2026-07-31): la figura declara *una*
cámara y no debería poder tener dos semánticas activas.

### 4.3 `plane3d` — dibujar en un plano de la escena

Sentencia de estado con alcance, como `translate`. Empuja la matriz que lleva las coordenadas
locales del plano a la página, con la `view3d` vigente:

<!-- ilustrativo: sintaxis que aún NO existe -->
```octave
plane3d(at=[x,y,z], u=[ux,uy,uz], v=[vx,vy,vz])
```

`at` es el origen local; `u` y `v` son los dos vectores del plano y **llevan la escala**: el
local `(0,0)` cae en `at`, el `(1,0)` en `at+u` y el `(0,1)` en `at+v`. Defaults: `at=[0,0,0]`,
`u=[1,0,0]`, `v=[0,1,0]` — o sea, el plano del papel.

```octave
view3d(azimuth=35, elevation=25)

% la pantalla ES un plano del espacio, y el anillo un círculo SOBRE ella
{ plane3d(at=[8,0,0], u=[0,0,1], v=[0,1,0])
  rectangle { 0 0  1.4 4.2 }
  circle(0.6) { 0.7 2.1 }            % → la elipse EXACTA, sin medir
}

% los meridianos de una esfera: catorce elipses en dos renglones
for k = 0 to 6 {
    lam = k * pi / 7
    { plane3d(u=[cos(lam), 0, sin(lam)], v=[0, 1, 0])   circle(R) { 0 0 } }
}
```

### 4.4 Lo que sale gratis, y el footgun que trae

📌 **Dentro de un `plane3d`, el dibujo 2-D corriente sigue funcionando.** `sine()`, `smooth`,
`bezier`, `polygon` relleno, `place` de una struct: todo se proyecta solo, porque lo único que
cambió es la matriz vigente. Por eso **`lira_II-1` y `waves` no necesitan muestrear nada a
mano y no necesitan `xyz()`**: cada onda es un `sine()` corriente dentro de su plano, y cada
medio ciclo relleno es un `polygon` cerrado sobre el eje.

⚠️ **Y de ahí sale el footgun.** `xyz()` devuelve un punto **ya proyectado**, en coordenadas del
documento. Dentro de un bloque `plane3d` se transformaría **dos veces**. La regla, que va a la
referencia junto con la sintaxis:

> `xyz()` se usa **fuera** de todo `plane3d`. Dentro de un `plane3d` se dibuja en coordenadas
> locales del plano.

### 4.5 `xyz(x, y, z)` — un punto del espacio de la escena

Función del evaluador que devuelve el punto 2-D proyectado con la `view3d` vigente. Es para lo
que **no pertenece a ningún plano**: los rayos.

```octave
view3d(azimuth=35, elevation=25)

% un rayo: óptica → celda del suelo. No pertenece a ninguna «pieza».
polyline { xyz(0, h, 0)   xyz(3*d, 0, 5*d) }

% y todo lo demás sigue funcionando sin enterarse
text("CIV") { xyz(3*d, 0, 5*d) }
place(Detector, at=(xyz(0, h, 0)))
```

📌 **Lo que se gana no es sintaxis, es que la cámara pasa a ser un PARÁMETRO.** Cambias
`elevation` y la malla, los rayos y las piezas se mueven juntos. Es la misma propiedad que ganó
`orbita_polar` el 2026-07-27 al derivar la órbita de kilómetros: el número que gobierna la
figura, escrito una sola vez.

### 4.6 Lo que este plan NO incluye

- **Sin z-buffer ni superficies ocultas.** Orden de pintado = orden de escritura, que es como ya
  se dibujan estas figuras. Dibujar lo lejano primero.
- **Sin iluminación ni sombreado automático.** El sombreado de caras se sigue eligiendo a mano
  (`prisma` ya lo hace con tres grises).
- **Sin perspectiva**, de entrada. Es la misma función con un divisor más; se añade si una figura
  la pide, no antes.
- **Sin recorte por volumen.** La escena se proyecta a las coordenadas del documento y ahí acaba;
  no hay ventana 3-D.
- **Sin 3-D de verdad.** Ver el aviso 🚧 del encabezado: eso es Blender.

---

## 5. Fases

El orden lo fija la tabla de §2.2, no la malla de suelo: **A y F aparecen en las nueve
figuras**, y A es además la más barata (§3.2).

### Fase A — `view3d` + `plane3d`
Las dos que todo necesita. **Criterio de aceptación: `lira_II-4`** — los ~14 círculos generados
en dos `for`, todos elipses exactas, y **cambiar `elevation` los mueve juntos**. Es el caso
puro: cero `xyz()`, cero números medidos.

### Fase B — `xyz()`
Los rayos y los segmentos que cruzan planos. **Criterio:** el abanico de rayos de
`richards_1-7`, del sensor a las celdas del suelo. Y, cerrando A+B, el rayo difractado de
`fig2-7b` aterrizando **sobre** el anillo por construcción, no por ajuste.

### ✅ Fase C — reescribir `lib/pseudo3d.mg` sobre el espacio — HECHA 2026-07-31
`plano` **se retiró**: era `plane3d` con menos generalidad. `prisma` se reescribió como
estrategia D —sus tres caras son tres planos de la escena— y toma `pos=[x,y,z]` en vez de un
`at=` 2-D (`at=` sigue siendo palabra de colocación de la struct, §8). Entró además `lamina`,
la placa de una sola cara con trama, que es lo que pedían las láminas policristalinas.

⚠️ **Corrección al criterio que decía este plan:** `fig10-2v3` **no usa la biblioteca** —tiene
su propio `shear` y sus propias structs—, así que el único cliente y único oráculo es
`fig2-7b-v3`. Y ahí **0 px era imposible por construcción**, no por falta de cuidado: las
piezas de esa figura no compartían cámara, y se puede medir —la pantalla (`plano k=0.3`)
recedía a **73.3°** y el cristal (`prisma a=35`) a **35.0°**, treinta y ocho grados de
diferencia—. Meterlas en una escena común OBLIGA a cambiar el dibujo; ese cambio *es* el
arreglo. (Contrástese con `angulo_solido`, que sí dio 0 px porque ya era escena-derivada.)

📌 **Y el puerto salió MÁS FIEL al original publicado.** Una pantalla perpendicular al haz es
un plano y-z, y en las dos proyecciones el eje y va vertical en la página: la pantalla tiene
lados **verticales** y arriba/abajo inclinados, que es lo que muestra `meta/fig2-7b.png` y lo
contrario de lo que producía `plano`. La pieza llevaba años girada 90° en carácter respecto de
su fuente, y nadie lo había visto porque no había con qué compararla.

**Criterio cumplido:** la cámara de la figura se **despejó del `.png`** (borde de la pantalla →
`angle=44°`; razón del anillo → `foreshorten=0.375`, dos medidas independientes que caen sobre
la misma cámara), y la figura compila con `gs` y paridad geométrica bajo **cuatro cámaras
distintas**, dos de ellas axonométricas — que la versión anterior no podía ni expresar.

### ✅ Fase D — documentar en `docs/referencia.md` — HECHA 2026-08-01
**§13 «Escenas pseudo-3D»**, nueva, en los **dos idiomas** (`reference.md` traducido y
re-sellado): convención de ejes, las dos proyecciones con sus fórmulas y sus casos de
comprobación, `plane3d`, `xyz()`, el footgun del doble transform y las piezas de
`lib/pseudo3d.mg`. Entró después de §12 —renumerando 13→14, 14→15, 15→16 y sus anclas—, más
`view3d`/`plane3d`/`xyz` en la referencia rápida y el matiz al «No hace 3D» de §1, que era
justo lo que un lector de fuera leería primero.

📌 **La compuerta `docfail` valió lo que costó, en su primera exposición a esto:** el ejemplo
`text("CIV") { xyz(3*d, 0, 5*d) }` —copiado tal cual de §4.5 de este plan— **no compilaba**.
`text` validaba la paridad de sus coordenadas en parse-time contando TÉRMINOS, así que un punto
[x,y] contaba como uno y el error decía «número impar de coordenadas (1)» señalando una línea
correcta. No era un hueco del 3-D: `text("A") { point_at(&p, 0.5) }` fallaba igual, y las
primitivas ya lo aceptaban desde siempre (`PrimStmt::evalPath`). Se arregló —`text` expande
puntos en eval-time, como las primitivas— con dos fixtures: `text_punto_calculado.mg` (que
compile) y `text_impar.mg` (que el error impar NO se perdiera al mudarse de fase).

⚠️ **Un `​```octave` dentro de un blockquote `>` no lo sabe extraer `docblocks.py`**: el `> ` se
cuela en el fuente. Se descubrió al documentar de paso la regla `scale sx (sy)`; la salida fue
poner esos dos ejemplos en línea, que es como el resto del documento escribe sus avisos.

### Fase E — figuras al corpus, **selectivo**

⚠️ **No entran las siete, y el plan anterior lo decía mal.** Tres frenos:

- **`local/` es confidencial a propósito** (`.gitignore`: «figuras de artículos sin publicar…
  se queda aquí por confidencialidad»). Los `.png` de referencia **nunca** salen de ahí. El
  precedente del corpus (`franck_condon`, `turning_points`, `fig4-4`) es que la **reproducción**
  sí entra, con su procedencia en el encabezado; el escaneo, no.
- **Una figura entra por cobertura de MOTOR, no de tema** — la regla que dejó fuera a
  `efectos_atmosfera` y que metió a `elevacion_solar`. Recomendación: **tres**, no siete —
  `lira_II-4` (única usuaria de A en bruto), `richards_1-7` (única de B), `waves` (única de C,
  y la única que ejercita el orden de pintado). Las demás se quedan en `local/`.
- **Nomenclatura:** un ejemplo del corpus **no puede llamarse `lira_II-4`**. El número de figura
  solo se usa cuando la edición es verificable por un lector (Cambridge 2025); si no, va
  **nombre de la física**: `angulo_solido`, `push_broom`, `onda_3d`.

Lo que entre gana goldens en tres backends, `docs/img`, galería y paridad geométrica, con
encabezado a la convención de 2026-07-23.

**✅ `onda_electromagnetica` ENTRÓ el 2026-08-01** (Lira fig. II-1) — la segunda, `ok=84`
(28 ejemplos). Cubre lo que `angulo_solido` no: un **generador** y un **relleno** dentro de un
`plane3d` —la promesa de §13 que no tenía prueba— y **marcadores bajo una matriz no conforme**,
la familia de `plan_anisotropia.md`, que aquí sale limpia (las puntas de E salen verticales, las
de B arriba-derecha). Sus dos amplitudes son distintas **derivando el factor de la cámara**: el
plano de B se escorza, y dibujar los dos campos igual de largos diría algo falso.
Le precedió `waves`/`onda_3d` como **ejercicio preliminar**, que resultó ser la misma figura sin
peines ni cota; **se descartó** al existir la buena, y de él salieron dos hallazgos de motor
(`sine` tragándose sus atributos, y `polygon(&p)` sobre curva generada).

**✅ `angulo_solido` ENTRÓ el 2026-08-01** — la primera, y con ella `ok=81` (27 ejemplos). Es la
única que ejercita `view3d`, `plane3d` y `xyz()`, y aporta el caso duro de la invariante (c) de
la Capa 3: semidiámetros conjugados **genuinamente oblicuos**. Antes de entrar se le cerró el
detalle que la separaba del original —que un meridiano parta el casquete por la mitad—, y la
solución vale la pena anotarla porque el camino evidente **no funciona**: poner el casquete
sobre el plano de un meridiano (`gam = lam`) lo corta por el centro en el ESPACIO, pero la
circunferencia dibujada va a radio 1 mientras el centro del disco está a `rho`, así que el arco
pasa por encima, descentrado ~0.29. Lo que hace falta es que cruce el centro **proyectado**, y
eso es un PUNTO de la esfera: el rayo visual por el centro del casquete la corta en dos y la
raíz **delantera** —la que no se oculta— fija la longitud. Se gasta el parámetro de la
**retícula** y no el del casquete, porque la fase de los meridianos es libre y la posición del
casquete es el asunto de la figura. Y como sale de la cámara, la propiedad sobrevive a cambiarla
(verificado a 35°/38°). 📌 Medido al llegar: el meridiano de 135° ya pasaba por el centro
proyectado con 0.0067 de error, **pero en su mitad oculta** — el problema no era la geometría
sino de qué lado caía.

**⚠️ Decisión de nomenclatura (2026-08-01): ni prefijo `pseudo3d_` ni subcarpeta.** Se
consideraron las dos. El prefijo choca con la regla del proyecto —los ejemplos se nombran por su
**asunto**, no por su técnica; por esa lógica `quickstart` sería `plot_quickstart`— y obliga a
decidir si `orbita_polar` y `gravitacion_orbita` entran, discusión que no termina. La subcarpeta
cuesta cuatro piezas de maquinaria plana: el glob de `make install` (que no las instalaría, en
silencio), la profundidad `examples/../lib` que ese layout existe para garantizar, el
`glob("*.mg")` de `galeria.py` y el `cd "$EXDIR"` de `run.sh`. La familia se distingue donde un
lector la consume: un **grupo editorial** en la galería, que es el mecanismo que ya existía para
esto.

### ✅ La SILUETA (estrategia E) — RESUELTA 2026-08-01, y **sin motor nuevo**

⚠️ **Este plan decía que E pedía «geometría nueva» en el motor. Era falso.** Se resuelve en el
`.mg`, en veinte líneas, y la pieza que faltaba llegó por otro camino: `acos`, añadido esa
misma mañana porque el rodeo `atan2(s, sqrt(1-s*s))` que la referencia enseñaba producía `-nan`
en silencio.

**La derivación.** La proyección restringida al plano de la base es una **afinidad**, y la
tangencia es invariante afín. Así que en vez de resolver «tangente a una elipse desde un punto»
en la página, se **retro-proyecta el ápice al marco de la base**, donde el borde vuelve a ser un
círculo:

1. `O' = xyz(O)`, `U' = xyz(O+u) − O'`, `V' = xyz(O+v) − O'`.
2. Resolver `[U' V']·(a,b) = xyz(A) − O'` — un 2×2.
3. `θ = atan2(b,a) ± acos(r/D)`, con `D = hypot(a,b)`. Pide `D > r`.
4. Los puntos de tangencia vuelven al ESPACIO y las generatrices se dibujan con `xyz()` en sus
   dos extremos: **son rectas de verdad del cono**, y la retro-proyección solo sirvió para
   encontrar θ.

**El cilindro es el hermano fácil:** dos círculos iguales en planos paralelos comparten marco,
así que en la retro-proyección están separados por `s` y las tangentes comunes tocan en
`atan2(s) ± 90°`. Sin `acos` siquiera.

**Verificado dos veces:** numéricamente sobre seis configuraciones aleatorias —cámara, ápice,
centro y una base `u,v` deliberadamente no ortonormal— la elipse nunca cruza la generatriz
(≥ 3e−10) y la toca (≈ 2e−9); y **dentro de la propia figura**, porque `dΩ` es la sección del
cono a una fracción del recorrido y tiene que salir tangente por construcción.

**`irradiancia` ENTRÓ AL CORPUS** con ella (`ok=87`, 29 ejemplos), y trae otra mejora sobre el
original: el ángulo φ se dibuja **en el plano que contiene a n̂ y al eje**, donde el barrido del
arco ES φ. El original lo marca con una flecha doble plana en el papel, y eso miente — el ángulo
entre dos direcciones proyectadas no es la proyección del ángulo del espacio. 📌 Se midió al
colocar el rótulo: puesto en la bisectriz **del espacio** caía sobre la flecha de n̂, porque la
proyección tampoco conserva la bisección. El arco mide; el rótulo señala, y va en la bisectriz
de la página.

Queda de E únicamente `fig18-5`, que suma el **cilindro** a lo ya resuelto.

### Fase F — diferidas
- **Silueta de revolución** (estrategia E): cono/cilindro = dos círculos de A + sus dos
  tangentes comunes. La piden `lira_II-7` y `fig18-5`, y **se difieren juntas** porque comparten
  exactamente esta pieza. Es lo único del plan que necesita geometría nueva (tangencia desde un
  punto a una elipse proyectada, en forma cerrada o con una aproximación documentada).
- Perspectiva; jaula 3-D con ejes y marcas (`box_axis`; su forma **mínima** —tres segmentos
  rotulados— ya la cubre B en `fig18-5`); líneas ocultas discontinuas.

---

## 6. Riesgos anotados

⚠️ **La familia anisótropa (`plan_anisotropia.md`).** Los puntos proyectados son puntos y pasan
por la matriz de mundo sin problema. Pero en cuanto se calcule una **dirección**, una
**perpendicular** o un **radio** en el espacio de la escena, se entra de lleno en la familia
«fórmula isótropa aplicada al caso anisótropo»: la tangente de una curva 3-D proyectada **no** es
la proyección de la tangente 3-D salvo casos particulares. **El caso concreto que ya está en la
tabla es el arco de φ** de `lira_II-7` y `fig18-5`: es estrategia F —se dibuja en el papel,
decorativo— y **no** debe intentar medir el ángulo del espacio. Leer ese plan antes de que
`xyz()` crezca hacia marcadores orientados o normales.

⚠️ **`plane3d` tiene que encender `using_ellipse`**, como ya hace `shear`, o el EPS sale
byte-estable y **revienta al interpretarse** (`/undefined in ellipse`). Es exactamente la clase
de bug para la que existe la compuerta `psfail`, así que está cubierta — pero es lo primero que
hay que comprobar al escribir la sentencia.

⚠️ **Ceros residuales y portabilidad.** La matriz de un plano sale de trigonometría y sus
términos llegan a los mismos sitios de `snap_zero` que ya usan `rotate` y `shear` (el marco de
elipse en EPS, `deviceRotate` en PDF, el SVD en SVG). **No es una clase de riesgo nueva** —hay
que *verificarlo* en la Fase A, no rediseñar nada—, pero sí es de las que ningún golden caza,
porque el golden se genera en una sola plataforma. Ver la nota de `CLAUDE.md` sobre la
identidad byte a byte Linux/Windows.

⚠️ **Esto AÑADE sintaxis antes de congelar la gramática** (condición 1 del 1.0), y ahora son
**dos** sentencias y una función, no una. Es legítimo por la regla de demanda —hay nueve figuras
que lo piden—, pero conviene decidir los nombres **con el mismo cuidado que los de §13**, porque
después del 1.0 renombrar cuesta una migración.

⚠️ **El footgun de siempre:** un identificador desnudo seguido de `(` se parsea como llamada a
función. En coordenadas, parentizar la variable: `(dx) (h+dy)`.

---

## 7. Decisiones tomadas (2026-07-31)

Las cuatro que §7 dejaba abiertas, cerradas con el mismo criterio que se usó en §13 —comparar
con lo que ya usa todo el mundo—:

1. **`xyz(x,y,z)`**, no `point3` ni `p3`. La casa usa nombres descriptivos (`point_at`,
   `angle_at`, `path_width`), lo que empujaría a `point3`; pero en un bloque de coordenadas se
   lee mejor lo corto: `polyline { xyz(0,0,0) xyz(1,0,0) }`.
2. **`view3d`**, no `camera` ni `scene3d`: describe lo que hace —fija el punto de vista— sin
   sugerir que abre un ámbito de escena. Y **`plane3d`** por simetría con él.
3. **Una sola sentencia con `type=`** para las dos proyecciones. Una figura declara *una*
   cámara.
4. **`z` = profundidad**, x derecha, y arriba (§4.1). Se eligió por la identidad en
   `azimuth=0, elevation=0`, que hace del caso frontal el default y no un caso especial.

Y una quinta, que el plan anterior no tenía planteada:

5. **Son dos adiciones al motor, no una:** `xyz()` **y** `plane3d`. La segunda resultó ser la
   más barata de las dos y la de más clientes (§3.2).

---

## 8. Lo que sigue vigente del plan anterior

- **La cizalla ya es ciudadana de primera clase** (`shear` §11.1, `transform=` §17); no hace
  falta construirla. Es lo que sostiene `fig10-2v3` hoy.
- **Nomenclatura:** no llamar `isometric(...)` a nada. El motor ya es «isométrico por
  construcción» (§3.1) y reusar el término confunde.
- **Fases 0-2 hechas:** `lib/pseudo3d.mg`, `fig10-2v3.mg` y `fig2-7b-v3.mg` compilan y están
  calibradas. Son el punto de partida de la Fase C, no trabajo perdido — con la corrección de
  que `plano` **desaparece** en esa fase, absorbida por `plane3d`.

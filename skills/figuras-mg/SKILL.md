---
name: figuras-mg
description: Escribir, corregir y revisar figuras técnicas y científicas con MetaGráfica (mg) — un .mg compilado a SVG, PDF o EPS. Úsalo cuando la tarea mencione una figura en .mg, el comando mg, rehacer una figura de un libro, o una gráfica que deba ser reproducible y paramétrica.
---

# Figuras con MetaGráfica

`mg` es un lenguaje descriptivo que compila un `.mg` a **EPS, SVG o PDF** (el formato lo elige
la extensión de salida). Describes *qué es la figura* y el compilador la dibuja.

> **Este archivo viaja CON el compilador.** Vive en `skills/figuras-mg/` del repositorio y sus
> bloques de código los compila la compuerta `docfail` en cada cambio del motor
> (`tools/docblocks.py`), así que **no puede quedarse mintiendo en silencio** cuando la gramática
> se mueva — que en beta se mueve. Si copias este archivo a otro sitio, la copia sí se pudre.

## Antes de escribir un `.mg`

1. **La referencia del lenguaje.** En el árbol, `docs/referencia.md` (o `docs/reference.md` en
   inglés); instalado, `share/doc/metagrafica/referencia.md`. Lee al menos §4 primitivas,
   §5 estilo, §6 texto, §7 expresiones y control, §8 estructuras y **§15 «Errores comunes»**,
   que es el destilado y el que más rinde.
2. **Los ejemplos**: `examples/` en el árbol, `share/metagrafica/examples/` instalado. Son 31 y
   todos compilan; la galería los muestra renderizados junto a su código.
3. ⚠️ **No copies la referencia a tu proyecto.** Apunta a la instalada: una copia se pudre sin
   que nada avise.

## Reglas duras

Cada una es un error **medido**, no una precaución. Rómpela y la figura sale mal o no compila.
Las tres primeras son las caras.

1. **`world_window` es la geometría de la PÁGINA, nunca las unidades de tus datos.** Escribir
   `world_window 0.4 2.5 0 100` porque los datos van de 0.4 a 2.5 y de 0 a 100 da una figura
   ilegible: el motor es isométrico y el eje largo aplasta al otro. **Compila limpio**, así que
   nada te avisa. Los datos van en `x=`/`y=` de `plot`, y `box=` es la región de la ventana que
   ocupa la caja:

   ```octave
   display_size 10 7
   world_window 0 10 0 7
   plot(x=(0.4,2.5), y=(0,100), box=(1.2,1, 9,6)) {
       xaxis(step=0.5, label="distancia")
       yaxis(step=25, label="intensidad")
   }
   ```

   Si los dos ejes comparten unidad —un plano, una órbita, un mapa, una figura geométrica—
   entonces sí, `world_window` directo y sin `plot`.

2. **Un rótulo de varios renglones es UN SOLO `text()`**, y el corte de renglón se escribe con la
   secuencia de dos caracteres BARRA-DIAGONAL-ENE, o sea `/n`, con la misma barra que `/b`
   (negrita) y `/e` (énfasis):

   ```octave
   display_size 8 5
   world_window 0 8 0 5
   text("Radiancia total/n$L_{tot} = \frac{\rho E T}{\pi} + L_p$") { 4 2.5 }
   ```

   ⚠️ **Y por eso una barra que quieras VER se escribe `\/`.** La barra se come la letra que
   sigue si es una de `b e i g r s c t $ n`, así que `text("5 m/s")` dibuja **`5 m`** —`/s` se
   lee «sans-serif»—. Toca a casi toda unidad con barra: `m/s`, `J/g`, `cal/g`, `1/e`. La regla
   general es que **`\` seguido de un carácter no alfabético es ese carácter, literal**, que es
   también como se escriben `\{`, `\}`, `\$` y `\\`:

   ```octave
   display_size 8 5
   world_window 0 8 0 5
   text("velocidad de 5 m\/s") { 4 3 }
   text("el conjunto \{a, b\} cuesta \$5") { 4 2 }
   ```

3. **Las griegas y los símbolos van por su NOMBRE, con barra invertida y dentro de `$…$`:**
   `$\mu$`, `$\rho$`, `$\pi$`, `$\theta$`. Pegar el glifo Unicode (µ, ρ, π) no funciona: la
   fuente no lo tiene por esa vía y el compilador lo descarta con aviso. Tampoco existe
   `\text{}` ni ningún comando de LaTeX que no esté en la referencia; un subíndice de varias
   letras va con llaves, `$L_{tot}$`.

4. **Los comentarios empiezan con `%`.** Un `#` es un error léxico fatal. Los comentarios sí
   admiten acentos y eñes; el **código** no.

5. **Dentro de un bloque `{ }` los valores se separan por espacios, sin comas,** y toda
   coordenada que sume o reste va entre paréntesis: `{ 12 (y-11) }`, porque `{ 12 y-11 }` son
   tres términos. Las llamadas a función van pegadas al paréntesis: `sqrt(x)`, nunca `sqrt (x)`.

6. **El bloque de coordenadas abre en la MISMA línea** en que acabó la cabeza de la primitiva.
   `rectangle(fill="red") { 0 0  4 3 }` ✅; bajar la `{` al renglón siguiente es un error que
   además señala al primer número, no a la llave.

7. **Un trayecto nombrado se pasa como PRIMER argumento y con `&`:**
   `polyline(&mi_ruta, color="red")`, `dot(&p, size=2)`.

8. **Las flechas y los marcadores son un ATRIBUTO** de la primitiva, que los coloca y los
   **orienta sola**. No los dibujes con `polygon`:

   ```octave
   display_size 8 5
   world_window 0 8 0 5
   polyline(marker_end="arrow") { 1 1  7 1 }
   polyline(marker_start="arrow", marker_end="arrow",
            marker_start_orient="reverse") { 1 3  7 3 }   % cota de doble punta
   ```

   Sin `marker_start_orient="reverse"` las dos puntas apuntan al mismo lado.

9. **`to` es inclusivo y `count` es una cantidad:** `for i = 0 to 4` da **cinco** vueltas.

10. **`display_size` fija el tamaño físico en cm; el texto es una cantidad física en pt.**
    Achicar el lienzo no achica el texto: lo agranda en relación con el dibujo. Para una figura
    más pequeña, baja `font_size`.

11. **Dónde se ancla un marcador depende de su forma, y solo una de las dos rebasa el vértice.**
    Medido: `arrow` se ancla por **la punta**, así que la punta cae *en* el vértice y no invade
    nada. Las formas simétricas (`circle`, `square`, `diamond`, `triangle`, `cross`) se anclan
    por **el centro**, así que medio marcador —`marker_size` pt— se sale por delante. Contra una
    caja se nota, y **solo ampliando**: a tamaño completo pasa por buena. Para retirarlo hay que
    convertir, porque `marker_size` es físico y las coordenadas son de mundo:

    ```octave
    display_size 12 6
    world_window 0 10 0 5
    retiro = 5/72*2.54 / (12 / 10)
    rectangle { 6 1  9 4 }
    polyline(marker_end="circle", marker_size=5) { 1 2.5  (6 - retiro) 2.5 }
    ```

    `marker_end_shift` es la otra vía y su unidad no es obvia: **fracción del segmento del
    extremo**, con default 1. Ver §4 de la referencia.

12. **`polygon` rellena por definición: no se le apaga el relleno.** `polygon(fill="none")` es
    **error** —lo era en silencio hasta el 2026-08-06, y salía relleno de NEGRO—. Un contorno
    cerrado sin relleno es `polyline(closed=true)`, que además no repite el primer vértice:

    ```octave
    display_size 6 5
    world_window 0 6 0 5
    polyline(closed=true, dash="dashed") { 1 1  5 1  5 4 }   % contorno, sin relleno
    polygon(fill=gray(0.85), color="black") { 1 1  5 1  5 4 }  % relleno contorneado
    ```

13. **`concat` SUELDA: traslada cada pieza para que arranque donde acabó la anterior.** No
    yuxtapone en coordenadas absolutas. Es lo que permite armar una curva larga con piezas
    definidas en un dominio canónico, y es una trampa cuando las piezas ya vienen colocadas:
    `concat(&arriba, reverse(&abajo))` para cerrar la banda entre dos curvas **desplaza la de
    abajo** por la diferencia entre los dos extremos que se unen — sin aviso, y el resultado
    compila y se ve como una curva plausible. Para que la soldadura sea la identidad, haz que
    las piezas **ya se toquen**: interpón el segmento de cierre.

    ```octave
    display_size 8 5
    world_window 0 8 0 5
    path arriba = { 1 3  3 4  6 4 }
    path abajo  = { 1 2  3 2  6 1 }
    path cierre = { 6 4  6 1 }                        % la soldadura de aquí es un no-op
    polygon(concat(&arriba, &cierre, reverse(&abajo)), fill=gray(0.85), color="black")
    ```

⚠️ **No inventes argumentos ni añadas mobiliario que no se pidió** (leyendas, retículas, tablas):
si no está en la referencia ni en `examples/`, no existe. Este fallo se midió — **darle el
catálogo entero del lenguaje a un modelo hace que use todo lo que ve**.

## Geometría calculada, no puesta a ojo

Es la regla de estilo de la casa y lo que hace que una figura de `mg` valga más que un dibujo:
la geometría sale de variables y trigonometría **a partir de los parámetros físicos**, de modo
que **el dibujo no pueda desmentir a los rótulos**. La prueba de que está bien hecha es que
cambiar un parámetro de arriba recalcule la figura entera sin tocar una sola coordenada.

`examples/franck_condon.mg` y `examples/turning_points.mg` son la calibración: se les da los
parámetros físicos y la geometría se deduce.

⚠️ **Si un comentario dice que algo se calcula, tiene que estar calculado.** Un literal tecleado
bajo un comentario que promete una fórmula coincide por casualidad hoy y despega mañana, sin que
nada avise. Un número mágico con un comentario que miente es peor que un número mágico a secas.

## Compilar

```bash
mg figura.mg              # → figura.eps
mg figura.mg fig.svg      # SVG
mg figura.mg fig.pdf      # PDF
```

`mg` emite **avisos** que no hacen fallar la compilación y son fáciles de perderse: léelos.

⚠️ **Para rasterizar un SVG hace falta un NAVEGADOR.** `rsvg-convert` e Inkscape ignoran el
`@font-face` con que `mg` incrusta Latin Modern Math y caen a una sans, así que **mienten** sobre
la tipografía matemática —una ρ correcta se ve como «ø»— y es facilísimo diagnosticar como bug de
la figura lo que es sustitución del visor. En el árbol, `tools/ver.sh` ya lo hace bien.

## El último paso no es compilar, es MIRAR

Una figura compila limpio en los tres backends y aun así puede tener rótulos encimados, flechas
al revés o —lo peor— geometría que afirma algo falso. **Nada de eso lo caza el compilador.**

📖 `references/revisar-figura.md` — el procedimiento y la lista de comprobación.

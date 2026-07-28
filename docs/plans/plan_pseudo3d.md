# Plan para simulación pseudo-3D (2.5D) en MG V3

> **Reescrito el 2026-07-28.** El plan anterior (2026-07-12) daba una **biblioteca de
> piezas**; las figuras que ahora lo piden necesitan un **espacio compartido**. Sus fases 0-2
> están hechas y siguen siendo válidas —`lib/pseudo3d.mg`, `fig10-2v3`, `fig2-7b-v3`
> compilan hoy—; lo que cambia es que la **Fase 3, que se difirió como especulativa, pasa a
> ser el núcleo**, porque ya tiene clientes concretos.

Objetivo: dar soporte fiel a la ilustración científica 2.5D —planos que receden, prismas,
pantallas inclinadas, mallas de terreno— manteniendo la filosofía 2D del lenguaje: espacio
isométrico por construcción (§3.1), ortogonalidad forma/posición, y la regla sagrada
`Subpath ::= (Coord Coord)+` **intacta**.

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

## 2. Los clientes (por qué esto deja de ser especulación)

La regla del proyecto es no construir sin una figura que lo pida. Ahora las hay, y son de dos
fuentes: varias del libro de mecánica cuántica, y las del curso de Percepción Remota
(`Libro_PR_P.pdf`). Las que fijan el requisito:

| figura | qué exige |
|---|---|
| **Fig. I.2** — generación de una imagen multiespectral | escena en el suelo, rayos al sensor, **pila de planos** espectrales recediendo |
| **Fig. II.11** — arreglo bidimensional de detectores | arreglo en un plano, óptica, **escena como malla** con franjas |
| **Fig. II.10** — medida de varios CIV en bandas | **la que rompe el diseño actual**, ver abajo |

**Fig. II.10 es el caso decisivo**, por dos razones que `plano`/`prisma` no pueden cubrir:

1. **El suelo es una malla**: decenas de líneas cuyos extremos comparten la misma proyección. No
   es una «pieza» con contorno — lo que importa es la retícula. Precomputarla a mano es inviable.
2. **Los rayos unen un punto en el aire con una celda del suelo.** Ese segmento **no pertenece a
   ninguna forma**: no hay `plano` ni `prisma` al que colgarlo. Necesita que **ambos extremos
   sean nombrables en el espacio de la escena**.

---

## 3. El límite concreto que justifica tocar el motor

El plan anterior puso la barra, y conviene citarla porque se cumple exactamente:

> *«Solo se consideraría subir algo a builtin si un límite concreto del lenguaje lo impide, y se
> anotaría aquí antes de hacerlo.»*

**El límite: MG no tiene funciones de usuario.** Tiene `struct`, y una struct **dibuja**, no
**devuelve**. Así que una función que proyecte `(x,y,z)` a un punto **no puede escribirse en un
`.mg`**. Es la única pieza que obliga a bajar al compilador — y es pequeña.

✅ **Y lo que NO hace falta cambiar, verificado el 2026-07-28:** un bloque de coordenadas **ya
acepta términos que valen un punto** (una lista de dos), que es lo que hace `point_at(&p, t)` y
lo que ejercita `examples/path_sample.mg`. Por tanto la gramática **no se toca** y la regla
sagrada `Subpath ::= (Coord Coord)+` queda intacta.

---

## 4. Diseño

Dos piezas, y todo lo demás sigue igual.

### 4.1 `view3d` — la cámara, como sentencia de estado con alcance

Igual que `translate`/`rotate`: vale desde donde aparece hasta el fin del bloque. **No** como
atributo por-primitiva (no duplicar el concepto, misma decisión que tomó el plan anterior).

```octave
view3d(azimuth=35, elevation=25)          % axonométrica (ortográfica)
view3d(type="oblique", angle=45, foreshorten=0.5)   % caballera/gabinete
```

**Dos proyecciones, porque el corpus tiene las dos:**

- **Axonométrica ortográfica** — la de las figuras del libro de PR. Con acimut θ y elevación φ:

      X = −x·sin θ + y·cos θ
      Y = −(x·cos θ + y·sin θ)·sin φ + z·cos φ

- **Oblicua (caballera/gabinete)** — la de `fig10-2` y `fig2-7b`: la cara frontal conserva su
  forma real y la profundidad recede a un ángulo `angle` con factor `foreshorten` (1 = caballera,
  0.5 = gabinete). Es lo que `lib/pseudo3d.mg` hace hoy con `shear`.

⚠️ **Decisión pendiente (§7):** si son un solo constructo con `type=` o dos sentencias. Recomiendo
uno solo: la figura declara *una* cámara y no debería poder tener dos semánticas activas.

### 4.2 `xyz(x, y, z)` — un punto del espacio de la escena

Función del evaluador que devuelve el punto 2-D proyectado con la `view3d` vigente:

```octave
view3d(azimuth=35, elevation=25)

% la malla del suelo: dos renglones
for i = 0 to n {
    polyline { xyz(i*d, 0, 0)   xyz(i*d, m*d, 0) }
    polyline { xyz(0, i*d, 0)   xyz(n*d, i*d, 0) }
}

% un rayo: óptica → celda del suelo. No pertenece a ninguna «pieza».
polyline { xyz(0, 0, altura)   xyz(3*d, 5*d, 0) }

% y todo lo demás sigue funcionando sin enterarse
text("CIV") { xyz(3*d, 5*d, 0) }
place(Detector, at=(xyz(0, 0, altura)))
```

📌 **Lo que se gana no es sintaxis, es que la cámara pasa a ser un PARÁMETRO.** Cambias
`elevation` y la malla, los rayos y las piezas se mueven juntos. Es la misma propiedad que ganó
`orbita_polar` el 2026-07-27 al derivar la órbita de kilómetros: el número que gobierna la figura,
escrito una sola vez.

### 4.3 Lo que este plan NO incluye

- **Sin z-buffer ni superficies ocultas.** Orden de pintado = orden de escritura, que es como ya
  se dibujan estas figuras. Dibujar lo lejano primero.
- **Sin iluminación ni sombreado automático.** El sombreado de caras se sigue eligiendo a mano
  (`prisma` ya lo hace con tres grises).
- **Sin perspectiva**, de entrada. Es la misma función con un divisor más; se añade si una figura
  la pide, no antes.
- **Sin recorte por volumen.**

---

## 5. Fases

### Fase A — `view3d` + `xyz()` en el evaluador
Lo mínimo para que las mallas y los rayos existan. **Criterio de aceptación:** reproducir la
malla del suelo de Fig. II.10 con el `for` de arriba, y un haz de rayos de la óptica a celdas
concretas. Sin biblioteca todavía.

### Fase B — reescribir `lib/pseudo3d.mg` sobre el espacio
`plano` y `prisma` dejan de hornear la proyección y **calculan sus vértices con `xyz()`**. Ganan
una posición 3-D en vez de un `at=` 2-D. **Criterio:** `fig10-2v3` y `fig2-7b-v3` se reproducen
—calibradas contra sus `.png` como ya lo estaban— y **cambiar la cámara mueve todo junto**, que
es la prueba de que el refactor sirvió de algo.

### Fase C — las figuras del libro entran al corpus
Con encabezado a la convención de 2026-07-23. Ahí ganan goldens en tres backends, `docs/img`,
galería y paridad geométrica. ⚠️ Hoy `local/simulate3d/` **no está trackeado**: mientras siga así,
nada vigila estas figuras.

### Fase D — diferidas
Perspectiva; jaula 3-D con ejes y marcas (`box_axis`); líneas ocultas discontinuas. Ninguna la
pide el corpus.

---

## 6. Riesgos anotados

⚠️ **La familia anisótropa (`plan_anisotropia.md`).** Los puntos proyectados son puntos y pasan
por la matriz de mundo sin problema. Pero en cuanto se calcule una **dirección**, una
**perpendicular** o un **radio** en el espacio de la escena, se entra de lleno en la familia
«fórmula isótropa aplicada al caso anisótropo»: la tangente de una curva 3-D proyectada **no** es
la proyección de la tangente 3-D salvo casos particulares. Leer ese plan antes de que `xyz()`
crezca hacia marcadores orientados o normales.

⚠️ **Esto AÑADE sintaxis antes de congelar la gramática** (condición 1 del 1.0). Es legítimo por
la regla de demanda —hay figuras que lo piden—, pero conviene decidir los nombres **con el mismo
cuidado que los de §13**, porque después del 1.0 renombrar cuesta una migración.

⚠️ **El footgun de siempre:** un identificador desnudo seguido de `(` se parsea como llamada a
función. En coordenadas, parentizar la variable: `(dx) (h+dy)`.

---

## 7. Decisiones pendientes (nombres y forma)

Antes de escribir código, con el mismo criterio que se usó en §13 —comparar con lo que ya usa
todo el mundo—:

1. **`xyz(x,y,z)` vs `point3(x,y,z)` vs `p3(x,y,z)`.** La casa usa nombres descriptivos
   (`point_at`, `angle_at`, `path_width`), lo que empujaría a `point3`. Pero en un bloque de
   coordenadas se lee mejor lo corto: `polyline { xyz(0,0,0) xyz(1,0,0) }`. Recomiendo **`xyz`**.
2. **`view3d` vs `camera` vs `scene3d`.** `view3d` describe lo que hace —fija el punto de
   vista— sin sugerir que abre un ámbito de escena.
3. **Una sentencia con `type=` o dos.** Recomiendo una.
4. **Nombres de los ejes.** ¿`z` es la vertical (convención de terreno, y la que piden estas
   figuras) o la profundidad (convención de pantalla)? Elegir y documentarlo en la referencia:
   es la primera pregunta que hará quien lo use.

---

## 8. Lo que sigue vigente del plan anterior

- **La cizalla ya es ciudadana de primera clase** (`shear` §11.1, `transform=` §17); no hace
  falta construirla.
- **Nomenclatura:** no llamar `isometric(...)` a nada. El motor ya es «isométrico por
  construcción» (§3.1) y reusar el término confunde.
- **Fases 0-2 hechas:** `lib/pseudo3d.mg`, `fig10-2v3.mg` y `fig2-7b-v3.mg` compilan y están
  calibradas. Son el punto de partida de la Fase B, no trabajo perdido.

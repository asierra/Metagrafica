# Calcular en vez de medir

> ⚠️ **BORRADOR** — para discusión, no enlazado todavía desde los README.
>
> **Convención de revisión mientras dure el borrador:** Alejandro edita la prosa
> directamente y deja lo estructural como `[[AS: ...]]`. Antes de dar el documento por
> terminado hay que **barrer que no quede ninguna** (`grep -n '\[\[AS:' docs/*.md`) y
> quitar este bloque de aviso completo.

Casi todas las figuras de un libro de física se **dibujan**: alguien decide dónde va
cada curva y cada marca, y el resultado se parece a lo que la física dice. Este documento
trata de las otras — las que se **calculan**, donde la geometría sale de las fórmulas y
nadie coloca nada a ojo.

MetaGráfica no es una herramienta de análisis de datos y no pretende serlo. La frontera es
deliberada y vale la pena decirla de entrada:

> **MG calcula la geometría de una ilustración a partir de su modelo.** No reduce datos:
> `polybar` recibe intervalos ya contados, no observaciones. Cuando hay que ir de 54 000
> filas a 30 barras, eso ocurre **fuera** del compilador (`tools/hist2mg.py`).

Lo que sí ocurre dentro es el otro caso, y es el que este documento defiende: cuando la
figura tiene un **modelo** detrás —un potencial, una condición de cuantización, un punto
de retorno—, ese modelo puede vivir *en el archivo de la figura* en vez de en la cabeza
del autor o en un script que se pierde.

---

## 1. Una figura con parámetros: Franck-Condon

`examples/franck_condon.mg` dibuja dos potenciales de Morse con sus niveles vibracionales
y sus funciones de onda. **Nada en él está medido.** Se dan cinco números por estado
electrónico y el resto es forma cerrada:

```text
a1  = 1.8            % alcance del potencial
re1 = 1.15           % distancia de equilibrio
we1 = 0.56           % frecuencia vibracional
xe1 = 0.028          % anarmonicidad
D1  = we1/(4*xe1)    % profundidad — NO es libre, ver §4
```

De ahí salen, sin que nadie las coloque:

| lo que se dibuja | de dónde sale |
|---|---|
| la curva | `V(r) = D(1 − exp(−a(r−re)))²` |
| los niveles | `E_v = we(v+½) − we·xe(v+½)²` |
| dónde empieza y acaba cada onda | `r± = re − ln(1 ∓ √(E/D))/a` (los puntos de retorno) |
| cuántos niveles ligados hay | `vmax = 1/(2·xe) − ½` |
| la amplitud de cada lóbulo | envolvente semiclásica WKB, `∝ (E−V)^(−¼)` |
| cuánto penetra la onda en la región prohibida | escala de Airy, `∝ |V′(retorno)|^(−⅓)` |

**La prueba está en el diff.** Cambiar un solo número —la anarmonicidad `xe1`, de `0.028`
a `0.045`— reacomoda la figura entera de forma coherente: el pozo se hace menos profundo
(`D` pasa de 5.0 a 3.1), la línea de disociación baja con él, los niveles se separan y se
apiñan antes, y las ondas se reajustan a sus nuevos puntos de retorno. La anarmonicidad
también decide **cuántos estados ligados admite el potencial**: el último pasa de v = 17 a
v = 10, porque `vmax = 1/(2·xe) − ½`.

> *(Aquí va el par de renders. Están generados y verificados; entran a `docs/img/` cuando
> acordemos el formato.)*

Ninguna de esas consecuencias está escrita en el archivo. Están escritas las **fórmulas**,
y la figura es lo que se deduce de ellas.

**El detalle que mejor resume la idea:** la transición vertical de Franck-Condon —la que
le da nombre al principio— aterriza en el nivel v′≈6 del estado excitado **sin que nadie
la ponga ahí**. Sale del desplazamiento entre las distancias de equilibrio de los dos
estados (`re1 = 1.15` frente a `re2 = 1.48`). Si acercas los dos pozos, la transición se
mueve sola al nivel que le toca. Eso es el principio de Franck-Condon *ocurriendo* en la
figura, no ilustrado en ella.

---

## 2. La física ata las manos, y esa es la ventaja

`examples/turning_points.mg` es un pozo con tres energías: un estado ligado y dos no
ligados. Se dan las asíntotas del potencial, su mínimo, los tres puntos de retorno, las
tres energías y el número de nodos del estado ligado; de ahí salen `V(x)`, las longitudes
de onda, las amplitudes y las colas.

Dos cosas que solo pasan cuando se calcula:

**La curva pasa por los puntos de retorno por construcción, no por ajuste.** La forma
`V(x) = V∞ − (V∞−Vm)·exp(−|(x−xm)/w|^s)` tiene dos parámetros libres de cada lado, y hay
exactamente dos condiciones que imponerles (que la curva valga `E_b` en un retorno y `E_c`
en el otro). Se despejan. No se ajustan: se resuelven. Los retornos rotulados caen sobre
la curva porque no pueden caer en otro sitio.

**Las tres ondas no son independientes.** La constante de fase sale de la condición de
cuantización de Bohr-Sommerfeld aplicada a la onda del estado ligado. Una vez fijada, las
otras dos **no tienen ninguna libertad**: es la misma partícula, así que es la misma
constante. Cambiar el número de nodos del estado ligado de 3 a 4 arrastra a las tres —la
ligada pasa de 3 a 4 lóbulos, y las otras dos de 23 a 30 y de 13 a 16— sin tocar nada más.

Dibujando a mano, cada una de esas ondas es una decisión estética independiente. Aquí es
una consecuencia, y por eso no pueden desmentirse entre sí.

**Y de hecho, medidas, se desmienten.** Esta figura *no* es un port fiel de la publicada,
y la razón es esa: al medir las tres ondas del original se ve que no pueden corresponder a
la misma partícula. La fase por lóbulo varía hasta tres veces dentro de una sola onda —cada
región se dibujó repitiendo un ciclo a la escala que se veía bien— y ninguna constante
reproduce las tres densidades a la vez: con la onda ligada a 4 antinodos, la física pide
unos 24 y 13 lóbulos para las otras dos, contra los 13 y 8 dibujados. Reproducirla
fielmente y *calcularla* eran objetivos incompatibles; se eligió calcularla, y por eso el
ejemplo perdió el número de figura y tomó nombre de la física (`turning_points`).

---

## 3. Calcular encuentra cosas que medir no

`examples/fig4-4.mg` reproduce tres potenciales: el oscilador armónico, el coulombiano y
el efectivo con barrera centrífuga.

En la versión original esas tres curvas estaban **digitalizadas**: 69 puntos cada una,
medidos sobre un dibujo. Al portarlas resultó que los puntos ajustan a formas analíticas
exactas —`V = x²`, `V = 1/r`, `V = 1/(2r²) − 1/r`— con un error del orden de `1e-6`. O
sea: las curvas *siempre fueron* esas fórmulas; lo que se había conservado era una copia
degradada, con 69 números en lugar de una línea.

Al reconstruirlas apareció algo más. El archivo original colocaba una marca con
`DOT 5 011 .3`: un punto decimal perdido que manda esa marca a `x = 454`, muy fuera del
lienzo. **Esa marca falta en la figura publicada**, y nadie lo había notado en veinticinco
años, porque una marca ausente no se ve — solo se ve si algo la calcula y espera
encontrarla.

Es el argumento más concreto a favor de esta forma de trabajar: una figura derivada
**sabe qué debería haber**, así que su ausencia es detectable. Una figura dibujada no lo
sabe.

> 📝 **Nota de tono, para discutir.** Este apartado y el final del §2 dicen que hay
> defectos en figuras publicadas. Están redactados en neutro y sin dramatizar: son
> figuras de finales de los noventa, dibujadas con las herramientas de entonces, y el
> punto no es el error sino que **calcular lo hace visible**. Se puede subir o bajar el
> tono, o quitarse; es la parte más delicada del documento y conviene decidirla a
> propósito.

---

## 4. El compilador entiende de física más de lo que parece

El caso que más dice sobre todo esto es un error que se cometió escribiendo
`franck_condon.mg`.

La profundidad del pozo de Morse **no es un parámetro independiente**: la relación
`D = we/(4·xe)` la fija a partir de la frecuencia vibracional y la anarmonicidad, que son
las dos cantidades que se miden en un espectro. Al principio se dio `D` por separado, como
si fuera libre.

El resultado no fue una figura fea. Fue un **error de compilación**:

```
Error de evaluación: ln requiere argumento positivo
```

Con una `D` inconsistente, algunos niveles quedan por encima de la energía de disociación,
y entonces el punto de retorno externo `r₊ = re − ln(1 − √(E/D))/a` deja de existir: el
argumento del logaritmo se vuelve negativo. La física se volvió aritmética imposible, y
el compilador lo dijo y se detuvo.

Un programa de dibujo habría producido una figura perfectamente presentable con niveles
flotando sobre la disociación. Y esa es, además, la razón más fuerte por la que en MG un
error de evaluación es **fatal** y no un aviso: un documento inconsistente no debe
producir salida.

---

## Qué gana la figura con esto

- **La derivación viaja con el dibujo.** No hay un script aparte que calculó los números y
  se perdió, ni una imagen cuyo origen haya que reconstruir. El archivo *es* el argumento.
- **Se puede volver a preguntar.** «¿Y si la anarmonicidad fuera mayor?» se responde
  cambiando un número y recompilando, no rehaciendo la figura.
- **Las partes no pueden desmentirse entre sí.** Si los niveles, los retornos y las ondas
  salen todos de los mismos parámetros, no hay forma de que uno diga una cosa y otro la
  contraria — que es el modo típico de fallar de una figura dibujada por partes.
- **Los errores se vuelven detectables.** Sea un decimal perdido o una relación de Morse
  ignorada: lo que está derivado se puede contradecir, y contradecirse es lo que hace que
  un error salte.
- **Es texto.** Se versiona, se compara con `diff`, se revisa en un pull request y se
  regenera dentro de veinte años sin depender de que exista el programa con el que se
  dibujó.

Nada de esto exige que todas las figuras se hagan así. El corpus de MG tiene ports fieles,
figuras esquemáticas y diagramas donde no hay ningún modelo que calcular, y están bien
como están. Pero cuando **sí** hay un modelo detrás, dejarlo escrito cuesta lo mismo que
esconderlo.

# Plan: dónde y cómo buscar usuarios

> **Estado (2026-07-28):** abierto. La **infraestructura ya está**; lo que falta es gente.
> Documento de decisión, no de código: no bloquea ninguna condición del 1.0 por sí mismo,
> pero es el único camino a la **condición 4** (§22.7, `PENDIENTES.md`).

---

## 0. El objetivo NO es "más usuarios"

Es una tentación fácil y lleva a optimizar la métrica equivocada. Lo que el proyecto necesita
está escrito en la condición 4 desde el 2026-07-21:

> *«El autor no puede hacerse esa prueba solo: ya sabe cómo se llaman las cosas.»*

Lo que falta es **retroalimentación sobre ergonomía y NOMBRES**, antes de congelar la gramática.
Descargas, estrellas y visitas no son eso. La prueba de que hace falta ya está en el repo: el
renombre de §13 (`title`→`label`) no lo cazó el corpus ni ninguna compuerta — lo cazó comparar
con matplotlib.

**Criterio de salida, por evidencia y no por calendario:** que alguien de fuera escriba figuras
no triviales desde cero y que **las figuras nuevas dejen de mover la gramática**. Es la regla del
proyecto («no se construye sin una figura que lo pida») aplicada a los nombres.

⚠️ Corolario incómodo: **diez personas que escriban figuras valen más que diez mil visitas.** El
orden de abajo sale de eso, no de alcance.

---

## 1. Lo que ya está resuelto (2026-07-27/28)

No hay que volver a discutirlo; está hecho y verificado:

- **Binarios para los tres sistemas** en Releases (`v3.0.0-beta`): `mg.exe` de Windows —el caso
  extremo, ahí no hay compilador de sistema—, Linux y macOS. Probar MetaGráfica ya no exige
  compilar, que era el filtro que se llevaba por delante a casi todo el que llegaba.
- **Galería bilingüe** (`docs/galeria.html` / `docs/gallery.html`) con caja «Pruébalo».
- **README y referencia en dos idiomas**, la traducción con compuerta de desfase (`trfail`).
- **Canal de retroalimentación** explícito: el gestor de issues, enlazado desde donde el texto
  dice «buscamos tu opinión», con la nota de que un «no encontré cómo hacer X» sirve tanto como
  un error.

📌 **El orden importaba:** buscar audiencia antes de tener dónde aterrizar la retroalimentación
habría desperdiciado la ola. Ahora el orden es el correcto.

---

## 2. Los foros, por rendimiento esperado

### (a) Taller con estudiantes — UNAM · el de mayor rendimiento

Dos horas, o una figura como ejercicio de curso. Diez personas dibujando desde cero, en la misma
sala, **produciendo exactamente la evidencia que la condición 4 pide** — con la ventaja de que
se les ve tropezar en vivo, que es cien veces más informativo que un issue.

Prueba concreta: si nadie pregunta «¿cómo se llamaba lo del eje?», la condición 4 está cumplida.
Si preguntan, ahí está la lista de renombres que hay que hacer **antes** de congelar.

Material listo: la galería sirve de guion, y los ejemplos son copiables.

### (b) TUGboat (TeX Users Group) — el mejor encaje internacional

Su público es exactamente el nuestro: gente que se preocupa por la calidad tipográfica de una
figura, que trabaja con PostScript y PDF, y que **ya piensa en describir figuras en vez de
dibujarlas**. **MetaPost es el pariente más cercano de MG** y ahí es donde vive esa conversación.
Publican reportes de herramientas en formato amable, sin la ceremonia de una revista de
investigación.

- **Costo real:** hay que escribirlo en inglés. La referencia ya está traducida (2026-07-28), así
  que el techo de idioma bajó, pero el artículo es trabajo aparte.
- **Ángulo:** ver §3. La comparación honesta con MetaPost/TikZ **incluyendo lo que ellos tienen y
  MG no** (estar instalado en todas partes, una década de paquetes) da más credibilidad que
  cualquier lista de características.

### (c) Revista Mexicana de Física E — enseñanza de la física, en español

Nuestras figuras **son** su tema. Complementa a Cuadernos TIC (§2d): uno sobre la herramienta,
otro sobre las figuras como material didáctico.

### (d) Cuadernos TIC (UNAM) — ya en curso

<https://cuadernos.tic.unam.mx/index.php/cua>. En español, sobre TIC en el trabajo académico.
Encaja el ángulo de **reproducibilidad** (§3) mejor que el de «hice un lenguaje».

### (e) Show HN en Hacker News — costo cero, ola de un día

Gancho real: un lenguaje de 35 años, de 1991 a hoy, que compila a EPS/SVG/PDF con salida idéntica
byte a byte en tres sistemas.

⚠️ Honestidad sobre lo que da: el tráfico de HN se va en un día y casi nadie se queda. El primer
comentario será que la documentación está en español. Aun así puede traer **el tipo de usuario que
abre buenos issues**, que es lo que buscamos.

### Descartados, con su razón

- **SIGGRAPH** — premia investigación en rendering y 3D. MG no compite ahí y el costo de asistir
  no se justifica. No es cuestión de calidad, es de foro.
- **StackOverflow** — es un sitio de preguntas y respuestas, **no de difusión**: un post de «miren
  mi herramienta» se cierra por autopromoción. Lo único legítimo es contestar preguntas reales
  donde MG sea una respuesta —«figuras de calidad de publicación sin arrastrar LaTeX», «figuras
  reproducibles»— declarando la autoría. Rendimiento bajo, fricción alta.

---

## 3. El ángulo del mensaje (vale para todos los foros)

**No «hice un lenguaje»** —hay muchos— **sino «la figura como documento fuente reproducible».**

Y con eso viene una afirmación rara y **verificable**, que casi nadie puede hacer de sus figuras:

> El mismo `.mg` produce los **72 archivos idénticos byte a byte en Linux, Windows y macOS**,
> medido por CI en cada release, no por confianza.

Es exactamente el problema que las revistas empujan sin que nadie lo resuelva para la parte
gráfica. Tres apoyos más:

1. **Las ilustraciones son autodemostrativas.** Cada figura puede ir junto al código que la
   dibuja, en media página. `franck_condon` y `turning_points` sostienen solas el argumento de
   «paramétrica, no medida a ojo»: se cambia un parámetro físico y la figura se reacomoda entera.
2. **La bitácora es material primario** — una libreta de laboratorio con la medición que sostiene
   cada decisión, incluidas las que se probaron y **no** funcionaron. Para un reporte de
   experiencia técnica, eso no hay que reconstruirlo de memoria.
3. **El arco 1991 → 2026.** El artículo de *Ciencias* **21** (1991) y su listado impreso en el
   apéndice, del que `fractal_tree` está reconstruido y hoy compila; la V1 que compuso los libros
   de Cetto y de la Peña; y una V3 que solo crece cuando una figura de clase lo pide. Treinta y
   cinco años de la misma idea, con la evidencia todavía ejecutable.

---

## 4. Lo que puede salir mal

- **Prometer estabilidad que no hay.** La etiqueta es beta y la gramática puede renombrar. Las
  notas del release ya lo dicen bien —qué está en beta (los nombres, no la salida), que los
  nombres viejos fallan ruidosamente y nunca en silencio—; conviene reusar ese texto y no
  improvisar uno más optimista.
- **El techo del idioma.** Con la referencia ya traducida el techo bajó, pero la bitácora, los
  planes y los comentarios de los ejemplos siguen en español (y así se quedan: es política del
  proyecto). Los foros en español van a convertir mejor, y eso está bien.
- **Confundir ruido con señal.** Si llegan 500 visitas y ningún issue, la condición 4 sigue sin
  cumplirse. La métrica es **figuras escritas por otros**, no descargas.

---

## 5. Siguiente paso concreto

Fijar fecha para el taller (§2a). Es el único de la lista que produce la evidencia que bloquea
el 1.0; todo lo demás es alcance, y el alcance sin retroalimentación no mueve ninguna condición.

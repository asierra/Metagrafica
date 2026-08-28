# Plan: dónde y cómo buscar usuarios

> **Estado (act. 2026-08-04):** abierto. La **infraestructura ya está**; lo que falta es gente.
> Documento de decisión, no de código: no bloquea ninguna condición del 1.0 por sí mismo,
> pero es el único camino a la **condición 4** (§22.7, `PENDIENTES.md`).
>
> **Revisión 2026-08-04.** Se contrastó el plan contra una lista genérica de «cómo conseguir
> usuarios para tu proyecto de código abierto» (generada por un LLM, fuera del contexto del
> proyecto). El saldo, que vale la pena registrar porque es reproducible: de sus ~13 consejos,
> **cinco ya estaban hechos** (gancho sin jerga, prueba visual, instalación de un paso, HN,
> traducción — esta última **al revés** de como la suponía), **cuatro se descartaron con razón**
> (Product Hunt, AlternativeTo, Dev.to, y la FAQ escrita antes de tener preguntas reales) y
> **cuatro entraron**: los *topics* de GitHub y la vista previa social (§5), el clip del cambio
> de parámetro (§5), los foros de TeX en Reddit (§2f) y el compromiso de responder issues (§6).
> ⚠️ **La lista optimizaba adopción —descargas, votos, alcance— que es exactamente la métrica
> que §0 declara equivocada.** Lo que sobrevivió, sobrevivió por servir a la condición 4, no por
> venir recomendado; y ese filtro es el que hay que volver a aplicar la próxima vez.

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

- **Binarios para los tres sistemas** en Releases (`v3.1.0-beta`): `mg.exe` de Windows —el caso
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

📦 **El instrumento está escrito: `taller_material.md`** (2026-08-04) — anuncio, boleto de
entrada, tres encargos de reserva, hoja de conteo y la disciplina del facilitador. El horario
de la sesión —minuto a minuto, y la regla que lo gobierna: se nombra desde la pantalla solo lo
que está *arriba* de `plot`— está en `taller_temario.md`. Tres
decisiones de ahí que conviene no deshacer al adaptarlo:

- **Dos sesiones separadas por dos semanas, no una.** La condición 4 pide *«un periodo»*, y las
  dos semanas de en medio son parte del instrumento: es donde se mide qué se rompe sin el autor
  delante. La sesión 1 da fricción de primer contacto; la 2 dice **quién volvió**.
- **Los encargos no nombran un solo comando.** Están escritos en el lenguaje del problema. Un
  encargo que diga «usa `xaxis`» deja de poder medir si alguien habría dado con `xaxis`.
- **Se mide descubribilidad, no memoria:** la referencia va abierta, porque un usuario real
  siempre la tiene. Eso pone a prueba de paso `docs/referencia.md` con humanos, que es algo que
  ninguna compuerta hace — `docfail` verifica que sus ejemplos compilen, no que alguien encuentre
  lo que busca.

⚠️ Y el dato que más se olvida registrar: **qué nombre INTENTÓ la persona**. Un nombre que
alguien adivina es un candidato a nombre nuevo; sin esa columna sabes que hubo fricción pero no
hacia dónde renombrar, que es la mitad útil.

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

### (f) r/LaTeX y foros de TeX — el mismo público de TUGboat, sin el artículo

Costo casi cero y **el mismo encaje que (b)**: gente que ya describe figuras en vez de dibujarlas
y que conoce MetaPost. La diferencia con TUGboat es el costo —un post contra un artículo en
inglés— y lo que devuelve: comentarios sueltos en vez de lectores. Sirve de **ensayo del ángulo**
antes de escribir (b): si el mensaje de §3 no engancha ahí, no va a enganchar en TUGboat.

⚠️ Declarar la autoría siempre, y llegar con una figura, no con un anuncio.

### Descartados, con su razón

- **SIGGRAPH** — premia investigación en rendering y 3D. MG no compite ahí y el costo de asistir
  no se justifica. No es cuestión de calidad, es de foro.
- **StackOverflow** — es un sitio de preguntas y respuestas, **no de difusión**: un post de «miren
  mi herramienta» se cierra por autopromoción. Lo único legítimo es contestar preguntas reales
  donde MG sea una respuesta —«figuras de calidad de publicación sin arrastrar LaTeX», «figuras
  reproducibles»— declarando la autoría. Rendimiento bajo, fricción alta.
- **Product Hunt** (2026-08-04) — su público busca **productos terminados** que resuelvan algo hoy,
  y su moneda son los votos del día del lanzamiento. Un compilador de línea de comandos, en beta,
  para figuras científicas, no tiene ahí ni audiencia ni forma. Y el fracaso sería del tipo
  caro: gastar el único lanzamiento que se puede hacer en el foro equivocado.
- **AlternativeTo** (2026-08-04) — su gancho es «alternativa libre a software propietario caro», y
  las alternativas reales de MG —TikZ, MetaPost, matplotlib— **son todas libres y gratuitas**. El
  encuadre no aplica, así que el listado no diría nada cierto ni útil.
- **Dev.to / Hashnode** (2026-08-04) — el consejo genérico («escribe artículos titulados por el
  problema») es bueno, pero **el foro está mal elegido para este público**: quien compone figuras
  de física no lee blogs de desarrollo web. El mismo esfuerzo puesto en (b), (c) o (d) llega a
  los lectores correctos. El *método* sí se adopta: ver §3, titular por el problema y no por la
  herramienta.

---

## 3. El ángulo del mensaje (vale para todos los foros)

**No «hice un lenguaje»** —hay muchos— **sino «la figura como documento fuente reproducible».**

Y con eso viene una afirmación rara y **verificable**, que casi nadie puede hacer de sus figuras:

> El mismo `.mg` produce los **93 archivos idénticos byte a byte en Linux, Windows y macOS**,
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

## 5. La vitrina: lo que ve quien llega, antes de decidir si se queda

Nada de esto trae visitantes; decide qué pasa con los que ya llegaron. Es barato y **está a
medias**, que es la peor combinación posible.

**Hecho, y conviene no re-litigarlo:** la frase de una línea sin jerga encabeza los dos README;
la prueba visual son **figuras renderizadas** —cuatro en el README, la galería completa— que para
este proyecto valen más que un GIF de demostración, porque lo que hay que enseñar no es una
interfaz moviéndose sino una figura de calidad publicable; y la instalación de un paso son los
binarios de §1.

**Lo que falta (medido el 2026-08-04 contra `api.github.com/repos/asierra/Metagrafica`, que
responde sin autenticar — así que esto se vuelve a comprobar en un comando), en orden de costo:**

- [x] ~~**Los *topics* del repositorio**~~ — **HECHO 2026-08-04.** Estaban en `[]`. Es el buscador
  propio de la plataforma y es donde se busca por problema; sin *topics* el repo solo aparecía si
  ya sabías su nombre — o sea, solo para quien no necesita encontrarlo. Quedaron doce:
  `vector-graphics`, `scientific-visualization`, `eps`, `svg`, `pdf`, `latex`, `postscript`,
  `metapost`, `figures`, `plotting`, `cpp`, `compiler`. ⚠️ **`metapost` y `latex` son los que
  importan**: son los que busca el público de §2b y §2f, y los únicos que traen a alguien que ya
  piensa en describir figuras en vez de dibujarlas. `cpp` y `compiler` describen la herramienta,
  no el problema, y por eso valen menos (§3).
- [x] ~~**El campo *Website* del repositorio**~~ — **HECHO 2026-08-04**, apuntando a
  `docs/gallery.html` (la inglesa, por alcance; la página enlaza a la española). Estaba vacío
  mientras las dos galerías **sí se servían** (HTTP 200): la galería llevaba publicada y
  funcionando desde el 2026-07-23 y el enlace que GitHub reserva justo para eso —arriba a la
  derecha, el primero que se ve— estaba en blanco.
- **La imagen de vista previa social** (Settings → Social preview) — **generada 2026-08-04, falta
  SUBIRLA**: es el único de los tres que la API no expone, así que el último paso es a mano.
  `docs/img/social-preview.png` (1280×640), y `tools/social_preview.sh` la rehace. La figura es
  `seccion_eficaz` porque su lienzo es 13×5.98 = **2.17:1**, casi el 2:1 que pide el formato: llena
  el marco sin blanco muerto, donde las candidatas 16:9 dejan bandas. Lleva el nombre abajo a la
  izquierda. ⚠️ **Se descartó `angulo_solido`, que era casi neutra de idioma** (solo `A = πr²`, `ρ`,
  `r`) frente a los rótulos españoles de ésta — a sabiendas, porque llenaba peor el marco; si el
  público internacional pesa más adelante, ahí está la alternativa y la razón. Medido antes: el
  `og:image` de la
  página apunta a `opengraph.githubassets.com`, que es **la tarjeta autogenerada** — cada enlace
  que alguien comparta sale con texto sobre gris, no con una figura. La imagen no hay que
  diseñarla: **es una figura del corpus**, que es justo el argumento. Diez minutos.

⚠️ Los tres son ajustes de **configuración del repositorio**, no del árbol; no hay commit que los
haga, y por eso **ninguna compuerta puede vigilarlos** — si alguien los borra, nada avisa. El
`curl` de arriba es la única verificación, y no cuesta nada repetirlo. Los dos primeros se
aplicaron con `gh repo edit` (requiere el alcance `repo`) y se comprobaron **contra la API
pública**, no contra la respuesta de `gh`: la herramienta que hace el cambio no es buen testigo
de que el cambio se hizo.
- [x] ~~**Un clip de 20 segundos del cambio de un parámetro**~~ — **HECHO 2026-08-05**, y
  quedó en un comando: `tools/clip_parametrico.sh`. Barre `xe1` de `0.028` a `0.045` sobre
  `franck_condon`, compila un cuadro por valor, rasteriza con la receta de `ver.sh` y arma el
  GIF (472 K, lazo de 8 s, 19 s de generación). El pozo se hace menos profundo, la disociación
  baja y los niveles se apiñan — **de mover un número**. Es el argumento entero de «paramétrica,
  no medida a ojo» sin una palabra de explicación, y sirve en (a), (e) y (f).

  ⚠️ **El GIF NO va en git; el script sí.** Se genera para la ocasión, como una diapositiva. No
  es por el peso: es que **no puede tener compuerta**. Dos corridas seguidas dan un GIF byte a
  byte idéntico (medido), pero esa determinación es prestada de la paleta de `ffmpeg` y del
  rasterizador de Chrome, y nadie fija esas versiones — la de `docs/img/*.svg` es del propio
  compilador y el workflow la verifica en tres sistemas. Una compuerta de bytes sería verde en
  una máquina y roja en la siguiente. Por eso el default escribe en el directorio actual y no
  en `docs/img/`, que es la carpeta que uno añade entera a un commit sin mirar. Lo que sí lo
  vigila es una compuerta de **humo** (`humofail`): corre el script con dos cuadros en cada
  `check`, para que no nos enteremos de que se rompió el día del taller.

  ⚠️ **Y no sustituye al par de imágenes del README** (se consideró y se descartó el
  2026-08-05): cambiaría vector por ráster justo en la sección que argumenta calidad
  tipográfica, y la tabla de dos columnas deja **comparar** las seis cosas que el párrafo
  siguiente enumera, mientras que un lazo de 8 s obliga a cazarlas al vuelo. Si algún día se
  quiere en la portada, la forma barata es un `<details>` plegado: GitHub no baja el GIF hasta
  que alguien lo abre.

  📌 **Lo que encontró de paso**, y es el argumento para barrer parámetros en general: un
  barrido visita valores que el ejemplo nunca compiló. Éste destapó que `franck_condon.mg`
  **abortaba** en `xe1 = 0.040` y que el render publicado del `0.045` dibujaba dos niveles
  inexistentes con las rayas saliéndose del papel. Las dos cosas llevaban ahí, en la portada,
  con las ocho compuertas en verde.

⚠️ **Una FAQ o guía de resolución de problemas NO entra todavía, a propósito.** La documentación
de errores comunes que existe (§15 de la referencia) la escribió quien ya sabe el lenguaje, y ese
es exactamente el sesgo que la condición 4 existe para corregir. **Las preguntas de la FAQ hay
que cosecharlas del taller (§2a)**, no inventarlas: escribirla antes es adivinar, y adivinar mal
la deja rancia desde el primer día.

---

## 6. Retener a los primeros: la responsividad ES la condición 4

Un consejo genérico de promoción que aquí **cambia de categoría**. En un proyecto normal contestar
rápido los issues es buena educación de mantenedor; aquí es el mecanismo de medición: cada issue
es un dato sobre nombres y ergonomía, y **el que no se contesta no trae el segundo**, que suele ser
el bueno. Quien abre «no encontré cómo hacer X» y no recibe respuesta en unos días concluye —con
razón— que el proyecto está muerto, y esa persona era la evidencia.

Compromiso, mientras los issues se cuenten con los dedos: **respuesta en 48 horas**, aunque sea
para decir «lo estoy viendo». Cuesta cero y es lo único de esta lista que no depende de nadie más.

📌 **La localización, invertida.** El consejo genérico supone un proyecto en inglés que debe
traducirse; MG nació en español y **ya está traducido al inglés** (README, referencia con compuerta
`trfail`, galería). El techo de idioma que queda —bitácora, planes, comentarios de los ejemplos—
es **política del proyecto** (§4) y no un pendiente.

---

## 7. Siguiente paso concreto

Fijar fecha para el taller (§2a). Es el único de la lista que produce la evidencia que bloquea
el 1.0; todo lo demás es alcance, y el alcance sin retroalimentación no mueve ninguna condición.

De los tres ítems de configuración de §5 (2026-08-04): *topics* y campo *Website* quedaron
**aplicados**, y la vista previa **generada** (`docs/img/social-preview.png`). Queda un solo gesto
manual: **subirla** en Settings → Social preview, porque es lo único que la API no expone.

# Material del taller — el instrumento de la condición 4

> **Qué es esto.** Lo que hay que tener listo para correr el taller de `plan_promocion.md` §2a:
> anuncio, encargos, boleto de entrada y hoja de conteo. **No es un plan** —el plan está en
> `plan_promocion.md`— sino el instrumento con el que se toma la medida.
>
> Creado 2026-08-04. Las fechas van en blanco a propósito: se fijan al elegir el curso.
>
> ⚠️ **Público supuesto:** posgrado/licenciatura avanzada en física o ciencias atmosféricas, que
> es lo que sugiere el corpus (`elevacion_solar` nació para un curso, `irradiancia` es de Lira,
> `multietapa` de percepción remota). **Si el grupo es otro, cambia el ASUNTO de los tres
> encargos y no su estructura**: cada uno apunta a una zona distinta de la gramática, y eso es lo
> que hace que la fricción salga repartida en vez de concentrada.

---

## Convención de este documento

> **Lo que va en cita se ENTREGA tal cual** (anuncio, boleto, encargos).

Todo lo demás —notas, «zona de la gramática», advertencias— es **para quien facilita y no sale de
sus manos**. ⚠️ Las notas de §3 nombran comandos a propósito, para que el facilitador sepa qué
está midiendo cada encargo: **si se copian junto con el encargo, los nombres se filtran y ese
encargo deja de medir nada.** Al repartir, copia **solo el bloque en cita**.

---

## 0. La regla que lo gobierna todo

**Lo que se mide es DESCUBRIBILIDAD, no memoria.** Se les da la referencia y la galería abiertas,
porque un usuario real siempre las tiene. La pregunta no es «¿se acuerdan del nombre?» sino
«¿pueden encontrarlo?».

De ahí salen dos prohibiciones que si se rompen dejan el taller sin datos:

1. ⚠️ **Los encargos NO nombran un solo comando.** Si el encargo dice «usa `xaxis`», ya no se
   puede medir si alguien habría dado con `xaxis`. Están escritos en el lenguaje del problema
   —ejes, rótulos, una curva— y nunca en el del compilador. **Al adaptarlos, respeta esto.**
2. ⚠️ **Nada de demo-y-copien.** Si los nombres pasan por la pantalla, la evidencia vale cero.

---

## 1. Anuncio

Para que un profesor lo lea en clase o lo reenvíe. Corto a propósito: abre con lo que la persona
se lleva, no con «hay un lenguaje».

> **Taller: figuras de publicación escritas como código**
>
> Dos sesiones de dos horas, ____ y ____ , en ____ .
>
> Traes una figura que necesites de verdad —de tu tesis, de un reporte, de un artículo— y sales
> con ella hecha, en EPS, SVG o PDF, y en un archivo de texto que puedes versionar, parametrizar
> y volver a generar cuando cambien los datos. Sin ratón y sin arrastrar LaTeX.
>
> La herramienta es **MetaGráfica**, que se usó para las figuras de los libros de mecánica
> cuántica de Cetto y de la Peña. Puedes ver qué produce aquí:
> <https://asierra.github.io/Metagrafica/docs/galeria.html>
>
> **Está en beta, y por eso el taller importa tanto como el resultado:** los nombres de los
> comandos todavía se pueden cambiar, y lo que a ti te resulte incómodo o difícil de encontrar es
> exactamente lo que hace falta saber antes de congelarlos. Vas a estar probando la herramienta
> tanto como usándola.
>
> **Cupo: 12.** Antes de la primera sesión hay que hacer una prueba de cinco minutos (va abajo);
> quien no la mande pierde el lugar, porque no queremos gastar la sesión instalando.
>
> Inscripción: ____ .

📌 **Por qué el cupo va en el anuncio.** No es escasez fabricada: con más de doce personas no se
alcanza a ver a nadie tropezar, y el tropiezo es el dato. Decirlo además filtra a quien viene por
curiosidad, que es justo quien no vuelve a la segunda sesión.

---

## 2. Boleto de entrada

Se manda con el anuncio. Cinco minutos, y ahorra media hora de la sesión — que es el 25 % de la
ventana de observación.

> **Antes del taller (5 minutos)**
>
> 1. Baja el binario de tu sistema de
>    <https://github.com/asierra/Metagrafica/releases/latest> y descomprímelo.
>    No hay que instalar ni compilar nada.
> 2. Dentro de la carpeta, corre:
>
>        ./mg examples/quickstart.mg prueba.svg        (Linux y macOS)
>        mg.exe examples\quickstart.mg prueba.svg      (Windows)
>
> 3. Abre `prueba.svg` en el navegador y **mándalo** a ____ .
>
> Si algo falla, mándame el mensaje de error tal cual: **eso también es información útil** y no
> es que lo hayas hecho mal.

⚠️ **El último renglón no es cortesía.** Un fallo de instalación reportado es un dato de la
condición 4 tan bueno como una pregunta sobre nombres, y sin esa frase la gente calla y
simplemente no llega.

---

## 3. Los tres encargos de reserva

Para quien llegue sin figura propia. **La figura propia siempre es mejor** —fidelidad y motivo
para seguir después— así que estos son red, no plan.

Van de menos a más y **cada uno cae en una zona distinta de la gramática**, para que la fricción
se reparta: el A en los generadores y sus nombres, el B en estructuras y colocación, el C en
expresiones y control de flujo.

### Encargo A — una curva medida, con sus ejes

> Tienes una serie de mediciones: una magnitud que crece con el tiempo. Haz una gráfica con sus
> dos ejes rotulados —uno con unidades— donde la curva teórica vaya como línea continua y los
> puntos medidos aparezcan como marcas sueltas encima. Que se distinga cuál es cuál sin tener que
> explicarlo.

*Zona de la gramática:* el generador de gráficas y sus argumentos, rótulos de eje, marcas de
datos, la caja que explica los símbolos. **Es donde se espera más fricción de nombres**, y donde
ya hubo un renombre (`title`→`label`, `labels`→`tick_labels`) que no cazó ninguna compuerta.

### Encargo B — un montaje experimental

> Dibuja el esquema de un montaje: una fuente, algo que el haz atraviesa o en lo que rebota, y un
> detector, con el camino del haz marcado y una flecha que indique hacia dónde va. Rotula cada
> pieza. Si alguna pieza se repite, no la dibujes dos veces.

*Zona de la gramática:* estructuras reutilizables, colocarlas y orientarlas, flechas, texto con
símbolos. La última frase es la trampa deliberada: empuja hacia `struct` sin nombrarla.

### Encargo C — una figura que se calcula sola

> Haz una figura donde los números físicos estén escritos **una sola vez, arriba del archivo**, y
> todo lo demás salga de ellos: si cambias un parámetro y recompilas, la figura entera se
> reacomoda. Puede ser una trayectoria, un pozo de potencial con sus niveles, una órbita — lo que
> te toque. La prueba de que está bien hecha es que **cambiar un número no te obligue a mover
> ninguna coordenada a mano**.

*Zona de la gramática:* variables, expresiones, funciones matemáticas, repetición. Es el argumento
central del proyecto («paramétrica, no medida a ojo») puesto en manos de alguien más.

📌 **Si el grupo es de otra área, cambia el asunto y conserva los tres papeles**: uno de datos con
ejes, uno de esquema con piezas repetidas, uno paramétrico. La zona de gramática es lo que
importa.

---

## 3-bis. El bloque de agente — segundo instrumento, no un tema más

**Va DESPUÉS del bloque sin agente, y el orden no es negociable:** una vez que alguien vio el
nombre que el modelo escribió, no se puede des-ver. Todo el §4.1 se mide antes o no se mide.

Que la gente vaya a querer un asistente no es una concesión a la realidad: **es el instrumento
que más ha rendido hasta hoy.** Los dos únicos datos que ha dado la condición 4 salieron de ahí
(bitácora 2026-07-28): un modelo con la referencia y una imagen destapó que `marker_end` era
**indescubrible en la documentación** —aparecía una sola vez, de pasada— y que faltaban
degradados. Los dos se arreglaron; el segundo es una de las novedades de la 3.1.0.

### Qué mide esto que aquellos experimentos NO podían

Los de julio los hizo el autor, y eso limita tres cosas que el taller sí alcanza:

1. **Quién escribe la petición.** Ahí prompteaba quien ya sabe qué pedir. En el taller lo hace
   alguien de fuera, y **la petición misma es el dato**: qué pide, con qué palabras, qué da por
   supuesto.
2. **El lazo cerrado.** En julio el modelo **no podía ejecutar `mg`**. Aquí sí: tienen el binario
   del boleto de entrada, así que el modelo itera contra **errores reales del compilador**.
   ⚠️ **Nadie ha medido nunca si los diagnósticos de MG le sirven a un modelo**, y es el peor
   escenario posible para un mensaje malo: `docblocks.py` existe porque *un humano tropieza y
   desconfía del documento; un modelo obedece*. Un mensaje que despista, despista con confianza.
3. **Qué contexto le dan.** Diez personas van a pegarle diez cosas distintas —la referencia, la
   galería, un ejemplo, nada—. Es un experimento natural y gratuito sobre qué contexto funciona.
   📌 El accidente de julio (pegarle la **bitácora** en vez de la referencia) quedó registrado
   como **el más informativo de los dos**.

### Cómo correrlo

> **Segunda parte: hazla con un asistente**
>
> Ahora sí, usa el modelo que quieras. Pégale lo que creas que necesita para escribir MetaGráfica
> y pídele la figura. Puedes compilar y devolverle los errores las veces que haga falta.
>
> Guarda **la conversación completa**, no solo el archivo final: los intentos fallidos son lo que
> nos sirve.
>
> Dos preguntas al terminar: ¿qué le pegaste de contexto? ¿en qué se equivocó más veces?

### 🔎 El registro propio de este bloque

| Qué contexto le pegó | Comando que el modelo INVENTÓ | Cómo se llama en realidad | ¿Compiló a la 1ª? | Nº de iteraciones |
|---|---|---|---|---|
|  |  |  | sí / no |  |

⚠️ **La columna del comando inventado vale más que la del §4.1**, y por una razón concreta: un
modelo escribe el nombre que le dicta *todo* lenguaje que ha visto. Si inventa `xlabel` o
`axis_label`, eso no es la corazonada de una persona — es **el nombre que el mundo ya usa** para
esa idea. Es el candidato a renombre con el prior más grande que vas a conseguir.

⚠️ **Y el riesgo que hay que vigilar en la sala:** si el bloque degenera en «mirar cómo el agente
la escribe», te quedas sin los dos datos y sin «periodo» —nadie aprendió el lenguaje, nadie vuelve
a escribir una figura sola—. Que la persona siga siendo quien decide y corrige.

### 🚧 Prerrequisito: hoy el asistente es INVISIBLE

`docs/modelfile_llm.txt` existe, está construido con conteo de tokens medido
(`tools/generar_modelfile.py`) y **no se menciona en ninguno de los dos README, ni en la
referencia, ni en la galería; no lo instala `make install` ni viaja en el paquete del release**.
Verificado el 2026-08-04.

Sin resolver eso, cada participante improvisa su propio contexto — que es *interesante* como
experimento (punto 3 de arriba) pero deja al proyecto sin poder recomendar nada. **Decidir antes
del taller** si el asistente es algo que MetaGráfica ofrece o algo que cada quien se arma. Es una
decisión de producto, no de material.

---

## 4. Hoja de conteo

El registro es frágil y vive en la sala. Esto es lo mínimo, y se llena mientras alguien más
atiende.

### 4.1 Fricción con los nombres — el dato principal

| # | Qué quería hacer (EN SUS PALABRAS) | Qué nombre intentó | Cómo se llama | ¿Lo halló en la doc? | Min. |
|---|---|---|---|---|---|
| 1 |  |  |  | sí / no / se rindió |  |
| 2 |  |  |  |  |  |

⚠️ **La columna que más vale es «qué nombre intentó», y es la que se olvida.** Un nombre que
alguien adivina **es un candidato a nombre nuevo**: si tres personas escriben `axis_label`, el
argumento debería llamarse así. Sin esa columna te quedas sabiendo que hubo fricción pero no hacia
dónde renombrar, que es la mitad útil.

La columna «¿lo halló en la doc?» mide otra cosa distinta y también sin vigilancia: `docfail`
comprueba que los ejemplos de la referencia compilen, **no que un humano encuentre lo que busca**.

### 4.2 Mensajes de error

| Mensaje que le salió | ¿Entendió qué hacer? | Qué creyó que significaba |
|---|---|---|
|  | sí / no |  |

📌 Los ~150 caminos de error tienen pruebas negativas que verifican que el mensaje **sale**.
Nadie ha medido nunca si alguien lo **entiende**.

### 4.3 Cierre de cada sesión, en dos preguntas

Una ronda rápida, y se anota literal:

- **«¿Qué fue lo más molesto?»** — respuesta libre, sin sugerir opciones.
- **«¿Volverías a usarlo para tu próxima figura?»** — sí / no / depende de qué.

La segunda es la que predice el «periodo» de la condición 4. Un «depende de que…» es el ítem más
accionable que va a salir del taller entero.

---

## 5. Disciplina del facilitador

Lo más difícil, porque va contra el instinto de ayudar. Va aquí para poder releerlo cinco minutos
antes de empezar.

- 🔇 **No contestes de inmediato una pregunta de nombre.** Anótala, deja pasar ~1 minuto, luego
  ayuda. Contestar en tres segundos destruye el dato **y** corta el forcejeo, que *es* la señal.
- ✍️ **Alguien más de escriba.** Si atiendes y registras a la vez, pierdes el registro. Es el
  papel más valioso de la sala y no requiere saber el lenguaje.
- 🚫 **No enseñes la solución en el proyector.** Ni siquiera «para ir más rápido»: a partir de ese
  momento la sala entera copia y no queda nada que medir.
- 🎯 **Que terminen la figura.** Nadie vuelve a la sesión 2 si se fue con las manos vacías, y la
  sesión 2 es donde está el dato bueno.

> *«El autor no puede hacerse esa prueba solo: ya sabe cómo se llaman las cosas.»*
> — condición 4, `PENDIENTES.md`

---

## 6. Entre las dos sesiones, y después

**Las dos semanas de en medio son parte del instrumento**, no una pausa: la condición 4 pide *«un
periodo de figuras escritas por otras personas»*, y ahí es donde se mide qué se rompe sin ti
delante.

- Un recordatorio a mitad: *«¿pudiste avanzar? Si te atoraste, mándame dónde»*. **Un atasco no
  reportado es evidencia perdida.**
- **Responder en 48 horas** lo que llegue (compromiso de `plan_promocion.md` §6). Aquí pesa más
  que nunca: quien no recibe respuesta no vuelve a la segunda sesión, y esa persona *era* la
  evidencia.

**Sesión 2 — el orden importa:** empieza preguntando **qué se rompió**, no revisando figuras
bonitas. Lo que quieres saber es quién volvió y con qué chocó estando solo.

**Al final, lo que sale del taller:**

1. La hoja 4.1 ordenada por frecuencia = **la lista de renombres**, que es lo que bloquea la
   condición 1 (congelar la gramática). **Crúzala con los comandos que inventaron los modelos**
   (§3-bis): un nombre que coincida en las dos columnas —lo que la gente busca y lo que el modelo
   escribe solo— es un renombre sin discusión posible.
2. Los `.mg` de la gente, con permiso. ⚠️ **Guarda los intentos FALLIDOS, no la figura final**:
   el error es el dato.
3. Un veredicto sobre la condición 4, con la prueba que ya fija §2a del plan: **si nadie preguntó
   «¿cómo se llamaba lo del eje?», está cumplida.** Si preguntaron, ahí está lo que hay que
   renombrar antes de congelar.

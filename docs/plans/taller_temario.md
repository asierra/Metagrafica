# Temario del taller — el reloj de la sesión

> **Qué es esto.** El programa minuto a minuto de la sesión de dos horas de `plan_promocion.md`
> §2a. Es el **reloj**; el **instrumento** —anuncio, boleto de entrada, encargos, hojas de
> conteo, disciplina del facilitador— está en `taller_material.md` y no se repite aquí.
>
> Va aparte a propósito: el material se reparte y se llena, el temario se relee cinco minutos
> antes de empezar y se ajusta según el grupo. Mezclarlos haría que cada retoque del horario
> tocara el documento que se entrega.
>
> Creado 2026-08-27.

---

## 0. La restricción que decide el temario entero

`taller_material.md` §0 impone dos prohibiciones: los encargos no nombran un solo comando, y
**nada de demo-y-copien**. Un temario de introducción escrito como se escriben normalmente las
viola las dos: en cuanto `plot`, `xaxis` o `struct` pasan por el proyector, la evidencia de la
condición 4 vale cero — ya no se puede medir si alguien habría dado con el nombre.

La salida no es enseñar menos, es **enseñar el modelo y no el vocabulario**:

> 🎯 **Se nombra desde la pantalla solo lo que está ARRIBA de `plot`** —el preámbulo y el comando
> de compilación, que son inevitables— y todo lo demás se busca.

Eso deja intactas las zonas de gramática que miden los tres encargos (generadores, estructuras,
expresiones) y a la vez quita de en medio el único concepto que, si no se dice, hace tropezar a
todos con lo mismo — y un tropiezo que comparten los doce no informa de nada.

📌 **Corolario:** nada de acordeón impreso. Una hoja con los nombres de los comandos destruye
exactamente lo que se mide. La referencia abierta en la pantalla de cada quien hace el mismo
trabajo y además pone a prueba la búsqueda, que es lo que ninguna compuerta verifica.

---

## 1. Sesión 1 — dos horas

### 0:00–0:10 · Arranque, no bienvenida

Todos compilan `examples/quickstart.mg` a SVG y lo abren en el navegador. El boleto de entrada ya
lo hizo en casa; esto solo verifica que los doce binarios funcionan en la sala. Quien lo tenga
roto se sienta con un vecino.

⚠️ **La sesión no se gasta instalando.** Para eso existe el boleto: media hora de instalación es
el 25 % de la ventana de observación.

### 0:10–0:25 · Por qué (proyector)

Un solo argumento y una sola demo:

- **La demo:** `tools/clip_parametrico.sh` — el barrido de `xe1` en `franck_condon`. Cambiar un
  número y ver la figura entera reacomodarse. Es el argumento central del proyecto en quince
  segundos y no requiere leer una línea de código.
- **La fidelidad:** son las figuras de los libros de mecánica cuántica de Cetto y de la Peña. La
  galería lo enseña; aquí basta decirlo.
- **El encuadre de beta**, dicho explícitamente: *«están probando la herramienta tanto como
  usándola»*. No es modestia — es lo que hace que la fricción se reporte en vez de que la persona
  crea que la culpa es suya, y una fricción no reportada es evidencia perdida.

⚠️ **No recorrer la referencia aquí.**

### 0:25–0:40 · El modelo (lo único que se nombra)

Cuatro cosas, y ninguna más:

1. `mg archivo.mg salida.svg` — **la extensión elige el backend** (`.eps` / `.svg` / `.pdf`).
2. El preámbulo: `display_size` (centímetros de papel), `world_window` (unidades de tu problema),
   `font_size`. ⚠️ **Que el papel y el mundo sean dos sistemas de coordenadas distintos es el
   único concepto del que cuelga todo lo demás**; es lo que se enseña, y lo que justifica que
   estos tres nombres sí se digan.
3. Una regla de sintaxis: dentro de un bloque `{ }`, **nombre o número desnudos, expresión entre
   paréntesis** — `{ 12 (y-11) }`, porque sin el paréntesis un `-` parte la coordenada en dos.
   Es sintaxis pura, no vocabulario, no mide nada, y ahorra el error más común.
4. **Dónde están las respuestas:** §16 de `docs/referencia.md` (referencia rápida), la galería
   —cada tarjeta trae el fuente completo, copiable— y los mensajes de error del compilador.
   Enseñar la galería 60 segundos, **como buscador**, sin leer ninguna tarjeta.

⚠️ **A partir de aquí el facilitador no nombra un comando en pantalla.** Es el punto de no
retorno del instrumento: todo lo que se mide se mide después de este minuto.

### 0:40–1:40 · La hora de trabajo

Su propia figura; los tres encargos de `taller_material.md` §3 como reserva. Es el 50 % de la
sesión y es *el* instrumento.

- Alguien más llena las hojas 4.1 (fricción con los nombres) y 4.2 (mensajes de error).
- Esperar **~1 minuto** antes de contestar una pregunta de nombre.
- Cada ~20 minutos, una ronda de 60 segundos: *«¿quién está atorado y en qué?»*. ⚠️ **El que se
  atora calla**, y esa es justo la evidencia que más cuesta recuperar después.
- **Corte duro a la 1:25**, avisando que en quince minutos se da la vuelta. Sin el aviso, la
  mitad llega a la vuelta de sala con la figura a medias.

### 1:40–1:55 · Vuelta de sala

60 segundos cada uno, su figura en pantalla. Cumple la disciplina de *«que terminen la figura»*
—que es lo que hace que vuelvan a la sesión 2, donde está el dato bueno— y regala un dato de
paso: los nombres se dicen por fin en voz alta, **dichos por ellos**.

### 1:55–2:00 · Cierre

Las dos preguntas de `taller_material.md` §4.3, anotadas literales. Y qué pasa en las dos
semanas: el recordatorio de en medio y el compromiso de responder en 48 horas.

---

## 2. Tres decisiones que van con este reloj

- **El bloque de agente (`taller_material.md` §3-bis) va a la sesión 2, no a ésta.** No cabe
  junto a la hora de trabajo, y su §3-bis dice que el orden no es negociable: una vez que alguien
  vio el nombre que escribió el modelo, no se puede des-ver. Todo el §4.1 se mide antes o no se
  mide. Además arrastra un prerrequisito abierto —`docs/modelfile_llm.txt` hoy es invisible: no
  está en los README ni en la referencia, no lo instala `make install` ni viaja en el paquete del
  release—, y eso hay que **decidirlo antes del taller**.
- **Un escriba dedicado.** Está en la disciplina del facilitador, pero aquí tiene consecuencia de
  horario: entre 0:40 y 1:40 el facilitador no puede registrar y atender a la vez.
- **12 personas.** Con más no se alcanza a ver a nadie tropezar en esos sesenta minutos, y el
  tropiezo es el dato.

---

## 3. Variante: taller suelto, sin medición

Si lo que se da es **una sola sesión de dos horas** y no la sesión 1 del instrumento —un curso
que pidió una clase, una invitación de una tarde—, el esqueleto es el mismo con dos cambios:

- El bloque del modelo sube a **30 minutos** y ahí sí se nombran los generadores de gráficas, los
  ejes y las estructuras, con la galería recorrida de verdad.
- La hora de trabajo baja a **50 minutos** y el cierre se vuelve «cómo seguir».

⚠️ **Lo que se pierde hay que saberlo antes de decidir:** se gana aprendizaje y se pierde la
condición 4 completa. Sin las dos semanas de por medio no hay «periodo», o sea que no se mide qué
se rompe sin el autor delante — que es la mitad del valor del taller para el proyecto. Es una
elección legítima; lo que no es legítimo es hacer la variante y anotar el resultado como si fuera
evidencia de la condición 4.

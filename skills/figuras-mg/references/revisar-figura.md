# Revisar una figura: el paso que ningún compilador hace

Una figura de `mg` compila limpio en los tres backends y aun así puede estar mal. El caso
documentado: la primera versión de una reconstrucción tuvo **seis defectos**; los seis compilaban
limpio, **cinco se veían a simple vista** en el render y el sexto solo se caza leyendo el texto
que la figura ilustra. Ninguno lo detecta el compilador, y ninguno lo detecta el propio autor si
no se obliga a mirar.

> ⚠️ Esto no lo cubre ninguna de las ocho compuertas de `test/run.sh`. Todas cazan clases de
> fallo —regresión, prólogo, paridad entre backends, rancidez de lo publicado— y **ninguna
> contesta «¿se ve bien?»**, que es la pregunta que ha destapado más defectos con las compuertas
> en verde.

## El procedimiento, en este orden

### 1. Rasterizar y mirar — y decir en voz alta lo que se ve

No «revisé la figura»: **describirla**. «Los rayos exteriores caen en las esquinas de la franja;
ningún rótulo está cruzado; la flecha del sensor apunta al terreno.» Enunciarlo es lo que obliga
a mirarlo de verdad; sin eso, la mirada se desliza por encima de lo que ya se espera encontrar.

Si trabajas en varias vueltas, describe **la zona que tocaste y la que no**: los defectos
aparecen donde no se estaba mirando.

### 2. Contrastar contra la FUENTE, no contra el propio diseño

Una figura internamente consistente puede seguir diciendo algo que la fuente contradice. En el
caso documentado, los rayos exteriores de un abanico caían **dentro** del paralelogramo, lo que
afirma que la franja barrida es más ancha que el campo de visión — coherente consigo mismo y
falso respecto del texto, que dice que el campo de visión proyectado *define* el ancho.

⚠️ Si extraes texto de un PDF para comprobarlo, **las fórmulas no sobreviven a la extracción**
(σ sale como `s`, λ como `l`, los exponentes al ras). Para cualquier ecuación, mira la página.

### 3. Repetir 1 y 2 después de cada corrección

Es donde más se falla: un arreglo mueve el problema en vez de resolverlo, y como ya se «revisó»,
nadie vuelve a ver.

## Lista de comprobación

Sale de defectos reales, no de imaginar qué podría fallar.

**Lo que se ve a simple vista**

- [ ] ¿Hay rótulos encimados, o cruzados por una línea? Un rótulo que roza otro basta para que se
      lean intercambiados. Si el hueco es estrecho, el rótulo va **fuera** con una guía, como
      suelen hacerlo los originales.
- [ ] ¿Las flechas apuntan a donde deben? Una cota de doble punta necesita
      `marker_start_orient="reverse"`; sin eso las dos apuntan al mismo lado.
- [ ] ¿Los elementos que deben coincidir, coinciden **exactamente**? Un rayo que debe terminar en
      un vértice, un arco que debe ser tangente a una recta. Si es «casi», está mal y además está
      puesto a ojo.
- [ ] ¿Un marcador simétrico invade lo que señala? Se ancla por el centro, así que rebasa el
      vértice `marker_size` pt. **Solo se ve ampliando** (regla 11 del `SKILL.md`). Una flecha no
      tiene ese problema: se ancla por la punta.
- [ ] ¿Se salió algo del lienzo? `mg` avisa solo cuando **todo** cae fuera; salirse por un borde
      es a menudo deliberado, así que se calla.
- [ ] ¿La tipografía matemática salió bien? Una ρ que se ve como «ø» es sustitución de fuente del
      visor, no un bug de la figura — rasteriza con un navegador, no con `rsvg-convert`.

**Lo que solo se caza leyendo**

- [ ] ¿La figura afirma lo que la fuente afirma? Identifica **la** frase que la figura ilustra y
      comprueba que el dibujo la sostenga.
- [ ] ¿Los rótulos y el trazo dicen lo mismo? Si un rótulo imprime un ángulo, tiene que ser el
      ángulo que gobernó el trazo, no un número tecleado al lado.
- [ ] ¿Lo que el original insinúa por esquemático, aquí queda **medido**? Los esquemas de libro
      suelen tener la geometría mal a propósito o por descuido. Reconstruir con física es la
      mejora; hay que decirlo en el encabezado.

**Lo que solo se caza leyendo el propio `.mg`**

- [ ] ¿Todo lo que un comentario dice que se calcula, está calculado? Un literal tecleado bajo un
      comentario que promete una fórmula coincide por casualidad hoy y despega mañana.
- [ ] ¿Cambiar el parámetro físico de arriba recalcula la figura entera? Es la prueba de que la
      geometría es derivada y no ajustada a ojo.

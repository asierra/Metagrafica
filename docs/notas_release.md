# Notas de release

> **Qué es este archivo.** Las novedades de la versión que se está por etiquetar.
> Lo lee `.github/workflows/release.yml` y publica **todo lo que va después del marcador
> `<!-- publicar -->`**, arriba del cuerpo genérico de la página de descarga; lo de aquí
> arriba es para quien lo edita y no sale publicado.
>
> Va versionado a propósito: así queda atado al tag que lo publica —no puede describir una
> versión distinta de la que viaja con él— y escribirlo es un acto revisable, no una edición
> a mano en GitHub que nadie vuelve a ver. **Se reescribe entero en cada release**, después
> de subir `include/version.h`. Si el archivo no existe, la página sale solo con el texto
> genérico.

<!-- publicar -->

## Novedades de 3.1.0-beta

Primera versión con **escenas pseudo-3D**, más degradados, cinco funciones nuevas y una
tanda de correcciones.

### Escenas pseudo-3D (§13 de la referencia)

Se describe el **espacio**, no la proyección. Tres piezas:

- **`view3d`** — la cámara, una sola por figura: axonométrica ortográfica
  (`azimuth`/`elevation`) u oblicua (caballera/gabinete).
- **`plane3d`** — dentro del bloque las coordenadas son locales a un plano de la escena,
  y **todo el dibujo 2-D corriente sigue funcionando ahí**: `sine`, rellenos, marcadores,
  arcos, texto. Un `circle` dibujado en un plano sale como la elipse exacta de su
  proyección, sin que nadie calcule un semieje.
- **`xyz(x, y, z)`** — un punto suelto del espacio, para lo que no pertenece a ningún plano.

Cuatro figuras nuevas lo estrenan: `angulo_solido` (esfera reticulada y casquete),
`onda_electromagnetica` (E y B en planos perpendiculares), `irradiancia` (la silueta de un
cono calculada como las tangentes desde un punto al borde de un disco) y `seccion_eficaz`.
Con ellas llegan `lib/pseudo3d.mg` (`cono`, `cilindro`) y los iconos `lib/aircraft.mg` y
`lib/people.mg`.

Es **pseudo**-3D a propósito: no hay motor de superficies ocultas ni iluminación. Qué va
delante de qué se decide en el fuente, en forma cerrada.

### Otras novedades

- **Rellenos degradados** en los tres backends (§4.14), estrenados por `espectro.mg`.
- **`asin`, `acos`, `atan`, `deg` y `rad`** en el evaluador.
- **`\hat` y `\vec`** en el markup matemático.
- **`make install`** reparte ahora también los ejemplos, `lib/` y la documentación legible.
- **`tools/ver.sh`** — compila y rasteriza los tres formatos para MIRAR una figura, y
  compara dos salidas píxel a píxel.

### Correcciones

- **El cuerpo de una `struct` escribía en el ámbito de quien la llamaba.** Incluir una
  biblioteca podía pisar en silencio cualquier variable cuyo nombre ésta reusara. Ahora el
  cuerpo es frontera para las escrituras; las lecturas la siguen cruzando.
- **`sine` se tragaba sus atributos** (color, grosor, trazo) en vez de aplicarlos.
- **Un arco de barrido cero abortaba el PDF entero.** No dibujar nada no es lo mismo que
  fallar.
- **La cara tipográfica se fugaba entre renglones, y solo en PDF.**
- **El EPS metía la ruta absoluta del disco en su `%%Title`**, y Ghostscript la propaga a los
  metadatos `/Title` del PDF: la ruta de quien compilaba viajaba dentro de la figura publicada.
  Ahora `%%Title` lleva el nombre del `.mg` de origen y `%%Creator` la versión, así que el mismo
  `.mg` produce el mismo EPS se compile donde se compile.
- Mensajes de error más precisos para el bloque huérfano y el bloque de coordenadas, y un
  **aviso nuevo** para la figura que compila limpia y sale en blanco.

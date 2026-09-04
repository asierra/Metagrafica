# Plan: gradientes de relleno

> **Estado (2026-07-28):** ✅ **FASES 1 y 2 CERRADAS.** `gradient=`/`gradient_angle` está en los
> tres backends, `examples/espectro.mg` está en el corpus (`ok=75`) y la cuarta invariante de la
> Capa 3 entró en el mismo commit, como pedía §4. La Fase 3 (paradas arbitrarias, radial,
> transparencia) sigue diferida por la regla de demanda. Lo que se decidió y no estaba en la
> versión original de este plan va en §7.

MG no tiene gradientes: ni la referencia ni `especificacion_mg.md` los mencionan, y ningún
backend los emite. Es una ausencia notoria —cualquier sistema de gráficos 2-D los tiene— y hasta
ahora ninguna figura los había pedido.

---

## 1. El cliente

La figura del espectro electromagnético del curso de Percepción Remota (`geo/infra.png`): la
banda principal **es** un gradiente continuo —violeta → azul → verde → amarillo → naranja → rojo
→ negro— y la de microondas otro, de gris claro a gris oscuro.

🔎 **Cómo apareció, que es la parte interesante.** Alejandro le dio la referencia del lenguaje y
la imagen a un modelo, **sin manera de ejecutar `mg`**, y le pidió la figura. El resultado
(`geo/espectro.mg`) compila y se parece bastante, pero aproximó los gradientes con **franjas
planas** — que es lo único que el lenguaje permite. No fue un error del modelo: fue el lenguaje
topándose con su techo, y quedó registrado porque alguien de fuera intentó una figura que el
autor no habría intentado.

**Y no es un caso aislado en ese dominio:** una barra de espectro es *el* gradiente canónico, y
en percepción remota reaparece en cada figura de bandas y en cada barra de color.

---

## 2. Forma propuesta: como `hatch`, porque ya existe esa decisión

MG ya resolvió una vez «un relleno que no es un color plano»: el **tramado**. `hatch` es un
atributo por-primitiva con nombre de estilo y sus parámetros al lado (`hatch_angle`,
`hatch_gap`). El gradiente es el mismo problema, así que debería tener la misma forma:

```octave
rectangle(gradient=["#5A478F", "#0000FF", "#00FF00", "#FFFF00", "#D93622", "black"]) { … }
rectangle(gradient=[gray(0.8), gray(0.2)], gradient_angle=0) { … }
```

- **Lista de colores = paradas repartidas por igual.** Cubre el caso del espectro sin sintaxis
  extra, que es el 90 % del uso.
- **`gradient_angle`** en grados, como `hatch_angle`. Default 0 (izquierda→derecha).
- ~~**El eje se define sobre la caja de la primitiva**, normalizado — lo mismo que hace SVG por
  default (`objectBoundingBox`).~~ **REVERTIDO al implementar (ver §7.1): el eje vive en el marco
  de la PÁGINA.**

**Decisiones diferidas a propósito**, por la regla de demanda: paradas en posiciones arbitrarias
(`gradient_stops=[0, 0.3, 1]`), gradientes **radiales**, y transparencia. Ninguna la pide la
figura del espectro. La radial además no es gratis (§3).

✅ **Interacción con lo que ya hay — RESUELTA.** `gradient` y `hatch` son la misma ranura, así que
**entre registros gana el último** (un `gradient=` por-primitiva pisa un `hatch` ambiente y se
restaura al salir, como cualquier atributo de §7.5) y **juntos en la misma primitiva es error**:
ahí no hay orden que interpretar, y descartar uno en silencio es el destino que §7.5 existe para
evitar. `color=` junto a un `gradient=` contornea, por la misma regla que ya valía para `fill=` —
que es justo lo que la banda del espectro necesita.

---

## 3. Viabilidad por backend — VERIFICADA, y no es simétrica

| backend | gradiente lineal | radial | notas |
|---|---|---|---|
| **SVG** | nativo | nativo | `<linearGradient>` en `<defs>` + `fill="url(#id)"`. Trivial. |
| **PDF** | **sí, por malla** | **no** | ver abajo — y ⚠️ §7.2, el archivo que faltaba |
| **EPS** | `shfill` tipo 2 | `shfill` tipo 3 | PostScript **nivel 3** |

🔎 **La limitación que condiciona el diseño: libharu solo implementa el sombreado tipo 4**
(`HPDF_SHADING_FREE_FORM_TRIANGLE_MESH`, `hpdf.h:1546` — el propio header dice «the only defined
option»). **No expone los tipos 2 (axial) ni 3 (radial)** del estándar PDF.

- **Lineal: se puede igual**, y sin parchear la biblioteca vendorizada. Un gradiente axial sobre
  un rectángulo es exactamente una malla de dos triángulos con los vértices coloreados (Gouraud):
  A,A en un borde y B,B en el otro. Con varias paradas, un triángulo por tramo.
- **Radial: no sale de ahí.** Habría que aproximarlo con un abanico de triángulos o con anillos
  concéntricos. Es la razón técnica —no estética— por la que este plan **difiere la radial**.

⚠️ **EPS exige nivel 3.** Ghostscript lo interpreta sin problema (la compuerta `psfail` lo
verificaría), pero un RIP viejo o un flujo que pase por Distiller antiguo podría no. Alternativa
de reserva: **franjas finas**, que es lo que hace hoy el `.mg` del modelo a mano y lo que el
lenguaje puede expresar sin motor. Decidir si se emite `shfill` o franjas, o si se ofrece un
interruptor.

---

## 4. La compuerta que hay que añadir CON la primera línea de código

**Un gradiente es justo la clase de cosa que un backend puede omitir en silencio.** Si SVG lo
dibuja y PDF sale plano, el golden lo bendice —cada uno es byte-estable— y nadie se entera: es
literalmente el escenario para el que existe la Capa 3 (paridad entre backends), y hoy sus tres
invariantes miran **texto**, **líneas rellenas** y **geometría de arcos**. Ninguna mira rellenos.

📌 **Por tanto: la cuarta invariante de la Capa 3 —contar los rellenos degradados en los tres
formatos y exigir que coincidan— va en el mismo commit que la característica, no después.** Es
la lección de la sesión del 2026-07-27, donde la única compuerta sin escapatoria por bendición
fue la que compara backend contra backend.

---

## 5. Fases

### Fase 1 — lineal, paradas equiespaciadas, en los tres backends
Con `gradient=[…]` y `gradient_angle=`. Criterio de aceptación: **reproducir la banda del
espectro de `geo/infra.png`**, que es el cliente que abrió el plan.

### Fase 2 — la figura entra al corpus
Con encabezado a la convención de 2026-07-23; gana goldens, `docs/img` y galería. Y **sirve de
ejemplo del constructo**, que hoy no tendría ninguno.

### Fase 3 — diferidas hasta que una figura las pida
Paradas en posiciones arbitrarias; gradiente radial (con la nota de libharu de §3);
transparencia.

---

## 6. Lo que este plan NO propone

- **No** un sistema de color nuevo. Los colores de las paradas son los que ya entiende `fill`
  (nombres CSS, `#rrggbb`, `gray(g)`).
- **No** gradientes en líneas ni en texto. Solo relleno de área, que es lo que la figura pide.
- **No** parchear libharu. Si la malla de triángulos resulta insuficiente para algo, se anota
  aquí antes de tocar `third_party/`.


---

## 7. Lo que se aprendió AL IMPLEMENTAR (2026-07-28)

Tres cosas que este plan no podía saber antes de escribir el código. Se anotan aquí porque las
tres son decisiones, no detalles.

### 7.1 El eje va en el marco de la PÁGINA, no en el de la figura

§2 proponía `objectBoundingBox` («el gradiente acompaña a la forma»). Se descartó por dos razones
que aparecieron al ir a implementarlo:

1. **Hay precedente y decía lo contrario.** `hatch_angle` ya barre su familia de líneas sobre el
   bbox de DISPOSITIVO: un rectángulo girado 30° con `hatch=45` sale a 45° **en el papel**, no
   respecto de la forma. Se verificó antes de decidir (los dos rectángulos, girado y no, comparten
   un solo `patternTransform="rotate(45)"` en el SVG). Tramado y degradado son las dos maneras de
   rellenar un área con algo que no es un color plano; que se orientaran en marcos distintos sería
   una incoherencia gratuita.
2. **`objectBoundingBox` SESGA el ángulo.** Mapea la caja de la forma al cuadrado unidad, así que
   en una caja 4:1 un gradiente «a 45°» sale a ~76° en la página. Es exactamente la familia de
   bugs de `plan_anisotropia.md` —fórmula isótropa aplicada al caso anisótropo—, y además obligaría
   a EPS y PDF, que no tienen ese modo, a reproducir el sesgo a mano para no discrepar.

Consecuencia práctica: **los tres backends consumen el MISMO eje**, `Display::gradientAxis`, y por
eso coinciden por construcción en vez de por vigilancia. En SVG eso obliga a
`gradientUnits="userSpaceOnUse"` con coordenadas calculadas.

### 7.2 Nuestro propio recorte se había comido `hpdf_shading.c`

> **Corregido el 2026-09-03.** Esta sección se tituló «La copia vendorizada de libharu estaba
> INCOMPLETA» y decía que el archivo «no se había vendorizado». **Es falso**, y la historia del
> archivo lo desmiente en tres commits. Se deja el título viejo aquí anotado porque la versión
> equivocada llegó a citarse en `CLAUDE.md` y en la bitácora.

§3 daba por hecho que el tipo 4 estaba disponible porque el header lo declara. No lo estaba —pero
la culpa no era de upstream, era nuestra:

| Commit | Fecha | Qué pasó |
|---|---|---|
| `ff385e1` | 2026-06-29 | `hpdf_shading.c` **entra** con la copia vendorizada, completo |
| `e630e08` | 2026-07-04 | el recorte de `plan_pdf.md` lo **borra**, junto a los encoders CJK |
| `1c791b0` | 2026-07-28 | se **restaura** de upstream, byte-idéntico al de `ff385e1` (`cmp`) |

O sea que el archivo estuvo siempre en upstream y volvió exactamente igual a como se había ido.
Lo que dejó el árbol incoherente con su propio `hpdf.h` fue el recorte, que se llevó
`HPDF_Shading_New`/`HPDF_Shading_AddVertexRGB` y dejó compilada la mitad consumidora
(`HPDF_Page_SetShading`, `HPDF_Page_GetShadingName`, el dict `/Shading` de `hpdf_pages.c`).

⚠️ **La lección no es «upstream viene incompleto» sino que recortar una dependencia vendorizada
no tiene compuerta.** El criterio del recorte fue «API que **mg** no usa» —cierto entonces— y le
faltaba cerrar el conjunto bajo las llamadas **internas de la propia biblioteca**. Nada verifica
eso: las diez compuertas miran la salida del compilador, la documentación y el árbol, y ninguna
puede ver que un `.c` que se queda llame a un `.c` que se fue, porque el enlazado no falla
mientras nadie ejercite ese camino. Tardó **24 días** en destaparse, y solo porque los degradados
fueron la primera característica que lo pisa. El aviso vive ahora en `plan_pdf.md`, junto a la
tabla que lo causó.

Se restauró el archivo de upstream v2.4.6 **tal cual** —misma licencia ZLIB, sin editar una línea,
y el `wildcard` del Makefile lo toma solo—. La política de §6 sigue en pie: eso no es parchear
libharu, es restaurar lo que nunca debió salir; si falta algo más, se restaura de upstream y se
anota aquí.

La limitación REAL de libharu es la que decía §3 y se confirma: no expone los tipos 2 (axial) ni
3 (radial). Para el lineal da igual —un tramo entre dos paradas es exactamente un cuadrilátero con
interpolación de Gouraud— y por eso el radial sigue diferido por razón técnica, no estética.

### 7.3 Las coordenadas del tipo 4 ENVUELVEN, no se recortan

El sombreado tipo 4 codifica cada coordenada como entero contra el rango de `/Decode`. Una
coordenada **fuera** de ese rango no se recorta: **envuelve**. Los cuadriláteros que reproducen el
`/Extend [true true]` del EPS —que por definición se salen de la forma— reaparecían por el otro
lado, encima de la figura, y la pintaban plana del color del extremo: el degradado entero se veía
de un solo color, con el mesh perfectamente correcto.

La corrección es declarar el bbox del sombreado sobre los **vértices ya construidos**, no sobre el
bbox del path. Vale la pena tenerlo presente para cualquier otro uso futuro del tipo 4.

⚠️ **Ninguna compuerta lo habría cazado por su cuenta**, y conviene saber por qué: la invariante (d)
cuenta operaciones de sombreado, y aquí había exactamente una por figura en los tres formatos. El
PDF era byte-estable, así que el golden lo bendecía. Se encontró **mirando el render**, que sigue
siendo la única prueba de que una figura se ve como debe.

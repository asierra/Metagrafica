# Plan: gradientes de relleno

> **Estado (2026-07-28):** abierto, con cliente. Sin código todavía — este documento existe
> para decidir la forma **antes** de escribirla, y porque el backend PDF tiene una limitación
> que condiciona el diseño (§3).

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
rectangle(gradient=["gray80", "gray20"], gradient_angle=0) { … }
```

- **Lista de colores = paradas repartidas por igual.** Cubre el caso del espectro sin sintaxis
  extra, que es el 90 % del uso.
- **`gradient_angle`** en grados, como `hatch_angle`. Default 0 (izquierda→derecha).
- **El eje se define sobre la caja de la primitiva**, normalizado — lo mismo que hace SVG por
  default (`objectBoundingBox`). Es lo predecible: el gradiente acompaña a la forma cuando esta
  se mueve o se escala, sin que haya que recolocarlo.

**Decisiones diferidas a propósito**, por la regla de demanda: paradas en posiciones arbitrarias
(`gradient_stops=[0, 0.3, 1]`), gradientes **radiales**, y transparencia. Ninguna la pide la
figura del espectro. La radial además no es gratis (§3).

⚠️ **Interacción con lo que ya hay, a decidir antes de implementar:** `gradient` y `hatch` son
mutuamente excluyentes (¿error, o gana el último?), y hay que definir qué hace `outlinefill` con
un relleno degradado.

---

## 3. Viabilidad por backend — VERIFICADA, y no es simétrica

| backend | gradiente lineal | radial | notas |
|---|---|---|---|
| **SVG** | nativo | nativo | `<linearGradient>` en `<defs>` + `fill="url(#id)"`. Trivial. |
| **PDF** | **sí, pero por malla** | **no** | ver abajo |
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

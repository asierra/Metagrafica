# Plan corregido: recorte de libharu vendorizada

Revisión del plan original (`plan_pdf.pdf`, generado con Gemini) contrastada con el
árbol real del repositorio. La biblioteca vendorizada es **libharu 2.4.6** en
`third_party/libharu/` y todas las afirmaciones de este documento están verificadas
empíricamente: análisis de clausura de símbolos (`nm`) sobre los objetos compilados,
compilación completa y generación de un PDF de prueba con `examples/primitives.mg`.

## Resumen de correcciones al plan original

| Afirmación del plan original | Realidad verificada |
|---|---|
| Borrar `hpdf_image_png.c` y `hpdf_image_jpeg.c` | **No existen** en este árbol. libharu 2.4.6 no los tiene; el soporte JPEG vive dentro de `hpdf_image.c` y no necesita libjpeg (incrusta el JPEG tal cual con DCTDecode). |
| Compilar con `-DLIBHPDF_HAVE_NOPNGLIB`, `-DLIBHPDF_HAVE_NOJPEGLIB`, `-DHPDF_NOPNGLIB` | **Macros inexistentes en 2.4.x.** La configuración va por `include/hpdf_config.h`, que ya está correcto: `LIBHPDF_HAVE_ZLIB` definido y `LIBHPDF_HAVE_LIBPNG` sin definir. No hay que pasar ninguna macro. |
| `-D_CRT_SECURE_NO_WARNINGS` | Solo tiene sentido en MSVC/Windows. Irrelevante aquí. |
| Listar a mano los `.c` supervivientes en el Makefile | Innecesario: el Makefile real ya usa `$(wildcard $(HARUDIR)/src/*.c)`, así que borrar un archivo lo saca de la compilación automáticamente. |
| Borrar `hpdf_memory.c` de la lista de supervivientes | No existe; el gestor de memoria es `hpdf_mmgr.c`. |
| "Puedes eliminar de forma segura" annotation, outline, u3d, encrypt, image, pdfa, ext_gstate… | **Falso.** Todos esos módulos están referenciados por `hpdf_doc.c`, `hpdf_pages.c` u otros archivos del núcleo (ver sección 2). Borrarlos rompe el enlazado salvo que se parchee el código vendorizado. |
| Makefile de ejemplo con `g++`, `-std=c++11`, `src/svg_backend.cpp`, binario `metagrafica` | No corresponde a este proyecto: aquí es `clang++`, `-std=c++14`, `-fno-rtti -fno-exceptions`, objetos en `obj/`, binario `bin/mg`. |
| LTO (`-flto`) para eliminar código muerto | Problemático tal como está el Makefile: los `.c` de haru se compilan con `cc` (gcc) y el enlace lo hace `clang++`; los formatos de LTO de gcc y clang no se mezclan. `--gc-sections` da el mismo beneficio sin ese conflicto (medido abajo). |

## 1. Borrado físico seguro (verificado)

Estos **10 archivos** no son referenciados por ningún otro objeto (ni de `mg` ni del
resto de libharu). Se pueden borrar sin tocar ni una línea de código ni el Makefile:

```bash
cd third_party/libharu/src
rm hpdf_encoder_cns.c hpdf_encoder_cnt.c hpdf_encoder_jp.c hpdf_encoder_kr.c \
   hpdf_encoder_utf.c \
   hpdf_fontdef_cns.c hpdf_fontdef_cnt.c hpdf_fontdef_jp.c hpdf_fontdef_kr.c \
   hpdf_shading.c
```

| Archivo(s) | Qué son | Fuente |
|---|---|---|
| `hpdf_encoder_{cns,cnt,jp,kr}.c` | Tablas de codificación CJK (chino simplificado/tradicional, japonés, coreano) | ~2,1 MB |
| `hpdf_fontdef_{cns,cnt,jp,kr}.c` | Métricas de fuentes CJK predefinidas | ~110 KB |
| `hpdf_encoder_utf.c` | Codificadores UTF activables con `HPDF_UseUTFEncodings` (mg no lo llama) | ~9 KB |
| `hpdf_shading.c` | Degradados (`HPDF_Shading_*`, API que mg no usa) | ~10 KB |

Resultado medido: la fuente de libharu pasa de 3,2 MB a ~1,0 MB y el segmento de
código del binario baja de **1005 KB a 578 KB (−42 %)**. `mg` compila sin warnings
nuevos y el PDF generado desde `examples/primitives.mg` es válido (verificado con
`file` y `pdftotext`).

No hay cabecera que borrar para estos módulos: sus prototipos viven en
`hpdf_encoder.h` y `hpdf_fontdef.h`, que sí se usan. (`include/t4.h` va con
`hpdf_image_ccitt.c`, que **no** se borra —ver sección 2—, así que también se queda.)

## 2. Lo que NO se puede borrar (y el plan original decía que sí)

El diseño de libharu hace que `HPDF_Doc` y `HPDF_Page` sean "objetos dios":
`hpdf_doc.c` y `hpdf_pages.c` llaman directamente a casi todos los módulos, aunque
`mg` jamás ejecute esas rutas. Dependencias medidas con `nm` (símbolos que el módulo
define y otro objeto necesita):

| Módulo | Referenciado por |
|---|---|
| `hpdf_annotation.c` | `hpdf_pages.c` (8 símbolos) |
| `hpdf_outline.c` | `hpdf_doc.c` |
| `hpdf_u3d.c`, `hpdf_3dmeasure.c` | `hpdf_pages.c` |
| `hpdf_encrypt.c` | Núcleo de escritura: `hpdf_xref.c`, `hpdf_streams.c`, `hpdf_dict.c`, `hpdf_string.c`, `hpdf_binary.c`, `hpdf_pdfa.c` |
| `hpdf_encryptdict.c` | `hpdf_doc.c` |
| `hpdf_pdfa.c` | `hpdf_doc.c` |
| `hpdf_ext_gstate.c` | `hpdf_doc.c` y `hpdf_page_operator.c` |
| `hpdf_image.c`, `hpdf_image_ccitt.c` | `hpdf_doc.c` |
| `hpdf_exdata.c`, `hpdf_page_label.c`, `hpdf_namedict.c`, `hpdf_destination.c` | `hpdf_doc.c` / `hpdf_pages.c` |
| `hpdf_font_type1.c`, `hpdf_fontdef_type1.c` | `hpdf_fontdef_base14.c` depende de `fontdef_type1` (las 14 fuentes base **son** Type1), y `hpdf_doc.c` de ambos |

Borrarlos exigiría cirugía dentro de `hpdf_doc.c`/`hpdf_pages.c` (eliminar los puntos
de entrada `HPDF_SetPassword`, `HPDF_Page_CreateTextAnnot`, `HPDF_LoadPngImage…`,
etc.). **No se recomienda**: parchear código vendorizado complica cualquier
actualización futura de libharu, y el mismo resultado en tamaño de binario se obtiene
gratis con el enlazador (sección 3).

## 3. Eliminación del código muerto restante: `--gc-sections`

En lugar de amputar libharu, se compila cada función en su propia sección y se deja
que el enlazador descarte las que nadie alcanza. Esto sí elimina del binario las
anotaciones, encriptación, U3D, imágenes, etc., aunque sus `.c` sigan compilándose.

Cambios al `Makefile` real del proyecto (no al ficticio del plan original):

```makefile
CXXFLAGS = -g -std=c++14 -ffunction-sections -fdata-sections
LDFLAGS  = -g -Wpedantic -Wl,--gc-sections

# La regla de los .c de haru tiene los flags en duro y no hereda CPPFLAGS:
# hay que añadírselos ahí también, o no se gana casi nada.
HARU_CFLAGS = -O2 -ffunction-sections -fdata-sections -I$(HARUDIR)/include

$(OBJDIR)/haru/%.o: $(HARUDIR)/src/%.c | $(OBJDIR)/haru
	$(CC) -c $(HARU_CFLAGS) $< -o $@
```

**Detalle importante medido:** aplicar `--gc-sections` solo al C++ apenas aporta
(578 → 570 KB), porque la regla actual de los `.c` de haru (`$(CC) -c -O2 …`) no
recibe `-ffunction-sections`. Con los flags también en esa regla el código baja a
**460 KB**.

Sobre LTO: si se quisiera, habría que unificar el toolchain (compilar los `.c` con
`clang` y enlazar con `clang++ -flto`), porque hoy conviven `cc`(gcc) y `clang++`.
Dado que `--gc-sections` ya captura casi todo el beneficio, no merece la pena.

## 4. Resultados medidos

Segmento de código (`size`, columna `text`) del binario `bin/mg`:

| Configuración | text | Reducción |
|---|---:|---:|
| Estado actual | 1 005 286 B | — |
| + borrado de los 10 archivos huérfanos | 578 198 B | −42 % |
| + `-ffunction-sections -fdata-sections` y `-Wl,--gc-sections` (C y C++) | 460 082 B | −54 % |

En todas las configuraciones `mg` genera PDFs válidos con texto extraíble correcto
(ver la verificación ampliada en §5, no solo `primitives.mg`).

**Nota sobre los números:** son las cifras medidas cuando se escribió el plan. El
árbol ha crecido desde entonces (p.ej. el embebido de la TTF matemática), así que
los valores absolutos serán algo mayores —una re-medición dio ~596 KB tras el
borrado— pero las proporciones (−42 % / −54 %) se mantienen. Que no cuadre 578 KB
al dígito no indica que algo haya fallado.

## 5. Pasos de ejecución

**Precondición: satisfecha (2026-07-04).** El riesgo que bloqueaba el recorte era
`hpdf_encoder_utf.c`: se conservaría si `PDFDisplay` acabara mostrando texto UTF-8
con la fuente TrueType (`HPDF_UseUTFEncodings` + encoder `UTF-8`). **Eso no ocurrió.**
El `PDFDisplay` terminado usa `WinAnsiEncoding` para todas las fuentes —incluida la
CMMI embebida con `HPDF_LoadTTFontFromMemory`— y `Symbol` para los símbolos; no
llama `HPDF_UseUTFEncodings` ni usa un encoder `UTF-8`. Comprobado empíricamente:
enlazar `mg` sin los 10 objetos no deja símbolos indefinidos, y el binario resultante
genera PDFs válidos en todos los ejemplos (incluidos fig2-3/fig6-1, que embeben la
TTF matemática, y fill_styles con patrones). Los 10 archivos son borrables.

**Orden de riesgo:** el paso 1 (borrado de fuentes vendorizadas) es la parte
destructiva —aunque git conserva el historial— y es la ya verificada arriba. Los
pasos 2–3 (`--gc-sections`) son solo cambios de flags, reversibles con `make clean`.

1. Borrar los 10 archivos de la sección 1. El `wildcard` del Makefile los saca de la
   compilación automáticamente.
2. Añadir `-ffunction-sections -fdata-sections` a `CXXFLAGS`, crear `HARU_CFLAGS`
   con los mismos flags para la regla de `obj/haru/%.o`, y añadir
   `-Wl,--gc-sections` a `LDFLAGS`.
3. `make clean && make`.
4. No tocar `hpdf_config.h` ni añadir macros `-D`: la configuración vendorizada ya
   es la correcta (zlib sí, libpng no).

**Verificación obligatoria (no basta `primitives.mg`).** `primitives.mg` no ejercita
la ruta de embebido de fuente, que es justo lo que el borrado pone en riesgo. Tras
recompilar, generar **todos** los ejemplos con salida `.pdf` y comprobar, para cada
uno, que la salida no está vacía y que `pdftotext` devuelve el texto esperado —no
solo que el archivo exista. Casos imprescindibles:

- `fig2-3.mg` y `fig6-1.mg` → TTF matemática embebida + Symbol (la ruta riesgosa);
  buscar en `pdftotext` la etiqueta rotada "heat capacity" y los subíndices (ψ, x₁′…).
- `fill_styles.mg` → patrones de relleno.
- El resto (`primitives`, `line_patterns`, `rpstest`, `bzsinepaths*`) sin errores de
  libharu en stderr.

Si algún ejemplo falla o le falta texto, restaurar los archivos borrados
(`git checkout -- third_party/libharu/src/`) y revisar cuál módulo hacía falta.

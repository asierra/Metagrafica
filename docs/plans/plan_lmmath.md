# Plan: Latin Modern Math como fuente math única (V3)

Adopción de **Latin Modern Math** (LM Math) como la fuente Computer Modern **Unicode**
única para el modo matemático en los tres backends (EPS/SVG/PDF), para uniformidad de
glifo real. Reemplaza el mosaico actual: cmmi (Type42 BaKoMa en EPS / TTF en PDF) para el
griego + Symbol para los operadores.

Contexto y decisiones: ver la memoria `project_math_fonts.md`. Resumen: math **ligero**
(griego + símbolos + sub/superíndice, sin motor de layout); LM Math elegida sobre CMU Serif
(cmunui es CFF y carece de varphi/varepsilon) y sobre fuentes modernas con tabla MATH (no
la usa ningún backend). Habilitador clave: se **restauró `hpdf_encoder_utf.c`** (el recorte
de libharu lo había borrado), que abre la ruta Unicode del PDF (`HPDF_UseUTFEncodings` +
TTF con cmap Unicode → CID TrueType Identity-H).

## Método probado (2026-07-10)

1. `latinmodern-math.otf` (en el sistema, `/usr/share/texmf/.../lm-math/`, o CTAN `lm-math`)
   es **CFF** → convertir OTF→TTF con `fontforge -lang=py -c "...generate('x.ttf')"`
   (o fonttools cu2qu). Da TTF/glyf que libharu **sí** carga.
2. **Subset agresivo** con `fonttools.subset` a solo los codepoints usados. Para el griego +
   hbar de `map_tex_cmmi` (41 codepoints): **8 KB**, todas las variantes presentes y
   correctas (phi recta U+03D5 / varphi lazo U+03C6, epsilon lunate U+03F5 / varepsilon
   script U+03B5, vartheta, varpi, varrho, varsigma). Verificado con espécimen.
3. Vendorizar el subset TTF como header C (`include/font_lmmath_ttf.h`, array + len) y
   cargarlo desde memoria (`HPDF_LoadTTFontFromMemory` en PDF; Type42 en EPS; `@font-face`
   base64 en SVG).
4. Mapear byte del proyecto → **Unicode** (tabla única, Plan C) y rendir por Unicode.

## Etapas

- **0. Preparar fuente** — ✅ HECHO: subset griego+hbar (8 KB, `include/font_lmmath_ttf.h`),
  espécimen revisado.
- **1. Tabla única byte→Unicode** — ✅ HECHO: `kNameToUnicode` + `makeUnicodeMap` +
  `cmmiUnicode()`/`symbolUnicode()` movidos a `src/text_parser.cpp`, declarados en
  `text_parser.h`; SVG usa las compartidas (refactor puro).
- **2. PDF** — ✅ HECHO: `HPDF_UseUTFEncodings` + LM Math subset desde memoria; griego
  `FN_TEX_CMMI`→LM Math por Unicode (verificado visual: phi/varphi/eps/vareps/α/Ω correctos,
  CID TrueType embebido). Eliminados `cmmi_to_sym` y `font_cmmi_ttf.h`. Latino math→Times-Italic.
- **3. EPS** — ✅ HECHO: `include/font_lmmath_eps.h` = Type42 de LM Math con `/Encoding`
  byte-cmmi→glifo (construido por Unicode; resuelve el naming confuso de LM Math: byte 193
  =φ recta=U+03D5=glifo `phi1`, byte 39=varphi=U+03C6=glifo `phi`). La emisión `(byte) show`
  del EPS no cambió; `setFontFace` usa `/LMMath`. `font_cmmi.h` queda (traductor V1), sin uso
  en el motor V3. Verificado visual: griego CM, coincide con PDF/SVG.
- **4. SVG** — ✅ HECHO: `@font-face` `'MGMath'` (LM Math base64, emisión perezosa con
  `ensureMathFont()`); griego `FN_TEX_CMMI`→MGMath recto, latino/símbolos sin cambios.
  Verificado visual (inkscape): griego CM, coincide con PDF. Self-contained.
- **5. Métricas** — ✅ HECHO: `cmmi_metrics_map` (`src/text.cpp`) regenerado con los anchos
  de avance de LM Math (/1000em, del hmtx del subset) para los 41 bytes griegos; corrige
  descuadres (Omega byte 172 faltaba → ancho 0; mayúsculas apretadas). Verificado: el
  espaciado del griego en SVG coincide con PDF. Latino cmmi sin cambio (ver P2).

## CERRADO 2026-07-20 (P1 y P2)

### P1. Migrar los símbolos (`map_symbol`) de Symbol → LM Math — ✅ HECHO 2026-07-20

Hoy el griego va por `FN_TEX_CMMI` (→ LM Math tras etapa 2) pero los **operadores,
relaciones, flechas y demás símbolos** de `map_symbol` (`int` ∫, `sum` ∑, `prod` ∏, `leq`
≤, `geq` ≥, `rightarrow` →, `infty` ∞, `partial` ∂, `nabla` ∇, `pm` ±, `times` ×, `cdot` ⋅,
`in` ∈, `subset` ⊂, `forall` ∀, `exists` ∃, etc.) salen del font **Symbol** (base-14).
LM Math los tiene todos al Unicode estándar, así que la migración es mecánica con el mismo
método:

1. Extender el **subset** con los codepoints Unicode de `map_symbol` (ya hay `kNameToUnicode`
   con casi todos: aleph 0x2135, partial 0x2202, infty 0x221E, int 0x222B, sum 0x2211,
   prod 0x220F, leq 0x2264, geq 0x2265, rightarrow 0x2192, etc.). Re-subsetear es 1 comando.
2. Extender la **tabla byte→Unicode** para la ruta `FN_SYMBOL` (`symbolUnicode()`).
3. Rutar `FN_SYMBOL` por LM Math (UTF en PDF, Unicode en SVG —ya—, Type42 en EPS) en vez de
   Symbol/`map_symbol`.
4. Verificar cobertura: algún símbolo de `map_symbol` podría faltar en LM Math o estar en
   otro codepoint (revisar con el mismo chequeo `getBestCmap`); ajustar `kNameToUnicode`.

Al cerrarlo, **Symbol desaparece** de los tres backends y la uniformidad es total.

### P2. Latino en modo matemático → itálica de LM Math — ✅ HECHO 2026-07-20

En `$…$` las letras latinas (`x`, `y`) hoy caen a **Times-Italic** (aprox. de math italic),
no a cmmi/LM Math. LM Math tiene la itálica matemática en el plano alfanumérico
(U+1D44E `a` … U+1D467 `z`, U+1D434 `A` …). Para consistencia total: mapear ASCII latino en
`FN_TEX_CMMI` → ese plano y rendir con LM Math. Requiere ampliar el subset con ~52 glifos y
la tabla. Menor prioridad (Times-Italic es una aproximación aceptable).

## Cierre de P1 y P2 (2026-07-20)

Subset único de **162 codepoints** (41 griego + 52 latino italico + 69 simbolos),
sin MATH/GSUB/GPOS. **El font Symbol desaparecio** de los tres backends.

- **EPS necesita DOS fuentes logicas** sobre el mismo Type42: 30 bytes colisionan
  entre `map_symbol` y `map_tex_cmmi`, o sea que el byte solo no identifica al
  glifo — lo identifica el par (byte, cara). `/LMMath` (griego + latino) y
  `/LMMathSym`, derivada con `dup length dict copy` para no duplicar sfnts.
  Griego y latino NO colisionan, por eso comparten encoding.
- **El default de `$…$` vuelve a FN_TEX_CMMI.** Habia pasado a FN_TIMES_ITALIC el
  2026-07-11 porque el latino no estaba en LM Math y un run mixto mandaba el byte
  griego a Times (= ¢). Con P2 esa razon desaparece.
- ⚠️ **Hueco de Unicode:** la `h` italica NO esta en U+1D455 (sin asignar); vive en
  **U+210E**, el signo de la constante de Planck. Unico caso de los 52.
- ⚠️ **El plano matematico esta FUERA del BMP** → hizo falta UTF-8 de 4 bytes. Los
  encoders de PDF y de SVG asumian 3 y desbordaban: `0xE0 | (cp>>12)` daba 0xFD
  para U+1D438, y el SVG dejaba de ser UTF-8 valido. De paso, el de PDF emitia
  secuencias OVERLONG para el griego (que cabe en 2 bytes).
- **SVG pasa a `<tspan>` por segmento.** Antes decidia la fuente por RUN
  (`isCmmiGreekRun`, todo o nada), lo que valia porque los runs eran homogeneos.
  Con P2 un run mezcla ("E = mc": letras si, " = " no). Se usa tspan porque fluye
  solo — un `<text>` por segmento obligaria a calcular anchos. El invariante de
  texto de la Capa 3 pasa a contar **tspans**, y por OCURRENCIAS, no por lineas.
- ⚠️ `ensureMathFont()` ESCRIBE al archivo, asi que va **antes** de abrir el
  `<text>`: llamarlo dentro del lazo metia el `<style>` en mitad del elemento y el
  primer rotulo salia como "@fo…". Lo cazo solo la inspeccion visual — ni el
  golden, ni `gs`, ni la Capa 3, ni la validacion UTF-8 lo veian.

### P3 (mismo dia): digitos, operadores y puntuacion del modo math

Cerrado tambien el 2026-07-20. Los digitos y el `=` seguian en el serif del
sistema, o sea la misma mezcla que P2 vino a quitar, corrida a los otros
caracteres. Entran a `cmmiUnicode()` digitos (RECTOS, como en TeX), operadores,
agrupadores, puntuacion y el espacio; el subset sube a 186 codepoints.

- ⚠️ **El `-` de una formula NO es el guion U+002D**: es el SIGNO MENOS U+2212,
  mas largo y a la altura de la barra del `+`. Es la diferencia entre que `x-1`
  parezca partido y `x−1` se lea como una resta. Solo aplica en modo math.
- 🐞 De paso se arreglo el **espacio de ancho CERO** en `cmmi_metrics_map[32]`:
  el avance de `cur_x` lo toma la tabla de la CARA, no la del font que dibuja,
  asi que un espacio dentro de `$…$` no avanzaba. EPS no lo notaba (usa
  `stringwidth` del propio font); SVG y PDF colocaban mal lo que viniera detras.
- ⚠️ **Literal de cadena > 65536**: el Type42 ya no cabe en un literal, que es el
  maximo que el estandar OBLIGA a soportar. Los literales ADYACENTES no valen —
  C++ los concatena en uno solo y el limite aplica al total—; hay que guardar
  trozos y unirlos en tiempo de ejecucion.

### Alias PUA para PDF (2026-07-20, mismo dia)

🐞 **En PDF faltaban las letras latinas de las formulas** —quickstart, fig6-4,
turning_points— mientras griegas y digitos salian bien. EPS y SVG, correctos.

Causa: **libharu no puede con el plano suplementario.** `HPDF_UNICODE` es
`HPDF_UINT16` y su decodificador UTF-8 hace `if (val > 65535) val = 32;`
("Convert everything outside UCS-2 to space", `hpdf_encoder_utf.c`); ademas su
rama de 4 bytes olvida enmascarar el segundo con `0x3f`, asi que para U+1D434
devuelve 0x9D434. Es limitacion de diseno, no descuido: el tipo es de 16 bits.
El latino de math (U+1D434..U+1D467) es lo UNICO fuera del BMP — de ahi que el
griego (2 bytes) y los digitos (1) no se enteraran.

Solucion **por nuestro lado**, sin tocar codigo vendorizado: el subset lleva
ALIAS en la zona de uso privado (U+E000+n) apuntando a los mismos glifos, y
`PDFDisplay` traduce al pedirlos. SVG sigue emitiendo los codepoints reales y EPS
va por nombre de glifo, asi que ninguno se entera.

⚠️ **Los alias hay que ponerlos en la subtabla de FORMATO 4**, no solo en la 12.
Un `for c in astral: t.cmap[alias]=t.cmap[c]` solo toca las tablas que YA tienen
el codepoint astral —las de formato 12—, y libharu lee la (3,1) de formato 4.
El primer intento emitia los codigos correctos (`E004`, `E031` en los `Tj`) y aun
asi no pintaba nada, justo por eso.

### Dos secuelas de P2, encontradas por Alejandro (2026-07-20)

🐞 **`v'` salia como `vφ`.** COLISION DE BYTES: el apostrofo es el byte 39 y en
`map_tex_cmmi` el byte 39 es `varphi`. Mientras la cara por defecto del math fue
Times-Italic no se notaba; al volver a FN_TEX_CMMI, el apostrofo empezo a salir
por esa tabla. Arreglado como manda TeX: en math `'` es una PRIMA (U+2032, via
`map_symbol`), fuera de math sigue siendo apostrofo.
- ⚠️ `add_symbol` acumula sobre el MISMO buffer y lo vuelca con la cara del
  SIMBOLO: hay que `textflush()` antes, o el texto pendiente sale con la cara
  equivocada (la `v` de `v'` desaparecia, porque su byte no esta en LMMathSym).

🐞 **El EPS no embebia LM Math en formulas SIN `\comando`.** `using_fontcmmi`
pide al backend que embeba la fuente, y solo la levantaba `get_symbol_code`. Eso
bastaba mientras el math sin comandos iba a Times-Italic; desde P2 el math YA es
LM Math por defecto, asi que `$y = x^2$` la necesita igual. Sin ella el EPS no
define `/LMMath` y Ghostscript SUSTITUYE por una fuente cualquiera — donde el
byte 162 sale como ¢. Afectaba a quickstart, fig1 y fig4-4, que son los del
corpus con math y sin comandos.
- 💡 Esto explica ademas buena parte de la divergencia EPS/PDF que el barrido
  atribuia a "metrica de texto": fig1 cae de 3.08% a **0.05%** al arreglarlo.

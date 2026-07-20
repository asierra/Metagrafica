# plan_text_struct.md — `\frac`, math apilado y multilínea (RESERVADO, sin implementar)

Redactado 2026-07-19 al discutir si vale la pena. **Conclusión: esperar** a una
figura V3 que lo exija; hoy ninguna lo pide (fig16-10, la que lo motivó, es V1
crudo, huérfana, y hace sus fracciones a mano con `PL` + dígitos colocados). Por
la disciplina del proyecto —una feature entra cuando el corpus la exige— no se
construye todavía. Esto solo deja la decisión planteada.

## Qué regala la fuente y qué no

MG coloca glifos de LM Math **por codepoint** pero **NO lee la tabla MATH de
OpenType**. Consecuencia:

- **Las FORMAS son gratis** (el 20% fácil): `∫ ∑ ∏` ya están mapeados
  (`\int`→0x222B, etc.) y un `$\int f\,dx$` inline ya sale hoy a tamaño fijo;
  `√`, `∮`, delimitadores grandes serían triviales de agregar al mapa.
- **El LAYOUT no** (el 80%): altura del eje matemático (dónde centrar), grosor de
  la barra, factor a *scriptstyle*, y sobre todo los **ensamblados extensibles**
  (∫ alto = gancho+extensión+gancho; delimitadores que estiran) viven en la tabla
  MATH, que MG no parsea.

**Dos gradas, decidir cuál:**
1. **Formas fijas + métricas horneadas** (barra de grosor fijo, eje ≈ media altura
   del math, script ≈ 0.7): cubre `\frac`, `√` sobre radicando chico, `∫` con
   límites inline. Es lo que pide la física de las figuras. Factible con lo que hay
   (glifos + anchos vía `fmmap` de `text.cpp` + una regla).
2. **Tabla MATH** (operadores display, radicales altos, delimitadores elásticos):
   trabajo grande, casi ninguna figura lo pide. Diferir indefinido.

## El contenedor: UNO, no dos

- Un **vbox = pila de `text_line`** con interlineado + alineación. Sirve **directo**
  para multilínea (§14.1, ya especificada sin implementar).
- `\frac` = ese mismo vbox (línea-numerador, línea-denominador) **+** una regla
  **+** colocación inline sobre el eje. La regla y el "medir y avanzar la pluma"
  son del **cliente** `\frac`, no del contenedor → el contenedor queda "solo
  `text_line`" y el mismo sirve para las dos necesidades. `√`/`∫`-con-límites siguen
  el mismo patrón (glifo + caja apilada para índice/límites).
- La `struct` de usuario **NO** encaja para lo inline: su semántica es colocación
  por matriz en caja unitaria, que pelea con el eje matemático y el avance de pluma
  a media línea. El primitivo compartido es más chico que una struct: "pila de
  cajas con métricas verticales".

## La pregunta que decide el costo: inline vs. suelto

- **Inline** (a media fórmula): caja-en-run — medir con `fmmap`, centrar en el eje,
  avanzar la pluma. Es la parte cara.
- **Suelto** (etiqueta en un punto, como fig16-10): el vbox de bloque basta, mucho
  más barato.

Cuando aparezca la primera figura V3 con una fracción, **decidir esto primero**.
Ver `text_parser.cpp`/`text.cpp` (sub/super = desplazamiento de baseline +
`setRelFontSize`; anchos = `fmmap`), y `especificacion_mg.md` §14.

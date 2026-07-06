# Corpus V1 (congelado)

Fuentes en la **sintaxis V1** (comandos de dos letras: `PL`, `CR`, `OPST`, `MKST`,
matrices `TL`/`RT`/`SC`…). Es el corpus histórico, congelado durante la transición
a V3. Cumple dos papeles:

1. **Fixtures del traductor** `mg1to2.py` (§20 de `especificacion_mg.md`): cada
   archivo aquí es una entrada; su traducción vive en `examples/v3/`.
2. **Procedencia**: el original junto a su traducción, para revisión lado a lado.

## Oráculo de migración — `reference/`

`reference/<nombre>.svg` es el render de referencia producido por el compilador
**mientras aún parsea V1**. Es la "imagen verdadera": una traducción V3 es correcta
cuando su salida iguala este SVG (salvo diferencias intencionales). Se eligió SVG
por tamaño (mucho más compacto que EPS en figuras con curvas); SVG, EPS y PDF
coinciden visualmente.

Regenerar (con un binario que aún parsee V1, p. ej. desde la rama `v1-legacy`):

```bash
cd examples/v1 && for f in *.mg; do ../../bin/mg "$f" "reference/${f%.mg}.svg"; done
```

## Importante

- **El compilador V3 de `main` NO parseará estos archivos** (sin modo dual, §22.1).
  No los compiles con el `mg` nuevo una vez que aterrice la gramática V3.
- Los `INPUT` (p. ej. `INPUT arrow`, `INPUT markers`) resuelven dentro de esta
  carpeta: la biblioteca incluida debe estar aquí junto al archivo que la incluye.
- El arnés de regresión (`test/run.sh`) apunta a esta carpeta mientras el
  compilador siga siendo V1. En el *cutover* pasará a probar `examples/v3/`.

## Cutover

Cuando el parser V3 reproduzca las imágenes de `reference/`, la promoción es un
solo commit: `examples/v3/*` → `examples/`, este directorio permanece como
`examples/v1/`, y `test/run.sh` cambia a probar el corpus V3.

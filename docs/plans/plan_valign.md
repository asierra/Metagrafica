# Plan: `valign` (alineación vertical de texto, §4.8)

Pasos para implementar la alineación vertical de texto en V3. Es una adición
**pequeña pero que toca el motor** (a diferencia de `dash`/`align`, que ya
existían en el motor): no hay ningún mecanismo previo de valign —solo el
horizontal (`AT_TALIGN` → `text_align` 0/1/2). Ver `especificacion_mg.md` §4.8:
`valign` = `"baseline"` (default), `"top"`, `"middle"`, `"bottom"`.

**Estado: diferido** porque **ningún ejemplo del corpus lo usa** → no hay oráculo
para verificarlo (solo test sintético / a ojo). Implementar cuando un ejemplo lo
ejercite, o si se quiere cerrar el estado de texto.

## Idea

Replicar el patrón del align horizontal, pero aplicar el desplazamiento
**vertical** en un solo punto (`TextLine::draw`), que cubre los 3 backends porque
todos dibujan en `cur_x/cur_y` (el `rmoveto` mueve el cursor de texto).

## Pasos

### 1. Enum de atributo — `include/primitives.h`
Añadir `AT_TVALIGN` al **final** de `AttributeType` (después de `AT_TSTYLE`; al
final = V1-safe, no desplaza ordinales existentes).

### 2. Estado en el display — `include/Display.h`
En `DisplayState` añadir `int text_valign = 0;` (0 = baseline, default). En
`Display`:
```cpp
void setTextValign(int v) { dspstate.text_valign = v; }
int  getTextValign() const { return dspstate.text_valign; }
```

### 3. Despacho del atributo — `src/primitives.cpp`
En `Attribute::draw`, junto a `case AT_TALIGN`:
```cpp
case AT_TVALIGN: g.setTextValign(value); break;
```

### 4. Aplicar el desplazamiento — `src/text.cpp`, `TextLine::draw`
Análogo al horizontal que ya está ahí (`rmoveto(-x, 0)`). Añadir el vertical:
```cpp
int va = g.getTextValign();
if (va > 0) {
    double fs = g.getFontSize();
    double dy = (va == 1) ? -0.70*fs   // top: el texto cuelga bajo la pluma
              : (va == 2) ? -0.35*fs   // middle: centro ≈ media altura de caja
              :             +0.20*fs;  // bottom (3): la base sube, el pie en la pluma
    g.rmoveto(0, dy);
    g.setTextValign(0);                // consumido, como el horizontal
}
// … dibujar la línea (bucle existente) …
// restaurar va al final, IGUAL que el horizontal restaura text_align.
```
> ⚠ El **signo** hay que verificarlo contra un test sintético: el flip global
> `scale(1,-1)` del SVG puede invertirlo. Los coeficientes 0.70/0.35/0.20 son
> aproximaciones de ascent / cap-height / descent; calibrar a ojo.

### 5. Parser — `src/parserv3.cpp`
En `emitStyleAttr` añadir:
```cpp
if (name == "valign") {
    int v = 0;  // baseline
    if (val.type == Value::STRING) {
        if (val.str == "top") v = 1;
        else if (val.str == "middle") v = 2;
        else if (val.str == "bottom") v = 3;
    }
    auto a = std::make_unique<Attribute>(); a->set(AT_TVALIGN, v); out.push_back(std::move(a));
    return true;
}
```
(el nombre del parámetro `Value` en esa función puede ser `v`; usa otro nombre
para el entero para no chocar). Y añadir `"valign"` a la lista de atributos
por-primitiva en `emitPrimStyle` (junto a `align`/`dash`), para
`text(..., valign="middle")`.

## Verificación (sintética — no hay oráculo)
```
display_size 10 10
world_window 0 10 0 10
text("Ay", valign="top")    { 5 5 }
text("Ay", valign="middle") { 5 5 }
text("Ay", valign="bottom") { 5 5 }
text("Ay")                  { 5 5 }   % baseline
```
Renderizar SVG y comprobar que las 4 `y` difieren en el orden esperado (top más
abajo, bottom más arriba, respecto a la baseline). **Y correr `test/run.sh check`
→ debe seguir `ok=28`**: como `text_valign` default = 0 y el `rmoveto` solo
dispara con `va>0`, no debe haber regresión en el corpus.

## Notas de riesgo
- **Riesgo mínimo**: default 0 → todo el texto existente intacto (goldens verdes).
- **Ojo con el reset / persistencia**: el align horizontal hace `setTextAlign(0)`
  tras aplicarlo. Revisa si `TextLine::draw` corre una vez por `text()` o si el
  estado persiste entre llamadas, y **replica exactamente** lo que hace el
  horizontal para que `valign` persista igual. (Esto fue justo lo que confundió al
  implementar `align`: el comportamiento empírico —el SVG lee `text_align` al
  emitir— mandó sobre la lectura del código. Verifica con el render, no solo
  leyendo.)
- Alcance: ~20 líneas en 4 archivos (`primitives.h`, `Display.h`,
  `primitives.cpp` + `text.cpp`, `parserv3.cpp`).

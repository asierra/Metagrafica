#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""hist2mg — puente de datos masivos a MetaGráfica.

MG grafica lo que le escriben: `polybar` recibe bins, no observaciones (§4.12). Ese
corte es deliberado —el lenguaje describe la figura, no analiza datos—, pero deja un
hueco práctico: entre un CSV de decenas de miles de filas y un `.mg` hay una reducción
que alguien tiene que hacer. Este script la hace y **solo** la hace: lee con pandas,
reduce a histograma + estadísticas, y emite un `.mg` incluible (§15).

Lo que NO hace, a propósito: no dibuja, no elige rangos de eje, no decide colores. El
`.mg` que lo incluye manda sobre todo eso. Y no copia los datos crudos — de 54 mil
filas salen 30 barras.

Uso:
    hist2mg.py config.json -o datos.mg

El config declara la fuente y los paneles:

    {
      "source": "ceniza_completa.csv",
      "bins": 30,
      "panels": [
        {"name": "a", "expr": "CMI_13 - CMI_15", "comment": "deltaT1"},
        {"name": "d", "expr": "CMI_04"}
      ]
    }

`expr` es una expresión de pandas sobre las columnas (`df.eval`), así que las columnas
derivadas —los deltaT de brillo, que son diferencias entre bandas— no necesitan existir
en el archivo. Por panel se emiten:

    <name>_width   ancho de barra (todos los bins son iguales)
    <name>_hist    path de pares (centro, conteo)  → polybar(&<name>_hist, width=…)
    <name>_lo/_hi  extremos del rango binado
    <name>_ymax    conteo máximo (para dimensionar el eje y)
    <name>_n       observaciones no nulas
    <name>_mean/_sd/_min/_max   estadísticas (§13.10 `table` no existe todavía;
                                se emiten para tenerlas a la mano)

El binning es `np.histogram(bins=N)` sobre el rango completo del dato, que es
exactamente lo que hace `seaborn.histplot(bins=N)` — así una figura portada desde
matplotlib cae en las mismas barras. Recortar la vista (el `set_xlim` de un panel) es
decisión del `.mg`, no del binning.
"""

import argparse
import json
import os
import sys

import numpy as np
import pandas as pd


def cargar(source):
    """Lee CSV o XLSX. El formato sale de la extensión."""
    if not os.path.exists(source):
        sys.exit("hist2mg: no existe la fuente: %s" % source)
    if source.endswith(".csv"):
        # low_memory=False: sin él pandas infiere el tipo por trozos y avisa de
        # columnas mixtas en archivos grandes. Aquí solo se leen columnas numéricas,
        # pero el aviso confunde y la inferencia en un paso es la correcta.
        return pd.read_csv(source, low_memory=False)
    return pd.read_excel(source)


def serie(df, expr):
    """Evalúa la expresión de columnas y descarta nulos.

    `df.eval` cubre el caso normal (aritmética entre columnas). Una sola columna no
    siempre es expresión válida —los nombres con espacios o guiones no lo son—, así
    que ese caso se atiende antes, por indexación directa.
    """
    if expr in df.columns:
        s = df[expr]
    else:
        try:
            s = df.eval(expr)
        except Exception as e:
            sys.exit("hist2mg: no se pudo evaluar '%s': %s" % (expr, e))
    return pd.to_numeric(s, errors="coerce").dropna()


def panel_mg(name, s, bins, comment, decimals):
    """Reduce una serie a las líneas .mg de su panel."""
    if len(s) == 0:
        sys.exit("hist2mg: el panel '%s' quedó sin datos" % name)

    counts, edges = np.histogram(s.values, bins=bins)
    centers = (edges[:-1] + edges[1:]) / 2.0
    width = edges[1] - edges[0]

    f = "%%.%df" % decimals
    out = []
    head = "%% panel %s" % name
    if comment:
        head += " — %s" % comment
    out.append("%s  (n=%d, %d bins)" % (head, len(s), bins))
    out.append((("%s_width = " + f) % (name, width)))

    # El path son pares (centro, conteo). Se parte en renglones de 4 pares para que
    # el .mg siga siendo legible y diffeable.
    out.append("path %s_hist = {" % name)
    for i in range(0, len(centers), 4):
        trozo = "   ".join((f + " %d") % (centers[j], counts[j])
                           for j in range(i, min(i + 4, len(centers))))
        out.append("    " + trozo)
    out.append("}")

    for k, v in (("lo", edges[0]), ("hi", edges[-1]),
                 ("mean", s.mean()), ("sd", s.std()),
                 ("min", s.min()), ("max", s.max())):
        out.append((("%s_%s = " + f) % (name, k, v)))
    out.append("%s_ymax = %d" % (name, counts.max()))
    out.append("%s_n = %d" % (name, len(s)))
    out.append("")
    return out


def main():
    ap = argparse.ArgumentParser(description="CSV/XLSX → histogramas y estadísticas en .mg")
    ap.add_argument("config", help="JSON con source/bins/panels")
    ap.add_argument("-o", "--output", required=True, help="archivo .mg a escribir")
    # 6 y no 4: el .mg es un archivo INTERMEDIO, así que no puede redondear a la
    # precisión con la que la figura va a presentar el número. Con 4 decimales, una
    # media de 0.00951836 se guardaba como 0.0095 y la tabla (que formatea a 3) la
    # imprimía como 0.009, cuando redondear UNA sola vez desde el dato da 0.010. El
    # doble redondeo es invisible hasta que alguien compara con la fuente.
    ap.add_argument("--decimals", type=int, default=6,
                    help="decimales de los valores en el .mg (default 6). Es precisión de "
                         "TRANSPORTE: la de presentación la fija la figura con decimals=")
    args = ap.parse_args()

    with open(args.config) as fh:
        cfg = json.load(fh)

    # El source del config se resuelve relativo al propio config, no al cwd: así el
    # config es movible junto con sus datos.
    base = os.path.dirname(os.path.abspath(args.config))
    source = cfg["source"]
    if not os.path.isabs(source):
        source = os.path.join(base, source)

    print("hist2mg: leyendo %s…" % source, file=sys.stderr)
    df = cargar(source)
    print("hist2mg: %d filas, %d columnas" % (len(df), len(df.columns)), file=sys.stderr)

    bins_default = cfg.get("bins", 30)
    lineas = [
        "% Generado por tools/hist2mg.py — NO editar a mano.",
        "%% Fuente: %s (%d filas)" % (os.path.basename(source), len(df)),
        "%",
        "% Cada panel trae su path de barras (pares centro conteo), el ancho de barra y",
        "% sus estadísticas. Se usa con: polybar(&<panel>_hist, width=<panel>_width).",
        "",
    ]
    for p in cfg["panels"]:
        s = serie(df, p["expr"])
        lineas += panel_mg(p["name"], s, p.get("bins", bins_default),
                           p.get("comment", p["expr"]), args.decimals)
        print("hist2mg:   panel %s (%s): n=%d mean=%.3f sd=%.3f min=%.3f max=%.3f"
              % (p["name"], p["expr"], len(s), s.mean(), s.std(), s.min(), s.max()),
              file=sys.stderr)

    with open(args.output, "w") as fh:
        fh.write("\n".join(lineas))
    print("hist2mg: escrito %s" % args.output, file=sys.stderr)


if __name__ == "__main__":
    main()

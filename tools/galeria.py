#!/usr/bin/env python3
"""Genera docs/galeria.html: cada ejemplo del corpus junto a su render y su fuente.

    python3 tools/galeria.py            # escribe docs/galeria.html
    python3 tools/galeria.py --check    # falla si la página está rancia

Auxiliar FUERA del compilador, como hist2mg.py: no se liga a bin/mg y el lenguaje
no depende de ella.

De dónde sale el texto de cada tarjeta. Del encabezado del propio .mg, que sigue
esta convención:

    % Título de una línea.            ← título de la tarjeta
    %
    % Dos a cinco líneas de qué es    ← descripción
    % y qué enseña del lenguaje.
    %
    % NOTAS ------------------------  ← para quien mantiene; NO se publica
    % ...

Así no hay lista de descripciones que mantener aparte: el archivo se describe a sí
mismo. Lo que sigue a "% NOTAS" nunca sale de aquí.

⚠ QUÉ ENTRA, y por qué NO es la misma regla de la compuerta `imgfail`. Aquí entra
un examples/X.mg que tenga docs/img/X.svg. La compuerta itera al revés —sobre lo
que hay en docs/img— y por eso vigila además las variantes que existen solo para
los ensayos (franck_condon_anarm, turning_points_nodos, parabola_vs_arco), que no
son ejemplos del corpus y no van a la galería. Son dos reglas parecidas y
distintas; no las unifiques.

Un ejemplo sin render no entra, y eso es deliberado: curvas3.mg es una biblioteca
de datos que compila a una página en blanco.
"""

import html
import pathlib
import sys

# Orden de presentación. Lo que no esté aquí cae al final, en "Más ejemplos": un
# ejemplo nuevo aparece solo, sin tocar esta lista.
GRUPOS = [
    ("Gráficas de datos",
     "El plot mapea unidades de datos a una caja en centímetros; los ejes heredan "
     "los rangos y se rotulan solos.",
     ["quickstart", "fig6-4", "fig4-4", "fig_polybar", "fig1", "tiro_parabolico"]),
    ("Figuras calculadas",
     "Nada está medido: se dan los parámetros físicos y la geometría se deduce. "
     "Cambia un número y la figura entera se reacomoda.",
     ["franck_condon", "turning_points", "fractal_tree"]),
    ("Ilustraciones",
     "Diagramas de aparato y esquemas, donde mandan las estructuras, la colocación "
     "y los marcadores que se orientan solos.",
     ["fig2-5", "fig2-1", "fig4-1", "rpstest"]),
    ("Láminas de referencia",
     "Catálogos: cada forma, cada relleno y cada símbolo en su expresión más "
     "simple, para copiar y pegar.",
     ["primitives", "fill_styles", "line_patterns", "markers-demo", "symbols",
      "texto", "path_sample", "sines"]),
]

REPO = "https://github.com/asierra/Metagrafica/blob/main/examples/"

CSS = """
:root {
  --fondo: #ffffff; --texto: #1a1a1a; --tenue: #5b6472; --borde: #e2e5ea;
  --tarjeta: #ffffff; --acento: #2b5c8a; --codigo: #f6f7f9;
}
@media (prefers-color-scheme: dark) {
  :root {
    --fondo: #14171c; --texto: #e6e8ec; --tenue: #99a2b0; --borde: #2b3038;
    --tarjeta: #1b1f26; --acento: #7fb2e0; --codigo: #101318;
  }
}
:root[data-theme="dark"] {
  --fondo: #14171c; --texto: #e6e8ec; --tenue: #99a2b0; --borde: #2b3038;
  --tarjeta: #1b1f26; --acento: #7fb2e0; --codigo: #101318;
}
:root[data-theme="light"] {
  --fondo: #ffffff; --texto: #1a1a1a; --tenue: #5b6472; --borde: #e2e5ea;
  --tarjeta: #ffffff; --acento: #2b5c8a; --codigo: #f6f7f9;
}
* { box-sizing: border-box; }
body {
  margin: 0; padding: 2.5rem 1.25rem 4rem;
  background: var(--fondo); color: var(--texto);
  font: 16px/1.6 -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
}
.envoltura { max-width: 1180px; margin: 0 auto; }
header { border-bottom: 1px solid var(--borde); padding-bottom: 1.75rem; margin-bottom: 2.5rem; }
h1 { font-size: 1.9rem; margin: 0 0 .4rem; letter-spacing: -.02em; }
header p { margin: .4rem 0; color: var(--tenue); max-width: 62ch; }
a { color: var(--acento); }
h2 { font-size: 1.25rem; margin: 3rem 0 .3rem; letter-spacing: -.01em; }
h2:first-of-type { margin-top: 0; }
.grupo-desc { color: var(--tenue); margin: 0 0 1.5rem; max-width: 62ch; font-size: .95rem; }
.rejilla { display: grid; gap: 1.5rem; grid-template-columns: repeat(auto-fill, minmax(330px, 1fr)); }
.tarjeta {
  border: 1px solid var(--borde); border-radius: 10px; overflow: hidden;
  background: var(--tarjeta); display: flex; flex-direction: column;
}
.lienzo {
  background: #fff; border-bottom: 1px solid var(--borde);
  padding: 1rem; display: flex; align-items: center; justify-content: center;
  min-height: 210px;
}
.lienzo img { max-width: 100%; height: auto; max-height: 380px; }
.cuerpo { padding: 1rem 1.15rem 1.15rem; flex: 1; display: flex; flex-direction: column; }
.cuerpo h3 { font-size: 1.02rem; margin: 0 0 .45rem; }
.cuerpo p { margin: 0 0 .9rem; color: var(--tenue); font-size: .9rem; flex: 1; }
.pie { display: flex; align-items: center; gap: .9rem; font-size: .85rem; }
.pie code { background: var(--codigo); padding: .15rem .4rem; border-radius: 4px; font-size: .85em; }
details { margin-top: .9rem; }
details summary { cursor: pointer; font-size: .85rem; color: var(--acento); }
details pre {
  margin: .7rem 0 0; padding: .85rem; background: var(--codigo);
  border: 1px solid var(--borde); border-radius: 6px;
  overflow-x: auto; font-size: .78rem; line-height: 1.45; max-height: 460px;
}
footer { margin-top: 4rem; padding-top: 1.5rem; border-top: 1px solid var(--borde);
         color: var(--tenue); font-size: .9rem; }
"""


def encabezado(texto):
    """Devuelve (título, descripción) del bloque de comentario inicial."""
    lineas = []
    for linea in texto.split("\n"):
        if not linea.startswith("%"):
            break
        lineas.append(linea[1:].strip())
    parrafos, actual = [], []
    for linea in lineas:
        if linea.startswith("NOTAS"):
            break
        if linea:
            actual.append(linea)
        elif actual:
            parrafos.append(" ".join(actual))
            actual = []
    if actual:
        parrafos.append(" ".join(actual))
    titulo = parrafos[0] if parrafos else ""
    desc = parrafos[1] if len(parrafos) > 1 else ""
    return titulo, desc


def marcado(texto):
    """Escapa a HTML y convierte `código` en <code>, que es el único marcado que
    usan los encabezados."""
    partes = html.escape(texto).split("`")
    return "".join(p if i % 2 == 0 else "<code>%s</code>" % p
                   for i, p in enumerate(partes))


def tarjeta(raiz, nombre):
    fuente = (raiz / "examples" / (nombre + ".mg")).read_text(encoding="utf-8")
    titulo, desc = encabezado(fuente)
    e = html.escape
    return f"""      <article class="tarjeta">
        <div class="lienzo"><img src="img/{nombre}.svg" alt="{e(titulo.replace('`', ''))}" loading="lazy"></div>
        <div class="cuerpo">
          <h3>{marcado(titulo)}</h3>
          <p>{marcado(desc)}</p>
          <div class="pie"><code>{nombre}.mg</code>
            <a href="{REPO}{nombre}.mg">ver en GitHub</a></div>
          <details><summary>Ver el código que la dibuja</summary>
            <pre>{e(fuente.rstrip())}</pre></details>
        </div>
      </article>
"""


def construir(raiz):
    disponibles = sorted(
        p.stem for p in (raiz / "examples").glob("*.mg")
        if (raiz / "docs" / "img" / (p.stem + ".svg")).exists()
    )
    partes, usados = [], set()
    for nombre, desc, ejemplos in GRUPOS:
        presentes = [x for x in ejemplos if x in disponibles]
        if not presentes:
            continue
        usados.update(presentes)
        partes.append(f"    <h2>{html.escape(nombre)}</h2>\n"
                      f"    <p class=\"grupo-desc\">{html.escape(desc)}</p>\n"
                      "    <div class=\"rejilla\">\n")
        partes += [tarjeta(raiz, x) for x in presentes]
        partes.append("    </div>\n")
    sobrantes = [x for x in disponibles if x not in usados]
    if sobrantes:
        partes.append("    <h2>Más ejemplos</h2>\n"
                      "    <div class=\"rejilla\">\n")
        partes += [tarjeta(raiz, x) for x in sobrantes]
        partes.append("    </div>\n")

    return f"""<!doctype html>
<html lang="es">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>MetaGráfica — galería de ejemplos</title>
<style>{CSS}</style>
</head>
<body>
<div class="envoltura">
<header>
  <h1>MetaGráfica — galería</h1>
  <p>Un lenguaje descriptivo para figuras técnicas y científicas. Cada figura de
  esta página es un archivo de texto: despliega el código bajo cualquiera de
  ellas y lo verás entero, tal como se compiló.</p>
  <p>Todas se generaron con <code>bin/mg ejemplo.mg salida.svg</code>. El mismo
  archivo produce EPS, SVG o PDF según la extensión que le pidas.</p>
  <p><a href="https://github.com/asierra/Metagrafica">Repositorio</a> ·
     <a href="https://github.com/asierra/Metagrafica/blob/main/docs/referencia.md">Referencia del lenguaje</a> ·
     <a href="https://github.com/asierra/Metagrafica#building">Cómo compilarlo</a></p>
</header>
{''.join(partes)}<footer>
  Página generada por <code>tools/galeria.py</code> a partir de
  <code>examples/</code> y <code>docs/img/</code>. El título y la descripción de
  cada tarjeta salen del encabezado de su propio <code>.mg</code>.
</footer>
</div>
</body>
</html>
"""


def main():
    raiz = pathlib.Path(__file__).resolve().parent.parent
    destino = raiz / "docs" / "galeria.html"
    pagina = construir(raiz)
    if "--check" in sys.argv:
        actual = destino.read_text(encoding="utf-8") if destino.exists() else ""
        if actual != pagina:
            print("galeria.html está RANCIA: corre python3 tools/galeria.py")
            return 1
        print("galeria.html al día")
        return 0
    destino.write_text(pagina, encoding="utf-8")
    print("escrito %s" % destino)
    return 0


if __name__ == "__main__":
    sys.exit(main())

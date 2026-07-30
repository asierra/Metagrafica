#!/usr/bin/env python3
"""
Genera el Modelfile de ollama para el agente que escribe MetaGráfica.

Emite el archivo COMPLETO (parámetros + SYSTEM) en `docs/modelfile_llm.txt`, listo para
`ollama create mg -f docs/modelfile_llm.txt`.

⚠ Las tres decisiones de este archivo salieron del experimento de tres brazos del
2026-07-29 (mismo modelo, tres SYSTEM, tres tareas cortas, `bin/mg` como juez):

1. NO se manda la referencia completa. El brazo con referencia costaba 15.8k tokens,
   compilaba igual que los demás, y ALUCINABA mobiliario —un `legend` vacío, un `table`
   con filas inventadas, argumentos que no existen (`frame=`, `grid_dash=`)—: darle el
   catálogo entero a un modelo chico hace que use todo lo que ve. Y era el único brazo
   que no lograba corregirse con el mensaje del compilador ni en dos vueltas. Va §15
   (la referencia rápida) y nada más.
2. §15 va ANTES de los ejemplos. Es el destilado, y en la referencia vive al final
   —donde le toca, para un lector humano—, que es justo lo primero que se pierde si algo
   se trunca. Reordenar es trabajo del Modelfile, no del documento.
3. Los ejemplos van SIN sus `% NOTAS` y son una lista EXPLÍCITA, no todos. Las NOTAS son
   procedencia bibliográfica, mediciones y avisos de cobertura de pruebas: 28% de lo que
   se mandaba, ruido para quien escribe una figura. Es la misma decisión que ya había
   tomado `tools/galeria.py` para el caso gemelo.

Las REGLAS DURAS de abajo no son precauciones inventadas: cada una es un fallo que
cometieron los TRES brazos del experimento.
"""

import pathlib
import sys

# Elegidos por cobertura de DECISIONES, no de características: cuándo usar `plot` en vez
# de `world_window`, cómo se rotula matemáticas, que las flechas son un atributo y no un
# dibujo, y cómo se compone con `struct`/`include`. Lista explícita a propósito: un `.mg`
# nuevo en `examples/` no entra solo, igual que en `test/run.sh`.
EJEMPLOS = [
    "quickstart",           # el arranque canónico
    "primitives",           # el catálogo de primitivas
    "texto",                # markup de texto, /n, matemáticas, negrita/itálica
    "markers-demo",         # marcadores y flechas COMO ATRIBUTO
    "path_sample",          # trayectos nombrados, point_at/angle_at
    "fig6-4",               # plot con eje log: la elección de herramienta
    "gravitacion_orbita",   # \frac, include de bibliotecas, struct colocada
    "elevacion_solar",      # geometría derivada de parámetros, iconos, str()
]

PREAMBULO = """Eres un ingeniero de software experto y el asistente definitivo para MetaGráfica (MG), un lenguaje descriptivo escrito en C++ que genera figuras técnicas, científicas y geométricas (EPS, SVG, PDF).

Tu objetivo es escribir código .mg impecable, calculando la geometría analíticamente en lugar de medir a ojo. Entiendes física, percepción remota, órbitas satelitales y visualización de datos, así que tus figuras deben ser matemáticamente correctas.

REGLAS DURAS. Cada una es un error que se midió, no una precaución: rómpela y la figura sale mal o no compila.

1. `world_window` es la geometría de la PÁGINA, nunca las unidades de tus datos. Escribir `world_window 0.4 2.5 0 100` porque tus datos van de 0.4 a 2.5 y de 0 a 100 produce una figura ilegible: el motor es isométrico y el eje más grande aplasta al otro. Los datos van en `x=`/`y=` de `plot`, y `box=(x0,y0,x1,y1)` es la región de la VENTANA que ocupa la caja. Ejemplo correcto: `world_window 0 10 0 7` con `plot(x=(0.4,2.5), y=(0,100), box=(1.2,1, 9,6))`.

2. Un rótulo de varios renglones es UN SOLO `text()`, no varios, y el corte de renglón se escribe con la secuencia de dos caracteres BARRA-DIAGONAL-ENE, o sea `/n`, con la misma barra que `/b` (negrita) y `/e` (énfasis). Ejemplo exacto, cópialo: `text("Radiancia total/n$L_{tot} = \\frac{\\rho E T}{\\pi} + L_p$")`.

3. En MG no existe `\\text{}`, ni ningún comando de LaTeX que no esté en la referencia rápida. Un subíndice de varias letras se escribe con llaves: `$L_{tot}$`. Si escribes `\\text{tot}` el compilador avisa «symbol name unknown text» y el rótulo sale mal.

4. Las letras griegas y los símbolos van por su NOMBRE con barra invertida y dentro de `$…$`: `$\\mu$`, `$\\rho$`, `$\\pi$`, `$\\theta$`. Nunca pegues el glifo Unicode (µ, ρ, π): la fuente no lo tiene por esa vía y el compilador lo descarta con aviso.

5. Los comentarios empiezan con `%`. Un `#` es un error léxico fatal.

MÁS REGLAS DE SINTAXIS:
6. `display_size` fija el tamaño físico en centímetros; `world_window` fija el recorte del plano. No los mezcles.
7. Si los ejes x e y miden cosas distintas o difieren mucho en magnitud, USA `plot` (ver la regla 1). Si comparten unidad —un plano, una órbita, un mapa, una figura geométrica— usa `world_window` directo.
8. Un trayecto nombrado (`path`) se pasa siempre como PRIMER argumento y con `&`: `polyline(&mi_ruta, color="red")`.
9. Dentro de un bloque de coordenadas `{ }` los valores se separan por espacios, sin comas. Toda suma o resta va entre paréntesis: `{ (x+1) (y-2) }`. Las llamadas a función van pegadas al paréntesis: `sqrt(x)`.
10. Las flechas y los marcadores sobre una línea son un ATRIBUTO de la primitiva, que los coloca y los orienta sola: `polyline(marker_end="arrow") { … }`. No los dibujes con `polygon`.

ESTILO DE RESPUESTA:
- Directo y profesional. Sin saludos, introducciones ni conclusiones genéricas.
- Muestra el código .mg de inmediato, en un bloque ```octave.
- No inventes argumentos ni añadas mobiliario que no se pidió (leyendas, tablas, retículas): si no lo viste en los ejemplos ni en la referencia rápida, no existe.
- Si deduces un ángulo o una coordenada con trigonometría, explica brevemente el razonamiento antes del código."""


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


def sin_notas(texto):
    """Quita el bloque `% NOTAS …` (decisión 3 del encabezado de este archivo)."""
    salida, dentro = [], False
    for linea in texto.split("\n"):
        if linea.startswith("%") and "NOTAS" in linea:
            dentro = True
            continue
        if dentro and linea.startswith("%"):
            continue
        if dentro and not linea.startswith("%"):
            dentro = False
        salida.append(linea)
    return "\n".join(salida).strip()


def referencia_rapida(raiz):
    """La §15 de la referencia, tal cual. Es el destilado que va PRIMERO (decisión 2)."""
    texto = (raiz / "docs" / "referencia.md").read_text(encoding="utf-8")
    marca = "\n## 15."
    if marca not in texto:
        print("aviso: no encontré la §15 en docs/referencia.md — el Modelfile va sin ella",
              file=sys.stderr)
        return ""
    return texto[texto.index(marca):].strip()


def main():
    raiz = pathlib.Path(__file__).resolve().parent.parent

    partes = [PREAMBULO, ""]

    rapida = referencia_rapida(raiz)
    if rapida:
        partes += ["--- INICIO DE REFERENCIA RÁPIDA MG ---", rapida,
                   "--- FIN DE REFERENCIA RÁPIDA MG ---", ""]

    partes.append("--- INICIO DE GALERÍA DE EJEMPLOS ---")
    for idx, nombre in enumerate(EJEMPLOS, 1):
        ruta = raiz / "examples" / f"{nombre}.mg"
        if not ruta.exists():
            print(f"aviso: falta examples/{nombre}.mg — se omite", file=sys.stderr)
            continue
        fuente = ruta.read_text(encoding="utf-8")
        titulo, desc = encabezado(fuente)
        bloque = f"\nEJEMPLO {idx}: {titulo}\n"
        if desc:
            bloque += f"INTENCIÓN: {desc}\n"
        bloque += "CÓDIGO:\n```octave\n" + sin_notas(fuente) + "\n```"
        partes.append(bloque)
    partes.append("\n--- FIN DE GALERÍA DE EJEMPLOS ---")

    cuerpo = "\n".join(partes)
    modelfile = ('FROM qwen2.5-coder\n'
                 'PARAMETER num_ctx 32768\n'
                 'PARAMETER temperature 0.1\n'
                 '\nSYSTEM """\n' + cuerpo + '\n"""\n')

    destino = raiz / "docs" / "modelfile_llm.txt"
    destino.write_text(modelfile, encoding="utf-8")
    print(f"escrito {destino}")
    print(f"  {len(modelfile)} bytes, ~{len(modelfile)//35*10} tokens, "
          f"{len(EJEMPLOS)} ejemplos sin NOTAS, §15 al frente")
    print("  crear el modelo:  ollama create mg -f docs/modelfile_llm.txt")
    return 0


if __name__ == "__main__":
    sys.exit(main())

#!/usr/bin/env python3
"""
Genera el Modelfile de ollama para el agente que escribe MetaGráfica.

Emite el archivo COMPLETO (parámetros + SYSTEM) en `docs/modelfile_llm.txt`, listo para
`ollama create mg -f docs/modelfile_llm.txt`.

    python3 tools/generar_modelfile.py            # regenera
    python3 tools/generar_modelfile.py --check    # falla (1) si quedó rancio

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

Las REGLAS DURAS no son precauciones inventadas: cada una es un fallo que cometieron los
TRES brazos del experimento.

⚠ **Desde el 2026-08-04 las reglas NO viven aquí: se derivan de `skills/figuras-mg/SKILL.md`.**
Estuvieron duplicadas —el mismo conocimiento en dos archivos que nadie cotejaba— y esa es
exactamente la forma en que se pudre algo en este proyecto. El skill es la fuente porque es
el único de los dos que una compuerta compila (`docfail`, vía `tools/docblocks.py`), así que
sus ejemplos no pueden quedarse mintiendo cuando la gramática se mueva.

El reparto es por NATURALEZA, no por comodidad:

  · del skill  → lo que es verdad sobre el LENGUAJE (reglas duras, geometría calculada).
  · de aquí    → lo que es propio de ESTE agente y de este runtime: el papel que se le pide,
                 el formato de respuesta y los parámetros de ollama. Nada de eso es
                 conocimiento del lenguaje y no tiene por qué viajar en el skill.
"""

import pathlib
import re
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

# El PAPEL. Propio de este agente, no del lenguaje: por eso no se deriva del skill.
PAPEL = """Eres un ingeniero de software experto y el asistente definitivo para MetaGráfica (MG), un lenguaje descriptivo escrito en C++ que genera figuras técnicas, científicas y geométricas (EPS, SVG, PDF).

Tu objetivo es escribir código .mg impecable, calculando la geometría analíticamente en lugar de medir a ojo. Entiendes física, percepción remota, órbitas satelitales y visualización de datos, así que tus figuras deben ser matemáticamente correctas."""

# El FORMATO DE RESPUESTA. Instrucciones de chat: tampoco son del lenguaje.
# ⚠ El punto de "no inventes mobiliario" NO se repite aquí: viene del skill, donde está
# con su medición al lado. Repetirlo sería reabrir la duplicación que este cambio cerró.
ESTILO = """ESTILO DE RESPUESTA:
- Directo y profesional. Sin saludos, introducciones ni conclusiones genéricas.
- Muestra el código .mg de inmediato, en un bloque ```octave.
- Si deduces un ángulo o una coordenada con trigonometría, explica brevemente el razonamiento antes del código."""

# Secciones del skill que SON conocimiento del lenguaje. El orden es el del archivo.
SECCIONES_SKILL = ["Reglas duras", "Geometría calculada, no puesta a ojo"]


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


def reglas_del_skill(raiz):
    """Las secciones de SECCIONES_SKILL de `skills/figuras-mg/SKILL.md`, en su orden.

    Se limpia lo que es del REPO y no del lenguaje: la nota de que el archivo lo compila
    una compuerta le sirve a quien mantiene, no a quien escribe una figura. Los bloques
    ```octave se conservan enteros — son ejemplos concretos, y el experimento del
    2026-07-29 midió que lo que hace alucinar es el CATÁLOGO, no el ejemplo.
    """
    ruta = raiz / "skills" / "figuras-mg" / "SKILL.md"
    if not ruta.exists():
        print(f"error: falta {ruta}; las reglas duras salen de ahí desde el 2026-08-04",
              file=sys.stderr)
        return None

    texto = ruta.read_text(encoding="utf-8")
    # Fuera el frontmatter YAML: `name`/`description` son para el cargador de skills.
    texto = re.sub(r"\A---\n.*?\n---\n", "", texto, flags=re.S)

    partes, faltan = [], []
    for titulo in SECCIONES_SKILL:
        m = re.search(r"^## " + re.escape(titulo) + r"\s*$(.*?)(?=^## |\Z)",
                      texto, flags=re.S | re.M)
        if not m:
            faltan.append(titulo)
            continue
        cuerpo = m.group(1)
        # Las citas en bloque del skill son notas de mantenimiento (dónde vive el archivo,
        # qué compuerta lo mira). Ninguna es conocimiento del lenguaje.
        cuerpo = "\n".join(l for l in cuerpo.split("\n") if not l.startswith(">"))
        partes.append(titulo.upper() + "\n" + cuerpo.strip())

    if faltan:
        # Si alguien renombra una sección del skill, esto se entera en vez de emitir un
        # Modelfile mutilado en silencio — que es como se pierde una regla sin avisar.
        print("error: no encontré en SKILL.md la(s) sección(es): " + ", ".join(faltan),
              file=sys.stderr)
        return None
    return "\n\n".join(partes)


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

    reglas = reglas_del_skill(raiz)
    if reglas is None:
        return 1   # sin las reglas el Modelfile no sirve: mejor no escribir nada

    partes = [PAPEL, "", reglas, "", ESTILO, ""]

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

    # --check: regenera EN MEMORIA y compara, igual que galeria.py. `docs/modelfile_llm.txt`
    # es un asset generado y committeado, y hasta el 2026-08-04 no lo miraba nada: se
    # encontró con 7.4k bytes de atraso —la §15 de la referencia y los ejemplos habían
    # cambiado— sin que ninguna compuerta pudiera verlo. Ahora lo mueve CUALQUIER cambio
    # en la referencia, en los ejemplos de la lista o en el skill, que es justo el punto:
    # es derivado de cuatro fuentes y ninguna avisa cuando se mueve.
    if "--check" in sys.argv:
        actual = destino.read_text(encoding="utf-8") if destino.exists() else ""
        if actual != modelfile:
            print("modelfile_llm.txt RANCIO: corre python3 tools/generar_modelfile.py")
            return 1
        print("modelfile al día")
        return 0

    destino.write_text(modelfile, encoding="utf-8")
    print(f"escrito {destino}")
    print(f"  {len(modelfile)} bytes, ~{len(modelfile)//35*10} tokens, "
          f"{len(EJEMPLOS)} ejemplos sin NOTAS, §15 al frente")
    print(f"  reglas duras: derivadas de skills/figuras-mg/SKILL.md "
          f"({len(SECCIONES_SKILL)} secciones)")
    print("  crear el modelo:  ollama create mg -f docs/modelfile_llm.txt")
    return 0


if __name__ == "__main__":
    sys.exit(main())

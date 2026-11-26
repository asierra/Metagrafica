/*
       File:  main.cpp
MetaGrafica:  Human descriptive language to generate publication quality
              Display in PostScript/SVG/PDF.
     Author:  Alejandro Aguilar Sierra, UNAM
Antecedents: 2011, 1999 C++ STL, 1991 C. Original: 1988, Pascal and Assembler.

Entry point del compilador V3: lee el .mg, llama a buildFromSource() (parserv3.
cpp, vía parserv3.h) y elige el backend (EPS/SVG/PDF) por la extensión de
salida. El front-end V1 (Parser.cpp/mgpp.l) que este archivo reemplazó se borró
del árbol el 2026-07-27; la semántica V1 que consulta tools/mg1to2.py vive en la
rama v1-legacy (git show v1-legacy:src/Parser.cpp).
*/

#include <clocale>   // setlocale: la salida numérica no depende del entorno
#include <cstdio>
#include <cstring>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>

#include "structures.h"
#include "EPSDisplay.h"
#include "PDFDisplay.h"
#include "SVGDisplay.h"
#include "parserv3.h"
#include "version.h"

// Ayuda de uso. Recibe el FILE* porque va a stdout cuando la piden (-h) y a
// stderr cuando es un error de uso (sin argumentos): un mensaje de error no
// debe ensuciar la salida que un script captura.
//
// El texto va COMPLETO y en un solo bloque, a propósito: es el único lugar
// donde vive, así que el día que haya un man en español la traducción es un
// segundo bloque y no una cacería de printf por todo main().
//
// ⚠️ En INGLÉS, igual que man/mg.1.md: la ayuda remite al manual y las dos
// piezas son una sola —una ayuda en español que manda a un texto en inglés
// miente sobre lo que el lector va a encontrar—. Los errores y avisos del
// compilador siguen en español (Code style). El nombre lleva acento, como el
// resto de los mensajes (ver parserv3.cpp, "es un comando de MetaGráfica");
// sin acento se escribe solo donde es identificador o formato de máquina:
// %%Creator del EPS, la clase MetaGrafica, el nombre del paquete.
static void usage(FILE *out) {
  std::fprintf(out,
    "MetaGráfica %s — compiler for scientific figures: EPS, SVG and PDF.\n"
    "\n"
    "Usage:  mg [options] <figure.mg> [output]\n"
    "\n"
    "  mg figure.mg            compiles to figure.eps\n"
    "  mg figure.mg fig.svg    the output EXTENSION chooses the format:\n"
    "  mg figure.mg fig.pdf    .eps (the default), .svg or .pdf\n"
    "\n"
    "Options:\n"
    "  -h, --help      this help\n"
    "  -v, --version   version\n"
    "\n"
    "The language itself is documented elsewhere:\n"
    "  man mg                 the reference manual\n"
    "  share/doc/metagrafica  reference (es/en), gallery and examples, as installed\n"
    "  https://asierra.github.io/Metagrafica/docs/gallery.html\n"
    "                         the gallery: every figure with its source\n",
    MG_VERSION);
}

int main(int argc, char **argv) {
  // Los tres backends emiten números con printf ("%f"/"%g"), y printf respeta
  // LC_NUMERIC: en un locale de coma decimal, cada coordenada saldría «12,5» y
  // el EPS, el SVG y el PDF quedarían corruptos.
  //
  // Hoy esto NO puede pasar por el entorno —un programa C arranca en el locale
  // "C" pase lo que pase en las variables, y nadie en este árbol llama a
  // setlocale (verificado 2026-07-22)—, pero sí podría pasar si alguna
  // biblioteca enlazada lo llamara, incluso desde un inicializador estático
  // antes de main. Fijarlo aquí es el seguro, y cuesta una línea.
  //
  // ⚠️ La red de pruebas NO cazaría esa rotura: test/run.sh exporta LC_ALL=C, así
  // que pasaría verde mientras el binario falla en la máquina del usuario.
  std::setlocale(LC_NUMERIC, "C");

  std::string inname;
  std::string outfmt;   // extensión de salida solicitada por el usuario

  if (argc >= 2) {
    if (!std::strcmp(argv[1], "-h") || !std::strcmp(argv[1], "--help")) {
      usage(stdout);
      return 0;
    } else if (!std::strcmp(argv[1], "-v") || !std::strcmp(argv[1], "--version")) {
      std::printf("MetaGráfica %s\n", MG_VERSION);
      return 0;
    } else {
      inname = argv[1];
    }
  } else {
    // Quien teclea `mg` a secas es justo el público nuevo: recibe la ayuda
    // entera, no una línea de sintaxis. Por stderr y con exit 1, porque sigue
    // siendo un error de uso.
    usage(stderr);
    return 1;
  }

  if (argc >= 3) outfmt = argv[2];

  std::ifstream in(inname);
  if (!in) { std::fprintf(stderr, "no se pudo abrir %s\n", inname.c_str()); return 1; }
  std::stringstream ss;
  ss << in.rdbuf();

  // Directorio del archivo principal: resuelve los include relativos (§15).
  size_t slash = inname.find_last_of('/');
  if (slash != std::string::npos) g_baseDir = inname.substr(0, slash);

  std::unique_ptr<MetaGrafica> mg(buildFromSource(ss.str()));

  // Determinar formato por la extensión del argumento de salida (si se dio)
  // o generar el nombre derivando el basename del input (default .eps).
  std::string outname;
  bool use_pdf = false, use_svg = false;
  if (!outfmt.empty()) {
    outname = outfmt;
    use_pdf = (outname.size() > 4 && outname.compare(outname.size() - 4, 4, ".pdf") == 0);
    use_svg = (outname.size() > 4 && outname.compare(outname.size() - 4, 4, ".svg") == 0);
  } else {
    std::string base = inname;
    size_t dot = base.find_last_of('.');
    if (dot != std::string::npos && (slash == std::string::npos || dot > slash)) base = base.substr(0, dot);
    outname = base + ".eps";
  }

  if (use_pdf) {
    PDFDisplay g(outname);
    mg->draw(g);
  } else if (use_svg) {
    SVGDisplay g(outname);
    g.flags = g_flags;
    mg->draw(g);
  } else {
    EPSDisplay g(outname);
    g.flags = g_flags;
    // Solo el nombre del .mg, sin directorio: %%Title no debe depender de dónde
    // se compiló (ver EPSDisplay.h). Con `\` además de `/` para que el .exe de
    // Windows produzca el mismo byte que Linux, que es lo que vigila release.yml.
    size_t sep = inname.find_last_of("/\\");
    g.source_name = (sep == std::string::npos) ? inname : inname.substr(sep + 1);
    mg->draw(g);
  }

  std::printf("render -> %s\n", outname.c_str());
  return 0;
}

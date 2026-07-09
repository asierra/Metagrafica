/*
       File:  main.cpp
MetaGrafica:  Human descriptive language to generate publication quality
              Display in PostScript/SVG/PDF.
     Author:  Alejandro Aguilar Sierra, UNAM
    Version:  2026 (V3, post-cutover §22.6)
Antecedents: 2011, 1999 C++ STL, 1991 C. Original: 1988, Pascal and Assembler.

Entry point del compilador V3: lee el .mg, llama a buildFromSource() (parserv3.
cpp, vía parserv3.h) y elige el backend (EPS/SVG/PDF) por la extensión de
salida. El front-end V1 (Parser.cpp/mgpp.l) que este archivo reemplazó sigue en
el árbol como referencia del traductor mg1to2.py, pero fuera del build.
*/

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

int main(int argc, char **argv) {
  std::string inname;
  std::string outfmt;   // extensión de salida solicitada por el usuario

  if (argc >= 2) {
    if (!std::strcmp(argv[1], "-h")) {
      std::printf("Please consult the man page \"man mg\"\n");
      return 0;
    } else if (!std::strcmp(argv[1], "-v")) {
      std::printf("Metagrafica version %s\n", MG_VERSION);
      return 0;
    } else {
      inname = argv[1];
    }
  } else {
    std::fprintf(stderr, "Usanza:  mg [-h|-v] <archivo.mg> [salida.eps|salida.pdf|salida.svg]\n");
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
    mg->draw(g);
  }

  std::printf("render -> %s\n", outname.c_str());
  return 0;
}

/*
       File:  main.cpp             
MetaGrafica:  Human descriptive language to generate publication quality
              Display in PostScript.
     Author:  Alejandro Aguilar Sierra, UNAM
    Version:  2024
Antecedents: 2011, 1999 C++ STL, 1991 C. Original: 1988, Pascal and Assembler.
*/

#include "mg.h"
#include "EPSDisplay.h"
#include "PDFDisplay.h"

#include <cerrno>
#include <cstdio>

#include "Parser.h"

int main(int argc, char **argv) {
  string inname;
  string outfmt;  // extensión de salida solicitada por el usuario

  if (argc >= 2) {
    if (!strcmp(argv[1], "-h")) {
      printf("Please consult the man page \"man mg\"\n");
      return 0;
    } else if (!strcmp(argv[1], "-v")) {
      printf("Metagrafica version %s\n", MG_VERSION);
      return 0;
    } else
      inname = argv[1];
  } else {
    fprintf(stderr, "Usanza:  mg [-h|-v] <archivo.mg> [salida.eps|salida.pdf]\n");
    exit(1);
  }

  // Nombre de salida explícito opcional
  if (argc >= 3)
    outfmt = argv[2];

  Parser parser(inname);
  std::unique_ptr<MetaGrafica> mg(parser.parse());

  string base = parser.getCanonicalName();

  // Determinar formato por la extensión del argumento de salida (si se dio)
  // o generar el nombre derivando la extensión del input
  bool use_pdf = false;
  string outname;

  if (!outfmt.empty()) {
    outname  = outfmt;
    use_pdf  = (outname.size() > 4 &&
                outname.compare(outname.size() - 4, 4, ".pdf") == 0);
  } else {
    outname = base + ".eps";
  }

  if (use_pdf) {
    PDFDisplay g(outname);
    mg->draw(g);
  } else {
    EPSDisplay g(outname);
    g.flags = parser.flags;
    mg->draw(g);
  }

  return 0;
}

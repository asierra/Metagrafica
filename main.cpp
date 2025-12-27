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

#include <cerrno>
#include <cstdio>

#include "Parser.h"

int main(int argc, char **argv) {
  string inname;

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
    fprintf(stderr, "Usanza:  mg [-h|-v] <archivo.mg>\n");
    exit(1);
  }

  Parser parser(inname);
  std::unique_ptr<MetaGrafica> mg(parser.parse());

  string outname = parser.getCanonicalName() + ".eps";
  EPSDisplay g(outname);
  mg->draw(g);

  Structure::cleanup();

  return 0;
}

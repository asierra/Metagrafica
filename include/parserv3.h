#ifndef MG_PARSERV3_H
#define MG_PARSERV3_H

#include <string>

#include "structures.h"
#include "mgflags.h"

// Compila código fuente V3 (ya leído en memoria) a un árbol MetaGrafica listo
// para draw(Display&). El llamador es dueño del puntero devuelto (igual que
// Parser::parse() en el front-end V1, ya retirado del build).
MetaGrafica *buildFromSource(const std::string &src);

// Directorio del archivo principal: usado por include "..." (§15) para resolver
// rutas relativas. Vive en parserv3.cpp; el entry point (main.cpp) lo fija antes
// de llamar a buildFromSource().
extern std::string g_baseDir;

// Banderas de uso recogidas DURANTE el parseo (parse_text las activa según el
// markup; hatch activa using_hatcher; escala anisótropa/shear activan
// using_ellipse; …). Tras buildFromSource(), el entry point las copia al
// Display (g.flags = g_flags), como hacía main.cpp V1 con parser.flags.
extern MGFlags g_flags;

#endif

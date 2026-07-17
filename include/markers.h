/*
       File:  markers.h
              Geometría física de los marcadores de dot (§4.11): única fuente
              de verdad, consultada por los tres backends. Fuera del parser
              (que solo resuelve el nombre a MarkerId).
MetaGrafica:  Human descriptive language to generate publication quality
              Display in PostScript.
Copyright (c) 1988-2026 Alejandro Aguilar Sierra (algsierra@gmail.com)
    Version:  2026
 This file is part of MetaGrafica.
 Licensed under the GNU General Public License v3.0 (see LICENSE file).
*/
#if !defined(MG_MARKERS_H)
#define MG_MARKERS_H

#include <string>
#include <vector>

#include "primitives.h"

// Forma de un marcador en caja unitaria [-1,1]; el backend la escala por
// `size` y la traslada al ancla ya transformada por el marco (§4.11).
struct MarkerShape {
  std::vector<Path> subpaths;   // en caja unitaria [-1,1]
  bool fillable = false;        // cuadrado/rombo/flecha sí; cruz/x no
};

// Geometría exacta de cada marcador, en caja unitaria [-1,1].
inline MarkerShape markerShapeForId(MarkerId id) {
  MarkerShape shape;
  switch (id) {
  case MK_CIRCLE:
    // El círculo se enruta a Display::dot (arco real); vacío por robustez,
    // nunca se consulta.
    break;
  case MK_CIRCLE_DOT:
    // ⊙: anillo + punto central, dos arcos reales (Dot::draw); vacío por
    // robustez, nunca se consulta (mismo trato que MK_CIRCLE).
    break;
  case MK_SQUARE:
    shape.fillable = true;
    shape.subpaths.push_back({
      point(-1, -1), point(1, -1), point(1, 1), point(-1, 1), point(-1, -1)
    });
    break;
  case MK_DIAMOND:
    shape.fillable = true;
    shape.subpaths.push_back({
      point(0, -1), point(1, 0), point(0, 1), point(-1, 0), point(0, -1)
    });
    break;
  case MK_CROSS:
    shape.fillable = false;
    shape.subpaths.push_back({ point(-1, 0), point(1, 0) });
    shape.subpaths.push_back({ point(0, -1), point(0, 1) });
    break;
  case MK_X:
    shape.fillable = false;
    shape.subpaths.push_back({ point(-1, -1), point(1, 1) });
    shape.subpaths.push_back({ point(-1, 1), point(1, -1) });
    break;
  case MK_ARROW:
    shape.fillable = true;
    shape.subpaths.push_back({
      point(-1, -0.7), point(1, 0), point(-1, 0.7), point(-1, -0.7)
    });
    break;
  }
  return shape;
}

// Resuelve el nombre (§7.5 marker=) a MarkerId. false si no reconoce el
// nombre (out queda intacto).
inline bool markerIdForName(const std::string &name, MarkerId &out) {
  if (name == "circle")  { out = MK_CIRCLE;  return true; }
  if (name == "square")  { out = MK_SQUARE;  return true; }
  if (name == "diamond") { out = MK_DIAMOND; return true; }
  if (name == "cross")   { out = MK_CROSS;   return true; }
  if (name == "x")       { out = MK_X;       return true; }
  if (name == "arrow")   { out = MK_ARROW;   return true; }
  if (name == "circle-dot") { out = MK_CIRCLE_DOT; return true; }
  return false;
}

#endif

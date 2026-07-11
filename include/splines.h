/*
       File:  splines.h
              Implementation of Catmull-Rom splines and conversion to Bezier.
MetaGrafica:  Human descriptive language to generate publication quality
              Display in PostScript.
Copyright (c) 2026 Alejandro Aguilar Sierra (asierra@unam.mx)
    Version:  2026
Antecedents: Version 0.0 1988 Pascal and Assembler, first published paper. 
			 Version 1.0 1991 C, first published book.
			 Version 2.0 1999-2024 C++ STL, EPS only, three published books. 
			 
 This file is part of MetaGrafica.
 Licensed under the GNU General Public License v3.0 (see LICENSE file).
*/
#if !defined(MG_SPLINES_H)
#define MG_SPLINES_H

#include "primitives.h"
#include <vector>

/** 
   From a list of control points, creates a path using Catmull-Rom splines.
 */
Path splines(Path controlpoints, int intervals);

/** 
   From a list of Catmull-Rom spline control points, creates a set of Bezier control points, 
   ready to be used in PostScript.
 */
Path splines_to_bezier(Path controlpoints);

Path process_path(Matrix matrix, Path path);

// Ops unarias del álgebra de paths §9 (reflexiones respecto al origen; la
// posición absoluta se reabsorbe en concat_paths y en fit):
//   transpose: (x,y)→(y,x)  reflexión en y=x (intercambia ejes de datos)
//   flip_x:    (x,y)→(−x,y)  voltea en x (izquierda↔derecha; espeja en el eje y)
//   flip_y:    (x,y)→(x,−y)  voltea en y (arriba↔abajo; espeja en el eje x)
Path transpose_path(const Path &p);
Path flip_x_path(const Path &p);
Path flip_y_path(const Path &p);

// concat_paths(a, b): suelda dos paths en uno continuo (álgebra de paths §9).
// Empareja los extremos MÁS CERCANOS (invierte a/b según convenga) y traslada b
// para que su inicio coincida con el final de a; el resultado no depende de la
// orientación de los operandos. Operación gráfica sobre los datos, no de parseo.
Path concat_paths(const Path &a, const Path &b);

void normalize_path(Path& path);

/**
  From four control points, compute the corresponding bezier tangent vectors for
  points p1 and p2 and so, create a bezier segment.
 */
void get_bezier_tangents(point p0, point p1, point p2, point p3, point &t1, point &t2);

/** 
   From a path of control points, creates a set of Bezier control points, 
   ready to be used in PostScript.
 */
Path path_to_bezier(Path controlpoints);
#endif
/*
       File:  splines.h
              Implementation of Catmull-Rom splines and conversion to Bezier.
MetaGrafica:  Human descriptive language to generate publication quality
graphics. Display in PostScript. Author:  Alejandro Aguilar Sierra, UNAM
    Version:  2024
Antecedents: 2011, 1999 C++ STL, 1991 C. Original: 1988, Pascal and Assembler.
*/
#if !defined(__SPLINES_H)
#define __SPLINES_H

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

void concat_paths(Path &path1, Path path2, Matrix mt, bool use_translation=true);

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
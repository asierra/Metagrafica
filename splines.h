/*
       File:  splines.h
              Implementation of Catmull-Rom splines and conversion to Bezier.
MetaGrafica:  Human descriptive language to generate publication quality
graphics. Display in PostScript. Author:  Alejandro Aguilar Sierra, UNAM
    Version:  2024
Antecedents: 2011, 1999 C++ STL, 1991 C. Original: 1988, Pascal and Assembler.
*/

#include "primitives.h"

/** 
   From a list of control points, creates a path using Catmull-Rom splines.
 */
Path spline(Path controlpoints, float tension, int intervals);

/** 
   From a list of Catmull-Rom spline control points, creates a set of Bezier control points, 
   ready to be used in PostScript.
 */
Path spline_to_bezier(Path controlpoints, float tension);
/*
       File:  splines.cpp
              Implementation of Catmull-Rom splines and conversion to Bezier. 
              Tayebi Arasteh, S., Kalisz, A. Conversion Between Cubic Bezier 
              Curves and Catmull–Rom Splines. SN COMPUT. SCI. 2, 398 (2021). 
              https://doi.org/10.1007/s42979-021-00770-x
MetaGrafica:  Human descriptive language to generate publication quality
graphics. Display in PostScript. Author:  Alejandro Aguilar Sierra, UNAM
    Version:  2024
Antecedents: 2011, 1999 C++ STL, 1991 C. Original: 1988, Pascal and Assembler.
*/

#include "primitives.h"

Path spline(Path controlpoints, float tension, int intervals) {
  Path outpath;
  float a, b, c, d;

  int n = controlpoints.size();
  for (int i = 1; i < n; i++) {
    for (int j = 0; j < intervals; j++) {
      point p;

      outpath.push_back(p);
    }
  }
  return outpath;
}

Path spline_to_bezier(Path controlpoints, float tension) {
  Path outpath;

  return outpath;
}
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

#include "splines.h"
#include <stdio.h>

Path splines(Path cp, float tension, int intervals) {
  Path outpath;

  // Must be at least 4 control points
  if (cp.size() < 4)
    return outpath;

  // Polynomial coefficients
  point a, b, c, d;

  int n = cp.size();
  Path::iterator cpit = cp.begin();
  printf("splines %d\n", n);
  for (int i = 0; i < n-3; i++) {
    Path::iterator it = cpit;
    point p0, p1, p2, p3;
    p0 = *it++;
    p1 = *it++;
    p2 = *it++;
    p3 = *it++;
    a = p1;
    b = tension*(p2 - p0);
    c = 3*(p2 - p1) - tension*(p3 - p1) - 2*tension*(p2 - p0);
    d = -2*(p2 - p1) + tension*(p3 - p1) + tension*(p2 - p0);
    for (int j = 0; j <= intervals; j++) {
      float u = (float)j/intervals;
      point p = a + u*(b + c*u + d*u*u);
      outpath.push_back(p);
    }
    printf("segmento %d\n", i);
    cpit++;
  }
  return outpath;
}

Path splines_to_bezier(Path controlpoints, float tension) {
  Path outpath;

  return outpath;
}
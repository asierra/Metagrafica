/*
       File:  splines.cpp
              Implementation of Catmull-Rom splines and conversion to Bezier. 
              Tayebi Arasteh, S., Kalisz, A. Conversion Between Cubic Bezier 
              Curves and Catmull–Rom Splines. SN COMPUT. SCI. 2, 398 (2021). 
              https://doi.org/10.1007/s42979-021-00770-x
              Yuksel C., Schaefer S., Keyser J.: On the parameterization of 
              Catmull-Rom curves. In 2009 SIAM/ACM Joint Conference on Geometric 
              and Physical Modeling (New York, NY, USA, 2009), ACM, pp. 47–53. 
              https://doi:10.1145/1629255.1629262. 
MetaGrafica:  Human descriptive language to generate publication quality
graphics. Display in PostScript. Author:  Alejandro Aguilar Sierra, UNAM
    Version:  2024
Antecedents: 2011, 1999 C++ STL, 1991 C. Original: 1988, Pascal and Assembler.
*/

#include "splines.h"
#include <stdio.h>
#include <math.h>

double distance(point a, point b)
{ 
  float x = a.x - b.x;
  float y = a.y - b.y;

  return sqrt(x*x + y*y);
}

void get_uniform_coefficients(point p0, point p1, point p2, point p3, float tension, 
  point &a, point &b, point &c, point &d) 
{
  a = p1;
  b = tension*(p2 - p0);
  c = 3*(p2 - p1) - tension*(p3 - p1) - 2*tension*(p2 - p0);
  d = -2*(p2 - p1) + tension*(p3 - p1) + tension*(p2 - p0);
}


void get_centripetal_coefficients(point p0, point p1, point p2, point p3, float tension, float alpha,
  point &a, point &b, point &c, point &d) 
{
  float t01 = pow(distance(p0, p1), alpha);
  float t12 = pow(distance(p1, p2), alpha);
  float t23 = pow(distance(p2, p3), alpha);

  point m1 = (1.0 - tension) * (p2 - p1 + t12 * ((p1 - p0)* (1.0/t01) - (p2 - p0)* (1.0/(t01 + t12))));
  point m2 = (1.0 - tension) * (p2 - p1 + t12 * ((p3 - p2)* (1.0/t23) - (p3 - p1)* (1.0/(t12 + t23))));
  d = 2.0 * (p1 - p2) + m1 + m2;
  c = -3.0 * (p1 - p2) - m1 - m1 - m2;
  b = m1;
  a = p1;
}


Path splines(Path cp, float tension, int intervals) {
  Path outpath;

  // Must be at least 4 control points
  if (cp.size() < 4)
    return outpath;

  // Polynomial coefficients
  point a, b, c, d;
  float alpha = 0.5;

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
    //get_uniform_coefficients(p0, p1, p2, p3, tension, a, b, c, d);
    get_centripetal_coefficients(p0, p1, p2, p3, tension, alpha, a, b, c, d);
    printf("punto p1 %g %g\n", p1.x, p1.y);
    for (int j = 0; j <= intervals; j++) {
      double u = (double)j/intervals;
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

  // Must be at least 4 control points
  if (cp.size() < 4)
    return outpath;


  int n = cp.size();
  Path::iterator cpit = cp.begin();
  for (int i = 0; i < n-3; i++) {
    Path::iterator it = cpit;
    point p0, p1, p2, p3;
    p0 = *it++;
    p1 = *it++;
    p2 = *it++;
    p3 = *it++;
    point q0, q1, q2, q3;

    q0 = p3 + 6*(p0 - p1);
    q1 = p0;
    q2 = p3;
    q3 = p0 + 6*(p3 - p1);
    outpath.push_back(p);
    cpit++;
  }
  return outpath;
}
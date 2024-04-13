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

float distance(point a, point b)
{ 
  float dx = a.x - b.x;
  float dy = a.y - b.y;

  return (dx*dx + dy*dy);
}

void get_spline_coefficients(point p0, point p1, point p2, point p3,
  float alpha,
  point &c0, point &c1, point &c2, point &c3) 
{
  float dt0 = powf(distance(p0, p1), alpha);
  float dt1 = powf(distance(p1, p2), alpha);
  float dt2 = powf(distance(p2, p3), alpha);

  // safety check for repeated points
  if (dt1 < 1e-4f)    dt1 = 1.0f;
  if (dt0 < 1e-4f)    dt0 = dt1;
  if (dt2 < 1e-4f)    dt2 = dt1;

  // compute tangents when parameterized in [t1,t2]
  point t1 = (p1 - p0) / dt0 - (p2 - p0) / (dt0 + dt1) + (p2 - p1) / dt1;
  point t2 = (p2 - p1) / dt1 - (p3 - p1) / (dt1 + dt2) + (p3 - p2) / dt2;
 
  // rescale tangents for parametrization in [0,1]
  t1 *= dt1;
  t2 *= dt1;
  c0 = p1;
  c1 = t1;
  c2 = -3*p1 + 3*p2 - 2*t1 - t2;
  c3 = 2*p1 - 2*p2 + t1 + t2;
}


Path splines(Path cp, int intervals) {
  Path outpath;

  // Must be at least 4 control points
  if (cp.size() < 4)
    return outpath;

  // Polynomial coefficients
  point c0, c1, c2, c3;
  float alpha = 0.5;  // Centripetal parametrization

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
    get_spline_coefficients(p0, p1, p2, p3, alpha, c0, c1, c2, c3);
    //printf("punto p1 %g %g p2 %g %g\n", p1.x, p1.y, p2.x, p2.y);
    for (int j = 0; j <= intervals; j++) {
      double u = (double)j/intervals;
      point p = c0 + u*(c1 + u*(c2 + c3*u));
      outpath.push_back(p);
    }
    cpit++;
  }
  return outpath;
}

Path splines_to_bezier(Path cp) {
  Path outpath;

  // Must be at least 4 control points
  if (cp.size() < 4)
    return outpath;

  int n = cp.size();
  point q0, q1, q2, q3;
  float conversion_factor=10;
  Path::iterator cpit = cp.begin();
  for (int i = 0; i < n-3; i++) {
    Path::iterator it = cpit;
    point p0, p1, p2, p3;
    p0 = *it++;
    p1 = *it++;
    p2 = *it++;
    p3 = *it++;

    q0 = p1;
    q1 = p1 + (p2 - p0)/conversion_factor;
    q2 = p2 - (p3 - p1)/conversion_factor;
    q3 = p2;
    outpath.push_back(q0);
    outpath.push_back(q1);
    outpath.push_back(q2);
    cpit++;
  }
  outpath.push_back(q3);
  return outpath;
}


Path process_path(Matrix mtpt, point pp, Path path) {
  Matrix mt;
  Path newpath;

  mt.translate(pp.x, pp.y);
  mt *= mtpt;
  for (const auto &op : path) {
    point p = op;
    mt.transform(p.x, p.y);
    newpath.push_back(p);
  }
  return newpath;
}

void concat_paths(Path &path1, Path path2) {
  printf("paths %zu %zu\n", path1.size(), path2.size());
  path1.insert(path1.end(), path2.begin(), path2.end());
  /*
  point last_point = path1.back();
  bool is_first=true;

  for (const auto &p : path2) {
    if (is_first && last_point==p)
      continue;
    path1.push_back(p);
  }*/
}
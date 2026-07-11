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
#include <algorithm>
#include <iterator>
#include <stdio.h>
#include <math.h>

double distancesq(point a, point b)
{ 
  double dx = a.x - b.x;
  double dy = a.y - b.y;

  return (dx*dx + dy*dy);
}

double distance(point a, point b)
{
  return sqrt(distancesq(a,b));
}


void get_spline_coefficients(point p0, point p1, point p2, point p3,
  double alpha,
  point &c0, point &c1, point &c2, point &c3) 
{
  double dt0 = powf(distancesq(p0, p1), alpha);
  double dt1 = powf(distancesq(p1, p2), alpha);
  double dt2 = powf(distancesq(p2, p3), alpha);

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
  double alpha = 0.5;  // Centripetal parametrization

  int n = cp.size();
  Path::iterator cpit = cp.begin();
  printf("spline new points %d\n", n*intervals);
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
  double conversion_factor=10;
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


Path process_path(Matrix mtpt, Path path) {
  Path newpath;

  for (const auto &op : path) {
    point p = op;
    mtpt.transform(p.x, p.y);
    newpath.push_back(p);
  }
  return newpath;
}

void compute_maxmins_path(Path path, point &mn, point &mx) {
  point min(1e10, 1e10), max(-1e10, -1e10);

  for (const auto &p : path) {
    if (p.x < min.x)
      min.x = p.x;
    if (p.x > max.x)
      max.x = p.x;
    if (p.y < min.y)
      min.y = p.y;
    if (p.y > max.y)
      max.y = p.y;
  }
  mn = min;
  mx = max;
}

void normalize_path(Path& path) {
  point min, max;
  compute_maxmins_path(path, min, max);
  Matrix mt;

  mt.scale(1/(max.x - min.x), 1/(max.y - min.y));
  mt.translate(-min.x, -min.y);

  for (auto &p : path) {
    mt.transform(p.x, p.y);
  }

  compute_maxmins_path(path, min, max);
}

Path transpose_path(const Path &p) {
  Path r = p;
  for (auto &pt : r) { double t = pt.x; pt.x = pt.y; pt.y = t; }
  return r;
}

Path flip_x_path(const Path &p) {
  Path r = p;
  for (auto &pt : r) pt.x = -pt.x;
  return r;
}

Path flip_y_path(const Path &p) {
  Path r = p;
  for (auto &pt : r) pt.y = -pt.y;
  return r;
}

static double concat_dist2(const point &p, const point &q) {
  double dx = p.x - q.x, dy = p.y - q.y; return dx * dx + dy * dy;
}

// concat_paths(a, b): suelda dos paths en uno continuo. Elige el emparejamiento
// de extremos MÁS CERCANO (invierte a/b según convenga) y traslada b para que su
// inicio coincida con el final de a. Así el usuario no hace reverse ni coloca, y
// el resultado no depende de la orientación de los operandos: p. ej. una media
// curva H y su espejo flip_x(H) comparten el pico, y concat(&H, flip_x(&H)) los
// suelda ahí, formando un perfil simétrico de UN pico central (§9).
Path concat_paths(const Path &a, const Path &b) {
  Path p1 = a, p2 = b;
  if (p1.empty()) return p2;
  if (p2.empty()) return p1;
  point a0 = p1.front(), a1 = p1.back(), b0 = p2.front(), b1 = p2.back();
  double d[4] = { concat_dist2(a1, b0), concat_dist2(a1, b1),
                  concat_dist2(a0, b0), concat_dist2(a0, b1) };
  int best = 0;
  for (int i = 1; i < 4; i++) if (d[i] < d[best]) best = i;
  if (best == 2 || best == 3) std::reverse(p1.begin(), p1.end());  // p1 termina en el punto común
  if (best == 1 || best == 3) std::reverse(p2.begin(), p2.end());  // p2 empieza en el punto común
  point tail = p1.back(), head = p2.front();
  double dx = tail.x - head.x, dy = tail.y - head.y;               // suelda (traslada b)
  for (size_t i = 1; i < p2.size(); i++)                           // salta el punto duplicado
    p1.push_back(point(p2[i].x + dx, p2[i].y + dy));
  return p1;
}

void get_bezier_tangents(point p0, point p1, point p2, point p3, point &t1, point &t2)
{
  double d, alpha = 0.5;
  double d1 = powf(distancesq(p0, p1), alpha);
  double d2 = powf(distancesq(p1, p2), alpha);
  double d3 = powf(distancesq(p2, p3), alpha);

  // compute tangents when parameterized in [t1,t2]
  point m = d1*d1*p2 - d2*d2*p0 + (2*d1*d1 + 3*d1*d2 + d2*d2)*p1;
  d = 3*d1*(d1+d2);
  t1 = m/d;

  point n = d3*d3*p1 - d2*d2*p3 + (2*d3*d3 + 3*d3*d2 + d2*d2)*p2;
  d = 3*d3*(d3+d2);
  t2 = n/d;
}

Path path_to_bezier(Path cp) {
  Path outpath;

  // Must be at least 4 control points
  if (cp.size() < 4)
    return outpath;

  int n = cp.size();

  // Add points to both extremes
  // ...

  point p0, p1, p2, p3;
  point t1, t2;
  Path::iterator cpit = cp.begin();
  for (int i = 0; i < n-3; i++) {
    Path::iterator it = cpit;
    p0 = *it++;
    p1 = *it++;
    p2 = *it++;
    p3 = *it++;
    get_bezier_tangents(p0, p1, p2, p3, t1, t2);
    outpath.push_back(p1);
    outpath.push_back(t1);
    outpath.push_back(t2);
    cpit++;
  }
  outpath.push_back(p2);
  return outpath;
}

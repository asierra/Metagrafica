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

float distancesq(point a, point b)
{ 
  float dx = a.x - b.x;
  float dy = a.y - b.y;

  return (dx*dx + dy*dy);
}

float distance(point a, point b)
{
  return sqrt(distancesq(a,b));
}


void get_spline_coefficients(point p0, point p1, point p2, point p3,
  float alpha,
  point &c0, point &c1, point &c2, point &c3) 
{
  float dt0 = powf(distancesq(p0, p1), alpha);
  float dt1 = powf(distancesq(p1, p2), alpha);
  float dt2 = powf(distancesq(p2, p3), alpha);

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

void concat_paths(Path &path1, Path path2, Matrix mt, bool use_translation) {
  Matrix mttl;
  //printf("concat paths %zu %zu\n", path1.size(), path2.size());
  if (path1.size()==0) {
    Path path2m = process_path(mt, path2);
    point p2b = path2m.back(), p2f = path2m.front();
    //printf("extremos %g %g - %g %g\n", p2f.x, p2f.y, p2b.x, p2b.y);
    if (p2f.x > p2b.x) 
      path2m.reverse();
    path1.insert(path1.end(), path2m.begin(), path2m.end());
  } else {
    point p1 = path1.back();
    point p2b = path2.back(), p2f = path2.front();
    mt.transform(p2b.x, p2b.y);
    mt.transform(p2f.x, p2f.y);
    float dx = p1.x;
    float dfy = p1.y - p2f.y;
    //printf("p2f %g p2b %g\n", p2f.x, p2b.x);
    if (p2f.x > p2b.x) {
      path2.reverse();
      dx += p2f.x - p2b.x;
      dfy = p1.y - p2b.y;
    }
    if (use_translation) 
      mttl.translate(dx, dfy);
    mttl = mttl*mt;
    Path path2m = process_path(mttl, path2);
    //printf("point %g %g - %g %g %g\n", p1.x, p1.y, path2m.front().x, path2m.back().x, dfy);
    path1.insert(path1.end(), std::next(path2m.begin()), path2m.end());
  }
}

void get_bezier_tangents(point p0, point p1, point p2, point p3, point &t1, point &t2)
{
  float d, alpha = 0.5;
  float d1 = powf(distancesq(p0, p1), alpha);
  float d2 = powf(distancesq(p1, p2), alpha);
  float d3 = powf(distancesq(p2, p3), alpha);

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

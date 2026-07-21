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
#include <cmath>
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


// alpha es el exponente sobre la DISTANCIA (convención de Yuksel et al.):
// 0 = uniforme, 0.5 = centrípeta, 1 = cordal. Como aquí se parte de la
// distancia al cuadrado, el exponente aplicado es alpha/2 — sin ese medio,
// un alpha=0.5 daría cordal (era el defecto: el código no hacía lo que
// decían su comentario y la spec §9.1).
void get_spline_coefficients(point p0, point p1, point p2, point p3,
  double alpha,
  point &c0, point &c1, point &c2, point &c3)
{
  double dt0 = powf(distancesq(p0, p1), alpha*0.5);
  double dt1 = powf(distancesq(p1, p2), alpha*0.5);
  double dt2 = powf(distancesq(p2, p3), alpha*0.5);

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
  for (int i = 0; i < n-3; i++) {
    Path::iterator it = cpit;
    point p0, p1, p2, p3;
    p0 = *it++;
    p1 = *it++;
    p2 = *it++;
    p3 = *it++;
    get_spline_coefficients(p0, p1, p2, p3, alpha, c0, c1, c2, c3);
    // El nodo j=0 de cada segmento coincide con el j=intervals del anterior:
    // solo el primer segmento lo emite (antes se duplicaba en cada unión).
    for (int j = (i == 0 ? 0 : 1); j <= intervals; j++) {
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
  // Catmull-Rom UNIFORME → Bézier: los controles interiores caen a un SEXTO
  // del vector entre los vecinos. No es una constante ajustable: es el límite
  // d1=d2=d3 de la fórmula no uniforme de get_bezier_tangents, que da
  // (d²p2 − d²p0 + 6d²p1) / 6d² = p1 + (p2 − p0)/6. Estuvo en 10 hasta
  // 2026-07-20, lo que aplanaba la curva y la separaba de la que produce
  // splines() sobre los mismos puntos de control.
  const double conversion_factor = 6;
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

// concat_paths(a, b): suelda dos paths en uno continuo (§9). Traslada b para
// que su INICIO continúe desde el FINAL de a; no invierte ningún operando (spec
// §9: sin auto-reversión — una heurística de extremos más cercanos elige mal en
// cuanto las piezas son cortas, p. ej. una recta de media unidad seguida de un
// medio ciclo de coseno; el autor orienta explícitamente con reverse()). El
// punto de unión duplicado se salta, lo que preserva la aritmética 3k+1 de los
// paths de control bezier.
Path concat_paths(const Path &a, const Path &b) {
  if (a.empty()) return b;
  if (b.empty()) return a;
  Path p1 = a;
  // Un operando de UN SOLO punto se AÑADE tal cual. Soldar (§9) traslada la pieza
  // para que continúe desde el final de a y salta su primer punto por ser la unión
  // duplicada; pero un punto suelto no tiene segmento que continuar ni unión que
  // duplicar, así que el bucle de abajo (que arranca en i=1) lo descartaba entero
  // —`path p += { x y }` en un lazo no acumulaba nada—. Un path de un punto es
  // degenerado pero legal. Las piezas soldadas del corpus son SIEMPRE multipunto
  // (empiezan en {0 0} y se trasladan), así que esto no las toca.
  if (b.size() == 1) { p1.push_back(b.front()); return p1; }
  double dx = p1.back().x - b.front().x, dy = p1.back().y - b.front().y;
  for (size_t i = 1; i < b.size(); i++)                            // salta el punto duplicado
    p1.push_back(point(b[i].x + dx, b[i].y + dy));
  return p1;
}

void get_bezier_tangents(point p0, point p1, point p2, point p3, point &t1, point &t2)
{
  // Parametrización centrípeta (alpha=0.5 sobre la DISTANCIA; el exponente
  // sobre la distancia al cuadrado es alpha/2 — ver get_spline_coefficients).
  // Es la que garantiza no formar cúspides ni autointersecciones (Yuksel et
  // al., citado en el encabezado); es la que documenta §9.2 para smooth.
  double d, alpha = 0.5;
  double d1 = powf(distancesq(p0, p1), alpha*0.5);
  double d2 = powf(distancesq(p1, p2), alpha*0.5);
  double d3 = powf(distancesq(p2, p3), alpha*0.5);

  // Guardas de puntos repetidos: sin ellas un nodo duplicado da distancia 0,
  // el denominador 3*d1*(d1+d2) se anula y las tangentes salen NaN — que el
  // backend escribe como "-nan" en el EPS y con código de salida 0. Son las
  // mismas guardas que get_spline_coefficients siempre tuvo; get_bezier_tangents
  // se quedó sin ellas (añadidas 2026-07-20). El caso llega solo: los paths V1
  // de `SP`/`GNBZPATH` DUPLICAN los extremos a propósito (era su convención
  // para que la curva alcanzara el primer y último punto), así que la primera
  // traducción literal de una figura V1 los trae.
  if (d2 < 1e-4)    d2 = 1.0;
  if (d1 < 1e-4)    d1 = d2;
  if (d3 < 1e-4)    d3 = d2;

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


// Cruces del path con la recta y = y_level; xmin/xmax de sus abscisas.
// El contrato del retorno está en splines.h: false = no cruza, xmin/xmax
// quedan intactos y el llamador NO debe leerlos.
bool path_x_bounds_at_y(const Path &path, double y_level, double &xmin, double &xmax) {
  if (path.size() < 2)
    return false;  // hace falta al menos un segmento (2 puntos) para cruzar

  // Tolerancia para los segmentos horizontales y la división del reparto.
  const double eps = 1e-9;

  std::vector<double> crossings;
  crossings.reserve(8);

  for (size_t i = 0; i + 1 < path.size(); ++i) {
    const point &p1 = path[i];
    const point &p2 = path[i + 1];

    // Segmento horizontal sobre la recta: aporta sus dos extremos.
    if (std::abs(p1.y - y_level) < eps && std::abs(p2.y - y_level) < eps) {
      crossings.push_back(p1.x);
      crossings.push_back(p2.x);
      continue;
    }

    // min/max para no depender de si el trazo sube o baja. Un vértice justo
    // sobre la recta lo cuentan los dos segmentos que lo comparten; da igual
    // para un mínimo y un máximo.
    if (y_level >= std::min(p1.y, p2.y) && y_level <= std::max(p1.y, p2.y) &&
        std::abs(p2.y - p1.y) > eps) {
      crossings.push_back(p1.x + (p2.x - p1.x) * ((y_level - p1.y) / (p2.y - p1.y)));
    }
  }

  if (crossings.empty())
    return false;

  std::pair<std::vector<double>::iterator, std::vector<double>::iterator> mm =
      std::minmax_element(crossings.begin(), crossings.end());
  xmin = *mm.first;
  xmax = *mm.second;
  return true;
}

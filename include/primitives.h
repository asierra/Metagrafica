/*
       File:  primitives.h
              Definition of graphic items like primitives, atributes and 
              transformations.
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
#if !defined(MG_PRIMITIVES_H)
#define MG_PRIMITIVES_H

#include <math.h>

#include <memory>
#include <utility>
#include <vector>

#include "matrix.h"


enum GraphicsItemType {
  // Primitives
  GI_POLYLINE,
  GI_RECTANGLE,
  GI_ARC,
  GI_CURVE,
  GI_TEXT,
  GI_TEXTLINE,
  // Graphics state
  GI_ATTRIBUTE,
  GI_TRANSFORM,
  GI_STATE,
  // Composed
  GI_STRUCTURE,
  GI_STRUCTURE_REF,
  // Parser extensions
  GI_NULL,
  GI_CIRCLE,
  GI_ELLIPSE,
  GI_POLYGON,
  GI_SPLINE,
  GI_BEZIER,
  GI_TICKS,
  GI_DOT,
  GI_CONCATENATEPATH,
  GI_POLYBAR,
  GI_HATCHATTR         // los valores nuevos van AL FINAL: hay código que depende de los ordinales
};

enum AttributeType {
  AT_LWIDTH,
  AT_LSTYLE,
  AT_LCOLOR,
  AT_LGRAY,
  AT_FCOLOR,
  AT_FGRAY,
  AT_THEIGHT,
  AT_TALIGN,
  AT_TSTYLE,
  AT_TVALIGN,
};

enum GraphicsStateType {
  GS_OPENPATH,
  GS_CLOSEPATH,
  GS_FILL,
  GS_NOFILL,
  GS_PLUMEPOSITION,
  GS_SAVE,
  GS_RESTORE,
  // V3 (al final del enum: no desplazar ordinales). Apilan/restauran el estado
  // de dibujo COMPLETO (color/grosor/fill/…) para el ámbito de estado de los
  // bloques §7.1. GS_SAVE/GS_RESTORE (arriba) solo tocan la matriz MTST.
  GS_PUSHSTATE,
  GS_POPSTATE,
  // Contornea un relleno (fill + trazo del borde): §4 "contorno = presencia de
  // color=". Antes solo se lograba con valor de relleno negativo (falla en negro).
  GS_OUTLINEFILL,
  // save/restore del DISPOSITIVO (gsave/grestore EPS; grupo <g> SVG): cierra las
  // transformaciones MTLC intermedias (p.ej. rotación de texto). Lo usa axis para
  // acotar el título rotado. Distinto de GS_SAVE/RESTORE (pila MTST) y de
  // GS_PUSHSTATE/POPSTATE (estado de dibujo).
  GS_DEVSAVE,
  GS_DEVRESTORE,
  // Apaga el contorneo de relleno (§4.11): gemelo de GS_OUTLINEFILL, como
  // GS_NOFILL lo es de GS_FILL. Lo emite la sentencia de estado `outlinefill false`.
  GS_NOOUTLINEFILL,
};

/// Predefined Matrices
enum PredefinedMatrix {
  MTLC, // Local
  MTST, // Structura
  MTPP, // Plume position
  MTPT, // Path
  MTRS, // Repeat structure 
};

/// Geometric point, Vectorial pixel, our basic element. Not vector because we don't define all vector operations
struct point {
  point() { x =0; y = 0; }
  point(double x1, double y1) { x = x1; y = y1; }
  point(const point&) = default;
  double x, y;

  double distancesq(point a) const
  {
    double dx = a.x - x;
    double dy = a.y - y;
    return (dx*dx + dy*dy);
  }
  double distance(point a) const
  {
    return sqrt(distancesq(a));
  }
  double length() const
  {
    return sqrt(x*x + y*y);
  }
  point operator*(const double& f) const {
    point p = *this;
    p.x *= f;
    p.y *= f;
    return p;
    }
  friend point operator*(const double& f, point p) {
    p.x *= f;
    p.y *= f;
    return p;
    }
  void operator*=(const double& f) {
    x *= f;
    y *= f;
  }
  friend point operator/(point p, const double& f) {
    p.x /= f;
    p.y /= f;
    return p;
  }
  void operator/=(const double& f) {
    x /= f;
    y /= f;
  }
  friend point operator+(point p1, const point& p2) {
    p1.x += p2.x;
    p1.y += p2.y;
    return p1;
  }
  friend point operator-(point p1, const point& p2) {
    p1.x -= p2.x;
    p1.y -= p2.y;
    return p1;
  }
  bool operator==(const point& p2) const {
   return (x==p2.x && y==p2.y);
  }
  bool operator!=(const point& p2) const {
    return (x!=p2.x || y!=p2.y);
  }
};


using Path = std::vector<point>;

// Tangente por diferencia sobre un path: central en interiores, unilateral en
// extremos. Devuelve el vector dirección (sin normalizar); (0,0) si n<2. Misma
// convención que StructurePath::draw (§C.1); la comparten dot y los placements.
inline point pathTangent(const Path &path, std::size_t i) {
  const std::size_t n = path.size();
  if (n < 2) return point(0, 0);
  if (i == 0)     return path[1] - path[0];
  if (i == n - 1) return path[n - 1] - path[n - 2];
  return path[i + 1] - path[i - 1];
}

/**
   Descriptor de patrón de relleno (tramado), independiente del dispositivo.
   Sigue el modelo del formato .pat de AutoCAD: un patrón es una o más familias
   de líneas paralelas. Una sola familia = rayado simple; dos = rayado cruzado;
   con `dashes` no vacío = líneas punteadas/de guiones. Cada backend traduce
   este descriptor a su mecanismo nativo (EPS/PDF: clip + líneas; SVG: <pattern>).
 */
struct HatchLine {
  double angle = 0.0;          // dirección de las líneas (grados)
  double gap   = 0.0;          // separación perpendicular entre líneas (pt)
  double ox = 0.0, oy = 0.0;   // origen/fase de la familia
  std::vector<double> dashes;  // vacío = línea continua
};
using FillPattern = std::vector<HatchLine>;

// Abstract output device
class Display;


/** 
   Abstract class to define all members of a graphics sequence.
 */
class GraphicsItem {
public:

  GraphicsItem(GraphicsItemType t) : type(t) { }
  virtual ~GraphicsItem()=default;

  // Jerarquía polimórfica con contabilidad en destructores (StructureUser):
  // una copia implícita rompería los contadores de uso.
  GraphicsItem(const GraphicsItem&) = delete;
  GraphicsItem& operator=(const GraphicsItem&) = delete;
  
  GraphicsItemType getType() const { return type; }
  
  virtual void draw(Display &)=0;
  
protected:
  GraphicsItemType type;
};


/** 
   Abstract class to define members that have a path.
 */
class GraphicsItemWithPath: public GraphicsItem {
public:
  GraphicsItemWithPath(GraphicsItemType t) : GraphicsItem(t) { }
  void setPath(Path p) { path = std::move(p); }
  const Path& getPath() const { return path; }
protected:
  Path path;
};


class Polyline : public GraphicsItemWithPath {
public:
  Polyline(GraphicsItemType t=GI_POLYLINE) : GraphicsItemWithPath(t) { }

  // §4.1: polilínea cerrada (closed=true). El trazo une el último punto con el
  // primero vía closepath, sin repetir la coordenada. No aplica a polygon (que
  // ya cierra por el relleno) ni a ticks.
  void setClosed(bool c) { closed = c; }

  void draw(Display &) override;

private:
  bool closed = false;
};


class Polybar : public GraphicsItemWithPath {
public:
  Polybar(GraphicsItemType t=GI_POLYBAR) : GraphicsItemWithPath(t) { }

  void draw(Display &) override;

  void setWidth(double w) { width = w; }
  void setHorizontal(bool h) { horizontal = h; }

private:
  double width = 1.0;
  bool horizontal = false;
};

/* 
   A rectangle is defined by a pair of points: the left lower point and the right upper point.
   The number of points in the path must be even.
 */
class Rectangle : public GraphicsItemWithPath {
public:
  Rectangle() : GraphicsItemWithPath(GI_RECTANGLE) { }

  void draw(Display &) override;
};

class Arc : public GraphicsItemWithPath {
public:
  Arc() : GraphicsItemWithPath(GI_ARC) { }

  void draw(Display &) override;

  void setRadius(double x, double y) { rx = x; ry = y; }

  void setAngles(double x, double y) { ai = x; af = y; }

private:
  double rx = 0, ry = 0;
  double ai = 0, af = 360;
};

// Marcadores físicos de dot (§4.11): formas alternas al círculo, dispersadas
// en cada punto del path. La geometría vive en markers.h (fuera del parser);
// aquí solo el identificador, para que Dot lo cargue sin ciclo de includes.
enum MarkerId { MK_CIRCLE, MK_SQUARE, MK_DIAMOND, MK_CROSS, MK_X, MK_ARROW };

// Rango de vértices donde un Dot dispersa su marcador (§4.11 marker_start/mid/end):
// todo el path, o solo el primero / interiores / último. La tangente se calcula
// SIEMPRE con el path COMPLETO (por eso es un rango, no un path recortado: recortar
// perdería la orientación en los extremos). first/mid/last particionan [0,n).
enum MarkerRange { MR_ALL, MR_FIRST, MR_MID, MR_LAST };

class Dot : public GraphicsItemWithPath {
public:
  Dot() : GraphicsItemWithPath(GI_DOT) { }

  void draw(Display &) override;

  void setRadius(double x) { r = x;  }
  void setMarker(MarkerId m) { marker_id = m; }
  void setOrient(bool o) { orient = o; }
  void setRange(MarkerRange r_) { range = r_; }

  // Marcador definido por una struct del usuario (§B): en vez de un MarkerId del
  // catálogo, lleva sus subtrayectos crudos (caja del propio struct; ancla = su
  // origen, sin recentrar). Recorren EL MISMO camino físico que los builtin.
  void setCustomShape(std::vector<Path> subs, bool fill) {
    custom_subpaths = std::move(subs); custom_fillable = fill; has_custom = true;
  }

private:
  double r = 1;
  MarkerId marker_id = MK_CIRCLE;
  bool orient = false;         // §B.3: orienta la forma a la tangente local del path
  MarkerRange range = MR_ALL;  // §4.11: subconjunto de vértices a marcar (start/mid/end)
  std::vector<Path> custom_subpaths;   // §B: geometría de un marcador-struct (vacío = builtin)
  bool custom_fillable = false;
  bool has_custom = false;
};

class Attribute : public GraphicsItem {
public:
  Attribute() : GraphicsItem(GI_ATTRIBUTE) {}

  void draw(Display &) override;
  
  void set(AttributeType t, int v) { atype = t; value = v; has_f = false; }
  // Variante flotante (V3): p. ej. line_width en pt, sin la cuantización a 0.2 pt
  // de V1. Si has_f, Attribute::draw usa fvalue en vez de value.
  void setF(AttributeType t, double v) { atype = t; fvalue = v; has_f = true; }

private:
  int value = 0;
  double fvalue = 0.0;
  bool has_f = false;
  AttributeType atype = AT_LWIDTH;
};

// §4.11: transporta un FillPattern completo (una o más familias de tramado).
// Attribute solo carga int/float y no puede cargar el vector de HatchLine, así
// que hatch= (§7.5) y la sentencia de estado `hatch` (§7) emiten esto.
class HatchAttr : public GraphicsItem {
public:
  HatchAttr(FillPattern fp) : GraphicsItem(GI_HATCHATTR), pattern(std::move(fp)) {}

  void draw(Display &) override;

private:
  FillPattern pattern;
};

class Transform : public GraphicsItem {
public:
  Transform() : GraphicsItem(GI_TRANSFORM) {}

  void draw(Display &) override;

  void setOperation(MatrixOperation o) { op = o; }

  MatrixOperation getOperation() const { return op; }

  void setPredefinedMatrix(PredefinedMatrix pdmt) { predefmat = pdmt; }

  PredefinedMatrix getPredefinedMatrix() const { return predefmat; }

  void setPoint(point p) { tl = p; }

  void setRotation(double r) { rt = r; }

  void setMatrix(Matrix m) { mt = m; }
  
private:
  // OPMMT es el caso inerte de Transform::draw(): un Transform nunca
  // configurado no debe alterar ninguna matriz.
  MatrixOperation op = OPMMT;
  PredefinedMatrix predefmat = MTLC;
  point tl;
  double rt = 0;
  Matrix mt;
};

/** 
   Immediate modifications to the graphics state, not included in attributes 
   nor transformations.
 */
class GraphicsState : public GraphicsItem {
public:
  GraphicsState() : GraphicsItem(GI_STATE) { }
  GraphicsState(GraphicsStateType gs) : GraphicsItem(GI_STATE) {
    gstype = gs;
  }

  void draw(Display &) override;

  void setGSType(GraphicsStateType gs) { gstype = gs; }
  void setPosition(point p) { position = p; gstype = GS_PLUMEPOSITION; }

  point getPosition() const { return position; }
  GraphicsStateType getGraphicsStateType() const { return gstype; }
  
private:
  // GS_CLOSEPATH es inocuo con el estado inicial (openpath ya es false);
  // el parser siempre fija el tipo real antes de dibujar.
  GraphicsStateType gstype = GS_CLOSEPATH;
  point position;
};

#endif

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
  GI_POLYBAR           // los valores nuevos van AL FINAL: hay código que depende de los ordinales
};

enum AttributeType {
  AT_LWIDTH,
  AT_LSTYLE,
  AT_LCOLOR,
  AT_LGRAY,
  AT_FCOLOR,
  AT_FGRAY,
  AT_FPATRN,
  AT_THEIGHT,
  AT_TALIGN,
  AT_TSTYLE,
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

  void draw(Display &) override;
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

class Dot : public GraphicsItemWithPath {
public:
  Dot() : GraphicsItemWithPath(GI_DOT) { }

  void draw(Display &) override;

  void setRadius(double x) { r = x;  }

private:
  double r = 1;
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

private:
  // GS_CLOSEPATH es inocuo con el estado inicial (openpath ya es false);
  // el parser siempre fija el tipo real antes de dibujar.
  GraphicsStateType gstype = GS_CLOSEPATH;
  point position;
};

#endif

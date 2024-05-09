/*
       File:  primitives.h
              Definition of graphic items like primitives, atributes and 
              transformations.
MetaGrafica:  Human descriptive language to generate publication quality graphics.
              Display in PostScript.
     Author:  Alejandro Aguilar Sierra, UNAM
    Version:  2024
Antecedents: 2011, 1999 C++ STL, 1991 C. Original: 1988, Pascal and Assembler.
*/
#if !defined(__PRIMITIVES_H)
#define __PRIMITIVES_H

#include <list>

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
  GI_CONCATENATEPATH
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
};

/// Predefined Matrices
enum PredefinedMatrix {
  MTLC, // Local
  MTST, // Structura
  MTPP, // Posicion de pluma
  MTPT, // Path
  MTMK,
};

/// Vectorial pixel, not vector because we don't define all vector operations
struct point {
  point() { x =0; y = 0; }
  point(float x1, float y1) { x = x1; y = y1; }
  point (const point&p) { x = p.x; y = p.y; }
  float x, y;
  point operator*(const double& f) {
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
  bool operator==(const point& p2) {
   return (x==p2.x && y==p2.y);
  }
  bool operator!=(const point& p2) {
    return (x!=p2.x || y!=p2.y);
  }
};


using Path = std::list<point>;

// Abstract output device
class Display;


/** 
   Abstract class to define all members of a graphics sequence.
 */
class GraphicsItem {
public:

  GraphicsItem(GraphicsItemType t) : type(t) { }
  virtual ~GraphicsItem()=default;
  
  int getType() const { return type; }
  
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
  void setPath(Path p) { path = p; }
  Path getPath() { return path; }
protected:
  Path path;
};


class Polyline : public GraphicsItemWithPath {
public:
  Polyline(GraphicsItemType t=GI_POLYLINE) : GraphicsItemWithPath(t) { }
  
  void draw(Display &);
};


/* 
   A rectangle is defined by a pair of points: the left lower point and the right upper point.
 */
class Rectangle : public GraphicsItemWithPath {
public:
  Rectangle() : GraphicsItemWithPath(GI_RECTANGLE) { }

  void draw(Display &);
};

class Arc : public GraphicsItemWithPath {
public:
  Arc() : GraphicsItemWithPath(GI_ARC) { ai = 0; af = 360; }

  void draw(Display &);

  void setRadius(float x, float y) { rx = x; ry = y; }

  void setAngles(float x, float y) { ai = x; af = y; }

private:
  float rx, ry;
  float ai, af;
};

class Dot : public GraphicsItemWithPath {
public:
  Dot() : GraphicsItemWithPath(GI_DOT) { r = 1; }

  void draw(Display &);

  void setRadius(float x) { r = x;  }

private:
  float r;
};

class Attribute : public GraphicsItem {
public:
  Attribute() : GraphicsItem(GI_ATTRIBUTE) {}
  
  void draw(Display &);
  
  void set(AttributeType t, int v) { type = t; value = v; }

private:
  int value;
  AttributeType type;
};

class Transform : public GraphicsItem {
public:
  Transform() : GraphicsItem(GI_TRANSFORM) {}

  void draw(Display &);

  void setOperation(MatrixOperation o) { op = o; }

  MatrixOperation getOperation() { return op; }

  void setPredefinedMatrix(PredefinedMatrix pdmt) { predefmat = pdmt; }

  PredefinedMatrix getPredefinedMatrix() { return predefmat; }

  void setPoint(point p) { tl = p; }

  void setRotation(float r) { rt = r; }
  
private:
  MatrixOperation op;
  PredefinedMatrix predefmat;
  union {
    point tl;
    point sc;
    float rt;
  };
};

/** 
   Immediate modifications to the graphics state, not included in attributes 
   nor transformations.
 */
class GraphicsState : public GraphicsItem {
public:
  GraphicsState() : GraphicsItem(GI_STATE) { }

  void draw(Display &);

  void setGSType(GraphicsStateType gs) { gstype = gs; }
  void setPosition(point p) { position = p; gstype = GS_PLUMEPOSITION; }

private:
  GraphicsStateType gstype;
  point position;
};

#endif

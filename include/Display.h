/*
       File:  display.h
              Display is the final output of the graphics. In this file
              we define its abstract base class, properties and methods.
MetaGrafica:  Human descriptive language to generate publication quality
              Display in PostScript.
     Author:  Alejandro Aguilar Sierra, UNAM
    Version:  2024
Antecedents: 2011, 1999 C++ STL, 1991 C. Original: 1988, Pascal and Assembler.
*/

#if !defined(__DISPLAY_H)
#define __DISPLAY_H

#include <stack>
#include <string>
using std::string;

class MetaGrafica;

constexpr float point_to_cm =  0.03527777777777;
constexpr float cm_to_point = 28.34645669291339;

#include "primitives.h"
#include "text.h"


/** 
   Important information relevant to the graphics output.
 */
struct DisplayState {
  DisplayState() {
    fill = false;
    line_style = 0;
    line_width = 0;
    fontSize = 10;
    fontFace = FN_NOFACE;
    fillpattern = 0;
    outlinefill = false;
    openpath = false;
    text_align = 0;
    fillgray = 0.0;
    linegray = 0;
    linecolor = 0;
    fillcolor = 0;
	textgray = 0;
  };
  
  // path under creation?
  bool openpath;

  int line_style;
  int line_width;
  float linegray;
  int linecolor;

  float fontSize;
  FontFace fontFace;
  int text_align;
  float textgray;

  // objects should be filled?
  bool fill;
  int fillpattern;
  bool outlinefill;
  float fillgray;
  int fillcolor;
};

/** 
   Abstract base class for physical output.
 */
class Display {
public:
  Display() {
    dvx = 10;
    dvy = 10;
    ratio = 1;
    max_fillpattern = 10;
  }

  virtual void start()=0;

  virtual void end()=0;

  virtual void moveto_nopath(float, float)=0;

  virtual void moveto(float, float)=0;

  virtual void rmoveto(float, float)=0;

  virtual void lineto(float, float)=0;

  virtual void rlineto(float, float)=0;

  virtual void line(float, float, float, float)=0;

  virtual void arc(float x, float y, float rx, float ry, float startAng, float arcAng)=0;

  virtual void dot(float x, float y, float r)=0;

  virtual void rect(float x1, float y1, float x2, float y2)=0;

  virtual void curveto(float x1, float y1, float x2, float y2, float x3, float y3)=0;

  virtual void structureDefBegin(string name)=0;

  virtual void structureDefEnd()=0;

  void structure(string name);   // rama común de los backends; en Display.cpp

  virtual void stroke()=0;

  // Transformaciones: la rama MTST es contabilidad común de la máquina de
  // estado; la rama MTLC se delega al hook device* de cada backend. Otros
  // valores de PredefinedMatrix no tienen efecto aquí (los maneja el parser).
  void translate(float x, float y, PredefinedMatrix pdmt=MTLC) {
    if (pdmt == MTLC) deviceTranslate(x, y);
    else if (pdmt == MTST) mtst.translate(x, y);
  }

  void scale(float x, float y, PredefinedMatrix pdmt=MTLC) {
    if (pdmt == MTLC) deviceScale(x, y);
    else if (pdmt == MTST) mtst.scale(x, y);
  }

  void shear(float x, float y, PredefinedMatrix pdmt=MTLC) {
    if (pdmt == MTLC) deviceShear(x, y);
    else if (pdmt == MTST) mtst.shear(x, y);
  }

  void rotate(float angle, PredefinedMatrix pdmt=MTLC) {
    if (pdmt == MTLC) deviceRotate(angle);
    else if (pdmt == MTST) mtst.rotate(angle);
  }

  void compose(Matrix m, PredefinedMatrix pdmt=MTLC) {
    if (pdmt == MTST) mtst = mtst * m;
  }

  void init_matrix(PredefinedMatrix pdmt=MTLC) {
    if (pdmt == MTLC) deviceInitMatrix();
    else if (pdmt == MTST) mtst.initialize();
  }

  void setDimension(float x, float y) { dvx = x; dvy = y; ratio = dvy/dvx; }
  float getRatio() { return ratio; }

  void setPlumePosition(point &p) { pp = p; }
  void getPlumePosition(point &p) { p = pp; }
  virtual void setOpenPath(bool op) { dspstate.openpath = op; }
  
  bool isFilled() { return dspstate.fill; }
  void setFilled(bool m) { dspstate.fill = m; }
  
  virtual void setLineStyle(int ls) { dspstate.line_style = ls; }
  virtual void setLineWidth(float lw)=0;
  void setLineGray(float lg) { dspstate.linegray = lg; }
  virtual void setLineColor(int lc) { dspstate.linecolor = lc; }

  virtual void setFillPattern(int fp) {
    dspstate.outlinefill = (fp < 0);
    dspstate.fillpattern = abs(fp);
  }
  
  void setFillGray(float fg) { 
    dspstate.outlinefill = (fg < 0);
    dspstate.fillgray = fabs(fg);
    dspstate.fillcolor = 0;
  }

  void setFillColor(int fc) { 
    dspstate.outlinefill = (fc < 0);
    dspstate.fillcolor = abs(fc);
    dspstate.fillgray = 0.0;
  }

  virtual void text(string)=0;

  virtual void setFontFace(FontFace face) { dspstate.fontFace = face; }
  virtual void setFontSize(float p) { dspstate.fontSize = p; }
  float getFontSize() { return dspstate.fontSize; }
  void setRelFontSize(float rfz) {
    if (rfz == relfontsize)
      return;
    relfontsize = rfz;
    dspstate.fontFace = FN_NOFACE;
  }

  void setTextAlign(int a) { dspstate.text_align = a; }
  int getTextAlign() { return dspstate.text_align; }

  void setMGContext(MetaGrafica* mg) { mg_context = mg; }

  virtual void save() = 0;
  virtual void restore() = 0;

  // Contabilidad de matrices, común a todos los backends
  void pushMatrix(Matrix &m)             { mtstack.push(mt); mt *= m; }
  void pushMatrix(PredefinedMatrix pdmt) { if (pdmt == MTST) { mtstack.push(mtst); mt *= mtst; } }
  void saveMatrix(PredefinedMatrix pdmt) { if (pdmt == MTST) mtstack.push(mtst); }
  void popMatrix()                       { mt = mtstack.top(); mtstack.pop(); }
  void popMatrix(PredefinedMatrix pdmt)  { if (pdmt == MTST) mtst = mtstack.top(); mtstack.pop(); }

  // Physical limitations for patterns
  int max_fillpattern;
  
protected:
  /// Hooks de la rama MTLC: solo la parte específica del dispositivo.
  /// El backend no ve (ni puede corromper) la pila de matrices.
  virtual void deviceTranslate(float x, float y) = 0;
  virtual void deviceScale(float x, float y) = 0;
  virtual void deviceShear(float x, float y) = 0;
  virtual void deviceRotate(float angle) = 0;
  virtual void deviceInitMatrix() {}

  /// Output Device properties

  // Dimensions of the vision port in cms
  float dvx, dvy;


  /// Graphics state
  DisplayState dspstate;

  // Current plume position
  point pp;

  /// Razon de aspecto
  float ratio;

  /// Contexto para resolver estructuras por nombre
  MetaGrafica* mg_context = nullptr;

  /// Tamaño relativo de fuente (sub/superíndices del texto)
  float relfontsize = 1.0f;

  /// Matriz acumulada (semilla dvx/dvy de start() + matrices MTST horneadas)
  Matrix mt;
  /// Matriz de estructuras
  Matrix mtst;
  std::stack<Matrix> mtstack;
  std::stack<DisplayState> dsstack;
};

#endif

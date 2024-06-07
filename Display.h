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

#include <string>
using std::string;

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

  virtual void structure(string name)=0;

  virtual void stroke()=0;

  virtual void translate(float x, float y, PredefinedMatrix pdmt=MTLC)=0;

  virtual void scale(float x, float y, PredefinedMatrix pdmt=MTLC)=0;
  
  virtual void rotate(float angle, PredefinedMatrix pdmt=MTLC)=0;

  virtual void compose(Matrix mt, PredefinedMatrix pdmt=MTLC)=0;
  
  virtual void init_matrix(PredefinedMatrix mt=MTLC)=0;

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
  virtual void setRelFontSize(float r) = 0;

  void setTextAlign(int a) { dspstate.text_align = a; }
  int getTextAlign() { return dspstate.text_align; }

  virtual void save() = 0;
  virtual void restore() = 0;
  virtual void pushMatrix(Matrix &) = 0;
  virtual void pushMatrix(PredefinedMatrix) = 0;
  virtual void saveMatrix(PredefinedMatrix) = 0;
  virtual void popMatrix() = 0;
  virtual void popMatrix(PredefinedMatrix) = 0;

protected:
  /// Output Device properties

  // Dimensions of the vision port in cms
  float dvx, dvy;

  /// Graphics state
  DisplayState dspstate;

  // Current plume position
  point pp;

  /// Razon de aspecto
  float ratio;
};

#endif

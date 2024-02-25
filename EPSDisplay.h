/*
       File:  EPSDisplay.h
              Implementation of graphics items like primitives, atributes and 
              transformations in Encapsulated PostScript.
MetaGrafica:  Human descriptive language to generate publication quality
              Display in PostScript.
     Author:  Alejandro Aguilar Sierra, UNAM
    Version:  2024
Antecedents: 2011, 1999 C++ STL, 1991 C. Original: 1988, Pascal and Assembler.
*/
#if !defined(__EPSDISPLAY_H)
#define __EPSDISPLAY_H 

#include <stdio.h>
#include <stack>
using std::stack;

#include "Display.h"

class EPSDisplay: public Display {
   
 public:
   ///
   EPSDisplay(string);
   /// 
   void start();
   ///
   void end();
   /// 
   void stroke();      
   ///
   void save();
   ///
   void restore();
   ///
   void setLineStyle(int);
   ///
   void setLineWidth(float);
   ///
   void setFillGray(float fg);
   void useFillPattern();
   ///
   void setOpenPath(bool op);
   
   void setLogFile(FILE *f) { logfile = f; }
  ///
  void structureDefBegin(std::string);
  ///
  void structureDefEnd();
  ///
  void structure(std::string);
  ///
  void translate(float x, float y, PredefinedMatrix mt=MTLC);
  ///
  void scale(float x, float y, PredefinedMatrix mt=MTLC);
  ///
  void rotate(float angle, PredefinedMatrix mt=MTLC);
  ///
  void init_matrix(PredefinedMatrix mt=MTLC);

  void pushMatrix(Matrix &);
  void pushMatrix(PredefinedMatrix);
  void popMatrix();

 protected: 
  void moveto_nopath(float, float);
  void moveto(float, float);
  void rmoveto(float, float);
  void lineto(float, float);
  void rlineto(float, float);
  void line(float, float, float, float);
  void rline(float, float, float, float);
  void rect(float, float, float, float);
  void curveto(float, float, float, float, float, float);
  void text(string);
  void setFontSize(float p);
  void setFontFace(FontFace face);
  void arc(float x, float y, float rx,float ry, float startAng, float endAng);
  void dot(float x, float y, float r);
  void setRelFontSize(float rfz) {
    if (rfz==relfontsize)
    return;
    relfontsize = rfz; 
    dspstate.fontFace = FN_NOFACE; 
  }
  void getTextSize(string s, float *w, float *h);
  void set_limits(float x1, float y1, float x2, float y2) {
    xmin = x1;
    xmax = x2;
    ymin = y1;
    ymax = y2;
  }
  void adjust_limits(float x1, float y1, float x2, float y2) {
    if (x1 < xmin)
      xmin = x1;
    if (x2 > xmax)
      xmax = x2;
    if (y1 < ymin)
      ymin = y1;
    if (y2 > ymax)
      ymax = y2;
  }
 private:
  string filename;
  FILE *file;
  FILE *logfile;
  float xmin, xmax, ymin, ymax;
  float relfontsize;

  // Local first level matrix
  Matrix mt;
  // Matriz para las estructuras
  Matrix mtst;
  stack<Matrix> mtstack;
  stack<DisplayState> dsstack;
};

#endif

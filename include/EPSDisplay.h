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

#include "Display.h"
#include "mgflags.h"

class EPSDisplay: public Display {

 public:
  EPSDisplay(string);

  MGFlags flags;

  ~EPSDisplay() {
    if (file) { fflush(file); fclose(file); file = nullptr; }
  }

  void start() override;
  void end() override;
  void stroke() override;
  void save() override;
  void restore() override;

  void setLineStyle(int) override;
  void setLineWidth(float) override;
  void setLineColor(int lc) override;
  void setOpenPath(bool op) override;

  void setLogFile(FILE *f) { logfile = f; }

  void structureDefBegin(std::string) override;
  void structureDefEnd() override;

  // EPSDisplay-specific
  void useFillPattern();
  void setColor(int lc);
  void setGray(float fg);

 protected:
  void moveto_nopath(float, float) override;
  void moveto(float, float) override;
  void rmoveto(float, float) override;
  void lineto(float, float) override;
  void rlineto(float, float) override;
  void line(float, float, float, float) override;
  void rect(float, float, float, float) override;
  void curveto(float, float, float, float, float, float) override;
  void text(string) override;
  void setFontSize(float p) override;
  void setFontFace(FontFace face) override;
  void arc(float x, float y, float rx, float ry, float startAng, float endAng) override;
  void dot(float x, float y, float r) override;

  void deviceTranslate(float x, float y) override;
  void deviceScale(float x, float y) override;
  void deviceShear(float x, float y) override;
  void deviceRotate(float angle) override;
  void deviceInitMatrix() override;

  // EPSDisplay-specific
  void rline(float, float, float, float);
  void getTextSize(string s, float *w, float *h);
  void set_limits(float x1, float y1, float x2, float y2) {
    xmin = x1;
    xmax = x2;
    ymin = y1;
    ymax = y2;
  }
  void adjust_limits(float x1, float y1, float x2, float y2) {
    if (x1 < xmin) xmin = x1;
    if (x2 > xmax) xmax = x2;
    if (y1 < ymin) ymin = y1;
    if (y2 > ymax) ymax = y2;
  }

 private:
  string filename;
  FILE *file = nullptr;
  FILE *logfile = nullptr;
  float xmin, xmax, ymin, ymax;
};

#endif

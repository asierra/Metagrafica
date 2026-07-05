/*
       File:  EPSDisplay.h
              Implementation of graphics items like primitives, atributes and
              transformations in Encapsulated PostScript.
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
#if !defined(MG_EPSDISPLAY_H)
#define MG_EPSDISPLAY_H

#include <stdio.h>

#include "Display.h"
#include "mgflags.h"

class EPSDisplay: public Display {

 public:
  EPSDisplay(std::string);

  MGFlags flags;

  ~EPSDisplay() {
    if (file) { fflush(file); fclose(file); file = nullptr; }
  }

  void start() override;
  void end() override;
  void stroke() override;
  void save() override;
  void restore() override;

  void setLineWidth(double) override;
  void setLineColor(int lc) override;
  void setOpenPath(bool op) override;

  void setLogFile(FILE *f) { logfile = f; }

  void structureDefBegin(std::string) override;
  void structureDefEnd() override;

  // EPSDisplay-specific
  void useFillPattern();
  void setColor(int lc);
  void setGray(double fg);

 protected:
  void moveto_nopath(double, double) override;
  void moveto(double, double) override;
  void rmoveto(double, double) override;
  void lineto(double, double) override;
  void rlineto(double, double) override;
  void line(double, double, double, double) override;
  void rect(double, double, double, double) override;
  void curveto(double, double, double, double, double, double) override;
  void text(std::string) override;
  void setFontSize(double p) override;
  void setFontFace(FontFace face) override;
  void arc(double x, double y, double rx, double ry, double startAng, double endAng) override;
  void dot(double x, double y, double r) override;

  void deviceTranslate(double x, double y) override;
  void deviceScale(double x, double y) override;
  void deviceShear(double x, double y) override;
  void deviceRotate(double angle) override;
  void deviceInitMatrix() override;

  void applyDash() override;
  void applyLineCap() override;
  void applyLineJoin() override;

  // EPSDisplay-specific
  void rline(double, double, double, double);
  void getTextSize(std::string s, double *w, double *h);
  // set_limits/adjust_limits y el bbox (xmin..ymax) viven ahora en la base Display.

 private:
  std::string filename;
  FILE *file = nullptr;
  FILE *logfile = nullptr;
};

#endif

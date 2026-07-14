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

#include <stack>
#include <utility>

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

  void closepath() override;
  void save() override;
  void restore() override;

  void pushDrawState() override;
  void popDrawState() override;

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
  void marker(double x, double y, const MarkerShape &shape, double size, double dirx, double diry) override;

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

  // El proc PostScript /ellipse se define en el prólogo cuando flags.using_ellipse
  // (bandera de parse-time, aproximación conservadora). Pero la decisión real de
  // emitir el operador `ellipse` la toma arc() al comparar las normas de columna
  // del CTM (p. ej. fit(stretch=true) sobre una struct con círculos), caso que la
  // bandera no cubre. Para no depender de esa aproximación, arc() define el proc
  // just-in-time en su primer uso si aún no se emitió; este bool evita redefinirlo.
  bool ellipse_defined = false;

  // Caché del estado de fuente REALMENTE emitido al dispositivo (findfont…setfont),
  // para que setFontFace no re-emita un setfont idéntico. La FUENTE es parte del
  // estado gráfico de PostScript: cada gsave/grestore la salva/restaura. Por eso
  // el caché se apila en lockstep con gsave (pushDevFont) y se restaura con
  // grestore (popDevFont) en save/restore y push/popDrawState: si no, tras un
  // grestore que revierte la fuente del dispositivo el caché quedaría obsoleto y
  // el guard omitiría el setfont del siguiente texto (que saldría con la fuente
  // vieja). El gsave/grestore crudo de useFillPattern no cambia la fuente, así
  // que queda balanceado sin tocar el caché.
  FontFace dev_face = FN_NOFACE;
  double dev_size = -1.0;
  std::stack<std::pair<FontFace, double>> dev_font_stack;
  void pushDevFont() { dev_font_stack.push({dev_face, dev_size}); }
  void popDevFont() {
    if (dev_font_stack.empty()) return;
    dev_face = dev_font_stack.top().first;
    dev_size = dev_font_stack.top().second;
    dev_font_stack.pop();
  }
};

#endif

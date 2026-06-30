/*
       File:  PDFDisplay.h
              Implementation of graphics items in PDF via libharu.
MetaGrafica:  Human descriptive language to generate publication quality
              graphics.
     Author:  Alejandro Aguilar Sierra, UNAM
    Version:  2024
*/
#if !defined(__PDFDISPLAY_H)
#define __PDFDISPLAY_H

#include <stack>
using std::stack;

#include "Display.h"
#include "hpdf.h"

class PDFDisplay : public Display {

public:
  PDFDisplay(string filename);
  ~PDFDisplay() {}

  void start() override;
  void end() override;
  void stroke() override;
  void save() override;
  void restore() override;

  void setLineStyle(int) override;
  void setLineWidth(float) override;
  void setLineColor(int lc) override;
  void setOpenPath(bool op) override;

  void structureDefBegin(std::string) override {}
  void structureDefEnd() override {}
  void structure(std::string) override;

  void translate(float x, float y, PredefinedMatrix pdmt=MTLC) override;
  void scale(float x, float y, PredefinedMatrix pdmt=MTLC) override;
  void shear(float x, float y, PredefinedMatrix pdmt=MTLC) override;
  void rotate(float angle, PredefinedMatrix pdmt=MTLC) override;
  void compose(Matrix mt, PredefinedMatrix pdmt=MTLC) override;
  void init_matrix(PredefinedMatrix mt=MTLC) override;

  void pushMatrix(Matrix &) override;
  void pushMatrix(PredefinedMatrix) override;
  void saveMatrix(PredefinedMatrix) override;
  void popMatrix() override;
  void popMatrix(PredefinedMatrix) override;

  void setMGContext(MetaGrafica* mg) override { mg_context = mg; }

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
  void setRelFontSize(float rfz) override {
    if (rfz == relfontsize) return;
    relfontsize = rfz;
    dspstate.fontFace = FN_NOFACE;
  }

private:
  void applyFillColor();
  void applyStrokeColor();
  void prepareDraw();  // aplica colores antes de iniciar un path (PAGE_DESCRIPTION)

  string filename;

  HPDF_Doc  pdf  = nullptr;
  HPDF_Page page = nullptr;

  MetaGrafica* mg_context = nullptr;

  float relfontsize = 1.0f;

  // Current position in page space (tracked for rmoveto/rlineto)
  float cur_x = 0.0f, cur_y = 0.0f;

  // Current font handle and dirty flag
  HPDF_Font current_font = nullptr;

  // Face name for embedded CMMI TTF (returned by HPDF_LoadTTFontFromMemory)
  const char* cmmi_face = nullptr;

  // C++ transform matrices (mirrors EPSDisplay)
  Matrix mt;
  Matrix mtst;
  stack<Matrix>       mtstack;
  stack<DisplayState> dsstack;
};

#endif

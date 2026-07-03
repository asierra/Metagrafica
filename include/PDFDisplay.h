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

private:
  void applyFillColor();
  void applyStrokeColor();
  void prepareDraw();  // aplica colores antes de iniciar un path (PAGE_DESCRIPTION)

  string filename;

  HPDF_Doc  pdf  = nullptr;
  HPDF_Page page = nullptr;

  // Current position in page space (tracked for rmoveto/rlineto)
  float cur_x = 0.0f, cur_y = 0.0f;

  // Current font handle and dirty flag
  HPDF_Font current_font = nullptr;

  // Face name for embedded CMMI TTF (returned by HPDF_LoadTTFontFromMemory)
  const char* cmmi_face = nullptr;
};

#endif

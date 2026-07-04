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

  void setLineWidth(double) override;
  void setLineColor(int lc) override;
  void setOpenPath(bool op) override;

  void structureDefBegin(std::string) override {}
  void structureDefEnd() override {}

protected:
  void moveto_nopath(double, double) override;
  void moveto(double, double) override;
  void rmoveto(double, double) override;
  void lineto(double, double) override;
  void rlineto(double, double) override;
  void line(double, double, double, double) override;
  void rect(double, double, double, double) override;
  void curveto(double, double, double, double, double, double) override;
  void text(string) override;
  void setFontSize(double p) override;
  void setFontFace(FontFace face) override;
  void arc(double x, double y, double rx, double ry, double startAng, double endAng) override;
  void dot(double x, double y, double r) override;

  void deviceTranslate(double x, double y) override;
  void deviceScale(double x, double y) override;
  void deviceShear(double x, double y) override;
  void deviceRotate(double angle) override;

  void applyDash() override;
  void applyLineCap() override;
  void applyLineJoin() override;

private:
  void applyFillColor();
  void applyStrokeColor();
  void prepareDraw();  // aplica colores antes de iniciar un path (PAGE_DESCRIPTION)

  // Relleno con patrón de tramado. libharu exige GSave en PAGE_DESCRIPTION (no
  // dentro de un path), así que se abre antes de construir el path y se cierra
  // tras el recorte. ensurePatternGSave() lo abre una sola vez; hatchCurrentPath()
  // recorta al path actual (operador W) y dibuja las líneas dentro del bbox.
  void ensurePatternGSave();
  void hatchCurrentPath();
  bool clip_pending = false;

  string filename;

  HPDF_Doc  pdf  = nullptr;
  HPDF_Page page = nullptr;

  // Current position in page space (tracked for rmoveto/rlineto)
  double cur_x = 0.0f, cur_y = 0.0f;

  // Current font handle and dirty flag
  HPDF_Font current_font = nullptr;

  // Face name for embedded CMMI TTF (returned by HPDF_LoadTTFontFromMemory)
  const char* cmmi_face = nullptr;
};

#endif

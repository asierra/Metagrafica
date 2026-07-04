/*
       File:  SVGDisplay.h
              Implementation of graphics items like primitives, attributes and
              transformations in Scalable Vector Graphics (SVG).
MetaGrafica:  Human descriptive language to generate publication quality
              Display in PostScript.
     Author:  Alejandro Aguilar Sierra, UNAM / IA Assistant
    Version:  2026
*/
#if !defined(__SVGDISPLAY_H)
#define __SVGDISPLAY_H

#include <stdio.h>
#include <stack>
#include <sstream>
#include <string>
#include <set>

#include "Display.h"
#include "mgflags.h"

class SVGDisplay: public Display {

 public:
  SVGDisplay(string filename);

  MGFlags flags;

  ~SVGDisplay() {
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

  void structureDefBegin(std::string) override;
  void structureDefEnd() override;

 protected:
  // Primitivas de dibujo
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

 private:
  // Helpers internos para generar el estilo SVG basado en dspstate
  std::string getStyleAttributes();
  std::string escapeXML(const std::string& data);
  // Convierte un run de texto a UTF-8 seguro para XML según la cara de fuente
  // actual: para FN_SYMBOL/FN_TEX_CMMI traduce los bytes a caracteres Unicode
  // (letras griegas, símbolos matemáticos); para el resto reinterpreta Latin-1.
  std::string renderText(const std::string& s);
  // Ancho de trazo en puntos para el estado actual (ver definición).
  double strokeWidth();
  // Color de relleno como hex de 6 dígitos (sin '#'), desde fillcolor/fillgray.
  std::string fillColorHex();
  // Garantiza que exista un <pattern> de tramado para el índice/color actual
  // (lo emite en un <defs> la primera vez) y devuelve su id. "" si idx<=0.
  std::string ensureHatchPattern(int idx);

  string filename;
  FILE *file = nullptr;

  // Posición actual (en espacio ya transformado), para <text x= y=>,
  // igual que cur_x/cur_y en PDFDisplay.
  double cur_x = 0, cur_y = 0;

  std::ostringstream path_builder; // Búfer para acumular los comandos del path actual
  std::stack<int> group_stack;     // Rastrea cuántas etiquetas <g> abrir por cada save()
  int current_open_groups = 0;     // Etiquetas <g> abiertas en el entorno actual
  std::set<std::string> emitted_patterns; // ids de <pattern> ya emitidos (dedup)
};

#endif

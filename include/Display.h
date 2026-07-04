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
#include <vector>
using std::string;

class MetaGrafica;

constexpr double point_to_cm =  0.03527777777777;
constexpr double cm_to_point = 28.34645669291339;

#include "primitives.h"
#include "text.h"


/** 
   Important information relevant to the graphics output.
 */
struct DisplayState {
  DisplayState() {
    fill = false;
    line_width_pt = 1.0f;
    line_width_set = false;
    line_cap = 0;
    line_join = 0;
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

  // Grosor en PUNTOS tipográficos (no en unidades de 0.2 pt de V1).
  double line_width_pt;
  // ¿Se fijó explícitamente el grosor? Distingue el default (1.0 pt, el del
  // dispositivo PS/PDF cuando nunca se llama LWIDTH) de un LWIDTH 0 explícito.
  bool line_width_set;
  // Patrón de guiones: longitudes on/off alternadas en pt. Vacío = línea continua.
  // Única fuente de verdad del patrón; ver Display::dashArrayForIndex().
  std::vector<double> dash_array;
  // 0 butt, 1 round, 2 square. Dormido en V1: ninguna ruta lo invoca todavía.
  int line_cap;
  // 0 miter, 1 round, 2 bevel. Dormido en V1: ninguna ruta lo invoca todavía.
  int line_join;
  double linegray;
  int linecolor;

  double fontSize;
  FontFace fontFace;
  int text_align;
  double textgray;

  // objects should be filled?
  bool fill;
  int fillpattern;
  bool outlinefill;
  double fillgray;
  int fillcolor;
};

/**
   Descriptor de patrón de relleno, independiente del dispositivo. Sigue el
   modelo del formato .pat de AutoCAD: un patrón es una o más familias de
   líneas paralelas. Una sola familia = rayado simple; dos = rayado cruzado;
   con `dashes` no vacío = líneas punteadas/de guiones. Cada backend traduce
   este descriptor a su mecanismo nativo (EPS/PDF: clip + líneas; SVG: <pattern>).
 */
struct HatchLine {
  double angle = 0.0f;          // dirección de las líneas (grados)
  double gap   = 0.0f;          // separación perpendicular entre líneas (pt)
  double ox = 0.0f, oy = 0.0f;  // origen/fase de la familia
  std::vector<double> dashes;   // vacío = línea continua
};
using FillPattern = std::vector<HatchLine>;

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

  virtual void moveto_nopath(double, double)=0;

  virtual void moveto(double, double)=0;

  virtual void rmoveto(double, double)=0;

  virtual void lineto(double, double)=0;

  virtual void rlineto(double, double)=0;

  virtual void line(double, double, double, double)=0;

  virtual void arc(double x, double y, double rx, double ry, double startAng, double arcAng)=0;

  virtual void dot(double x, double y, double r)=0;

  virtual void rect(double x1, double y1, double x2, double y2)=0;

  virtual void curveto(double x1, double y1, double x2, double y2, double x3, double y3)=0;

  virtual void structureDefBegin(string name)=0;

  virtual void structureDefEnd()=0;

  void structure(string name);   // rama común de los backends; en Display.cpp

  virtual void stroke()=0;

  // Transformaciones: la rama MTST es contabilidad común de la máquina de
  // estado; la rama MTLC se delega al hook device* de cada backend. Otros
  // valores de PredefinedMatrix no tienen efecto aquí (los maneja el parser).
  void translate(double x, double y, PredefinedMatrix pdmt=MTLC) {
    if (pdmt == MTLC) deviceTranslate(x, y);
    else if (pdmt == MTST) mtst.translate(x, y);
  }

  void scale(double x, double y, PredefinedMatrix pdmt=MTLC) {
    if (pdmt == MTLC) deviceScale(x, y);
    else if (pdmt == MTST) mtst.scale(x, y);
  }

  void shear(double x, double y, PredefinedMatrix pdmt=MTLC) {
    if (pdmt == MTLC) deviceShear(x, y);
    else if (pdmt == MTST) mtst.shear(x, y);
  }

  void rotate(double angle, PredefinedMatrix pdmt=MTLC) {
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

  void setDimension(double x, double y) { dvx = x; dvy = y; ratio = dvy/dvx; }
  double getRatio() { return ratio; }

  void setPlumePosition(point &p) { pp = p; }
  void getPlumePosition(point &p) { p = pp; }
  virtual void setOpenPath(bool op) { dspstate.openpath = op; }
  
  bool isFilled() { return dspstate.fill; }
  void setFilled(bool m) { dspstate.fill = m; }
  
  // Traduce el índice de patrón (V1: LPATRN n) al arreglo on/off en puntos.
  // Única fuente de verdad para los cinco patrones clásicos; V2 la reutiliza
  // para sus alias ("dashed", "dotted", ...). idx 0 o 1 -> línea continua
  // (en el motor V1, LPATRN 1 nunca se distinguió de LPATRN 0).
  static std::vector<double> dashArrayForIndex(int idx) {
    switch (idx) {
      case 2:  return {4, 2};
      case 3:  return {2, 1.6f};
      case 4:  return {4, 2, 1, 2};
      case 5:  return {4, 2, 2, 2, 2, 2};
      default: return {};
    }
  }

  virtual void setLineStyle(int ls) {
    dspstate.dash_array = dashArrayForIndex(ls);
    applyDash();
  }
  virtual void setLineCap(int c) { dspstate.line_cap = c; applyLineCap(); }
  virtual void setLineJoin(int j) { dspstate.line_join = j; applyLineJoin(); }
  virtual void setLineWidth(double lw)=0;
  void setLineGray(double lg) { dspstate.linegray = lg; }
  virtual void setLineColor(int lc) { dspstate.linecolor = lc; }

  virtual void setFillPattern(int fp) {
    dspstate.outlinefill = (fp < 0);
    dspstate.fillpattern = abs(fp);
  }

  // Traduce el índice de patrón al descriptor común. idx 0 (o negativo) =
  // relleno sólido → devuelve vector vacío. La tabla actual son 10 patrones
  // de rayado unidireccional (4 ángulos x 3 densidades); reproduce byte a byte
  // la aritmética ENTERA del EPSDisplay original.
  virtual FillPattern patternFor(int idx) {
    FillPattern fp;
    if (idx <= 0) return fp;
    double angle = (double)(((idx - 1) * 45) % 180);
    double gap   = (double)(4 / (1 + (idx - 1) / 4));  // división entera, a propósito
    fp.push_back(HatchLine{angle, gap});
    return fp;
  }
  
  void setFillGray(double fg) { 
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
  virtual void setFontSize(double p) { dspstate.fontSize = p; }
  double getFontSize() { return dspstate.fontSize; }
  void setRelFontSize(double rfz) {
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
  virtual void deviceTranslate(double x, double y) = 0;
  virtual void deviceScale(double x, double y) = 0;
  virtual void deviceShear(double x, double y) = 0;
  virtual void deviceRotate(double angle) = 0;
  virtual void deviceInitMatrix() {}

  /// Hooks de emisión del estilo de línea. No-op por defecto: SVG no los
  /// necesita (lee dspstate al construir el atributo de estilo del elemento).
  /// EPS y PDF los sobre-escriben para emitir de inmediato al backend nativo.
  virtual void applyDash() {}
  virtual void applyLineCap() {}
  virtual void applyLineJoin() {}

  /// Output Device properties

  // Dimensions of the vision port in cms
  double dvx, dvy;

  // Bounding box del path actual, en coordenadas de dispositivo. Se usa para
  // barrer líneas de tramado dentro del área rellena. Común a EPS y (futuro)
  // PDF; SVG no lo necesita (el <pattern> teja y recorta solo).
  double xmin, xmax, ymin, ymax;
  void set_limits(double x1, double y1, double x2, double y2) {
    xmin = x1;
    xmax = x2;
    ymin = y1;
    ymax = y2;
  }
  void adjust_limits(double x1, double y1, double x2, double y2) {
    if (x1 < xmin) xmin = x1;
    if (x2 > xmax) xmax = x2;
    if (y1 < ymin) ymin = y1;
    if (y2 > ymax) ymax = y2;
  }


  /// Graphics state
  DisplayState dspstate;

  // Current plume position
  point pp;

  /// Razon de aspecto
  double ratio;

  /// Contexto para resolver estructuras por nombre
  MetaGrafica* mg_context = nullptr;

  /// Tamaño relativo de fuente (sub/superíndices del texto)
  double relfontsize = 1.0f;

  /// Matriz acumulada (semilla dvx/dvy de start() + matrices MTST horneadas)
  Matrix mt;
  /// Matriz de estructuras
  Matrix mtst;
  std::stack<Matrix> mtstack;
  std::stack<DisplayState> dsstack;
};

#endif

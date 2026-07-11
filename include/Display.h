/*
       File:  display.h
              Display is the final output of the graphics. In this file
              we define its abstract base class, properties and methods.
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

#if !defined(MG_DISPLAY_H)
#define MG_DISPLAY_H

#include <stdio.h>

#include <stack>
#include <string>
#include <vector>

class MetaGrafica;

constexpr double point_to_cm =  0.03527777777777;
constexpr double cm_to_point = 28.34645669291339;

#include "primitives.h"
#include "text.h"
#include "structures.h"


/** 
   Important information relevant to the graphics output.
 */
struct DisplayState {
  // path under creation?
  bool openpath = false;

  // Grosor en PUNTOS tipográficos (no en unidades de 0.2 pt de V1).
  double line_width_pt = 1.0;
  // ¿Se fijó explícitamente el grosor? Distingue el default (1.0 pt, el del
  // dispositivo PS/PDF cuando nunca se llama LWIDTH) de un LWIDTH 0 explícito.
  bool line_width_set = false;
  // Patrón de guiones: longitudes on/off alternadas en pt. Vacío = línea continua.
  // Única fuente de verdad del patrón; ver Display::dashArrayForIndex().
  std::vector<double> dash_array;
  // 0 butt, 1 round, 2 square. Dormido en V1: ninguna ruta lo invoca todavía.
  int line_cap = 0;
  // 0 miter, 1 round, 2 bevel. Dormido en V1: ninguna ruta lo invoca todavía.
  int line_join = 0;
  double linegray = 0.0;
  int linecolor = 0;

  double fontSize = 10;
  FontFace fontFace = FN_NOFACE;
  int text_align = 0;
  int text_valign = 0;
  double textgray = 0.0;

  // objects should be filled?
  bool fill = false;
  // §4.11: familias de tramado activas (una o más HatchLine); vacío = relleno
  // sólido. Reemplaza el índice entero FPATRN (ver Display::setHatch).
  FillPattern hatch;
  bool outlinefill = false;
  double fillgray = 0.0;
  int fillcolor = 0;
};

/**
   Abstract base class for physical output.
 */
class Display {
public:
  Display() {
    dvx = 10;
    dvy = 10;
  }

  virtual ~Display() = default;

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

  // Dibuja un marcador físico (§4.11 marcadores): ancla transformada por el marco,
  // forma en unidades de dispositivo (tamaño `size`). (dirx,diry) es la tangente
  // EN MUNDO para orientar la forma; el backend la lleva a dispositivo por `mt`
  // (así respeta stretch y el flip); (0,0) = sin orientar. Relleno o contorno
  // según el estado y si la forma es rellenable (markers.h).
  virtual void marker(double x, double y, MarkerId id, double size, double dirx, double diry) = 0;

  virtual void rect(double x1, double y1, double x2, double y2)=0;

  virtual void curveto(double x1, double y1, double x2, double y2, double x3, double y3)=0;

  virtual void structureDefBegin(std::string name)=0;

  virtual void structureDefEnd()=0;

  void structure(const std::string &name);   // rama común de los backends; en Display.cpp

  virtual void stroke()=0;

  // Cierra el subtrayecto en construcción (une el último punto con el inicial).
  // Lo usa la polilínea cerrada (§4.1, closed=true) antes del stroke final, para
  // que la costura sea una esquina real y no un traslape. El relleno (polygon)
  // ya cierra por su cuenta en stroke(), así que no la necesita.
  virtual void closepath()=0;

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

  void setDimension(double x, double y) { dvx = x; dvy = y; }

  /// Ventana de mundo del documento (§3.1). Default: cuadrado unitario.
  void setWindow(double x0, double y0, double dx, double dy) {
    wwx = x0; wwy = y0; wwdx = dx; wwdy = dy;
  }

  /// stretch=true: escala por-eje al rectángulo exacto (marco de datos §13.7);
  /// default false = escala isométrica *meet* (§3.1). El front-end V1 lo dejaba
  /// en false; el V3 lo activa desde world_window ... stretch=true.
  void setStretchMode(bool b) { stretch_mode = b; }

  void setPlumePosition(point &p) { pp = p; }
  void getPlumePosition(point &p) { p = pp; }
  virtual void setOpenPath(bool op) { dspstate.openpath = op; }
  
  bool isFilled() const { return dspstate.fill; }
  void setFilled(bool m) { dspstate.fill = m; }
  void setOutlineFill(bool m) { dspstate.outlinefill = m; }
  
  // Traduce el índice de patrón (V1: LPATRN n) al arreglo on/off en puntos.
  // Única fuente de verdad para los cinco patrones clásicos; V2 la reutiliza
  // para sus alias ("dashed", "dotted", ...). idx 0 -> línea continua
  static std::vector<double> dashArrayForIndex(int idx) {
    switch (idx) {
      case 1:  return {8, 2};
      case 2:  return {4, 2};
      case 3:  return {1, 1};
      case 4:  return {4, 2, 1, 2};
      case 5:  return {4, 2, 1, 2, 1, 2};   
      case 6:  return {2, 2};
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

  // Nota: estos setters ya NO tocan outlinefill. En V1 un valor de relleno
  // negativo activaba el contorno; en V3 el contorno es estado explícito
  // (GS_OUTLINEFILL/GS_NOOUTLINEFILL, §4.11) o la convención color= por-primitiva.
  // Reintroducir el signo aquí clobbea ese estado según el orden de emisión.

  // §4.11: fija el patrón de tramado activo (una o más familias), ya construido
  // por el parser (HatchAttr). Reemplaza el setFillPattern(int) por índice: el
  // patrón viaja completo, sin pasar por una tabla restringida.
  void setHatch(const FillPattern &fp) { dspstate.hatch = fp; }


  void setFillGray(double fg) {
    dspstate.fillgray = fabs(fg);
    dspstate.fillcolor = 0;
  }

  void setFillColor(int fc) {
    dspstate.fillcolor = abs(fc);
    dspstate.fillgray = 0.0;
  }

  virtual void text(std::string)=0;

  virtual void setFontFace(FontFace face) { dspstate.fontFace = face; }
  virtual void setFontSize(double p) { dspstate.fontSize = p; }
  double getFontSize() const { return dspstate.fontSize; }
  void setRelFontSize(double rfz) {
    if (rfz == relfontsize)
      return;
    relfontsize = rfz;
    dspstate.fontFace = FN_NOFACE;
  }

  void setTextAlign(int a) { dspstate.text_align = a; }
  int getTextAlign() const { return dspstate.text_align; }
  void setTextValign(int a) { dspstate.text_valign = a; }
  int getTextValign() const { return dspstate.text_valign; }

  // Desplazamiento vertical de la pluma (unidades de dispositivo, y hacia arriba)
  // para materializar valign: mueve el ancla del texto respecto de la línea base.
  // Fracciones de la altura de fuente calibradas a ojo (no métricas reales), como
  // el centrado horizontal de §4.8. Fuente única de las constantes: la usan tanto
  // la ruta de texto compuesto (TextLine::draw) como la de texto simple (Text::draw).
  double valignShift() const {
    switch (dspstate.text_valign) {
    case 1: return -0.70 * dspstate.fontSize;   // top: el texto cuelga bajo la pluma
    case 2: return -0.30 * dspstate.fontSize;   // middle: centro ≈ media altura de caja
    case 3: return  0.20 * dspstate.fontSize;   // bottom: la base sube, el pie en la pluma
    default: return 0.0;                         // 0 = baseline (sin desplazamiento)
    }
  }

  void setMGContext(MetaGrafica* mg) { mg_context = mg; }

  virtual void save() = 0;
  virtual void restore() = 0;

  // Contabilidad de matrices, común a todos los backends
  void pushMatrix(Matrix &m)             { mtstack.push(mt); mt *= m; }
  void pushMatrix(PredefinedMatrix pdmt) { if (pdmt == MTST) { mtstack.push(mtst); mt *= mtst; } }
  // V3: apila/restaura el estado de dibujo COMPLETO (color, grosor, fill, …),
  // igual que structure(), para el ámbito de estado de los bloques (§7.1) y de los
  // atributos por-primitiva (§7.5). Restaura el estado LÓGICO (dspstate). Los
  // backends que además emiten estado al DISPOSITIVO de forma ansiosa (EPS
  // setlinewidth/setdash, PDF SetLineWidth/SetDash) SOBREESCRIBEN estos métodos
  // para envolver el ámbito en gsave/grestore (q/Q), y así el estado de dispositivo
  // también se restaure al salir —si no, un `line_width=`/`dash=` por-primitiva se
  // filtraría al exterior en EPS/PDF aunque no en SVG (que relee dspstate por path).
  virtual void pushDrawState() { dsstack.push(dspstate); }
  virtual void popDrawState() { if (!dsstack.empty()) { dspstate = dsstack.top(); dsstack.pop(); } }

  void saveMatrix(PredefinedMatrix pdmt) { if (pdmt == MTST) mtstack.push(mtst); }
  void popMatrix() {
    if (mtstack.empty()) {
      fprintf(stderr, "Error: pila de matrices vacía (restore sin save previo)\n");
      return;
    }
    mt = mtstack.top();
    mtstack.pop();
  }
  void popMatrix(PredefinedMatrix pdmt) {
    if (pdmt != MTST) return;
    if (mtstack.empty()) {
      fprintf(stderr, "Error: pila de matrices vacía (restore sin save previo)\n");
      return;
    }
    mtst = mtstack.top();
    mtstack.pop();
  }

protected:
  /// Semilla mundo→dispositivo (§3.1): p ↦ o + s·(p − wm). Los backends la
  /// aplican en start() después de convertir dvx/dvy a puntos. Default
  /// isométrico *meet*: escala uniforme al mayor factor que hace caber la
  /// ventana en el canvas, sobrante repartido como margen centrado.
  /// stretch=true reproduce el estiramiento por eje de V1 (lo usará el
  /// traductor); no hay compensaciones por primitiva en ningún modo.
  void pushWorldMatrix() {
    sx = dvx / wwdx;
    sy = dvy / wwdy;
    double ox = 0, oy = 0;
    if (!stretch_mode) {
      sx = sy = (sx < sy) ? sx : sy;
      ox = (dvx - sx * wwdx) / 2;
      oy = (dvy - sy * wwdy) / 2;
    }
    Matrix m;
    m.translate(ox, oy);
    m.scale(sx, sy);
    m.translate(-wwx, -wwy);
    pushMatrix(m);
  }

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
  double xmin = 0, xmax = 0, ymin = 0, ymax = 0;
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

  /// Ventana de mundo (setWindow) y factores de escala mundo→pt de la semilla.
  /// sx == sy salvo en modo stretch. deviceTranslate() de cada backend usa
  /// sx/sy para convertir vectores de mundo a puntos.
  double wwx = 0, wwy = 0, wwdx = 1, wwdy = 1;
  double sx = 1, sy = 1;
  bool stretch_mode = false;

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

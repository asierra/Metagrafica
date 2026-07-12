/*
       File:  PDFDisplay.cpp
              PDF output backend using libharu (vendored).
MetaGrafica:  Human descriptive language to generate publication quality
              graphics.
     Author:  Alejandro Aguilar Sierra, UNAM
    Version:  2024
*/
#include "PDFDisplay.h"
#include "markers.h"
#include "font_lmmath_ttf.h"   // Latin Modern Math subset (g_lmmath_ttf / _len)
#include "text_parser.h"       // cmmiUnicode(): byte cmmi -> Unicode
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

using std::string;

static void hpdf_error_handler(HPDF_STATUS error_no, HPDF_STATUS detail_no, void*) {
  // El primer error deja el documento inválido: todo lo posterior falla en
  // cascada (0x1025 INVALID_DOCUMENT). No tiene sentido continuar, así que
  // abortamos de inmediato con el error real en vez de inundar la salida.
  fprintf(stderr, "Error de libharu 0x%04X (detalle 0x%04X): "
                  "no se puede generar el PDF.\n",
          (unsigned)error_no, (unsigned)detail_no);
  exit(EXIT_FAILURE);
}

/* Descompone un entero RGB empaquetado en componentes [0,1]. */
static void int2rgb(int c, double &r, double &g, double &b) {
  b = (c & 0xff) / 255.0f;
  g = ((c >> 8)  & 0xff) / 255.0f;
  r = ((c >> 16) & 0xff) / 255.0f;
}

/*
  Aproxima un arco elíptico (o circular) con segmentos cúbicos de Bézier.
  Funciona para cualquier dirección (barrido positivo = CCW, negativo = CW)
  y para elipses con rx != ry.
*/
static void arc_bezier(HPDF_Page page,
                       double cx, double cy, double rx, double ry,
                       double startDeg, double endDeg) {
  const double PI = (double)M_PI;
  double startRad = startDeg * PI / 180.0f;
  double endRad   = endDeg   * PI / 180.0f;
  double sweep    = endRad - startRad;

  if (sweep == 0.0f) return;

  int numSegs = (int)ceil(fabs(sweep) / (PI * 0.5f));
  if (numSegs < 1) numSegs = 1;
  double segSweep = sweep / numSegs;
  double alpha = (4.0f / 3.0f) * tan(segSweep / 4.0f);

  double curAngle = startRad;
  double x0 = cx + rx * cos(curAngle);
  double y0 = cy + ry * sin(curAngle);
  HPDF_Page_MoveTo(page, x0, y0);

  for (int i = 0; i < numSegs; i++) {
    double nextAngle = curAngle + segSweep;
    double x3 = cx + rx * cos(nextAngle);
    double y3 = cy + ry * sin(nextAngle);
    double x1 = x0 - alpha * rx * sin(curAngle);
    double y1 = y0 + alpha * ry * cos(curAngle);
    double x2 = x3 + alpha * rx * sin(nextAngle);
    double y2 = y3 - alpha * ry * cos(nextAngle);
    HPDF_Page_CurveTo(page, x1, y1, x2, y2, x3, y3);
    curAngle = nextAngle;
    x0 = x3; y0 = y3;
  }
}

/* ------------------------------------------------------------------ */

PDFDisplay::PDFDisplay(string f) : filename(f) {
  dvx = 10;
  dvy = 10;
}

void PDFDisplay::start() {
  // Escala y tamaño de página exactos (§3.2), sin el +0.5 de V1.
  dvx = dvx * cm_to_point;
  dvy = dvy * cm_to_point;

  pdf  = HPDF_New(hpdf_error_handler, nullptr);
  page = HPDF_AddPage(pdf);
  HPDF_Page_SetWidth(page,  dvx);
  HPDF_Page_SetHeight(page, dvy);

  // Embeber Latin Modern Math (subset griego, vendorizado) y habilitar la ruta
  // Unicode: el griego del modo matemático se dibuja por su codepoint Unicode con
  // esta fuente Computer Modern, igual en cualquier host y coherente con EPS/SVG.
  HPDF_UseUTFEncodings(pdf);
  lmmath_face = HPDF_LoadTTFontFromMemory(pdf, g_lmmath_ttf, g_lmmath_ttf_len, HPDF_TRUE);
  if (!lmmath_face)
    fprintf(stderr, "Advertencia: no se pudo cargar Latin Modern Math vendorizado\n");

  // Semilla mundo→dispositivo (§3.1): isométrica por default, en la base.
  pushWorldMatrix();
}

void PDFDisplay::end() {
  fprintf(stderr, "Closing %s\n", filename.c_str());
  HPDF_SaveToFile(pdf, filename.c_str());
  HPDF_Free(pdf);
  pdf  = nullptr;
  page = nullptr;
}

/* ------------------------------------------------------------------ */
/* Colores                                                              */
/* Deben llamarse en PAGE_DESCRIPTION, nunca en PATH_OBJECT.           */
/* ------------------------------------------------------------------ */

void PDFDisplay::applyFillColor() {
  if (dspstate.fillcolor > 0) {
    double r, g, b;
    int2rgb(dspstate.fillcolor, r, g, b);
    HPDF_Page_SetRGBFill(page, r, g, b);
  } else {
    HPDF_Page_SetGrayFill(page, dspstate.fillgray);
  }
}

void PDFDisplay::applyStrokeColor() {
  if (dspstate.linecolor > 0) {
    double r, g, b;
    int2rgb(dspstate.linecolor, r, g, b);
    HPDF_Page_SetRGBStroke(page, r, g, b);
  } else {
    HPDF_Page_SetGrayStroke(page, dspstate.linegray);
  }
}

/* Aplica fill y stroke color antes de iniciar un path. */
void PDFDisplay::prepareDraw() {
  if (dspstate.fill) applyFillColor();
  applyStrokeColor();
}

/* Abre un GSave (una sola vez por path) cuando el relleno es con patrón. Debe
   invocarse en PAGE_DESCRIPTION, antes de construir el path: libharu no permite
   GSave en PATH_OBJECT y, a diferencia de PostScript, q/Q no preservan el path
   actual, así que el recorte debe quedar contenido entre GSave/GRestore que
   envuelven toda la construcción del path. */
void PDFDisplay::ensurePatternGSave() {
  if (dspstate.fill && !dspstate.hatch.empty() && !clip_pending) {
    HPDF_Page_GSave(page);
    clip_pending = true;
  }
}

/* Recorta al path actual y lo rellena con líneas de tramado. Asume el path ya
   construido (PATH_OBJECT) y el GSave abierto por ensurePatternGSave().
   Reproduce el enfoque del EPS (clip + líneas) porque libharu no expone
   patrones de mosaico nativos. Usa el operador W (clip), que no consume el
   path: W S dibuja el contorno y fija el recorte (outlinefill); W n solo
   recorta. Las líneas cubren todo el bbox y el recorte las limita a la forma. */
void PDFDisplay::hatchCurrentPath() {
  HPDF_Page_ClosePath(page);
  HPDF_Page_Clip(page);
  if (dspstate.outlinefill)
    HPDF_Page_Stroke(page);   // W S: contorno (color de línea) + recorte
  else
    HPDF_Page_EndPath(page);  // W n: solo recorte

  // Las líneas de tramado son trazos: se pintan con el color de relleno.
  if (dspstate.fillcolor > 0) {
    double r, g, b;
    int2rgb(dspstate.fillcolor, r, g, b);
    HPDF_Page_SetRGBStroke(page, r, g, b);
  } else {
    HPDF_Page_SetGrayStroke(page, dspstate.fillgray);
  }

  const FillPattern &fp = dspstate.hatch;
  double cx = (xmin + xmax) / 2, cy = (ymin + ymax) / 2;
  double ddx = xmax - xmin, ddy = ymax - ymin;
  double D = sqrtf(ddx * ddx + ddy * ddy);  // diagonal: cubre el bbox a cualquier ángulo
  if (D <= 0) D = 1;
  for (const HatchLine &h : fp) {
    if (h.gap <= 0) continue;
    // Dirección visual de las líneas = 90 - ángulo, misma semántica que el
    // prólogo hatch* del EPS (0=vertical, 90=horizontal, 45/135 diagonales).
    double th = (90.0f - h.angle) * (double)M_PI / 180.0f;
    double ux = cosf(th), uy = sinf(th);   // a lo largo de la línea
    double px = -uy, py = ux;              // perpendicular: paso entre líneas
    int N = (int)(D / (2 * h.gap)) + 1;
    for (int k = -N; k <= N; k++) {
      double ox = cx + k * h.gap * px, oy = cy + k * h.gap * py;
      HPDF_Page_MoveTo(page, ox - (D / 2) * ux, oy - (D / 2) * uy);
      HPDF_Page_LineTo(page, ox + (D / 2) * ux, oy + (D / 2) * uy);
    }
  }
  HPDF_Page_Stroke(page);
}

/* ------------------------------------------------------------------ */
/* Estado gráfico                                                       */
/* ------------------------------------------------------------------ */

void PDFDisplay::save()    { HPDF_Page_GSave(page);    }
void PDFDisplay::restore() { HPDF_Page_GRestore(page); }

// Ámbito de estado (§7.1/§7.5): envuelve el ámbito en q/Q (GSave/GRestore) para que
// el estado de dispositivo emitido dentro (SetLineWidth/SetDash/color/fuente) se
// restaure al salir, igual que en EPS. Sin esto un `line_width=`/`dash=` por-primitiva
// se filtraría al exterior. GSave/GRestore son legales aquí: GS_PUSHSTATE/POPSTATE se
// emiten entre primitivas (modo page-description), nunca dentro de un path.
void PDFDisplay::pushDrawState() {
  HPDF_Page_GSave(page);
  Display::pushDrawState();
}

void PDFDisplay::popDrawState() {
  HPDF_Page_GRestore(page);
  Display::popDrawState();
}

void PDFDisplay::applyDash() {
  const std::vector<double> &d = dspstate.dash_array;
  if (d.empty()) {
    HPDF_Page_SetDash(page, nullptr, 0, 0);
    return;
  }
  std::vector<HPDF_REAL> buf(d.begin(), d.end());
  HPDF_Page_SetDash(page, buf.data(), (HPDF_UINT)buf.size(), 0);
}

void PDFDisplay::applyLineCap() {
  HPDF_Page_SetLineCap(page, (HPDF_LineCap)dspstate.line_cap);
}

void PDFDisplay::applyLineJoin() {
  HPDF_Page_SetLineJoin(page, (HPDF_LineJoin)dspstate.line_join);
}

void PDFDisplay::setLineWidth(double l) {
  dspstate.line_width_pt = l;
  dspstate.line_width_set = true;
  HPDF_Page_SetLineWidth(page, l);
}

void PDFDisplay::setLineColor(int lc) {
  Display::setLineColor(lc);
  applyStrokeColor();
}

/* ------------------------------------------------------------------ */
/* Paths                                                                */
/* ------------------------------------------------------------------ */

void PDFDisplay::moveto_nopath(double x, double y) {
  // Solo actualiza posición para texto; no inicia un path
  // (HPDF_Page_MoveTo pondría la página en PATH_OBJECT, bloqueando BeginText)
  mt.transform(x, y);
  cur_x = x; cur_y = y;
}

void PDFDisplay::moveto(double x, double y) {
  mt.transform(x, y);
  cur_x = x; cur_y = y;
  // En un path abierto el estado gráfico ya se fijó en setOpenPath(); libharu
  // prohíbe cambiarlo en modo path-construction, así que aquí solo se extiende.
  if (!dspstate.openpath) {
    set_limits(x, y, x, y);
    ensurePatternGSave();
    prepareDraw();
  } else
    adjust_limits(x, y, x, y);
  HPDF_Page_MoveTo(page, x, y);
}

void PDFDisplay::rmoveto(double x, double y) {
  // Se usa para reposicionar sub/superíndices dentro de una línea de texto.
  // Solo mueve el cursor de texto: HPDF_Page_MoveTo iniciaría un path y el
  // siguiente text() fallaría con GMODE al fijar color de relleno.
  cur_x += x; cur_y += y;
  adjust_limits(cur_x, cur_y, cur_x, cur_y);
}

void PDFDisplay::lineto(double x, double y) {
  mt.transform(x, y);
  cur_x = x; cur_y = y;
  adjust_limits(x, y, x, y);
  HPDF_Page_LineTo(page, x, y);
}

void PDFDisplay::rlineto(double x, double y) {
  cur_x += x; cur_y += y;
  adjust_limits(cur_x, cur_y, cur_x, cur_y);
  HPDF_Page_LineTo(page, cur_x, cur_y);
}

void PDFDisplay::curveto(double x1, double y1, double x2, double y2,
                         double x3, double y3) {
  mt.transform(x1, y1);
  mt.transform(x2, y2);
  mt.transform(x3, y3);
  cur_x = x3; cur_y = y3;
  // El bbox de la curva queda acotado por sus puntos de control.
  adjust_limits(x1, y1, x1, y1);
  adjust_limits(x2, y2, x2, y2);
  adjust_limits(x3, y3, x3, y3);
  HPDF_Page_CurveTo(page, x1, y1, x2, y2, x3, y3);
}

void PDFDisplay::line(double x1, double y1, double x2, double y2) {
  mt.transform(x1, y1);
  mt.transform(x2, y2);
  prepareDraw();
  HPDF_Page_MoveTo(page, x1, y1);
  HPDF_Page_LineTo(page, x2, y2);
  HPDF_Page_Stroke(page);
}

void PDFDisplay::stroke() {
  if (dspstate.openpath) return;
  // Colores ya aplicados por prepareDraw() antes de construir el path.
  // En PDF fill y stroke son colores independientes: se usa FillStroke
  // en lugar del truco gsave/fill/grestore del driver EPS.
  if (dspstate.fill) {
    if (!dspstate.hatch.empty()) {
      hatchCurrentPath();
      if (clip_pending) { HPDF_Page_GRestore(page); clip_pending = false; }
      return;
    }
    HPDF_Page_ClosePath(page);
    if (dspstate.outlinefill)
      HPDF_Page_FillStroke(page);
    else
      HPDF_Page_Fill(page);
  } else {
    HPDF_Page_Stroke(page);
  }
}

void PDFDisplay::closepath() {
  if (dspstate.openpath) return;
  HPDF_Page_ClosePath(page);
}

void PDFDisplay::setOpenPath(bool op) {
  Display::setOpenPath(op);
  if (op) {
    set_limits(1e10, 1e10, -1e10, -1e10);  // reinicia bbox del path (como EPS)
    // libharu prohíbe cambiar el estado gráfico (color, gsave) en modo
    // path-construction, así que se fija UNA vez aquí, antes de acumular. Las
    // primitivas dentro del path abierto lo omiten (solo extienden el path).
    ensurePatternGSave();
    prepareDraw();
  } else
    stroke();
}

/* ------------------------------------------------------------------ */
/* Primitivos                                                           */
/* ------------------------------------------------------------------ */

void PDFDisplay::rect(double x1, double y1, double x2, double y2) {
  // Las 4 esquinas en coords de mundo, transformadas individualmente: soporta
  // rotación/shear (§4.4). El orden preserva el sentido (esquina invertida refleja).
  double px[4] = {x1, x2, x2, x1};
  double py[4] = {y1, y1, y2, y2};
  for (int i = 0; i < 4; i++)
    mt.transform(px[i], py[i]);
  double minx = px[0], maxx = px[0], miny = py[0], maxy = py[0];
  for (int i = 1; i < 4; i++) {
    if (px[i] < minx) minx = px[i];
    if (px[i] > maxx) maxx = px[i];
    if (py[i] < miny) miny = py[i];
    if (py[i] > maxy) maxy = py[i];
  }
  if (!dspstate.openpath) {
    set_limits(minx, miny, maxx, maxy);
    ensurePatternGSave();
    prepareDraw();
  } else
    adjust_limits(minx, miny, maxx, maxy);
  HPDF_Page_MoveTo(page, px[0], py[0]);
  HPDF_Page_LineTo(page, px[1], py[1]);
  HPDF_Page_LineTo(page, px[2], py[2]);
  HPDF_Page_LineTo(page, px[3], py[3]);
  HPDF_Page_ClosePath(page);
  // En un path abierto solo se acumula; el cierre y relleno ocurren en CLPT.
  if (dspstate.openpath) return;
  if (dspstate.fill && !dspstate.hatch.empty()) {
    hatchCurrentPath();
    if (clip_pending) { HPDF_Page_GRestore(page); clip_pending = false; }
  } else if (dspstate.fill && dspstate.outlinefill)
    HPDF_Page_FillStroke(page);
  else if (dspstate.fill)
    HPDF_Page_Fill(page);
  else
    HPDF_Page_Stroke(page);
}

void PDFDisplay::arc(double x, double y, double w, double h,
                     double startAng, double endAng) {
  double sa = startAng, ea = endAng;
  mt.transform(x, y);
  // Radios por norma de columna: círculo se conserva bajo isometría+rotación;
  // sin forzado w=h (compensación V1).
  mt.transform_radii(w, h);
  if (h == 0) h = w;

  // Misma corrección de signos que EPSDisplay para reflejos matriciales
  if (w < 0 && h >= 0) {
    ea = 180 - startAng;
    sa = ea - endAng;
  } else if (w >= 0 && h < 0) {
    ea = 360 - startAng;
    sa = ea - endAng;
  } else if (w < 0 && h < 0) {
    sa = startAng + 180;
    ea = sa + endAng;
  }

  double aw = fabs(w), ah = fabs(h);
  if (!dspstate.openpath) {
    set_limits(x - aw, y - ah, x + aw, y + ah);
    ensurePatternGSave();
    prepareDraw();
  } else
    adjust_limits(x - aw, y - ah, x + aw, y + ah);
  arc_bezier(page, x, y, aw, ah, sa, ea);
  stroke();
}

void PDFDisplay::dot(double x, double y, double r) {
  // r = RADIO del marcador (§4.6). Relleno (disco) o contorno (círculo abierto)
  // según el estado: dot(r)=disco; dot(r,color=c) sin fill=abierto.
  mt.transform(x, y);
  HPDF_Page_GSave(page);
  if (dspstate.fill) {
    applyFillColor();            // en PAGE_DESCRIPTION, antes del path
    HPDF_Page_Circle(page, x, y, r);
    HPDF_Page_Fill(page);
  } else {
    applyStrokeColor();
    HPDF_Page_SetLineWidth(page, dspstate.line_width_pt);
    HPDF_Page_Circle(page, x, y, r);
    HPDF_Page_Stroke(page);
  }
  HPDF_Page_GRestore(page);
}

void PDFDisplay::marker(double x, double y, const MarkerShape &shape, double size, double dirx, double diry) {
  // Marcadores físicos (§4.11): forma en unidades de dispositivo (size), en cada
  // subtrayecto de markers.h. Orientación: tangente EN MUNDO (dirx,diry)
  // transformada por el marco (dos puntos), ángulo visual en dispositivo (respeta
  // stretch). Rellenable (cuadrado/rombo/flecha) -> Fill; no-rellenable (cruz/x)
  // -> Stroke, igual convención que dot.
  double ax = x, ay = y;
  mt.transform(ax, ay);
  double angle = 0;
  if (dirx != 0 || diry != 0) {
    double bx = x + dirx, by = y + diry;
    mt.transform(bx, by);
    angle = atan2(by - ay, bx - ax);
  }
  double ca = cos(angle), sa = sin(angle);
  bool doFill = shape.fillable && dspstate.fill;
  for (const auto &sub : shape.subpaths) {
    if (sub.empty()) continue;
    HPDF_Page_GSave(page);
    // El color (y el ancho de trazo) se fija en PAGE_DESCRIPTION, ANTES de
    // iniciar el path (MoveTo lo pasa a PATH_OBJECT), igual que dot().
    if (doFill) {
      applyFillColor();
    } else {
      applyStrokeColor();
      HPDF_Page_SetLineWidth(page, dspstate.line_width_pt);
    }
    bool first = true;
    for (const auto &u : sub) {
      double dx = ax + size * (u.x * ca - u.y * sa);
      double dy = ay + size * (u.x * sa + u.y * ca);
      if (first) { HPDF_Page_MoveTo(page, dx, dy); first = false; }
      else       HPDF_Page_LineTo(page, dx, dy);
    }
    if (doFill) HPDF_Page_Fill(page);
    else        HPDF_Page_Stroke(page);
    HPDF_Page_GRestore(page);
  }
}

/* ------------------------------------------------------------------ */
/* Texto                                                                */
/* ------------------------------------------------------------------ */

void PDFDisplay::setFontSize(double fz) {
  if (fz == dspstate.fontSize) return;
  Display::setFontSize(fz);
  dspstate.fontFace = FN_NOFACE;
}

void PDFDisplay::setFontFace(FontFace face) {
  if (face == dspstate.fontFace) return;
  Display::setFontFace(face);

  const char* fname    = nullptr;
  const char* encoding = "WinAnsiEncoding";

  switch (face) {
  default:
  case FN_TIMES_ROMAN:       fname = "Times-Roman";          break;
  case FN_TIMES_ITALIC:      fname = "Times-Italic";         break;
  case FN_TIMES_ROMAN_BOLD:  fname = "Times-Bold";           break;
  case FN_TIMES_ITALIC_BOLD: fname = "Times-BoldItalic";     break;
  case FN_SYMBOL:            fname = "Symbol";  encoding = nullptr; break;
  case FN_TEX_CMMI:
    // Base latina del math itálico; el griego se resuelve por Unicode en text().
    fname    = "Times-Italic";
    encoding = "WinAnsiEncoding";
    break;
  case FN_SANSERIF:              fname = "Helvetica";             break;
  case FN_SANSERIF_ITALIC:       fname = "Helvetica-Oblique";     break;
  case FN_SANSERIF_BOLD:         fname = "Helvetica-Bold";        break;
  case FN_SANSERIF_ITALIC_BOLD:  fname = "Helvetica-BoldOblique"; break;
  case FN_COURIER:               fname = "Courier";               break;
  case FN_COURIER_ITALIC:        fname = "Courier-Oblique";       break;
  case FN_COURIER_BOLD:          fname = "Courier-Bold";          break;
  }

  if (fname)
    current_font = HPDF_GetFont(pdf, fname, encoding);
}

void PDFDisplay::text(string s) {
  if (!current_font) return;

  double size = dspstate.fontSize * relfontsize;

  // FN_TEX_CMMI: el griego se traduce a su codepoint Unicode (cmmiUnicode) y se
  // dibuja con Latin Modern Math por la ruta UTF-8 (mismos glifos CM que EPS/SVG);
  // los latinos (no están en la tabla) caen a Times-Italic como aprox. de math
  // itálico. Un run puede MEZCLAR ambos (p.ej. "\Delta V"): se parte en segmentos
  // homogéneos, cada uno con su fuente, para que el byte griego no caiga en
  // Times-Italic (= ¢) ni el ASCII en LM Math (= .notdef). Igual criterio que EPS/SVG.
  // (Los símbolos de map_symbol siguen en Symbol; pendiente P1 del plan.)
  struct Seg { HPDF_Font font; string txt; };
  std::vector<Seg> segs;
  if (dspstate.fontFace == FN_TEX_CMMI) {
    const auto &cu = cmmiUnicode();
    auto isGreek = [&](unsigned char c) { return lmmath_face && cu.count(c) > 0; };
    size_t i = 0, n = s.size();
    while (i < n) {
      bool greek = isGreek((unsigned char)s[i]);
      size_t j = i;
      while (j < n && isGreek((unsigned char)s[j]) == greek) j++;
      if (greek) {
        string u;                          // UTF-8 de cada codepoint griego (U+0800..U+FFFF)
        for (size_t k = i; k < j; k++) {
          unsigned int cp = cu.at((unsigned char)s[k]);
          u += (char)(0xE0 | (cp >> 12));
          u += (char)(0x80 | ((cp >> 6) & 0x3F));
          u += (char)(0x80 | (cp & 0x3F));
        }
        segs.push_back({ HPDF_GetFont(pdf, lmmath_face, "UTF-8"), u });
      } else {
        segs.push_back({ HPDF_GetFont(pdf, "Times-Italic", "WinAnsiEncoding"), s.substr(i, j - i) });
      }
      i = j;
    }
  } else {
    segs.push_back({ current_font, s });
  }

  // El texto debe usar el color de trazo (linecolor), no el fill color de shapes.
  // En PDF modo 0 (fill), SetRGBFill controla el color del texto.
  if (dspstate.linecolor > 0) {
    double r, g, b; int2rgb(dspstate.linecolor, r, g, b);
    HPDF_Page_SetRGBFill(page, r, g, b);
  } else {
    HPDF_Page_SetGrayFill(page, dspstate.linegray);
  }

  // §19: los glifos honran la rotación de la matriz de MUNDO (`rotate 90 text(...)`).
  // Ángulo = dirección del eje x transformado por mt; la pluma (px,py) y el avance
  // van a lo largo de la línea base ROTADA (cos,sin). ang≈0 → cc=1,ss=0 → horizontal.
  double ox = 0, oy = 0, ux = 1, uy = 0;
  mt.transform(ox, oy); mt.transform(ux, uy);
  double ang = atan2(uy - oy, ux - ox);
  double cc = cos(ang), ss = sin(ang);

  // Dibuja los segmentos en secuencia, avanzando la pluma por la línea base. La
  // alineación center/right se aplica por-segmento (multi-run centrado ya era
  // aproximado; caso raro).
  for (const Seg &seg : segs) {
    HPDF_Page_SetFontAndSize(page, seg.font, size);
    double tw = HPDF_Page_TextWidth(page, seg.txt.c_str());
    double off = 0;
    if (dspstate.text_align == 1)      off = -tw / 2.0f;
    else if (dspstate.text_align == 2) off = -tw;
    double px = cur_x + off * cc, py = cur_y + off * ss;   // inicio sobre la base rotada
    HPDF_Page_BeginText(page);
    HPDF_Page_SetTextMatrix(page, cc, ss, -ss, cc, px, py);
    HPDF_Page_ShowText(page, seg.txt.c_str());
    HPDF_Page_EndText(page);
    cur_x = px + tw * cc;                   // avanza por la línea base
    cur_y = py + tw * ss;
  }

  // Restaurar fill color para shapes posteriores
  applyFillColor();
}

/* ------------------------------------------------------------------ */
/* Transformaciones                                                     */
/* ------------------------------------------------------------------ */

// Solo la rama MTLC; la contabilidad MTST vive en Display

void PDFDisplay::deviceTranslate(double x, double y) {
  // Vector en unidades de mundo → puntos, igual que en EPS/SVG.
  HPDF_Page_Concat(page, 1, 0, 0, 1, x * sx, y * sy);
}

void PDFDisplay::deviceScale(double x, double y) {
  HPDF_Page_Concat(page, x, 0, 0, y, 0, 0);
}

void PDFDisplay::deviceShear(double, double) {
  fprintf(stderr, "PDFDisplay: shear MTLC no implementado\n");
}

void PDFDisplay::deviceRotate(double angle) {
  double rad = angle * (double)M_PI / 180.0f;
  double c = cos(rad), s = sin(rad);
  // Se rota alrededor de la posición actual de la pluma (cur_x, cur_y), no del
  // origen, para preservar ese punto igual que `rotate` en PostScript/EPS y que
  // el <g rotate(a,cx,cy)> de SVGDisplay. Sin esto, el texto rotado (p.ej. una
  // etiqueta de eje colocada con XYPP+RTLC) se posiciona con MoveTextPos en el
  // sistema ya rotado y aterriza fuera de la página. La traslación (e,f) deja
  // fijo (cur_x, cur_y).
  double e = cur_x * (1 - c) + s * cur_y;
  double f = cur_y * (1 - c) - s * cur_x;
  HPDF_Page_Concat(page, c, s, -s, c, e, f);
}
// deviceInitMatrix: sin override — no hay "defaultmatrix" en un stream PDF

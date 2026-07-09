/*
       File:  PDFDisplay.cpp
              PDF output backend using libharu (vendored).
MetaGrafica:  Human descriptive language to generate publication quality
              graphics.
     Author:  Alejandro Aguilar Sierra, UNAM
    Version:  2024
*/
#include "PDFDisplay.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

using std::string;

static const char* CMMI_TTF_PATH = "/usr/share/fonts/truetype/lyx/cmmi10.ttf";

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
  max_fillpattern = 10;  // patrones de tramado vía clip+líneas (hatchCurrentPath)
}

void PDFDisplay::start() {
  // Escala y tamaño de página exactos (§3.2), sin el +0.5 de V1.
  dvx = dvx * cm_to_point;
  dvy = dvy * cm_to_point;

  pdf  = HPDF_New(hpdf_error_handler, nullptr);
  page = HPDF_AddPage(pdf);
  HPDF_Page_SetWidth(page,  dvx);
  HPDF_Page_SetHeight(page, dvy);

  // Embeber CMMI10 TTF (instalado por LyX) en el documento PDF
  FILE* f = fopen(CMMI_TTF_PATH, "rb");
  if (f) {
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    rewind(f);
    HPDF_BYTE* buf = (HPDF_BYTE*)malloc(sz);
    if (buf && (long)fread(buf, 1, sz, f) == sz)
      cmmi_face = HPDF_LoadTTFontFromMemory(pdf, buf, (HPDF_UINT)sz, HPDF_TRUE);
    free(buf);
    fclose(f);
  }
  if (!cmmi_face)
    fprintf(stderr, "Advertencia: no se pudo cargar %s\n", CMMI_TTF_PATH);

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
  if (dspstate.fill && dspstate.fillpattern > 0 && !clip_pending) {
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

  FillPattern fp = patternFor(dspstate.fillpattern);
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
    if (dspstate.fillpattern > 0) {
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
  if (dspstate.fill && dspstate.fillpattern > 0) {
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
  mt.transform(x, y);
  HPDF_Page_GSave(page);
  applyFillColor();            // en PAGE_DESCRIPTION, antes del path
  HPDF_Page_Circle(page, x, y, r);   // r = RADIO del marcador (§4.6)
  HPDF_Page_Fill(page);
  HPDF_Page_GRestore(page);
}

/* ------------------------------------------------------------------ */
/* Texto                                                                */
/* ------------------------------------------------------------------ */

/* Traduce byte codes de CMMI10 (TeX) al código equivalente en la fuente Symbol de PDF.
   Retorna 0 si el byte corresponde a una letra latina (usar Times-Italic). */
static unsigned char cmmi_to_sym(unsigned char c) {
  switch (c) {
  // Letras griegas minúsculas variantes (posiciones TeX especiales)
  case 33: return 119;  // omega
  case 34: return 101;  // varepsilon
  case 35: return  74;  // vartheta
  case 36: return 118;  // varpi
  case 37: return 114;  // varrho (≈ rho)
  case 38: return  86;  // varsigma
  case 39: return 106;  // varphi
  // Letras griegas mayúsculas
  case 161: return  71; // Gamma
  case 162: return  68; // Delta
  case 163: return  81; // Theta
  case 164: return  76; // Lambda
  case 165: return  88; // Xi
  case 166: return  80; // Pi
  case 167: return  83; // Sigma
  case 168: return  85; // Upsilon
  case 169: return  70; // Phi
  case 170: return  89; // Psi
  case 172: return  87; // Omega
  // Letras griegas minúsculas principales
  case 174: return  97; // alpha
  case 175: return  98; // beta
  case 176: return 103; // gamma
  case 177: return 100; // delta
  case 178: return 101; // epsilon
  case 179: return 122; // zeta
  case 180: return 104; // eta
  case 181: return 113; // theta
  case 182: return 105; // iota
  case 183: return 107; // kappa
  case 184: return 108; // lambda
  case 185: return 109; // mu
  case 186: return 110; // nu
  case 187: return 120; // xi
  case 188: return 112; // pi
  case 189: return 114; // rho
  case 190: return 115; // sigma
  case 191: return 116; // tau
  case 192: return 117; // upsilon
  case 193: return 102; // phi
  case 194: return  99; // chi
  case 195: return 121; // psi
  case 199: return 104; // hbar (sin equivalente → 'h')
  default:  return   0; // letra latina/dígito: pasar a Times-Italic
  }
}

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
    fname    = cmmi_face ? cmmi_face : "Times-Italic";
    encoding = nullptr;
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

  // Para FN_TEX_CMMI, un solo carácter puede ser griego (traducir a Symbol)
  // o latino (usar Times-Italic como aproximación a math italic).
  HPDF_Font use_font = current_font;
  string s_out = s;
  if (dspstate.fontFace == FN_TEX_CMMI && s.size() == 1) {
    unsigned char sym = cmmi_to_sym((unsigned char)s[0]);
    if (sym != 0) {
      use_font = HPDF_GetFont(pdf, "Symbol", nullptr);
      s_out = string(1, (char)sym);
    } else {
      use_font = HPDF_GetFont(pdf, "Times-Italic", "WinAnsiEncoding");
    }
  }

  // El texto debe usar el color de trazo (linecolor), no el fill color de shapes.
  // En PDF modo 0 (fill), SetRGBFill controla el color del texto.
  if (dspstate.linecolor > 0) {
    double r, g, b; int2rgb(dspstate.linecolor, r, g, b);
    HPDF_Page_SetRGBFill(page, r, g, b);
  } else {
    HPDF_Page_SetGrayFill(page, dspstate.linegray);
  }

  HPDF_Page_SetFontAndSize(page, use_font, size);
  double tw = HPDF_Page_TextWidth(page, s_out.c_str());

  double tx = cur_x;
  if (dspstate.text_align == 1)      tx -= tw / 2.0f;
  else if (dspstate.text_align == 2) tx -= tw;

  HPDF_Page_BeginText(page);
  HPDF_Page_SetFontAndSize(page, use_font, size);
  HPDF_Page_MoveTextPos(page, tx, cur_y);
  HPDF_Page_ShowText(page, s_out.c_str());
  HPDF_Page_EndText(page);

  // Avanzar cur_x para el siguiente segmento de la misma línea de texto
  cur_x = tx + tw;

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

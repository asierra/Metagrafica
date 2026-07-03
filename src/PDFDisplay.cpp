/*
       File:  PDFDisplay.cpp
              PDF output backend using libharu (vendored).
MetaGrafica:  Human descriptive language to generate publication quality
              graphics.
     Author:  Alejandro Aguilar Sierra, UNAM
    Version:  2024
*/
#include "PDFDisplay.h"
#include "mg.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static const char* CMMI_TTF_PATH = "/usr/share/fonts/truetype/lyx/cmmi10.ttf";

static void hpdf_error_handler(HPDF_STATUS error_no, HPDF_STATUS detail_no, void*) {
  fprintf(stderr, "libharu error: 0x%04X detail: 0x%04X\n",
          (unsigned)error_no, (unsigned)detail_no);
}

/* Descompone un entero RGB empaquetado en componentes [0,1]. */
static void int2rgb(int c, float &r, float &g, float &b) {
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
                       float cx, float cy, float rx, float ry,
                       float startDeg, float endDeg) {
  const float PI = (float)M_PI;
  float startRad = startDeg * PI / 180.0f;
  float endRad   = endDeg   * PI / 180.0f;
  float sweep    = endRad - startRad;

  if (sweep == 0.0f) return;

  int numSegs = (int)ceil(fabs(sweep) / (PI * 0.5f));
  if (numSegs < 1) numSegs = 1;
  float segSweep = sweep / numSegs;
  float alpha = (4.0f / 3.0f) * tan(segSweep / 4.0f);

  float curAngle = startRad;
  float x0 = cx + rx * cos(curAngle);
  float y0 = cy + ry * sin(curAngle);
  HPDF_Page_MoveTo(page, x0, y0);

  for (int i = 0; i < numSegs; i++) {
    float nextAngle = curAngle + segSweep;
    float x3 = cx + rx * cos(nextAngle);
    float y3 = cy + ry * sin(nextAngle);
    float x1 = x0 - alpha * rx * sin(curAngle);
    float y1 = y0 + alpha * ry * cos(curAngle);
    float x2 = x3 + alpha * rx * sin(nextAngle);
    float y2 = y3 - alpha * ry * cos(nextAngle);
    HPDF_Page_CurveTo(page, x1, y1, x2, y2, x3, y3);
    curAngle = nextAngle;
    x0 = x3; y0 = y3;
  }
}

/* ------------------------------------------------------------------ */

PDFDisplay::PDFDisplay(string f) : filename(f) {
  dvx = 10;
  dvy = 10;
  max_fillpattern = 0;
}

void PDFDisplay::start() {
  dvx = dvx * cm_to_point + 0.5f;
  dvy = dvy * cm_to_point + 0.5f;

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

  // Matriz inicial: escala de coordenadas normalizadas [0,1] a puntos
  Matrix mtmp;
  mtmp.scale(dvx, dvy);
  pushMatrix(mtmp);
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
    float r, g, b;
    int2rgb(dspstate.fillcolor, r, g, b);
    HPDF_Page_SetRGBFill(page, r, g, b);
  } else {
    HPDF_Page_SetGrayFill(page, dspstate.fillgray);
  }
}

void PDFDisplay::applyStrokeColor() {
  if (dspstate.linecolor > 0) {
    float r, g, b;
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

/* ------------------------------------------------------------------ */
/* Estado gráfico                                                       */
/* ------------------------------------------------------------------ */

void PDFDisplay::save()    { HPDF_Page_GSave(page);    }
void PDFDisplay::restore() { HPDF_Page_GRestore(page); }

void PDFDisplay::setLineStyle(int l) {
  static const HPDF_REAL dash2[] = {4, 2};
  static const HPDF_REAL dash3[] = {2, 2};
  static const HPDF_REAL dash4[] = {4, 2, 1, 2};
  static const HPDF_REAL dash5[] = {4, 2, 2, 2, 2, 2};
  switch (l) {
  case 0: case 1: HPDF_Page_SetDash(page, nullptr, 0, 0); break;
  case 2:         HPDF_Page_SetDash(page, dash2, 2, 0);   break;
  case 3:         HPDF_Page_SetDash(page, dash3, 2, 0);   break;
  case 4:         HPDF_Page_SetDash(page, dash4, 4, 0);   break;
  case 5:         HPDF_Page_SetDash(page, dash5, 6, 0);   break;
  }
}

void PDFDisplay::setLineWidth(float l) {
  HPDF_Page_SetLineWidth(page, l * 0.2f);
}

void PDFDisplay::setLineColor(int lc) {
  Display::setLineColor(lc);
  applyStrokeColor();
}

/* ------------------------------------------------------------------ */
/* Paths                                                                */
/* ------------------------------------------------------------------ */

void PDFDisplay::moveto_nopath(float x, float y) {
  // Solo actualiza posición para texto; no inicia un path
  // (HPDF_Page_MoveTo pondría la página en PATH_OBJECT, bloqueando BeginText)
  mt.transform(x, y);
  cur_x = x; cur_y = y;
}

void PDFDisplay::moveto(float x, float y) {
  mt.transform(x, y);
  cur_x = x; cur_y = y;
  prepareDraw();
  HPDF_Page_MoveTo(page, x, y);
}

void PDFDisplay::rmoveto(float x, float y) {
  cur_x += x; cur_y += y;
  HPDF_Page_MoveTo(page, cur_x, cur_y);
}

void PDFDisplay::lineto(float x, float y) {
  mt.transform(x, y);
  cur_x = x; cur_y = y;
  HPDF_Page_LineTo(page, x, y);
}

void PDFDisplay::rlineto(float x, float y) {
  cur_x += x; cur_y += y;
  HPDF_Page_LineTo(page, cur_x, cur_y);
}

void PDFDisplay::curveto(float x1, float y1, float x2, float y2,
                         float x3, float y3) {
  mt.transform(x1, y1);
  mt.transform(x2, y2);
  mt.transform(x3, y3);
  cur_x = x3; cur_y = y3;
  HPDF_Page_CurveTo(page, x1, y1, x2, y2, x3, y3);
}

void PDFDisplay::line(float x1, float y1, float x2, float y2) {
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
  if (!op)
    stroke();
}

/* ------------------------------------------------------------------ */
/* Primitivos                                                           */
/* ------------------------------------------------------------------ */

void PDFDisplay::rect(float x1, float y1, float x2, float y2) {
  mt.transform(x1, y1);
  mt.transform(x2, y2);
  prepareDraw();
  HPDF_Page_Rectangle(page, x1, y1, x2-x1, y2-y1);
  if (dspstate.fill && dspstate.outlinefill)
    HPDF_Page_FillStroke(page);
  else if (dspstate.fill)
    HPDF_Page_Fill(page);
  else
    HPDF_Page_Stroke(page);
}

void PDFDisplay::arc(float x, float y, float w, float h,
                     float startAng, float endAng) {
  float sa = startAng, ea = endAng;
  mt.transform(x, y);
  if (w != h)
    mt.transf2d(w, h);
  else {
    mt.transf2d(w, h);
    w = h;
  }
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

  prepareDraw();
  arc_bezier(page, x, y, fabs(w), fabs(h), sa, ea);
  stroke();
}

void PDFDisplay::dot(float x, float y, float r) {
  mt.transform(x, y);
  HPDF_Page_GSave(page);
  applyFillColor();            // en PAGE_DESCRIPTION, antes del path
  HPDF_Page_Circle(page, x, y, r / 2.0f);
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

void PDFDisplay::setFontSize(float fz) {
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

  float size = dspstate.fontSize * relfontsize;

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
    float r, g, b; int2rgb(dspstate.linecolor, r, g, b);
    HPDF_Page_SetRGBFill(page, r, g, b);
  } else {
    HPDF_Page_SetGrayFill(page, dspstate.linegray);
  }

  HPDF_Page_SetFontAndSize(page, use_font, size);
  float tw = HPDF_Page_TextWidth(page, s_out.c_str());

  float tx = cur_x;
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

void PDFDisplay::deviceTranslate(float x, float y) {
  HPDF_Page_Concat(page, 1, 0, 0, 1, x * dvx, y * dvy);
}

void PDFDisplay::deviceScale(float x, float y) {
  HPDF_Page_Concat(page, x, 0, 0, y, 0, 0);
}

void PDFDisplay::deviceShear(float, float) {
  fprintf(stderr, "PDFDisplay: shear MTLC no implementado\n");
}

void PDFDisplay::deviceRotate(float angle) {
  float rad = angle * (float)M_PI / 180.0f;
  float c = cos(rad), s = sin(rad);
  HPDF_Page_Concat(page, c, s, -s, c, 0, 0);
}
// deviceInitMatrix: sin override — no hay "defaultmatrix" en un stream PDF

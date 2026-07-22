#include "EPSDisplay.h"
#include "markers.h"
#include "text_parser.h"   // cmmiUnicode(): byte cmmi -> Unicode (griego vs ASCII)
#include <math.h>
#include <stdlib.h>

using std::string;

#include "font_lmmath_eps.h"


string ps_creator = R"(%%Creator: MetaGrafica v4.0 2023
"%%CreationDate: DATE)";

static const char *ps_ellipse = R"(
/ellipsedict 8 dict def
ellipsedict /mtrx matrix put
/ellipse
        { ellipsedict begin
          /endangle exch def
          /startangle exch def
          /yrad exch def
          /xrad exch def
          /y exch def
          /x exch def
          /savematrix mtrx currentmatrix def
          x y translate
          xrad yrad scale
          startangle endangle lt
          { 0 0 1 startangle endangle arc }
          { 0 0 1 startangle endangle arcn }
          ifelse 
          savematrix setmatrix
          end
} def
)";

static const char *ps_simpletextalign = R"(
/cshow
{ /mystring exch def
  mystring stringwidth pop -2 div 0 rmoveto
  mystring show
} def 

/rshow
{ /mystring exch def
  mystring stringwidth pop neg 0 rmoveto
  mystring show
} def 
)";

static const char *ps_reencode = R"(
/STARTDIFFENC { mark } bind def
/ENDDIFFENC { 

% /NewEnc BaseEnc STARTDIFFENC number or glyphname ... ENDDIFFENC -
	counttomark 2 add -1 roll 256 array copy
	/TempEncode exch def
	
	% pointer for sequential encodings
	/EncodePointer 0 def
	{
		% Get the bottom object
		counttomark -1 roll
		% Is it a mark?
		dup type dup /marktype eq {
			% End of encoding
			pop pop exit
		} {
			/nametype eq {
			% Insert the name at EncodePointer 

			% and increment the pointer.
			TempEncode EncodePointer 3 -1 roll put
			/EncodePointer EncodePointer 1 add def
			} {
			% Set the EncodePointer to the number
			/EncodePointer exch def
			} ifelse
		} ifelse
	} loop	

	TempEncode def
} bind def

% Define ISO Latin1 encoding if it doesn't exist
/ISOLatin1Encoding where {
} {
	(ISOLatin1 does not exist, creating...) =
	/ISOLatin1Encoding StandardEncoding STARTDIFFENC
		144 /dotlessi /grave /acute /circumflex /tilde 
		/macron /breve /dotaccent /dieresis /.notdef /ring 
		/cedilla /.notdef /hungarumlaut /ogonek /caron /space 
		/exclamdown /cent /sterling /currency /yen /brokenbar 
		/section /dieresis /copyright /ordfeminine 
		/guillemotleft /logicalnot /hyphen /registered 
		/macron /degree /plusminus /twosuperior 
		/threesuperior /acute /mu /paragraph /periodcentered 
		/cedilla /onesuperior /ordmasculine /guillemotright 
		/onequarter /onehalf /threequarters /questiondown 
		/Agrave /Aacute /Acircumflex /Atilde /Adieresis 
		/Aring /AE /Ccedilla /Egrave /Eacute /Ecircumflex 
		/Edieresis /Igrave /Iacute /Icircumflex /Idieresis 
		/Eth /Ntilde /Ograve /Oacute /Ocircumflex /Otilde 
		/Odieresis /multiply /Oslash /Ugrave /Uacute 
		/Ucircumflex /Udieresis /Yacute /Thorn /germandbls 
		/agrave /aacute /acircumflex /atilde /adieresis 
		/aring /ae /ccedilla /egrave /eacute /ecircumflex 
		/edieresis /igrave /iacute /icircumflex /idieresis 
		/eth /ntilde /ograve /oacute /ocircumflex /otilde 
		/odieresis /divide /oslash /ugrave /uacute 
		/ucircumflex /udieresis /yacute /thorn /ydieresis
	ENDDIFFENC
} ifelse

% Name: Re-encode Font
% Description: Creates a new font using the named encoding. 

/REENCODEFONT { % /Newfont NewEncoding /Oldfont
	findfont dup length 4 add dict
	begin
		{ % forall
			1 index /FID ne 

			2 index /UniqueID ne and
			2 index /XUID ne and
			{ def } { pop pop } ifelse
		} forall
		/Encoding exch def
		% defs for DPS
		/BitmapWidths false def
		/ExactSize 0 def
		/InBetweenSize 0 def
		/TransformedChar 0 def
		currentdict
	end
	definefont pop
} bind def

% MGTextEncoding = ISOLatin1 mas los glifos de texto que la fuente SI tiene pero
% Latin-1 no sabe nombrar (comillas tipograficas, rayas, puntos suspensivos...).
% Van en las ranuras 1..27, controles C0 que en texto nunca aparecen. La tabla
% que las asigna vive en text_parser.cpp (kExtraTextGlyphs) y la emite
% EPSDisplay::start; aqui solo se reserva el nombre.
/MGTextEncoding ISOLatin1Encoding 256 array copy def
MGEXTRAS
/ISOTimes-Roman MGTextEncoding /Times-Roman REENCODEFONT
/ISOTimes-Italic MGTextEncoding /Times-Italic REENCODEFONT
/ISOHelvetica MGTextEncoding /Helvetica REENCODEFONT
/ISOTimes-Bold MGTextEncoding /Times-Bold REENCODEFONT
/ISOHelvetica-Bold MGTextEncoding /Helvetica-Bold REENCODEFONT
)";

EPSDisplay::EPSDisplay(string f) {
  dvx = 10;
  dvy = 10;
  filename = f;
}

void EPSDisplay::start() {
  // Escala de dibujo exacta (§3.2); el redondeo a entero es solo para el
  // %%BoundingBox impreso. El +0.5 de V1 contaminaba la escala y rompía la
  // isometría en el último decimal.
  dvx = dvx * cm_to_point;
  dvy = dvy * cm_to_point;
  // "wb" y no "w": en Windows el modo texto traduce \n -> \r\n, y la salida
  // dejaría de ser byte-idéntica a la de Unix (66 FAIL en la red golden que no
  // son regresiones). El compilador ya decide sus propios saltos de línea. En
  // Unix los dos modos son lo mismo, así que este cambio no mueve un byte aquí.
  // El PDF ya lo hacía bien: libharu abre con "wb".
  file = fopen(filename.c_str(), "wb");
  if (!file) {
    fprintf(stderr, "no se pudo abrir %s\n", filename.c_str());
    exit(1);
  }
  fprintf(file, "%%!PS-Adobe-3.0 EPSF-3.0\n%%%%Title: %s\n", filename.c_str());
  fprintf(file, "%%%%BoundingBox: 0 0 %d %d\n%%%%EndComments\n",
          (int)(dvx + 0.5), (int)(dvy + 0.5));
  if (flags.using_ellipse) {
    fprintf(file, "%s", ps_ellipse);
    ellipse_defined = true;
  }
  if (flags.using_textalign)
    fprintf(file, "%s", ps_simpletextalign);
  if (flags.using_reencode) {
    // El prologo lleva un marcador MGEXTRAS que se sustituye por los `put` de las
    // ranuras: asi la tabla vive en UN solo sitio (text_parser.cpp) en vez de
    // duplicada como texto PostScript.
    std::string pro = ps_reencode;
    std::string extras;
    for (int i = 0; i < kNumExtraTextGlyphs; i++) {
      char buf[80];
      snprintf(buf, sizeof buf, "MGTextEncoding %d /%s put\n",
               kExtraTextGlyphs[i].slot, kExtraTextGlyphs[i].psname);
      extras += buf;
    }
    size_t at = pro.find("MGEXTRAS\n");
    if (at != std::string::npos) pro.replace(at, 9, extras);
    fprintf(file, "%s", pro.c_str());
  }
  if (flags.using_fontcmmi) {
    // Type42 de Latin Modern Math (subset). Define DOS fuentes lógicas: /LMMath
    // (bytes de map_tex_cmmi, griego) y /LMMathSym (bytes de map_symbol) → griego
    // y símbolos CM idénticos a PDF/SVG (§14, plan_lmmath.md P1).
    fprintf(file, "%s", font_lmmath_eps.c_str());
  }
  // Semilla mundo→dispositivo (§3.1): isométrica por default, en la base.
  pushWorldMatrix();
}

void EPSDisplay::end() {
  fprintf(stderr, "Closing %s\n", filename.c_str());
  if (file) { fflush(file); fclose(file); file = nullptr; }
}

void EPSDisplay::moveto_nopath(double x, double y) {
  mt.transform(x, y);
  fprintf(file, "%f %f moveto\n", x, y);
}

void EPSDisplay::moveto(double x, double y) {
  mt.transform(x, y);
  if (!dspstate.openpath) {
    fprintf(file, "newpath\n");
    set_limits(x,y,x,y);
  } else 
    adjust_limits(x,y,x,y);
  fprintf(file, "%f %f moveto\n", x, y);
}


void EPSDisplay::rmoveto(double x, double y) {
  fprintf(file, " %f %f rmoveto\n", x, y);
}

void EPSDisplay::lineto(double x, double y) {
  mt.transform(x, y);
  fprintf(file, "%f %f lineto\n", x, y);
  adjust_limits(x,y,x,y);
}

void EPSDisplay::curveto(double x1, double y1, double x2, double y2, double x3,
                        double y3) {
  mt.transform(x1, y1);
  mt.transform(x2, y2);
  mt.transform(x3, y3);
  fprintf(file, "%g %g %g %g %g %g curveto\n", x1, y1, x2, y2, x3, y3);
}

void EPSDisplay::rlineto(double x, double y) {
  fprintf(file, "%f %f rlineto\n", x, y);
}

void EPSDisplay::line(double x1, double y1, double x2, double y2) {
  mt.transform(x1, y1);
  mt.transform(x2, y2);
  fprintf(file, "newpath %f %f moveto %f %f lineto stroke\n", x1, y1, x2, y2);
}

void EPSDisplay::rect(double x1, double y1, double x2, double y2) {
  // Las 4 esquinas en coords de mundo, transformadas individualmente: soporta
  // rotación/shear (§4.4). El orden preserva el sentido (esquina invertida refleja).
  double px[4] = {x1, x2, x2, x1};
  double py[4] = {y1, y1, y2, y2};
  for (int i = 0; i < 4; i++)
    mt.transform(px[i], py[i]);

  // Bounding box en dispositivo (para %%BoundingBox y el barrido del tramado)
  double minx = px[0], maxx = px[0], miny = py[0], maxy = py[0];
  for (int i = 1; i < 4; i++) {
    if (px[i] < minx) minx = px[i];
    if (px[i] > maxx) maxx = px[i];
    if (py[i] < miny) miny = py[i];
    if (py[i] > maxy) maxy = py[i];
  }
  if (!dspstate.openpath)
    set_limits(minx, miny, maxx, maxy);
  else
    adjust_limits(minx, miny, maxx, maxy);

  char quad[256];
  snprintf(quad, sizeof quad,
           "newpath %f %f moveto %f %f lineto %f %f lineto %f %f lineto closepath\n",
           px[0], py[0], px[1], py[1], px[2], py[2], px[3], py[3]);

  if (isFilled()) {
    save();
    if (dspstate.fillcolor > 0)
      setColor(dspstate.fillcolor);
    else
      setGray(dspstate.fillgray);
    if (dspstate.hatch.empty())
      fprintf(file, "%sfill\n", quad);
    else {
      fprintf(file, "%s", quad);
      useFillPattern();
    }
    restore();
  }
  // El contorno se traza aquí, fuera del gsave, en el color de LÍNEA (no el de
  // relleno). Antes había un stroke extra tras useFillPattern dentro del gsave,
  // que contorneaba en el color de relleno: redundante y del color equivocado.
  if (!dspstate.fill || dspstate.outlinefill)
    fprintf(file, "%sstroke\n", quad);
}

// §4.11 fase 2: barrido genérico por ángulo (mismo método que
// PDFDisplay::hatchCurrentPath, no 4 procs PS fijos): centro + diagonal del
// bbox del path activo (xmin..ymax, ya en dispositivo), líneas paralelas cada
// `gap` a lo largo de esa diagonal, recortadas por el `clip` al path real.
// Un solo gsave/clip envolviendo TODAS las familias (en vez de uno por
// hatch<N> como antes): stroke() vacía el path corriente, así que un segundo
// `clip` por familia recortaría contra un path vacío y perdería la trama
// (bug latente del esquema anterior, nunca visible con una sola familia).
void EPSDisplay::useFillPattern() {
  if (dspstate.hatch.empty()) return;
  fprintf(file, "gsave clip\n");
  double cx = (xmin + xmax) / 2, cy = (ymin + ymax) / 2;
  double ddx = xmax - xmin, ddy = ymax - ymin;
  double D = sqrt(ddx * ddx + ddy * ddy);
  if (D <= 0) D = 1;
  for (const HatchLine &h : dspstate.hatch) {
    if (h.gap <= 0) continue;
    // Dirección visual de las líneas = 90 - ángulo (misma convención que PDF):
    // ángulo 0 = vertical, 90 = horizontal, 45/135 = diagonales.
    double th = (90.0 - h.angle) * M_PI / 180.0;
    double ux = cos(th), uy = sin(th);   // a lo largo de la línea
    double px = -uy, py = ux;            // perpendicular: paso entre líneas
    int N = (int)(D / (2 * h.gap)) + 1;
    fprintf(file, "newpath\n");
    for (int k = -N; k <= N; k++) {
      double ox = cx + k * h.gap * px, oy = cy + k * h.gap * py;
      fprintf(file, "%f %f moveto %f %f lineto\n",
              ox - (D / 2) * ux, oy - (D / 2) * uy,
              ox + (D / 2) * ux, oy + (D / 2) * uy);
    }
    fprintf(file, "stroke\n");
  }
  fprintf(file, "grestore\n");
}

void EPSDisplay::text(string s) {
  // Si aún no se emitió NINGUNA fuente al dispositivo (dev_size<0), un `show` sin
  // fuente actual no dibuja nada. Pasa cuando el primer texto del documento hereda
  // la cara ambiente (FN_NOFACE) —rótulos de axis/numbers/grid— y nunca se fijó una
  // fuente con `font`: Text::draw omite setFontFace para FN_NOFACE. Fija la cara
  // lógica vigente, o Times-Roman por default (igual que el default de text()).
  if (dev_size < 0.0)
    setFontFace(dspstate.fontFace != FN_NOFACE ? dspstate.fontFace : FN_TIMES_ROMAN);

  // §19: los glifos honran la rotación de la matriz de MUNDO (`rotate 90 text(...)`).
  // Ángulo = dirección del eje x transformado por mt; si ≠0 se gira la CTM alrededor
  // del punto actual (currentpoint translate · rotate · 0 0 moveto), como deviceRotate.
  // (Sirve para un run simple —etiquetas de eje/"energy"—; texto rotado multi-run con
  // subíndices es raro.) mt sin rotación → ang≈0 → sin gsave.
  double ox = 0, oy = 0, ux = 1, uy = 0;
  mt.transform(ox, oy); mt.transform(ux, uy);
  double ang = atan2(uy - oy, ux - ox) * 180.0 / M_PI;
  bool rot = fabs(ang) > 0.01;
  if (rot) fprintf(file, "gsave currentpoint translate %f rotate 0 0 moveto\n", ang);

  // Emite un segmento (con la fuente ya fijada) usando el operador de alineación
  // vigente. Escapa lo que un literal de cadena PostScript no admite crudo.
  //
  // El BACKSLASH va PRIMERO y no es opcional: en PostScript es el carácter de
  // escape, así que si se dobla después de insertar los de los paréntesis, se
  // doblarían también ésos. Faltaba (añadido 2026-07-20) y producía un EPS
  // inválido —`(\) show`, cadena sin cerrar → /syntaxerror en Ghostscript— en el
  // único símbolo cuyo byte es 92: `\therefore`. No lo cazaba nadie porque el
  // golden por bytes bendice un EPS sintácticamente roto (es byte-estable) y la
  // compuerta `gs`, que sí lo cazaría, solo mira los ejemplos del corpus, y
  // ninguno usaba ese símbolo. Lo destapó `symbols.mg`.
  auto emitSeg = [&](string seg) {
    size_t pos = 0;
    while ((pos = seg.find('\\', pos)) != std::string::npos) { seg.insert(pos, "\\"); pos += 2; }
    // Las ranuras 1..31 (kExtraTextGlyphs) son controles C0: en un literal de
    // cadena PostScript son legales, pero un 10 o un 13 crudos romperian la
    // estructura por lineas de la salida. Se emiten en octal.
    for (pos = 0; pos < seg.size(); pos++) {
      unsigned char c = (unsigned char)seg[pos];
      if (c >= 32) continue;
      char oct[8];
      snprintf(oct, sizeof oct, "\\%03o", c);
      seg.replace(pos, 1, oct);
      pos += 3;
    }
    pos = 0;
    while ((pos = seg.find('(', pos)) != std::string::npos) { seg.insert(pos, "\\"); pos += 2; }
    pos = 0;
    while ((pos = seg.find(')', pos)) != std::string::npos) { seg.insert(pos, "\\"); pos += 2; }
    if (dspstate.text_align == 1)      fprintf(file, "(%s) cshow\n", seg.c_str());
    else if (dspstate.text_align == 2) fprintf(file, "(%s) rshow\n", seg.c_str());
    else                               fprintf(file, "(%s) show\n", seg.c_str());
  };

  // FN_TEX_CMMI: un run puede MEZCLAR bytes griegos (∈ cmmiUnicode, glifos del
  // subset LM Math → /LMMath, embebida) y ASCII (E, dígitos, ' V = W'… que NO están
  // en ese subset griego+hbar → Times-Italic, aprox. de math itálico). P.ej.
  // "\Delta V": sin partir, el byte griego 162 saldría en Times-Italic (= ¢) o el
  // ASCII en /LMMath (= blanco). Se parte en segmentos homogéneos, cada uno con su
  // fuente (igual criterio que SVG por-byte y PDF). Alineación center/right sobre un
  // run mixto queda por-segmento (limitación pre-existente del multi-run, rara).
  if (dspstate.fontFace == FN_TEX_CMMI) {
    const std::map<unsigned char, unsigned int> &gm = cmmiUnicode();
    string prefix = flags.using_reencode ? "/ISO" : "/";
    size_t i = 0, n = s.size();
    while (i < n) {
      bool greek = gm.count((unsigned char)s[i]) > 0;
      size_t j = i;
      while (j < n && (gm.count((unsigned char)s[j]) > 0) == greek) j++;
      if (greek) fprintf(file, "/LMMath findfont %g scalefont setfont\n", dev_size);
      else       fprintf(file, "%sTimes-Italic findfont %g scalefont setfont\n", prefix.c_str(), dev_size);
      emitSeg(s.substr(i, j - i));
      i = j;
    }
    dev_face = FN_NOFACE;   // el dispositivo quedó en una fuente ad-hoc → re-sincroniza
  } else {
    emitSeg(std::move(s));
  }

  if (rot) fprintf(file, "grestore\n");
}

void EPSDisplay::getTextSize(string s, double *w, double *h) {}

void EPSDisplay::setFontSize(double fz)
{
  //printf("size %g %g\n", fz, dspstate.fontSize);
  if (fz==dspstate.fontSize)
    return;
  Display::setFontSize(fz);
  dspstate.fontFace = FN_NOFACE; // force to set font
}

void EPSDisplay::setFontFace(FontFace face) {
  //printf("face %d %d\n", face, dspstate.fontFace);
  Display::setFontFace(face);   // estado lógico (lo restaura push/popDrawState)

  // El guard compara contra el estado de DISPOSITIVO (dev_face/dev_size), no contra
  // dspstate: así, tras salir de un bloque con font_size local (pop restaura el
  // tamaño lógico pero el setfont emitido sigue en el tamaño del bloque), la próxima
  // fuente se re-emite. Se compara tamaño además de cara por la misma razón.
  double size = getFontSize() * relfontsize;
  if (face == dev_face && size == dev_size)
    return;
  dev_face = face;
  dev_size = size;

  string font;
  string prefix = "/";
  if (flags.using_reencode)
    prefix += "ISO";
  switch (face) {
  default:
  case FN_TIMES_ROMAN:
    font = prefix+"Times-Roman";
    break;
  case FN_TIMES_ITALIC:
    font = prefix+"Times-Italic";
    break;
  case FN_TIMES_ROMAN_BOLD:
    font = prefix+"Times-Bold";
    break;
  case FN_TIMES_ITALIC_BOLD:
    font = prefix+"Times-Italic-Bold";
    break;
  case FN_SYMBOL:
    // §14/P1: los símbolos salen de LM Math, no del Symbol base-14. Es la segunda
    // fuente lógica del mismo Type42 (font_lmmath_eps.h): mismo programa de glifos,
    // /Encoding en las posiciones de byte de map_symbol. Hacen falta las dos porque
    // 30 bytes colisionan con las de map_tex_cmmi.
    font = "/LMMathSym";
    break;
  case FN_TEX_CMMI:
    font = "/LMMath";
    break;
  case FN_SANSERIF:
    font = prefix+"Helvetica";
    break;
  case FN_SANSERIF_ITALIC:
    font = prefix+"Helvetica-Italic";
    break;
  case FN_SANSERIF_BOLD:
    font = prefix+"Helvetica-Bold";
    break;
  case FN_SANSERIF_ITALIC_BOLD:
    font = prefix+"Helvetica-Italic-Bold";
    break;
  case FN_COURIER:
    font = prefix+"Courier";
    break;
  case FN_COURIER_ITALIC:
    font = prefix+"Courier-Italic";
    break;
  case FN_COURIER_BOLD:
    font = prefix+"Courier-bold";
  }
  //printf("font size %g %g %g\n", fontSize, getFontSize(), relfontsize);
  fprintf(file, "%s findfont %g scalefont setfont\n", font.c_str(),
          getFontSize() * relfontsize);
}

void EPSDisplay::arc(double x, double y, double w, double h, double startAng,
                     double endAng) {
  double sa = startAng, ea = endAng;
  mt.transform(x, y);
  // Radios por norma de columna: un círculo sigue siendo círculo bajo
  // isometría+rotación; ya no se fuerza w=h (compensación V1 de la
  // normalización por eje). Una deformación explícita (SCST anisotrópico,
  // stretch) produce elipse, que es lo honesto.
  mt.transform_radii(w, h);
  if (h == 0)
    h = w;

  // To prevent sillyness of some EPS interpreters
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
  if (!dspstate.openpath) {
    fprintf(file, "newpath\n");
    set_limits(x - w, y - h, x + w, y + h);
  } else 
    adjust_limits(x - w, y - h, x + w, y + h);
  
  // Igualdad con tolerancia relativa: las normas de columna de un círculo
  // rotado difieren solo por redondeo flotante.
  if (fabs(fabs(w) - fabs(h)) <= 1e-9 * (fabs(w) + fabs(h))) {
    if (endAng < startAng)
      fprintf(file, "%f %f %f %f %f arcn\n", x, y, w, sa, ea);
    else
      fprintf(file, "%f %f %f %f %f arc\n", x, y, w, sa, ea);
  } else {
    // Define el proc /ellipse en su primer uso si el prólogo no lo emitió (la
    // bandera de parse-time no cubre p. ej. fit(stretch=true) sobre círculos).
    // `def` afecta el diccionario actual, no el estado gráfico: sobrevive a
    // gsave/grestore, así que definirlo aquí (dentro de cualquier contexto) basta.
    if (!ellipse_defined) {
      fprintf(file, "%s", ps_ellipse);
      ellipse_defined = true;
    }
    fprintf(file, "%g %g %g %g %g %g ellipse\n", x, y, w, h, sa, ea);
  }
  stroke();
}

void EPSDisplay::dot(double x, double y, double r) {
  // r = RADIO del marcador (§4.6). Se dibuja como círculo y stroke() decide
  // relleno (disco) o contorno (círculo abierto) según el estado, igual que arc:
  // dot(r)=disco; dot(r,color=c) sin fill=abierto.
  mt.transform(x, y);
  fprintf(file, "newpath %f %f %f 0 360 arc\n", x, y, r);
  stroke();
}

void EPSDisplay::marker(double x, double y, const MarkerShape &shape, double size, double dirx, double diry) {
  // Marcadores físicos (§4.11): forma en unidades de dispositivo (size), en cada
  // subtrayecto de markers.h. La orientación sale de la tangente EN MUNDO
  // (dirx,diry) transformada por el marco (dos puntos: ancla y ancla+dir), así el
  // ángulo es el visual en dispositivo (respeta stretch). Reutiliza stroke() como
  // dot(); las formas no-rellenables (cruz/x) fuerzan fill=false.
  double ax = x, ay = y;
  mt.transform(ax, ay);
  double angle = 0;
  if (dirx != 0 || diry != 0) {
    double bx = x + dirx, by = y + diry;
    mt.transform(bx, by);
    angle = atan2(by - ay, bx - ax);
  }
  bool saved_fill = dspstate.fill;
  if (!shape.fillable) dspstate.fill = false;
  double ca = cos(angle), sa = sin(angle);
  for (const auto &sub : shape.subpaths) {
    if (sub.empty()) continue;
    fprintf(file, "newpath\n");
    bool first = true;
    for (const auto &u : sub) {
      double dx = ax + size * (u.x * ca - u.y * sa);
      double dy = ay + size * (u.x * sa + u.y * ca);
      fprintf(file, "%f %f %s\n", dx, dy, first ? "moveto" : "lineto");
      first = false;
    }
    stroke();
  }
  dspstate.fill = saved_fill;
}

void EPSDisplay::stroke() {
  if (dspstate.openpath)
    return;
  if (dspstate.fill) {
    save();
    if (dspstate.fillcolor > 0)
      setColor(dspstate.fillcolor);
    else 
      setGray(dspstate.fillgray);
    if (dspstate.hatch.empty())
      fprintf(file, "closepath fill\n");
    else
      useFillPattern();
    restore();
  }
  if (!dspstate.fill || dspstate.outlinefill) {
    fprintf(file, "stroke\n");
  }
}

void EPSDisplay::closepath() {
  if (dspstate.openpath)
    return;
  fprintf(file, "closepath\n");
}

void EPSDisplay::setOpenPath(bool op) {
  Display::setOpenPath(op);
  if (dspstate.openpath)  {
    fprintf(file, "newpath\n");
    set_limits(1e10, 1e10, -1e10, -1e10);
   } else {
    fprintf(file, "closepath\n");
    stroke();
  }
}

void EPSDisplay::save() { pushDevFont(); fprintf(file, "gsave\n"); }

void EPSDisplay::restore() { fprintf(file, "grestore\n"); popDevFont(); }

// Ámbito de estado (§7.1/§7.5): además del estado lógico, envuelve el ámbito en
// gsave/grestore para que el estado de DISPOSITIVO emitido dentro (setlinewidth,
// setdash, color, fuente) se restaure al salir. Sin esto, un `line_width=`/`dash=`
// por-primitiva se filtra al exterior en EPS. Seguro: la semilla y los transforms
// de mundo/§11.1 son software (mt); el CTM de dispositivo es identidad aquí, y cada
// primitiva completa su path+stroke dentro del ámbito (no cruza el gsave/grestore).
void EPSDisplay::pushDrawState() {
  pushDevFont();
  fprintf(file, "gsave\n");
  Display::pushDrawState();
}

void EPSDisplay::popDrawState() {
  fprintf(file, "grestore\n");
  popDevFont();
  Display::popDrawState();
}

void EPSDisplay::applyDash() {
  if (dspstate.dash_array.empty()) {
    fprintf(file, "[] 0 setdash\n");
    return;
  }
  fprintf(file, "[");
  for (double v : dspstate.dash_array)
    fprintf(file, " %g", v);
  fprintf(file, " ] 0 setdash\n");
}

void EPSDisplay::applyLineCap() {
  fprintf(file, "%d setlinecap\n", dspstate.line_cap);
}

void EPSDisplay::applyLineJoin() {
  fprintf(file, "%d setlinejoin\n", dspstate.line_join);
}

void EPSDisplay::setLineWidth(double l) {
  dspstate.line_width_pt = l;
  dspstate.line_width_set = true;
  fprintf(file, "%g setlinewidth\n", l);
}

void int2rgb(int c, double &r, double &g, double &b) {
  b = (c & 0xff)/255.0;
  g = ((c>>8) & 0xff)/255.0;
  r = ((c>>16) & 0xff)/255.0;
  //printf("color %x %g %g %g\n", c, r, g, b);
}

void EPSDisplay::setColor(int fc) {
  double r, g, b;
  int2rgb(fc, r, g, b);
  fprintf(file, "%g %g %g setrgbcolor\n", r, g, b);
}

void EPSDisplay::setLineColor(int lc) {
  Display::setLineColor(lc);
  setColor(dspstate.linecolor);
}

void EPSDisplay::setGray(double fg) {
  fprintf(file, "%g setgray\n", fg);
}


/* Transformaciones: solo la rama MTLC; la contabilidad MTST vive en Display */

void EPSDisplay::deviceTranslate(double x, double y) {
  // x,y son un vector en unidades de mundo; sx/sy los llevan a puntos.
  fprintf(file, "%f %f translate\n", x * sx, y * sy);
}

void EPSDisplay::deviceScale(double x, double y) {
  fprintf(file, "%f %f scale\n", x, y);
}

void EPSDisplay::deviceShear(double, double) {
  fprintf(stderr, "Error PS no tiene shear\n");
}

void EPSDisplay::deviceRotate(double angle) {
  fprintf(file, "%f rotate\n", angle);
}

void EPSDisplay::deviceInitMatrix() {
  fprintf(file, "initmatrix\n");
}

void EPSDisplay::structureDefBegin(std::string name) {
  fprintf(file, "/%s {\n", name.c_str());
}

void EPSDisplay::structureDefEnd() { fprintf(file, "} bind def\n"); }

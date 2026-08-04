#include "EPSDisplay.h"
#include "markers.h"
#include "text_parser.h"   // cmmiUnicode(): byte cmmi -> Unicode (griego vs ASCII)
#include "version.h"       // MG_VERSION: procedencia en %%Creator
#include <math.h>
#include <stdlib.h>

using std::string;

#include "font_lmmath_eps.h"



// §4.5/§4.9 — arco o elipse bajo una matriz ARBITRARIA. En vez de reparar la
// descomposición (radios + ángulos) en espacio de dispositivo, se le entrega la
// matriz a PostScript y se traza el arco UNITARIO con los ángulos ORIGINALES,
// intactos. Es exacto para rotación, reflejo, escala anisótropa y shear, sin un
// solo caso especial — el mismo movimiento que ya usa el PDF con sus Béziers.
//
// El proc anterior recibía `rotangle` y hacía `rotate` + `scale xrad yrad`, que
// solo describe la elipse cuando la matriz es (escala uniforme)·(rotación)·(escala
// alineada a ejes); con `scale` fuera de la rotación, o con shear, mentía. Y como
// la rama circular del operador `arc` nativo no tiene parámetro de rotación, un
// arco circular girado perdía el giro por completo.
//
// `savematrix setmatrix` restaura la CTM ANTES de volver: el path ya quedó
// construido en coordenadas de dispositivo, así que la `concat` no escala el
// grosor de línea ni el guionado del `stroke` posterior, ni el tramado.
//
// El sentido lo decide `sa < ea` en el espacio LOCAL. Si la matriz refleja, el
// trazo sale invertido en dispositivo, que es exactamente lo correcto: antes eso
// lo intentaban tres bloques de corrección de signo que confundían el ángulo
// final con el barrido (un arco de 190°→350°, de 160°, salía de 350°).
static const char *ps_ellipse = R"(
/mgarcdict 8 dict def
mgarcdict /mtrx matrix put
/mgarc
        { mgarcdict begin
          /m exch def
          /ea exch def
          /sa exch def
          /savematrix mtrx currentmatrix def
          m concat
          sa ea lt
          { 0 0 1 sa ea arc }
          { 0 0 1 sa ea arcn }
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
  // %%Title lleva el .mg de ORIGEN, no la ruta de salida (ver source_name en
  // EPSDisplay.h). %%Creator declara la procedencia: Ghostscript la propaga a
  // los metadatos del PDF, así que una figura publicada dice con qué la hicieron.
  // ⚠️ Consecuencia asumida: al llevar MG_VERSION, cada subida de versión mueve
  // los goldens EPS (los de SVG y PDF no, que no lo emiten).
  fprintf(file, "%%!PS-Adobe-3.0 EPSF-3.0\n%%%%Title: %s\n",
          source_name.empty() ? "MetaGrafica figure" : source_name.c_str());
  fprintf(file, "%%%%Creator: MetaGrafica " MG_VERSION "\n");
  fprintf(file, "%%%%BoundingBox: 0 0 %d %d\n", (int)(dvx + 0.5), (int)(dvy + 0.5));
  // §4.14: shfill es nivel 3. Se declara SOLO si hay degradado, para no mover la
  // salida de todo lo demás (que sigue siendo nivel 2) por una característica que
  // no usa. Un intérprete que no llegue al nivel debe decirlo, no pintar basura.
  if (flags.using_gradient)
    fprintf(file, "%%%%LanguageLevel: 3\n");
  fprintf(file, "%%%%EndComments\n");
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
  inkPoint(x, y);
  fprintf(file, "%f %f moveto\n", x, y);
}

void EPSDisplay::moveto(double x, double y) {
  inkPoint(x, y);
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
  inkPoint(x, y);
  fprintf(file, "%f %f lineto\n", x, y);
  adjust_limits(x,y,x,y);
}

void EPSDisplay::curveto(double x1, double y1, double x2, double y2, double x3,
                        double y3) {
  inkPoint(x1, y1);
  inkPoint(x2, y2);
  inkPoint(x3, y3);
  fprintf(file, "%g %g %g %g %g %g curveto\n", x1, y1, x2, y2, x3, y3);
}

void EPSDisplay::rlineto(double x, double y) {
  fprintf(file, "%f %f rlineto\n", x, y);
}

void EPSDisplay::line(double x1, double y1, double x2, double y2) {
  inkPoint(x1, y1);
  inkPoint(x2, y2);
  fprintf(file, "newpath %f %f moveto %f %f lineto stroke\n", x1, y1, x2, y2);
}

void EPSDisplay::rect(double x1, double y1, double x2, double y2) {
  // Las 4 esquinas en coords de mundo, transformadas individualmente: soporta
  // rotación/shear (§4.4). El orden preserva el sentido (esquina invertida refleja).
  double px[4] = {x1, x2, x2, x1};
  double py[4] = {y1, y1, y2, y2};
  for (int i = 0; i < 4; i++)
    inkPoint(px[i], py[i]);

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
    if (!dspstate.gradient.empty()) {
      fprintf(file, "%s", quad);
      useGradient();
    } else if (dspstate.hatch.empty())
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

void EPSDisplay::fracRule(double dy, double len, double lw) {
  // Raya de \frac (plan_frac.md). gsave/grestore preserva el currentpoint (la pluma)
  // y el grosor de línea de afuera; la raya se traza relativa al currentpoint. En PS
  // `currentpoint newpath moveto` re-establece el punto tras limpiar el path. El color
  // vigente es el del texto (en PS fill y stroke comparten color), así que la raya sale
  // del color correcto sin fijarlo.
  fprintf(file,
          "gsave %f setlinewidth currentpoint newpath moveto 0 %f rmoveto %f 0 rlineto stroke grestore\n",
          lw, dy, len);
}

void EPSDisplay::penSegment(double dx1, double dy1, double dx2, double dy2, double lw) {
  // Igual que fracRule pero con los dos extremos libres: gsave/grestore preserva la
  // pluma y el grosor, y el color vigente es el del texto.
  fprintf(file,
          "gsave %f setlinewidth currentpoint newpath moveto %f %f rmoveto %f %f rlineto stroke grestore\n",
          lw, dx1, dy1, dx2 - dx1, dy2 - dy1);
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

// §4.14: degradado lineal con el operador NATIVO de PostScript, `shfill` con un
// sombreado tipo 2 (axial). Se eligió sobre la alternativa —franjas finas de color
// interpolado— porque el shading es exacto y sin bandeo, el archivo no crece con la
// resolución, y las franjas son justamente lo que ya se puede escribir a mano en un
// .mg: emitirlas no añadiría nada que el lenguaje no tuviera.
//
// ⚠️ `shfill` es PostScript NIVEL 3 (1997). Ghostscript lo interpreta —y la compuerta
// psfail lo verifica en cada corrida sobre el corpus—, pero un RIP anterior a esa
// fecha no. Por eso el prólogo declara `%%LanguageLevel: 3` cuando hay algún
// degradado: un intérprete que no llegue debe DECIRLO, no pintar basura.
//
// El eje sale de Display::gradientAxis (bbox del path en dispositivo + ángulo de
// página), el mismo que consumen SVG y PDF. `clip` lo recorta a la forma real, igual
// que el tramado; /Extend [true true] pinta los colores extremos más allá de las
// paradas, que es lo que evita una franja sin pintar por redondeo en los bordes.
void int2rgb(int c, double &r, double &g, double &b);   // definida más abajo

void EPSDisplay::useGradient() {
  const std::vector<GradientStop> &st = dspstate.gradient.stops;
  if (st.size() < 2) return;
  double x0, y0, x1, y1;
  gradientAxis(x0, y0, x1, y1);

  fprintf(file, "gsave clip\n<< /ShadingType 2 /ColorSpace /DeviceRGB\n");
  fprintf(file, "   /Coords [%f %f %f %f] /Extend [true true]\n", x0, y0, x1, y1);
  fprintf(file, "   /Function ");
  emitShadingFunction();
  fprintf(file, ">> shfill\ngrestore\n");
}

// La función color(t) del sombreado. Con DOS paradas es una interpolación
// exponencial (tipo 2) con N=1, o sea lineal. Con más, una función de COSTURA
// (tipo 3) que reparte [0,1] entre los tramos y reencaja cada uno en su propio
// [0,1] — que es como PostScript expresa "varias paradas", sin operador propio.
void EPSDisplay::emitShadingFunction() {
  const std::vector<GradientStop> &st = dspstate.gradient.stops;
  auto emitRamp = [&](const GradientStop &a, const GradientStop &b) {
    double r0, g0, b0, r1, g1, b1;
    int2rgb(a.color, r0, g0, b0);
    int2rgb(b.color, r1, g1, b1);
    fprintf(file, "<< /FunctionType 2 /Domain [0 1] /C0 [%g %g %g] /C1 [%g %g %g] /N 1 >>",
            r0, g0, b0, r1, g1, b1);
  };
  if (st.size() == 2) {
    emitRamp(st[0], st[1]);
    fprintf(file, "\n");
    return;
  }
  fprintf(file, "<< /FunctionType 3 /Domain [0 1]\n      /Functions [");
  for (size_t i = 0; i + 1 < st.size(); i++) { fprintf(file, " "); emitRamp(st[i], st[i + 1]); }
  fprintf(file, " ]\n      /Bounds [");
  for (size_t i = 1; i + 1 < st.size(); i++) fprintf(file, " %g", st[i].at);
  fprintf(file, " ]\n      /Encode [");
  for (size_t i = 0; i + 1 < st.size(); i++) fprintf(file, " 0 1");
  fprintf(file, " ] >>\n");
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
  if (h == 0)
    h = w;
  // La elipse imagen, en la única forma cerrada bajo afinidad: centro y
  // semidiámetros conjugados (§4.5/§4.9, Matrix::ellipse_frame).
  double Cx, Cy, ux, uy, vx, vy;
  mt.ellipse_frame(x, y, w, h, Cx, Cy, ux, uy, vx, vy);

  // Bounding box EXACTO de la elipse completa: el extremo en x de
  // C + u·cos t + v·sin t es |(ux,vx)|, y el de y es |(uy,vy)|.
  const double hw = hypot(ux, vx), hh = hypot(uy, vy);
  noteInk(Cx - hw, Cy - hh);
  noteInk(Cx + hw, Cy + hh);
  if (!dspstate.openpath) {
    fprintf(file, "newpath\n");
    set_limits(Cx - hw, Cy - hh, Cx + hw, Cy + hh);
  } else
    adjust_limits(Cx - hw, Cy - hh, Cx + hw, Cy + hh);

  // Atajo al operador `arc` nativo cuando la matriz es una escala uniforme SIN
  // rotación, reflejo ni shear (u ∥ +x, v ∥ +y, |u| = |v|): ahí describe la
  // figura exactamente y sale más compacto. Es el caso de casi todos los
  // círculos del corpus, cuya salida no se mueve un byte. Cualquier otra cosa
  // —incluido un arco circular GIRADO, que antes perdía el giro aquí— va al proc.
  const double s = fabs(ux) + fabs(uy) + fabs(vx) + fabs(vy);
  const bool plain = fabs(uy) <= 1e-12 * s && fabs(vx) <= 1e-12 * s &&
                     fabs(ux - vy) <= 1e-9 * s && ux > 0;
  if (plain) {
    if (endAng < startAng)
      fprintf(file, "%f %f %f %f %f arcn\n", Cx, Cy, ux, startAng, endAng);
    else
      fprintf(file, "%f %f %f %f %f arc\n", Cx, Cy, ux, startAng, endAng);
  } else {
    // Define el proc en su primer uso si el prólogo no lo emitió: la bandera es
    // de parse-time y no puede saber qué matriz habrá en runtime (un `rotate`
    // sobre un círculo, un fit(stretch=true)). `def` afecta el diccionario, no el
    // estado gráfico: sobrevive a gsave/grestore, así que definirlo aquí basta.
    if (!ellipse_defined) {
      fprintf(file, "%s", ps_ellipse);
      ellipse_defined = true;
    }
    // Los términos que la composición dejó en ~1e-15 son CERO (ver snap_zero en
    // matrix.h): un `rotate` acumulado hasta 180° deja basura fuera de la
    // diagonal, y es lo único que hacía que la salida dependiera de la libm de
    // cada plataforma. `s` ya es la escala de referencia de la matriz.
    fprintf(file, "%g %g [%g %g %g %g %g %g] mgarc\n",
            startAng, endAng, snap_zero(ux, s), snap_zero(uy, s),
            snap_zero(vx, s), snap_zero(vy, s), Cx, Cy);
  }
  stroke();
}

void EPSDisplay::dot(double x, double y, double r) {
  // r = RADIO del marcador (§4.6). Se dibuja como círculo y stroke() decide
  // relleno (disco) o contorno (círculo abierto) según el estado, igual que arc:
  // dot(r)=disco; dot(r,color=c) sin fill=abierto.
  inkPoint(x, y);
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
  inkPoint(ax, ay);
  double angle = 0;
  if (dirx != 0 || diry != 0) {
    double bx = x + dirx, by = y + diry;
    mt.transform(bx, by);
    angle = atan2(by - ay, bx - ax);
  }
  bool saved_fill = dspstate.fill;
  if (!shape.fillable) dspstate.fill = false;
  // Ver SVGDisplay::marker: 5.5 es el límite que le da punta a la flecha sin
  // sacarle púas a las lengüetas. PostScript trae 10, así que hay que fijarlo —y
  // devolverlo al salir, porque es estado gráfico y afectaría a lo que siga.
  fprintf(file, "5.5 setmiterlimit\n");
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
    // Un lazo se CIERRA (ver markerSubpathIsLoop): así sus dos extremos forman
    // una unión con miter y no dos tapas planas enfrentadas.
    if (markerSubpathIsLoop(sub)) fprintf(file, "closepath\n");
    stroke();
  }
  fprintf(file, "10 setmiterlimit\n");        // default de PostScript
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
    if (!dspstate.gradient.empty())
      useGradient();
    else if (dspstate.hatch.empty())
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

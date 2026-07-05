#include "EPSDisplay.h"
#include <math.h>

using std::string;

#include "font_cmmi.h"


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

static const char *ps_dot = R"(
/dot { 3 dict begin
  /rad exch def
  /y exch def
  /x exch def
  gsave
  [] 0 setdash
  1 setlinecap
  rad setlinewidth
  newpath x y moveto 0 0 rlineto stroke
  grestore
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

static const char *ps_hatchers = R"(
/rect { 4 dict begin
  /y2 exch def /x2 exch def /y1 exch def /x1 exch def
  newpath
  x1 y1 moveto
  x1 y2 lineto
  x2 y2 lineto
  x2 y1 lineto
  closepath
  end
} def

/hatch0 { 5 dict begin
  /y2 exch def /x2 exch def /y1 exch def /x1 exch def /wgap exch def
  gsave
  clip
  newpath
  {
    x1 wgap add
    /x1 exch def
    x1 x2 gt {exit} if
    x1 y1 moveto
    x1 y2 lineto
  } loop
  stroke
  grestore
  end
} def

/hatch45 { 5 dict begin
  /yu exch def /xr exch def /yl exch def /xl exch def /gap exch def
  yu /y1 exch def
  xl /x2 exch def
  gsave
  clip
  newpath
  {   y1 yl gt
      {	  y1 gap sub /y1 exch def
	  xl /x1 exch def
	  y1 yl le
	  {   yl y1 sub x1 add /x1 exch def
	      yl /y1 exch def
	  } if
      }{  yl /y1 exch def
          x1 gap add /x1 exch def
      } ifelse
      x1 xr gt {exit} if
      x2 gap add xr lt { x2 gap add /x2 exch def }{ xr /x2 exch def } ifelse
      x2 x1 sub y1 add /y2 exch def
      x1 y1 moveto
      x2 y2 lineto
  } loop
  stroke
  grestore
  end
} def

/hatch90 { 5 dict begin
  /y2 exch def /x2 exch def /y1 exch def /x1 exch def /wgap exch def
  gsave
  clip
  newpath
  {
    y1 y2 gt {exit} if
    x1 y1 moveto
    x2 y1 lineto
    y1 wgap add
    /y1 exch def
  } loop
  stroke
  grestore
  end
} def

/hatch135 { 5 dict begin
  /yu exch def /xr exch def /yl exch def /xl exch def /gap exch def
  yl /y1 exch def
  xl /x2 exch def
  gsave
  clip
  newpath
  {   y1 yu lt
      {	  y1 gap add /y1 exch def
	  xl /x1 exch def
	  y1 yu ge
	  {   y1 yu sub x1 add /x1 exch def
	      yu /y1 exch def
	  } if
      }{  yu /y1 exch def
          x1 gap add /x1 exch def
      } ifelse
      x1 xr gt {exit} if
      x2 gap add xr lt { x2 gap add /x2 exch def }{ xr /x2 exch def } ifelse
      x1 x2 sub y1 add /y2 exch def
      x1 y1 moveto
      x2 y2 lineto
  } loop
  stroke
  grestore
  end
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

/ISOTimes-Roman ISOLatin1Encoding /Times-Roman REENCODEFONT
/ISOTimes-Italic ISOLatin1Encoding /Times-Italic REENCODEFONT
/ISOHelvetica ISOLatin1Encoding /Helvetica REENCODEFONT
/ISOTimes-Bold ISOLatin1Encoding /Times-Bold REENCODEFONT
/ISOHelvetica-Bold ISOLatin1Encoding /Helvetica-Bold REENCODEFONT
)";

EPSDisplay::EPSDisplay(string f) {
  dvx = 10;
  dvy = 10;
  max_fillpattern = 10;
  filename = f;
}

void EPSDisplay::start() {
  dvx = dvx * cm_to_point + 0.5;
  dvy = dvy * cm_to_point + 0.5;
  file = fopen(filename.c_str(), "w");
  fprintf(file, "%%!PS-Adobe-3.0 EPSF-3.0\n%%%%Title: %s\n", filename.c_str());
  fprintf(file, "%%%%BoundingBox: 0 0 %d %d\n%%%%EndComments\n", (int)dvx,
          (int)dvy);
  if (flags.using_ellipse)
    fprintf(file, "%s", ps_ellipse);
  if (flags.using_dot)
    fprintf(file, "%s", ps_dot);
  if (flags.using_textalign)
    fprintf(file, "%s", ps_simpletextalign);
  if (flags.using_reencode)
    fprintf(file, "%s", ps_reencode);
  if (flags.using_hatcher)
    fprintf(file, "%s", ps_hatchers);
  if (flags.using_fontcmmi) {
    fprintf(file, "%s", font_cmmi1.c_str());
    fprintf(file, "%s", font_cmmi2.c_str());
  }
  // Define structures without initial transform
  // Structure::define_in_device(*this);
  Matrix mtmp;
  mtmp.scale(dvx, dvy);
  pushMatrix(mtmp);
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
  mt.transform(x1, y1);
  mt.transform(x2, y2);
  if (!dspstate.openpath) {
    fprintf(file, "newpath\n");
    set_limits(x1,y1,x2,y2);
  } else 
    adjust_limits(x1,y1,x2,y2);
  if (isFilled()) {
    save();
    if (dspstate.fillcolor > 0)
      setColor(dspstate.fillcolor);
    else 
      setGray(dspstate.fillgray);
    if (dspstate.fillpattern==0)
      fprintf(file, "%f %f %f %f rectfill\n", x1, y1, x2 - x1, y2 - y1);
    else {
      fprintf(file,
              "%f %f %f %f rect\n", x1, y1, x2, y2);
      useFillPattern();
      if (dspstate.outlinefill)
        fprintf(file, "stroke\n");
    }
    restore();
  } 
  if (!dspstate.fill || dspstate.outlinefill) {
    fprintf(file, "%f %f %f %f rectstroke\n", x1, y1, x2 - x1, y2 - y1);
  }
}

void EPSDisplay::useFillPattern() {
  // El descriptor del patrón (ángulo + separación por familia de líneas) vive
  // en la base Display; aquí solo se traduce a las rutinas hatch* del prólogo.
  for (const HatchLine &h : patternFor(dspstate.fillpattern))
    fprintf(file, "%f %f %f %f %f hatch%d\n",
            h.gap, xmin, ymin, xmax, ymax, (int)h.angle);
}

void EPSDisplay::text(string s) {
  // escape parentheses
  size_t pos = 0;
  while ((pos=s.find('(', pos))!=std::string::npos) {
    s.insert(pos, "\\");
    pos += 2;
  }
  pos = 0;
  while ((pos=s.find(')', pos))!=std::string::npos) {
    s.insert(pos, "\\");
    pos += 2;
  }
  if (dspstate.text_align == 1)
    fprintf(file, "(%s) cshow\n", s.c_str());
  else if (dspstate.text_align == 2)
    fprintf(file, "(%s) rshow\n", s.c_str());
  else
    fprintf(file, "(%s) show\n", s.c_str());
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
  if (face==dspstate.fontFace)
    return;

  Display::setFontFace(face);

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
    font = "/Symbol";
    break;
  case FN_TEX_CMMI:
    font = "/cmmi10";
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
  if (w != h)
    mt.transf2d(w, h);
  else {
    mt.transf2d(w, h);
    w = h;
  }
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
  
  if (fabs(w) == fabs(h)) {
    if (endAng < startAng)
      fprintf(file, "%f %f %f %f %f arcn\n", x, y, w, sa, ea);
    else
      fprintf(file, "%f %f %f %f %f arc\n", x, y, w, sa, ea);
  } else {

    fprintf(file, "%g %g %g %g %g %g ellipse\n", x, y, w, h, sa, ea);
  }
  stroke();
}

void EPSDisplay::dot(double x, double y, double r) {
  mt.transform(x, y);
  fprintf(file, "%f %f %f dot\n", x, y, r);                   
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
    if (dspstate.fillpattern == 0)
      fprintf(file, "closepath fill\n");
    else 
      useFillPattern();
    restore();
  }
  if (!dspstate.fill || dspstate.outlinefill) {
    fprintf(file, "stroke\n");
  }
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

void EPSDisplay::save() { fprintf(file, "gsave\n"); }

void EPSDisplay::restore() { fprintf(file, "grestore\n"); }

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
  fprintf(file, "%f %f translate\n", x * dvx, y * dvy);
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

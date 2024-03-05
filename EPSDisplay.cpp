#include "EPSDisplay.h"
#include "font_cmmi.h"
#include "mg.h"
#include <math.h>


bool is_using_dot=false;
bool is_using_ellipse=false;
bool is_using_reencode=false;
bool is_using_textalign=false;
bool is_using_fontcmmi=false;
bool is_using_hatcher=false;

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
          0 0 1 startangle endangle arc
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
  mystring stringwidth neg 0 rmoveto
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
    y1 wgap add
    /y1 exch def
    y1 y2 gt {exit} if
    x1 y1 moveto
    x2 y1 lineto
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
  filename = f;
  relfontsize = 1.0;
}

void EPSDisplay::start() {
  dvx *= cm_to_point + 0.5;
  dvy *= cm_to_point + 0.5;
  file = fopen(filename.c_str(), "w");
  fprintf(file, "%%!PS-Adobe-3.0 EPSF-3.0\n%%%%Title: %s\n", filename.c_str());
  fprintf(file, "%%%%BoundingBox: 0 0 %d %d\n%%%%EndComments\n", (int)dvx,
          (int)dvy);
  if (is_using_ellipse)
    fprintf(file, "%s", ps_ellipse);
  if (is_using_dot)
    fprintf(file, "%s", ps_dot);
  if (is_using_textalign)
    fprintf(file, "%s", ps_simpletextalign);
  if (is_using_reencode)
    fprintf(file, "%s", ps_reencode);
  if (is_using_hatcher)
    fprintf(file, "%s", ps_hatchers);
  if (is_using_fontcmmi) {
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
  fflush(file);
  fclose(file);
}

void EPSDisplay::moveto_nopath(float x, float y) {
  mt.transform(x, y);
  fprintf(file, "%f %f moveto\n", x, y);
}

void EPSDisplay::moveto(float x, float y) {
  mt.transform(x, y);
  if (!dspstate.openpath) {
    fprintf(file, "newpath\n");
    set_limits(x,y,x,y);
  } else 
    adjust_limits(x,y,x,y);
  fprintf(file, "%f %f moveto\n", x, y);
}


void EPSDisplay::rmoveto(float x, float y) {
  fprintf(file, " %f %f rmoveto\n", x, y);
}

void EPSDisplay::lineto(float x, float y) {
  mt.transform(x, y);
  fprintf(file, "%f %f lineto\n", x, y);
  adjust_limits(x,y,x,y);
}

void EPSDisplay::curveto(float x1, float y1, float x2, float y2, float x3,
                        float y3) {
  mt.transform(x1, y1);
  mt.transform(x2, y2);
  mt.transform(x3, y3);
  fprintf(file, "%g %g %g %g %g %g curveto\n", x1, y1, x2, y2, x3, y3);
}

void EPSDisplay::rlineto(float x, float y) {
  fprintf(file, "%f %f rlineto\n", x, y);
}

void EPSDisplay::line(float x1, float y1, float x2, float y2) {
  mt.transform(x1, y1);
  mt.transform(x2, y2);
  fprintf(file, "newpath %f %f moveto %f %f lineto stroke\n", x1, y1, x2, y2);
}

void EPSDisplay::rect(float x1, float y1, float x2, float y2) {
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
  int angle = (int)(((dspstate.fillpattern - 1) * 45) % 180);
  float wgap = 4 / (1 + (dspstate.fillpattern-1)/3);
  fprintf(file, "%f %f %f %f %f hatch%d\n", wgap, xmin, ymin, xmax, ymax, angle);
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

void EPSDisplay::getTextSize(string s, float *w, float *h) {}

void EPSDisplay::setFontSize(float fz)
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
  if (is_using_reencode)
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

void EPSDisplay::arc(float x, float y, float w, float h, float startAng,
                     float endAng) {
  float sa = startAng, ea = endAng;
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
  } else
    fprintf(file, "%g %g %g %g %g %g ellipse\n", x, y, w, h, sa, ea);
  stroke();
}

void EPSDisplay::dot(float x, float y, float r) {
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

void EPSDisplay::setLineStyle(int l) {
  switch (l) {
  case 0:
  case 1:
    fprintf(file, "[] 0 setdash\n");
    break;
  case 2:
    fprintf(file, "[ 4 2 ] 0 setdash\n");
    break;
  case 3:
    fprintf(file, "[ 2 1.6 ] 0 setdash\n");
    break;
  case 4:
    fprintf(file, "[ 4 2 1 2 ] 0 setdash\n");
    break;
  case 5:
    fprintf(file, "[ 4 2 2 2 2 2 ] 0 setdash\n");
    break;
  }
}

void EPSDisplay::setLineWidth(float l) {
  fprintf(file, "%f setlinewidth\n", l * 0.2);
}

void int2rgb(int c, float &r, float &g, float &b) {
  b = (c & 0xff)/255.0;
  g = ((c>>8) & 0xff)/255.0;
  r = ((c>>16) & 0xff)/255.0;
  printf("color %x %g %g %g\n", c, r, g, b);
}

void EPSDisplay::setColor(int fc) {
  float r, g, b;
  int2rgb(fc, r, g, b);
  fprintf(file, "%g %g %g setrgbcolor\n", r, g, b);
}

void EPSDisplay::setLineColor(int lc) {
  Display::setLineColor(lc);
  setColor(dspstate.linecolor);
}

void EPSDisplay::setGray(float fg) {
  fprintf(file, "%g setgray\n", fg);
}


/* Transformaciones */

void EPSDisplay::translate(float x, float y, PredefinedMatrix pdmt) {
  if (pdmt == MTLC)
    fprintf(file, "%f %f translate\n", x * dvx, y * dvy);
  else if (pdmt == MTST)
    mtst.translate(x, y);
}

void EPSDisplay::scale(float x, float y, PredefinedMatrix pdmt) {
  if (pdmt == MTLC)
    fprintf(file, "%f %f scale\n", x, y);
  else if (pdmt == MTST)
    mtst.scale(x, y);
}

void EPSDisplay::rotate(float angle, PredefinedMatrix pdmt) {
  if (pdmt == MTLC)
    fprintf(file, "%f rotate\n", angle);
  else if (pdmt == MTST)
    mtst.rotate(angle);
}

void EPSDisplay::init_matrix(PredefinedMatrix pdmt) {
  if (pdmt == MTLC)
    fprintf(file, "defaultmatrix\n");
  else if (pdmt == MTST)
    mtst.initialize();
}

void EPSDisplay::structureDefBegin(std::string name) {
  fprintf(file, "/%s {\n", name.c_str());
}

void EPSDisplay::structureDefEnd() { fprintf(file, "} bind def\n"); }

void EPSDisplay::structure(std::string name) {
  Structure *strct = Structure::getStructure(name);
  if (strct->isDefinedInDevice())
    fprintf(file, "%s\n", name.c_str());
  else {
    dsstack.push(dspstate);
    Matrix prevmtst = mtst;
    strct->draw(*this);
    dspstate = dsstack.top();
    dsstack.pop();
    mtst = prevmtst;
  }
}

void EPSDisplay::pushMatrix(PredefinedMatrix pdmt) {
  if (pdmt == MTST) {
    //mt *= mtst;
    mtstack.push(mtst);
    mt *= mtst;
  }
}
void EPSDisplay::pushMatrix(Matrix &m) {
  //	printf("pushing\n");
  //mt *= m;
  // mt.print();
  mtstack.push(mt);
  mt *= m;
}

void EPSDisplay::popMatrix() {
  //	printf("poping\n");
  //mtstack.pop();
  mt = mtstack.top();
  mtstack.pop();
  // mt.print();
}


void EPSDisplay::popMatrix(PredefinedMatrix pdmt) {
  //	printf("poping\n");
  if (pdmt == MTST) 
    mtst = mtstack.top();
  mtstack.pop();
  // mt.print();
}
/*
       File:  primitives.cpp
              Graphic items like primitives, atributes and 
              transformations.
MetaGrafica:  Human descriptive language to generate publication quality graphics.
              Display in PostScript.
     Author:  Alejandro Aguilar Sierra, UNAM
    Version:  2024
Antecedents: 2011, 1999 C++ STL, 1991 C. Original: 1988, Pascal and Assembler.
*/
#include "primitives.h"
#include "Display.h"


void Polyline::draw(Display &g) {
  point tk;
  auto i = path.begin();
  bool filled = g.isFilled();

  if (type==GI_POLYGON && !filled) 
    g.setFilled(true);
  
  while (i != path.end()) {
    if (i == path.begin()) {
      if (type==GI_TICKS) {
        tk.x = i->x;
        tk.y = i->y;
      } else
        g.moveto(i->x, i->y);
    } else {
      if (type==GI_BEZIER) {
        auto j = i;
        i++;
        auto k = i;
        i++;
        if (i==path.end())
          break;
        g.curveto(j->x, j->y, k->x, k->y, i->x, i->y);
      } else if (type==GI_TICKS) {
        g.moveto(i->x, i->y);
        g.lineto(i->x + tk.x, i->y + tk.y);
        g.stroke();
      } else
        g.lineto(i->x, i->y);
    }
    i++;
  }
  g.stroke();
  if (type==GI_POLYGON && !filled) 
    g.setFilled(false);
}

void Rectangle::draw(Display &g) {
  auto i = path.begin();
  while (i != path.end()) {
    point llp, rup;
    llp = *i;
    i++;
    if (i==path.end()) 
      break;
    rup = *i;
    g.rect(llp.x, llp.y, rup.x, rup.y);
    i++;
  }
}

void Arc::draw(Display &g) {
  for (const auto &p : path) {
    g.arc(p.x, p.y, rx, ry, ai, af);
  }
}

void Dot::draw(Display &g) {
  for (const auto &p : path) {
    g.dot(p.x, p.y, r);
  }
}

void Attribute::draw(Display &g) {
  switch (type) {
  case AT_LCOLOR:
    g.setLineColor(value);
    break;
  case AT_LSTYLE:
    g.setLineStyle(value);
    break;
  case AT_LWIDTH:
    g.setLineWidth(value);
    break;
  case AT_LGRAY:
    g.setLineGray(value/100.0);
    break;
  case AT_FGRAY:
    g.setFillGray(value/100.0);
    break;
  case AT_FCOLOR:
    g.setFillColor(value);
    break;
  case AT_FPATRN:
    g.setFillPattern(value);
    break;
  case AT_THEIGHT:
    g.setFontSize((float)value);
    break;
  case AT_TALIGN:
    g.setTextAlign(value);
    break;
  case AT_TSTYLE:
    g.setFontFace((FontFace)value);
    break;
  }
}

void Transform::draw(Display &g) {
  switch(op) {
    case OPMTL:
      g.translate(tl.x, tl.y, predefmat);
      break;
    case OPMSC:
      g.scale(sc.x, sc.y, predefmat);
      break;
    case OPMRT:
      g.rotate(rt, predefmat);
      break;
    case OPMID:
      g.init_matrix(predefmat);
      break;
    case OPMMT:
      break;
  }
}

void GraphicsState::draw(Display &g) {
  switch (gstype) {
    case GS_PLUMEPOSITION: 
      g.setPlumePosition(position);
      g.moveto_nopath(position.x, position.y);
      break;
    case GS_OPENPATH:
      g.setOpenPath(true);
      break;
    case GS_CLOSEPATH:
      g.setOpenPath(false);
      break;
  case GS_FILL:
    g.setFilled(true);
    break;
  case GS_NOFILL:
    g.setFilled(false);
    break;
  }
}
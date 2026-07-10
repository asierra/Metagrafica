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
        if (i==path.end())
          break;
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
  // Los ticks ya hicieron su propio stroke por cada marca dentro del bucle;
  // un stroke final aquí no tendría path y en libharu dispara GMODE.
  if (type!=GI_TICKS) {
    // §4.1: cierra la costura antes del trazo. Un polígono es cerrado por
    // definición (su contorno une el último punto con el primero); en EPS/PDF
    // el `closepath fill` del relleno va dentro de gsave/grestore y no persiste,
    // así que el trazo de contorno (outlinefill) necesita su propio closepath o
    // la costura queda abierta. La polilínea solo cierra con closed=true.
    if (closed || type==GI_POLYGON)
      g.closepath();
    g.stroke();
  }
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


void Polybar::draw(Display &g) {
  // Cada punto del path es el centro superior de una barra; se expande a un
  // rectángulo desde la base común (0) usando la primitiva rect() existente.
  double half = width / 2.0;
  for (const auto &p : path) {
    if (horizontal)
      g.rect(0.0, p.y - half, p.x, p.y + half);   // crece en x desde 0
    else
      g.rect(p.x - half, 0.0, p.x + half, p.y);    // crece en y desde 0
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
  switch (atype) {
  case AT_LCOLOR:
    g.setLineColor(value);
    break;
  case AT_LSTYLE:
    g.setLineStyle(value);         // LPATRN n: el índice se resuelve a dash_array en la base
    break;
  case AT_LWIDTH:
    // V1: n en unidades de 0.2 pt. V3 (has_f): pt directo, sin cuantizar.
    g.setLineWidth(has_f ? fvalue : value * 0.2f);
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
    // Solo alcanzable desde el front-end V1 (Parser.cpp, fuera del build);
    // V3 emite HatchAttr directamente y nunca construye este Attribute.
    if (value > g.max_fillpattern)
      value = value % (g.max_fillpattern+1);
    g.setHatch(g.patternFor(value));
    break;
  case AT_THEIGHT:
    g.setFontSize((double)value);
    break;
  case AT_TALIGN:
    g.setTextAlign(value);
    break;
  case AT_TVALIGN:
    g.setTextValign(value);
    break;
  case AT_TSTYLE:
    g.setFontFace((FontFace)value);
    break;
  }
}

void HatchAttr::draw(Display &g) {
  g.setHatch(pattern);
}

void Transform::draw(Display &g) {
  switch(op) {
    case OPMTL:
      g.translate(tl.x, tl.y, predefmat);
      break;
    case OPMSC:
      g.scale(tl.x, tl.y, predefmat);
      break;
    case OPMSH:
      g.shear(tl.x, tl.y, predefmat);
      break;
    case OPMRT:
      g.rotate(rt, predefmat);
      break;
    case OPMID:
      g.init_matrix(predefmat);
      break;
    case OPMCP:
      g.compose(mt, predefmat);
      break;
    case OPMPUSH:
      g.pushMatrix(mt);          // mt (miembro) = marco T·R·S de la invocación
      break;
    case OPMPOP:
      g.popMatrix();
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
    case GS_CLOSEPATH: // Ojo, debería de cerrar el path
      g.setOpenPath(false);
      break;
  case GS_FILL:
    g.setFilled(true);
    break;
  case GS_NOFILL:
    g.setFilled(false);
    break;
  case GS_SAVE:
    g.saveMatrix(MTST);
    //printf("save\n");
    break;
  case GS_RESTORE:
    g.popMatrix(MTST);
    //printf("restore\n");
    break;
  case GS_PUSHSTATE:
    g.pushDrawState();
    break;
  case GS_POPSTATE:
    g.popDrawState();
    break;
  case GS_OUTLINEFILL:
    g.setOutlineFill(true);
    break;
  case GS_NOOUTLINEFILL:
    g.setOutlineFill(false);
    break;
  case GS_DEVSAVE:
    g.save();
    break;
  case GS_DEVRESTORE:
    g.restore();
    break;
  }
}

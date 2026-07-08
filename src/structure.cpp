#include <math.h>
#include <stdio.h>
#include "structures.h"
#include "Display.h"

using std::string;
using std::map;

void Structure::define_in_device(Display &g, const map<string, std::unique_ptr<Structure>>& smap) {
  for (auto& it : smap) {
    Structure* st = it.second.get();
    if (st->times_used > 1 && !st->isDefinedInDevice()) {
      g.structureDefBegin(st->getName());
      st->draw(g);
      g.structureDefEnd();
      st->setDefinedInDevice();
    }
  }
}

MetaGrafica::~MetaGrafica() {
  // Destroy StructureUsers in prlist before the map releases the Structures
  // they reference, to avoid decUses() on freed memory.
  prlist.clear();
}

void Structure::draw(Display &g) {
  for (const auto &pr : prlist) {
    pr->draw(g);
  }
}

void StructurePath::draw(Display &g) {
  const size_t n = path.size();
  for (size_t i = 0; i < n; i++) {
    const point &p = path[i];
    Matrix mtpt;
    mtpt.translate(p.x, p.y);
    // Orden T·R·S: la struct (definida en el origen) se escala, se rota con la
    // tangente y se traslada al vértice. Mismo patrón que StructureLine.
    if (orient && n >= 2) {
      // Tangente por diferencia: central en interiores, unilateral en extremos.
      point d;
      if (i == 0)
        d = path[1] - path[0];
      else if (i == n - 1)
        d = path[n - 1] - path[n - 2];
      else
        d = path[i + 1] - path[i - 1];
      double a = atan2(d.y, d.x) * 180 / M_PI;
      if (a != 0.0)
        mtpt.rotate(a);
    }
    if (scale != 1)
      mtpt.scale(scale, scale);
    g.pushMatrix(mtpt);
    g.pushMatrix(MTST);
    g.save();
    g.structure(structure->getName());
    g.restore();
    g.popMatrix(MTST);
    g.popMatrix();
  }
}

void StructureRectangle::draw(Display &g) {
  Matrix mtrc;
  g.save();
  mtrc.to_rectangle(llp.x, llp.y, rup.x, rup.y);
  g.pushMatrix(mtrc);
  g.structure(structure->getName());
  g.popMatrix();
  g.restore();
}


void StructureLine::draw_side(Display &g, bool side) {
  Matrix mtln;
  point pos;
  double rt = 0.0;
  double dx = (rup.x - llp.x);
  double dy = (rup.y - llp.y);

  if (side) {
    pos.x = llp.x + shift*dx;
    pos.y = llp.y + shift*dy;
  } else {
    dx = -dx;
    dy = -dy;
    pos.x = rup.x + shift*dx;
    pos.y = rup.y + shift*dy;
  }
  mtln.translate(pos.x, pos.y);
  mtln.scale(scale.x, scale.y);
  rt = atan2(dy, dx)*180/M_PI;
  if (rt!=0.0)
    mtln.rotate(rt);
  g.save();
  g.pushMatrix(mtln);
  g.structure(structure->getName());
  g.popMatrix();
  g.restore();
}

void StructureLine::draw(Display &g) {
  if (gap > 0) {
    point mm = (rup - llp)/2;
    double d = 1-gap/2;
    point lm = llp + d*mm;
    point rm = rup - d*mm;
    //printf("gap %g %g %g %g\n", mm.x, mm.y, gap, d);
    g.line(llp.x, llp.y, lm.x, lm.y);
    g.line(rm.x, rm.y, rup.x, rup.y);
  } else
    g.line(llp.x, llp.y, rup.x, rup.y);
  draw_side(g, true);
  if (both_sides) 
    draw_side(g, false);
}

void StructureArc::draw_side(Display &g, bool side) {
  Matrix mtar;
  double angi, angf, rt = 0.0;
  point stpos;

  if (side) {
    angi = ai;
    angf = af;
  } else {
    angf = ai;
    angi = af;
  }
  if (shift!=1.0) {
    double da = (angf - angi)*shift;
    angf = angi + da;
  }
  if (angi < angf)
    rt = angf + 90;
  else
    rt = angf - 90;
  stpos.x = r*cos(angf*M_PI/180);
  stpos.y = r*sin(angf*M_PI/180);
  stpos.x += pos.x;
  stpos.y += pos.y;
  mtar.translate(stpos.x, stpos.y);
  mtar.scale(scale.x, scale.y);
  if (rt!=0.0)
    mtar.rotate(rt);
  
  g.save();
  g.pushMatrix(mtar);
  g.structure(structure->getName());
  g.popMatrix();
  g.restore();
}

void StructureArc::draw(Display &g) {
  g.arc(pos.x, pos.y, r, r, ai, af);
  draw_side(g, true);
  if (both_sides)
    draw_side(g, false);
}

MetaGrafica::MetaGrafica() : Structure() {
  maxDepth = 10;
  dcmx = 0;
  dcmy = 0;
  fontsize = 0;
}

void MetaGrafica::draw(Display &g) {
  g.setMGContext(this);
  if (dcmx > 0)
    g.setDimension(dcmx, dcmy);
  g.setWindow(wwx, wwy, wwdx, wwdy);
  g.setStretchMode(stretch);
  if (fontsize > 0)
    g.setFontSize(fontsize);
  g.start();
  for (const auto &pr : prlist) {
      if (pr != nullptr) // ojo
       pr->draw(g);
  }
  g.end();
}

/*
       File:  structures.h
              Definition of complex graphic items based on a list of primitive
              graphics items and a name as identifier.
MetaGrafica:  Human descriptive language to generate publication quality
              Display in PostScript.
Copyright (c) 2026 Alejandro Aguilar Sierra (asierra@unam.mx)
    Version:  2026
Antecedents: Version 0.0 1988 Pascal and Assembler, first published paper. 
			 Version 1.0 1991 C, first published book.
			 Version 2.0 1999-2024 C++ STL, EPS only, three published books. 
			 
 This file is part of MetaGrafica.
 Licensed under the GNU General Public License v3.0 (see LICENSE file).
*/
#if !defined(MG_H)
#define MG_H

#include "version.h"
#include "text.h"


using GraphicsItemList = std::vector<std::unique_ptr<GraphicsItem>>;


/** 
    A structure is defined by a name and a list of GraphicsItems in order to draw complex objects. 
 */
class Structure {
public:
  Structure() { times_used = 0; defined_in_device = false; }
  virtual ~Structure() = default;

  void draw(Display &);

  std::string getName() const { return name; }

  int setName(std::string n, const std::map<std::string, std::unique_ptr<Structure>>& smap) {
    if (smap.count(n)) return -1;
    name = n;
    return 0;
  }

  static Structure* getStructure(std::string nombre, const std::map<std::string, std::unique_ptr<Structure>>& smap) {
    auto it = smap.find(nombre);
    return (it != smap.end()) ? it->second.get() : nullptr;
  }

  static void define_in_device(Display &g, const std::map<std::string, std::unique_ptr<Structure>>& smap);

  void setGraphicsItems(GraphicsItemList p) { prlist = std::move(p); }
 
  bool isDefinedInDevice() const { return defined_in_device; }


  void incUses() { times_used++; }

  void decUses() { times_used--; }

  void setDefinedInDevice() { defined_in_device = true; }

protected:
  std::string name;
  unsigned times_used;
  bool defined_in_device;
  GraphicsItemList prlist;
};


/** 
   The basic structure user has a reference to a structure and is displayed 
   using the structure matrix.
 */
class StructureUser : public GraphicsItem {
public:
  StructureUser() : GraphicsItem(GI_STRUCTURE_REF), structure(nullptr) { }
  
  ~StructureUser() { if (structure) structure->decUses(); }

  void setStructure(Structure *s) { structure = s; if (structure) structure->incUses(); }

  void draw(Display &) override = 0;
protected:
  Structure* structure;
};


/** 
   A rectangle structure user is displayed over a specific user defined rectangle.
*/
class StructureRectangle : public StructureUser {
public:
  void setPoints(point q1, point q2) {  llp = q1; rup = q2; }

  void draw(Display &) override;
private:
  point llp, rup;
};


/** 
   A line structure user is displayed with a line between two points and the
   referenced structure rotated according to the line inclination.
*/
class StructureLine : public StructureUser {
public:
  StructureLine() { both_sides = false; shift = 1; gap = 0; }
  void setScale(double sx, double sy) { scale.x = sx; scale.y = sy; }
  void setPoints(point q1, point q2) {  llp = q1; rup = q2; }
  void setShift(double p) {  shift = p; }
  void setGap(double p) {  gap = p; }
  void setBothSides(bool both=true) { both_sides = both; }
  void draw(Display &) override;
private:
  void draw_side(Display &g, bool side);
  point scale;
  point llp, rup;
  double shift;
  double gap;
  bool both_sides;
};


/** 
   An arc structure user is displayed with a circular arc and the
   referenced structure rotated according to the inclination.
*/
class StructureArc : public StructureUser {
public:
  StructureArc() { both_sides = false; shift = 1; }
  void setScale(double sx, double sy) { scale.x = sx; scale.y = sy; }  
  void setRadius(double rr) { r = rr; }
  void setAngles(double x, double y) { ai = x; af = y; }
  void setPoint(point q1) {  pos = q1; }
  void setShift(double p) {  shift = p; }
  void setBothSides(bool both=true) { both_sides = both; }
  void draw(Display &) override;
private:
  void draw_side(Display &g, bool side);
  point scale;
  double r;
  double ai, af;
  point pos;
  double shift;
  bool both_sides;
};


/** 
   A path structure user is displayed over every point of the path.
*/
class StructurePath : public StructureUser {
public:
  void setPath(Path p) { path = std::move(p); }

  void draw(Display &) override;
private:
  Path path;
};

/**
 The main drawing is in this structure.
*/
class MetaGrafica : public Structure {
public:
  MetaGrafica();
  ~MetaGrafica();

  void draw(Display &);

  // Invariante de destrucción: cada StructureUser en `prlist` (heredado de
  // Structure) apunta con un puntero crudo a una Structure de este mapa y llama
  // structure->decUses() al morir. Como `prlist` vive en la base y se destruye
  // DESPUÉS que este mapa, ~MetaGrafica() vacía `prlist` explícitamente antes de
  // que el mapa libere las Structure (ver src/structure.cpp). No quitar ese clear().
  std::map<std::string, std::unique_ptr<Structure>> structure_map;

  int setName(std::string n) { name = n; return 0; }

  Structure* getStructure(std::string name) {
    return Structure::getStructure(name, structure_map);
  }

  void setDimension(double x, double y) {
    dcmx = x;
    dcmy = y;
  }

  void getDimension(double &x, double &y) {
    x = dcmx;
    y = dcmy;
  }
  
  void setDepth(int d) { maxDepth = d; }
  int getDepth() { return maxDepth; }

  void setFontSize(double d) { fontsize = d; }
//  int getFontSize() { return fontsize; }

private:
  /// Profundidad maxima de recursion
  int maxDepth;
  /// Dimensiones físicas en cm
  double dcmx, dcmy;
  /// Font size in points
  double fontsize;
  ///
  //Matrix matrix;
};

#endif

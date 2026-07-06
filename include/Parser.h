/*
       File:  Parser.h
              Parser of the MetaGrafica language: reads a .mg source with
              MGLexer and builds the in-memory MetaGrafica tree.
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
#if !defined(MG_PARSER_H)
#define MG_PARSER_H

#include "structures.h"
#include "MGLexer.h"
#include "mgflags.h"
#include <memory>
#include <fstream>

#include <string.h>


// typedef hash_map<const char*, PrimitiveList, hash<const char*>, eqstr>
// PrimitiveMap;

//typedef std::map<const char *, PrimitiveList, ltstr> PrimitiveMap;
//typedef std::map<const char *, PrimitiveList, ltstr>::iterator mapIterator;


class Parser {

public:
  ///
  Parser(std::string filename);
  ///
  ~Parser();
  std::unique_ptr<MetaGrafica> parse();

  std::string getName() { return filename; }

  std::string getCanonicalName() { return canfilename; }

  void setLogFile(FILE *);

  MGFlags flags;

private:
  double parseFloat();
  std::string parseString();
  std::string location() const;
  void parseDef(int def);
  Path parsePath();
  GraphicsItemList parsePrimitives();
  void oldParseMatrix(int mo, Matrix &mt);
  std::unique_ptr<Transform> parseMatrix(int mo);
  std::unique_ptr<GraphicsItem> parsePrimitive(int);
  Path parseRectangle();
  point parsePoint();

  // Helper methods to modularize parsePrimitives
  void parseMatrixOp(int token, GraphicsItemList &prlist, Matrix &mtpp, Matrix &mtpt, Matrix &mtrs, bool &using_mtlc);
  void parseStructureOp(int token, GraphicsItemList &prlist, Structure* &st, Matrix &mtpp, Matrix &mtrs, point &pp);
  void parsePathOp(int token, bool &is_concat, std::string &ctname, Path &ctpath, Matrix &mtpt);
  void parseAttribute(int token, GraphicsItemList &prlist, FontFace &ff);
  void parseGraphicsState(int token, GraphicsItemList &prlist, bool &is_concat, std::string &ctname, Path &ctpath, point &pp);
  void parseTextOp(int token, GraphicsItemList &prlist, point &pp, Matrix &mtpp, FontFace &ff);

  /// Window
  double wmx, wmy, wdx, wdy;
  /// min(wdx, wdy) de la ventana real del documento: factor que traduce la
  /// escala V1 de placements (relativa a min del canvas) a unidades de mundo.
  double docwmin = 1;
  Matrix mtpt;
  //point pp;
  //StructureMap structure_map;
  std::map<std::string, Path> listmap;
  MetaGrafica *mg;
  int lastyylex;
  std::string filename;
  std::string canfilename;
  bool is_spline_to_bezier;
  int spline_nodes_per_segment;
  // Disparador TEMPORAL (V1) para probar la orientación tangente de placements
  // sobre path (§C.1 de plan_marcadores). Se activa con `$O 1` y lo lee cada
  // StructurePath al construirse. Morirá con la capa de atributos V3
  // (marker_orient), que lo fijará por primitiva.
  bool orient_next = false;
  Path bufferpt;
  int currentDepth;
  std::unique_ptr<MGLexer> lexer;
  std::unique_ptr<std::ifstream> pInfile;
};

#endif

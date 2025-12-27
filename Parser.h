/*
       File:  structure.h
              Definition of a complex graphic item based on a list of basic
              Display items and a name as identifier.
MetaGrafica:  Human descriptive language to generate publication quality
              Display in PostScript.
     Author:  Alejandro Aguilar Sierra, UNAM
    Version:  2024
Antecedents: 2011, 1999 C++ STL, 1991 C. Original: 1988, Pascal and Assembler.
*/
#if !defined(__PARSER_H)
#define __PARSER_H

#include "mg.h"
#include "MGLexer.h"
#include <memory>
#include <fstream>

#include <string.h>


// typedef hash_map<const char*, PrimitiveList, hash<const char*>, eqstr>
// PrimitiveMap;

//typedef map<const char *, PrimitiveList, ltstr> PrimitiveMap;
//typedef map<const char *, PrimitiveList, ltstr>::iterator mapIterator;


class Parser {

public:
  ///
  Parser(string filename);
  ///
  ~Parser();
  ///
  std::unique_ptr<MetaGrafica> parse();
  ///
  string getName() { return filename; }
  ///
  string getCanonicalName() { return canfilename; }
  ///
  void setLogFile(FILE *);

private:
  float parseFloat();
  string parseString();
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
  void parsePathOp(int token, bool &is_concat, string &ctname, Path &ctpath, Matrix &mtpt);
  void parseAttribute(int token, GraphicsItemList &prlist, FontFace &ff);
  void parseGraphicsState(int token, GraphicsItemList &prlist, bool &is_concat, string &ctname, Path &ctpath, point &pp);
  void parseTextOp(int token, GraphicsItemList &prlist, point &pp, Matrix &mtpp, FontFace &ff);

  /// Window
  float wmx, wmy, wdx, wdy;
  Matrix mtpt;
  //point pp;
  //StructureMap structure_map;
  map<string, Path> listmap;
  MetaGrafica *mg;
  int lastyylex;
  string filename;
  string canfilename;
  bool is_spline_to_bezier;
  int spline_nodes_per_segment;
  Path bufferpt;
  int currentDepth;
  std::unique_ptr<MGLexer> lexer;
  std::unique_ptr<std::ifstream> pInfile;
};

#endif

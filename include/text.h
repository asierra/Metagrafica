/*
       File:  text.h
              Text operations.
MetaGrafica:  Human descriptive language to generate publication quality
              Display in PostScript.
Copyright (c) 1988-2026 Alejandro Aguilar Sierra (algsierra@gmail.com)
    Version:  2026
Antecedents: Version 0.0 1988 Pascal and Assembler, first published paper. 
			 Version 1.0 1991 C, first published book.
			 Version 2.0 1999-2024 C++ STL, EPS only, three published books. 
			 
 This file is part of MetaGrafica.
 Licensed under the GNU General Public License v3.0 (see LICENSE file).
*/

#if !defined(MG_TEXT_H)
#define MG_TEXT_H

#include <string>

#include<map>

#include <vector>
#include <memory>
#include "primitives.h"

// Font properties, bit code:: sanserif = 04, weight = 02, slant = 01
// default 0,0,0,0
// serif italic 1
// serif bold 10
// serif italic bold 11
// sanserif 100
// sanserif italic 101
// sanserif bold 110
// sanserif italic bold 111
enum FontFace {
  FN_DEFAULT,
  FN_SERIF=0,
  FN_SERIF_ITALIC,
  FN_SERIF_BOLD,
  FN_SERIF_ITALIC_BOLD,
  FN_SANSERIF,
  FN_SANSERIF_ITALIC,
  FN_SANSERIF_BOLD,
  FN_SANSERIF_ITALIC_BOLD,
  FN_COURIER,
  FN_COURIER_ITALIC,
  FN_COURIER_BOLD,
  FN_COURIER_ITALIC_BOLD,
  FN_SYMBOL,
  FN_TEX_CMMI,
  FN_ITALIC = 1,
  FN_BOLD = 2,
  FN_TIMES_ROMAN=FN_SERIF,
  FN_TIMES_ITALIC=FN_SERIF_ITALIC,
  FN_TIMES_ROMAN_BOLD=FN_SERIF_BOLD,
  FN_TIMES_ITALIC_BOLD=FN_SERIF_ITALIC_BOLD,
  FN_NOFACE=-1,
};


using FontMetricsMap = std::map<unsigned char, int>;


struct TextState {
  TextState() { font_face = FN_DEFAULT; font_size = 1; script = 0; pre_space = 0; }

  FontFace font_face;

  // This is a relative font size, the real size is property of the
  // external device
  double font_size;

  // 0 no script, upper > 0 , down < 0
  int script;

  // Espacio matemático ANTES de este run, en em (plan_text_space, Parte B): lo pone
  // la tabla de clases TeX (Ord/Rel/Bin/…) en parse-time. Es propiedad del RUN, no
  // del glifo: TextLine::width() lo suma y TextLine::draw() lo aplica (rmoveto) antes
  // de dibujar el run. Entra en operator== para que dos runs con espaciado distinto
  // NO se fusionen al acumular (colapsaría dos espacios en uno).
  double pre_space;

  bool operator==( const TextState& other ) const {
        return font_face == other.font_face &&
               font_size    == other.font_size    &&
               script    == other.script &&
               pre_space == other.pre_space;
  }
  std::string str() { return "Face " + std::to_string(font_face) +
    " Size " + std::to_string(font_size) +
    " Script " + std::to_string(script) +
    " Pre " + std::to_string(pre_space); }
};

// Ancho de una cadena en unidades em relativas (multiplicar por el tamaño de
// fuente del dispositivo para obtener puntos). Definido en text.cpp.
double text_width(TextState ts, std::string s);

constexpr double fn_relsz_script = 0.7;
//constexpr double fn_relsz_script2 = 0.5;
constexpr double fn_relsz_big = 1.5;
constexpr double fn_relsz_small = 0.5;



/** 
   Single font single line text.
 */
class Text : public GraphicsItem {
public:
  Text() : GraphicsItem(GI_TEXT) { }

  void draw(Display &) override;


  void setText(std::string s) { text = s; }
  void addText(std::string s) { text.append(s); }
  std::string getText() const { return text; }

  void setState(TextState s) { textstate = s; }
  TextState getState() const { return textstate; }
  void setFontFace(FontFace face) { textstate.font_face = face; }

  size_t length() const { return text.length(); }

  //int width();

private:
  std::string text;
  TextState textstate;
};


/**
   A text line starts in the pen position and can include text with
   different font faces or sizes.
 */
class TextLine : public GraphicsItem {
public:
  TextLine() : GraphicsItem(GI_TEXTLINE) { }

  void draw(Display &) override;

  void addText(std::unique_ptr<Text> t) { textline.push_back(std::move(t)); }

  size_t length() const { return textline.size(); }

  double width();

private:
  std::vector<std::unique_ptr<Text>> textline;
};


/**
   A text block: several lines stacked vertically (§14.1, `/n`).

   Es deliberadamente lo MÁS simple que resuelve el caso: una lista de renglones ya
   construidos (cada uno Text o TextLine, que ya saben su ancho y su alineación) más
   el interlineado. No compone nada — apila.

   Vive en el motor y no en el parser por una razón concreta: el desplazamiento entre
   renglones es `leading · font_size`, y el tamaño de fuente solo existe en DRAW-TIME
   (el parser no lleva sombra del estado gráfico, y `font_size` viaja como atributo en
   el flujo de items). Resolverlo en parse-time obligaría a adivinar el tamaño
   heredado — exactamente la clase de error que produjo los bugs de FN_NOFACE.
 */
class TextBlock : public GraphicsItem {
public:
  TextBlock() : GraphicsItem(GI_TEXTBLOCK) { }

  void draw(Display &) override;

  /// Un renglón; nullptr = renglón vacío (solo avanza el interlineado).
  void addLine(std::unique_ptr<GraphicsItem> l) { lines.push_back(std::move(l)); }
  size_t length() const { return lines.size(); }
  void setLeading(double f) { leading = f; }

private:
  std::vector<std::unique_ptr<GraphicsItem>> lines;
  double leading = 1.2;     ///< múltiplo de font_size entre líneas base
};


/**
   SPIKE \frac: composición 2-D de numerador sobre denominador con raya.
   A diferencia de TextBlock (que APILA renglones sin componer), esto MIDE cada
   parte, centra la más angosta sobre la más ancha, traza la raya y avanzaría la
   pluma por su ancho. Solo usa virtuals de Display (device-space), así que NO
   toca los backends. Exploratorio: standalone (frac como todo el text()), sin
   inline dentro de una fórmula mayor.
 */
class Fraction : public GraphicsItem {
public:
  Fraction() : GraphicsItem(GI_FRACTION) { }
  void draw(Display &) override;
  void setNum(std::unique_ptr<GraphicsItem> n) { num = std::move(n); }
  void setDen(std::unique_ptr<GraphicsItem> d) { den = std::move(d); }

private:
  std::unique_ptr<GraphicsItem> num, den;
  static double childWidth(GraphicsItem *g);   // ancho em del hijo (Text/TextLine)
};


/**
   A text struct uses one or more text lines to show a fraction, sum, product, etc.
   RESERVADO (ver el comentario de GI_TEXTSTRUCT en primitives.h).
 *
class TextStruct : public GraphicsItem {public:
  TextStruct() : GraphicsItem(GI_TEXTSTRUCT) { }
  
  void draw(Display &)=0;
    
  void addTextLine(TextLine *t) { textlines.push_back(t); }

  size_t length(); 

  double width();

private:
  std::list<TextLine*> textlines;
};
*/

#endif
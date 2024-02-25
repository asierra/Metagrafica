/*
       File:  text.h
              Text operations.
MetaGrafica:  Human descriptive language to generate publication quality graphics.
              Display in PostScript.
     Author:  Alejandro Aguilar Sierra, UNAM
    Version:  2024
Antecedents: 2011, 1999 C++ STL, 1991 C, Original: 1988, Pascal and Assembler.
*/

#if !defined(__TEXT_H)
#define __TEXT_H

#include <string>
using std::string;

#include<map>
using std::map;

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


using FontMetricsMap = map<unsigned char, int>;


struct TextState {
  TextState() { font_face = FN_DEFAULT; font_size = 1; script = 0; }

  FontFace font_face;

  // This is a relative font size, the real size is property of the 
  // external device
  float font_size;

  // 0 no script, upper > 0 , down < 0
  int script;
  
  bool operator==( const TextState& other ) {
        return font_face == other.font_face &&
               font_size    == other.font_size    &&
               script    == other.script;
  }
  string str() { return "Face " + std::to_string(font_face) + 
    " Size " + std::to_string(font_size) + 
    " Script " + std::to_string(script); }
};

constexpr float fn_relsz_script = 0.7;
//constexpr float fn_relsz_script2 = 0.5;
constexpr float fn_relsz_big = 1.5;
constexpr float fn_relsz_small = 0.5;



/** 
   Single font single line text.
 */
class Text : public GraphicsItem {
public:
  Text() : GraphicsItem(GI_TEXT) { }
  
  void draw(Display &);
  

  void setText(string s) { text = s; }
  void addText(string s) { text.append(s); }
  string getText() { return text; }

  void setState(TextState s) { textstate = s; }
  TextState getState() { return textstate; }
  void setFontFace(FontFace face) { textstate.font_face = face; }

  size_t length() { return text.length(); } 

  //int width();

private:
  string text;
  TextState textstate;
};


/** 
   A text line starts in the pen position and can include text with
   different font faces or sizes.
 */
class TextLine : public GraphicsItem {
public:
  TextLine() : GraphicsItem(GI_TEXTLINE) { }
  
  void draw(Display &);
    
  void addText(Text *t) { textline.push_back(t); }

  size_t length() { return distance(textline.begin(), textline.end()); } 

  float width();

private:
  std::list<Text*> textline;
};


#endif
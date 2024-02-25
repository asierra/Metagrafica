#include "text.h"

#include <stack>
using std::stack;


extern bool is_using_reencode;
extern bool is_using_fontcmmi;

const string ALFABETIC = "abcdefghijklmnopqrstuvwxyz";
const string TOKENLIM = "{}^$_/\\ ";
const string DIGIT = "1234567890";


// Standar symbol font codes
map<string, unsigned char> map_symbol = { 
   {"alpha", 97}, {"beta", 98}, {"gamma", 103}, {"delta", 100},
   {"zeta", 122}, {"eta", 104}, {"theta", 113}, {"iota", 105},
   {"kappa", 107},{"lambda", 108}, {"mu", 109}, {"nu", 110},
   {"xi", 120}, {"pi", 112}, {"rho", 114}, {"sigma", 115},
   {"tau", 116}, {"upsilon", 117}, {"phi", 102}, {"chi", 99},
   {"psi", 121}, {"omega", 119}, {"varepsilon", 101}, {"vartheta", 74},
   {"varpi", 118}, {"varsigma", 86}, {"varphi", 106}, {"Gamma", 71},
   {"Delta", 68},  {"Theta", 81}, {"Lambda", 76}, {"Xi", 88},
   {"Pi", 80}, {"Sigma", 83}, {"Upsilon", 161}, {"Phi", 70},
   {"Psi", 89}, {"Omega", 87}, {"aleph", 192}, {"wp", 195},
   {"Re", 194}, {"Im", 193}, {"partial", 182}, {"infty", 165}, {"prime", 162},
   {"nabla", 209}, {"bot", 94}, {"forall", 34}, {"exists", 36}, {"neg", 216},
   {"sharp", 35}, {"clubsuit", 167}, {"diamondsuit", 168}, {"heartsuit", 169},
   {"spadesuit", 170}, {"int", 242}, {"prod", 213}, {"sum", 229},
   {"wedge", 217},   {"vee", 218},   {"cap", 199},   {"cup", 200},
   {"diamond", 224},   {"bullet", 183},   {"div", 184},   {"oslash", 198},
   {"otimes", 196},   {"oplus", 197},   {"pm", 177},   {"cdot", 215},
   {"times", 180},   {"propto", 181},   {"mid", 124},   {"Leftrightarrow", 219},
   {"Leftarrow", 220},   {"Rightarrow", 222},   {"leq", 163},   {"geq", 179},
   {"approx", 187},   {"supset", 201},   {"subset", 204},   {"supseteq", 202},
   {"subseteq", 205},   {"in", 206},   {"ni", 39},   {"leftrightarrow", 171},
   {"leftarrow", 172},   {"rightarrow", 174},   {"sim", 126},   {"equiv", 186},
   {"colon", 58},   {"uparrow", 173},   {"alpha", 97},   {"beta", 98},
   {"gamma", 103},   {"delta", 100},   {"zeta", 122},   {"eta", 104},
   {"theta", 113},   {"iota", 105},   {"kappa", 107},   {"lambda", 108},
   {"mu", 109},   {"nu", 110},   {"xi", 120},   {"pi", 112},
   {"rho", 114},   {"sigma", 115},   {"tau", 116},   {"upsilon", 117},
   {"phi", 102},   {"chi", 99},   {"psi", 121},   {"omega", 119},
   {"varepsilon", 101},   {"vartheta", 74},   {"varpi", 118},   {"varsigma", 86},
   {"varphi", 106},   {"Gamma", 71},   {"Delta", 68},   {"Theta", 81},
   {"Lambda", 76},   {"Xi", 88},   {"Pi", 80},   {"Sigma", 83},
   {"Upsilon", 161},   {"Phi", 70},   {"Psi", 89},   {"Omega", 87},
   {"aleph", 192},   {"wp", 195},   {"Re", 194},   {"Im", 193},
   {"partial", 182},   {"infty", 165},   {"prime", 162},   {"nabla", 209},
   {"bot", 94},   {"forall", 34},   {"exists", 36},   {"neg", 216},
   {"sharp", 35},   {"clubsuit", 167},   {"diamondsuit", 168},   {"heartsuit", 169},
   {"spadesuit", 170},   {"int", 242},   {"prod", 213},   {"sum", 229},
   {"wedge", 217},   {"vee", 218},   {"cap", 199},   {"cup", 200},
   {"diamond", 224},   {"bullet", 183},   {"div", 184},   {"oslash", 198},
   {"otimes", 196},   {"oplus", 197},   {"pm", 177},   {"cdot", 215},
   {"times", 180},   {"propto", 181},   {"mid", 124},   {"Leftrightarrow", 219},
   {"Leftarrow", 220},   {"Rightarrow", 222},   {"leq", 163},   {"geq", 179},
   {"approx", 187},   {"supset", 201},   {"subset", 204},   {"supseteq", 202},
   {"subseteq", 205},   {"in", 206},   {"ni", 39},   {"leftrightarrow", 171},
   {"leftarrow", 172},   {"rightarrow", 174},   {"sim", 126},   {"equiv", 186},
   {"colon", 58},   {"uparrow", 173},   {"downarrow", 175},   {"Uparrow", 221},
   {"Downarrow", 223},   {"rangle", 241},   {"langle", 225},   {"rceil", 249},
   {"lceil", 233},   {"rfloor", 251},   {"lfloor", 235},   {"angle", 208},
   {"therefore", 92},   {"neq", 185},   {"textdegree", 176},   {"cong", 64},
   {"downarrow", 175},   {"Uparrow", 221},   {"Downarrow", 223},   {"rangle", 241},
   {"langle", 225},   {"rceil", 249},   {"lceil", 233},   {"rfloor", 251},
   {"lfloor", 235},   {"angle", 208},   {"therefore", 92},   {"neq", 185},
   {"textdegree", 176},   {"cong", 64},   {"surd", 214}
   };

// TeX computer modern math font with hbar
map<string, unsigned char> map_tex_cmmi = {
  {"alpha", 174},  {"beta", 175},  {"gamma", 176},
  {"delta", 177},  {"epsilon", 178},  {"zeta", 179},  {"eta", 180},
  {"theta", 181},  {"iota", 182},  {"kappa", 183},  {"lambda", 184},
  {"mu", 185},  {"nu", 186},  {"xi", 187},  {"pi", 188},
  {"rho", 189},  {"sigma", 190},  {"tau", 191},  {"upsilon", 192},
  {"phi", 193},  {"chi", 194},  {"psi", 195},  {"omega", 33},
  {"varepsilon", 34},  {"vartheta", 35},  {"varpi", 36},  {"varrho", 37},
  {"varsigma", 38},  {"varphi", 39},  {"Gamma", 161},  {"Delta", 162},
  {"Theta", 163},  {"Lambda", 164},  {"Xi", 165},  {"Pi", 166},
  {"Sigma", 167},  {"Upsilon", 168},  {"Phi", 169},  {"Psi", 170},
  {"Omega", 172}, {"hbar", 199}
};


string UTF8toISO8859_1(const char * in)
{
    std::string out;
    if (in == NULL)
        return out;

    unsigned int codepoint;
    while (*in != 0)
    {
        unsigned char ch = static_cast<unsigned char>(*in);
        if (ch <= 0x7f)
            codepoint = ch;
        else if (ch <= 0xbf)
            codepoint = (codepoint << 6) | (ch & 0x3f);
        else if (ch <= 0xdf)
            codepoint = ch & 0x1f;
        else if (ch <= 0xef)
            codepoint = ch & 0x0f;
        else
            codepoint = ch & 0x07;
        ++in;
        if (((*in & 0xc0) != 0x80) && (codepoint <= 0x10ffff))
        {
            if (codepoint <= 255)
            {
                out.append(1, static_cast<char>(codepoint));
            }
            else
            {
              fprintf(stderr, "Caracter fuera de límites %c\n", ch);
                // do whatever you want for out-of-bounds characters
            }
        }
    }
    return out;
}

FontFace change_font_face(unsigned char code_font_face, FontFace font_face) {
  switch (code_font_face) {
  case 'b':
    if (font_face < FN_SYMBOL)                      // There is no bold for symbol
      font_face = (FontFace)(font_face|FN_BOLD);
    else
      font_face = FN_SERIF_BOLD;
    break;
  case 'e':
  case 'i':
    if (font_face < FN_SYMBOL)                      // There is no oblique for symbol
      font_face = (FontFace)(font_face|FN_ITALIC);
    else
      font_face = FN_SERIF_ITALIC;
    break;
  case 'g':
    font_face = FN_SYMBOL;
    break;
  case 'r': 
    font_face = FN_TIMES_ROMAN;
    break;
  case 's': 
    font_face = FN_SANSERIF;    
    break;
  case 'c':
  case 't':
    font_face = FN_COURIER;
    break;
  case '$':
    font_face = FN_TEX_CMMI;
    is_using_fontcmmi = true;
    break;
  default:
    fprintf(stderr, "Warning: font face style unknown <%c>.\n", code_font_face);
    font_face = FN_DEFAULT;
    break;
  }
  return font_face;
}


FontFace get_font_face_from_string(string s) {
  string out;
  FontFace font_face = FN_DEFAULT;
  if (s.find("roman") != std::string::npos)
    out += 'r';
  if (s.find("sanserif")!=std::string::npos || s.find("helvetic")!=std::string::npos)
    out += 's';
  if (s.find("courier") != std::string::npos)
    out += 't';
  if (s.find("bold") != std::string::npos)
    out += 'b';
  if (s.find("italic") != std::string::npos)
    out += 'i';
  if (out.empty()) {
    fprintf(stderr, "Warning: font face style unknown <%s>.\n", s.c_str());
    return FN_NOFACE;
  }
  for (auto &cff : out) {
    font_face = change_font_face(cff, font_face);
  }
  return font_face;
}


unsigned char get_symbol_code(string symbol_name, FontFace &font_face)
{
  unsigned char symbol_code;
   
  /*if (map_symbol.find(symbol_name) != map_symbol.end()) {  
    symbol_code = map_symbol[symbol_name];
    font_face = FN_SYMBOL;
  } else if (map_tex_cmmi.find(symbol_name) != map_tex_cmmi.end()) {  
    symbol_code = map_tex_cmmi[symbol_name];
    font_face = FN_TEX_CMMI;
    is_using_fontcmmi = true;
  }*/
  if (map_tex_cmmi.find(symbol_name) != map_tex_cmmi.end()) {  
    symbol_code = map_tex_cmmi[symbol_name];
    font_face = FN_TEX_CMMI;
    is_using_fontcmmi = true;
  } else if (map_symbol.find(symbol_name) != map_symbol.end()) {  
    symbol_code = map_symbol[symbol_name];
    font_face = FN_SYMBOL;
  } else {
    fprintf(stderr, "Warning: symbol name unknown %s\n", symbol_name.c_str());
    symbol_code = 0;
    font_face = FN_DEFAULT;
  }
  //printf("symbol %s %d\n", symbol_name.c_str(), symbol_code);
  return symbol_code;
}


// Auxiliary text accumulator to create text chunks
string accum;
TextState text_state;
stack<TextState> tstack;
Text *text;
TextLine *text_line;


void tspush() 
{
  //printf("pushing %s\n", text_state.str().c_str());
  tstack.push(text_state);
}

void tspop()
{
  text_state = tstack.top();
  tstack.pop();
  //printf("poping %s\n", text_state.str().c_str());
}

// text flush creates new Text with full font state and add it to list
void textflush()
{
  if (accum.size() > 0) {
    if (text==nullptr) {
      text = new Text();
    } 
    if (text_line==nullptr)
      text_line = new TextLine();

    if ((text->length() > 0) && (text->getState()==text_state)) {
      text->addText(accum);
      //printf("Cadena add <%s> text state %s\n", accum.c_str(), 
      //  text_state.str().c_str());
    }
    else {
      if (text->length() > 0) {
        text_line->addText(text);
        text = new Text();
      }
      text->setState(text_state);
      text->setText(accum);
      //printf("Cadena set <%s> text state %s\n", accum.c_str(), 
      //  text_state.str().c_str());
    } 
    accum = "";
  } else {
    if (text && text->length() > 0) {
      text_line->addText(text);
      text = nullptr;
    }
  }
}


GraphicsItem* parse_text(string input_utf8, FontFace ff)
{
  FontFace font_face = ff;
  bool tex_mode_on = false;
  text = nullptr;
  text_line = nullptr;
  text_state = TextState();
  text_state.font_face = font_face;
  string input = UTF8toISO8859_1(input_utf8.c_str());
  for (char & c : input) 
    if (c < 0) {
      //printf("no mames\n");
      is_using_reencode = true;
      break;
    }
  int v_end, it=0, iend = input.size();
 
  while (it < iend) {
    unsigned char c = input[it];
    switch(c) {
    case '_':
    case '^': 
      textflush();
      tspush();
      it++;
      text_state.font_size *= fn_relsz_script;
      if (c=='_')
        text_state.script--;
      else
        text_state.script++;
      if (input[it]=='{') {
        break;
      } else if (input[it]=='\\') {
        v_end = input.find_first_not_of(ALFABETIC, ++it);
        if (v_end == string::npos) // Last word in string
          v_end = iend;
        if (v_end > it && v_end <= iend) {
          string variable = input.substr(it, v_end - it);
          unsigned char code = get_symbol_code(variable, font_face);
          if (code > 0) {
            text_state.font_face = font_face;
            accum.push_back(code);
          } else {
            fprintf(stderr, "Error: Unrecognized variable <%s>.\n", variable.c_str());
            break;
          }
        }
        //it = v_end-1;
      } else {
        v_end = input.find_first_of(TOKENLIM, it);
        if (v_end == string::npos) // Last word in string
          v_end = iend;
        if (it >= v_end){
          //printf("puto %d %d %c\n", it, v_end, input[it]);
                  break;}
        accum = input.substr(it, v_end - it);
      }
      textflush();
      tspop();
      it = v_end-1;
      break;
    case '{':
        textflush();
        tspush();
        //printf("abriendo\n");
        break;
    case '}':
        textflush();
        tspop();
        //printf("cerrando\n");
        break;
    case '$':
      if (text_state.font_face!=FN_TEX_CMMI) {
        textflush();
        tspush();
        text_state.font_face = FN_TEX_CMMI;
        tex_mode_on = true;
      } else {
        textflush();
        tex_mode_on = false;
        tspop();
      }
      break;
    case '/':
      font_face = change_font_face(input[++it], text_state.font_face);
      if (font_face != text_state.font_face) {
        textflush();
        text_state.font_face = font_face;
      }
      break;
    case '\\': 
      textflush();
      v_end = input.find_first_not_of(ALFABETIC, ++it);
      if (v_end == string::npos) // Last word in string
        v_end = iend;
      if (v_end > it && v_end <= iend) {
        string variable = input.substr(it, v_end - it);
        unsigned char code = get_symbol_code(variable, font_face);
        if (code > 0) {
          tspush();
          text_state.font_face = font_face;
          accum.push_back(code);
          textflush();
          tspop();
        } else {
          fprintf(stderr, "Error: Unrecognized variable <%s>.\n", variable.c_str());
        }
        it = v_end-1;
      }
      break; 
    default:
      accum.push_back(c);
    }
    it++;
  }
  if (accum.length() > 0)
    textflush();
  
  if (text_line->length() > 1) {
    textflush();
    return text_line;
  } else if (text_line->length()==1 && text) {
    text_line->addText(text);
    return text_line;
  } else {
    //printf("simple texto %s\n", text->getText().c_str());
    return text;
    }
}

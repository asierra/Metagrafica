#ifndef __MGLEXER_H
#define __MGLEXER_H

#if ! defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif

#include "mgpp_tab.h"
#include "primitives.h"
#include <map>
#include <string>
#include <vector>
#include <memory>
#include <iostream>

using std::map;
using std::string;
using std::vector;

class MGLexer : public yyFlexLexer {
public:
    MGLexer(std::istream* in) : yyFlexLexer(in) { init_tables(); }
    virtual ~MGLexer() = default;

    using yyFlexLexer::yylex;
    virtual int yylex();
    
    void setBegin(int s) { 
        yy_start = 1 + 2 * s; 
    }

    void push_stream(std::unique_ptr<std::istream> stream) {
        include_streams.push_back(std::move(stream));
    }

    YYSTYPE yylval;
    YYSTYPE yylvalaux;

private:
    void init_tables();
    int busca_key(const char* key);
    bool search_mat(string key, int &opm, int &dfm);
    
    struct KeyInfo {
        int token;
        int type;
    };
    
    map<string, KeyInfo> keyword_map;
    map<string, MatrixOperation> map_opmat;
    map<string, PredefinedMatrix> map_dfmat;
    
    std::vector<std::unique_ptr<std::istream>> include_streams;
};

#endif
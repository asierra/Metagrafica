#CXX = g++
CXX = clang++
CXXFLAGS = -g -std=c++14 -ffunction-sections -fdata-sections
LIBS = -lm -Wmultichar
LDFLAGS = -g -Wpedantic -Wl,--gc-sections
CPPFLAGS = -I./include -I./third_party/libharu/include -fno-rtti -fno-exceptions -Wpedantic -Wall -Wsuggest-override -O3
HARU_CFLAGS = -O2 -ffunction-sections -fdata-sections -I$(HARUDIR)/include

SHELL = /bin/sh
PREFIX = /usr/local
MANPREFIX ?= ${PREFIX}/share/man

SRCDIR = src
INCDIR = include
OBJDIR = obj
BINDIR = bin
MANDIR = man

HARUDIR = third_party/libharu
HARU_SRCS = $(wildcard $(HARUDIR)/src/*.c)
HARU_OBJS = $(patsubst $(HARUDIR)/src/%.c, $(OBJDIR)/haru/%.o, $(HARU_SRCS))

SRCS = $(addprefix $(SRCDIR)/, Display.cpp EPSDisplay.cpp PDFDisplay.cpp SVGDisplay.cpp main.cpp structure.cpp matrix.cpp \
	primitives.cpp lexmg.cpp text.cpp text_parser.cpp Parser.cpp splines.cpp)

OBJS = $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SRCS))

all: $(BINDIR)/mg $(MANDIR)/mg.1

$(MANDIR)/mg.1: $(MANDIR)/mg.1.md
	pandoc $< -s -t man -o $@

$(MANDIR)/mg.1.pdf: $(MANDIR)/mg.1.md
	pandoc $< -s -t pdf -o $@

$(SRCDIR)/lexmg.cpp: $(SRCDIR)/mgpp.l
	flex -o $@ $<

# Lexer V3 (en desarrollo): src/lexer.l -> src/lexv3.cpp
$(SRCDIR)/lexv3.cpp: $(SRCDIR)/lexer.l
	flex -o $@ $<

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) $< -o $@

$(OBJDIR)/haru/%.o: $(HARUDIR)/src/%.c | $(OBJDIR)/haru
	$(CC) -c $(HARU_CFLAGS) $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/haru:
	mkdir -p $(OBJDIR)/haru

$(BINDIR):
	mkdir -p $(BINDIR)

# --- Cutover (§22.6): bin/mg ES el compilador V3 -----------------------------
# main.cpp es el entry point V3 (parserv3.h: buildFromSource/g_baseDir/g_flags).
# Objetos del motor reutilizados por el parser V3 (incluye PDF/haru para paridad
# de backends con el V1). El front-end V1 (Parser.cpp, lexmg.cpp) queda fuera
# del binario; sigue congelado en la rama v1-legacy.
V3_ENGINE_OBJS = $(addprefix $(OBJDIR)/, Display.o EPSDisplay.o SVGDisplay.o PDFDisplay.o structure.o \
	matrix.o primitives.o text.o text_parser.o splines.o)

$(BINDIR)/mg: $(SRCDIR)/main.cpp $(SRCDIR)/lexv3.cpp $(SRCDIR)/parserv3.cpp $(V3_ENGINE_OBJS) $(HARU_OBJS) $(INCDIR)/ast.h $(INCDIR)/tokens.h $(INCDIR)/parserv3.h | $(BINDIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(SRCDIR)/main.cpp $(SRCDIR)/lexv3.cpp $(SRCDIR)/parserv3.cpp $(V3_ENGINE_OBJS) $(HARU_OBJS) -o $@ $(LDFLAGS) $(LIBS) -lz

# Alias histórico: v3test == mg (mismo compilador V3).
v3test: $(BINDIR)/mg | $(BINDIR)
	cp -f $(BINDIR)/mg $(BINDIR)/v3test

install: $(BINDIR)/mg $(MANDIR)/mg.1
	install -m 755 $(BINDIR)/mg $(PREFIX)/bin
	install $(MANDIR)/mg.1 ${MANPREFIX}/man1/

uninstall:
	rm $(PREFIX)/bin/mg
	rm ${MANPREFIX}/man1/mg.1

clean:
	rm -rf $(OBJDIR) $(BINDIR) $(MANDIR)/mg.1 $(SRCDIR)/lexmg.cpp $(SRCDIR)/lexv3.cpp
# DO NOT DELETE

$(OBJDIR)/Display.o: $(INCDIR)/Display.h $(INCDIR)/primitives.h $(INCDIR)/matrix.h $(INCDIR)/text.h $(INCDIR)/structures.h
$(OBJDIR)/EPSDisplay.o: $(INCDIR)/EPSDisplay.h $(INCDIR)/Display.h $(INCDIR)/primitives.h
$(OBJDIR)/EPSDisplay.o: $(INCDIR)/matrix.h $(INCDIR)/text.h $(INCDIR)/font_cmmi.h $(INCDIR)/structures.h
$(OBJDIR)/PDFDisplay.o: $(INCDIR)/PDFDisplay.h $(INCDIR)/Display.h $(INCDIR)/primitives.h
$(OBJDIR)/PDFDisplay.o: $(INCDIR)/matrix.h $(INCDIR)/text.h $(INCDIR)/structures.h
$(OBJDIR)/SVGDisplay.o: $(INCDIR)/SVGDisplay.h $(INCDIR)/Display.h $(INCDIR)/primitives.h
$(OBJDIR)/SVGDisplay.o: $(INCDIR)/matrix.h $(INCDIR)/text.h $(INCDIR)/structures.h
$(OBJDIR)/lexmg.o: $(INCDIR)/mgpp_tab.h
$(OBJDIR)/main.o: $(INCDIR)/structures.h $(INCDIR)/text.h $(INCDIR)/primitives.h $(INCDIR)/matrix.h
$(OBJDIR)/main.o: $(INCDIR)/EPSDisplay.h $(INCDIR)/Parser.h $(INCDIR)/version.h
$(OBJDIR)/matrix.o: $(INCDIR)/matrix.h
$(OBJDIR)/Parser.o: $(INCDIR)/Parser.h $(INCDIR)/structures.h $(INCDIR)/text.h $(INCDIR)/primitives.h
$(OBJDIR)/Parser.o: $(INCDIR)/matrix.h $(INCDIR)/mgpp_tab.h $(INCDIR)/text_parser.h $(INCDIR)/splines.h
$(OBJDIR)/primitives.o: $(INCDIR)/primitives.h $(INCDIR)/matrix.h $(INCDIR)/Display.h $(INCDIR)/text.h
$(OBJDIR)/splines.o: $(INCDIR)/splines.h $(INCDIR)/primitives.h $(INCDIR)/matrix.h
$(OBJDIR)/structure.o: $(INCDIR)/structures.h $(INCDIR)/text.h $(INCDIR)/primitives.h
$(OBJDIR)/structure.o: $(INCDIR)/matrix.h $(INCDIR)/Display.h
$(OBJDIR)/text.o: $(INCDIR)/text.h $(INCDIR)/primitives.h $(INCDIR)/matrix.h $(INCDIR)/Display.h
$(OBJDIR)/text_parser.o: $(INCDIR)/text.h $(INCDIR)/primitives.h $(INCDIR)/matrix.h

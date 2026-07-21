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
HARU_LIB = $(OBJDIR)/haru/libharu.a

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

# -MMD -MP: el compilador escribe obj/X.d con los headers de los que depende X.cpp,
# y esos .d se incluyen abajo. Sin esto la regla no tenía NINGUNA dependencia de
# header, así que tocar include/ no recompilaba nada: es lo que ya había pasado con
# version.h (parcheado a mano solo en el enlace de bin/mg, ver abajo) y volvió a
# morder el 2026-07-20 al regenerar las fuentes vendorizadas —los tres backends
# siguieron con la fuente vieja y el golden pasaba midiendo un binario rancio—.
# -MP añade targets falsos para que borrar un header no rompa el build.
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) -MMD -MP $< -o $@

$(OBJDIR)/haru/%.o: $(HARUDIR)/src/%.c | $(OBJDIR)/haru
	$(CC) -c $(HARU_CFLAGS) $< -o $@

$(HARU_LIB): $(HARU_OBJS)
	$(AR) rcs $@ $(HARU_OBJS)

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

# main.cpp, lexv3.cpp y parserv3.cpp se compilan DIRECTO en este enlace (no pasan por
# obj/*.o), así que sus headers tienen que estar aquí: no hay un obj/main.o al que
# colgarle dependencias. Faltaba version.h → cambiar la versión no recompilaba nada
# y `mg -v` seguía mintiendo hasta un `make clean` (bug encontrado 2026-07-16).
$(BINDIR)/mg: $(SRCDIR)/main.cpp $(SRCDIR)/lexv3.cpp $(SRCDIR)/parserv3.cpp $(V3_ENGINE_OBJS) $(HARU_LIB) \
              $(INCDIR)/ast.h $(INCDIR)/tokens.h $(INCDIR)/parserv3.h $(INCDIR)/version.h \
              $(INCDIR)/structures.h $(INCDIR)/EPSDisplay.h $(INCDIR)/PDFDisplay.h $(INCDIR)/SVGDisplay.h | $(BINDIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(SRCDIR)/main.cpp $(SRCDIR)/lexv3.cpp $(SRCDIR)/parserv3.cpp $(V3_ENGINE_OBJS) -o $@ -L$(OBJDIR)/haru -lharu $(LDFLAGS) $(LIBS) -lz

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
# Dependencias de headers: AUTOMÁTICAS (los obj/*.d que genera -MMD, ver arriba).
# Sustituyen a la lista que mantenía `makedepend` aquí abajo, que se había podrido:
# decía que EPSDisplay.o depende de font_cmmi.h y NO de font_lmmath_eps.h, que es
# posterior. Una lista de dependencias a mano solo es correcta el día que se genera.
-include $(OBJS:.o=.d)
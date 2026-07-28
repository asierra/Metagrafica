#CXX = g++
CXX = clang++
CXXFLAGS = -g -std=c++14 -ffunction-sections -fdata-sections
LIBS = -lm -Wmultichar
# Descartar secciones muertas: GNU ld y el de MinGW usan --gc-sections; el de
# Apple NO lo conoce y aborta el enlace («ld: unknown options: --gc-sections»),
# ahí se llama -dead_strip. Lo destapó la primera corrida de CI en macOS: el
# proyecto nunca se había compilado en un Mac.
ifeq ($(shell uname -s),Darwin)
  GC_SECTIONS = -Wl,-dead_strip
else
  GC_SECTIONS = -Wl,--gc-sections
endif
LDFLAGS = -g -Wpedantic $(GC_SECTIONS)
# -D_USE_MATH_DEFINES: M_PI y compañía NO son de C++ estándar. glibc las da
# siempre, pero MinGW solo si la macro está definida ANTES de <cmath> — por eso
# va en la línea de compilación y no en un header. Sin ella el build cruzado a
# Windows muere en matrix.h:23. Inocua fuera de Windows.
CPPFLAGS = -I./include -I./third_party/libharu/include -fno-rtti -fno-exceptions -Wpedantic -Wall -Wsuggest-override -O3 -D_USE_MATH_DEFINES -DMG_LIBDIR='"$(LIBDIR)"'
HARU_CFLAGS = -O2 -ffunction-sections -fdata-sections -I$(HARUDIR)/include

# --- Compilación cruzada a Windows (MinGW) -----------------------------------
# Windows es el hueco que más usuarios cuesta: no hay compilador de sistema, así
# que lo que se reparte es un .exe ya hecho. Se cruza desde Linux con MinGW-w64,
# que es como se armó también la versión de 1999:
#
#   make CROSS=x86_64-w64-mingw32          # → bin/mg.exe (64 bits)
#   make CROSS=i686-w64-mingw32            # → bin/mg.exe (32 bits)
#
# Necesita el toolchain y zlib para MinGW (en Debian/Ubuntu: mingw-w64 y
# libz-mingw-w64-dev). Se enlaza ESTÁTICO a propósito: el .exe tiene que correr
# recién descomprimido, sin DLLs de gcc al lado ni instalador.
#
# ⚠️ La biblioteca de `include` (§15) no puede vivir en una ruta horneada como en
# Unix: en Windows el reparto es un .zip que se descomprime donde sea. El binario
# busca además `lib\` JUNTO AL .exe (parserv3.cpp, exeDir), así que el .zip lleva
# mg.exe y lib/ al mismo nivel y funciona sin instalar nada.
CROSS ?=
EXE   =
ifneq ($(CROSS),)
  CXX = $(CROSS)-g++
  CC  = $(CROSS)-gcc
  AR  = $(CROSS)-ar
  EXE = .exe
  # Los objetos del cruce van a SU PROPIO directorio. Si comparten obj/ con el
  # build nativo, `make` no los reconstruye —tienen fecha más nueva que el
  # fuente— y el enlace muere con «undefined reference» a símbolos que sí están:
  # son objetos ELF que el ld de MinGW no puede leer. Pasa en cuanto alternas
  # `make` y `make CROSS=…`, y el mensaje no dice nada de eso.
  OBJDIR = obj-win
  # -s (strip): el .exe se DESCARGA, no se depura. Sin esto son 15 MB de
  # símbolos que nadie va a leer.
  LDFLAGS += -static -static-libgcc -static-libstdc++ -s
endif

SHELL = /bin/sh
PREFIX = /usr/local
MANPREFIX ?= ${PREFIX}/share/man
# Biblioteca de .mg incluibles (§15): `include "x.mg"` la busca DESPUÉS de lo local.
# La ruta se hornea en el binario vía -DMG_LIBDIR (CPPFLAGS). Overridable para probar.
LIBDIR = $(PREFIX)/share/metagrafica/lib

SRCDIR = src
INCDIR = include
OBJDIR ?= obj
BINDIR = bin
MANDIR = man

HARUDIR = third_party/libharu
HARU_SRCS = $(wildcard $(HARUDIR)/src/*.c)
HARU_OBJS = $(patsubst $(HARUDIR)/src/%.c, $(OBJDIR)/haru/%.o, $(HARU_SRCS))
HARU_LIB = $(OBJDIR)/haru/libharu.a

SRCS = $(addprefix $(SRCDIR)/, Display.cpp EPSDisplay.cpp PDFDisplay.cpp SVGDisplay.cpp main.cpp structure.cpp matrix.cpp \
	primitives.cpp text.cpp text_parser.cpp splines.cpp)

OBJS = $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SRCS))

# La página de manual necesita `pandoc`, que es una dependencia de DOCUMENTACIÓN y no
# del compilador. Si está, `make` la genera como siempre; si no, produce el binario y
# avisa, en vez de fallar. Motivo: quien llega nuevo quiere `bin/mg`, y un `make` que
# se detiene por una herramienta que no va a usar es el primer obstáculo que se lleva
# por delante su interés (y en Windows/macOS pandoc es un instalador aparte).
PANDOC := $(shell command -v pandoc 2>/dev/null)

ifeq ($(PANDOC),)
all: $(BINDIR)/mg$(EXE)
	@echo "aviso: pandoc no encontrado; no se generó $(MANDIR)/mg.1 (el binario sí está en $(BINDIR)/mg)"
else
all: $(BINDIR)/mg$(EXE) $(MANDIR)/mg.1
endif

$(MANDIR)/mg.1: $(MANDIR)/mg.1.md
	pandoc $< -s -t man -o $@

$(MANDIR)/mg.1.pdf: $(MANDIR)/mg.1.md
	pandoc $< -s -t pdf -o $@

# Lexer V3: src/lexer.l -> src/lexv3.cpp
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
# de backends con el V1). El front-end V1 se BORRÓ del árbol el 2026-07-27, ya
# cerrado el traductor; sigue completo en la rama v1-legacy.
V3_ENGINE_OBJS = $(addprefix $(OBJDIR)/, Display.o EPSDisplay.o SVGDisplay.o PDFDisplay.o structure.o \
	matrix.o primitives.o text.o text_parser.o splines.o)

# main.cpp, lexv3.cpp y parserv3.cpp se compilan DIRECTO en este enlace (no pasan por
# obj/*.o), así que sus headers tienen que estar aquí: no hay un obj/main.o al que
# colgarle dependencias. Faltaba version.h → cambiar la versión no recompilaba nada
# y `mg -v` seguía mintiendo hasta un `make clean` (bug encontrado 2026-07-16).
$(BINDIR)/mg$(EXE): $(SRCDIR)/main.cpp $(SRCDIR)/lexv3.cpp $(SRCDIR)/parserv3.cpp $(V3_ENGINE_OBJS) $(HARU_LIB) \
              $(INCDIR)/ast.h $(INCDIR)/tokens.h $(INCDIR)/parserv3.h $(INCDIR)/version.h \
              $(INCDIR)/structures.h $(INCDIR)/EPSDisplay.h $(INCDIR)/PDFDisplay.h $(INCDIR)/SVGDisplay.h | $(BINDIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(SRCDIR)/main.cpp $(SRCDIR)/lexv3.cpp $(SRCDIR)/parserv3.cpp $(V3_ENGINE_OBJS) -o $@ -L$(OBJDIR)/haru -lharu $(LDFLAGS) $(LIBS) -lz

# Alias histórico: v3test == mg (mismo compilador V3).
v3test: $(BINDIR)/mg$(EXE) | $(BINDIR)
	cp -f $(BINDIR)/mg$(EXE) $(BINDIR)/v3test$(EXE)

install: $(BINDIR)/mg$(EXE) $(MANDIR)/mg.1
	install -m 755 $(BINDIR)/mg $(PREFIX)/bin
	install $(MANDIR)/mg.1 ${MANPREFIX}/man1/
	install -d $(LIBDIR)
	install -m 644 lib/*.mg $(LIBDIR)

uninstall:
	rm $(PREFIX)/bin/mg
	rm ${MANPREFIX}/man1/mg.1
	rm -f $(LIBDIR)/*.mg
	-rmdir $(LIBDIR)

clean:
	rm -rf obj obj-win $(BINDIR) $(MANDIR)/mg.1 $(SRCDIR)/lexv3.cpp
# Dependencias de headers: AUTOMÁTICAS (los obj/*.d que genera -MMD, ver arriba).
# Sustituyen a la lista que mantenía `makedepend` aquí abajo, que se había podrido:
# decía que EPSDisplay.o depende de font_cmmi.h y NO de font_lmmath_eps.h, que es
# posterior. Una lista de dependencias a mano solo es correcta el día que se genera.
-include $(OBJS:.o=.d)
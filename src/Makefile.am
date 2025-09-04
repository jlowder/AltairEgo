if IS_EMSCRIPTEN
noinst_PROGRAMS = altair_ego
altair_ego_SOURCES = \
  main.cpp \
  lexer.cpp \
  parser.cpp \
  interpreter.cpp \
  variable.cpp \
  functions.cpp \
  lexer.h \
  parser.h \
  interpreter.h \
  variable.h \
  functions.h
altair_ego_LDFLAGS = -s WASM=1 -s EXTRA_EXPORTED_RUNTIME_METHODS='["ccall", "cwrap"]' --shell-file $(top_srcdir)/shell.html
else
bin_PROGRAMS = altair_ego
altair_ego_SOURCES = \
  main.cpp \
  lexer.cpp \
  parser.cpp \
  interpreter.cpp \
  variable.cpp \
  functions.cpp \
  lexer.h \
  parser.h \
  interpreter.h \
  variable.h \
  functions.h
endif

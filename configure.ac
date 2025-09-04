AC_INIT([altair-ego], [0.9.0])
AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CXX

AC_CANONICAL_HOST
is_emscripten=no
case "$host_alias" in
  wasm32-*-emscripten* | asmjs-*-emscripten*)
    is_emscripten=yes
    ;;
esac
AM_CONDITIONAL([IS_EMSCRIPTEN], [test "$is_emscripten" = "yes"])

AC_CONFIG_FILES([
  Makefile
  src/Makefile
])
AC_OUTPUT

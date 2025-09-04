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
    # The configure test for cross-compilation fails when run with node.
    # We know we are cross-compiling, so we can set the result of the check directly
    # to avoid running the failing test.
    ac_cv_prog_cxx_cross=yes
    ;;
esac
AM_CONDITIONAL([IS_EMSCRIPTEN], [test "$is_emscripten" = "yes"])

AC_CONFIG_FILES([
  Makefile
  src/Makefile
])
AC_OUTPUT

dnl config.m4 for extension llhttp

PHP_ARG_WITH([llhttp],
  [for llhttp support],
  [AS_HELP_STRING([--with-llhttp],
    [Include llhttp support])])

if test "$PHP_LLHTTP" != "no"; then
  dnl Use bundled llhttp source files
  AC_MSG_CHECKING([for bundled llhttp source])
  
  dnl Check for bundled llhttp files
  if test -r "llhttp.h" && test -r "api.c"; then
    AC_MSG_RESULT([found])
    LLHTTP_SOURCES="api.c http.c llhttp.c"
  else
    AC_MSG_RESULT([not found - checking absolute paths])
    if test -r "$ext_srcdir/llhttp.h" && test -r "$ext_srcdir/api.c"; then
      AC_MSG_RESULT([found in srcdir])
      LLHTTP_SOURCES="api.c http.c llhttp.c"
    else
      AC_MSG_ERROR([Bundled llhttp source files not found])
    fi
  fi

  dnl Define extension
  AC_DEFINE(HAVE_LLHTTP, 1, [Whether you have llhttp])
  
  dnl Add all source files including bundled llhttp
  PHP_NEW_EXTENSION(llhttp, 
    php_llhttp.c \
    llhttp_parser.c \
    llhttp_events.c \
    llhttp_error.c \
    $LLHTTP_SOURCES,
    $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
  
  dnl Add current directory to include path for llhttp.h
  PHP_ADD_INCLUDE($ext_srcdir)
  
  dnl Add header files
  PHP_ADD_BUILD_DIR($ext_builddir)
fi
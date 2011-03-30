AC_INIT([qsar], [0.1], [jfree143dev@gmail.com])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CXX

PKG_PROG_PKG_CONFIG()
PKG_CHECK_MODULES(QT4, [QtCore >= 4.0.0],,)

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
    Makefile
    src/Makefile
])
AC_OUTPUT

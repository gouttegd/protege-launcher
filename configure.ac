AC_INIT([Protégé launcher], [0.1.0],
        [dpg44@cam.ac.uk], [protege])
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_LIBOBJ_DIR([lib])
AC_USE_SYSTEM_EXTENSIONS
AM_INIT_AUTOMAKE([foreign subdir-objects silent-rules])

dnl Check for development tools
AC_PROG_CC

dnl Check for dlopen
AC_SEARCH_LIBS([dlopen], [dl])

dnl Java Development Kit
dnl We don't try to detect it; whoever calls configure should
dnl specify the path to the JDK
AC_ARG_WITH([jdk],
            [AS_HELP_STRING([--with-jdk],
                            [Path to the Java Development Kit])],
            [CPPFLAGS="$CPPFLAGS -I${with_jdk}/include -I${with_jdk}/include/linux"],
            [AC_MSG_ERROR([No JDK specified.])])

dnl Output files
AC_CONFIG_FILES([Makefile])
AC_OUTPUT

dnl Summary
echo "

${PACKAGE_NAME} version ${PACKAGE_VERSION}
Configuration complete

Compiler:           '${CC} ${CFLAGS} ${CPPFLAGS}'

"

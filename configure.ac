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
            [AS_IF([test -f ${with_jdk}/include/jni.h],
                   [JDK_CPPFLAGS="-I${with_jdk}/include"],
                   [AC_MSG_ERROR(JDK not found at '${with_jdk}'.)])],
            [AC_MSG_ERROR([No JDK specified.])])

dnl Host-based tweaks
AC_CANONICAL_HOST
AS_CASE([$host],
        [*-*-linux-*], [AC_DEFINE([PROTEGE_LINUX], [1], [GNU/Linux version])
                        JDK_CPPFLAGS="$JDK_CPPFLAGS -I${with_jdk}/include/linux"],
        [*-*-darwin*], [AC_DEFINE([PROTEGE_MACOS], [1], [macOS version])
                        JDK_CPPFLAGS="$JDK_CPPFLAGS -I${with_jdk}/include/darwin"],
        [*-*-mingw32], [AC_DEFINE([PROTEGE_WIN32], [1], [Windows version])
                        JDK_CPPFLAGS="$JDK_CPPFLAGS -I${with_jdk}/include/win32"],
        [AC_MSG_ERROR(["Unsupported host: ${host}"])])

AC_SUBST([JDK_CPPFLAGS])

dnl Output files
AC_CONFIG_FILES([Makefile])
AC_OUTPUT

dnl Summary
echo "

${PACKAGE_NAME} version ${PACKAGE_VERSION}
Configuration complete

Host:               '${host}'
Compiler:           '${CC} ${CFLAGS} ${CPPFLAGS}'
Java flags:         '${JDK_CPPFLAGS}'

"

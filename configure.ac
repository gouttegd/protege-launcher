AC_INIT([Protégé launcher], [0.3.1],
        [dpg44@cam.ac.uk], [protege-launcher])
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_LIBOBJ_DIR([lib])
AC_USE_SYSTEM_EXTENSIONS
AM_INIT_AUTOMAKE([foreign subdir-objects silent-rules])

dnl Program infos
PROTEGE_WINDOWS_VERSION="5, 6, 3, 0"
PROTEGE_VERSION="5.6.3"
AC_SUBST([PROTEGE_WINDOWS_VERSION])
AC_SUBST([PROTEGE_VERSION])

dnl Check for development tools
AC_PROG_CC

dnl Check for err* functions, and provide replacements as needed
dnl Check for some non-ibiquitous functions
ICP_CHECK_NOTCH_FUNCS

dnl Check for dlopen
AC_CHECK_HEADERS([dlfcn.h], [], [AC_CONFIG_LINKS([dlfcn.h:lib/dlopen.compat.h])])
AC_SEARCH_LIBS([dlopen], [dl])
AC_REPLACE_FUNCS([dlopen])

dnl Check for Windows API functions
AC_CHECK_HEADERS([windows.h])
AC_SEARCH_LIBS([PathFileExistsA], [shlwapi])
AC_CHECK_TOOL([WINDRES], [windres])

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

dnl Support for macOS universal binaries
AC_ARG_ENABLE([fat-binaries],
              [AS_HELP_STRING([--disable-fat-binaries],
                              [On macOS, only build for the current architecture.])],
              [build_fat_binaries=$enableval],
              [build_fat_binaries=yes])
AS_IF([test "x$build_fat_binaries" = xyes],
      [MACOS_EXTRA_CFLAGS="$MACOS_EXTRA_CFLAGS -arch x86_64 -arch arm64"],
      [MACOS_EXTRA_CFLAGS="$MACOS_EXTRA_CFLAGS"])

dnl Set minimal version of macOS targeted
AC_ARG_WITH([min-osx-version],
            [AS_HELP_STRING([--with-min-osx-version],
                            [Specify the minimum macOS version the launcher will run on.])],
            [MACOS_EXTRA_CFLAGS="$MACOS_EXTRA_CFLAGS -mmacosx-version-min=$withval"],
            [MACOS_EXTRA_CFLAGS="$MACOS_EXTRA_CFLAGS"])

dnl Host-based tweaks
AC_CANONICAL_HOST
AS_CASE([$host],
        [*-*-linux-*], [AC_DEFINE([PROTEGE_LINUX], [1], [GNU/Linux version])
                        JDK_CPPFLAGS="$JDK_CPPFLAGS -I${with_jdk}/include/linux"
                        PROTEGE_PLATFORM=linux],
        [*-*-darwin*], [AC_DEFINE([PROTEGE_MACOS], [1], [macOS version])
                        JDK_CPPFLAGS="$JDK_CPPFLAGS -I${with_jdk}/include/darwin"
                        CFLAGS="$CFLAGS $MACOS_EXTRA_CFLAGS"
                        LDFLAGS="$LDFLAGS -framework Cocoa"
                        PROTEGE_PLATFORM=macos],
        [*-*-mingw32], [AC_DEFINE([PROTEGE_WIN32], [1], [Windows version])
                        JDK_CPPFLAGS="$JDK_CPPFLAGS -I${with_jdk}/include/win32"
                        CFLAGS="$CFLAGS -mwindows"
                        PROTEGE_PLATFORM=windows],
        [AC_MSG_ERROR(["Unsupported host: ${host}"])])

AC_SUBST([JDK_CPPFLAGS])

AM_CONDITIONAL([PROTEGE_WIN32], [test x$PROTEGE_PLATFORM = xwindows])

dnl Output files
AC_CONFIG_FILES([Makefile src/winres.rc])
AC_OUTPUT

dnl Summary
echo "

${PACKAGE_NAME} version ${PACKAGE_VERSION}
Configuration complete

Host:               '${host}'
Compiler:           '${CC} ${CFLAGS} ${CPPFLAGS}'
Java flags:         '${JDK_CPPFLAGS}'

"

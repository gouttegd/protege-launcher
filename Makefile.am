ACLOCAL_AMFLAGS = -I m4 --install

bin_PROGRAMS = protege

convlib_sources = lib/xmem.c lib/xmem.h

protege_SOURCES = src/protege.c src/options.h src/options.c \
		  src/util.c src/util.h src/java.c src/java.h \
		  $(convlib_sources)

AM_CPPFLAGS = -I$(top_srcdir)/lib ${JDK_CPPFLAGS}
LDADD = $(LIBOBJS)

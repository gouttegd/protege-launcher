ACLOCAL_AMFLAGS = -I m4 --install

dist_doc_DATA = AUTHORS COPYING.BSD COPYING.GPL README.md src/protege.ico

bin_PROGRAMS = protege

convlib_sources = lib/xmem.c lib/xmem.h lib/compat.h

protege_SOURCES = src/protege.c src/options.h src/options.c \
		  src/util.c src/util.h src/java.c src/java.h \
		  $(convlib_sources)

AM_CPPFLAGS = -I$(top_srcdir)/lib $(JDK_CPPFLAGS)

if PROTEGE_WIN32
  LDADD = $(LIBOBJS) src/winres.$(OBJEXT)
else
  LDADD = $(LIBOJS)
endif

src/winres.$(OBJEXT): src/winres.rc
	$(WINDRES) -I$(top_srcdir)/src $< $@

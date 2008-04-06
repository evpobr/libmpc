bin_PROGRAMS = mpcchap

common_sources = ../common/tags.c

INCLUDES = -I$(top_srcdir)/include -I/home/nico/src/cuetools-1.3.1/
METASOURCES = AUTO
mpcchap_SOURCES = dictionary.c iniparser.c mpcchap.c $(common_sources)
mpcchap_LDADD = $(top_builddir)/libmpcdec/libmpcdec.la \
	$(top_builddir)/libmpcenc/libmpcenc.a /home/nico/src/cuetools-1.3.1/src/lib/libcuefile.a -lm

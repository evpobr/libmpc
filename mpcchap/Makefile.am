bin_PROGRAMS = mpcchap

common_sources = ../common/tags.c

INCLUDES = -I$(top_srcdir)/include
METASOURCES = AUTO
mpcchap_SOURCES = dictionary.c iniparser.c mpcchap.c $(common_sources)
mpcchap_LDADD = $(top_builddir)/libmpcdec/libmpcdec.la \
	$(top_builddir)/libmpcenc/libmpcenc.a -lm

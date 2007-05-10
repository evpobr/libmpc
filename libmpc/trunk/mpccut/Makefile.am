INCLUDES = -I$(top_srcdir)/include
METASOURCES = AUTO
bin_PROGRAMS = mpccut
mpccut_SOURCES = mpccut.c
mpccut_LDADD = $(top_builddir)/libmpcenc/libmpcenc.a \
	$(top_builddir)/libmpcdec/libmpcdec.la -lm

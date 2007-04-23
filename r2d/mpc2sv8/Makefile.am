INCLUDES = -I$(top_srcdir)/include
METASOURCES = AUTO
bin_PROGRAMS = mpc2sv8
mpc2sv8_SOURCES = mpc2sv8.c
mpc2sv8_LDADD = -lm \
	$(top_builddir)/libmpcdec/libmpcdec.la \
	$(top_builddir)/libmpcenc/libmpcenc.a

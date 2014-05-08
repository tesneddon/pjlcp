AM_CPPFLAGS = -I$(top_srcdir)/include
noinst_LIBRARIES = libprs.a
libprs_a_SOURCES = prsrea.c prscmd.c
noinst_PROGRAMS = prs geninclude genkeywords
prs_SOURCES = prs.c
geninclude_SOURCES = geninclude.c
geninclude_LDADD = ../libfnv/libfnv.a
genkeywords_SOURCES = genkeywords.c
genkeywords_LDADD = ../libfnv/libfnv.a


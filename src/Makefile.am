CLEANFILES = pjlcp.bin
BUILT_SOURCES = pjlcp.bin
AM_CPPFLAGS = -I$(top_srcdir)/include
bin_PROGRAMS = pjlcp
pjlcp_SOURCES = help.c message.c misc.c net.c pjl.c pjlcp.c
pjlcp_LDADD = ../prs/libprs.a

.cdt.bin :
	../prs/prs $< $@


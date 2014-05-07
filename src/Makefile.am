CLEANFILES = pjlcp.bin action_routines_decl.h action_routines_init.h
BUILT_SOURCES = pjlcp.bin action_routines_decl.h action_routines_init.h
AM_CPPFLAGS = -I$(top_srcdir)/include
bin_PROGRAMS = pjlcp
pjlcp_SOURCES = help.c message.c misc.c net.c pjl.c pjlcp.c set.c system.c
pjlcp_LDADD = ../prs/libprs.a

action_routines_decl.h : pjlcp.cdt
	../prs/geninclude $@ /dev/null $^

action_routines_init.h : pjlcp.cdt
	../prs/geninclude /dev/null $@ $^

.cdt.bin :
	cp $< /tmp/a.c
	$(CC) -E ${INCDIRS} ${CFLAGS} /tmp/a.c > /tmp/a.i
	../prs/prs /tmp/a.i
	cp /tmp/a.bin $@

#	rm /tmp/a.*

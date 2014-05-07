CLEANFILES = pjlcp.bin action_routines_decl.h action_routines_init.h \
             keywords.h keywords.c
BUILT_SOURCES = pjlcp.bin action_routines_decl.h action_routines_init.h \
                keywords.h keywords.c
AM_CPPFLAGS = -I$(top_srcdir)/include
bin_PROGRAMS = pjlcp
pjlcp_SOURCES = help.c message.c misc.c net.c pjl.c pjlcp.c set.c system.c \
                keywords.c
pjlcp_LDADD = ../prs/libprs.a

# Generate action routine definitions.
#
action_routines_decl.h : pjlcp.cdt
	../prs/geninclude $@ /dev/null $^

action_routines_init.h : pjlcp.cdt
	../prs/geninclude /dev/null $@ $^

# Generate keyword definitions.
#
keywords.h : pjlcp.cdt
	./genkeywords KW_ /dev/null $@ $^

keywords.c : pjlcp.cdt
	./genkeywords KW_ $@ /dev/null $^

# Default rule for building command definition table.
#
.cdt.bin :
	cp $< /tmp/a.c
	$(CPP) ${INCDIRS} ${CFLAGS} /tmp/a.c > /tmp/a.i
	../prs/prs /tmp/a.i
	cp /tmp/a.bin $@
	rm /tmp/a.*

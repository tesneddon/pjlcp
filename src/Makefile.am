CLEANFILES = pjlcp.bin action_routines_decl.h action_routines_init.h \
             keywords.h keywords.c response.bin
BUILT_SOURCES = pjlcp.bin action_routines_decl.h action_routines_init.h \
                keywords.h keywords.c response.bin
AM_CPPFLAGS = -I$(top_srcdir)/include
bin_PROGRAMS = pjlcp
pjlcp_SOURCES = fs.c help.c message.c misc.c net.c pjl.c pjlcp.c set.c \
                show.c status.c keywords.c util.c kernel.c display.c
pjlcp_LDADD = ../prs/libprs.a

# Generate action routine definitions.
#
action_routines_decl.h : pjlcp.cdt response.cdt
	@ echo "  GENINC $@"
	@ ../prs/geninclude $@ /dev/null $<

action_routines_init.h : pjlcp.cdt response.cdt
	@ echo "  GENINC $@"
	@ ../prs/geninclude /dev/null $@ $<

# Generate keyword definitions.
#
keywords.h : pjlcp.cdt response.cdt
	@ echo "  GENKWD $@"
	@ ../prs/genkeywords KW_ $@ /dev/null $<

keywords.c : pjlcp.cdt response.cdt
	@ echo "  GENKWD $@"
	@ ../prs/genkeywords KW_ /dev/null $@ $<

# Default rule for building command definition table.
#
pjlcp.bin : pjlcp.cdt keywords.h operators.h
	@ echo "  PRS    $@"
	@ cp $< /tmp/a.c
	@ $(CPP) -DPREPROCESS_CDT ${INCDIRS} ${CFLAGS} /tmp/a.c > /tmp/a.i
	@ ../prs/prs /tmp/a.i
	@ cp /tmp/a.bin $@
	@ rm /tmp/a.*

response.bin : response.cdt keywords.h operators.h
	@ echo "  PRS    $@"
	@ cp $< /tmp/a.c
	@ $(CPP) -DPREPROCESS_CDT ${INCDIRS} ${CFLAGS} /tmp/a.c > /tmp/a.i
	@ ../prs/prs /tmp/a.i
	@ cp /tmp/a.bin $@
	@ rm /tmp/a.*

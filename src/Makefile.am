CLEANFILES = pjlcp.bin action_routines_decl.h action_routines_init.h \
             keywords.h keywords.c response.bin
BUILT_SOURCES = pjlcp.bin action_routines_decl.h action_routines_init.h \
                keywords.h keywords.c response.bin
AM_CPPFLAGS = -I$(top_srcdir)/include
bin_PROGRAMS = pjlcp
pjlcp_SOURCES = fs.c fsutil.c help.c message.c misc.c net.c pjlcp.c set.c \
                show.c status.c keywords.c util.c kernel.c display.c
pjlcp_LDADD = ../prs/libprs.a

# Generate action routine definitions.
#
AM_V_GENINC = $(am__v_GENINC_$(V))
am__v_GENINC_ = $(am__v_GENINC_$(AM_DEFAULT_VERBOSITY))
am__v_GENINC_0 = @echo "  GENINC " $@;
am__v_GENINV_1 =
action_routines_decl.h : pjlcp.cdt response.cdt
	$(AM_V_GENINC) ../prs/geninclude $@ /dev/null $<

action_routines_init.h : pjlcp.cdt response.cdt
	$(AM_V_GENINC)  ../prs/geninclude /dev/null $@ $<

# Generate keyword definitions.
#
AM_V_GENKWD = $(am__v_GENKWD_$(V))
am__v_GENKWD_ = $(am__v_GENKWD_$(AM_DEFAULT_VERBOSITY))
am__v_GENKWD_0 = @echo "  GENKWD " $@;
am__v_GENKWD_1 =
keywords.h : pjlcp.cdt response.cdt
	$(AM_V_GENKWD) ../prs/genkeywords KW_ $@ /dev/null $<

keywords.c : pjlcp.cdt response.cdt
	$(AM_V_GENKWD)  ../prs/genkeywords KW_ /dev/null $@ $<

# Default rule for building command definition table.
#
AM_V_PRS = $(am__v_PRS_$(V))
am__v_PRS_ = $(am__v_PRS_$(AM_DEFAULT_VERBOSITY))
am__v_PRS_0 = @echo "  PRS      " $@;
am__v_PRS_1 = 
pjlcp.bin : pjlcp.cdt keywords.h operators.h
	$(AM_V_PRS)  \
        cp $< /tmp/a.c && \
        $(CPP) -DPREPROCESS_CDT ${INCDIRS} ${CFLAGS} /tmp/a.c > /tmp/a.i && \
        ../prs/prs /tmp/a.i && \
        cp /tmp/a.bin $@ && \
        rm /tmp/a.*

response.bin : response.cdt keywords.h operators.h
	$(AM_V_PRS) \
        cp $< /tmp/a.c && \
        $(CPP) -DPREPROCESS_CDT ${INCDIRS} ${CFLAGS} /tmp/a.c > /tmp/a.i && \
        ../prs/prs /tmp/a.i && \
        cp /tmp/a.bin $@ && \
        rm /tmp/a.*

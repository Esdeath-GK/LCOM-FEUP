PROG=proj

SRCS = proj.c  videocard_essentials.c mouse_essentials.c kbd_essentials.c timer.c proj_essentials.c  rtc.c xpm_management.c cursor.c game.c graphic_management.c

# __LCOM_OPTIMIZED__ disables most of the LCF's overhead (and functionality!)
CPPFLAGS += -pedantic -D __LCOM_OPTIMIZED__

DPADD += ${LIBLCF}
LDADD += -llcf

.include <minix.lcom.mk>

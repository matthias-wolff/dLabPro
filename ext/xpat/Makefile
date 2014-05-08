## External library xpat
## - Makefile
##
## AUTHOR : Thai Open Source Software Center Ltd. (xpat), Matthias Wolff (Makefile)
## PACKAGE: dLabPro/ext
##
## LICENSE: Freeware (see COPYING)

ifeq (${DLABPRO_HOME},)
  DLABPRO_HOME = ../..
endif

PROJNAME=xpat

## Target settings
SEXT    = c
SOURCES = xmlparse \
          xmlrole  \
          xmltok
DEPS    = dlp_base.h    \
          ascii.h       \
          asciitab.h    \
          xpat.h        \
          iasciitab.h   \
          latin1tab.h   \
          nametab.h     \
          utf8tab.h     \
          xmlrole.h     \
          xmltok_impl.h \
          xmltok.h

include $(DLABPRO_HOME)/make/ext.mk

## EOF

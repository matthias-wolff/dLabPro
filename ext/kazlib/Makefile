## External library kazlib
## - Makefile
##
## AUTHOR : Kaz Kylheku (kazlib), Matthias Eichner (Makefile)
## PACKAGE: dLabPro/ext
##
## LICENSE: kazlib is free for any purpose. See source files for details.

ifeq (${DLABPRO_HOME},)
  DLABPRO_HOME = ../..
endif

PROJNAME=kazlib

CFLAGS_FILTEROUT = -Wall -D_DEBUG -D_RELEASE
CFLAGS_RELEASE   = -DNDEBUG

## Target settings
SEXT    = c
SOURCES = list dict hash
DEPS    = kzl_list.h kzl_dict.h kzl_hash.h

include $(DLABPRO_HOME)/make/ext.mk

## EOF

## External library zlib
## - Makefile
##
## AUTHOR : Jean-loup Gailly, Mark Adler (zlib), Matthias Eichner (Makefile)
## PACKAGE: dLabPro/ext
##
## LICENSE: zlib is free for any purpose. See ../../include/zlib.h for details.

ifeq (${DLABPRO_HOME},)
  DLABPRO_HOME = ../..
endif

PROJNAME=zlib

CFLAGS_FILTEROUT += -D_RELEASE -D_DEBUG -Wall -ansi

## Target settings
SEXT    = c
SOURCES = adler32    \
          compress    \
          crc32    \
          deflate    \
          gzio    \
          infback    \
          inffast    \
          inflate    \
          inftrees    \
          trees    \
          uncompr    \
          zutil
DEPS    = zlib.h zconf.h zutil.h crc32.h

include $(DLABPRO_HOME)/make/ext.mk

## EOF

## External library libsvm
## - Makefile
##
## AUTHOR : Chih-Chung Chang, Chih-Jen Lin (libsvm), Matthias Wolff (Makefile)
## PACKAGE: dLabPro/ext
##
## LICENSE: libsvm is free for any purpose. See source files for details.

ifeq (${DLABPRO_HOME},)
  DLABPRO_HOME = ../..
endif

PROJNAME=libsvm

CFLAGS_FILTEROUT += -Wall -ansi

## Target settings
SEXT    = cpp
SOURCES = libsvm
DEPS    = libsvm.h

include $(DLABPRO_HOME)/make/ext.mk

## EOF

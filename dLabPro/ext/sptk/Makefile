## External library sptk
## - Makefile
##
## AUTHOR : Keiichi Tokuda et al. (see file AUTHORS), Guntram Strecha (Makefile)
## PACKAGE: dLabPro/ext
##
## LICENSE: sptk is free for any purpose. See ./README for details.

ifeq (${DLABPRO_HOME},)
  DLABPRO_HOME = ../..
endif

PROJNAME=sptk

INCL += -I.
CFLAGS += -include dlp_type_extern.h -include dlp_defines_extern.h
CFLAGS_FILTEROUT += -D_RELEASE -D_DEBUG -Wall -ansi

## Target settings
SEXT    = c
SOURCES = getmem     \
          movem      \
          fillz      \
          theq       \
          ifftr      \
          _fftr      \
          _ifft      \
          _fft       \
          _nrand     \
          _ivq       \
          _vq        \
          _lbg       \
          _freqt     \
          _gnorm     \
          _ignorm    \
          _levdur    \
          _par2lpc   \
          _acorr     \
          _lpc       \
          _gc2gc     \
          _mlsadf    \
          _mc2b      \
          _b2mc      \
          _mcep
DEPS    = SPTK.h

include $(DLABPRO_HOME)/make/ext.mk

## EOF

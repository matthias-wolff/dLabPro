## dLabPro mathematics library
## - Makefile
##
## AUTHOR : Matthias Wolff
## PACKAGE: dLabPro/base
## 
## Copyright 2013 dLabPro contributors and others (see COPYRIGHT file) 
## - Chair of System Theory and Speech Technology, TU Dresden
## - Chair of Communications Engineering, BTU Cottbus
## 
## This file is part of dLabPro.
## 
## dLabPro is free software: you can redistribute it and/or modify it under the
## terms of the GNU Lesser General Public License as published by the Free
## Software Foundation, either version 3 of the License, or (at your option)
## any later version.
## 
## dLabPro is distributed in the hope that it will be useful, but WITHOUT ANY
## WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
## FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
## details.
## 
## You should have received a copy of the GNU Lesser General Public License
## along with dLabPro. If not, see <http://www.gnu.org/licenses/>.

## Common settings
ifdef (${CGENPATH})
  CGEN = ${CGENPATH}/dcg
else
  CGEN = dcg
endif

vpath %.h ../../include ../../include/automatic
INCL = -I ../../include -I ../../include/automatic

ifneq (${MACHINE},)
  MEXT=.${MACHINE}
else
  MEXT=
endif

## Compiler specific settings 
ifeq (${DLABPRO_USE_MSVC},1)
  ## - MSVC
  CC       = CL
  CFLAGS   = -nologo -Od -Gm -EHsc -RTC1 -Wp64 -ZI -D_DEBUG ${DLABPRO_MSVC_FLAGS_DEBUG}
  CCoO     = -Fo
  AR       = LIB
  ARFLAGS  = -nologo
  ARoO     = -OUT:
  OEXT     = obj
  LEXT     = lib
  TOOLBOX  = MSVC
else
  ifeq (${DLABPRO_USE_MSVC},2)
    ## - MSVC 6.0 - 32-Bit C/C++-Compiler for x86
    CC       = CL
    CFLAGS   = -nologo -Od -Gm -EHsc -RTC1 -ZI -D_DEBUG ${DLABPRO_MSVC_FLAGS_DEBUG}
    CCoO     = -Fo
    AR       = LIB
    ARFLAGS  = -nologo
    ARoO     = -OUT:
    OEXT     = obj
    LEXT     = lib
    TOOLBOX  = MSVC6
  else
    ANSI=$(if $(findstring mingw,$(shell gcc -dumpmachine)),,-ansi)
    ## - GCC
    CC       = gcc
    CFLAGS   = -g -D_DEBUG -Wall $(ANSI) ${DLABPRO_GCC_CFLAGS_DEBUG}
    CCoO     = -o
    AR       = ar
    ARFLAGS  = rvs
    ARoO     =
    OEXT     = o
    LEXT     = a
    TOOLBOX  = GCC
  endif
endif

## Configuration - DEBUG (Default)
LIB_PATH = ../../lib.debug${MEXT}
OBJ_PATH = ../../obj.debug${MEXT}

ifeq ($(MAKECMDGOALS),)
  MAKECMDGOALS = DEBUG
endif

## Configuration - RELEASE
ifeq ($(MAKECMDGOALS),RELEASE)
  LIB_PATH = ../../lib.release${MEXT}
  OBJ_PATH = ../../obj.release${MEXT}
  ifeq (${DLABPRO_USE_MSVC},1)
    CFLAGS  = -nologo -O2 -GL -D_RELEASE -EHsc -W3 -Wp64 -D_CRT_SECURE_NO_WARNINGS ${DLABPRO_MSVC_FLAGS_RELEASE}
    ARFLAGS = -nologo -LTCG
  else
    ifeq (${DLABPRO_USE_MSVC},2)
      ## - MSVC 6.0 - 32-Bit C/C++-Compiler for x86
      CFLAGS  = -nologo -O2 -D_RELEASE -EHsc -W3 -D_CRT_SECURE_NO_WARNINGS ${DLABPRO_MSVC_FLAGS_RELEASE}
    else
      CFLAGS  = -O2 -D_RELEASE -Wall $(ANSI) ${DLABPRO_GCC_CFLAGS_RELEASE}
    endif
  endif
endif

## Configuration - clean_release
ifeq ($(MAKECMDGOALS),clean_release)
  LIB_PATH = ../../lib.release${MEXT}
  OBJ_PATH = ../../obj.release${MEXT}
endif

## Target settings
MANFILE = ../../manual/automatic/dlpmath.html
LIBRARY = $(LIB_PATH)/dlpmath.$(LEXT)
OBJECTS = $(OBJ_PATH)/dlm_arith.$(OEXT)     \
          $(OBJ_PATH)/dlm_chf.$(OEXT)       \
          $(OBJ_PATH)/dlm_cep.$(OEXT)       \
          $(OBJ_PATH)/dlm_crc32.$(OEXT)     \
          $(OBJ_PATH)/dlm_comb.$(OEXT)      \
          $(OBJ_PATH)/dlm_dft.$(OEXT)       \
          $(OBJ_PATH)/dlm_dtw.$(OEXT)       \
          $(OBJ_PATH)/dlm_eigen.$(OEXT)     \
          $(OBJ_PATH)/dlm_filter.$(OEXT)    \
          $(OBJ_PATH)/dlm_gcep.$(OEXT)      \
          $(OBJ_PATH)/dlm_gel.$(OEXT)       \
          $(OBJ_PATH)/dlm_lpc.$(OEXT)       \
          $(OBJ_PATH)/dlm_lsf.$(OEXT)       \
          $(OBJ_PATH)/dlm_lcq.$(OEXT)       \
          $(OBJ_PATH)/dlm_lud.$(OEXT)       \
          $(OBJ_PATH)/dlm_melfilter.$(OEXT) \
          $(OBJ_PATH)/dlm_mlt.$(OEXT)       \
          $(OBJ_PATH)/dlm_noiserdc.$(OEXT)  \
          $(OBJ_PATH)/dlm_ftt.$(OEXT)       \
          $(OBJ_PATH)/dlm_pitch.$(OEXT)     \
          $(OBJ_PATH)/dlm_fba.$(OEXT)       \
          $(OBJ_PATH)/dlm_vad.$(OEXT)       \
          $(OBJ_PATH)/dlm_pow.$(OEXT)       \
          $(OBJ_PATH)/dlm_rand.$(OEXT)      \
          $(OBJ_PATH)/dlm_roots.$(OEXT)     \
          $(OBJ_PATH)/dlm_sig.$(OEXT)       \
          $(OBJ_PATH)/dlm_stat.$(OEXT)      \
          $(OBJ_PATH)/dlm_trig.$(OEXT)      \
          $(OBJ_PATH)/dlm_xft.$(OEXT)       \
          $(OBJ_PATH)/dlm_ldl.$(OEXT)       \
          $(OBJ_PATH)/dlm_fwt.$(OEXT)       \
          $(OBJ_PATH)/dlm_vq.$(OEXT)
DEPS    = dlp_base.h dlp_math.h

## Build rules
# Do not invoke code generation if cgen is not executable
  DEBUG   : ECHOCNF MKDIR $(MANFILE) $(LIBRARY)
  RELEASE : ECHOCNF MKDIR $(MANFILE) $(LIBRARY)

$(MANFILE): dlpmath.def \
            dlm_arith.c \
            dlm_cep.c   \
            dlm_chf.c   \
            dlm_crc32.c \
            dlm_comb.c  \
            dlm_eigen.c \
            dlm_gcep.c  \
            dlm_gel.c   \
            dlm_rand.c  \
            dlm_sig.c   \
            dlm_stat.c  \
            dlm_xft.c   \
            dlm_ldl.c   \
            dlm_fwt.c   \
            dlm_vq.c
	@-$(CGEN) dlpmath.def

$(LIBRARY): $(OBJECTS)
	$(AR) $(ARFLAGS) $(ARoO)$(LIBRARY) $(OBJECTS)

$(OBJ_PATH)/dlm_arith.$(OEXT) : dlm_arith.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/dlm_arith.$(OEXT) dlm_arith.c

$(OBJ_PATH)/dlm_chf.$(OEXT) : dlm_chf.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/dlm_chf.$(OEXT) dlm_chf.c

$(OBJ_PATH)/dlm_cep.$(OEXT) : dlm_cep.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/dlm_cep.$(OEXT) dlm_cep.c

$(OBJ_PATH)/dlm_crc32.$(OEXT) : dlm_crc32.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/dlm_crc32.$(OEXT) dlm_crc32.c

$(OBJ_PATH)/dlm_comb.$(OEXT) : dlm_comb.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/dlm_comb.$(OEXT) dlm_comb.c

$(OBJ_PATH)/dlm_dft.$(OEXT) : dlm_dft.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/dlm_dft.$(OEXT) dlm_dft.c

$(OBJ_PATH)/dlm_eigen.$(OEXT) : dlm_eigen.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/dlm_eigen.$(OEXT) dlm_eigen.c

$(OBJ_PATH)/dlm_filter.$(OEXT) : dlm_filter.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/dlm_filter.$(OEXT) dlm_filter.c

$(OBJ_PATH)/dlm_gcep.$(OEXT) : dlm_gcep.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/dlm_gcep.$(OEXT) dlm_gcep.c

$(OBJ_PATH)/dlm_gel.$(OEXT) : dlm_gel.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/dlm_gel.$(OEXT) dlm_gel.c

$(OBJ_PATH)/dlm_lpc.$(OEXT) : dlm_lpc.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/dlm_lpc.$(OEXT) dlm_lpc.c

$(OBJ_PATH)/dlm_lsf.$(OEXT) : dlm_lsf.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/dlm_lsf.$(OEXT) dlm_lsf.c

$(OBJ_PATH)/dlm_lcq.$(OEXT) : dlm_lcq.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/dlm_lcq.$(OEXT) dlm_lcq.c

$(OBJ_PATH)/dlm_lud.$(OEXT) : dlm_lud.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/dlm_lud.$(OEXT) dlm_lud.c

$(OBJ_PATH)/dlm_melfilter.$(OEXT) : dlm_melfilter.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/dlm_melfilter.$(OEXT) dlm_melfilter.c

$(OBJ_PATH)/dlm_mlt.$(OEXT) : dlm_mlt.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/dlm_mlt.$(OEXT) dlm_mlt.c

$(OBJ_PATH)/dlm_noiserdc.$(OEXT) : dlm_noiserdc.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/dlm_noiserdc.$(OEXT) dlm_noiserdc.c

$(OBJ_PATH)/dlm_ftt.$(OEXT) : dlm_ftt.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/dlm_ftt.$(OEXT) dlm_ftt.c

$(OBJ_PATH)/dlm_dtw.$(OEXT) : dlm_dtw.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/dlm_dtw.$(OEXT) dlm_dtw.c

$(OBJ_PATH)/dlm_pitch.$(OEXT) : dlm_pitch.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/dlm_pitch.$(OEXT) dlm_pitch.c

$(OBJ_PATH)/dlm_fba.$(OEXT) : dlm_fba.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/dlm_fba.$(OEXT) dlm_fba.c

$(OBJ_PATH)/dlm_vad.$(OEXT) : dlm_vad.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/dlm_vad.$(OEXT) dlm_vad.c

$(OBJ_PATH)/dlm_pow.$(OEXT) : dlm_pow.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/dlm_pow.$(OEXT) dlm_pow.c

$(OBJ_PATH)/dlm_rand.$(OEXT) : dlm_rand.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/dlm_rand.$(OEXT) dlm_rand.c

$(OBJ_PATH)/dlm_roots.$(OEXT) : dlm_roots.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/dlm_roots.$(OEXT) dlm_roots.c

$(OBJ_PATH)/dlm_sig.$(OEXT) : dlm_sig.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/dlm_sig.$(OEXT) dlm_sig.c

$(OBJ_PATH)/dlm_stat.$(OEXT) : dlm_stat.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/dlm_stat.$(OEXT) dlm_stat.c

$(OBJ_PATH)/dlm_trig.$(OEXT) : dlm_trig.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/dlm_trig.$(OEXT) dlm_trig.c

$(OBJ_PATH)/dlm_xft.$(OEXT) : dlm_xft.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/dlm_xft.$(OEXT) dlm_xft.c

$(OBJ_PATH)/dlm_ldl.$(OEXT) : dlm_ldl.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/dlm_ldl.$(OEXT) dlm_ldl.c
	
$(OBJ_PATH)/dlm_fwt.$(OEXT) : dlm_fwt.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/dlm_fwt.$(OEXT) dlm_fwt.c

$(OBJ_PATH)/dlm_vq.$(OEXT) : dlm_vq.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/dlm_vq.$(OEXT) dlm_vq.c

## Additional rules
.PHONY: ECHOCNF MKDIR clean clean_debug clean_release

ECHOCNF:
	@echo
	@echo '// ----- Make ($(TOOLBOX)): dLabPro library dlpmath -- $(MAKECMDGOALS) -----'

MKDIR:
	@-test -w $(OBJ_PATH) || mkdir $(OBJ_PATH)
	@-test -w $(LIB_PATH) || mkdir $(LIB_PATH)

clean: clean_debug

clean_debug:
	@echo '// ----- Make ($(TOOLBOX)): dLabPro library dlpmath -- cleaning DEBUG -----'
	-rm -f $(OBJECTS) $(LIBRARY)
	-rm -f vc80.?db
	-touch dlpmath.def

clean_release:
	@echo '// ----- Make ($(TOOLBOX)): dLabPro library dlpmath -- cleaning RELEASE -----'
	-rm -f $(OBJECTS) $(LIBRARY)
	-rm -f vc80.?db
	-touch dlpmath.def

## EOF

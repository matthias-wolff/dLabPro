## External library sptk
## - Makefile
##
## AUTHOR : Keiichi Tokuda et al. (see file AUTHORS), Guntram Strecha (Makefile)
## PACKAGE: dLabPro/ext
##
## LICENSE: sptk is free for any purpose. See ./README for details.

## Common settings
vpath %.h ../../include ./
INCL = -I ../../include -I ./

ifneq (${MACHINE},)
  MEXT=.${MACHINE}
else
  MEXT=
endif

## Compiler specific settings 
ifeq (${DLABPRO_USE_MSVC},1)
  ## - MSVC
  CC       = CL
  CFLAGS   = -nologo -Od -Gm -EHsc -RTC1 -Wp64 -ZI ${DLABPRO_MSVC_FLAGS_DEBUG}
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
    CFLAGS   = -nologo -Od -Gm -EHsc -RTC1  -ZI ${DLABPRO_MSVC_FLAGS_DEBUG}
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
    #CFLAGS   = -g -D_DEBUG -Wall $(ANSI) ${DLABPRO_GCC_CFLAGS_DEBUG}
    CFLAGS   = -g ${DLABPRO_GCC_CFLAGS_DEBUG}
    CCoO     = -o
    AR       = ar
    ARFLAGS  = rvs
    ARoO     =
    OEXT     = o
    LEXT     = a
    TOOLBOX  = GCC
  endif
endif

## Configuration - DEBUG (default)
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
      #CFLAGS  = -O2 -D_RELEASE -Wall $(ANSI) ${DLABPRO_GCC_CFLAGS_RELEASE}
      CFLAGS   = -O2 ${DLABPRO_GCC_CFLAGS_RELEASE}
    endif
  endif
endif

## Configuration - clean_release
ifeq ($(MAKECMDGOALS),clean_release)
  LIB_PATH = ../../lib.release${MEXT}
  OBJ_PATH = ../../obj.release${MEXT}
endif

## Target settings
LIBRARY        = $(LIB_PATH)/sptk.$(LEXT)
SHARED_LIBRARY = zlb.so
OBJECTS        = $(OBJ_PATH)/getmem.$(OEXT)     \
                 $(OBJ_PATH)/movem.$(OEXT)      \
                 $(OBJ_PATH)/fillz.$(OEXT)      \
                 $(OBJ_PATH)/theq.$(OEXT)       \
                 $(OBJ_PATH)/ifftr.$(OEXT)      \
                 $(OBJ_PATH)/_fftr.$(OEXT)      \
                 $(OBJ_PATH)/_ifft.$(OEXT)      \
                 $(OBJ_PATH)/_fft.$(OEXT)       \
                 $(OBJ_PATH)/_nrand.$(OEXT)     \
                 $(OBJ_PATH)/_ivq.$(OEXT)       \
                 $(OBJ_PATH)/_vq.$(OEXT)        \
                 $(OBJ_PATH)/_lbg.$(OEXT)       \
                 $(OBJ_PATH)/_freqt.$(OEXT)     \
                 $(OBJ_PATH)/_gnorm.$(OEXT)     \
                 $(OBJ_PATH)/_ignorm.$(OEXT)    \
                 $(OBJ_PATH)/_levdur.$(OEXT)    \
                 $(OBJ_PATH)/_par2lpc.$(OEXT)   \
                 $(OBJ_PATH)/_acorr.$(OEXT)     \
                 $(OBJ_PATH)/_lpc.$(OEXT)       \
                 $(OBJ_PATH)/_gc2gc.$(OEXT)     \
                 $(OBJ_PATH)/_mlsadf.$(OEXT)    \
                 $(OBJ_PATH)/_mc2b.$(OEXT)      \
                 $(OBJ_PATH)/_b2mc.$(OEXT)      \
                 $(OBJ_PATH)/_mcep.$(OEXT)
                 
DEPS           = SPTK.h

## Build rules
DEBUG   : ECHOCNF MKDIR $(LIBRARY)
RELEASE : ECHOCNF MKDIR $(LIBRARY)
SHARED  : ECHOCNF MKDIR $(SHARED_LIBRARY)

$(LIBRARY): $(OBJECTS)
	$(AR) $(ARFLAGS) $(ARoO)$(LIBRARY) $(OBJECTS)

$(SHARED_LIBRARY): $(OBJECTS)
	$(CC) -shared -Wl,-soname,$(SHARED_LIBRARY).0 $(OBJECTS) \
          $(CCoO)$(LIB_PATH)/$(SHARED_LIBRARY).0.0
	-cd $(LIB_PATH) && ln -sf $(SHARED_LIBRARY).0.0 $(SHARED_LIBRARY).0 \
          && ln -sf $(SHARED_LIBRARY).0 $(SHARED_LIBRARY) 

$(OBJ_PATH)/%.$(OEXT): %.c $(DEPS)
	$(CC) -include dlp_type_extern.h -include dlp_defines_extern.h -c $(CFLAGS) $(INCL) $(CCoO) $@ $<

.PHONY: ECHOCNF MKDIR clean clean_debug clean_release

ECHOCNF:
	@echo
	@echo '// ----- Make ($(TOOLBOX)): External library sptk -- $(MAKECMDGOALS) -----'

MKDIR:
	@-test -w $(OBJ_PATH) || mkdir $(OBJ_PATH)
	@-test -w $(LIB_PATH) || mkdir $(LIB_PATH)

clean:  clean_debug

clean_debug:
	@echo '// ----- Make ($(TOOLBOX)): External library sptk -- cleaning DEBUG -----'
	-rm -f $(OBJECTS) $(LIBRARY)
	-rm -f vc80.?db

clean_release:
	@echo '// ----- Make ($(TOOLBOX)): External library sptk -- cleaning RELEASE -----'
	-rm -f $(OBJECTS) $(LIBRARY)
	-rm -f vc80.?db

## EOF

## External library libsndfile
## - Makefile
##
## AUTHOR : Erik de Castro Lopo (libsndfile), Matthias Wolff (Makefile)
## PACKAGE: dLabPro/ext
##
## LICENSE: libsndfile is under the Gnu Lesser General Public License.

## Common settings
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
    CFLAGS   = -nologo -Od -Gm -EHsc -RTC1 -ZI ${DLABPRO_MSVC_FLAGS_DEBUG}
    CCoO     = -Fo
    AR       = LIB
    ARFLAGS  = -nologo
    ARoO     = -OUT:
    OEXT     = obj
    LEXT     = lib
    TOOLBOX  = MSVC6
  else
    ## - GCC
    CC       = gcc
    #CFLAGS   = -g -D_DEBUG -Wall -ansi ${DLABPRO_GCC_CFLAGS_DEBUG}
    CFLAGS   = -g -D_DEBUG ${DLABPRO_GCC_CFLAGS_DEBUG}
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
      #CFLAGS  = -O2 -D_RELEASE -Wall -ansi ${DLABPRO_GCC_CFLAGS_RELEASE}
      CFLAGS   = -O2 -D_RELEASE ${DLABPRO_GCC_CFLAGS_RELEASE}
    endif
  endif
endif

## Configuration - clean_release
ifeq ($(MAKECMDGOALS),clean_release)
  LIB_PATH = ../../lib.release${MEXT}
  OBJ_PATH = ../../obj.release${MEXT}
endif

## Target settings
LIBRARY = $(LIB_PATH)/libsndfile.$(LEXT)
OBJECTS = $(OBJ_PATH)/aiff.$(OEXT)     \
          $(OBJ_PATH)/au.$(OEXT)       \
          $(OBJ_PATH)/command.$(OEXT)  \
          $(OBJ_PATH)/double64.$(OEXT) \
          $(OBJ_PATH)/file_io.$(OEXT)  \
          $(OBJ_PATH)/float32.$(OEXT)  \
          $(OBJ_PATH)/pcm.$(OEXT)      \
          $(OBJ_PATH)/raw.$(OEXT)      \
          $(OBJ_PATH)/sndfile.$(OEXT)  \
          $(OBJ_PATH)/strings.$(OEXT)  \
          $(OBJ_PATH)/wav.$(OEXT)      \
          $(OBJ_PATH)/common.$(OEXT)   \
          $(OBJ_PATH)/wav_w64.$(OEXT)
          
DEPS    = common.h     \
					config.h     \
					float_cast.h \
					sf_unistd.h  \
					sfconfig.h   \
					sfendian.h   \
					wav_w64.h    \
					../../include/sndfile.h

## Build Rules
DEBUG   : ECHOCNF MKDIR $(MANFILE) $(LIBRARY)
RELEASE : ECHOCNF MKDIR $(MANFILE) $(LIBRARY)

$(LIBRARY): $(OBJECTS)
	$(AR) $(ARFLAGS) $(ARoO)$(LIBRARY) $(OBJECTS)

$(OBJ_PATH)/aiff.$(OEXT): aiff.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/aiff.$(OEXT)  aiff.c

$(OBJ_PATH)/au.$(OEXT): au.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/au.$(OEXT)  au.c

$(OBJ_PATH)/command.$(OEXT): command.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/command.$(OEXT)  command.c

$(OBJ_PATH)/common.$(OEXT): common.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/common.$(OEXT)  common.c

$(OBJ_PATH)/double64.$(OEXT): double64.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/double64.$(OEXT)  double64.c

$(OBJ_PATH)/file_io.$(OEXT): file_io.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/file_io.$(OEXT)  file_io.c

$(OBJ_PATH)/float32.$(OEXT): float32.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/float32.$(OEXT)  float32.c

$(OBJ_PATH)/pcm.$(OEXT): pcm.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/pcm.$(OEXT)  pcm.c

$(OBJ_PATH)/raw.$(OEXT): raw.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/raw.$(OEXT)  raw.c

$(OBJ_PATH)/sndfile.$(OEXT): sndfile.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/sndfile.$(OEXT)  sndfile.c

$(OBJ_PATH)/strings.$(OEXT): strings.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/strings.$(OEXT)  strings.c

$(OBJ_PATH)/wav.$(OEXT): wav.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/wav.$(OEXT)  wav.c

$(OBJ_PATH)/wav_w64.$(OEXT): wav_w64.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/wav_w64.$(OEXT)  wav_w64.c


## Additional rules
.PHONY: ECHOCNF MKDIR clean clean_debug clean_release

ECHOCNF:
	@echo
	@echo '// ----- Make ($(TOOLBOX)): External library libsndfile -- $(MAKECMDGOALS) -----'

MKDIR:
	@-test -w $(OBJ_PATH) || mkdir $(OBJ_PATH)
	@-test -w $(LIB_PATH) || mkdir $(LIB_PATH)

clean:  clean_debug

clean_debug:
	@echo '// ----- Make ($(TOOLBOX)): External library libsndfile -- cleaning DEBUG -----'
	-rm -f vc80.?db
	-rm -f $(OBJECTS) $(LIBRARY)

clean_release:
	@echo '// ----- Make ($(TOOLBOX)): External library libsndfile -- cleaning RELEASE -----'
	-rm -f vc80.?db
	-rm -f $(OBJECTS) $(LIBRARY)

## EOF

## External library zlib
## - Makefile
##
## AUTHOR : Jean-loup Gailly, Mark Adler (zlib), Matthias Eichner (Makefile)
## PACKAGE: dLabPro/ext
##
## LICENSE: zlib is free for any purpose. See ../../include/zlib.h for details.

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
    CFLAGS   = -nologo -Od -Gm -EHsc -RTC1  -ZI ${DLABPRO_MSVC_FLAGS_DEBUG}
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
      #CFLAGS  = -O2 -D_RELEASE -Wall -ansi ${DLABPRO_GCC_CFLAGS_RELEASE}
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
LIBRARY        = $(LIB_PATH)/zlib.$(LEXT)
SHARED_LIBRARY = zlb.so
OBJECTS        = $(OBJ_PATH)/adler32.$(OEXT)    \
                 $(OBJ_PATH)/compress.$(OEXT)    \
                 $(OBJ_PATH)/crc32.$(OEXT)    \
                 $(OBJ_PATH)/deflate.$(OEXT)    \
                 $(OBJ_PATH)/gzio.$(OEXT)    \
                 $(OBJ_PATH)/infback.$(OEXT)    \
                 $(OBJ_PATH)/inffast.$(OEXT)    \
                 $(OBJ_PATH)/inflate.$(OEXT)    \
                 $(OBJ_PATH)/inftrees.$(OEXT)    \
                 $(OBJ_PATH)/trees.$(OEXT)    \
                 $(OBJ_PATH)/uncompr.$(OEXT)    \
                 $(OBJ_PATH)/zutil.$(OEXT)
                 
DEPS           = zlib.h             \
                 zconf.h            \
				 zutil.h            \
				 crc32.h

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

$(OBJ_PATH)/adler32.$(OEXT): adler32.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/adler32.$(OEXT)  adler32.c

$(OBJ_PATH)/compress.$(OEXT): compress.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/compress.$(OEXT)  compress.c

$(OBJ_PATH)/crc32.$(OEXT): crc32.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/crc32.$(OEXT)  crc32.c

$(OBJ_PATH)/deflate.$(OEXT): deflate.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/deflate.$(OEXT)  deflate.c

$(OBJ_PATH)/gzio.$(OEXT): gzio.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/gzio.$(OEXT)  gzio.c

$(OBJ_PATH)/infback.$(OEXT): infback.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/infback.$(OEXT)  infback.c

$(OBJ_PATH)/inffast.$(OEXT): inffast.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/inffast.$(OEXT)  inffast.c

$(OBJ_PATH)/inflate.$(OEXT): inflate.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/inflate.$(OEXT)  inflate.c

$(OBJ_PATH)/inftrees.$(OEXT): inftrees.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/inftrees.$(OEXT)  inftrees.c

$(OBJ_PATH)/trees.$(OEXT): trees.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/trees.$(OEXT)  trees.c

$(OBJ_PATH)/uncompr.$(OEXT): uncompr.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/uncompr.$(OEXT)  uncompr.c

$(OBJ_PATH)/zutil.$(OEXT): zutil.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/zutil.$(OEXT)  zutil.c

## Additional rules
.PHONY: ECHOCNF MKDIR clean clean_debug clean_release

ECHOCNF:
	@echo
	@echo '// ----- Make ($(TOOLBOX)): External library zlib -- $(MAKECMDGOALS) -----'

MKDIR:
	@-test -w $(OBJ_PATH) || mkdir $(OBJ_PATH)
	@-test -w $(LIB_PATH) || mkdir $(LIB_PATH)

clean:  clean_debug

clean_debug:
	@echo '// ----- Make ($(TOOLBOX)): External library zlib -- cleaning DEBUG -----'
	-rm -f $(OBJECTS) $(LIBRARY)
	-rm -f vc80.?db

clean_release:
	@echo '// ----- Make ($(TOOLBOX)): External library zlib -- cleaning RELEASE -----'
	-rm -f $(OBJECTS) $(LIBRARY)
	-rm -f vc80.?db

## EOF

## External library kazlib
## - Makefile
##
## AUTHOR : Kaz Kylheku (kazlib), Matthias Eichner (Makefile)
## PACKAGE: dLabPro/ext
##
## LICENSE: kazlib is free for any purpose. See source files for details.

## Common settings
INCL    = -I ../../include
vpath %.h ../../include

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
    CFLAGS   = -ansi -g ${DLABPRO_GCC_CFLAGS_DEBUG}
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
LIB_PATH = ../../lib.debug.${MACHINE}
OBJ_PATH = ../../obj.debug.${MACHINE}

ifeq ($(MAKECMDGOALS),)
  MAKECMDGOALS = DEBUG
endif

## Configuration - RELEASE
ifeq ($(MAKECMDGOALS),RELEASE)
  LIB_PATH = ../../lib.release.${MACHINE}
  OBJ_PATH = ../../obj.release.${MACHINE}
  ifeq (${DLABPRO_USE_MSVC},1)
    CFLAGS  = -nologo -O2 -GL -DNDEBUG -EHsc -W3 -Wp64 -D_CRT_SECURE_NO_WARNINGS ${DLABPRO_MSVC_FLAGS_RELEASE}
    ARFLAGS = -nologo -LTCG
  else
    ifeq (${DLABPRO_USE_MSVC},2)
      ## - MSVC 6.0 - 32-Bit C/C++-Compiler for x86
      CFLAGS  = -nologo -O2 -DNDEBUG -EHsc -W3 -D_CRT_SECURE_NO_WARNINGS ${DLABPRO_MSVC_FLAGS_RELEASE}
    else
      CFLAGS  = -O2 -DNDEBUG ${DLABPRO_GCC_CFLAGS_RELEASE}
    endif
  endif
endif

## Configuration - clean_release
ifeq ($(MAKECMDGOALS),clean_release)
  LIB_PATH = ../../lib.release.${MACHINE}
  OBJ_PATH = ../../obj.release.${MACHINE}
endif

## Target settings
LIBRARY = $(LIB_PATH)/kazlib.$(LEXT)
OBJECTS = $(OBJ_PATH)/list.$(OEXT) \
          $(OBJ_PATH)/dict.$(OEXT) \
          $(OBJ_PATH)/hash.$(OEXT)
DEPS    = kzl_list.h         \
          kzl_dict.h         \
          kzl_hash.h

## Build rules
DEBUG   : ECHOCNF MKDIR $(LIBRARY)
RELEASE : ECHOCNF MKDIR $(LIBRARY)

$(LIBRARY): $(OBJECTS)
	$(AR) $(ARFLAGS) $(ARoO)$(LIBRARY) $(OBJECTS)

$(OBJ_PATH)/list.$(OEXT): list.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/list.$(OEXT)  list.c

$(OBJ_PATH)/dict.$(OEXT): dict.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/dict.$(OEXT)  dict.c

$(OBJ_PATH)/hash.$(OEXT): hash.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/hash.$(OEXT)  hash.c


## Additional rules
.PHONY: ECHOCNF MKDIR clean clean_debug clean_release

ECHOCNF:
	@echo
	@echo '// ----- Make ($(TOOLBOX)): External library kazlib -- $(MAKECMDGOALS) -----'

MKDIR:
	@-test -w $(OBJ_PATH) || mkdir $(OBJ_PATH)
	@-test -w $(LIB_PATH) || mkdir $(LIB_PATH)

clean: clean_debug

clean_debug:
	@echo '// ----- Make ($(TOOLBOX)): External library kazlib -- cleaning DEBUG -----'
	-rm -f vc80.?db
	-rm -f $(OBJECTS) $(LIBRARY)

clean_release:
	@echo '// ----- Make ($(TOOLBOX)): External library kazlib -- cleaning RELEASE -----'
	-rm -f vc80.?db
	-rm -f $(OBJECTS) $(LIBRARY)

## EOF
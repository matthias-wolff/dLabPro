## dLabPro makefiles
## - Ext make include file
##
## AUTHOR : Frank Duckhorn
## PACKAGE: dLabPro/make

## Common settings
vpath %.h ../../include ../../include/automatic
INCL += -I ../../include -I ../../include/automatic

ifneq (${MACHINE},)
  MEXT=.${MACHINE}
else
  MEXT=
endif

## Compiler specific settings 
ifeq (${DLABPRO_USE_MSVC},1)
  ## - MSVC
  CC       = CL
  CFLAGS  += -nologo -Od -Gm -EHsc -RTC1 -Wp64 -ZI $(CFLAGS_MSV) ${DLABPRO_MSVC_FLAGS_DEBUG}
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
    CFLAGS  += -nologo -Od -Gm -EHsc -RTC1  -ZI $(CFLAGS_MSV) ${DLABPRO_MSVC_FLAGS_DEBUG}
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
    CFLAGS  += -g -D_DEBUG -Wall $(ANSI) ${DLABPRO_GCC_CFLAGS_DEBUG}
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
  CFLAGS += $(CFLAGS_RELEASE)
  ifeq (${DLABPRO_USE_MSVC},1)
    CFLAGS += -nologo -O2 -GL -D_RELEASE -EHsc -W3 -Wp64 -D_CRT_SECURE_NO_WARNINGS ${DLABPRO_MSVC_FLAGS_RELEASE}
    ARFLAGS = -nologo -LTCG
  else
    ifeq (${DLABPRO_USE_MSVC},2)
      ## - MSVC 6.0 - 32-Bit C/C++-Compiler for x86
      CFLAGS += -nologo -O2 -D_RELEASE -EHsc -W3 -D_CRT_SECURE_NO_WARNINGS ${DLABPRO_MSVC_FLAGS_RELEASE}
    else
      CFLAGS += -O2 -D_RELEASE -Wall $(ANSI) ${DLABPRO_GCC_CFLAGS_RELEASE}
    endif
  endif
endif

## Filter-out Variables
CFLAGS := $(filter-out $(CFLAGS_FILTEROUT),$(CFLAGS))

## Configuration - clean_release
ifeq ($(MAKECMDGOALS),clean_release)
  LIB_PATH = ../../lib.release${MEXT}
  OBJ_PATH = ../../obj.release${MEXT}
endif

## Target settings
LIBRARY = $(LIB_PATH)/$(PROJNAME).$(LEXT)
SHARED_LIBRARY = $(LIB_PATH)/$(PROJNAME).so
SRCFILES= $(addsuffix .$(SEXT),$(SOURCES))
OBJECTS = $(addprefix $(OBJ_PATH)/,$(addsuffix .$(OEXT),$(SOURCES)))

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

$(OBJ_PATH)/%.$(OEXT): %.$(SEXT) $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$@ $<

.PHONY: ECHOCNF MKDIR clean clean_debug clean_release

ECHOCNF:
	@echo
	@echo '// ----- Make ($(TOOLBOX)): External library $(PROJNAME) -- $(MAKECMDGOALS) -----'

MKDIR:
	@-test -w $(OBJ_PATH) || mkdir $(OBJ_PATH)
	@-test -w $(LIB_PATH) || mkdir $(LIB_PATH)

clean:  clean_debug

clean_debug:
	@echo '// ----- Make ($(TOOLBOX)): External library $(PROJNAME) -- cleaning DEBUG -----'
	-rm -f $(OBJECTS) $(LIBRARY) $(SHARED_LIBRARY)
	-rm -f vc80.?db

clean_release:
	@echo '// ----- Make ($(TOOLBOX)): External library $(PROJNAME) -- cleaning RELEASE -----'
	-rm -f $(OBJECTS) $(LIBRARY) $(SHARED_LIBRARY)
	-rm -f vc80.?db

## EOF

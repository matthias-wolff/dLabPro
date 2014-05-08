## dLabPro makefiles
## - Base CPP make include file
##
## AUTHOR : Frank Duckhorn
## PACKAGE: dLabPro/make
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
  CFLAGS  += -nologo -Od -Gm -EHsc -RTC1 -Wp64 -ZI -D_DEBUG -D_DLP_CPP -TP ${DLABPRO_MSVC_FLAGS_DEBUG}
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
    CFLAGS  += -nologo -Od -Gm -EHsc -RTC1 -ZI -D_DEBUG -D_DLP_CPP -TP ${DLABPRO_MSVC_FLAGS_DEBUG}
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
    CFLAGS  += -g -D_DEBUG -Wall $(ANSI) -x c++ -D_DLP_CPP ${DLABPRO_GCC_CFLAGS_DEBUG}
    CCoO     = -o
    AR       = ar
    ARFLAGS  = rvs
    ARoO     =
    OEXT     = o
    LEXT     = a
    TOOLBOX  = GCC
  endif
endif

## Configuration - DEBUG_CPP (Default)
LIB_PATH = ../../lib.debug${MEXT}
OBJ_PATH = ../../obj.debug${MEXT}

ifeq ($(MAKECMDGOALS),)
  MAKECMDGOALS = DEBUG_CPP
endif

## Configuration - DEBUG_C
ifeq ($(MAKECMDGOALS),DEBUG_C)
  LIB_PATH = ../../lib.debug${MEXT}
  OBJ_PATH = ../../obj.debug${MEXT}
  ifeq (${DLABPRO_USE_MSVC},1)
    CFLAGS+= -nologo -Od -Gm -EHsc -RTC1 -Wp64 -ZI -TC -D_DEBUG -D_DLP_C -D_DLP_C ${DLABPRO_MSVC_FLAGS_DEBUG}
  else
    ifeq (${DLABPRO_USE_MSVC},2)
      ## - MSVC 6.0 - 32-Bit C/C++-Compiler for x86
      CFLAGS+= -nologo -Od -Gm -EHsc -RTC1 -ZI -TC -D_DEBUG -D_DLP_C -D_DLP_C ${DLABPRO_MSVC_FLAGS_DEBUG}
    else
      CFLAGS+= -g -D_DEBUG -Wall $(ANSI) -x c -D_DLP_C ${DLABPRO_GCC_CFLAGS_DEBUG}
    endif
  endif
endif

## Configuration - RELEASE_CPP
ifeq ($(MAKECMDGOALS),RELEASE_CPP)
  LIB_PATH = ../../lib.release${MEXT}
  OBJ_PATH = ../../obj.release${MEXT}
  ifeq (${DLABPRO_USE_MSVC},1)
    CFLAGS += -nologo -O2 -GL -D_RELEASE -EHsc -W3 -Wp64 -TP -D_CRT_SECURE_NO_WARNINGS -D_DLP_CPP ${DLABPRO_MSVC_FLAGS_RELEASE}
    ARFLAGS = -nologo -LTCG
  else
    ifeq (${DLABPRO_USE_MSVC},2)
      ## - MSVC 6.0 - 32-Bit C/C++-Compiler for x86
      CFLAGS += -nologo -O2 -D_RELEASE -EHsc -W3 -TP -D_CRT_SECURE_NO_WARNINGS -D_DLP_CPP ${DLABPRO_MSVC_FLAGS_RELEASE}
    else
      CFLAGS += -O2 -D_RELEASE -Wall $(ANSI) -x c++ -D_DLP_CPP ${DLABPRO_GCC_CFLAGS_RELEASE}
    endif
  endif
endif

## Configuration - RELEASE_C
ifeq ($(MAKECMDGOALS),RELEASE_C)
  LIB_PATH = ../../lib.release${MEXT}
  OBJ_PATH = ../../obj.release${MEXT}
  ifeq (${DLABPRO_USE_MSVC},1)
    CFLAGS += -nologo -O2 -GL -D_RELEASE -EHsc -W3 -Wp64 -TC -D_CRT_SECURE_NO_WARNINGS -D_DLP_C ${DLABPRO_MSVC_FLAGS_RELEASE}
    ARFLAGS = -nologo -LTCG
  else
    ifeq (${DLABPRO_USE_MSVC},2)
      ## - MSVC 6.0 - 32-Bit C/C++-Compiler for x86
      CFLAGS += -nologo -O2 -D_RELEASE -EHsc -W3 -TC -D_CRT_SECURE_NO_WARNINGS -D_DLP_C ${DLABPRO_MSVC_FLAGS_RELEASE}
    else
      CFLAGS += -O2 -D_RELEASE -Wall $(ANSI) -x c -D_DLP_C ${DLABPRO_GCC_CFLAGS_RELEASE}
    endif
  endif
endif

## Configuration - clean_release
ifeq ($(MAKECMDGOALS),clean_release)
  LIB_PATH = ../../lib.release${MEXT}
  OBJ_PATH = ../../obj.release${MEXT}
endif

## Target settings
MANFILE = ../../manual/automatic/$(PROJNAME).html
LIBRARY = $(LIB_PATH)/$(PROJNAME).$(LEXT)
DEFFILE = $(PROJNAME).def
SRCFILES= $(addsuffix .$(SEXT),$(SOURCES))
OBJECTS = $(addprefix $(OBJ_PATH)/,$(addsuffix .$(OEXT),$(SOURCES)))

## Build rules
DEBUG_CPP  : ECHOCNF MKDIR $(MANFILE) $(LIBRARY)
DEBUG_C    : ECHOCNF MKDIR $(MANFILE) $(LIBRARY)
RELEASE_CPP: ECHOCNF MKDIR $(MANFILE) $(LIBRARY)
RELEASE_C  : ECHOCNF MKDIR $(MANFILE) $(LIBRARY)

$(MANFILE): $(DEFFILE) $(SRCFILES)
	@-$(CGEN) $(DEFFILE)

$(LIBRARY): $(OBJECTS)
	$(AR) $(ARFLAGS) $(ARoO)$(LIBRARY) $(OBJECTS)

$(OBJ_PATH)/%.$(OEXT) : %.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$@ $<

## Additional rules
.PHONY: ECHOCNF MKDIR clean clean_debug clean_release

ECHOCNF:
	@echo
	@echo '// ----- Make ($(TOOLBOX)): dLabPro library $(PROJNAME) -- $(MAKECMDGOALS) -----'

MKDIR:
	@-test -w $(OBJ_PATH) || mkdir $(OBJ_PATH)
	@-test -w $(LIB_PATH) || mkdir $(LIB_PATH)

clean:  clean_debug

clean_debug:
	@echo '// ----- Make ($(TOOLBOX)): dLabPro library $(PROJNAME) -- cleaning DEBUG -----'
	-rm -f $(OBJECTS) $(LIBRARY)
	-rm -f vc80.?db
	-touch $(DEFFILE)

clean_release:
	@echo '// ----- Make ($(TOOLBOX)): dLabPro library $(PROJNAME) -- cleaning RELEASE -----'
	-rm -f $(OBJECTS) $(LIBRARY)
	-rm -f vc80.?db
	-touch $(DEFFILE)

## EOF

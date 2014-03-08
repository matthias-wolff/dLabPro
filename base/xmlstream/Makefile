## dLabPro XML stream library
## - Makefile
##
## AUTHOR : Matthias Eichner
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
    ## - GCC
    CC       = gcc
    CFLAGS   = -g -D_DEBUG -Wall -ansi ${DLABPRO_GCC_CFLAGS_DEBUG}
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
    CFLAGS  = -nologo -O2 -GL -D_RELEASE -EHsc -W3 -Wp64 -D_CRT_SECURE_NO_WARNINGS ${DLABPRO_MSVC_FLAGS_RELEASE}
    ARFLAGS = -nologo -LTCG
  else
    ifeq (${DLABPRO_USE_MSVC},2)
      ## - MSVC 6.0 - 32-Bit C/C++-Compiler for x86
      CFLAGS  = -nologo -O2 -D_RELEASE -EHsc -W3 -D_CRT_SECURE_NO_WARNINGS ${DLABPRO_MSVC_FLAGS_RELEASE}
    else
      CFLAGS  = -O2 -D_RELEASE -Wall -ansi ${DLABPRO_GCC_CFLAGS_RELEASE}
    endif
  endif
endif

## Configuration - clean_release
ifeq ($(MAKECMDGOALS),clean_release)
  LIB_PATH = ../../lib.release.${MACHINE}
  OBJ_PATH = ../../obj.release.${MACHINE}
endif

## Target settings
MANFILE = ../../manual/automatic/xmlstream.html
LIBRARY = $(LIB_PATH)/xmlstream.$(LEXT)
OBJECTS = $(OBJ_PATH)/xmlstream.$(OEXT)
DEPS    = dlp_base.h      \
          dlp_table.h     \
          dlp_xmlstream.h \
          xpat.h

## Build rules
DEBUG   : ECHOCNF MKDIR $(MANFILE) $(LIBRARY)
RELEASE : ECHOCNF MKDIR $(MANFILE) $(LIBRARY)

$(MANFILE): xmlstream.def xmlstream.c
	@-$(CGEN) xmlstream.def

$(LIBRARY): $(OBJECTS)
	$(AR) $(ARFLAGS) $(ARoO)$(LIBRARY) $(OBJECTS)

$(OBJ_PATH)/xmlstream.$(OEXT) : xmlstream.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/xmlstream.$(OEXT) xmlstream.c


## Additional rules
.PHONY: ECHOCNF MKDIR clean clean_debug clean_release

ECHOCNF:
	@echo
	@echo '// ----- Make ($(TOOLBOX)): dLabPro library xmlstream -- $(MAKECMDGOALS) -----'

MKDIR:
	@-test -w $(OBJ_PATH) || mkdir $(OBJ_PATH)
	@-test -w $(LIB_PATH) || mkdir $(LIB_PATH)

clean:  clean_debug

clean_debug:
	@echo '// ----- Make ($(TOOLBOX)): dLabPro library xmlstream -- cleaning DEBUG -----'
	-rm -f $(OBJECTS) $(LIBRARY)
	-rm -f vc80.?db
	-touch xmlstream.def

clean_release:
	@echo '// ----- Make ($(TOOLBOX)): dLabPro library xmlstream -- cleaning RELEASE -----'
	-rm -f $(OBJECTS) $(LIBRARY)
	-rm -f vc80.?db
	-touch xmlstream.def

## EOF

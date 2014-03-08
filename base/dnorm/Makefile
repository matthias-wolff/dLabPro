## dLabPro DNorm3 library
## - Makefile
##
## AUTHOR : Torsten Rudolph, Matthias Eichner
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
      #CFLAGS  = -O2 -D_RELEASE -Wall -ansi ${DLABPRO_GCC_CFLAGS_RELEASE}
      CFLAGS  = -O2 -D_RELEASE ${DLABPRO_GCC_CFLAGS_RELEASE}
    endif
  endif
endif

## Configuration - clean_release
ifeq ($(MAKECMDGOALS),clean_release)
  LIB_PATH = ../../lib.release.${MACHINE}
  OBJ_PATH = ../../obj.release.${MACHINE}
endif

## Target settings
MANFILE = ../../manual/automatic/dnorm.html
LIBRARY = $(LIB_PATH)/dnorm.$(LEXT)
OBJECTS = $(OBJ_PATH)/dopen.$(OEXT)  \
          $(OBJ_PATH)/dget.$(OEXT)   \
          $(OBJ_PATH)/dread.$(OEXT)  \
          $(OBJ_PATH)/krtree.$(OEXT) \
          $(OBJ_PATH)/derror.$(OEXT) \
          $(OBJ_PATH)/dsvl.$(OEXT)
DEPS    = dlp_dnorm.h dlp_dnormint.h

## Build Rules
DEBUG   : ECHOCNF MKDIR $(MANFILE) $(LIBRARY)
RELEASE : ECHOCNF MKDIR $(MANFILE) $(LIBRARY)

$(MANFILE): dnorm.def  \
            derror.c   \
            dget.c     \
            dopen.c    \
            dread.c    \
            dsvl.c     \
            krtree.c
	@-$(CGEN) dnorm.def

$(LIBRARY): $(OBJECTS)
	$(AR) $(ARFLAGS) $(ARoO)$(LIBRARY) $(OBJECTS)

$(OBJ_PATH)/dopen.$(OEXT): dopen.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/dopen.$(OEXT)  dopen.c

$(OBJ_PATH)/dget.$(OEXT): dget.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/dget.$(OEXT)  dget.c

$(OBJ_PATH)/dread.$(OEXT): dread.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/dread.$(OEXT)  dread.c

$(OBJ_PATH)/krtree.$(OEXT): krtree.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/krtree.$(OEXT)  krtree.c

$(OBJ_PATH)/derror.$(OEXT): derror.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/derror.$(OEXT)  derror.c

$(OBJ_PATH)/dsvl.$(OEXT): dsvl.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/dsvl.$(OEXT)  dsvl.c

## Additional rules
.PHONY: ECHOCNF MKDIR clean clean_debug clean_release

ECHOCNF:
	@echo
	@echo '// ----- Make ($(TOOLBOX)): dLabPro library dnorm -- $(MAKECMDGOALS) -----'

MKDIR:
	@-test -w $(OBJ_PATH) || mkdir $(OBJ_PATH)
	@-test -w $(LIB_PATH) || mkdir $(LIB_PATH)

clean:  clean_debug

clean_debug:
	@echo '// ----- Make ($(TOOLBOX)): dLabPro library dnorm -- cleaning DEBUG -----'
	-rm -f $(OBJECTS) $(LIBRARY)
	-rm -f vc80.?db
	-touch dnorm.def

clean_release:
	@echo '// ----- Make ($(TOOLBOX)): dLabPro library dnorm -- cleaning RELEASE -----'
	-rm -f $(OBJECTS) $(LIBRARY)
	-rm -f vc80.?db
	-touch dnorm.def

## EOF
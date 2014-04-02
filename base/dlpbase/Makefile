## dLabPro base library
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
      CFLAGS  = -O2 -D_RELEASE -Wall -ansi ${DLABPRO_GCC_CFLAGS_RELEASE}
    endif
  endif
endif

## Configuration - clean_release
ifeq ($(MAKECMDGOALS),clean_release)
  LIB_PATH = ../../lib.release${MEXT}
  OBJ_PATH = ../../obj.release${MEXT}
endif

## Target settings
MANFILE = ../../manual/automatic/dlpbase.html
LIBRARY = $(LIB_PATH)/dlpbase.$(LEXT)
OBJECTS = $(OBJ_PATH)/dlp_arith.$(OEXT)   \
          $(OBJ_PATH)/dlp_assert.$(OEXT)  \
          $(OBJ_PATH)/dlp_heap.$(OEXT)    \
          $(OBJ_PATH)/dlp_io.$(OEXT)      \
          $(OBJ_PATH)/dlp_ncnv.$(OEXT)    \
          $(OBJ_PATH)/dlp_optab.$(OEXT)   \
          $(OBJ_PATH)/dlp_session.$(OEXT) \
          $(OBJ_PATH)/dlp_sort.$(OEXT)    \
          $(OBJ_PATH)/dlp_string.$(OEXT)  \
          $(OBJ_PATH)/dlp_thread.$(OEXT)  \
          $(OBJ_PATH)/dlp_type.$(OEXT)    \
          $(OBJ_PATH)/dlp_file.$(OEXT)
DEPS    = dlp_base.h kzl_hash.h

## Build rules
DEBUG   : ECHOCNF MKDIR $(MANFILE) $(LIBRARY)
RELEASE : ECHOCNF MKDIR $(MANFILE) $(LIBRARY)

$(MANFILE): dlpbase.def   \
            dlp_arith.c   \
            dlp_assert.c  \
            dlp_heap.c    \
            dlp_io.c      \
            dlp_ncnv.c    \
            dlp_optab.c   \
            dlp_session.c \
            dlp_sort.c    \
            dlp_string.c  \
            dlp_thread.c  \
            dlp_type.c    \
            dlp_file.c
	@-$(CGEN) dlpbase.def

$(LIBRARY): $(OBJECTS)
	$(AR) $(ARFLAGS) $(ARoO)$(LIBRARY) $(OBJECTS)

$(OBJ_PATH)/dlp_arith.$(OEXT) : dlp_arith.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/dlp_arith.$(OEXT) dlp_arith.c

$(OBJ_PATH)/dlp_assert.$(OEXT) : dlp_assert.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/dlp_assert.$(OEXT) dlp_assert.c

$(OBJ_PATH)/dlp_io.$(OEXT) : dlp_io.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/dlp_io.$(OEXT) dlp_io.c

$(OBJ_PATH)/dlp_heap.$(OEXT) : dlp_heap.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/dlp_heap.$(OEXT) dlp_heap.c

$(OBJ_PATH)/dlp_ncnv.$(OEXT) : dlp_ncnv.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/dlp_ncnv.$(OEXT) dlp_ncnv.c

$(OBJ_PATH)/dlp_optab.$(OEXT) : dlp_optab.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/dlp_optab.$(OEXT) dlp_optab.c

$(OBJ_PATH)/dlp_session.$(OEXT) : dlp_session.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/dlp_session.$(OEXT) dlp_session.c

$(OBJ_PATH)/dlp_sort.$(OEXT) : dlp_sort.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/dlp_sort.$(OEXT) dlp_sort.c

$(OBJ_PATH)/dlp_string.$(OEXT) : dlp_string.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/dlp_string.$(OEXT) dlp_string.c

$(OBJ_PATH)/dlp_thread.$(OEXT) : dlp_thread.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/dlp_thread.$(OEXT) dlp_thread.c

$(OBJ_PATH)/dlp_type.$(OEXT) : dlp_type.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/dlp_type.$(OEXT) dlp_type.c

$(OBJ_PATH)/dlp_file.$(OEXT) : dlp_file.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/dlp_file.$(OEXT) dlp_file.c

## Additional rules
.PHONY: ECHOCNF MKDIR clean clean_debug clean_release

ECHOCNF:
	@echo
	@echo '// ----- Make ($(TOOLBOX)): dLabPro library dlpbase -- $(MAKECMDGOALS) -----'

MKDIR:
	@-test -w $(OBJ_PATH) || mkdir $(OBJ_PATH)
	@-test -w $(LIB_PATH) || mkdir $(LIB_PATH)

clean: clean_debug

clean_debug:
	@echo '// ----- Make ($(TOOLBOX)): dLabPro library dlpbase -- cleaning DEBUG -----'
	-rm -f $(OBJECTS) $(LIBRARY)
	-rm -f vc80.?db
	-touch dlpbase.def

clean_release:
	@echo '// ----- Make ($(TOOLBOX)): dLabPro library dlpbase -- cleaning RELEASE -----'
	-rm -f $(OBJECTS) $(LIBRARY)
	-rm -f vc80.?db
	-touch dlpbase.def

## EOF

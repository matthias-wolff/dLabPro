## dLabPro makefiles
## - Compiler settings for programs
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

TARGET_DEF = DEBUG

ANSI  =$(if $(findstring mingw,$(OS)),,-ansi)
STATIC=$(if $(findstring mingw,$(OS)),-static,)

## Common settings
ifeq (${DLABPRO_USE_MSVC},1)
  CC      = CL
  CCoO    = -Fo
  AR      = LIB
  LL      = LINK
  LLoO    = -OUT:
  OEXT    = obj
  LEXT    = lib
  EEXT    = .exe
  TOOLBOX = MSVC
else 
  ifeq (${DLABPRO_USE_MSVC},2)
    ## - MSVC 6.0 - 32-Bit C/C++-Compiler for x86
    CC      = CL
    CCoO    = -Fo
    AR      = LIB
    LL      = LINK
    LLoO    = -OUT:
    OEXT    = obj
    LEXT    = lib
    EEXT    = .exe
    TOOLBOX = MSVC6
  else
    CC      = gcc
    CCoO    = -o
    AR      = ar
    LL      = g++
    LLoO    = -o 
    OEXT    = o
    LEXT    = a
    DEXT    = d
    EEXT    = 
    TOOLBOX = GCC
    ifeq ($(SEXT),c)
      LL   = gcc
    endif
  endif
endif
ifneq ($(or $(findstring mingw,$(OS)),$(findstring msv,$(OS))),)
  EEXT = .exe
endif

## Special settings
ifeq (${DLABPRO_USE_MSVC},1)
  CFLAGS_DEBUG   = -nologo -Od -Gm -EHsc -RTC1 -Wp64 -ZI -D_DEBUG -D_DLP_CPP ${DLABPRO_MSVC_FLAGS_DEBUG}
  CFLAGS_RELEASE = -nologo -O2 -GL -D_RELEASE -D_DLP_CPP -EHsc -W3 -Wp64 -D_CRT_SECURE_NO_WARNINGS ${DLABPRO_MSVC_FLAGS_RELEASE}
  LFLAGS_DEBUG   = -NOLOGO -INCREMENTAL -DEBUG -NODEFAULTLIB:libcmt libcmtd.lib
  LFLAGS_RELEASE = -NOLOGO -LTCG
else
ifeq (${DLABPRO_USE_MSVC},2)
  CFLAGS_DEBUG   = -nologo -Od -Gm -EHsc -RTC1 -ZI -D_DEBUG -D_DLP_CPP ${DLABPRO_MSVC_FLAGS_DEBUG}
  CFLAGS_RELEASE = -nologo -O2 -D_RELEASE -D_DLP_CPP -EHsc -W3 -D_CRT_SECURE_NO_WARNINGS ${DLABPRO_MSVC_FLAGS_RELEASE}
  LFLAGS_DEBUG   = -NOLOGO -INCREMENTAL -DEBUG -NODEFAULTLIB:libcmt libcmtd.lib
  LFLAGS_RELEASE = -NOLOGO -LTCG
else
  CFLAGS_DEBUG   = -g -D_DEBUG -D_DLP_CPP $(ANSI) ${DLABPRO_GCC_CFLAGS_DEBUG}
  CFLAGS_RELEASE = -O2 -D_RELEASE -D_DLP_CPP -Wall $(ANSI) ${DLABPRO_GCC_CFLAGS_RELEASE}
  LFLAGS_DEBUG   = $(STATIC) -lm ${DLABPRO_GCC_LFLAGS_DEBUG}
  LFLAGS_RELEASE = $(STATIC) -lm ${DLABPRO_GCC_LFLAGS_RELEASE}
endif
endif
LFLAGS   = $(LFLAGS_$(TRG_LIB))
CFLAGS   = $(CFLAGS_$(TRG_LIB))

## EOF

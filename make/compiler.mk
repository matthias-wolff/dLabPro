## dLabPro makefiles
## - Compiler settings
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

ifeq ($(OS),msv1)
  TOOLBOX  = MSVC
endif
ifeq ($(OS),msv2)
  TOOLBOX  = MSVC6
endif

## Compiler config for MSV
ifneq ($(findstring msv,$(OS)),)
  CC       = CL
  CCoO     = -Fo
  AR       = LIB
  ARoO     = -OUT:
  LL       = LINK
  LLoO     = -OUT:
  OEXT     = obj
  LEXT     = lib
endif

## Compiler config for GCC
ifneq ($(or $(findstring mingw,$(OS)),$(findstring lin,$(OS))),)
  TOOLBOX  = GCC
  CC       = gcc
  CCoO     = -o
  AR       = ar
  ARoO     =
  LL       = g++
  LLoO     = -o 
  OEXT     = o
  LEXT     = a
  DEXT     = d
  ifeq ($(SEXT),c)
    LL    = gcc
  endif
endif

ifneq ($(or $(findstring mingw,$(OS)),$(findstring msv,$(OS))),)
  EEXT = .exe
endif

## Compiler flags for MSV
ifneq ($(findstring msv,$(OS)),)
  CFLAGS  += -nologo -EHsc $(CFLAGS_MSV)
  ARFLAGS  = -nologo
  LFLAGS  += -NOLOGO
  ifeq ($(TRG_LIB),RELEASE)
    CFLAGS += -O2 -D_RELEASE -W3 -D_CRT_SECURE_NO_WARNINGS ${DLABPRO_MSVC_FLAGS_RELEASE}
    CFLAGS += -D_DLP_CPP
    ARFLAGS += -LTCG
    LFLAGS += -LTCG
  else
    CFLAGS += -Od -Gm -RTC1 -ZI ${DLABPRO_MSVC_FLAGS_DEBUG}
    CFLAGS += -D_DEBUG
    LFLAGS += -INCREMENTAL -DEBUG -NODEFAULTLIB:libcmt libcmtd.lib
  endif
  ifeq ($(TRG_TYPE),C)
    CFLAGS += -TC -D_DLP_C
  else ifeq ($(TRG_TYPE),CPP)
    CFLAGS  += -TP -D_DLP_CPP
  endif
endif

ifeq ($(OS),msv1)
  CFLAGS  += -Wp64
  ifeq ($(TRG_LIB),RELEASE)
    CFLAGS += -GL
  endif
endif

## Compiler flags for GCC
ifneq ($(or $(findstring mingw,$(OS)),$(findstring lin,$(OS))),)
  CFLAGS  += -Wall $(CFLAGS_GCC)
  ARFLAGS  = rvs
  LFLAGS  += -lm ${DLABPRO_GCC_LFLAGS_DEBUG}
  ifeq ($(TRG_LIB),RELEASE)
    CFLAGS += -O2 -D_RELEASE $(CFLAGS_GCC_REL) ${DLABPRO_GCC_CFLAGS_RELEASE}
  else
    CFLAGS += -g -D_DEBUG ${DLABPRO_GCC_CFLAGS_DEBUG}
  endif
  ifeq ($(TRG_TYPE),C)
    CFLAGS += -x c -D_DLP_C
  else ifeq ($(TRG_TYPE),CPP)
    CFLAGS += -x c++ -D_DLP_CPP
  endif
  ifeq ($(findstring mingw,$(OS)),)
    CFLAGS += -ansi
  else
    LFLAGS += -static
  endif
endif

## Filter-out Variables
CFLAGS := $(filter-out $(CFLAGS_FILTEROUT),$(CFLAGS))

## EOF

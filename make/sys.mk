## dLabPro makefiles
## - System configuration
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
  DCG = ${CGENPATH}/dcg
else
  DCG = dcg
endif

INCL += -I$(DLABPRO_HOME)/include -I$(DLABPRO_HOME)/include/automatic -I$(PRG_PATH)
vpath %.h $(DLABPRO_HOME)/include $(DLABPRO_HOME)/include/automatic $(PRG_PATH)

ifneq (${MACHINE},)
  MEXT=.${MACHINE}
else
  MEXT=
endif

## Detect compile OS
## Implemented values: lin32, lin64, mingw32, mingw64, msv1, msv2

# Test #1 for MSVC
ifneq (${DLABPRO_USE_MSVC},)
  ifeq (${DLABPRO_USE_MSVC},1)
    ## - MSVC
    OS = msv1
  else ifeq (${DLABPRO_USE_MSVC},2)
    ## - MSVC 6.0 - 32-Bit C/C++-Compiler for x86
    OS = msv2
  else
    $(error Unimplemented MSVC Version: "$(DLABPRO_USE_MSVC)")
  endif
else 
  # Test #2 for machine of gcc
  OS := $(call lc,$(shell gcc -dumpmachine))
  ifneq ($(OS),)
    ifeq ($(OS),x86_64-linux-gnu)
      OS = lin64
    else ifeq ($(OS),x86_64-redhat-linux)
      OS = lin64
    else ifeq ($(OS),x86-linux-gnu)
      OS = lin32
    else ifneq ($(findstring mingw32,$(OS)),)
      OS = mingw32
    else
      $(error Unimplemented machine of gcc: "$(OS)")
    endif
  else
    # Test #3 for machine uname
    OS := $(call lc,$(shell uname))
    ifeq ($(OS),Linux)
      OS = lin64
    else ifneq ($(findstring mingw32,$(OS)),)
      OS = mingw32
    else
      $(error Unimplemented operating system: "$(OS)")
    endif
  endif
endif

## Detect git head revision
DLPREV  = $(DLABPRO_HOME)/include/automatic/dlp_rev.h
GITREV := $(shell cat $(DLABPRO_HOME)/.git/HEAD)
ifneq ($(filter ref:,$(GITREV)),)
  GITREV := $(shell cat $(DLABPRO_HOME)/.git/$(filter-out ref:,$(GITREV)))
endif
ifneq ($(wildcard $(DLPREV)),)
  GITREVO := $(subst ",,$(filter-out \#define __DLP_BUILD,$(shell cat $(DLPREV))))
else
  GITREVO = CREATE
endif
ifneq ($(GITREVO),$(GITREV))
  $(shell echo "" >&2)
  $(shell echo "// Update dlp_rev.h with new revision $(GITREV)" >&2)
  $(shell echo '#define __DLP_BUILD "$(GITREV)"' >$(DLPREV))
endif

## EOF

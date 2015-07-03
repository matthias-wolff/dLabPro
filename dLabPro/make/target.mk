## dLabPro makefiles
## - Target configuration
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

## Default make target
ifeq ($(MAKECMDGOALS),)
  MAKECMDGOALS = $(TARGET_DEF)
endif


## Get target for libraries (DEBUG or RELEASE)
ifeq ($(findstring RELEASE,$(call uc,$(MAKECMDGOALS))),)
  TRG_BASE = DEBUG
else
  TRG_BASE = RELEASE
endif

## Get target type (C,CPP or empty)
ifeq ($(TRG_EXT),)
  TRG_EXT  = $(subst $(TRG_BASE),,$(MAKECMDGOALS))
  TRG_EXT := $(subst _,,$(TRG_EXT))
endif

## Create dependency files
ifneq ($(or $(findstring lin,$(OS)),$(findstring mingw,$(OS)),$(findstring cygwin,$(OS))),)
  ifeq ($(findstring CLEAN,$(call uc,$(MAKECMDGOALS))),)
    DEPINC = yes
  endif
endif

## Target directory (debug or release)
TRG_DIR = $(call lc,$(TRG_BASE))

CLEAN    = clean_$(TRG_DIR)
CLEANALL = cleanall_$(TRG_DIR)

## EOF

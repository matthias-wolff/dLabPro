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

ifeq ($(findstring RELEASE,$(call uc,$(MAKECMDGOALS))),)
  TRG_LIB = DEBUG
else
  TRG_LIB = RELEASE
endif

ifeq ($(TRG_TYPE),)
  TRG_TYPE  = $(subst $(TRG_LIB),,$(MAKECMDGOALS))
  TRG_TYPE := $(subst _,,$(TRG_TYPE))
endif

ifeq ($(findstring CLEAN,$(call uc,$(MAKECMDGOALS))),)
  DEPINC = yes
else
  DEPINC = no
endif

TRG_DIR = $(call lc,$(TRG_LIB))

CLEAN    = clean_$(TRG_DIR)
CLEANALL = cleanall_$(TRG_DIR)

## EOF

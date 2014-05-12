## dLabPro makefiles
## - Make include file for programs
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
TRG_EXT    = $(call uc,$(SEXT))
export PROGRAM := $(PROJNAME)

include $(DLABPRO_HOME)/make/func.mk
include $(DLABPRO_HOME)/make/sys.mk
include $(DLABPRO_HOME)/make/target.mk
include $(DLABPRO_HOME)/make/compiler.mk
include $(DLABPRO_HOME)/make/paths.mk

## Finalize library list
CLS_LPTH  = $(LIB_PATH)
BAS_LPTH  = $(LIB_PATH)
SDK_LPTH  = $(LIB_PATH)
EXT_LPTH  = $(LIB_PATH)
LET       = .$(LEXT)
LPOST     = 
LIBRARIES := $(LIBS) $(LIBRARIES_EXTRA)
CLS_LPTH  = $(MAKE) -C $(CLS_PATH)
BAS_LPTH  = $(MAKE) -C $(BAS_PATH)
SDK_LPTH  = $(MAKE) -C $(SDK_PATH)
EXT_LPTH  = $(MAKE) -C $(EXT_PATH)
LET       = 
LPOST     = $(CLEAN) ;
LIBRARIES_CLEANALL := $(LIBS)

## Finalize sources list
SEXTB     = $(call uc,$(SEXT))

## Include deprecated classes
ifeq ($(DLABPRO_DEPRECATED),1)
  LIBS      += ${LIBS_DEPR}
endif

include $(DLABPRO_HOME)/make/libsys.mk

DEBUG RELEASE: ECHOCNF $(PROJECT)
	@echo
	@echo  '// ----- Make ($(TOOLBOX)): Build complete -----'
	@echo  '$(PROJECT) build $(BUILD)'
	@-which $(DCG)
	@which $(CC)
	@which $(AR)
	@which $(LL)
	@echo

include $(DLABPRO_HOME)/make/rules_prg.mk

## EOF

## dLabPro makefiles
## - Build path configuration
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

## Set default program
ifeq ($(PROGRAM),)
  PROGRAM = dlabpro
endif

## Set all paths
BIN_PATH = $(DLABPRO_HOME)/bin.${TRG_DIR}${MEXT}
BLD_PATH = $(DLABPRO_HOME)/build/$(PROGRAM)
LIB_PATH = $(BLD_PATH)/lib.${TRG_DIR}${MEXT}
OBJ_PATH = $(BLD_PATH)/obj.${TRG_DIR}${MEXT}
DEP_PATH = $(BLD_PATH)/dep.${TRG_DIR}${MEXT}
BAS_PATH = $(DLABPRO_HOME)/base
CLS_PATH = $(DLABPRO_HOME)/classes
EXT_PATH = $(DLABPRO_HOME)/ext
SDK_PATH = $(DLABPRO_HOME)/sdk
INC_PATH = $(DLABPRO_HOME)/include
MAN_PATH = $(DLABPRO_HOME)/manual
PRG_PATH = $(DLABPRO_HOME)/programs/$(PROGRAM)

## Target settings
ifeq ($(LIBFILE),)
  LIBFILE=$(PROJNAME)
endif

PROJECT  = $(BIN_PATH)/$(PROJNAME)$(EEXT)
LIBRARY  = $(LIB_PATH)/$(LIBFILE).$(LEXT)
SHARED_LIBRARY = $(LIB_PATH)/lib$(LIBFILE).so
DEFFILE  = $(wildcard $(PROJNAME).def)
MANFILE  = $(MAN_PATH)/automatic/$(PROJNAME).html
SRCFILES = $(addsuffix .$(SEXT),$(SOURCES))
OBJECTS  = $(addprefix $(OBJ_PATH)/,$(addsuffix .$(OEXT),$(SOURCES)))
DEPENTS  = $(addprefix $(DEP_PATH)/,$(addsuffix .$(DEXT),$(SOURCES)))
SRCFILES_NOAUTO = $(filter-out $(CPPFILE),$(SRCFILES))
DCGDEP   = $(wildcard $(DLABPRO_HOME)/bin.release$(MEXT)/dcg$(EEXT))

## EOF

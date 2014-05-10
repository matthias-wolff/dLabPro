## dLabPro makefiles
## - Make include file for libraries in classes package
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

DISPLAY_NAME = dLabPro class $(CXXNAME) ($(SLNAME))

TARGET_DEF = DEBUG_CPP
CFLAGS_GCC_REL += -Wno-trigraphs -finline-functions 

include $(DLABPRO_HOME)/make/func.mk
include $(DLABPRO_HOME)/make/sys.mk
include $(DLABPRO_HOME)/make/target.mk
include $(DLABPRO_HOME)/make/compiler.mk
include $(DLABPRO_HOME)/make/paths.mk

HFILE    = dlp_$(PROJNAME).h
CPPFILE  = $(PROJNAME).$(SEXT)

## Rules 
DEBUG_CPP  : ECHOCNF MKDIR $(LIBRARY)
DEBUG_C    : ECHOCNF MKDIR $(LIBRARY)
RELEASE_CPP: ECHOCNF MKDIR $(LIBRARY)
RELEASE_C  : ECHOCNF MKDIR $(LIBRARY)
SHARED     : ECHOCNF MKDIR $(SHARED_LIBRARY) LDCONF

include $(DLABPRO_HOME)/make/rules.mk

## EOF

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

OS   := $(shell uname)
INCL += -I $(DLABPRO_HOME)/include -I $(DLABPRO_HOME)/include/automatic
vpath %.h $(DLABPRO_HOME)/include $(DLABPRO_HOME)/include/automatic

ifneq ($(findstring MINGW,$(OS)),)
  OS := MinGW
endif

ifneq (${MACHINE},)
  MEXT=.${MACHINE}
else
  MEXT=
endif

## EOF

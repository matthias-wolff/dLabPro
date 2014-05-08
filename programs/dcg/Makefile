## dLabPro program dcg (dLabPro code generator)
## - Main make file
##
## AUTHOR : Matthias Eichner
## PACKAGE: dLabPro/programs
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

ifeq (${DLABPRO_HOME},)
  DLABPRO_HOME = ../..
endif

PROJNAME = dcg
SEXT     = cpp

SOURCES  = $(PROJNAME)

LIBS      = \
	${CLS_LPTH}/function$(LET) $(LPOST)  \
	${CLS_LPTH}/fst$(LET) $(LPOST)       \
	${CLS_LPTH}/data$(LET) $(LPOST)      \
	${CLS_LPTH}/var$(LET) $(LPOST)       \
	${CLS_LPTH}/dgen$(LET) $(LPOST)      \
	${SDK_LPTH}/cgen$(LET) $(LPOST)      \
	${BAS_LPTH}/dlpobject$(LET) $(LPOST) \
	${BAS_LPTH}/dlptable$(LET) $(LPOST)  \
	${BAS_LPTH}/dlpbase$(LET) $(LPOST)   \
	${BAS_LPTH}/dlpmath$(LET) $(LPOST)   \
  ${EXT_LPTH}/clapack$(LET) $(LPOST)   \
	${EXT_LPTH}/kazlib$(LET) $(LPOST)
LIBS_DEPR = 
LIBS_SYS = 

include $(DLABPRO_HOME)/make/program.mk


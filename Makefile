## dLabPro program dLabPro (dlabpro)
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
  DLABPRO_HOME = ../dLabPro
endif

PROJNAME = externalprogram
SEXT     = cpp
export PRG_PATH := $(CURDIR)

SOURCES  = main

LIBS      = \
  ${CLS_LPTH}/file$(LET) $(LPOST)       \
  ${CLS_LPTH}/gmm$(LET) $(LPOST)        \
  ${CLS_LPTH}/vmap$(LET) $(LPOST)       \
  ${CLS_LPTH}/profile$(LET) $(LPOST)    \
  ${CLS_LPTH}/matrix$(LET) $(LPOST)     \
  ${CLS_LPTH}/signal$(LET) $(LPOST)     \
  ${CLS_LPTH}/fst$(LET) $(LPOST)        \
  ${CLS_LPTH}/var$(LET) $(LPOST)        \
  ${CLS_LPTH}/data$(LET) $(LPOST)       \
  ${BAS_LPTH}/dn3stream$(LET) $(LPOST)  \
  ${BAS_LPTH}/xmlstream$(LET) $(LPOST)  \
  ${BAS_LPTH}/dlpobject$(LET) $(LPOST)  \
  ${BAS_LPTH}/dlptable$(LET) $(LPOST)   \
  ${BAS_LPTH}/dlpbase$(LET) $(LPOST)    \
  ${BAS_LPTH}/dlpmath$(LET) $(LPOST)    \
  ${BAS_LPTH}/dnorm$(LET) $(LPOST)      \
  ${EXT_LPTH}/ipkclib$(LET) $(LPOST)    \
  ${EXT_LPTH}/sptk$(LET) $(LPOST)       \
  ${EXT_LPTH}/clapack$(LET) $(LPOST)    \
  ${EXT_LPTH}/libsndfile$(LET) $(LPOST) \
  ${EXT_LPTH}/zlib$(LET) $(LPOST)       \
  ${EXT_LPTH}/xpat$(LET) $(LPOST)       \
  ${EXT_LPTH}/kazlib$(LET) $(LPOST)
LIBS_DEPR = 
LIBS_SYS = 

include $(DLABPRO_HOME)/make/program.mk


## dLabPro DNorm3 library
## - Makefile
##
## AUTHOR : Torsten Rudolph, Matthias Eichner
## PACKAGE: dLabPro/base
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

PROJNAME=dnorm

CFLAGS_FILTEROUT=-Wall -ansi

## Target settings
SEXT    = c
SOURCES = dopen  \
          dget   \
          dread  \
          krtree \
          derror \
          dsvl
DEPS    = dlp_dnorm.h dlp_dnormint.h

include $(DLABPRO_HOME)/make/base.mk

## EOF

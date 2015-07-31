## dLabPro mathematics library
## - Makefile
##
## AUTHOR : Matthias Wolff
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

PROJNAME=dlpmath

## Target settings
SEXT    = c
SOURCES = dlm_arith     \
          dlm_chf       \
          dlm_cep       \
          dlm_crc32     \
          dlm_comb      \
          dlm_dft       \
          dlm_dtw       \
          dlm_eigen     \
          dlm_filter    \
          dlm_gcep      \
          dlm_mgcep     \
          dlm_mgcepfix  \
          dlm_gel       \
          dlm_lpc       \
          dlm_lsf       \
          dlm_lcq       \
          dlm_lud       \
          dlm_melfilter \
          dlm_mlt       \
          dlm_noiserdc  \
          dlm_ftt       \
          dlm_pitch     \
          dlm_fba       \
          dlm_vad       \
          dlm_pow       \
          dlm_rand      \
          dlm_roots     \
          dlm_sig       \
          dlm_stat      \
          dlm_trig      \
          dlm_xft       \
          dlm_ldl       \
          dlm_fwt       \
          dlm_vq        \
          dlm_fix       \
          dlp_measurement
DEPS    = dlp_base.h dlp_math.h

include $(DLABPRO_HOME)/make/base.mk

## EOF

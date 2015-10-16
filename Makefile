## dLabPro
## - Main make file
##
## AUTHOR : Frank Duckhorn
## PACKAGE: dLabPro
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

all: RELEASE

.PHONY: RELEASE DEBUG CLEANALL DLABPRO DLABPRO_DEBUG

DLABPRO:
	$(MAKE) -C programs/dlabpro    RELEASE
	
DLABPRO_DEBUG:
	$(MAKE) -C programs/dlabpro    DEBUG

RELEASE: CLEANALL
	$(MAKE) -C programs/dcg        RELEASE
	$(MAKE) -C programs/dlabpro    RELEASE
	$(MAKE) -C programs/recognizer RELEASE

DEBUG: CLEANALL
	$(MAKE) -C programs/dcg        RELEASE
	$(MAKE) -C programs/dcg        DEBUG
	$(MAKE) -C programs/dlabpro    DEBUG
	$(MAKE) -C programs/recognizer DEBUG

CLEANALL:
	$(MAKE) -C programs/dcg        CLEANALL
	$(MAKE) -C programs/dlabpro    CLEANALL
	$(MAKE) -C programs/recognizer CLEANALL


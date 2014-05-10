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
TRG_TYPE   = $(call uc,$(SEXT))
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

## Build targets
.PHONY: DEBUG RELEASE clean clean_debug clean_release CLEANALL cleanall_debug cleanall_release distclean

DEBUG RELEASE: $(PROJECT)
	@echo
	@echo  '// ----- Make ($(TOOLBOX)): Build complete -----'
	@echo  '$(PROJECT) build $(BUILD)'
	@-which $(DCG)
	@which $(CC)
	@which $(AR)
	@which $(LL)
	@echo

${PROJECT}: ${OBJECTS} ${LIBRARIES}
	@echo
	@echo '// ----- Make ($(TOOLBOX)): $(PROJNAME) linking -- $(MAKECMDGOALS) -----'
	$(LL) $(OBJECTS) $(LIBRARIES) $(LFLAGS) $(LLoO)$(PROJECT)

$(LIB_PATH)/%.$(LEXT): $(BAS_PATH)/% FORCE
	$(MAKE) -C $(BAS_PATH)/$*   $(TRG_LIB)`[ $* = dlpobject ] && echo _$(SEXTB)`

$(LIB_PATH)/%.$(LEXT): $(CLS_PATH)/% FORCE
	$(MAKE) -C $(CLS_PATH)/$*   $(TRG_LIB)_$(SEXTB)

$(LIB_PATH)/%.$(LEXT): $(SDK_PATH)/% FORCE
	$(MAKE) -C $(SDK_PATH)/$*   $(TRG_LIB)_$(SEXTB)

$(LIB_PATH)/%.$(LEXT): $(EXT_PATH)/% FORCE
	$(MAKE) -C $(EXT_PATH)/$*   $(TRG_LIB)

$(DEP_PATH)/%.$(DEXT): %.$(SEXT)
	$(CC) -MM -MP -MT $(OBJ_PATH)/$*.$(OEXT) -MT $@ $(CFLAGS) $(INCL) -MF $@ $<

## Include dependency makefiles
ifeq ($(DEPINC),yes)
  ifneq ($(or $(findstring lin,$(OS)),$(findstring mingw,$(OS))),)
    -include $(DEPENTS)
  endif
endif

$(OBJ_PATH)/%.$(OEXT): %.$(SEXT)
	@echo
	@echo '// ----- Make ($(TOOLBOX)): Program $(PROJNAME) -- $(MAKECMDGOALS) -----'
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$@ $<

FORCE:

clean:
	$(MAKE) clean_debug
	$(MAKE) clean_release

CLEANALL:
	$(MAKE) cleanall_debug
	$(MAKE) cleanall_release

$(CLEAN):
	@echo '// ----- Make: Program $(PROJNAME) -- cleaning $(TRG_LIB) -----'
	-rm -f $(OBJECTS) $(DEPENTS)
	-touch -c -t 199001010000 $(PROJECT)

$(CLEANALL): $(CLEAN)
	$(LIBRARIES_CLEANALL)
	@echo '// ----- Make: Program $(PROJNAME) -- cleaning all $(TRG_LIB) -----'
	-rm -f $(LIBRARIES)
	-touch -c dlp_config.h
	-find $(DLABPRO_HOME)/ \( -name "*.def" \) -exec touch {} \;


distclean:
	-rm -rf $(DLABPRO_HOME)/bin.*
	-rm -rf $(DLABPRO_HOME)/lib.*
	-rm -rf $(DLABPRO_HOME)/obj.*
	-rm -rf $(DLABPRO_HOME)/dep.*
	-find $(DLABPRO_HOME)/ \( -name "core" -o -name "*~" -o -name "*.RUN" -o -name "*.plg" -o -name "*.ncb" -o -name "\#*\#" \) -exec rm -f {} \;

backup: cleanall distclean
	cd $(DLABPRO_HOME)/ && tar czf ../dlpabpro.tgz .

dos2unix:
	find $(DLABPRO_HOME)/ \( -name "*.c" -o -name "*.cpp" -o -name "*.h" -o -name "*.hpp" -o -name "*.def" -o -name "Makefile" -o -name "makefile" \) -exec recode pc {} \;

## EOF

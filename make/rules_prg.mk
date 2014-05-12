## dLabPro makefiles
## - Rules for programs
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

## Build targets
.PHONY: ECHOCNF DEBUG RELEASE clean clean_debug clean_release CLEANALL cleanall_debug cleanall_release distclean

## Rules for program link

${PROJECT}: ${OBJECTS} ${LIBRARIES}
	@echo
	@echo '// ----- Make ($(TOOLBOX)): $(PROJNAME) linking -- $(MAKECMDGOALS) -----'
	$(LL) $(OBJECTS) $(LIBRARIES) $(LFLAGS) $(LLoO)$(PROJECT)

## Rules for library generation

FORCE:

$(LIB_PATH)/%.$(LEXT): $(BAS_PATH)/% FORCE
	$(MAKE) -C $(BAS_PATH)/$*   $(TRG_LIB)$(if $(filter dlpobject,$*),_$(SEXTB),)

$(LIB_PATH)/%.$(LEXT): $(CLS_PATH)/% FORCE
	$(MAKE) -C $(CLS_PATH)/$*   $(TRG_LIB)_$(SEXTB)

$(LIB_PATH)/%.$(LEXT): $(SDK_PATH)/% FORCE
	$(MAKE) -C $(SDK_PATH)/$*   $(TRG_LIB)_$(SEXTB)

$(LIB_PATH)/%.$(LEXT): $(EXT_PATH)/% FORCE
	$(MAKE) -C $(EXT_PATH)/$*   $(TRG_LIB)

## Rules for dependencies + source compile

$(DEP_PATH)/%.$(DEXT): %.$(SEXT)
	@$(CC) -MM -MP -MT $(OBJ_PATH)/$*.$(OEXT) -MT $@ $(CFLAGS) $(INCL) -MF $@ $<

ifeq ($(DEPINC),yes)
  -include $(DEPENTS)
endif

$(OBJ_PATH)/%.$(OEXT): %.$(SEXT)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$@ $<

## Echo current configuration

ECHOCNF:
	@echo
	@echo '// ----- Make ($(TOOLBOX)): Program $(PROJNAME) -- $(MAKECMDGOALS) -----'

## Rules for clean targets

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

## Rules for distribution

distclean:
	-rm -rf $(DLABPRO_HOME)/bin.*
	-rm -rf $(DLABPRO_HOME)/build
	-find $(DLABPRO_HOME)/ \( -name "core" -o -name "*~" -o -name "*.RUN" -o -name "*.plg" -o -name "*.ncb" -o -name "\#*\#" \) -exec rm -f {} \;

backup: cleanall distclean
	cd $(DLABPRO_HOME)/ && tar czf ../dlpabpro.tgz .

dos2unix:
	find $(DLABPRO_HOME)/ \( -name "*.c" -o -name "*.cpp" -o -name "*.h" -o -name "*.hpp" -o -name "*.def" -o -name "Makefile" -o -name "makefile" \) -exec recode pc {} \;

## EOF

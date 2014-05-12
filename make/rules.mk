## dLabPro makefiles
## - Rules for packages
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
.PHONY: ECHOCNF MKDIR LDCONF clean clean_debug clean_release

## Rules for library link

$(LIBRARY): $(OBJECTS)
	$(AR) $(ARFLAGS) $(ARoO)$(LIBRARY) $(OBJECTS)

$(SHARED_LIBRARY): $(OBJECTS)
	$(CC) -shared -Wl,-soname,$(SHARED_LIBRARY).0 $(OBJECTS) \
          $(CCoO)$(LIB_PATH)/$(SHARED_LIBRARY).0.0

LDCONF:
	@-cd $(LIB_PATH) && ln -sf $(SHARED_LIBRARY).0.0 $(SHARED_LIBRARY).0 \
          && ln -sf $(SHARED_LIBRARY).0 $(SHARED_LIBRARY)

## Rules for dependencies + source compile

$(DEP_PATH)/%.$(DEXT): %.$(SEXT)
	@$(CC) -MM -MP -MT $(OBJ_PATH)/$*.$(OEXT) -MT $@ $(CFLAGS) $(INCL) -MF $@ $<

ifeq ($(DEPINC),yes)
  -include $(DEPENTS)
endif

$(OBJ_PATH)/%.$(OEXT): %.$(SEXT)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$@ $<

## Rules for code generator

ifneq ($(DEFFILE),)

ifneq ($(HFILE),)
  TOUCH_H=@touch -c -r $(DEFFILE) $(HFILE)
endif

$(CPPFILE) $(HFILE) $(MANFILE): $(DEFFILE) $(DCGDEP) #$(SRCFILES_NOAUTO)
	@-$(DCG) $(DEFFILE)
	$(TOUCH_H)

endif

## Echo current configuration

ECHOCNF:
	@echo
	@echo '// ----- Make ($(TOOLBOX)): $(DISPLAY_NAME) -- $(MAKECMDGOALS) -----'

## Create target directory

MKDIR:
	@-test -w $(OBJ_PATH) || mkdir $(OBJ_PATH)
	@-test -w $(LIB_PATH) || mkdir $(LIB_PATH)

## Rules for clean targets

clean:
	$(MAKE) clean_debug
	$(MAKE) clean_release

ifneq ($(and $(DEFFILE),$(CPPFILE)),)
  TOUCH_DEF=-touch -c -r $(DEFFILE) -d yesterday $(CPPFILE)
endif

$(CLEAN):
	@echo '// ----- Make ($(TOOLBOX)): $(DISPLAY_NAME) -- cleaning $(TRG_LIB) -----'
	-rm -f $(OBJECTS) $(DEPENTS) $(LIBRARY) $(SHARED_LIBRARY)
	-rm -f vc80.?db
	$(TOUCH_DEF)


## EOF

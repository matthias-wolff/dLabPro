## dLabPro makefiles
## - Rules make include file
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

$(LIBRARY): $(OBJECTS)
	$(AR) $(ARFLAGS) $(ARoO)$(LIBRARY) $(OBJECTS)

$(SHARED_LIBRARY): $(OBJECTS)
	$(CC) -shared -Wl,-soname,$(SHARED_LIBRARY).0 $(OBJECTS) \
          $(CCoO)$(LIB_PATH)/$(SHARED_LIBRARY).0.0

# Create links to shared lib
LDCONF:
	@-cd $(LIB_PATH) && ln -sf $(SHARED_LIBRARY).0.0 $(SHARED_LIBRARY).0 \
          && ln -sf $(SHARED_LIBRARY).0 $(SHARED_LIBRARY)

$(OBJ_PATH)/%.$(OEXT): %.$(SEXT) $(DEPS)
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$@ $<

$(MANFILE): $(DEFFILE) $(SRCFILES)
	@-$(DCG) $(DEFFILE)

$(HFILE): $(DEFFILE) $(CDEPS)
	@-$(DCG) $(DEFFILE)

$(CPPFILE): $(DEFFILE) $(CDEPS)
	@-$(DCG) $(DEFFILE)

## Additional rules
.PHONY: ECHOCNF MKDIR LDCONF clean clean_debug clean_release

# Echo current configuration
ECHOCNF:
	@echo
	@echo '// ----- Make ($(TOOLBOX)): $(DISPLAY_NAME) -- $(MAKECMDGOALS) -----'

# Create target directory
MKDIR:
	@-test -w $(OBJ_PATH) || mkdir $(OBJ_PATH)
	@-test -w $(LIB_PATH) || mkdir $(LIB_PATH)

clean:  clean_debug

ifneq ($(DEFFILE),)
  TOUCH=touch
endif

clean_debug: $(TOUCH)
	@echo '// ----- Make ($(TOOLBOX)): $(DISPLAY_NAME) -- cleaning DEBUG -----'
	-rm -f $(OBJECTS) $(LIBRARY) $(SHARED_LIBRARY)
	-rm -f vc80.?db

clean_release: $(TOUCH)
	@echo '// ----- Make ($(TOOLBOX)): $(DISPLAY_NAME) -- cleaning RELEASE -----'
	-rm -f $(OBJECTS) $(LIBRARY) $(SHARED_LIBRARY)
	-rm -f vc80.?db

touch:
	-touch -c -r $(DEFFILE) -d yesterday $(CPPFILE)


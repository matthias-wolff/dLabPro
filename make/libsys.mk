## dLabPro makefiles
## - System library configuration
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

## Platform specific - GNU on Windows
ifneq ($(or $(findstring mingw,$(OS)),$(findstring cygwin,$(OS))),)
  PT_AVAILABLE := $(shell echo -e "\#include <pthread.h>\nint main(){ return 0; }" | $(CC) $(INCL) $(CFLAGS) -E - >/dev/null 2>&1 && echo yes || echo no)
  CC_VERS := $(shell echo -e "int main(){ return 0; }" | $(CC) -E -dM - | grep __GNUC__ | cut -d" " -f3)
  ifeq ($(shell test ${CC_VERS} -lt 4 && echo yes || echo no ),yes)
    LFLAGS += -lwinmm
  else
    ifneq ($(findstring readline,$(LIBS_SYS)),)
      RL_AVAILABLE := $(shell echo -e "\#include <readline/readline.h>\nint main(){ return 0; }" | $(CC) $(INCL) $(CFLAGS) -E - >/dev/null 2>&1 && echo yes || echo no)
      ifeq ($(RL_AVAILABLE),yes)
        LFLAGS  += -l:libreadline.a -l:libncursesw.a
      else
        CFLAGS += -D__NOREADLINE
      endif
    endif
  endif
endif

ifneq ($(findstring lin,$(OS)),)
## Test for lib readline
  ifneq ($(findstring readline,$(LIBS_SYS)),)
    RL_AVAILABLE := $(shell echo "\#include <readline/readline.h>\nint main(){ return 0; }" | $(CC) $(INCL) $(CFLAGS) -E - >/dev/null 2>&1 && echo yes || echo no)
    LD_VERS := $(shell ld -v | sed -e 's/([[:alnum:][:blank:][:punct:]]*)//g;s/^[[:alpha:][:blank:]]*//;s/\.//;s/\..*//')
    LD_V_OK := $(shell test "$(LD_VERS)" -ge 218 && echo yes || echo no)
    ifeq ($(LD_V_OK),yes)
      RL_STAT := $(shell g++ -l:libreadline.a 2>&1 | grep -q "cannot find libreadline.a" && echo no || echo yes)
    else
      RL_STAT := no
    endif

  # Add readline libary or disable readline libary 
    ifeq ($(RL_AVAILABLE),yes)
      ifeq ($(RL_STAT),yes)
        LFLAGS  += -l:libreadline.a -lncurses
      else
        LFLAGS  += -lreadline
      endif
    else
      CFLAGS += -D__NOREADLINE
      $(shell echo "-- Warning: readline libary support disabled (please install readline-devel with <readline/readline.h>) --" >&2)
    endif
  endif
endif

ifneq ($(findstring portaudio,$(LIBS_SYS)),)
  PA_DIR = $(DLABPRO_HOME)/ext/portaudio/$(OS)
  ## Test for portaudio
  ifneq ($(findstring lin,$(OS)),)
    PA_AVAILABLE := $(shell test -f $(PA_DIR)/libportaudio.a -a -f $(PA_DIR)/portaudio.h && echo yes || echo no)
  else
    PA_AVAILABLE := $(shell test -f $(PA_DIR)/PortAudio.dll -a -f $(PA_DIR)/portaudio.h && echo yes || echo no)
  endif

  ## Add portaudio library
  ifeq ($(PA_AVAILABLE),yes)
    CFLAGS += -D__USE_PORTAUDIO -I$(PA_DIR)
    ifeq ($(shell test -f $(PA_DIR)/LFLAGS && echo yes || echo no),yes)
      LFLAGS += $(PA_DIR)/$(shell cat $(PA_DIR)/LFLAGS)
    endif
    ifeq ($(shell test -f $(PA_DIR)/LFLAGS.sh && echo yes || echo no),yes)
      LFLAGS += $(shell sh $(PA_DIR)/LFLAGS.sh $(PA_DIR))
    endif
    ifneq ($(findstring mingw,$(OS)),)

$(PROJECT): $(BIN_PATH)/portaudio_x86.dll

$(BIN_PATH)/portaudio_x86.dll: $(PA_DIR)/PortAudio.dll
	cp $< $@

    endif
  endif
endif

# Add pthread libary
ifneq ($(findstring lin,$(OS)),)
  LFLAGS  += -lpthread -lrt
else
  ifeq ($(PT_AVAILABLE),yes)
    LFLAGS  += -lpthread
  endif
endif

## EOF

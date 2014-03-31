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

## Common settings
ifdef (${CGENPATH})
  CGEN = ${CGENPATH}/dcg
else
  CGEN = dcg
endif

OS   := $(shell uname)
INCL  = -I ../../include -I ../../include/automatic 
vpath %.h ../../include ../../include/automatic

## Get build ID
BUILD := $(shell svnversion ../../ 2> /dev/null)

## Paths
BIN_PATH = ../../bin.${TRGS}.${MACHINE}
LIB_PATH = ../../lib.${TRGS}.${MACHINE}
OBJ_PATH = ../../obj.${TRGS}.${MACHINE}
DEP_PATH = ../../dep.${TRGS}.${MACHINE}
BAS_PATH = ../../base
CLS_PATH = ../../classes
SDK_PATH = ../../sdk
EXT_PATH = ../../ext
INC_PATH = ../../include
CONFIG_DST = ${INC_PATH}/dlp_config.h
CONFIG_SRC = dlp_config.h
DLPSVNREV  = ${INC_PATH}/automatic/dlp_svnrev.h

## Update dlp_svnrec.h if necessary
ifeq ($(OS),Linux)
  SVNREV := $(shell grep '\#define __DLP_BUILD "$(BUILD)"' $(DLPSVNREV))
endif
ifeq ($(SVNREV),)
  X := $(shell echo "// ----- Make: Updating dlp_svnrev.h ($(BUILD)) -----" >&2)
  X := $(shell echo '\#define __DLP_BUILD "$(BUILD)"' >$(DLPSVNREV))
endif


## Target settings
PROJNAME  = dlabpro
PROJECT   = $(BIN_PATH)/$(PROJNAME)$(EEXT)
SOURCES   = $(SPTH)/$(PROJNAME).$(SEXT)
LIBS      = \
  ${CLS_LPTH}/fstsearch$(LET) $(LPOST)  \
  ${CLS_LPTH}/fsttools$(LET) $(LPOST)   \
  ${CLS_LPTH}/melproc$(LET) $(LPOST)    \
  ${CLS_LPTH}/fttproc$(LET) $(LPOST)    \
  ${CLS_LPTH}/vadproc$(LET) $(LPOST)    \
  ${CLS_LPTH}/hfaproc$(LET) $(LPOST)    \
  ${CLS_LPTH}/hmm$(LET) $(LPOST)        \
  ${CLS_LPTH}/svm$(LET) $(LPOST)        \
  ${CLS_LPTH}/histogram$(LET) $(LPOST)  \
  ${CLS_LPTH}/file$(LET) $(LPOST)       \
  ${CLS_LPTH}/fwtproc$(LET) $(LPOST)    \
  ${CLS_LPTH}/cpproc$(LET) $(LPOST)     \
  ${CLS_LPTH}/lpcproc$(LET) $(LPOST)    \
  ${CLS_LPTH}/lsfproc$(LET) $(LPOST)    \
  ${CLS_LPTH}/lcqproc$(LET) $(LPOST)    \
  ${CLS_LPTH}/pmproc$(LET) $(LPOST)     \
  ${CLS_LPTH}/gmm$(LET) $(LPOST)        \
  ${CLS_LPTH}/statistics$(LET) $(LPOST) \
  ${CLS_LPTH}/vmap$(LET) $(LPOST)       \
  ${CLS_LPTH}/fftproc$(LET) $(LPOST)    \
  ${CLS_LPTH}/fbaproc$(LET) $(LPOST)    \
  ${CLS_LPTH}/process$(LET) $(LPOST)    \
  ${CLS_LPTH}/profile$(LET) $(LPOST)    \
  ${CLS_LPTH}/prosody$(LET) $(LPOST)    \
  ${SDK_LPTH}/helloworld$(LET) $(LPOST) \
  ${CLS_LPTH}/function$(LET) $(LPOST)   \
  ${CLS_LPTH}/matrix$(LET) $(LPOST)     \
  ${CLS_LPTH}/signal$(LET) $(LPOST)     \
  ${CLS_LPTH}/dgen$(LET) $(LPOST)       \
  ${CLS_LPTH}/fst$(LET) $(LPOST)        \
  ${CLS_LPTH}/var$(LET) $(LPOST)        \
  ${CLS_LPTH}/data$(LET) $(LPOST)       \
  ${EXT_LPTH}/libsvm$(LET) $(LPOST)     \
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

## Default make target
ifeq ($(MAKECMDGOALS),)
  MAKECMDGOALS = DEBUG
endif

## Common settings
ifeq (${DLABPRO_USE_MSVC},1)
  CC      = CL
  CCoO    = -Fo
  AR      = LIB
  LL      = LINK
  LLoO    = -OUT:
  OEXT    = obj
  LEXT    = lib
  EEXT    = .exe
  TOOLBOX = MSVC
else 
  ifeq (${DLABPRO_USE_MSVC},2)
    ## - MSVC 6.0 - 32-Bit C/C++-Compiler for x86
    CC      = CL
    CCoO    = -Fo
    AR      = LIB
    LL      = LINK
    LLoO    = -OUT:
    OEXT    = obj
    LEXT    = lib
    EEXT    = .exe
    TOOLBOX = MSVC6
  else
    CC      = gcc
    CCoO    = -o
    AR      = ar
    LL      = g++
    LLoO    = -o 
    OEXT    = o
    LEXT    = a
    DEXT    = d
    EEXT    = 
    TOOLBOX = GCC
  endif
endif


## Special settings
ifeq (${DLABPRO_USE_MSVC},1)
  CFLAGS_DBG = -nologo -Od -Gm -EHsc -RTC1 -Wp64 -ZI -D_DEBUG -D_DLP_CPP ${DLABPRO_MSVC_FLAGS_DEBUG}
  CFLAGS_REL = -nologo -O2 -GL -D_RELEASE -D_DLP_CPP -EHsc -W3 -Wp64 -D_CRT_SECURE_NO_WARNINGS ${DLABPRO_MSVC_FLAGS_RELEASE}
  LFLAGS_DBG = -NOLOGO -INCREMENTAL -DEBUG -NODEFAULTLIB:libcmt libcmtd.lib
  LFLAGS_REL = -NOLOGO -LTCG
else
ifeq (${DLABPRO_USE_MSVC},2)
  CFLAGS_DBG = -nologo -Od -Gm -EHsc -RTC1 -ZI -D_DEBUG -D_DLP_CPP ${DLABPRO_MSVC_FLAGS_DEBUG}
  CFLAGS_REL = -nologo -O2 -D_RELEASE -D_DLP_CPP -EHsc -W3 -D_CRT_SECURE_NO_WARNINGS ${DLABPRO_MSVC_FLAGS_RELEASE}
  LFLAGS_DBG = -NOLOGO -INCREMENTAL -DEBUG -NODEFAULTLIB:libcmt libcmtd.lib
  LFLAGS_REL = -NOLOGO -LTCG
else
  CFLAGS_DBG = -g -D_DEBUG -D_DLP_CPP -ansi ${DLABPRO_GCC_CFLAGS_DEBUG}
  CFLAGS_REL = -O2 -D_RELEASE -D_DLP_CPP -Wall -ansi ${DLABPRO_GCC_CFLAGS_RELEASE}
  LFLAGS_DBG = -lm ${DLABPRO_GCC_LFLAGS_DEBUG}
  LFLAGS_REL = -lm ${DLABPRO_GCC_LFLAGS_RELEASE}
endif
endif

## Finalize Target settings
LFLAGS   = $(LFLAGS_$(TRG))
CFLAGS   = $(CFLAGS_$(TRG))
CLEAN    = clean_$(TRGS)
CLEANALL = cleanall$(TRGCLN)

TARGET = DEBUG
TRG    = DBG
TRGS   = debug
TRGCLN = _debug
DEPINC = yes
ifeq ($(MAKECMDGOALS),RELEASE)
  TARGET = RELEASE
endif
ifeq ($(MAKECMDGOALS),clean_release)
  TARGET = RELEASE
  DEPINC = no
endif
ifeq ($(MAKECMDGOALS),cleanall_release)
  TARGET = RELEASE
  DEPINC = no
endif
ifeq ($(MAKECMDGOALS),clean)
  DEPINC = no
endif
ifeq ($(MAKECMDGOALS),clean_debug)
  DEPINC = no
endif
ifeq ($(MAKECMDGOALS),cleanall_debug)
  DEPINC = no
endif
ifeq ($(MAKECMDGOALS),CLEANALL)
  DEPINC = no
endif
ifeq ($(MAKECMDGOALS),distclean)
  DEPINC = no
endif

ifeq ($(TARGET),RELEASE)
  TRG    = REL
  TRGS   = release
  TRGCLN = _release
endif

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
SPTH      = $(OBJ_PATH)
SEXT      = $(OEXT)
OBJECTS  := $(SOURCES)
SPTH      = $(DEP_PATH)
SEXT      = $(DEXT)
DEPENTS  := $(SOURCES)
SPTH      = .
SEXT      = cpp
SEXTB     = CPP


## Include deprecated classes
ifeq ($(DLABPRO_DEPRECATED),1)
  LIBS      += ${LIBS_DEPR}
  CONFIG_SRC = dlp_config_deprecated.h
endif

## Copy config
X := $(shell cp -p $(CONFIG_SRC) $(CONFIG_DST))


## Use additional libraries depending on machine type

## Platform specific - GNU on Windows
ifeq (${MACHINE},$(filter ${MACHINE},x86-w32 x86-w64))
  ifneq (${DLABPRO_USE_MSVC},1)
    ifneq (${DLABPRO_USE_MSVC},2)
      PT_AVAILABLE := $(shell echo -e "\#include <pthread.h>\nint main(){ return 0; }" | $(CC) $(INCL) $(CFLAGS) -E - >/dev/null 2>&1 && echo yes || echo no)
      CC_VERS := $(shell echo -e "int main(){ return 0; }" | $(CC) -E -dM - | grep __GNUC__ | cut -d" " -f3)
      ifeq ($(shell test ${CC_VERS} -lt 4 && echo yes || echo no ),yes)
        LFLAGS += -lwinmm
      else
        RL_AVAILABLE := $(shell echo -e "\#include <readline/readline.h>\nint main(){ return 0; }" | $(CC) $(INCL) $(CFLAGS) -E - >/dev/null 2>&1 && echo yes || echo no)
        ifeq ($(RL_AVAILABLE),yes)
          LFLAGS  += -l:libreadline.a
        else
          CFLAGS += -D__NOREADLINE
        endif
      endif
    endif
  endif
endif

## Test for lib readline
ifeq ($(OS),Linux)
  ifneq (${DLABPRO_USE_MSVC},1)
    ifneq (${DLABPRO_USE_MSVC},2)
      RL_AVAILABLE := $(shell echo -e "\#include <readline/readline.h>\nint main(){ return 0; }" | $(CC) $(INCL) $(CFLAGS) -E - >/dev/null 2>&1 && echo yes || echo no)
      LD_VERS := $(shell ld -v | sed -e 's/([[:alnum:][:blank:][:punct:]]*)//g;s/^[[:alpha:][:blank:]]*//;s/\.//;s/\..*//')
      LD_V_OK := $(shell test "$(LD_VERS)" -ge 218 && echo yes || echo no)
      ifeq ($(LD_V_OK),yes)
        RL_STAT := $(shell g++ -l:libreadline.a 2>&1 | grep -q "cannot find libreadline.a" && echo no || echo yes)
      else
        RL_STAT := no
      endif
    endif
  endif

# Add readline libary or disable readline libary 
  ifeq ($(RL_AVAILABLE),yes)
    ifeq ($(RL_STAT),yes)
      LFLAGS  += -l:libreadline.a -lncurses
    else
      LFLAGS  += -lreadline
    endif
  else
    DLABPRO_GCC_CFLAGS_DEBUG += -D__NOREADLINE
    DLABPRO_GCC_CFLAGS_RELEASE += -D__NOREADLINE
    ifeq ($(OS),Linux)
      $(shell echo "-- Warning: readline libary support disabled (please install readline-devel with <readline/readline.h>) --" >&2)
    endif
  endif
endif

# Add pthread libary
ifeq ($(OS),Linux)
  LFLAGS  += -lpthread -lrt
else
  ifeq ($(PT_AVAILABLE),yes)
    LFLAGS  += -lpthread
  endif
endif


## Create directories
X:=$(shell mkdir -p $(OBJ_PATH))
X:=$(shell mkdir -p $(BIN_PATH))
ifeq (${OS},Linux)
  X:=$(shell mkdir -p $(DEP_PATH))
endif


## Build targets
.PHONY: DEBUG RELEASE clean clean_debug clean_release CLEANALL cleanall_debug cleanall_release distclean

DEBUG RELEASE: $(PROJECT)
	@echo
	@echo  '// ----- Make ($(TOOLBOX)): Build complete -----'
	@echo  'build $(BUILD)'
	@-which $(CGEN)
	@which $(CC)
	@which $(AR)
	@which $(LL)
	@echo

${PROJECT}: ${OBJECTS} ${LIBRARIES}
	@echo
	@echo '// ----- Make ($(TOOLBOX)): $(PROJNAME) linking -- $(MAKECMDGOALS) -----'
	$(LL) $(OBJECTS) $(LIBRARIES) $(LFLAGS) $(LLoO)$(PROJECT)

$(LIB_PATH)/%.$(LEXT): $(BAS_PATH)/% FORCE
	$(MAKE) -C $(BAS_PATH)/$*   $(TARGET)`[ $* = dlpobject ] && echo _$(SEXTB)`

$(LIB_PATH)/%.$(LEXT): $(CLS_PATH)/% FORCE
	$(MAKE) -C $(CLS_PATH)/$*   $(TARGET)_$(SEXTB)

$(LIB_PATH)/%.$(LEXT): $(SDK_PATH)/% FORCE
	$(MAKE) -C $(SDK_PATH)/$*   $(TARGET)_$(SEXTB)

$(LIB_PATH)/%.$(LEXT): $(EXT_PATH)/% FORCE
	$(MAKE) -C $(EXT_PATH)/$*   $(TARGET)

$(DEP_PATH)/%.$(DEXT): %.$(SEXT)
	$(CC) -MM -MP -MT $(OBJ_PATH)/$*.$(OEXT) -MT $@ $(CFLAGS) $(INCL) -MF $@ $*.$(SEXT)

## Include dependency makefiles
ifeq ($(DEPINC),yes)
  ifeq (${OS},Linux)
    -include $(DEPENTS)
  endif
endif

$(OBJ_PATH)/%.$(OEXT): %.$(SEXT)
	@echo
	@echo '// ----- Make ($(TOOLBOX)): Program $(PROJNAME) -- $(MAKECMDGOALS) -----'
	$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$@ $*.$(SEXT)

FORCE:

clean:
	$(MAKE) clean_debug
	$(MAKE) clean_release

CLEANALL:
	$(MAKE) cleanall_debug
	$(MAKE) cleanall_release

$(CLEAN):
	@echo '// ----- Make: Program $(PROJNAME) -- cleaning $(TARGET) -----'
	-rm -f $(OBJECTS) $(DEPENTS)
	-touch -c -t 19900101 $(PROJECT)

$(CLEANALL): $(CLEAN)
	$(LIBRARIES_CLEANALL)
	@echo '// ----- Make: Program $(PROJNAME) -- cleaning all $(TARGET) -----'
	-rm -f $(LIBRARIES)
	-touch -c dlp_config.h
	-find ../../ \( -name "*.def" \) -exec touch {} \;


distclean:
	-rm -rf ../../include/automatic
	-rm -rf ../../bin.*
	-rm -rf ../../lib.*
	-rm -rf ../../obj.*
	-rm -rf ../../dep.*
	-rm -rf ../../manual
	-find ../../ \( -name "core" -o -name "*~" -o -name "*.RUN" -o -name "*.plg" -o -name "*.ncb" -o -name "\#*\#" \) -exec rm -f {} \;

## EOF

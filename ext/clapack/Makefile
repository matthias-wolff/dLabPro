## External library clapack
## - Makefile
##
## AUTHOR : Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd., Courant Institute, Argonne National Lab, and Rice Universit, Guntram Strecha (Makefile)
## PACKAGE: dLabPro/ext
##
## LICENSE: clapack is free for any purpose. See ./README for details.

## Common settings
vpath %.h ../../include ../../include/automatic
INCL = -I ../../include -I ../../include/automatic

ifneq (${MACHINE},)
  MEXT=.${MACHINE}
else
  MEXT=
endif

## Compiler specific settings 
ifeq (${DLABPRO_USE_MSVC},1)
  ## - MSVC
  CC       = CL
  CFLAGS   = -nologo -Od -Gm -EHsc -RTC1 -Wp64 -ZI ${DLABPRO_MSVC_FLAGS_DEBUG}
  CCoO     = -Fo
  AR       = LIB
  ARFLAGS  = -nologo
  ARoO     = -OUT:
  OEXT     = obj
  LEXT     = lib
  TOOLBOX  = MSVC
else
  ifeq (${DLABPRO_USE_MSVC},2)
    ## - MSVC 6.0 - 32-Bit C/C++-Compiler for x86
    CC       = CL
    CFLAGS   = -nologo -Od -Gm -EHsc -RTC1  -ZI ${DLABPRO_MSVC_FLAGS_DEBUG}
    CCoO     = -Fo
    AR       = LIB
    ARFLAGS  = -nologo
    ARoO     = -OUT:
    OEXT     = obj
    LEXT     = lib
    TOOLBOX  = MSVC6
  else
    ANSI=$(if $(findstring mingw,$(shell gcc -dumpmachine)),,-ansi)
    ## - GCC
    CC       = gcc
    #CFLAGS   = -g -D_DEBUG -Wall $(ANSI) ${DLABPRO_GCC_CFLAGS_DEBUG}
    CFLAGS   = -g ${DLABPRO_GCC_CFLAGS_DEBUG} -Wno-write-strings
    CCoO     = -o
    AR       = ar
    ARFLAGS  = rvs
    ARoO     =
    OEXT     = o
    LEXT     = a
    TOOLBOX  = GCC
  endif
endif

## Configuration - DEBUG (default)
LIB_PATH = ../../lib.debug${MEXT}
OBJ_PATH = ../../obj.debug${MEXT}

ifeq ($(MAKECMDGOALS),)
  MAKECMDGOALS = DEBUG
endif

## Configuration - RELEASE
ifeq ($(MAKECMDGOALS),RELEASE)
  LIB_PATH = ../../lib.release${MEXT}
  OBJ_PATH = ../../obj.release${MEXT}
  ifeq (${DLABPRO_USE_MSVC},1)
    CFLAGS  = -nologo -O2 -GL -D_RELEASE -EHsc -W3 -Wp64 -D_CRT_SECURE_NO_WARNINGS ${DLABPRO_MSVC_FLAGS_RELEASE}
    ARFLAGS = -nologo -LTCG
  else
    ifeq (${DLABPRO_USE_MSVC},2)
      ## - MSVC 6.0 - 32-Bit C/C++-Compiler for x86
      CFLAGS  = -nologo -O2 -D_RELEASE -EHsc -W3 -D_CRT_SECURE_NO_WARNINGS ${DLABPRO_MSVC_FLAGS_RELEASE}
    else
      #CFLAGS  = -O2 -D_RELEASE -Wall $(ANSI) ${DLABPRO_GCC_CFLAGS_RELEASE}
      CFLAGS   = -O2 ${DLABPRO_GCC_CFLAGS_RELEASE} -Wno-unused-variable -Wno-implicit-int -Wno-return-type -Wno-parentheses -Wno-sign-compare -Wno-format -Wno-implicit-function-declaration
    endif
  endif
endif

## Configuration - clean_release
ifeq ($(MAKECMDGOALS),clean_release)
  LIB_PATH = ../../lib.release${MEXT}
  OBJ_PATH = ../../obj.release${MEXT}
endif

## Target settings
LIBRARY        = $(LIB_PATH)/clapack.$(LEXT)
SHARED_LIBRARY = clapack.so
OBJECTS        = \
	$(OBJ_PATH)/backspace.$(OEXT) \
	$(OBJ_PATH)/c_div.$(OEXT)     \
	$(OBJ_PATH)/cdotc.$(OEXT)     \
	$(OBJ_PATH)/cgemm.$(OEXT)     \
	$(OBJ_PATH)/cgemv.$(OEXT)     \
	$(OBJ_PATH)/cgeru.$(OEXT)     \
	$(OBJ_PATH)/cgetf2.$(OEXT)    \
	$(OBJ_PATH)/cgetrf.$(OEXT)    \
	$(OBJ_PATH)/cgetri.$(OEXT)    \
	$(OBJ_PATH)/cgetrs.$(OEXT)    \
	$(OBJ_PATH)/cherk.$(OEXT)     \
	$(OBJ_PATH)/clacgv.$(OEXT)    \
	$(OBJ_PATH)/clacpy.$(OEXT)    \
	$(OBJ_PATH)/claswp.$(OEXT)    \
	$(OBJ_PATH)/close.$(OEXT)     \
	$(OBJ_PATH)/cpotf2.$(OEXT)    \
	$(OBJ_PATH)/cpotrf.$(OEXT)    \
	$(OBJ_PATH)/cscal.$(OEXT)     \
	$(OBJ_PATH)/csscal.$(OEXT)    \
	$(OBJ_PATH)/cswap.$(OEXT)     \
	$(OBJ_PATH)/ctrmm.$(OEXT)     \
	$(OBJ_PATH)/ctrmv.$(OEXT)     \
	$(OBJ_PATH)/ctrsm.$(OEXT)     \
	$(OBJ_PATH)/ctrti2.$(OEXT)    \
	$(OBJ_PATH)/ctrtri.$(OEXT)    \
	$(OBJ_PATH)/d_cnjg.$(OEXT)    \
	$(OBJ_PATH)/d_lg10.$(OEXT)    \
	$(OBJ_PATH)/d_sign.$(OEXT)    \
	$(OBJ_PATH)/daxpy.$(OEXT)     \
	$(OBJ_PATH)/dcabs1.$(OEXT)    \
	$(OBJ_PATH)/dcopy.$(OEXT)     \
	$(OBJ_PATH)/ddot.$(OEXT)      \
	$(OBJ_PATH)/dgebal.$(OEXT)    \
	$(OBJ_PATH)/dgemm.$(OEXT)     \
	$(OBJ_PATH)/dgemv.$(OEXT)     \
	$(OBJ_PATH)/dger.$(OEXT)      \
	$(OBJ_PATH)/dgetf2.$(OEXT)    \
	$(OBJ_PATH)/dgetrf.$(OEXT)    \
	$(OBJ_PATH)/dgetri.$(OEXT)    \
	$(OBJ_PATH)/dgetrs.$(OEXT)    \
	$(OBJ_PATH)/dhseqr.$(OEXT)    \
	$(OBJ_PATH)/dlabad.$(OEXT)    \
	$(OBJ_PATH)/dlacpy.$(OEXT)    \
	$(OBJ_PATH)/dlae2.$(OEXT)     \
	$(OBJ_PATH)/dlaev2.$(OEXT)    \
	$(OBJ_PATH)/dlahqr.$(OEXT)    \
	$(OBJ_PATH)/dlamch.$(OEXT)    \
	$(OBJ_PATH)/dlanhs.$(OEXT)    \
	$(OBJ_PATH)/dlanst.$(OEXT)    \
	$(OBJ_PATH)/dlansy.$(OEXT)    \
	$(OBJ_PATH)/dlanv2.$(OEXT)    \
	$(OBJ_PATH)/dlapy2.$(OEXT)    \
	$(OBJ_PATH)/dlarf.$(OEXT)     \
	$(OBJ_PATH)/dlarfb.$(OEXT)    \
	$(OBJ_PATH)/dlarfg.$(OEXT)    \
	$(OBJ_PATH)/dlarft.$(OEXT)    \
	$(OBJ_PATH)/dlarfx.$(OEXT)    \
	$(OBJ_PATH)/dlartg.$(OEXT)    \
	$(OBJ_PATH)/dlascl.$(OEXT)    \
	$(OBJ_PATH)/dlaset.$(OEXT)    \
	$(OBJ_PATH)/dlasr.$(OEXT)     \
	$(OBJ_PATH)/dlasrt.$(OEXT)    \
	$(OBJ_PATH)/dlassq.$(OEXT)    \
	$(OBJ_PATH)/dlaswp.$(OEXT)    \
	$(OBJ_PATH)/dlatrd.$(OEXT)    \
	$(OBJ_PATH)/dnrm2.$(OEXT)     \
	$(OBJ_PATH)/dorg2l.$(OEXT)    \
	$(OBJ_PATH)/dorg2r.$(OEXT)    \
	$(OBJ_PATH)/dorgl2.$(OEXT)    \
	$(OBJ_PATH)/dorgql.$(OEXT)    \
	$(OBJ_PATH)/dorgqr.$(OEXT)    \
	$(OBJ_PATH)/dorgtr.$(OEXT)    \
	$(OBJ_PATH)/dpotf2.$(OEXT)    \
	$(OBJ_PATH)/dpotrf.$(OEXT)    \
	$(OBJ_PATH)/drot.$(OEXT)      \
	$(OBJ_PATH)/dscal.$(OEXT)     \
	$(OBJ_PATH)/dsteqr.$(OEXT)    \
	$(OBJ_PATH)/dsterf.$(OEXT)    \
	$(OBJ_PATH)/dswap.$(OEXT)     \
	$(OBJ_PATH)/dsyev.$(OEXT)     \
	$(OBJ_PATH)/dsymv.$(OEXT)     \
	$(OBJ_PATH)/dsyr2.$(OEXT)     \
	$(OBJ_PATH)/dsyr2k.$(OEXT)    \
	$(OBJ_PATH)/dsyrk.$(OEXT)     \
	$(OBJ_PATH)/dsytd2.$(OEXT)    \
	$(OBJ_PATH)/dsytrd.$(OEXT)    \
	$(OBJ_PATH)/dtrmm.$(OEXT)     \
	$(OBJ_PATH)/dtrmv.$(OEXT)     \
	$(OBJ_PATH)/dtrsm.$(OEXT)     \
	$(OBJ_PATH)/dtrti2.$(OEXT)    \
	$(OBJ_PATH)/dtrtri.$(OEXT)    \
	$(OBJ_PATH)/endfile.$(OEXT)   \
	$(OBJ_PATH)/err.$(OEXT)       \
	$(OBJ_PATH)/exit_.$(OEXT)     \
	$(OBJ_PATH)/F77_aloc.$(OEXT)  \
	$(OBJ_PATH)/fmt.$(OEXT)       \
	$(OBJ_PATH)/fmtlib.$(OEXT)    \
	$(OBJ_PATH)/icamax.$(OEXT)    \
	$(OBJ_PATH)/idamax.$(OEXT)    \
	$(OBJ_PATH)/ieeeck.$(OEXT)    \
	$(OBJ_PATH)/ilaenv.$(OEXT)    \
	$(OBJ_PATH)/isamax.$(OEXT)    \
	$(OBJ_PATH)/izamax.$(OEXT)    \
	$(OBJ_PATH)/lsame.$(OEXT)     \
	$(OBJ_PATH)/lsame.$(OEXT)     \
	$(OBJ_PATH)/open.$(OEXT)      \
	$(OBJ_PATH)/pow_di.$(OEXT)    \
	$(OBJ_PATH)/pow_ri.$(OEXT)    \
	$(OBJ_PATH)/r_cnjg.$(OEXT)    \
	$(OBJ_PATH)/r_imag.$(OEXT)    \
	$(OBJ_PATH)/r_lg10.$(OEXT)    \
	$(OBJ_PATH)/r_sign.$(OEXT)    \
	$(OBJ_PATH)/s_cat.$(OEXT)     \
	$(OBJ_PATH)/s_cmp.$(OEXT)     \
	$(OBJ_PATH)/s_copy.$(OEXT)    \
	$(OBJ_PATH)/s_stop.$(OEXT)    \
	$(OBJ_PATH)/saxpy.$(OEXT)     \
	$(OBJ_PATH)/scopy.$(OEXT)     \
	$(OBJ_PATH)/sdot.$(OEXT)      \
	$(OBJ_PATH)/sfe.$(OEXT)       \
	$(OBJ_PATH)/sgebal.$(OEXT)    \
	$(OBJ_PATH)/sgemm.$(OEXT)     \
	$(OBJ_PATH)/sgemv.$(OEXT)     \
	$(OBJ_PATH)/sger.$(OEXT)      \
	$(OBJ_PATH)/sgetf2.$(OEXT)    \
	$(OBJ_PATH)/sgetrf.$(OEXT)    \
	$(OBJ_PATH)/sgetri.$(OEXT)    \
	$(OBJ_PATH)/sgetrs.$(OEXT)    \
	$(OBJ_PATH)/shseqr.$(OEXT)    \
	$(OBJ_PATH)/sig_die.$(OEXT)   \
	$(OBJ_PATH)/slabad.$(OEXT)    \
	$(OBJ_PATH)/slacpy.$(OEXT)    \
	$(OBJ_PATH)/slae2.$(OEXT)     \
	$(OBJ_PATH)/slaev2.$(OEXT)    \
	$(OBJ_PATH)/slahqr.$(OEXT)    \
	$(OBJ_PATH)/slamch.$(OEXT)    \
	$(OBJ_PATH)/slanhs.$(OEXT)    \
	$(OBJ_PATH)/slanst.$(OEXT)    \
	$(OBJ_PATH)/slansy.$(OEXT)    \
	$(OBJ_PATH)/slanv2.$(OEXT)    \
	$(OBJ_PATH)/slapy2.$(OEXT)    \
	$(OBJ_PATH)/slarf.$(OEXT)     \
	$(OBJ_PATH)/slarfb.$(OEXT)    \
	$(OBJ_PATH)/slarfg.$(OEXT)    \
	$(OBJ_PATH)/slarft.$(OEXT)    \
	$(OBJ_PATH)/slarfx.$(OEXT)    \
	$(OBJ_PATH)/slartg.$(OEXT)    \
	$(OBJ_PATH)/slascl.$(OEXT)    \
	$(OBJ_PATH)/slaset.$(OEXT)    \
	$(OBJ_PATH)/slasr.$(OEXT)     \
	$(OBJ_PATH)/slasrt.$(OEXT)    \
	$(OBJ_PATH)/slassq.$(OEXT)    \
	$(OBJ_PATH)/slaswp.$(OEXT)    \
	$(OBJ_PATH)/slatrd.$(OEXT)    \
	$(OBJ_PATH)/snrm2.$(OEXT)     \
	$(OBJ_PATH)/sorg2l.$(OEXT)    \
	$(OBJ_PATH)/sorg2r.$(OEXT)    \
	$(OBJ_PATH)/sorgl2.$(OEXT)    \
	$(OBJ_PATH)/sorgql.$(OEXT)    \
	$(OBJ_PATH)/sorgqr.$(OEXT)    \
	$(OBJ_PATH)/sorgtr.$(OEXT)    \
	$(OBJ_PATH)/spotf2.$(OEXT)    \
	$(OBJ_PATH)/spotrf.$(OEXT)    \
	$(OBJ_PATH)/srot.$(OEXT)      \
	$(OBJ_PATH)/sscal.$(OEXT)     \
	$(OBJ_PATH)/ssteqr.$(OEXT)    \
	$(OBJ_PATH)/ssterf.$(OEXT)    \
	$(OBJ_PATH)/sswap.$(OEXT)     \
	$(OBJ_PATH)/ssyev.$(OEXT)     \
	$(OBJ_PATH)/ssymv.$(OEXT)     \
	$(OBJ_PATH)/ssyr2.$(OEXT)     \
	$(OBJ_PATH)/ssyr2k.$(OEXT)    \
	$(OBJ_PATH)/ssyrk.$(OEXT)     \
	$(OBJ_PATH)/ssytd2.$(OEXT)    \
	$(OBJ_PATH)/ssytrd.$(OEXT)    \
	$(OBJ_PATH)/strmm.$(OEXT)     \
	$(OBJ_PATH)/strmv.$(OEXT)     \
	$(OBJ_PATH)/strsm.$(OEXT)     \
	$(OBJ_PATH)/strti2.$(OEXT)    \
	$(OBJ_PATH)/strtri.$(OEXT)    \
	$(OBJ_PATH)/util.$(OEXT)      \
	$(OBJ_PATH)/wref.$(OEXT)      \
	$(OBJ_PATH)/wrtfmt.$(OEXT)    \
	$(OBJ_PATH)/wsfe.$(OEXT)      \
	$(OBJ_PATH)/xerbla.$(OEXT)    \
	$(OBJ_PATH)/xerbla.$(OEXT)    \
	$(OBJ_PATH)/z_div.$(OEXT)     \
	$(OBJ_PATH)/zdotc.$(OEXT)     \
	$(OBJ_PATH)/zdscal.$(OEXT)    \
	$(OBJ_PATH)/zgemm.$(OEXT)     \
	$(OBJ_PATH)/zgemv.$(OEXT)     \
	$(OBJ_PATH)/zgeru.$(OEXT)     \
	$(OBJ_PATH)/zgetf2.$(OEXT)    \
	$(OBJ_PATH)/zgetrf.$(OEXT)    \
	$(OBJ_PATH)/zgetri.$(OEXT)    \
	$(OBJ_PATH)/zgetrs.$(OEXT)    \
	$(OBJ_PATH)/zherk.$(OEXT)     \
	$(OBJ_PATH)/zlacgv.$(OEXT)    \
	$(OBJ_PATH)/zlacpy.$(OEXT)    \
	$(OBJ_PATH)/zlaswp.$(OEXT)    \
	$(OBJ_PATH)/zpotf2.$(OEXT)    \
	$(OBJ_PATH)/zpotrf.$(OEXT)    \
	$(OBJ_PATH)/zscal.$(OEXT)     \
	$(OBJ_PATH)/zswap.$(OEXT)     \
	$(OBJ_PATH)/ztrmm.$(OEXT)     \
	$(OBJ_PATH)/ztrmv.$(OEXT)     \
	$(OBJ_PATH)/ztrsm.$(OEXT)     \
	$(OBJ_PATH)/ztrti2.$(OEXT)    \
	$(OBJ_PATH)/ztrtri.$(OEXT)

DEPS           = blaswrap.h f2c.h

## Build rules
DEBUG   : ECHOCNF MKDIR $(LIBRARY)
RELEASE : ECHOCNF MKDIR $(LIBRARY)
SHARED  : ECHOCNF MKDIR $(SHARED_LIBRARY)

$(LIBRARY): $(OBJECTS)
	$(AR) $(ARFLAGS) $(ARoO)$(LIBRARY) $(OBJECTS)

$(SHARED_LIBRARY): $(OBJECTS)
	$(CC) -shared -Wl,-soname,$(SHARED_LIBRARY).0 $(OBJECTS) \
          $(CCoO)$(LIB_PATH)/$(SHARED_LIBRARY).0.0
	-cd $(LIB_PATH) && ln -sf $(SHARED_LIBRARY).0.0 $(SHARED_LIBRARY).0 \
          && ln -sf $(SHARED_LIBRARY).0 $(SHARED_LIBRARY) 

$(OBJ_PATH)/%.$(OEXT): %.c $(DEPS)
	$(CC) -include dlp_defines_extern.h -c $(CFLAGS) $(INCL) $(CCoO) $@ $<

.PHONY: ECHOCNF MKDIR clean clean_debug clean_release

ECHOCNF:
	@echo
	@echo '// ----- Make ($(TOOLBOX)): External library clapack -- $(MAKECMDGOALS) -----'

MKDIR:
	@-test -w $(OBJ_PATH) || mkdir $(OBJ_PATH)
	@-test -w $(LIB_PATH) || mkdir $(LIB_PATH)

clean:  clean_debug

clean_debug:
	@echo '// ----- Make ($(TOOLBOX)): External library clapack -- cleaning DEBUG -----'
	-rm -f $(OBJECTS) $(LIBRARY)
	-rm -f vc80.?db

clean_release:
	@echo '// ----- Make ($(TOOLBOX)): External library clapack -- cleaning RELEASE -----'
	-rm -f $(OBJECTS) $(LIBRARY)
	-rm -f vc80.?db

## EOF

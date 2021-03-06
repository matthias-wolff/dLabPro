## External library clapack
## - Makefile
##
## AUTHOR : Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd., Courant Institute, Argonne National Lab, and Rice Universit, Guntram Strecha (Makefile)
## PACKAGE: dLabPro/ext
##
## LICENSE: clapack is free for any purpose. See ./README for details.

ifeq (${DLABPRO_HOME},)
  DLABPRO_HOME = ../..
endif

PROJNAME=clapack

CFLAGS += -include dlp_defines_extern.h
CFLAGS_FILTEROUT += -Wall -ansi

## Target settings
SEXT    = c
SOURCES = \
	backspace \
	c_div     \
	cdotc     \
	cgemm     \
	cgemv     \
	cgeru     \
	cgetf2    \
	cgetrf    \
	cgetri    \
	cgetrs    \
	cherk     \
	clacgv    \
	clacpy    \
	claswp    \
	close     \
	cpotf2    \
	cpotrf    \
	cscal     \
	csscal    \
	cswap     \
	ctrmm     \
	ctrmv     \
	ctrsm     \
	ctrti2    \
	ctrtri    \
	d_cnjg    \
	d_lg10    \
	d_sign    \
	daxpy     \
	dcabs1    \
	dcopy     \
	ddot      \
	dgebal    \
	dgemm     \
	dgemv     \
	dger      \
	dgetf2    \
	dgetrf    \
	dgetri    \
	dgetrs    \
	dhseqr    \
	dlabad    \
	dlacpy    \
	dlae2     \
	dlaev2    \
	dlahqr    \
	dlamch    \
	dlanhs    \
	dlanst    \
	dlansy    \
	dlanv2    \
	dlapy2    \
	dlarf     \
	dlarfb    \
	dlarfg    \
	dlarft    \
	dlarfx    \
	dlartg    \
	dlascl    \
	dlaset    \
	dlasr     \
	dlasrt    \
	dlassq    \
	dlaswp    \
	dlatrd    \
	dnrm2     \
	dorg2l    \
	dorg2r    \
	dorgl2    \
	dorgql    \
	dorgqr    \
	dorgtr    \
	dpotf2    \
	dpotrf    \
	drot      \
	dscal     \
	dsteqr    \
	dsterf    \
	dswap     \
	dsyev     \
	dsymv     \
	dsyr2     \
	dsyr2k    \
	dsyrk     \
	dsytd2    \
	dsytrd    \
	dtrmm     \
	dtrmv     \
	dtrsm     \
	dtrti2    \
	dtrtri    \
	endfile   \
	err       \
	exit_     \
	F77_aloc  \
	fmt       \
	fmtlib    \
	icamax    \
	idamax    \
	ieeeck    \
	ilaenv    \
	isamax    \
	izamax    \
	lsame     \
	lsame     \
	open      \
	pow_di    \
	pow_ri    \
	r_cnjg    \
	r_imag    \
	r_lg10    \
	r_sign    \
	s_cat     \
	s_cmp     \
	s_copy    \
	s_stop    \
	saxpy     \
	scopy     \
	sdot      \
	sfe       \
	sgebal    \
	sgemm     \
	sgemv     \
	sger      \
	sgetf2    \
	sgetrf    \
	sgetri    \
	sgetrs    \
	shseqr    \
	sig_die   \
	slabad    \
	slacpy    \
	slae2     \
	slaev2    \
	slahqr    \
	slamch    \
	slanhs    \
	slanst    \
	slansy    \
	slanv2    \
	slapy2    \
	slarf     \
	slarfb    \
	slarfg    \
	slarft    \
	slarfx    \
	slartg    \
	slascl    \
	slaset    \
	slasr     \
	slasrt    \
	slassq    \
	slaswp    \
	slatrd    \
	snrm2     \
	sorg2l    \
	sorg2r    \
	sorgl2    \
	sorgql    \
	sorgqr    \
	sorgtr    \
	spotf2    \
	spotrf    \
	srot      \
	sscal     \
	ssteqr    \
	ssterf    \
	sswap     \
	ssyev     \
	ssymv     \
	ssyr2     \
	ssyr2k    \
	ssyrk     \
	ssytd2    \
	ssytrd    \
	strmm     \
	strmv     \
	strsm     \
	strti2    \
	strtri    \
	util      \
	wref      \
	wrtfmt    \
	wsfe      \
	xerbla    \
	xerbla    \
	z_div     \
	zdotc     \
	zdscal    \
	zgemm     \
	zgemv     \
	zgeru     \
	zgetf2    \
	zgetrf    \
	zgetri    \
	zgetrs    \
	zherk     \
	zlacgv    \
	zlacpy    \
	zlaswp    \
	zpotf2    \
	zpotrf    \
	zscal     \
	zswap     \
	ztrmm     \
	ztrmv     \
	ztrsm     \
	ztrti2    \
	ztrtri
DEPS = blaswrap.h f2c.h

include $(DLABPRO_HOME)/make/ext.mk

## EOF

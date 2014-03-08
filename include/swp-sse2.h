/*  SSE2 interface library
 *  Copyright (C) 2002 Rainer Schaffer
 *  
 *  This depends on sse.h which should have been included.
 *
 */ 

#ifndef _SSE2_H
#define _SSE2_H

#include "swp-sse.h"

int sse2_ok( void );

/*  Function to test if sse instructions are supported...
*/
/* inline extern int */
int
sse2_ok( void )
{
  /* Returns 1 if SSE2 instructions are supported, 0 otherwise */
  return ( (mm_support() & 0x8) >> 3  );
}

/*  Helper functions for the instruction macros that follow...
  (note that memory-to-register, m2r, instructions are nearly
   as efficient as register-to-register, r2r, instructions;
   however, memory-to-memory instructions are really simulated
   as a convenience, and are only 1/3 as efficient)
*/

/*  1x128 MOVe Aligned four Packed Double-fp
*/
#define  movapd_m2r(var, reg)  sse_m2r(movapd, var, reg)
#define  movapd_r2m(reg, var)  sse_r2m(movapd, reg, var)
#define  movapd_r2r(regs, regd)  sse_r2r(movapd, regs, regd)
#define  movapd(vars, vard) \
  __asm__ __volatile__ ("movapd %1, %%mm0\n\t" \
            "movapd %%mm0, %0" \
            : "=X" (vard) \
            : "X" (vars))


/*  1x128 MOVe aligned Non-Temporal four Packed Double-fp
*/
#define  movntpd_r2m(xmmreg, var)  sse_r2m(movntpd, xmmreg, var)


/*  1x32 MOVe Non-Temporal Doubleword (r32)
*/
#define  movnti_r2m(reg32, var)    sse_r2m(movnti, reg32, var)


/*  1x128 MOVe Unaligned four Packed Double-fp
*/
#define  movupd_m2r(var, reg)  sse_m2r(movupd, var, reg)
#define  movupd_r2m(reg, var)  sse_r2m(movupd, reg, var)
#define  movupd_r2r(regs, regd)  sse_r2r(movupd, regs, regd)
#define  movupd(vars, vard) \
  __asm__ __volatile__ ("movupd %1, %%mm0\n\t" \
            "movupd %%mm0, %0" \
            : "=X" (vard) \
            : "X" (vars))

/*  MOVe High to Low Packed Double-fp
  high half of 2x64f (x) -> low half of 2x64f (y)
*/
#define  movhlpd_r2r(regs, regd)  sse_r2r(movhlpd, regs, regd)


/*  MOVe Low to High Packed Double-fp
  low half of 2x64f (x) -> high half of 2x64f (y)
*/
#define  movlhpd_r2r(regs, regd)  sse_r2r(movlhpd, regs, regd)


/*  MOVe High Packed Double-fp
  1x64f -> high half of 2x64f
*/
#define  movhpd_m2r(var, reg)  sse_m2r(movhpd, var, reg)
#define  movhpd_r2m(reg, var)  sse_r2m(movhpd, reg, var)
#define  movhpd(vars, vard) \
  __asm__ __volatile__ ("movhpd %1, %%mm0\n\t" \
            "movhpd %%mm0, %0" \
            : "=X" (vard) \
            : "X" (vars))

/*  MOVe Low Packed Double-fp
  1x64f -> low half of 2x64f
*/
#define  movlpd_m2r(var, reg)  sse_m2r(movlpd, var, reg)
#define  movlpd_r2m(reg, var)  sse_r2m(movlpd, reg, var)
#define  movlpd(vars, vard) \
  __asm__ __volatile__ ("movlpd %1, %%mm0\n\t" \
            "movlpd %%mm0, %0" \
            : "=X" (vard) \
            : "X" (vars))

/*  MOVe Scalar Double-fp
  lowest field of 2x64f (x) -> lowest field of 2x64f (y)
*/
#define  movsd_m2r(var, reg)  sse_m2r(movsd, var, reg)
#define  movsd_r2m(reg, var)  sse_r2m(movsd, reg, var)
#define  movsd_r2r(regs, regd)  sse_r2r(movsd, regs, regd)
#define  movsd(vars, vard) \
  __asm__ __volatile__ ("movsd %1, %%mm0\n\t" \
            "movsd %%mm0, %0" \
            : "=X" (vard) \
            : "X" (vars))

/*  4x16 Packed SHUFfle Word low half of 8x16
*/
#define  pshuflw_m2r(var, reg, index)  sse_m2ri(pshuflw, var, reg, index)
#define  pshuflw_r2r(regs, regd, index)  sse_r2ri(pshuflw, regs, regd, index)

/*  4x16 Packed SHUFfle Word high half of 8x16
*/
#define  pshufhw_m2r(var, reg, index)  sse_m2ri(pshufhw, var, reg, index)
#define  pshufhw_r2r(regs, regd, index)  sse_r2ri(pshufhw, regs, regd, index)

/*  4x32 Packed SHUFfle Doubleword
*/
#define  pshufpd_m2r(var, reg, index)  sse_m2ri(pshufpd, var, reg, index)
#define  pshufpd_r2r(regs, regd, index)  sse_r2ri(pshufpd, regs, regd, index)


/*  ConVerT Packed Signed Int32 to(2) two Packed signed Single-fp  
*/
#define  cvtdq2ps_m2r(var, xmmreg)  sse_m2r(cvtdq2ps, var, xmmreg)
#define  cvtdq2ps_r2r(regs, regd)  sse_r2r(cvtdq2ps, regs, regd)

/*  ConVerT Packed signed Single-fp to(2) Packed Signed Int32 
*/
#define  cvtps2dq_m2r(var, xmmreg)  sse_m2r(cvtps2dq, var, xmmreg)
#define  cvtps2dq_r2r(regs, regd)  sse_r2r(cvtps2dq, regs, regd)

/*  ConVerT Packed signed Double-fp to(2) Packed Single-fp
*/
#define  cvtpd2ps_m2r(var, xmmreg)  sse_m2r(cvtpd2ps, var, xmmreg)
#define  cvtpd2ps_r2r(regs, regd)  sse_r2r(cvtpd2ps, regs, regd)

/*  ConVerT Packed signed Single-fp to(2) Packed Double-fp
*/
#define  cvtps2pd_m2r(var, xmmreg)  sse_m2r(cvtps2pd, var, xmmreg)
#define  cvtps2pd_r2r(regs, regd)  sse_r2r(cvtps2pd, regs, regd)

/*  ConVerT Packed two signed Int32 to(2) two Packed Double-fp
*/
#define  cvtpi2pd_m2r(var, xmmreg)  sse_m2r(cvtpi2pd, var, xmmreg)
#define  cvtpi2pd_r2r(mmreg, xmmreg)  sse_r2r(cvtpi2pd, mmreg, xmmreg)

/*  ConVerT Packed Double-fp to(2) Packed signed Int32
*/
#define  cvtpd2pi_m2r(var, mmreg)  sse_m2r(cvtpd2pi, var, mmreg)
#define  cvtpd2pi_r2r(xmmreg, mmreg)  sse_r2r(cvtpd2pi, xmmreg, mmreg)

/*  ConVerT Double-fp to(2) Signed Int32  (Scalar)
*/
#define  cvtsd2si_m2r(var, xmmreg)  sse_m2r(cvtsi2ss, var, xmmreg)
#define  cvtsd2si_r2r(reg, xmmreg)  sse_r2r(cvtsi2ss, reg, xmmreg)

/*  ConVerT Single-fp to(2) Double-fp (Scalar)
*/
#define  cvtss2sd_m2r(var, xmmreg)  sse_m2r(cvtss2sd, var, xmmreg)
#define  cvtss2sd_r2r(reg, xmmreg)  sse_r2r(cvtss2sd, reg, xmmreg)

/*  ConVerT Double-fp to(2) Single-fp (Scalar)
*/
#define  cvtsd2ss_m2r(var, xmmreg)  sse_m2r(cvtsd2ss, var, xmmreg)
#define  cvtsd2ss_r2r(reg, xmmreg)  sse_r2r(cvtsd2ss, reg, xmmreg)


#define punpckhqdq_r2r(reg, xmmreg)      sse_r2r(punpckhqdq, reg, xmmreg)
#define punpcklqdq_r2r(reg, xmmreg)      sse_r2r(punpcklqdq, reg, xmmreg)

#define movq2dq_r2r()
#define movdq2q_r2r()


#define paddq_r2r()
#define psubq_r2r()
#define pmuludq_r2r()


/*  MOVe MaSK from Packed Double-fp
*/
#ifdef  SSE2_TRACE
  #define  movmskpd(xmmreg, reg) \
  { \
    fprintf(stderr, "movmskpd()\n"); \
    __asm__ __volatile__ ("movmskpd %" #xmmreg ", %" #reg) \
  }
#else
  #define  movmskpd(xmmreg, reg) \
  __asm__ __volatile__ ("movmskpd %" #xmmreg ", %" #reg)
#endif


/*  Parallel MOVe MaSK from xmm reg to 32-bit reg
*/
#ifdef  SSE2_TRACE
  #define  pmovmskpd(mmreg, reg) \
  { \
    fprintf(stderr, "movmskpd()\n"); \
    __asm__ __volatile__ ("movmskpd %" #mmreg ", %" #reg) \
  }
#else
  #define  pmovmskpd(mmreg, reg) \
  __asm__ __volatile__ ("movmskpd %" #mmreg ", %" #reg)
#endif


/*  MASKed MOVe -- Store selected Byts by Doubleword
*/
#define  maskmovdqu_r2r(regs, regd)  sse_r2r(maskmovdqu, regs, regd)




/*  2x64f Parallel ADDs
*/
#define  addpd_m2r(var, reg)    sse_m2r(addpd, var, reg)
#define  addpd_r2r(regs, regd)    sse_r2r(addpd, regs, regd)
#define  addpd(vars, vard, xmmreg)  sse_m2m(addpd, vars, vard, xmmreg)


/*  Lowest Field of 2x64f Parallel ADDs
*/
#define  addsd_m2r(var, reg)    sse_m2r(addsd, var, reg)
#define  addsd_r2r(regs, regd)    sse_r2r(addsd, regs, regd)
#define  addsd(vars, vard, xmmreg)  sse_m2m(addsd, vars, vard, xmmreg)


/*  2x64f Parallel SUBs
*/
#define  subpd_m2r(var, reg)    sse_m2r(subpd, var, reg)
#define  subpd_r2r(regs, regd)    sse_r2r(subpd, regs, regd)
#define  subpd(vars, vard, xmmreg)  sse_m2m(subpd, vars, vard, xmmreg)


/*  Lowest Field of 2x64f Parallel SUBs
*/
#define  subsd_m2r(var, reg)    sse_m2r(subsd, var, reg)
#define  subsd_r2r(regs, regd)    sse_r2r(subsd, regs, regd)
#define  subsd(vars, vard, xmmreg)  sse_m2m(subsd, vars, vard, xmmreg)

/*  2x64f Parallel MULs
*/
#define  mulpd_m2r(var, reg)    sse_m2r(mulpd, var, reg)
#define  mulpd_r2r(regs, regd)    sse_r2r(mulpd, regs, regd)
#define  mulpd(vars, vard, xmmreg)  sse_m2m(mulpd, vars, vard, xmmreg)


/*  Lowest Field of 2x64f Parallel MULs
*/
#define  mulsd_m2r(var, reg)    sse_m2r(mulsd, var, reg)
#define  mulsd_r2r(regs, regd)    sse_r2r(mulsd, regs, regd)
#define  mulsd(vars, vard, xmmreg)  sse_m2m(mulsd, vars, vard, xmmreg)

/*  2x64f Parallel DIVs
*/
#define  divpd_m2r(var, reg)    sse_m2r(divpd, var, reg)
#define  divpd_r2r(regs, regd)    sse_r2r(divpd, regs, regd)
#define  divpd(vars, vard, xmmreg)  sse_m2m(divpd, vars, vard, xmmreg)


/*  Lowest Field of 2x64f Parallel DIVs
*/
#define  divsd_m2r(var, reg)    sse_m2r(divsd, var, reg)
#define  divsd_r2r(regs, regd)    sse_r2r(divsd, regs, regd)
#define  divsd(vars, vard, xmmreg)  sse_m2m(divsd, vars, vard, xmmreg)

/*  2x64f Parallel Reciprocals
*/
#define  rcppd_m2r(var, reg)    sse_m2r(rcppd, var, reg)
#define  rcppd_r2r(regs, regd)    sse_r2r(rcppd, regs, regd)
#define  rcppd(vars, vard, xmmreg)  sse_m2m(rcppd, vars, vard, xmmreg)


/*  Lowest Field of 2x64f Parallel Reciprocals
*/
#define  rcpsd_m2r(var, reg)    sse_m2r(rcpsd, var, reg)
#define  rcpsd_r2r(regs, regd)    sse_r2r(rcpsd, regs, regd)
#define  rcpsd(vars, vard, xmmreg)  sse_m2m(rcpsd, vars, vard, xmmreg)

/*  2x64f Parallel Square Root of Reciprocals
*/
#define  rsqrtpd_m2r(var, reg)    sse_m2r(rsqrtpd, var, reg)
#define  rsqrtpd_r2r(regs, regd)    sse_r2r(rsqrtpd, regs, regd)
#define  rsqrtpd(vars, vard, xmmreg)  sse_m2m(rsqrtpd, vars, vard, xmmreg)


/*  Lowest Field of 2x64f Parallel Square Root of Reciprocals
*/
#define  rsqrtsd_m2r(var, reg)    sse_m2r(rsqrtsd, var, reg)
#define  rsqrtsd_r2r(regs, regd)    sse_r2r(rsqrtsd, regs, regd)
#define  rsqrtsd(vars, vard, xmmreg)  sse_m2m(rsqrtsd, vars, vard, xmmreg)

/*  2x64f Parallel Square Roots
*/
#define  sqrtpd_m2r(var, reg)    sse_m2r(sqrtpd, var, reg)
#define  sqrtpd_r2r(regs, regd)    sse_r2r(sqrtpd, regs, regd)
#define  sqrtpd(vars, vard, xmmreg)  sse_m2m(sqrtpd, vars, vard, xmmreg)


/*  Lowest Field of 2x64f Parallel Square Roots
*/
#define  sqrtsd_m2r(var, reg)    sse_m2r(sqrtsd, var, reg)
#define  sqrtsd_r2r(regs, regd)    sse_r2r(sqrtsd, regs, regd)
#define  sqrtsd(vars, vard, xmmreg)  sse_m2m(sqrtsd, vars, vard, xmmreg)



/*  1x128 bitwise AND
*/
#define  andpd_m2r(var, reg)    sse_m2r(andpd, var, reg)
#define  andpd_r2r(regs, regd)    sse_r2r(andpd, regs, regd)
#define  andpd(vars, vard, xmmreg)  sse_m2m(andpd, vars, vard, xmmreg)


/*  1x128 bitwise AND with Not the destination
*/
#define  andnpd_m2r(var, reg)    sse_m2r(andnpd, var, reg)
#define  andnpd_r2r(regs, regd)    sse_r2r(andnpd, regs, regd)
#define  andnpd(vars, vard, xmmreg)  sse_m2m(andnpd, vars, vard, xmmreg)


/*  1x128 bitwise OR
*/
#define  orpd_m2r(var, reg)    sse_m2r(orpd, var, reg)
#define  orpd_r2r(regs, regd)    sse_r2r(orpd, regs, regd)
#define  orpd(vars, vard, xmmreg)  sse_m2m(orpd, vars, vard, xmmreg)


/*  1x128 bitwise eXclusive OR
*/
#define  xorpd_m2r(var, reg)    sse_m2r(xorpd, var, reg)
#define  xorpd_r2r(regs, regd)    sse_r2r(xorpd, regs, regd)
#define  xorpd(vars, vard, xmmreg)  sse_m2m(xorpd, vars, vard, xmmreg)


/*  2x64f Parallel Maximum
*/
#define  maxpd_m2r(var, reg)    sse_m2r(maxpd, var, reg)
#define  maxpd_r2r(regs, regd)    sse_r2r(maxpd, regs, regd)
#define  maxpd(vars, vard, xmmreg)  sse_m2m(maxpd, vars, vard, xmmreg)


/*  Lowest Field of 2x64f Parallel Maximum
*/
#define  maxsd_m2r(var, reg)    sse_m2r(maxsd, var, reg)
#define  maxsd_r2r(regs, regd)    sse_r2r(maxsd, regs, regd)
#define  maxsd(vars, vard, xmmreg)  sse_m2m(maxsd, vars, vard, xmmreg)


/*  2x64f Parallel Minimum
*/
#define  minpd_m2r(var, reg)    sse_m2r(minpd, var, reg)
#define  minpd_r2r(regs, regd)    sse_r2r(minpd, regs, regd)
#define  minpd(vars, vard, xmmreg)  sse_m2m(minpd, vars, vard, xmmreg)


/*  Lowest Field of 2x64f Parallel Minimum
*/
#define  minsd_m2r(var, reg)    sse_m2r(minsd, var, reg)
#define  minsd_r2r(regs, regd)    sse_r2r(minsd, regs, regd)
#define  minsd(vars, vard, xmmreg)  sse_m2m(minsd, vars, vard, xmmreg)


/*  2x64f Parallel CoMPares
  (resulting fields are either 0 or -1)
*/
#define  cmppd_m2r(var, reg, op)    sse_m2ri(cmppd, var, reg, op)
#define  cmppd_r2r(regs, regd, op)  sse_r2ri(cmppd, regs, regd, op)
#define  cmppd(vars, vard, op, xmmreg)  sse_m2mi(cmppd, vars, vard, xmmreg, op)

/*  Lowest Field of 2x64f Parallel CoMPares
  (resulting fields are either 0 or -1)
*/
#define  cmpsd_m2r(var, reg, op)    sse_m2ri(cmpsd, var, reg, op)
#define  cmpsd_r2r(regs, regd, op)  sse_r2ri(cmpsd, regs, regd, op)
#define  cmpsd(vars, vard, op, xmmreg)  sse_m2mi(cmpsd, vars, vard, xmmreg, op)

/*  Lowest Field of 2x64f Parallel CoMPares to set EFLAGS
  (resulting fields are either 0 or -1)
*/
#define  comisd_m2r(var, reg)    sse_m2r(comisd, var, reg)
#define  comisd_r2r(regs, regd)    sse_r2r(comisd, regs, regd)
#define  comisd(vars, vard, xmmreg)  sse_m2m(comisd, vars, vard, xmmreg)


/*  Lowest Field of 4x32f Unordered Parallel CoMPares to set EFLAGS
  (resulting fields are either 0 or -1)
*/
#define  ucomisd_m2r(var, reg)    sse_m2r(ucomisd, var, reg)
#define  ucomisd_r2r(regs, regd)    sse_r2r(ucomisd, regs, regd)
#define  ucomisd(vars, vard, xmmreg)  sse_m2m(ucomisd, vars, vard, xmmreg)


#endif /* _SSE2_H */

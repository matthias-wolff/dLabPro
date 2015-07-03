/*  mmx.h

  MultiMedia eXtensions GCC interface library for IA32.

  To use this library, simply include this header file
  and compile with GCC.  You MUST have inlining enabled
  in order for mmx_ok() to work; this can be done by
  simply using -O on the GCC command line.

  Compiling with -DMMX_TRACE will cause detailed trace
  output to be sent to stderr for each mmx operation.
  This adds lots of code, and obviously slows execution to
  a crawl, but can be very useful for debugging.

  THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY
  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, WITHOUT
  LIMITATION, THE IMPLIED WARRANTIES OF MERCHANTABILITY
  AND FITNESS FOR ANY PARTICULAR PURPOSE.

  1997-98 by H. Dietz and R. Fisher

 History:
  97-98*  R.Fisher  Early versions
  980501  R.Fisher  Original Release
  980611*  H.Dietz    Rewrite, correctly implementing inlines, and
    R.Fisher   including direct register accesses.
  980616  R.Fisher  Release of 980611 as 980616.
  980714  R.Fisher  Minor corrections to Makefile, etc.
  980715  R.Fisher  mmx_ok() now prevents optimizer from using
         clobbered values.
        mmx_ok() now checks if cpuid instruction is
         available before trying to use it.
  980726*  R.Fisher  mm_support() searches for AMD 3DNow, Cyrix
         Extended MMX, and standard MMX.  It returns a
         value which is positive if any of these are
         supported, and can be masked with constants to
         see which.  mmx_ok() is now a call to this
  980726*  R.Fisher  Added i2r support for shift functions
  980919  R.Fisher  Fixed AMD extended feature recognition bug.
  980921  R.Fisher  Added definition/check for _MMX_H.
        Added "float s[2]" to mmx_t for use with
          3DNow and EMMX.  So same mmx_t can be used.

  * Unreleased (internal or interim) versions

 Notes:
  It appears that the latest gas has the pand problem fixed, therefore
    I'll undefine BROKEN_PAND by default.
  String compares may be quicker than the multiple test/jumps in vendor
    test sequence in mmx_ok(), but I'm not concerned with that right now.

 Acknowledgments:
  Jussi Laako for pointing out the errors ultimately found to be
    connected to the failure to notify the optimizer of clobbered values.
  Roger Hardiman for reminding us that CPUID isn't everywhere, and that
    someone may actually try to use this on a machine without CPUID.
    Also for suggesting code for checking this.
  Robert Dale for pointing out the AMD bug.
*/

#ifndef _MMX_H
#define _MMX_H


/*  Warning:  at this writing, the version of GAS packaged
  with most Linux distributions does not handle the
  parallel AND operation mnemonic correctly.  If the
  symbol BROKEN_PAND is defined, a slower alternative
  coding will be used.  If execution of mmxtest results
  in an illegal instruction fault, define this symbol.
*/
#undef  BROKEN_PAND


/*  The type of an value that fits in an MMX register
  (note that long long constant values MUST be suffixed
   by LL and unsigned long long values by ULL, lest
   they be truncated by the compiler)
*/

#ifdef WIN32
typedef  union {
  __int64      q;  /* Quadword (64-bit) value */
  unsigned __int64  uq;  /* Unsigned Quadword */
  int      d[2];  /* 2 Doubleword (32-bit) values */
  unsigned int    ud[2];  /* 2 Unsigned Doubleword */
  short      w[4];  /* 4 Word (16-bit) values */
  uint16_t    uw[4];  /* 4 Unsigned Word */
  char      b[8];  /* 8 Byte (8-bit) values */
  unsigned char    ub[8];  /* 8 Unsigned Byte */
  float      s[2];  /* Single-precision (32-bit) value */
} mmx_t;
#else
typedef  union {
  long long    q;  /* Quadword (64-bit) value */
  unsigned long long  uq;  /* Unsigned Quadword */
  int      d[2];  /* 2 Doubleword (32-bit) values */
  unsigned int    ud[2];  /* 2 Unsigned Doubleword */
  short      w[4];  /* 4 Word (16-bit) values */
  uint16_t    uw[4];  /* 4 Unsigned Word */
  char      b[8];  /* 8 Byte (8-bit) values */
  unsigned char    ub[8];  /* 8 Unsigned Byte */
  float      s[2];  /* Single-precision (32-bit) value */
} mmx_t;
#endif

int mm_support( void );
int mmx_ok( void );

/*  Function to test if multimedia instructions are supported...
*/
int 
mm_support( void )
{
  volatile int flag0 = 0;
  volatile int flag = 0;
  /* mmx = 1, 
     mmxExt = 2, 
     sse = 4,
     sse2 = 8, 
     threednow = 16, 
     threednowExt = 32, 
     threednowProf = 64;
     FXSR = 128;
  */

  /* Befehlstest */
#ifdef WIN32
 __asm{
  pushfd ;
  pop eax ;
  mov ecx, eax ;
  xor eax, 0x200000 ;
  push eax ;
  popfd ;
  pushfd ;
  pop eax ;
  xor eax, ebx ;
    mov flag0 , eax ;
  }
#else
  __asm__ __volatile__ ( "pushf\n\t"
       "popl %eax\n\t"
       "movl %eax, %ecx\n\t"
       "xorl $0x200000, %eax\n\t"
       "push %eax\n\t"
       "popf\n\t"
       "pushf\n\t"
       "popl %eax\n\t"
       "xor %ebx, %eax\n\t" );
  __asm__ __volatile__ ( "movl %%eax, %0" \
       : "=X" (flag0) \
       : /* nothing */ );
#endif

/*   printf( "Befehlstest: flag0 = %x\n", flag0 ); */
  if( flag0 == 0 ){
    printf( "Befehl 'cpuid' wird nicht unterstuetzt.\n" );
    printf( "flag0 = %x\n", flag0 );
#ifdef WIN32
    __asm emms ;
#else
    __asm__ __volatile__ ( "emms" );
#endif
    return( 0 );
  }
  
  /* Bestimmung des Herstellers */
#ifdef WIN32
  __asm{
  pusha;
  mov eax, 0; 
  cpuid;
  mov flag0, eax ;
    popa;
  }
#else
  __asm__ __volatile__ ( "pusha" );
  __asm__ __volatile__ ( "mov $0, %eax\n\t"
       "cpuid\n\t" );
  __asm__ __volatile__ ( "movl %%eax, %0" \
       : "=X" (flag0) \
       : /* nothing */ );
  __asm__ __volatile__ ( "popa" );
#endif

/*   printf( "Herstellertest: eax register: 0x%x\n", flag0 ); */

  /* Lies Standard-Flags */
  if( flag0 > 0 ){
#ifdef WIN32
  __asm {
  pusha ;
  mov eax, 1 ;
  cpuid ;
  mov flag0, edx ;
  popa ;
  }
#else
    __asm__ __volatile__ ( "pusha \n\t"
         "mov $1, %eax \n\t" 
         "cpuid \n\t" );
    __asm__ __volatile__ ( "mov %%edx, %0 \n\t" \
         : "=X" (flag0) \
         : /* nothing */ );
    __asm__ __volatile__ ("popa \n\t" );
#endif
/*     printf( "Standard-Flag register: %x\n", flag0 ); */
    if( ( flag0 | 0xff7fffff ) == 0xffffffff ) /* MMX */
      flag += 1;
    if( ( flag0 | 0xfeffffff ) == 0xffffffff ) /* FXSR */
      flag += 128;
    if( ( flag0 | 0xfdffffff ) == 0xffffffff ) /* SSE */
      flag += 4;
    if( ( flag0 | 0xfbffffff ) == 0xffffffff ) /* SSE2 */
      flag += 8;
  }

  /* Test Zusatz-Flags */
#ifdef WIN32
  __asm {
  pusha ;
  mov eax, 0x80000000 ;
  cpuid ;
  mov flag0, eax ;
  popa ;
  }
#else
  __asm__ __volatile__ ( "pusha \n\t"
       "mov $0x80000000, %eax \n\t"
       "cpuid \n\t" );
  __asm__ __volatile__ ( "mov %%eax, %0 \n\t" \
       : "=X" (flag0) \
       : /* nothing */ );
  __asm__ __volatile__ ( "popa \n\t" );
#endif

/*   printf( "\nInput eax register: 0x80000000\n" ); */
/*   printf( "output eax register: %x\n", flag0 ); */

  /* Lies Zusatz-Flags */ 
  if( flag0 > 0x80000000 ){
#ifdef WIN32
  __asm {
  pusha ;
  mov eax, 0x80000001 ;
  cpuid ;
  mov flag0, edx ;
  popa ;
  }
#else
    __asm__ __volatile__ ( "pusha \n\t" 
         "mov $0x80000001, %eax \n\t"  
         "cpuid \n\t" );
    __asm__ __volatile__ ( "mov %%edx, %0 \n\t" \
         : "=X" (flag0) \
         : /* nothing */ );
    __asm__ __volatile__ ( "popa \n\t" );
#endif
    /* printf( "Flag register: %x\n", flags[3] ); */
    if( ( flag0 | 0xffbfffff ) == 0xffffffff ) /* 22 AMD MMX Ext */
      flag += 2;
    if( ( flag0 | 0xbfffffff ) == 0xffffffff ) /* 30 3DNow! Ext */
      flag += 32;
    if( ( flag0 | 0x7fffffff ) == 0xffffffff ) /* 31 3DNow! */
      flag += 16;
  }
#ifdef WIN32
  __asm emms
#else
  __asm__ __volatile__ ( "emms" );
#endif
  return flag;
}

/* { */
/*   fprintf( stderr, "mm_support doesn't work! Output allways \"0\"(no MMX).\n" ); */
   /* Returns 1 if MMX instructions are supported, 
      3 if Cyrix MMX and Extended MMX instructions are supported 
      5 if AMD MMX and 3DNow! instructions are supported 
      0 if hardware does not support any of these */

/*   return( 0 ); */
/* } */

/*  Function to test if mmx instructions are supported...
*/
int
mmx_ok( void )
{
  /* Returns 1 if MMX instructions are supported, 0 otherwise */
  return ( mm_support() & 0x1 );
}


/*  Helper functions for the instruction macros that follow...
  (note that memory-to-register, m2r, instructions are nearly
   as efficient as register-to-register, r2r, instructions;
   however, memory-to-memory instructions are really simulated
   as a convenience, and are only 1/3 as efficient)
*/
#ifdef  MMX_TRACE

/*  Include the stuff for printing a trace to stderr...
*/

#include <stdio.h>

#define  mmx_i2r(op, imm, reg) \
  { \
    mmx_t mmx_trace; \
    mmx_trace = (imm); \
    fprintf(stderr, #op "_i2r(" #imm "=0x%016llx, ", mmx_trace.q); \
    __asm__ __volatile__ ("movq %%" #reg ", %0" \
              : "=X" (mmx_trace) \
              : /* nothing */ ); \
    fprintf(stderr, #reg "=0x%016llx) => ", mmx_trace.q); \
    __asm__ __volatile__ (#op " %0, %%" #reg \
              : /* nothing */ \
              : "X" (imm)); \
    __asm__ __volatile__ ("movq %%" #reg ", %0" \
              : "=X" (mmx_trace) \
              : /* nothing */ ); \
    fprintf(stderr, #reg "=0x%016llx\n", mmx_trace.q); \
  }

#define  mmx_m2r(op, mem, reg) \
  { \
    mmx_t mmx_trace; \
    mmx_trace = (mem); \
    fprintf(stderr, #op "_m2r(" #mem "=0x%016llx, ", mmx_trace.q); \
    __asm__ __volatile__ ("movq %%" #reg ", %0" \
              : "=X" (mmx_trace) \
              : /* nothing */ ); \
    fprintf(stderr, #reg "=0x%016llx) => ", mmx_trace.q); \
    __asm__ __volatile__ (#op " %0, %%" #reg \
              : /* nothing */ \
              : "X" (mem)); \
    __asm__ __volatile__ ("movq %%" #reg ", %0" \
              : "=X" (mmx_trace) \
              : /* nothing */ ); \
    fprintf(stderr, #reg "=0x%016llx\n", mmx_trace.q); \
  }

#define  mmx_r2m(op, reg, mem) \
  { \
    mmx_t mmx_trace; \
    __asm__ __volatile__ ("movq %%" #reg ", %0" \
              : "=X" (mmx_trace) \
              : /* nothing */ ); \
    fprintf(stderr, #op "_r2m(" #reg "=0x%016llx, ", mmx_trace.q); \
    mmx_trace = (mem); \
    fprintf(stderr, #mem "=0x%016llx) => ", mmx_trace.q); \
    __asm__ __volatile__ (#op " %%" #reg ", %0" \
              : "=X" (mem) \
              : /* nothing */ ); \
    mmx_trace = (mem); \
    fprintf(stderr, #mem "=0x%016llx\n", mmx_trace.q); \
  }

#define  mmx_r2r(op, regs, regd) \
  { \
    mmx_t mmx_trace; \
    __asm__ __volatile__ ("movq %%" #regs ", %0" \
              : "=X" (mmx_trace) \
              : /* nothing */ ); \
    fprintf(stderr, #op "_r2r(" #regs "=0x%016llx, ", mmx_trace.q); \
    __asm__ __volatile__ ("movq %%" #regd ", %0" \
              : "=X" (mmx_trace) \
              : /* nothing */ ); \
    fprintf(stderr, #regd "=0x%016llx) => ", mmx_trace.q); \
    __asm__ __volatile__ (#op " %" #regs ", %" #regd); \
    __asm__ __volatile__ ("movq %%" #regd ", %0" \
              : "=X" (mmx_trace) \
              : /* nothing */ ); \
    fprintf(stderr, #regd "=0x%016llx\n", mmx_trace.q); \
  }

#define  mmx_m2m(op, mems, memd, mmreg) \
  { \
    mmx_t mmx_trace; \
    mmx_trace = (mems); \
    fprintf(stderr, #op "_m2m(" #mems "=0x%016llx, ", mmx_trace.q); \
    mmx_trace = (memd); \
    fprintf(stderr, #memd "=0x%016llx) => ", mmx_trace.q); \
    __asm__ __volatile__ ("movq %0, %%" #mmreg " \n\t" \
              #op " %1, %%" #mmreg " \n\t" \
              "movq %%" #mmreg ", %0" \
              : "=X" (memd) \
              : "X" (mems)); \
    mmx_trace = (memd); \
    fprintf(stderr, #memd "=0x%016llx\n", mmx_trace.q); \
  }

#define mmx_m(op, mem) \
  { \
    fprintf(stderr, #op "()\n"); \
    __asm__ __volatile__ ( #op " %0" \
            : /* nothing */ \
            : "X" (mem)) \
  }

#define  mmx_r(op, reg) \
  { \
    fprintf(stderr, #op "()\n"); \
    __asm__ __volatile__ ( #op " %" #reg )
  }

#else

/*  These macros are a lot simpler without the tracing...
*/
#ifdef WIN32
#define  mmx_i2r(op, imm, reg) \
  __asm op reg, imm 
#else
#define  mmx_i2r(op, imm, reg) \
  __asm__ __volatile__ (#op " %0, %%" #reg \
            : /* nothing */ \
            : "X" (imm))
#endif

#ifdef WIN32
#define  mmx_m2r(op, mem, reg) \
  __asm op reg, mem
#else
#define  mmx_m2r(op, mem, reg) \
  __asm__ __volatile__ (#op " %0, %%" #reg \
            : /* nothing */ \
            : "X" (mem))
#endif

#ifdef WIN32
#define  mmx_r2m(op, reg, mem) \
  __asm op mem, reg 
#else
#define  mmx_r2m(op, reg, mem) \
  __asm__ __volatile__ (#op " %%" #reg ", %0" \
            : "=X" (mem) \
            : /* nothing */ )
#endif

#ifdef WIN32
#define  mmx_r2r(op, regs, regd) \
  __asm op regd, regs
#else
#define  mmx_r2r(op, regs, regd) \
  __asm__ __volatile__ (#op " %" #regs ", %" #regd)
#endif

#ifdef WIN32
#define  mmx_r2ri(op, regs, regd, imm) \
  __asm op regd, regs, imm
#else
#define  mmx_r2ri(op, regs, regd, imm) \
  __asm__ __volatile__ (#op " %0, %%" #regs ", %%" #regd \
            : /* nothing */ \
            : "X" (imm) )
#endif

#ifdef WIN32
#define  mmx_m2m(op, mems, memd, mmreg) \
  __asm movq mmreg, memd  \
  __asm op mmreg, mems    \
  __asm movq memd, mmreg 
#else
#define  mmx_m2m(op, mems, memd, mmreg) \
  __asm__ __volatile__ ("movq %0, %%" #mmreg " \n\t" \
            #op " %1, %%" #mmreg " \n\t" \
            "movq %%" #mmreg ", %0" \
            : "=X" (memd) \
            : "X" (mems))
#endif

#ifdef WIN32
#define  mmx_m2ri(op, mem, mmreg, subop) \
  __asm op mmreg, mem, subop
#else
#define  mmx_m2ri(op, mem, mmreg, subop) \
  __asm__ __volatile__ (#op " $" #subop ", %0, %%" #mmreg \
            : /* nothing */ \
            : "X" (mem))
#endif

#ifdef WIN32
#define  mmx_m2mi(op, mems, memd, mmreg, subop) \
  __asm movq mmreg, memd  \
  __asm op mmreg, mems, subop    \
  __asm movq memd, mmreg 
#else
#define  mmx_m2mi(op, mems, memd, mmreg, subop) \
  __asm__ __volatile__ ("movups %0, %%" #mmreg "\n\t" \
            #op " $" #subop ",  %1, %%" #mmreg "\n\t" \
            "movups %%" #mmreg ", %0" \
            : "=X" (memd) \
            : "X" (mems))
#endif

#ifdef WIN32
#define  mmx_m(op, mem) \
  __asm op mem
#else
#define mmx_m(op, mem) \
  __asm__ __volatile__ (#op " %0" \
            : /* nothing */ \
            : "X" (mem))
#endif

#ifdef WIN32
#define  mmx_r(op, reg) \
  __asm op reg
#else
#define mmx_r(op, reg) \
  __asm__ __volatile__ (#op " %" #reg )
#endif

#ifdef WIN32
#define  mmx_non(op) \
  __asm op
#else
#define mmx_non(op) \
  __asm__ __volatile__ (#op)
#endif

#endif


/*  1x64 MOVe Quadword
  (this is both a load and a store...
   in fact, it is the only way to store)
*/
#define  movq_m2r(var, reg)  mmx_m2r(movq, var, reg)
#define  movq_r2m(reg, var)  mmx_r2m(movq, reg, var)
#define  movq_r2r(regs, regd)  mmx_r2r(movq, regs, regd)
#define  movq(vars, vard)  mmx_m2m(movq, vars, vard, mm0)


/*  1x32 MOVe Doubleword
  (like movq, this is both load and store...
   but is most useful for moving things between
   mmx registers and ordinary registers)
*/
#define  movd_m2r(var, reg)  mmx_m2r(movd, var, reg)
#define  movd_r2m(reg, var)  mmx_r2m(movd, reg, var)
#define  movd_r2r(regs, regd)  mmx_r2r(movd, regs, regd)
#define  movd(vars, vard)   mmx_m2m(movd, vars, vard, mm0)


/*  2x32, 4x16, and 8x8 Parallel ADDs
*/
#define  paddd_m2r(var, reg)  mmx_m2r(paddd, var, reg)
#define  paddd_r2r(regs, regd)  mmx_r2r(paddd, regs, regd)
#define  paddd(vars, vard, mmreg)  mmx_m2m(paddd, vars, vard, mmreg)

#define  paddw_m2r(var, reg)  mmx_m2r(paddw, var, reg)
#define  paddw_r2r(regs, regd)  mmx_r2r(paddw, regs, regd)
#define  paddw(vars, vard, mmreg)  mmx_m2m(paddw, vars, vard, mmreg)

#define  paddb_m2r(var, reg)  mmx_m2r(paddb, var, reg)
#define  paddb_r2r(regs, regd)  mmx_r2r(paddb, regs, regd)
#define  paddb(vars, vard, mmreg)  mmx_m2m(paddb, vars, vard, mmreg)


/*  4x16 and 8x8 Parallel ADDs using Saturation arithmetic
*/
#define  paddsw_m2r(var, reg)  mmx_m2r(paddsw, var, reg)
#define  paddsw_r2r(regs, regd)  mmx_r2r(paddsw, regs, regd)
#define  paddsw(vars, vard, mmreg)  mmx_m2m(paddsw, vars, vard, mmreg)

#define  paddsb_m2r(var, reg)  mmx_m2r(paddsb, var, reg)
#define  paddsb_r2r(regs, regd)  mmx_r2r(paddsb, regs, regd)
#define  paddsb(vars, vard, mmreg)  mmx_m2m(paddsb, vars, vard, mmreg)


/*  4x16 and 8x8 Parallel ADDs using Unsigned Saturation arithmetic
*/
#define  paddusw_m2r(var, reg)  mmx_m2r(paddusw, var, reg)
#define  paddusw_r2r(regs, regd)  mmx_r2r(paddusw, regs, regd)
#define  paddusw(vars, vard, mmreg)  mmx_m2m(paddusw, vars, vard, mmreg)

#define  paddusb_m2r(var, reg)  mmx_m2r(paddusb, var, reg)
#define  paddusb_r2r(regs, regd)  mmx_r2r(paddusb, regs, regd)
#define  paddusb(vars, vard, mmreg)  mmx_m2m(paddusb, vars, vard, mmreg)


/*  2x32, 4x16, and 8x8 Parallel SUBs
*/
#define  psubd_m2r(var, reg)  mmx_m2r(psubd, var, reg)
#define  psubd_r2r(regs, regd)  mmx_r2r(psubd, regs, regd)
#define  psubd(vars, vard, mmreg)  mmx_m2m(psubd, vars, vard, mmreg)

#define  psubw_m2r(var, reg)  mmx_m2r(psubw, var, reg)
#define  psubw_r2r(regs, regd)  mmx_r2r(psubw, regs, regd)
#define  psubw(vars, vard, mmreg)  mmx_m2m(psubw, vars, vard, mmreg)

#define  psubb_m2r(var, reg)  mmx_m2r(psubb, var, reg)
#define  psubb_r2r(regs, regd, mmreg)  mmx_r2r(psubb, regs, regd)
#define  psubb(vars, vard, mmreg)  mmx_m2m(psubb, vars, vard, mmreg)


/*  4x16 and 8x8 Parallel SUBs using Saturation arithmetic
*/
#define  psubsw_m2r(var, reg)  mmx_m2r(psubsw, var, reg)
#define  psubsw_r2r(regs, regd)  mmx_r2r(psubsw, regs, regd)
#define  psubsw(vars, vard, mmreg)  mmx_m2m(psubsw, vars, vard, mmreg)

#define  psubsb_m2r(var, reg)  mmx_m2r(psubsb, var, reg)
#define  psubsb_r2r(regs, regd)  mmx_r2r(psubsb, regs, regd)
#define  psubsb(vars, vard, mmreg)  mmx_m2m(psubsb, vars, vard, mmreg)


/*  4x16 and 8x8 Parallel SUBs using Unsigned Saturation arithmetic
*/
#define  psubusw_m2r(var, reg)  mmx_m2r(psubusw, var, reg)
#define  psubusw_r2r(regs, regd)  mmx_r2r(psubusw, regs, regd)
#define  psubusw(vars, vard, mmreg)  mmx_m2m(psubusw, vars, vard, mmreg)

#define  psubusb_m2r(var, reg)  mmx_m2r(psubusb, var, reg)
#define  psubusb_r2r(regs, regd)  mmx_r2r(psubusb, regs, regd)
#define  psubusb(vars, vard, mmreg)  mmx_m2m(psubusb, vars, vard, mmreg)


/*  4x16 Parallel MULs giving Low 4x16 portions of results
*/
#define  pmullw_m2r(var, reg)  mmx_m2r(pmullw, var, reg)
#define  pmullw_r2r(regs, regd)  mmx_r2r(pmullw, regs, regd)
#define  pmullw(vars, vard, mmreg)  mmx_m2m(pmullw, vars, vard, mmreg)


/*  4x16 Parallel MULs giving High 4x16 portions of results
*/
#define  pmulhw_m2r(var, reg)  mmx_m2r(pmulhw, var, reg)
#define  pmulhw_r2r(regs, regd)  mmx_r2r(pmulhw, regs, regd)
#define  pmulhw(vars, vard, mmreg)  mmx_m2m(pmulhw, vars, vard, mmreg)


/*  4x16->2x32 Parallel Mul-ADD
  (muls like pmullw, then adds adjacent 16-bit fields
   in the multiply result to make the final 2x32 result)
*/
#define  pmaddwd_m2r(var, reg)  mmx_m2r(pmaddwd, var, reg)
#define  pmaddwd_r2r(regs, regd)  mmx_r2r(pmaddwd, regs, regd)
#define  pmaddwd(vars, vard, mmreg)  mmx_m2m(pmaddwd, vars, vard, mmreg)


/*  1x64 bitwise AND
*/
#ifdef  BROKEN_PAND
#define  pand_m2r(var, reg) \
  { \
    mmx_m2r(pandn, (mmx_t) -1LL, reg); \
    mmx_m2r(pandn, var, reg); \
  }
#define  pand_r2r(regs, regd) \
  { \
    mmx_m2r(pandn, (mmx_t) -1LL, regd); \
    mmx_r2r(pandn, regs, regd) \
  }
#define  pand(vars, vard) \
  { \
    movq_m2r(vard, mm0); \
    mmx_m2r(pandn, (mmx_t) -1LL, mm0); \
    mmx_m2r(pandn, vars, mm0); \
    movq_r2m(mm0, vard); \
  }
#else
#define  pand_m2r(var, reg)  mmx_m2r(pand, var, reg)
#define  pand_r2r(regs, regd)  mmx_r2r(pand, regs, regd)
#define  pand(vars, vard, mmreg)  mmx_m2m(pand, vars, vard, mmreg)
#endif


/*  1x64 bitwise AND with Not the destination
*/
#define  pandn_m2r(var, reg)  mmx_m2r(pandn, var, reg)
#define  pandn_r2r(regs, regd)  mmx_r2r(pandn, regs, regd)
#define  pandn(vars, vard, mmreg)  mmx_m2m(pandn, vars, vard, mmreg)


/*  1x64 bitwise OR
*/
#define  por_m2r(var, reg)  mmx_m2r(por, var, reg)
#define  por_r2r(regs, regd)  mmx_r2r(por, regs, regd)
#define  por(vars, vard, mmreg)  mmx_m2m(por, vars, vard, mmreg)


/*  1x64 bitwise eXclusive OR
*/
#define  pxor_m2r(var, reg)  mmx_m2r(pxor, var, reg)
#define  pxor_r2r(regs, regd)  mmx_r2r(pxor, regs, regd)
#define  pxor(vars, vard, mmreg)  mmx_m2m(pxor, vars, vard, mmreg)


/*  2x32, 4x16, and 8x8 Parallel CoMPare for EQuality
  (resulting fields are either 0 or -1)
*/
#define  pcmpeqd_m2r(var, reg)  mmx_m2r(pcmpeqd, var, reg)
#define  pcmpeqd_r2r(regs, regd)  mmx_r2r(pcmpeqd, regs, regd)
#define  pcmpeqd(vars, vard, mmreg)  mmx_m2m(pcmpeqd, vars, vard, mmreg)

#define  pcmpeqw_m2r(var, reg)  mmx_m2r(pcmpeqw, var, reg)
#define  pcmpeqw_r2r(regs, regd)  mmx_r2r(pcmpeqw, regs, regd)
#define  pcmpeqw(vars, vard, mmreg)  mmx_m2m(pcmpeqw, vars, vard, mmreg)

#define  pcmpeqb_m2r(var, reg)  mmx_m2r(pcmpeqb, var, reg)
#define  pcmpeqb_r2r(regs, regd)  mmx_r2r(pcmpeqb, regs, regd)
#define  pcmpeqb(vars, vard, mmreg)  mmx_m2m(pcmpeqb, vars, vard, mmreg)


/*  2x32, 4x16, and 8x8 Parallel CoMPare for Greater Than
  (resulting fields are either 0 or -1)
*/
#define  pcmpgtd_m2r(var, reg)  mmx_m2r(pcmpgtd, var, reg)
#define  pcmpgtd_r2r(regs, regd)  mmx_r2r(pcmpgtd, regs, regd)
#define  pcmpgtd(vars, vard, mmreg)  mmx_m2m(pcmpgtd, vars, vard, mmreg)

#define  pcmpgtw_m2r(var, reg)  mmx_m2r(pcmpgtw, var, reg)
#define  pcmpgtw_r2r(regs, regd)  mmx_r2r(pcmpgtw, regs, regd)
#define  pcmpgtw(vars, vard, mmreg)  mmx_m2m(pcmpgtw, vars, vard, mmreg)

#define  pcmpgtb_m2r(var, reg)  mmx_m2r(pcmpgtb, var, reg)
#define  pcmpgtb_r2r(regs, regd)  mmx_r2r(pcmpgtb, regs, regd)
#define  pcmpgtb(vars, vard, mmreg)  mmx_m2m(pcmpgtb, vars, vard, mmreg)


/*  1x64, 2x32, and 4x16 Parallel Shift Left Logical
*/
#define  psllq_i2r(imm, reg)  mmx_m2r(psllq, imm, reg)
#define  psllq_m2r(var, reg)  mmx_m2r(psllq, var, reg)
#define  psllq_r2r(regs, regd)  mmx_r2r(psllq, regs, regd)
#define  psllq(vars, vard, mmreg)  mmx_m2m(psllq, vars, vard, mmreg)

#define  pslld_i2r(imm, reg)  mmx_m2r(pslld, imm, reg)
#define  pslld_m2r(var, reg)  mmx_m2r(pslld, var, reg)
#define  pslld_r2r(regs, regd)  mmx_r2r(pslld, regs, regd)
#define  pslld(vars, vard, mmreg)  mmx_m2m(pslld, vars, vard, mmreg)

#define  psllw_i2r(imm, reg)  mmx_m2r(psllw, imm, reg)
#define  psllw_m2r(var, reg)  mmx_m2r(psllw, var, reg)
#define  psllw_r2r(regs, regd)  mmx_r2r(psllw, regs, regd)
#define  psllw(vars, vard, mmreg)  mmx_m2m(psllw, vars, vard, mmreg)


/*  1x64, 2x32, and 4x16 Parallel Shift Right Logical
*/
#define  psrlq_i2r(imm, reg)  mmx_m2r(psrlq, imm, reg)
#define  psrlq_m2r(var, reg)  mmx_m2r(psrlq, var, reg)
#define  psrlq_r2r(regs, regd)  mmx_r2r(psrlq, regs, regd)
#define  psrlq(vars, vard, mmreg)  mmx_m2m(psrlq, vars, vard, mmreg)

#define  psrld_i2r(imm, reg)  mmx_m2r(psrld, imm, reg)
#define  psrld_m2r(var, reg)  mmx_m2r(psrld, var, reg)
#define  psrld_r2r(regs, regd)  mmx_r2r(psrld, regs, regd)
#define  psrld(vars, vard, mmreg)  mmx_m2m(psrld, vars, vard, mmreg)

#define  psrlw_i2r(imm, reg)  mmx_m2r(psrlw, imm, reg)
#define  psrlw_m2r(var, reg)  mmx_m2r(psrlw, var, reg)
#define  psrlw_r2r(regs, regd)  mmx_r2r(psrlw, regs, regd)
#define  psrlw(vars, vard, mmreg)  mmx_m2m(psrlw, vars, vard, mmreg)


/*  2x32 and 4x16 Parallel Shift Right Arithmetic
*/
#define  psrad_i2r(imm, reg)  mmx_m2r(psrad, imm, reg)
#define  psrad_m2r(var, reg)  mmx_m2r(psrad, var, reg)
#define  psrad_r2r(regs, regd)  mmx_r2r(psrad, regs, regd)
#define  psrad(vars, vard, mmreg)  mmx_m2m(psrad, vars, vard, mmreg)

#define  psraw_i2r(imm, reg)    mmx_m2r(psraw, imm, reg)
#define  psraw_m2r(var, reg)    mmx_m2r(psraw, var, reg)
#define  psraw_r2r(regs, regd)    mmx_r2r(psraw, regs, regd)
#define  psraw(vars, vard, mmreg)  mmx_m2m(psraw, vars, vard, mmreg)


/*  2x32->4x16 and 4x16->8x8 PACK and Signed Saturate
  (packs source and dest fields into dest in that order)
*/
#define  packssdw_m2r(var, reg)  mmx_m2r(packssdw, var, reg)
#define  packssdw_r2r(regs, regd) mmx_r2r(packssdw, regs, regd)
#define  packssdw(vars, vard, mmreg)  mmx_m2m(packssdw, vars, vard, mmreg)

#define  packsswb_m2r(var, reg)  mmx_m2r(packsswb, var, reg)
#define  packsswb_r2r(regs, regd) mmx_r2r(packsswb, regs, regd)
#define  packsswb(vars, vard, mmreg)  mmx_m2m(packsswb, vars, vard, mmreg)


/*  4x16->8x8 PACK and Unsigned Saturate
  (packs source and dest fields into dest in that order)
*/
#define  packuswb_m2r(var, reg)  mmx_m2r(packuswb, var, reg)
#define  packuswb_r2r(regs, regd) mmx_r2r(packuswb, regs, regd)
#define  packuswb(vars, vard, mmreg)  mmx_m2m(packuswb, vars, vard, mmreg)


/*  2x32->1x64, 4x16->2x32, and 8x8->4x16 UNPaCK Low
  (interleaves low half of dest with low half of source
   as padding in each result field)
*/
#define  punpckldq_m2r(var, reg)  mmx_m2r(punpckldq, var, reg)
#define  punpckldq_r2r(regs, regd) mmx_r2r(punpckldq, regs, regd)
#define  punpckldq(vars, vard, mmreg)  mmx_m2m(punpckldq, vars, vard, mmreg)

#define  punpcklwd_m2r(var, reg)  mmx_m2r(punpcklwd, var, reg)
#define  punpcklwd_r2r(regs, regd) mmx_r2r(punpcklwd, regs, regd)
#define  punpcklwd(vars, vard, mmreg)  mmx_m2m(punpcklwd, vars, vard, mmreg)

#define  punpcklbw_m2r(var, reg)  mmx_m2r(punpcklbw, var, reg)
#define  punpcklbw_r2r(regs, regd) mmx_r2r(punpcklbw, regs, regd)
#define  punpcklbw(vars, vard, mmreg)  mmx_m2m(punpcklbw, vars, vard, mmreg)


/*  2x32->1x64, 4x16->2x32, and 8x8->4x16 UNPaCK High
  (interleaves high half of dest with high half of source
   as padding in each result field)
*/
#define  punpckhdq_m2r(var, reg)  mmx_m2r(punpckhdq, var, reg)
#define  punpckhdq_r2r(regs, regd) mmx_r2r(punpckhdq, regs, regd)
#define  punpckhdq(vars, vard, mmreg)  mmx_m2m(punpckhdq, vars, vard, mmreg)

#define  punpckhwd_m2r(var, reg)  mmx_m2r(punpckhwd, var, reg)
#define  punpckhwd_r2r(regs, regd) mmx_r2r(punpckhwd, regs, regd)
#define  punpckhwd(vars, vard, mmreg)  mmx_m2m(punpckhwd, vars, vard, mmreg)

#define  punpckhbw_m2r(var, reg)  mmx_m2r(punpckhbw, var, reg)
#define  punpckhbw_r2r(regs, regd) mmx_r2r(punpckhbw, regs, regd)
#define  punpckhbw(vars, vard, mmreg)  mmx_m2m(punpckhbw, vars, vard, mmreg)


/*  Empty MMx State
  (used to clean-up when going from mmx to float use
   of the registers that are shared by both; note that
   there is no float-to-mmx operation needed, because
   only the float tag word info is corruptible)
*/

#define  emms()  mmx_non( emms )

#endif


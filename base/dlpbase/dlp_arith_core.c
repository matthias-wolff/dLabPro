/* dLabPro base library
 * - Basic arithmetic core functions
 *
 * AUTHOR : Frank Duckhorn
 * PACKAGE: dLabPro/base
 * 
 * Copyright 2013 dLabPro contributors and others (see COPYRIGHT file) 
 * - Chair of System Theory and Speech Technology, TU Dresden
 * - Chair of Communications Engineering, BTU Cottbus
 * 
 * This file is part of dLabPro.
 * 
 * dLabPro is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 * 
 * dLabPro is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with dLabPro. If not, see <http://www.gnu.org/licenses/>.
 */

#if ARITH_FTYPE_CODE == T_FLOAT
  #define ARITH_FTYPE    FLOAT32
  #define EXP            expf
  #define LOG            logf
#elif ARITH_FTYPE_CODE == T_DOUBLE
  #define ARITH_FTYPE    FLOAT64
  #define EXP            exp
  #define LOG            log
#else
  #error ARITH_FTYPE_CODE must be T_FLOAT or T_DOUBLE
#endif

#if ARITH_FTYPE_CODE == T_FLOAT
ARITH_FTYPE dlp_scalopF(ARITH_FTYPE nParam1, ARITH_FTYPE nParam2, INT16 nOpcode)
#else
ARITH_FTYPE dlp_scalop(ARITH_FTYPE nParam1, ARITH_FTYPE nParam2, INT16 nOpcode)
#endif
{
  INT32   i = 0;                                                                /* Universal loop counter            */
  ARITH_FTYPE x = 0.;                                                           /* Universal double buffer           */

  switch (nOpcode)
  {
  case OP_NOOP     : return nParam1;
  case OP_REAL     : return nParam1;
  case OP_IMAG     : return 0;
  case OP_NEG      : return -nParam1;
  case OP_ANGLE    : return 0;
  case OP_SQR      : return nParam1*nParam1;
  case OP_ABS      : return nParam1>=0.?nParam1:-nParam1;
  case OP_SIGN     : if (nParam1 <  0) return -1.;
                     if (nParam1 == 0) return 0.;
                     return 1.;
  case OP_ENT      : return ((FLOAT64)((INT64)nParam1));
  case OP_FLOOR    : return floor(nParam1);
  case OP_CEIL     : return ceil(nParam1);
  case OP_INC      : return nParam1+1;
  case OP_DEC      : return nParam1-1;
  case OP_INVT     : return 1./nParam1;
  case OP_LN       : return LOG(nParam1);
  case OP_EXP      : return EXP(nParam1);
#if ARITH_FTYPE_CODE == T_FLOAT
  case OP_SQRT     : return sqrtf(nParam1);
  case OP_LOG      : return log10f(nParam1);
  case OP_LOG2     : return logf(nParam1)/logf(2.0f);
  case OP_SIN      : return sinf(nParam1);
  case OP_SINC     : return nParam1==0?1.f:sinf(nParam1)/nParam1;
  case OP_ASIN     : return asinf(nParam1);
  case OP_SINH     : return sinhf(nParam1);
  case OP_COS      : return cosf(nParam1);
  case OP_ACOS     : return acosf(nParam1);
  case OP_COSH     : return coshf(nParam1);
  case OP_TAN      : return tanf(nParam1);
  case OP_ATAN     : return atanf(nParam1);
  case OP_TANH     : return tanhf(nParam1);
#else /* ARITH_FTYPE_CODE == T_DOUBLE */
  case OP_SQRT     : return sqrt(nParam1);
  case OP_LOG      : return log10(nParam1);
  case OP_LOG2     : return log(nParam1)/log(2.0);
  case OP_SIN      : return sin(nParam1);
  case OP_SINC     : return nParam1==0?1.:sin(nParam1)/nParam1;
  case OP_ASIN     : return asin(nParam1);
  case OP_SINH     : return sinh(nParam1);
  case OP_COS      : return cos(nParam1);
  case OP_ACOS     : return acos(nParam1);
  case OP_COSH     : return cosh(nParam1);
  case OP_TAN      : return tan(nParam1);
  case OP_ATAN     : return atan(nParam1);
  case OP_TANH     : return tanh(nParam1);
#endif /* ARITH_FTYPE_CODE */
  case OP_SET      : return nParam1;
  case OP_FCTRL    : for (i=2,x=1.; i<=(INT32)nParam1; i++) x*=(ARITH_FTYPE)i; return x;
  #ifdef __TMS
  case OP_GAMMA    : return 0.;
  #else
  case OP_GAMMA    : return tgamma(nParam1);
  #endif
  case OP_STUDT    : return dlm_studt(nParam1,nParam2);
  case OP_ADD      : return nParam1+nParam2;
  case OP_LSADD    :
#ifdef __OPTIMIZE_LSADD
    return MIN(nParam1,nParam2)+DLP_LSADD_ERRORLIN(nParam1,nParam2); /* TODO: port double -> float */
#else
    return MIN(nParam1,nParam2)-LOG(EXP(MIN(nParam1,nParam2)-MAX(nParam1,nParam2))+1.);
#endif
  case OP_EXPADD   : return MAX(nParam1,nParam2)+LOG(EXP(MIN(nParam1,nParam2)-MAX(nParam1,nParam2))+1.);
  case OP_DIFF     : return nParam1-nParam2;
  case OP_ABSDIFF  : return nParam1>nParam2?nParam1-nParam2:nParam2-nParam1;
  case OP_QDIFF    : return (nParam1-nParam2)*(nParam1-nParam2);
  case OP_QABSDIFF : return (nParam1-nParam2)*(nParam1-nParam2);
  case OP_MULT     : return nParam1*nParam2;
  case OP_DIV      : return nParam1/nParam2;
  case OP_DIV1     : return nParam1/(nParam2+1);
  case OP_MOD      : return (ARITH_FTYPE)((INT64)nParam1%(INT64)nParam2);
  case OP_LNL      : return LOG(nParam1)<nParam2?nParam2:LOG(nParam1);
  case OP_POW      : return dlm_pow(nParam1,nParam2); /* TODO: port double -> float */
  case OP_NOVERK   : return (ARITH_FTYPE)(dlm_n_over_k((INT32)nParam1,(INT32)nParam2));
  case OP_GAUSS    : return EXP(-((nParam1*nParam1)/(nParam2*nParam2)));
  case OP_SIGMOID  : return (nParam2==0.)?nParam1:1./(1.+EXP(-(nParam1/nParam2)));
  case OP_AIZER    : return (nParam1/nParam2<0)?1/(1-nParam1/nParam2):1/(1+nParam1/nParam2);
  case OP_POTENTIAL: return (nParam2==0.)?nParam1:1./(1.+(nParam1/nParam2)*(nParam1/nParam2));
  case OP_OR       : return (nParam1!=0. || nParam2!=0.)?1.:0.;
  case OP_BITOR    : return (INT32)nParam1|(INT32)nParam2;
  case OP_AND      : return (nParam1!=0. && nParam2!=0.)?1.:0.;
  case OP_BITAND   : return (INT32)nParam1&(INT32)nParam2;
  case OP_NOT      : return (nParam1!=0.)?0.:1.;
  case OP_EQUAL    : return nParam1==nParam2?1.:0.;
  case OP_NEQUAL   : return nParam1!=nParam2?1.:0.;
  case OP_LESS     : return dlp_isnan(nParam1)||dlp_isnan(nParam2) ? 0 : (nParam1< nParam2?1.:0.);
  case OP_GREATER  : return dlp_isnan(nParam1)||dlp_isnan(nParam2) ? 0 : (nParam1> nParam2?1.:0.);
  case OP_LEQ      : return dlp_isnan(nParam1)||dlp_isnan(nParam2) ? 0 : (nParam1<=nParam2?1.:0.);
  case OP_GEQ      : return dlp_isnan(nParam1)||dlp_isnan(nParam2) ? 0 : (nParam1>=nParam2?1.:0.);
  case OP_MAX      : return dlp_isnan(nParam1)||dlp_isnan(nParam2) ? 0.0/0.0 : (nParam1>nParam2?nParam1:nParam2);
  case OP_AMAX     : return dlp_isnan(nParam1)||dlp_isnan(nParam2) ? 0.0/0.0 : (fabs(nParam1)>fabs(nParam2))?fabs(nParam1):fabs(nParam2);
  case OP_SMAX     : return dlp_isnan(nParam1)||dlp_isnan(nParam2) ? 0.0/0.0 : (fabs(nParam1)>fabs(nParam2))?nParam1:nParam2;
  case OP_MIN      : return dlp_isnan(nParam1)||dlp_isnan(nParam2) ? 0.0/0.0 : (nParam1<nParam2?nParam1:nParam2);
  case OP_AMIN     : return dlp_isnan(nParam1)||dlp_isnan(nParam2) ? 0.0/0.0 : (fabs(nParam1)<fabs(nParam2))?fabs(nParam1):fabs(nParam2);
  case OP_SMIN     : return dlp_isnan(nParam1)||dlp_isnan(nParam2) ? 0.0/0.0 : (fabs(nParam1)<fabs(nParam2))?nParam1:nParam2;
  case OP_ROUND    : return dlp_isnan(nParam1) ? nParam1 : (INT32)round(nParam1);
  case OP_ERF      : return erf(nParam1);
  case OP_ERFC     : return erfc(nParam1);
  default          : DLPASSERT(FMSG("Unknown scalar operation code"));
                     return 0.;
  }
}

#undef ARITH_FTYPE
#undef EXP
#undef LOG

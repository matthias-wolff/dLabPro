/* dLabPro class CFst (fst)
 * - String and weight semirings
 *
 * AUTHOR : Matthias Wolff
 * PACKAGE: dLabPro/classes
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

/* MWX 2004-10-13 HACK: Implementation of string table is inefficient; Use
 *                      sorted n-multigram instead!
 */

#include "dlp_cscope.h" /* Indicate C scope */
#include "dlp_fst.h"
#include "dlp_math.h"
#ifndef __UNENTANGLE_FST
  #include "dlp_matrix.h"
#endif /* #ifndef __UNENTANGLE_FST */

/* Defines */
#define ST_LP(A,B) (*(FST_STYPE**)CDlpTable_XAddr(A->iST,B,0))

/**
 * <p>Creates a new string table. A string table stores and manages a set of
 * transducer input or output symbol strings. Strings in the table are
 * identified by a unique index.</p>
 * <h3 style="color:red">Specifications for re-implementations</h3>
 * <ul>
 *  <li>The empty string has the index -1.</code>
 *  <li>Identical strings <i>must</i> have the same index.</li>
 *  <li>String indices <i>must</i> start with 0 for the first non-empty string
 *      stored and be consecutive (1,2,3,...) for further non-empty strings
 *      (this feature is needed for <code>CFst_Compose</code>).
 * </ul>
 *
 * @param nGrany Granularity of memory (re)allocations
 * @return A pointer to the new string table instance
 * @see Ssr_Done CFst_Ssr_Done
 */
FST_SST_TYPE* CGEN_SPROTECTED CFst_Ssr_Init(INT32 nGrany)
{
  FST_SST_TYPE* lpST = (FST_SST_TYPE*)__dlp_calloc(1,sizeof(FST_SST_TYPE),__FILE__,__LINE__,"CFst","");
  lpST->iST          = CDlpTable_CreateInstance();
  lpST->nGrany       = nGrany;
  lpST->lpBuf        = NULL;

  CDlpTable_AddComp(lpST->iST,"str",sizeof(FST_STYPE*));

  return lpST;
}

/**
 * Destroys a string table.
 *
 * @param lpST String table data structure (returned by {@link Ssr_Init CFst_Ssr_Init})
 * @see Ssr_Init CFst_Ssr_Init
 */
void CGEN_SPROTECTED CFst_Ssr_Done(FST_SST_TYPE* lpST)
{
  INT32 i;

  for (i=0; i<CDlpTable_GetNRecs(lpST->iST); i++) __dlp_free(ST_LP(lpST,i));

  CDlpTable_DestroyInstance(lpST->iST);
  dlp_free(lpST->lpBuf);
  dlp_free(lpST);
}

/**
 * Finds a string in a string table.
 *
 * @param lpST  Pointer to a string table data structure (returned by
 *              {@link Ssr_Init CFst_Ssr_Init})
 * @param lpBuf Pointer to string to be stored
 * @return The unique string index or -1 if not found
 */
FST_ITYPE CGEN_SPRIVATE CFst_Ssr_Find(FST_SST_TYPE* lpST, FST_ITYPE* lpBuf)
{
  INT32       i   = 0;                                                          /* String index in table             */
  INT32       j   = 0;                                                          /* Character index in current string */
  FST_STYPE* lpS = NULL;                                                       /* Pointer to current string         */

  if (lpBuf[0]==-1) return -1;                                                 /* epsilon string                    */

  for (i=0; i<CDlpTable_GetNRecs(lpST->iST); i++)                              /* Loop over all strings in table    */
  {
    for (j=0, lpS=ST_LP(lpST,i); lpBuf[j]!=-1 && lpS[j]!=-1; j++)              /* Compare character by character    */
      if (lpBuf[j]!=lpS[j])
        break;

    if (lpBuf[j]==-1 && lpS[j]==-1) return i;                                  /* Found: return string index        */
  }

  return -1;                                                                   /* Not found: return -1              */
}

/**
 * Stores a string in a string table.
 *
 * @param lpST  Pointer to a string table data structure (returned by
 *              {@link Ssr_Init CFst_Ssr_Init})
 * @param lpBuf Pointer to string to be stored
 * @return The unique string index of the new string
 */
FST_ITYPE CGEN_SPROTECTED CFst_Ssr_Store(FST_SST_TYPE* lpST, FST_ITYPE* lpBuf)
{
  FST_ITYPE nS = 0;
  INT32      nL = 0;

  if (lpBuf[0]                      ==-1) return -1;                           /* epsilon string                    */
  if ((nS=CFst_Ssr_Find(lpST,lpBuf))>= 0) return nS;                           /* String already present in table   */

  nS = CDlpTable_AddRecs(lpST->iST,1,lpST->nGrany);
  for (nL=0; lpBuf[nL]!=-1; nL++) {}
  nL++;

  *(FST_STYPE**)CDlpTable_XAddr(lpST->iST,nS,0) =
    (FST_STYPE*)__dlp_calloc(nL,sizeof(FST_STYPE),__FILE__,__LINE__,"CFst","");

  dlp_memmove(ST_LP(lpST,nS),lpBuf,sizeof(FST_STYPE)*nL);

  return nS;
}

/**
 * Fetches a string from a string table.
 *
 * @param lpST    Pointer to a string table data structure (returned by
 *                {@link Ssr_Init CFst_Ssr_Init})
 * @param nS      Index of string to fetch
 * @param lpBuf   Pointer to a buffer to be filled
 * @param nMaxLen Maximal number of symbols to fetch (including the terminal
 *                symbol <code>-1</code>)
 */
void CGEN_SPROTECTED CFst_Ssr_Fetch(FST_SST_TYPE* lpST, FST_ITYPE nS, FST_ITYPE* lpBuf, INT32 nMaxLen)
{
  INT32 nL = CFst_Ssr_Len(lpST,nS);
  if (nMaxLen<nL) nL = nMaxLen;
  dlp_memmove(lpBuf,ST_LP(lpST,nS),sizeof(FST_STYPE)*nL);
  if (nL==nMaxLen) lpBuf[nL]=-1;
}

/**
 * Returns the n'th character of a string.
 *
 * @param lpST Pointer to a string table data structure (returned by
 *             {@link Ssr_Init CFst_Ssr_Init})
 * @param nS   Index of string to fetch
 * @param nPos Index of character to fetch
 * return The character or -1 in case of errors.
 */
FST_STYPE CGEN_SPROTECTED CFst_Ssr_GetAt(FST_SST_TYPE* lpST, FST_ITYPE nS, FST_ITYPE nPos)
{
  INT32 nL = CFst_Ssr_Len(lpST,nS);
  if (nL  <= 0) return -1;
  if (nPos> nL) return -1;
  return ST_LP(lpST,nS)[nPos];
}

/**
 * Determines the length of a symbol string.
 *
 * @param lpST Pointer to a string table data structure (returned by
 *             {@link Ssr_Init CFst_Ssr_Init})
 * @param nS   Index of string
 * @return The length (excluding the terminal symbol <code>-1</code>)
 */
INT32 CGEN_SPROTECTED CFst_Ssr_Len(FST_SST_TYPE* lpST, FST_ITYPE nS)
{
  INT32 nLen = 0;
  if (nS<0) return 0;
  while (ST_LP(lpST,nS)[nLen]!=-1) nLen++;
  return nLen;
}

/**
 * Returns the neutral element of the string semiring multipilcation.
 */
FST_ITYPE CGEN_SPROTECTED CFst_Ssr_NeMult()
{
  return -1;
}

/**
 * Returns the neutral element of the string semiring addition.
 */
FST_ITYPE CGEN_SPROTECTED CFst_Ssr_NeAdd()
{
  return -2;
}

/**
 * Multiplication operation of the symbol string semiring (concatentation).
 *
 * @param lpST Pointer to a string table data structure (returned by
 *             {@link Ssr_Init CFst_Ssr_Init})
 * @param nS1  Index of first (left) string
 * @param nS2  Index of second (right) string
 * @return Index of result string
 */
FST_ITYPE CGEN_SPROTECTED CFst_Ssr_Mult(FST_SST_TYPE* lpST, FST_ITYPE nS1, FST_ITYPE nS2)
{
  INT32 nL1 = 0;
  INT32 nL2 = 0;

  if (nS1<0) return nS2;
  if (nS2<0) return nS1;

  nL1 = CFst_Ssr_Len(lpST,nS1);
  nL2 = CFst_Ssr_Len(lpST,nS2);

  if ((INT32)dlp_size(lpST->lpBuf)<nL1+nL2+1)
    lpST->lpBuf=(FST_STYPE*)__dlp_realloc(lpST->lpBuf,nL1+nL2+lpST->nGrany,sizeof(FST_STYPE),__FILE__,__LINE__,"CFst","");

  dlp_memmove( lpST->lpBuf     ,ST_LP(lpST,nS1),nL1*sizeof(FST_STYPE));
  dlp_memmove(&lpST->lpBuf[nL1],ST_LP(lpST,nS2),nL2*sizeof(FST_STYPE));
  lpST->lpBuf[nL1+nL2]=-1;

  return CFst_Ssr_Store(lpST,lpST->lpBuf);
}

/**
 * Addition operation of the symbol string semiring (longest common prefix).
 *
 * @param lpST Pointer to a string table data structure (returned by
 *             {@link Ssr_Init CFst_Ssr_Init})
 * @param nS1  Index of first string
 * @param nS2  Index of second string
 * @return Index of result string or -1 if no matching prefixes
 */
FST_ITYPE CGEN_SPROTECTED CFst_Ssr_Add(FST_SST_TYPE* lpST, FST_ITYPE nS1, FST_ITYPE nS2)
{
  INT32 nL1 = 0;
  INT32 nL2 = 0;
  INT32 nLr = 0;

  if (nS1==nS2          ) return nS1;  /* Args are identical                  */
  if (nS1==-2           ) return nS2;  /* Arg. 1 is the infinite string       */
  if (nS2==-2           ) return nS1;  /* Arg. 2 is the infinite string       */
  if (nS1==-1 || nS2==-1) return -1;   /* Either argument is the empty string */

  nL1  = CFst_Ssr_Len(lpST,nS1);
  nL2  = CFst_Ssr_Len(lpST,nS2);

  for (nLr=0; nLr<nL1 && nLr<nL2; nLr++)
  {
    if (ST_LP(lpST,nS1)[nLr]!=ST_LP(lpST,nS2)[nLr]) break;
    if (ST_LP(lpST,nS1)[nLr]==-1 || ST_LP(lpST,nS2)[nLr]==-1) break;
  }

  if (nLr==0) return -1;

  if ((INT32)dlp_size(lpST->lpBuf)<nLr)
    lpST->lpBuf=(FST_STYPE*)__dlp_realloc(lpST->lpBuf,nLr+lpST->nGrany,sizeof(FST_STYPE),__FILE__,__LINE__,"CFst","");

  dlp_memmove(lpST->lpBuf,ST_LP(lpST,nS1),nLr);
  lpST->lpBuf[nLr]=-1;

  return CFst_Ssr_Store(lpST,lpST->lpBuf);
}

/**
 * Difference (division) operation of the symbol string semiring (residual
 * of longest common prefix).
 *
 * @param lpST Pointer to a string table data structure (returned by
 *             {@link Ssr_Init CFst_Ssr_Init})
 * @param nS1  Index of first string
 * @param nS2  Index of second string
 * @return Index of result string or -1 if strings identical
 */
FST_ITYPE CGEN_SPROTECTED CFst_Ssr_Dif(FST_SST_TYPE* lpST, FST_ITYPE nS1, FST_ITYPE nS2)
{
  FST_ITYPE nSs = -1;
  INT32      nL1 = 0;

  if (nS1==nS2          ) return -1;   /* Args are identical                 */
  if (nS1==-1           ) return -1;   /* Arg. 1 is the empty string         */
  if (nS2==-1           ) return nS1;  /* Arg. 2 is the empty string         */
  if (nS1==-2 || nS2==-2) return -1;   /* Either arg. is the infinite string */

  nL1 = CFst_Ssr_Len(lpST,nS1)+1;
  if (nL1==0) return -1;

  nSs = CFst_Ssr_Add(lpST,nS1,nS2);
  if (nSs<0) return nS1;

  if ((INT32)dlp_size(lpST->lpBuf)<nL1)
    lpST->lpBuf=(FST_STYPE*)__dlp_realloc(lpST->lpBuf,nL1+lpST->nGrany,sizeof(FST_STYPE),__FILE__,__LINE__,"CFst","");

  CFst_Ssr_Fetch(lpST,nS1,lpST->lpBuf,nL1);

  return CFst_Ssr_Store(lpST,&lpST->lpBuf[CFst_Ssr_Len(lpST,nSs)]);
}

/**
 * Prints one string.
 *
 * @param lpST Pointer to a string table data structure (returned by
 *             {@link Ssr_Init CFst_Ssr_Init})
 * @param nS   Index of string
 */
void CGEN_SPROTECTED CFst_Ssr_Print(FST_SST_TYPE* lpST, FST_ITYPE nS)
{
  INT32 i=0;

  if (CFst_Ssr_Len(lpST,nS))
  {
    for (i=0; i<=CFst_Ssr_Len(lpST,nS); i++)
    {
      printf(i==0?"(":",");
      printf("%ld",(long)ST_LP(lpST,nS)[i]);
    }
    printf(")");
  }
  else printf("(-1)");
}

/**
 * Returns the type of the weight semiring of an automaton.
 *
 * @param _this   Pointer to automaton instance
 * @param lpnComp Pointer to a long variable to be filled with the component
 *                index of the weight component in the transition table
 *                {@link td} (may be <code>NULL</code>).
 * @return The type of the weight semiring (<code>FST_WSR_PROB</code>,
 *         <code>FST_WSR_LOG</code> or <code>FST_WSR_TROP</code>) or
 *         <code>FST_WSR_NONE</code> if the automaton is unweighted.
 * @see Wsr_Op CFst_Wsr_Op
 */
INT16 CGEN_PUBLIC CFst_Wsr_GetType(CFst* _this, INT32* lpnComp)
{
  INT32 nIc = -1;

  CHECK_THIS_RV(-1);
  if (lpnComp) *lpnComp=-1;

  if ((nIc=CData_FindComp(AS(CData,_this->td),NC_TD_PSR))>=IC_TD_DATA)
  {
    if (lpnComp) *lpnComp=nIc;
    return FST_WSR_PROB;
  }
  if ((nIc=CData_FindComp(AS(CData,_this->td),NC_TD_LSR))>=IC_TD_DATA)
  {
    if (lpnComp) *lpnComp=nIc;
    return FST_WSR_LOG;
  }
  if ((nIc=CData_FindComp(AS(CData,_this->td),NC_TD_TSR))>=IC_TD_DATA)
  {
    if (lpnComp) *lpnComp=nIc;
    return FST_WSR_TROP;
  }
  return FST_WSR_NONE;
}

/**
 * Returns the name for a given weight semiring type.
 *
 * @param nWsrType
 *          The weight semiring type
 * @return
 *          A pointer to a static buffer containing the name
 * @see CFst_Wsr_GetType
 */
const char* CGEN_SPUBLIC CFst_Wsr_GetName(INT16 nWsrType)
{
  switch (nWsrType)
  {
    case FST_WSR_PROB: return "probability";
    case FST_WSR_LOG : return "logarithmic";
    case FST_WSR_TROP: return "tropical";
    default          : return "???";
  }
}

/**
 * <p>Performs an arithmetic operation <code>OP(nW1,nW2)</code> depending on the
 * specified weight semiring type.</p>
 *
 * <p>The following operations are supported (parameter <code><b>nOpc</b></code>):</p>
 * <div class="indent">
 * <table>
 *   <tr><th><code>nOpc      </code></th><th colspan="2">Description                                              </th></tr>
 *   <tr><td><code>OP_ADD    </code></td><td><code>nW1(+)nW2 </code></td><td>Addition                              </td></tr>
 *   <tr><td><code>OP_MULT   </code></td><td><code>nW1(*)nW2 </code></td><td>Multiplication                        </td></tr>
 *   <tr><td><code>OP_DIV    </code></td><td><code>nW1(/)nW2 </code></td><td>Division (multiplicative residual)    </td></tr>
 *   <tr><td><code>OP_EQUAL  </code></td><td><code>nW1==nW2  </code></td><td>Floating point comparison<sup>1)</sup></td></tr>
 *   <tr><td><code>OP_LESS   </code></td><td><code>nW1&lt;nW2</code></td><td>Less than <sup>1)</sup>               </td></tr>
 *   <tr><td><code>OP_GREATER</code></td><td><code>nW1&gt;nW2</code></td><td>Greater than<sup>1)</sup>             </td></tr>
 * </table>
 * <p>1) Result is 1.0 if condition is true, 0.0 otherwise</p>
 * </div>
 *
 * <h3 style="color:red">Important Note:</h3>
 * <p>The method evaluates the field
 * <code>_this->{@link wsr m_nWsr}</code> to  determine the semiring type. This
 * field is <em>not</em> maintained automatically, meaning you must ensure
 * <code>_this->{@link wsr m_nWsr}</code> to specify the correct semiring type
 * prior to calling <code>CFst_Wsr_Op</code>! You can determine the current
 * semiring type by calling {@link Wsr_GetType CFst_Wsr_GetType}.</p>
 *
 * @param _this Pointer to automaton instance
 * @param nW1   Operand 1
 * @param nW2   Operand 2
 * @param nOpc  Operation code (see table above)
 * @return The  result of the operation (<em>no</em> value is reserved for reporting errors!).
 * @see Wsr_GetType CFst_Wsr_GetType
 * @see Wsr_NeAdd   CFst_Wsr_NeAdd
 * @see Wsr_NeMult  CFst_Wsr_NeMult
 */
FST_WTYPE CGEN_PROTECTED CFst_Wsr_Op(CFst* _this, FST_WTYPE nW1, FST_WTYPE nW2, INT16 nOpc)
{
  /* Validation */
  CHECK_THIS_RV(0.);

  /* In case the user forgot: determine current semiring type if not yet set */
  if (_this->m_nWsr<=0) _this->m_nWsr = CFst_Wsr_GetType(_this,NULL);

  /* Operations */
  switch (nOpc)
  {
    /* Addition */
    case OP_ADD:
      switch (_this->m_nWsr)
      {
        case FST_WSR_PROB: return nW1+nW2;
        case FST_WSR_LOG : return dlp_scalop(nW1,nW2,OP_LSADD);
        case FST_WSR_TROP: return nW1<nW2?nW1:nW2;
        case 0           : return 0.;                                          /* Not weighted; just do nothing     */
        default          : DLPASSERT(FMSG("Invalid weight semiring type"));
      }
      return 0.;

    /* Multiplication */
    case OP_MULT:
      switch (_this->m_nWsr)
      {
        case FST_WSR_PROB: return nW1*nW2;
        case FST_WSR_LOG : return nW1+nW2;
        case FST_WSR_TROP: return nW1+nW2;
        case 0           : return 0.;                                          /* Not weighted; just do nothing     */
        default          : DLPASSERT(FMSG("Invalid weight semiring type"));
      }
      return 0.;

    /* Power */
    case OP_POW:
      switch (_this->m_nWsr)
      {
        case FST_WSR_PROB: return dlm_pow(nW1,nW2);
        case FST_WSR_LOG : return nW1*nW2;
        case FST_WSR_TROP: return nW1*nW2;
        case 0           : return 0.;                                          /* Not weighted; just do nothing     */
        default          : DLPASSERT(FMSG("Invalid weight semiring type"));
      }
      return 0.;

    /* Division (multiplicative residual) */
    case OP_DIV:
      switch (_this->m_nWsr)
      {
        case FST_WSR_PROB: return nW1/nW2;
        case FST_WSR_LOG : return nW1-nW2;
        case FST_WSR_TROP: return nW1-nW2;
        case 0           : return 0.;                                          /* Not weighted; just do nothing     */
        default          : DLPASSERT(FMSG("Invalid weight semiring type"));
      }
      return 0.;

    /* Floating point comparison */
    case OP_EQUAL:
      if      (fabs(nW1)<_this->m_nFtol) return (fabs(nW2)<_this->m_nFtol);
      else if (fabs(nW2)<_this->m_nFtol) return (fabs(nW1)<_this->m_nFtol);
      else                               return (fabs((nW1-nW2)/nW1)<_this->m_nFtol);

    /* Less than */
    case OP_LESS:
      switch (_this->m_nWsr)
      {
        case FST_WSR_LOG : return (nW1>nW2);
        case FST_WSR_TROP: return (nW1>nW2);
        case FST_WSR_PROB: return (nW1<nW2);
        case 0           : return (nW1<nW2);
        default          : DLPASSERT(FMSG("Invalid weight semiring type"));
      }
      return 0.;

    /* Greater than */
    case OP_GREATER:
      switch (_this->m_nWsr)
      {
        case FST_WSR_LOG : return (nW1<nW2);
        case FST_WSR_TROP: return (nW1<nW2);
        case FST_WSR_PROB: return (nW1>nW2);
        case 0           : return (nW1>nW2);
        default          : DLPASSERT(FMSG("Invalid weight semiring type"));
      }
      return 0.;

    /* Invalid opcode */
    default:
      DLPASSERT(FMSG("Invalid weight semiring operation"));
      return 0.;
  }
}

/**
 * Returns the neutral element of the addition operation of the weight
 * semiring.
 *
 * @param nSrType Semiring type (<code>FST_WSR_PROB</code>,
 *                <code>FST_WSR_LOG</code> or <code>FST_WSR_TROP</code>)
 * @return The neutral element
 */
FST_WTYPE CGEN_SPROTECTED CFst_Wsr_NeAdd(INT16 nSrType)
{
  switch (nSrType)
  {
    case FST_WSR_LOG : return T_DOUBLE_MAX;
    case FST_WSR_TROP: return T_DOUBLE_MAX;
    case FST_WSR_PROB: return 0.;
    case 0           : return 0.;                                              /* Not weighted; just do nothing */
    default          : DLPASSERT(FMSG("Invalid weight semiring type"));
  }
  return 0.;
}

/**
 * Returns the neutral element of the multiplication operation of the weight
 * semiring.
 *
 * @param nSrType Semiring type (<code>FST_WSR_PROB</code>,
 *                <code>FST_WSR_LOG</code> or <code>FST_WSR_TROP</code>)
 * @return The neutral element
 */
FST_WTYPE CGEN_SPROTECTED CFst_Wsr_NeMult(INT16 nSrType)
{
  switch (nSrType)
  {
    case FST_WSR_PROB: return 1.;
    case FST_WSR_LOG : return 0.;
    case FST_WSR_TROP: return 0.;
    case 0           : return 0.;                                              /* Not weighted; just do nothing */
    default          : DLPASSERT(FMSG("Invalid weight semiring type"));
  }
  return 0.;
}

/**
 * Converts the automaton weights to weights to another semiring.
 *
 * @param nSrType Semiring type (<code>FST_WSR_PROB</code>,
 *                <code>FST_WSR_LOG</code> or <code>FST_WSR_TROP</code>)
 * @return <code>O_K</code> if successfull, a (negative) error code otherwise
 */
INT16 CGEN_PUBLIC CFst_Wsr_Convert(CFst* _this, INT16 nSrType)
{
  INT16 nWsrt = FST_WSR_NONE;
  INT32  nIcW  = -1;
  INT32  nT    = 0;
  INT32  nXXT  = 0;
  INT32  nRlt  = 0;
  BYTE* lpW   = NULL;
  BYTE* lpW0  = NULL;

  /* Validation */                                                              /* --------------------------------- */
  CHECK_THIS_RV(NOT_EXEC);                                                      /* Check this pointer                */
  CFst_Check(_this);                                                            /* Check FST(s)                      */

  /* Initialize */                                                              /* --------------------------------- */
  nWsrt = CFst_Wsr_GetType(_this,&nIcW);                                        /* Get current weight semiring type  */
  if (nWsrt==FST_WSR_NONE) return IERROR(_this,FST_UNWEIGHTED,0,0,0);           /* No weights, no service            */
  if (nWsrt==nSrType     ) return O_K;                                          /* Nothing to be done                */
  lpW0 = CData_XAddr(AS(CData,_this->td),0,nIcW);                               /* Get pointer to weight component   */
  nRlt = CData_GetRecLen(AS(CData,_this->td));                                  /* Get record length of trans. list  */
  nXXT = UD_XXT(_this);                                                         /* Get total number of transitions   */

  /* Convert weights */                                                         /* --------------------------------- */
  switch (nSrType)                                                              /* Branch for target weight sr. type */
  {                                                                             /* >>                                */
  case FST_WSR_PROB:                                                            /* Probability semiring              */
    /* TODO: Push log/tropical weights to the left!!! */                        /* X X X X X X X X X X X X X X X X X */
    IERROR(_this,FST_INTERNALW,"Probabilities may be >1",__FILE__,__LINE__);    /* Warn user!                        */
    for (nT=0,lpW=lpW0; nT<nXXT; nT++,lpW+=nRlt)                                /* Loop over all transitions         */
      *(FST_WTYPE*)lpW = exp(*(FST_WTYPE*)lpW*-1);                              /*   P = exp(-w)                     */
    CData_SetCname(AS(CData,_this->td),nIcW,NC_TD_PSR);                         /*   Rename weight component         */
    break;                                                                      /* ==                                */
  case FST_WSR_LOG:                                                             /* Log semiring                      */
    if (nWsrt==FST_WSR_PROB)                                                    /* Converting from probabilities     */
      for (nT=0,lpW=lpW0; nT<nXXT; nT++,lpW+=nRlt)                              /*   Loop over all transitions       */
        *(FST_WTYPE*)lpW = -1*log(*(FST_WTYPE*)lpW);                            /*     w = -log(P)                   */
    CData_SetCname(AS(CData,_this->td),nIcW,NC_TD_LSR);                         /*   Rename weight component         */
    break;                                                                      /* ==                                */
  case FST_WSR_TROP:                                                            /* Topical semiring                  */
    if (nWsrt==FST_WSR_PROB)                                                    /* Converting from probabilities     */
      for (nT=0,lpW=lpW0; nT<nXXT; nT++,lpW+=nRlt)                              /*   Loop over all transitions       */
        *(FST_WTYPE*)lpW = -1*log(*(FST_WTYPE*)lpW);                            /*     w = -log(P)                   */
    CData_SetCname(AS(CData,_this->td),nIcW,NC_TD_TSR);                         /*   Rename weight component         */
    break;                                                                      /* ==                                */
  default:                                                                      /* nSrType unknown                   */
    return IERROR(_this,FST_INVALID,"weight semiring type",0,0);                /*   Error                           */
  }                                                                             /* <<                                */
  return O_K;                                                                   /* All right                         */
}

/*
 * Manual page at fst_man.def
 */
INT16 CGEN_PUBLIC CFst_Probs(CFst* _this, INT32 nUnit)
{
  INT32          nU     = 0;                                                     /* Current unit                      */
  FST_ITYPE     nS     = 0;                                                     /* Current state                     */
  FST_ITYPE     nT     = 0;                                                     /* Current transition                */
  FST_ITYPE     nXS    = 0;                                                     /* Number of states in current unit  */
  INT16         nWsrt  = 0;                                                     /* Weight semiring type              */
  INT32          nIcW   = -1;                                                    /* Weight component in td            */
  INT32          nIcPfl = -1;                                                    /* Floor probability component in sd */
  INT32          nIcRct = -1;                                                    /* Reference counter component in td */
  INT32          nTrCnt = 0;                                                     /* Number of outgoing trans./state   */
  FST_WTYPE     nW     = 0.;                                                    /* Current weight                    */
  FST_WTYPE     nRc    = 0.;                                                    /* Current reference counter/prob.   */
  FST_WTYPE     nRcSum = 0.;                                                    /* Reference counter sum/state       */
  FST_WTYPE     nTW    = 0.;                                                    /* trans.weight                      */
  FST_WTYPE     nTWAgg = 0.;                                                    /* trans.weight aggregator for MAP   */
  CData*        idTd   = NULL;                                                  /* Pointer to transition table       */
  FST_TID_TYPE* lpTI   = NULL;                                                  /* Automaton iterator data structure */
  BYTE*         lpT    = NULL;                                                  /* Current transition (iteration)    */
  FST_WTYPE     nMAP   = _this->m_bUsemap ? _this->m_nMapexp : 0.;              /* MAP exponent (0:off,1:min,-1:max) */
  INT32          nMAPi  = 0;                                                     /* MAP iteration index               */

  /* Validation */                                                              /* --------------------------------- */
  CHECK_THIS_RV(NOT_EXEC);                                                      /* Check this pointer                */
  CFst_Check(_this);                                                            /* Check FST(s)                      */

  /* Initialize */                                                              /* --------------------------------- */
  idTd   = AS(CData,_this->td);                                                 /* Get pointer to transition table   */
  nIcRct = CData_FindComp(AS(CData,_this->td),NC_TD_RC);                        /* Find reference counters           */
  nWsrt  = CFst_Wsr_GetType(_this,&nIcW);                                       /* Find weights and get type         */
  switch (nWsrt)                                                                /* branch for weight semiring type   */
  {                                                                             /* >>                                */
    case FST_WSR_NONE:                                                          /*   No weights                      */
      CData_AddComp(idTd,NC_TD_PSR,DLP_TYPE(FST_WTYPE));                        /*     Add some                      */
      nIcW = CData_GetNComps(idTd)-1;                                           /*     Get index of new weight comp. */
      for (nT=0; nT<UD_XXT(_this); nT++)                                        /*     Initialize probabilities      */
        CData_Dstore(idTd,1.,nT,nIcW);                                          /*     ...                           */
      break;                                                                    /*     *                             */
    case FST_WSR_PROB:                                                          /*   Probabilities                   */
      break;                                                                    /*     * (nothing to be done)        */
    case FST_WSR_TROP: /* fall through */                                       /*   Tropical weights                */
    case FST_WSR_LOG:                                                           /*   Logarithmic weights             */
      for (nT=0; nT<UD_XXT(_this); nT++)                                        /*     Loop over all transitions     */
        CData_Dstore(idTd,                                                      /*       Convert to probabilities    */
          exp(-1.*CData_Dfetch(idTd,nT,nIcW)),nT,nIcW);                         /*       |                           */
      break;                                                                    /*     *                             */
    default:                                                                    /*   Unknown weight semiring         */
      DLPASSERT(FMSG("Unknown weight semiring"));                               /*     New weight semiring type?     */
      CData_SetCname(idTd,nIcW,NC_TD_PSR);                                      /*     Rename to probabilities       */
      for (nT=0; nT<UD_XXT(_this); nT++)                                        /*     Initialize probabilities      */
        CData_Dstore(idTd,1.,nT,nIcW);                                          /*     ...                           */
  }                                                                             /* <<                                */
  if (_this->m_nSymbols>0 && _this->m_nRcfloor>0.)                              /* Smoothing enabled                 */
  {                                                                             /* >>                                */
    if (CData_FindComp(AS(CData,_this->sd),"~PFL")<0)                           /*   No floor prob. comp. at states  */
      CData_AddComp(AS(CData,_this->sd),"~PFL",DLP_TYPE(FST_WTYPE));            /*     Add it                        */
    nIcPfl = CData_FindComp(AS(CData,_this->sd),"~PFL");                        /*   Get index of floor prob. comp.  */
  }                                                                             /* <<                                */

  /* Loop over units */                                                         /* --------------------------------- */
  for (nU=nUnit<0?0:nUnit; nU<UD_XXU(_this); nU++)                              /* For all units ...                 */
  {                                                                             /* >>                                */
    lpTI = CFst_STI_Init(_this,nU,FSTI_SORTINI);                                /*   Get sorted transition iterator  */

    /* Loop over states */                                                      /*   - - - - - - - - - - - - - - - - */
    for (nS=0,nXS=UD_XS(_this,nU); nS<nXS; nS++)                                /*   For all states of the unit ...  */
    /* Loop over MAP-Iterations if MAP enabled */                               /*   - - - - - - - - - - - - - - - - */
    for(nMAPi=0;nMAPi<(nMAP!=0.?10:1);nMAPi++)                                  /*   For all MAP-iterations ...      */
    {                                                                           /*   >>                              */
      /* Pass 1: Count outgoing transitions and sum up reference counters */    /*                                   */
      nRcSum = 0.;                                                              /*     Reset ref. ctr. accumulator   */
      nTWAgg = 0.;                                                              /*     Reset MAP TW accumulator      */
      nTrCnt = 0;                                                               /*     Reset transition counter      */
      lpT    = NULL;                                                            /*     Initialize transition pointer */
      while ((lpT=CFst_STI_TfromS(lpTI,nS,lpT))!=NULL)                          /*     Enumerate transitions at nS   */
      {                                                                         /*     >>                            */
        nRc = nIcRct>=0                                                         /*       Get ref. ctr. or prob.      */
            ? CData_Dfetch(idTd,CFst_STI_GetTransId(lpTI,lpT),nIcRct)           /*       |                           */
            : *CFst_STI_TW(lpTI,lpT);                                           /*       |                           */
        if (nRc>=0.) nRcSum+=nRc;                                               /*       Accumulate non-neg. values  */
        if (nMAP!=0. && nRc>0.)                                                 /*       if MAP enabled and T. used  */
        {                                                                       /*       >>                          */
          nTW = *CFst_STI_TW(lpTI,lpT);                                         /*         get trans.weight          */
          if(nTW<=0.) nTW=0.0001;                                               /*         solve prob. with 0 TW's   */
          nTWAgg += nTW*log(nTW);                                               /*         acc. TW's                 */
        }                                                                       /*       <<                          */
        nTrCnt++;                                                               /*       Count transitions           */
      }                                                                         /*     <<                            */
      if (nIcPfl)                                                               /*     Smoothing eneabled            */
        CData_Dstore(AS(CData,_this->sd),                                       /*       Store floor probability     */
          _this->m_nRcfloor / (nRcSum + _this->m_nSymbols*_this->m_nRcfloor),   /*       |                           */
          nS,nIcPfl);                                                           /*       |                           */
                                                                                /*                                   */
      /* Pass 2: Calculate probabilities */                                     /*                                   */
      if (nTrCnt)                                                               /*     If there were trans. at nS    */
      {                                                                         /*     >>                            */
        lpT = NULL;                                                             /*       Initialize transition ptr.  */
        while ((lpT=CFst_STI_TfromS(lpTI,nS,lpT))!=NULL)                        /*       Enumerate transitions at nS */
        {                                                                       /*       >>                          */
          nRc = nIcRct>=0                                                       /*         Get ref. ctr. or prob.    */
              ? CData_Dfetch(idTd,CFst_STI_GetTransId(lpTI,lpT),nIcRct)         /*         |                         */
              : *CFst_STI_TW(lpTI,lpT);                                         /*         |                         */
          if (nMAP!=0.)                                                         /*         if MAP enabled            */
          {                                                                     /*         <<                        */
            nTW = *CFst_STI_TW(lpTI,lpT);                                       /*           get trans.weight        */
            if(nTW<=0.) nTW=0.0001;                                             /*           solve prob. with 0 TW's */
            if(nIcRct>=0 && _this->m_nSymbols>0 && _this->m_nRcfloor>0)         /*           do jeffrey smooth?      */
              nRc = nRcSum * (nRc + (FST_WTYPE)_this->m_nRcfloor) /             /*             smooth nRc            */
                (nRcSum + ((FST_WTYPE)_this->m_nSymbols*_this->m_nRcfloor));    /*             |                     */
            nTW = (nRc<=0. ? 0. : (nRc+nMAP*nTW*log(nTW))/(nRcSum+nMAP*nTWAgg));/*           do MAP-iteration        */
            *CFst_STI_TW(lpTI,lpT) = nTW;                                       /*           write trans.weight      */
          }                                                                     /*         >>                        */
          else                                                                  /*         else                      */
            if (nIcRct<0 || _this->m_nSymbols<=0 || _this->m_nRcfloor<=0.)      /*         No ref.ctrs. or smoothing */
              *CFst_STI_TW(lpTI,lpT) =                                          /*           Store equally distrib.  */
                nRcSum==0. ? 1./(FST_WTYPE)nTrCnt : nRc/nRcSum;                 /*           | or renormalized probs.*/
            else                                                                /*         Ref. ctrs. or smoothing   */
              *CFst_STI_TW(lpTI,lpT) =                                          /*           Store Jeffrey smoothed  */
                (nRc + (FST_WTYPE)_this->m_nRcfloor) /                          /*           | probabilities         */
                (nRcSum + ((FST_WTYPE)_this->m_nSymbols*_this->m_nRcfloor));    /*           |                       */
        }                                                                       /*       <<                          */
      }                                                                         /*     <<                            */
    }                                                                           /*   <<                              */
    CFst_STI_Done(lpTI);                                                        /*   Destroy iterator                */
    if (nUnit>=0) break;                                                        /*   Stop in single unit mode        */
  }                                                                             /* <<                                */

  /* Convert back to logarithmic/tropical weights */                            /* --------------------------------- */
  if (nWsrt==FST_WSR_LOG || nWsrt==FST_WSR_TROP)                                /* Loarithmic or tropical weights    */
    for (nT=0; nT<UD_XXT(_this); nT++)                                          /*   Loop over all transitions       */
    {                                                                           /*   >>                              */
      nW = CData_Dfetch(idTd,nT,nIcW);                                          /*     Get probability               */
      nW = nW>0. ? -log(nW) : _this->m_nWceil;                                  /*     Convert to log./trop. weight  */
      CData_Dstore(idTd,nW,nT,nIcW);                                            /*     Store weight                  */
    }                                                                           /*   <<                              */

  /* Clean up */                                                                /* --------------------------------- */
  return O_K;                                                                   /* The end                           */
}

/*
 * Manual page at fst_man.def
 */
INT16 CGEN_PUBLIC CFst_Rcs(CFst* _this, INT32 nUnit, FLOAT64 nSeed)
{
#ifdef __UNENTANGLE_FST

  return IERROR(_this,FST_INTERNAL,__FILE__,__LINE__,0);
  /* NOTE: __UNENTANGLE_FST was defined (probably in dlp_config.h or fst.def).
   *       Undefine it to use this feature!
   */

#else /* #ifdef __UNENTANGLE_FST */

  INT32      nU     = 0;                                                        /* Current unit                       */
  FST_ITYPE nS     = 0;                                                        /* Current state                      */
  FST_ITYPE nS2    = 0;                                                        /* Current state                      */
  FST_ITYPE nFS    = 0;                                                        /* First state of current unit        */
  FST_ITYPE nXS    = 0;                                                        /* Number of states of current unit   */
  FST_ITYPE nT     = 0;                                                        /* Current transition                 */
  FST_ITYPE nFT    = 0;                                                        /* First transition of current unit   */
  FST_ITYPE nXT    = 0;                                                        /* Number of tran. of current unit    */
  CData*    idP    = NULL;                                                     /* Incidence matrix                   */
  CData*    idB    = NULL;                                                     /* Constanct vector of (idP-E)idX=idB */
  CData*    idI    = NULL;                                                     /* idI = (idP-E)^-1                   */
  CData*    idX    = NULL;                                                     /* Solution of (idP-E)idX=idB         */
  FST_WTYPE nPSum  = 0.;                                                       /* Probability sum/state              */
  INT32      nIcW   = -1;                                                       /* Index of probability comp. in td   */
  INT32      nIcRcs = -1;                                                       /* Index of ref. counter comp. in sd  */
  INT32      nIcRct = -1;                                                       /* Index of ref. counter comp. in td  */

  /* Validation */
  CHECK_THIS_RV(NOT_EXEC);
  CFst_Check(_this);

  if (CFst_Wsr_GetType(_this,&nIcW)!=FST_WSR_PROB)
    return IERROR(_this,FST_MISS,"transition probability component",NC_TD_PSR,"transition table");

  /* Initialize - Find or add reference counters */
  nIcRcs = CData_FindComp(AS(CData,_this->sd),NC_SD_RC);
  nIcRct = CData_FindComp(AS(CData,_this->td),NC_TD_RC);
  if (nIcRcs<0)
  {
    CData_AddComp(AS(CData,_this->sd),NC_SD_RC,DLP_TYPE(FST_WTYPE));
    nIcRcs = CData_GetNComps(AS(CData,_this->sd))-1;
  }
  if (nIcRct<0)
  {
    CData_AddComp(AS(CData,_this->td),NC_TD_RC,DLP_TYPE(FST_WTYPE));
    nIcRct = CData_GetNComps(AS(CData,_this->td))-1;
  }

  /* Initialize - Create auxilary instances */
  ICREATEEX(CData,idP,"~CFst_Reverse.idP",NULL);
  ICREATEEX(CData,idB,"~CFst_Reverse.idB",NULL);
  ICREATEEX(CData,idI,"~CFst_Reverse.idI",NULL);
  ICREATEEX(CData,idX,"~CFst_Reverse.idX",NULL);

  /* Loop over units */
  for (nU=nUnit<0?0:nUnit; nU<UD_XXU(_this); nU++)
  {
    CData_Reset(BASEINST(idP),TRUE);
    CData_Reset(BASEINST(idB),TRUE);
    CData_Reset(BASEINST(idI),TRUE);
    CData_Reset(BASEINST(idX),TRUE);
    nFS = UD_FS(_this,nU);
    nXS = UD_XS(_this,nU);
    nFT = UD_FT(_this,nU);
    nXT = UD_XT(_this,nU);

    /* Export transposed ergodic incidence matrix */
    IF_NOK(CData_AddNcomps(idP,DLP_TYPE(FST_WTYPE),UD_XS(_this,nU)+1)) break;
    IF_NOK(CData_Allocate (idP,UD_XS(_this,nU)+1)                    ) break;
    IF_NOK(CData_AddNcomps(idB,DLP_TYPE(FST_WTYPE),1                )) break;
    IF_NOK(CData_Allocate (idB,UD_XS(_this,nU)+1)                    ) break;

    /* Fill transposed incidence matrix
       (summing up probabilities of parallel transitions) */
    for (nT=nFT; nT<nFT+nXT; nT++)
      *(FST_WTYPE*)CData_XAddr(idP,TD_TER(_this,nT),TD_INI(_this,nT)) +=
        *(FST_WTYPE*)CData_XAddr(AS(CData,_this->td),nT,nIcW);

    for (nS=0; nS<nXS; nS++)
    {
      if ((SD_FLG(_this,nS+nFS)&0x01)==0x01)     /* Connect final states with start state */
      {
        for (nS2=1, nPSum=0.; nS2<nXS; nS2++)
          nPSum += *(FST_WTYPE*)CData_XAddr(idP,nS2,nS);

        *(FST_WTYPE*)CData_XAddr(idP,0,nS) = 1.-nPSum;
      }

      *(FST_WTYPE*)CData_XAddr(idP,nS,nS)-=1.;   /* Subtract eigenvalue 1 from main diagonal */
      *(FST_WTYPE*)CData_XAddr(idP,nXS,nS)=1.;   /* Additional equation implementing constraint sum(P_state)=1 */
      *(FST_WTYPE*)CData_XAddr(idP,nS,nXS)=1.;   /* Additional variable making incidence matrix quadratic */
    }

    /* Fill constant vector */
    CData_Fill(idB,CMPLX(1.),CMPLX(0.));

    /* Calculate eigenvector of length 1 and eigenvalue 1
       --> stationary state probabilities */
    CMatrix_Op(idI,idP,T_INSTANCE,NULL,T_IGNORE,OP_INVT);
    CMatrix_Op(idX,idB,T_INSTANCE,idI,T_INSTANCE,OP_MULT);

    /* Fill in state reference counters */
    if (nSeed>0.) nSeed /= CData_Dfetch(idX,0,0); else nSeed = 1.;
    for (nS=0; nS<nXS; nS++)
      CData_Dstore(AS(CData,_this->sd),nSeed*CData_Dfetch(idX,nS,0),nS+nFS,nIcRcs);

    /* Calculate stationary transition probabilities */
    for (nT=nFT; nT<nFT+nXT; nT++)
      CData_Dstore
      (
        AS(CData,_this->td),
        CData_Dfetch(AS(CData,_this->sd),TD_INI(_this,nT)+nFS,nIcRcs) *
          CData_Dfetch(AS(CData,_this->td),nT,nIcW),
        nT,nIcRct
      );

    /* Clean up */
    IDESTROY(idP);
    IDESTROY(idB);
    IDESTROY(idI);
    IDESTROY(idX);

    /* Stop in single unit mode */
    if (nUnit>=0) break;
  }

  return O_K;

#endif /* #ifdef __UNENTANGLE_FST */
}

/* EOF */

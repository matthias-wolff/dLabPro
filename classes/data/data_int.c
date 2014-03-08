/* dLabPro class CData (data)
 * - Private methods
 *
 * AUTHOR : Matthias Eichner
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

#include "dlp_cscope.h" /* Indicate C scope */
#include "dlp_data.h"

/**
 * Finds a given value in selected components of this instance. All search
 * components must be of the same variable type.
 *
 * @param _this      This instance
 * @param nRecStart  First record to search
 * @param nRecEnd    Last record to search plus one
 * @param nCountComp Length of component list
 * @param ...        nCountComp component indices (of type "long"!) to search in AND
 *                   search value
 */
INT32 CGEN_PROTECTED CData_Find(CData* _this, INT32 nRecStart, INT32 nRecEnd, INT32 nCountComp, ...)
{
  INT32    i      = 0;
  INT32    j      = 0;
  INT32    retVal = -1;
  INT32    nComp  = 0;
  va_list ap;

  CHECK_THIS_RV(-1);
  
  if((nRecStart>=0)&&(nRecEnd<=CData_GetNRecs(_this))&&(nRecStart<nRecEnd))
  {
    for(i=nRecStart;i<nRecEnd;i++)
    {
      va_start(ap,nCountComp);
      for(j=0;j<nCountComp;j++)
      {
        nComp=va_arg(ap,long);
        
        switch(CData_GetCompType(_this,nComp))
        {
        case T_UCHAR:
          {
            UINT8 ucWhat;
            ucWhat=(UINT8)va_arg(ap,int);
            if(ucWhat!=(UINT8)(CData_Dfetch(_this,i,nComp))) retVal=-1;
            else retVal=i;
            break;
          }
        case T_CHAR:
          {
            INT8 cWhat;
            cWhat=(INT8)va_arg(ap,int);
            if(cWhat!=(UINT8)(CData_Dfetch(_this,i,nComp))) retVal=-1;
            else retVal=i;
            break;
          }
        case T_USHORT:
          {
            UINT16 usWhat;
            usWhat=(UINT16)va_arg(ap,int);
            if(usWhat!=(UINT16)(CData_Dfetch(_this,i,nComp))) retVal=-1;
            else retVal=i;
            break;
          }
        case T_SHORT:
          {
            INT16 shWhat;
            shWhat=(INT16)va_arg(ap,int);
            if(shWhat!=(INT16)(CData_Dfetch(_this,i,nComp))) retVal=-1;
            else retVal=i;
            break;
          }
        case T_UINT:
          {
            UINT32 iWhat;
            iWhat=(UINT32)va_arg(ap,int);
            if(iWhat!=(UINT32)(CData_Dfetch(_this,i,nComp))) retVal=-1;
            else retVal=i;
            break;
          }
        case T_INT:
          {
            INT32 iWhat;
            iWhat=(INT32)va_arg(ap,int);
            if(iWhat!=(INT32)(CData_Dfetch(_this,i,nComp))) retVal=-1;
            else retVal=i;
            break;
          }
        case T_ULONG:
          {
            UINT64 nWhat;
            nWhat=(UINT64)va_arg(ap,long);
            if(nWhat!=(UINT64)(CData_Dfetch(_this,i,nComp))) retVal=-1;
            else retVal=i;
            break;
          }
        case T_LONG:
          {
            INT64 lWhat;
            lWhat=(INT64)va_arg(ap,long);
            if(lWhat!=(INT64)(CData_Dfetch(_this,i,nComp))) retVal=-1;
            else retVal=i;
            break;
          }
        case T_FLOAT:
          {
            FLOAT32 fWhat;
            fWhat=(FLOAT32)va_arg(ap,double);
            if(fWhat!=(FLOAT32)(CData_Dfetch(_this,i,nComp))) retVal=-1;
            else retVal=i;
            break;
          }
        case T_DOUBLE:
          {
            FLOAT64 dWhat;
            dWhat=(FLOAT64)va_arg(ap,double);
            if(dWhat!=(FLOAT64)(CData_Dfetch(_this,i,nComp))) retVal=-1;
            else retVal=i;
            break;
          }
        case T_COMPLEX:
          {
            COMPLEX64 dWhat;
            dWhat=(COMPLEX64)va_arg(ap,COMPLEX64);
            if(!CMPLX_EQUAL(dWhat,(COMPLEX64)(CData_Cfetch(_this,i,nComp)))) retVal=-1;
            else retVal=i;
            break;
          }
        default:
          {
            char* sWhat;
            sWhat=va_arg(ap,char*);
            if(sWhat==NULL) return -1;
            if(dlp_strcmp(sWhat,(char*)CData_XAddr(_this,i,nComp))) retVal=-1;
            else retVal=i;
            break;
          }
        }
        if(retVal==-1) break;
      }
      if(retVal!=-1) break;
      va_end(ap);
    }
  }

  return retVal;
}

/**
 * Verify mark map and build new one if necessary depending on mark mode.
 *
 * @param _this This instance.
 * @return      O_K if successfull, NOT_EXEC otherwise.
 */
INT16 CGEN_PROTECTED CData_VerifyMarkMap(CData* _this)
{
  INT32 nSize = 0;

  CHECK_THIS_RV(NOT_EXEC);

  /* determine new size of mark map */
  switch(_this->m_markMode)
  {
  case CDATA_MARK_RECS   : nSize = CData_GetNRecs(_this); break;
  case CDATA_MARK_BLOCKS : nSize = CData_GetNBlocks(_this); break;
  case CDATA_MARK_CELLS  : nSize = (INT32)(CData_GetNRecs(_this)*CData_GetNComps(_this)); break;
  case CDATA_MARK_COMPS  : nSize = CData_GetNComps(_this); break;
  default                : DLPASSERT(FMSG("Unknown mark mode."));
  }
  nSize = (INT32)((nSize/8)+1);

  /* allocate new mark map if necessary */
  if(NULL==_this->m_markMap || _this->m_markMapSize!=nSize)
  {
    if(_this->m_markMap) dlp_free(_this->m_markMap);
    _this->m_markMap     = (char*)dlp_calloc(nSize,sizeof(BYTE));
    _this->m_markMapSize = nSize;
  }

  if(NULL==_this->m_markMap) return NOT_EXEC;
  return O_K;
}

/**
 * Set one bit in the mark map.
 *
 * @param _this This instance
 * @param nElem The bit to set
 * @return      O_K if successfull, a (negative) error code otherwise
 */
INT16 CGEN_PROTECTED CData_MarkElem(CData* _this, INT32 nElem)
{
  BYTE nMask = 0x01;

  CHECK_THIS_RV(NOT_EXEC);

  if(_this->m_markMapSize<(INT32)nElem/8) return NOT_EXEC;

  nMask <<= nElem%8;
  *(_this->m_markMap+((INT32)nElem/8)) |= nMask;

  return O_K;
}

/**
 * Evaluate if a certain element in mark map is marked.
 *
 * @param _this This instance.
 * @param nElem Element to eval.
 * @return      TRUE if element is marked, FALSE otherwise.
 */
BOOL CGEN_PROTECTED CData_IsMarked(CData* _this, INT32 nElem)
{
  BYTE nMask = 0x01;

  CHECK_THIS_RV(NOT_EXEC);

  if(_this->m_markMapSize<=0) return FALSE;

  nMask <<= nElem%8;
  if((*(_this->m_markMap+((INT32)nElem/8)) & nMask)) return TRUE;
  
  return FALSE;
}

/**
 * Evaluate if a certain record is marked.
 *
 * @param _this This instance.
 * @param nRec  Record to eval.
 * @return      TRUE if record is marked, FALSE otherwise.
 */
BOOL CGEN_PROTECTED CData_RecIsMarked(CData* _this, INT32 nRec)
{
  CHECK_THIS_RV(NOT_EXEC);
  if(_this->m_markMode!=CDATA_MARK_RECS) return FALSE;
  return CData_IsMarked(_this,nRec);
}


/**
 * Evaluate if a certain component is marked.
 *
 * @param _this This instance.
 * @param nComp Component to eval.
 * @return      TRUE if component is marked, FALSE otherwise.
 */
BOOL CGEN_PROTECTED CData_CompIsMarked(CData* _this, INT32 nComp)
{
  CHECK_THIS_RV(NOT_EXEC);
  if(_this->m_markMode!=CDATA_MARK_COMPS) return FALSE;
  return CData_IsMarked(_this,nComp);
}

/**
 * Evaluate if a certain block is marked.
 *
 * @param _this  This instance.
 * @param nBlock Block to eval.
 * @return       TRUE if block is marked, FALSE otherwise.
 */
BOOL CGEN_PROTECTED CData_BlockIsMarked(CData* _this, INT32 nBlock)
{
  CHECK_THIS_RV(NOT_EXEC);
  if(_this->m_markMode!=CDATA_MARK_BLOCKS) return FALSE;
  return CData_IsMarked(_this,nBlock);
}

/**
 * Evaluate if a certain cell is marked.
 *
 * @param _this This instance.
 * @param nRec  Cell to eval.
 * @return      TRUE if cell is marked, FALSE otherwise.
 */
BOOL CGEN_PROTECTED CData_CellIsMarked(CData* _this, INT32 nCell)
{
  CHECK_THIS_RV(NOT_EXEC);
  if(_this->m_markMode!=CDATA_MARK_CELLS) return FALSE;
  return CData_IsMarked(_this,nCell);
}

/**
 * Polymorphic replacement for <code>MIC_GET_N</code> for component indices. The
 * method returns the numeric value on the methods invocation context's stack
 * top or converts the string at the stack top to a component index through
 * {@link -find_comp CData_FindComp}.
 * 
 * @param _this
 *          Pointer to this instance
 * @param nArg
 *           The index of the method/function argument being retrieved. The
 *           value is (only) used for displaying error messages.
 * @param nPos
 *          The index of the argument by type (for compatibility with dLabPro
 *          2.4)
 * @return The component index. If no method invocation context is set or if
 *         there is no such argument, the method will return -1.
 */
INT32 CGEN_PRIVATE CData_MicGetIc(CData* _this, INT16 nArg, INT16 nPos)
{
  StkItm si;
  char   lpMsg[255];

  if (!CDlpObject_MicGet(BASEINST(_this))->GetX )                               /* Numeric only for dLabPro 2.4      */
     return (INT32)MIC_GET_N(nArg,nPos);                                        /* |                                 */
  if (_this->m_bRec || _this->m_bBlock) return (INT32)MIC_GET_N(nArg,nPos);     /* Numeric only in /rec, /block mode */
  
  if (!MIC_GET_X(nArg,&si)) return -1;                                          /* Pop stack top                     */
  switch (si.nType)                                                             /* Branch for stack item type        */
  {                                                                             /* >>                                */
    case T_BOOL    : return (INT32)(si.val.b?1.:0.);                            /*   Boolean : default cast          */
    case T_DOUBLE  : return (INT32)si.val.n.x;                                  /*   Double  : ok                    */
    case T_COMPLEX : return (INT32)si.val.n.x;                                  /*   Complex : ok                    */
    case T_INSTANCE:                                                            /*   Instance: type cast error       */
      sprintf(lpMsg,"argument %hd from instance",(short)nArg);                  /*     Prepare error message         */
      IERROR(_this,DATA_CNVT,lpMsg,"number",0);                                 /*     Error message                 */
      return -1;                                                                /*     "No component"                */
    case T_STRING  : return CData_FindComp(_this,si.val.s);                     /*   String  : find comp. by name    */
    case T_STKBRAKE: return -1;                                                 /*   Brake   : (must not happen)     */ 
    default        : DLPASSERT(FMSG("Unknown type of stack item"));             /*   others  : (must not happen)     */ 
  }                                                                             /* <<                                */
  return -1;                                                                    /* Default return value              */
}

/* EOF */

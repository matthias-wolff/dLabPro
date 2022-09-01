/* dLabPro class CVar (var)
 * - Implementation file
 *
 * AUTHOR : Christian-M. Westendorf, Matthias Wolff 
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

#include "dlp_cscope.h"
#include "dlp_var.h"

/* -- Interactive API -- */

/*
 * Manual page at var.def
 */
INT16 CGEN_PUBLIC CVar_Status(CVar *_this)
{
  char lpsBuf[255];
  
  CVar_PopOwnValue(_this);
  printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());
  printf("\n   Status of instance");
  printf("\n   var %s\n",BASEINST(_this)->m_lpInstanceName);
  dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());

  switch (_this->m_nType)
  {
    case T_BOOL:
      printf("\n   Type : boolean");
      printf("\n   Value: %d",(int)_this->m_bBVal);
      break;
    case T_DOUBLE:
      dlp_sprintc(lpsBuf,_this->m_nNVal,FALSE);
      printf("\n   Type : double");
      printf("\n   Value: %s", lpsBuf);
      break;
    case T_COMPLEX:
      dlp_sprintc(lpsBuf,_this->m_nNVal,FALSE);
      printf("\n   Type : complex");
      printf("\n   Value: %s", lpsBuf);
      break;
    case T_STRING:
      printf("\n   Type : string");
      printf("\n   Value: \"%s\"",_this->m_lpsSVal?_this->m_lpsSVal:"(null)");
      break;
    case T_INSTANCE:
      printf("\n   Type : instance");
      printf("\n   Value: %s",CDlpObject_GetFQName(BASEINST(_this)->m_iAliasInst,lpsBuf,255));
      break;
    case T_RDOUBLE:
      printf("\n   Type             : double random" );
      printf("\n   Low boundary     : %8lg", (double)_this->m_nLow    );
      printf("\n   High boundary    : %8lg", (double)_this->m_nHi     );
      printf("\n   Step size        : %8lg", (double)_this->m_nDelta  );
      printf("\n   Most recent value: %8lg", (double)_this->m_nNVal.x );
      break;
    case T_RDDATA:
      printf("\n   Type             : numeric random data selector");
      printf("\n   Data table       : %s",CDlpObject_GetFQName(_this->m_idRndSet,lpsBuf,255));
      printf("\n   Component index  : %5ld",(long)_this->m_nInd );
      printf("\n   Component type   : %5ld",(long)_this->m_nType);
      printf("\n   Most recent value: %8lg",(double)_this->m_nNVal.x);
      break;
    case T_RSDATA:
      printf("\n   Type             : string random data selector");
      printf("\n   Data table       : %s",CDlpObject_GetFQName(_this->m_idRndSet,lpsBuf,255));
      printf("\n   Component index  : %5ld", (long)_this->m_nInd );
      printf("\n   Component type   : %5ld", (long)_this->m_nType);
      printf("\n   Most recent value: %s",_this->m_lpsSVal?_this->m_lpsSVal:"(null)");
    default:
      DLPASSERT(FMSG("Unknown variable type"));
  }

  printf("\n");
  dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());
  printf("\n");

  return O_K;
}

/* -- Protected API -- */

/**
 * Polymorphic set operations.
 * 
 * @param lpsOpname
 *          <p>Operator name</p>
 *          <table>
 *            <tr><th><code>lpsOpname</code></th><th>Boolean</th><th>Numeric</th><th>String</th><th>Instance</th></tr>
 *            <tr><td><code>=  </code></td><td>x</td><td>x</td><td>x</td><td>x</td></tr>
 *            <tr><td><code>+= </code></td><td>x</td><td>x</td><td>x</td><td>-</td></tr>
 *            <tr><td><code>-= </code></td><td>-</td><td>x</td><td>-</td><td>-</td></tr>
 *            <tr><td><code>*= </code></td><td>x</td><td>x</td><td>-</td><td>-</td></tr>
 *            <tr><td><code>/= </code></td><td>-</td><td>x</td><td>-</td><td>-</td></tr>
 *            <tr><td><code>++=</code></td><td>-</td><td>x</td><td>-</td><td>-</td></tr>
 *            <tr><td><code>--=</code></td><td>-</td><td>x</td><td>-</td><td>-</td></tr>
 *          </table>
 *          <p>For type conversion rules see
 *            <code><a href="function.html"><code�class="link">CFunction</code></a><code>::StackLogic</code>,
 *            <code><a href="function.html"><code�class="link">CFunction</code></a><code>::StackNumber</code>,
 *            <code><a href="function.html"><code�class="link">CFunction</code></a><code>::StackString</code> and
 *            <code><a href="function.html"><code�class="link">CFunction</code></a><code>::StackInstance</code>.
 *          </p>
 * @return <code>O_K</code> if successfull, a (negative) error code otherwise
 */
INT16 CGEN_PROTECTED CVar_SetOp(CVar *_this,const char* lpsOpname)
{
  StkItm si;
  if (dlp_strcmp(lpsOpname,"++=")!=0 && dlp_strcmp(lpsOpname,"--=")!=0)
  {
    if (!CDlpObject_MicGet(BASEINST(_this))->GetX)
      return
        IERROR(_this,VAR_NOTSUPPORTED,"Polymorphic signatures"," by caller",0);
    if (!MIC_GET_X(1,&si)) return NOT_EXEC;
  }
    
  if (dlp_strcmp(lpsOpname,"=")==0)
    switch (si.nType)
    {
      case T_BOOL    : return CVar_Bset(_this,si.val.b);
      case T_COMPLEX : return CVar_Vset(_this,si.val.n);
      case T_STRING  : return CVar_Sset(_this,si.val.s);
      case T_INSTANCE: return CVar_Iset(_this,si.val.i);
      default:
        DLPASSERT(FMSG("Unknown variable type"));
        return NOT_EXEC;
    }
  else if (dlp_strcmp(lpsOpname,"+=")==0)
  {
    if (_this->m_nType==T_COMPLEX) return CVar_Vset(_this,CMPLX_PLUS(_this->m_nNVal,si.val.n));
    if (_this->m_nType==T_STRING)
    {
      char* lpsSi = NULL;
      MIC_PUT_X(&si);
      lpsSi = MIC_GET_S(1,0);
      _this->m_lpsSVal = (char*)dlp_realloc(_this->m_lpsSVal,
        dlp_strlen(_this->m_lpsSVal)+dlp_strlen(lpsSi)+1,sizeof(char));
      if (!_this->m_lpsSVal) return IERROR(_this,ERR_NOMEM,0,0,0);
      dlp_strcat(_this->m_lpsSVal,lpsSi);
      return O_K;
    }
    if (_this->m_nType==T_BOOL)
    {
      MIC_PUT_X(&si);
      _this->m_bBVal|=MIC_GET_B(1,0);
      return O_K;
    }
    return
      IERROR(_this,VAR_NOTSUPPORTED,"Operator +="," for this variable type",0);
  }
  else if (dlp_strcmp(lpsOpname,"*=")==0)
  {
    if (_this->m_nType==T_COMPLEX) return CVar_Vset(_this,CMPLX_MULT(_this->m_nNVal,si.val.n));
    if (_this->m_nType==T_BOOL)
    {
      MIC_PUT_X(&si);
      _this->m_bBVal&=MIC_GET_B(1,0);
      return O_K;
    }
    return
      IERROR(_this,VAR_NOTSUPPORTED,"Operator *="," for this variable type",0);
  }
  else if (dlp_strcmp(lpsOpname,"-=")==0)
  {
    if (_this->m_nType==T_COMPLEX) return CVar_Vset(_this,CMPLX_MINUS(_this->m_nNVal,si.val.n));
    return
      IERROR(_this,VAR_NOTSUPPORTED,"Operator -="," for this variable type",0);
  }
  else if (dlp_strcmp(lpsOpname,"/=")==0)
  {
    if (_this->m_nType==T_COMPLEX) return CVar_Vset(_this,CMPLX_MULT(_this->m_nNVal,CMPLX_INVT(si.val.n)));
    return
      IERROR(_this,VAR_NOTSUPPORTED,"Operator /="," for this variable type",0);
  }
  else if (dlp_strcmp(lpsOpname,"++=")==0)
  {
    if (_this->m_nType==T_COMPLEX) return CVar_Vset(_this,CMPLX_INC(_this->m_nNVal));
    return
      IERROR(_this,VAR_NOTSUPPORTED,"Operator ++="," for this variable type",0);
  }
  else if (dlp_strcmp(lpsOpname,"--=")==0)
  {
    if (_this->m_nType==T_COMPLEX) return CVar_Vset(_this,CMPLX_DEC(_this->m_nNVal));
    return 
      IERROR(_this,VAR_NOTSUPPORTED,"Operator --="," for this variable type",0);
  }
  
  return NOT_EXEC;
}

/**
 * Removes the variable's value from the stack. This method <em>must</em> be
 * called from all interactive methods.
 * 
 * @see Exec
 */
void CGEN_PROTECTED CVar_PopOwnValue(CVar *_this)
{
  switch (_this->m_nType)
  {
    case T_BOOL:
      CDlpObject_MicGetB(BASEINST(_this),-1,0);
      break;
    case T_COMPLEX:
    case T_RDOUBLE:
    case T_DOUBLE:
    case T_RDDATA:
      CDlpObject_MicGetN(BASEINST(_this),-1,0);
      break;
    case T_STRING:
    case T_RSDATA:
      CDlpObject_MicGetS(BASEINST(_this),-1,0);
      break;
    case T_INSTANCE:
      CDlpObject_MicGetI(BASEINST(_this),-1,0);
      break;
    default: DLPASSERT(FMSG("Unknown variable type"));
  }
}

/**
 * Implementation of the class function. Leaves the value of the variable on
 * the stack.
 */
void CGEN_PROTECTED CVar_Exec(CVar *_this)
{
  switch (_this->m_nType)
  {
    case T_BOOL:
      CDlpObject_MicPutB(BASEINST(_this),_this->m_bBVal);
      break;
    case T_DOUBLE:
    case T_COMPLEX:
      CDlpObject_MicPutN(BASEINST(_this),_this->m_nNVal);
      break;
    case T_STRING:
      CDlpObject_MicPutS(BASEINST(_this),_this->m_lpsSVal);
      break;
    case T_INSTANCE:
      CDlpObject_MicPutI(BASEINST(_this),BASEINST(_this)->m_iAliasInst);
      break;
    case T_RDOUBLE: {
      _this->m_nInd  = (INT32)(dlp_rand()/_this->m_nNorm);
      _this->m_nNVal = CMPLX(_this->m_nLow+_this->m_nInd*_this->m_nDelta);
      CDlpObject_MicPutN(BASEINST(_this),_this->m_nNVal);
      break; }
    case T_RDDATA:
      _this->m_nInd  = (INT32)(dlp_rand()/_this->m_nNorm);
      _this->m_nNVal = CData_Cfetch(AS(CData,_this->m_idRndSet),_this->m_nInd, _this->m_nIcomp);
      CDlpObject_MicPutN(BASEINST(_this),_this->m_nNVal);
      break;
    case T_RSDATA:
      _this->m_nInd = (INT32)(dlp_rand()/_this->m_nNorm);
      CVar_Sset(_this,(char*)CData_XAddr(AS(CData,_this->m_idRndSet),_this->m_nInd,_this->m_nIcomp));
      _this->m_nType = T_RSDATA;
      CDlpObject_MicPutS(BASEINST(_this),_this->m_lpsSVal);
      break;
    default: DLPASSERT(FMSG("Unknown variable type"));
  }
}

/* EOF */

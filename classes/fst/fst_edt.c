/* dLabPro class CFst (fst)
 * - Low level editing methods
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

#include "dlp_cscope.h" /* Indicate C scope */
#include "dlp_fst.h"

/**
 * Adds one unit (= one finite state automaton graph) to the instance.
 *
 * @param _this   Automaton instance
 * @param lpsName Name of the unit (may be NULL)
 * @return        The non-negative unit index if successfull, a (negative)
 *                error code otherwise
 */
INT32 CGEN_PUBLIC CFst_Addunit(CFst* _this, const char* lpsName)
{
  CData* idAux = NULL;
  INT16  nLen  = 0;
  INT32   nU    = 0;
  INT32   i     = 0;

  /* Validation */
  CHECK_THIS_RV(NOT_EXEC);
  CFst_Check(_this);

  /* Create / modify component structure */
  if (_this->m_bFsa || _this->m_bFst)                                           /* Wanna have FSA ot FST?            */
  {                                                                             /* >> (oh yeah)                      */
    INT32 nIcTis = CData_FindComp(AS(CData,_this->td),NC_TD_TIS);                /*   Find input symbol component     */
    INT32 nIcTos = CData_FindComp(AS(CData,_this->td),NC_TD_TOS);                /*   Find output symbol component    */
    if (nIcTis<0)                                                               /*   Have no TIS comp. ...           */
      CData_AddComp(AS(CData,_this->td),NC_TD_TIS,DLP_TYPE(FST_STYPE));         /*     ... create it                 */
    if (_this->m_bFst && nIcTos<0)                                              /*   Wanna FST and have no TOS comp..*/
      CData_AddComp(AS(CData,_this->td),NC_TD_TOS,DLP_TYPE(FST_STYPE));         /*     ... cretae it                 */
    if (_this->m_bFsa && nIcTos>=0)                                             /*   Wanna FSA and HAVE TOS comp. ...*/
    {                                                                           /*   >>                              */
      IERROR(_this,FST_DELETE,"component",NC_TD_TOS,"transition table");        /*     Warning!                      */
      CData_DeleteComps(AS(CData,_this->td),nIcTos,1);                          /*     Delete output symbol comps.   */
    }                                                                           /*   <<                              */
  }                                                                             /* <<                                */

  if (_this->m_bPsr || _this->m_bLsr || _this->m_bTsr)                          /* Wanna weighted one                */
  {
    INT32 nIcW   = -1;
    INT16 nWsrt = CFst_Wsr_GetType(_this,&nIcW);
    if (nIcW<0)
    {
      if (_this->m_bPsr)
        CData_AddComp(AS(CData,_this->td),NC_TD_PSR,DLP_TYPE(FST_WTYPE));
      else if (_this->m_bLsr)
        CData_AddComp(AS(CData,_this->td),NC_TD_LSR,DLP_TYPE(FST_WTYPE));
      else if (_this->m_bTsr)
        CData_AddComp(AS(CData,_this->td),NC_TD_TSR,DLP_TYPE(FST_WTYPE));
    }
    else
    {
      if (_this->m_bPsr)
      {
        if (nWsrt!=FST_WSR_PROB)
          IERROR(_this,FST_CHANGE,"weight semiring type","probability",0);
        CData_SetCname(AS(CData,_this->td),nIcW,NC_TD_PSR);
      }
      else if (_this->m_bLsr)
      {
        if (nWsrt!=FST_WSR_LOG)
          IERROR(_this,FST_CHANGE,"weight semiring type","logarithmic",0);
        CData_SetCname(AS(CData,_this->td),nIcW,NC_TD_LSR);
      }
      else if (_this->m_bTsr)
      {
        if (nWsrt!=FST_WSR_TROP)
          IERROR(_this,FST_CHANGE,"weight semiring type","tropical",0);
        CData_SetCname(AS(CData,_this->td),nIcW,NC_TD_TSR);
      }
    }
  }

  /* Allocate a new unit */
  nU = CData_AddRecs(AS(CData,_this->ud),1,_this->m_nGrany);

  /* Enlarge length of unit name component */
  nLen = (INT16)dlp_strlen(lpsName);
  if (nLen>255) nLen=255;
  if (nLen>=CData_GetCompType(AS(CData,_this->ud),0)-1)
  {
    ICREATEEX(CData,idAux,"CFst_Addunit~idAux",NULL);

    CData_Copy(BASEINST(idAux),_this->ud);
    CData_Reset(_this->ud,TRUE);
    CData_AddComp(AS(CData,_this->ud),CData_GetCname(idAux,0),(INT16)(nLen+1));
    CData_AllocateUninitialized(AS(CData,_this->ud),CData_GetNRecs(idAux));

    for (i=0; i<CData_GetNRecs(idAux); i++)
      dlp_strcpy((char*)CData_XAddr(AS(CData,_this->ud),i,0),(
        const char*)CData_XAddr(idAux,i,0));

    CData_Delete(idAux,idAux,0,1);
    CData_Join(AS(CData,_this->ud),idAux);

    IDESTROY(idAux);
  }

  /* Store data */
  CData_Sstore(AS(CData,_this->ud),lpsName      ,nU,IC_UD_NAME);
  CData_Dstore(AS(CData,_this->ud),0.           ,nU,IC_UD_XS  );
  CData_Dstore(AS(CData,_this->ud),0.           ,nU,IC_UD_XT  );
  CData_Dstore(AS(CData,_this->ud),UD_XXS(_this),nU,IC_UD_FS  );
  CData_Dstore(AS(CData,_this->ud),UD_XXT(_this),nU,IC_UD_FT  );

  CFst_Check(_this); /* TODO: Remove after debugging */
  return nU;
}

/*
 * Manual page at fst_man.def
 */
INT16 CGEN_PUBLIC CFst_AddunitIam(CFst* _this, const char* lpsName)
{
  INT16 nEc                  = O_K;
  INT32  nU                   = 0;
  char  lpsInit[L_INPUTLINE] = "";

  /* Validation */
  CHECK_THIS_RV(NOT_EXEC);

  /* Add a unit */
  if ((nU=CFst_Addunit(_this,lpsName))<0)
    return IERROR(_this,FST_INTERNAL,__FILE__,__LINE__,"");

  /* Initialize unit description */
  if (CData_ReadInitializer(AS(CData,_this->ud),lpsInit,L_INPUTLINE,FALSE))
    IF_NOK((nEc=CData_InitializeRecordEx(AS(CData,_this->ud),lpsInit,nU,IC_UD_DATA)))
      return IERROR(_this,FST_INTERNAL,__FILE__,__LINE__,"");

  return O_K;
}

/**
 * Adds <code>nCount</code> states to unit <code>nUnit</code>.
 *
 * <h3>TODO:</h3><p>Return unit relative state index?</p>
 *
 * @param _this  Automaton instance
 * @param nUnit  The unit to add states to
 * @param nCount The number of states to add
 * @param bFinal Specifies if the new states are final states
 * @return       The non-negative absolute index of the first new state in the
 *               {@link sd state table} if successfull, a (negative) error code
 *               otherwise
 */
INT32 CGEN_PUBLIC CFst_Addstates(CFst* _this, INT32 nUnit, INT32 nCount, BOOL bFinal)
{
  INT32 nU  = 0;
  INT32 nS  = 0;
  INT32 nFS = 0;

  /* Validation */
  CHECK_THIS_RV(NOT_EXEC);
  CFst_Check(_this);
  if (nCount<=0 || nUnit<0 || nUnit>=UD_XXU(_this)) return NOT_EXEC;

  /* Insert nCount new states into state table */
  nFS = UD_FS(_this,nUnit)+UD_XS(_this,nUnit);
  DLPASSERT(CData_InsertRecs(AS(CData,_this->sd),nFS,nCount,_this->m_nGrany)==nFS);

  /* Adjust unit descriptions */
  UD_XS(_this,nUnit)+=nCount;
  for (nU=nUnit+1; nU<UD_XXU(_this); nU++)
    UD_FS(_this,nU)+=nCount;

  /* Initialize new states */
  for (nS=nFS; nS<nFS+nCount; nS++)
    SD_FLG(_this,nS)=bFinal?0x01:0x00;

  CFst_Check(_this); /* TODO: Remove after debugging */
  return nFS;
}

/*
 * Manual page at fst_man.def
 */
INT16 CGEN_PUBLIC CFst_AddstatesIam(CFst* _this, INT32 nUnit, INT32 nCount)
{
  INT16 nEc                  = O_K;
  INT32 nS                   = 0;
  INT32 nFS                  = 0;
  char  lpsInit[L_INPUTLINE] = "";

  /* Validation */
  CHECK_THIS_RV(NOT_EXEC);
  if (nCount<=0) return NOT_EXEC;
  if (nUnit<0 || nUnit>=UD_XXU(_this))
    return IERROR(_this,FST_BADID,"unit",nUnit,0);

  /* Add new states */
  if ((nFS=CFst_Addstates(_this,nUnit,nCount,_this->m_bFinal))<0)
    return IERROR(_this,FST_INTERNAL,__FILE__,__LINE__,"");

  /* Initialize state descriptions */
  if (CData_ReadInitializer(AS(CData,_this->sd),lpsInit,L_INPUTLINE,FALSE))
    for (nS=nFS; nS<nFS+nCount; nS++)
      IF_NOK((nEc=CData_InitializeRecordEx(AS(CData,_this->sd),lpsInit,nS,IC_SD_DATA)))
        return IERROR(_this,FST_INTERNAL,__FILE__,__LINE__,"");

  return O_K;
}

/**
 * Adds one state to unit <code>nUnit</code> and copies its qualification
 * (including the final state flag) from a source state. The compatibility of
 * the qualifications will <i>not</i> be checked!
 *
 * @param _this Destination automaton instance
 * @param nUnit The unit to add the state to
 * @param itSrc Source automaton instance
 * @param nSsrc Absolute index of source state in the source automaton's
 *              {@link sd state table} <code>itSrc->sd</code>
 * @return      The non-negative absolute index of the new state in the
 *              destination automaton's {@link sd state table} if successfull,
 *              a (negative) error code otherwise
 */
FST_ITYPE CGEN_PROTECTED CFst_AddstateCopy(CFst* _this, INT32 nUnit, CFst* itSrc, FST_ITYPE nSsrc)
{
  FST_ITYPE nSdst = -1;

  CHECK_THIS_RV(NOT_EXEC);
  DLPASSERT(CData_GetRecLen(AS(CData,_this->sd))>=CData_GetRecLen(AS(CData,itSrc->sd)));

  nSdst = CFst_Addstates(_this,nUnit,1,FALSE);
  dlp_memmove
  (
    CData_XAddr(AS(CData,_this->sd),nSdst,0),
    CData_XAddr(AS(CData,itSrc->sd),nSsrc,0),
    CData_GetRecLen(AS(CData,itSrc->sd))
  );

  return nSdst;
}

/**
 * Adds one transition between state <code>nIni</code> and state
 * <code>nTer</code> to unit <code>nUnit</code>.
 *
 * @param _this Automaton instance
 * @param nUnit The unit to add the transition to
 * @param nIni  Unit relative initial state index of new transition
 * @param nTer  Unit relative terminal state index of new transition
 * @return      The non-negative absolute index of the new transition in the
 *              transition table if successfull, a (negative) error code
 *              otherwise
 */
INT32 CGEN_PUBLIC CFst_Addtrans(CFst* _this, INT32 nUnit, FST_ITYPE nIni, FST_ITYPE nTer)
{
  INT16 nSrt = 0;

  CHECK_THIS_RV(NOT_EXEC);

  nSrt=CFst_Wsr_GetType(_this,NULL);
  return CFst_AddtransEx(_this,nUnit,nIni,nTer,-1,-1,CFst_Wsr_NeMult(nSrt));
}

/**
 * Adds one transition between state <code>nIni</code> and state
 * <code>nTer</code> to unit <code>nUnit</code>.
 *
 * @param _this Automaton instance
 * @param nUnit The unit to add transition to
 * @param nIni  Unit relative initial state index of new transition
 * @param nTer  Unit relative terminal state index of new transition
 * @param nTis  Input Symbol of new transition
 * @param nTos  Output Symbol of new transition
 * @param nW    Weight of new transition
 * @return      The non-negative absolute index of the new transition in the
 *              {@link td transition table} if successfull, a (negative) error
 *              code otherwise
 */
INT32 CGEN_PUBLIC CFst_AddtransEx
(
  CFst*     _this,
  INT32      nUnit,
  FST_ITYPE nIni,
  FST_ITYPE nTer,
  FST_STYPE nTis,
  FST_STYPE nTos,
  FST_WTYPE nW
)
{
  INT32 nC  = 0;
  INT32 nU  = 0;
  INT32 nFT = 0;

  /* Validation */
  CHECK_THIS_RV(NOT_EXEC);
  CFst_Check(_this);
  if (nUnit<0 || nUnit> UD_XXU(_this      )) return NOT_EXEC;
  if (nIni <0 || nIni >=UD_XS (_this,nUnit)) return NOT_EXEC;
  if (nTer <0 || nTer >=UD_XS (_this,nUnit)) return NOT_EXEC;

  /* Insert one new transition into transition table */
  nFT = UD_FT(_this,nUnit)+UD_XT(_this,nUnit);
  DLPASSERT(CData_InsertRecs(AS(CData,_this->td),nFT,1,_this->m_nGrany)==nFT);

  /* Adjust unit descriptions */
  UD_XT(_this,nUnit)++;
  for (nU=nUnit+1; nU<UD_XXU(_this); nU++) UD_FT(_this,nU)++;

  /* Initialize new transition */
  TD_INI(_this,nFT) = (FST_ITYPE)nIni;
  TD_TER(_this,nFT) = (FST_ITYPE)nTer;

  /* Store input symbol */
  if ((nC=CData_FindComp(AS(CData,_this->td),NC_TD_TIS))>=IC_TD_DATA)
    CData_Dstore(AS(CData,_this->td),nTis,nFT,nC);

  /* Store output symbol */
  if ((nC=CData_FindComp(AS(CData,_this->td),NC_TD_TOS))>=IC_TD_DATA)
    CData_Dstore(AS(CData,_this->td),nTos,nFT,nC);

  /* Store weight */
  if (CFst_Wsr_GetType(_this,&nC)!=0)
    CData_Dstore(AS(CData,_this->td),nW,nFT,nC);

  CFst_Check(_this); /* TODO: Remove after debugging */
  return nFT;
}

/*
 * Manual page at fst_man.def
 */
INT16 CGEN_PUBLIC CFst_AddtransIam(CFst* _this, INT32 nUnit, INT32 nIni, INT32 nTer)
{
  INT16 nEc                  = O_K;
  INT32 nFT                  = 0;
  char  lpsInit[L_INPUTLINE] = "";

  /* Validation */
  CHECK_THIS_RV(NOT_EXEC);
  if (nUnit<0 || nUnit>=UD_XXU(_this      )) return IERROR(_this,FST_BADID,"unit" ,nUnit,0);
  if (nIni <0 || nIni >=UD_XS (_this,nUnit)) return IERROR(_this,FST_BADID,"state",nIni ,0);
  if (nTer <0 || nTer >=UD_XS (_this,nUnit)) return IERROR(_this,FST_BADID,"state",nTer ,0);

  /* Add new transition */
  if ((nFT=CFst_Addtrans(_this,nUnit,nIni,nTer))<0)
    return IERROR(_this,FST_INTERNAL,__FILE__,__LINE__,"");

  /* Make terminal state final */
  if (_this->m_bFinal) SD_FLG(_this,nTer)|=SD_FLG_FINAL;

  /* Initialize transition description */
  if (CData_ReadInitializer(AS(CData,_this->td),lpsInit,L_INPUTLINE,FALSE))
    IF_NOK((nEc=CData_InitializeRecordEx(AS(CData,_this->td),lpsInit,nFT,IC_TD_DATA)))
      return IERROR(_this,FST_INTERNAL,__FILE__,__LINE__,"");

  return O_K;
}

/**
 * Adds one transition between state <code>nIni</code> and state
 * <code>nTer</code> to unit <code>nUnit</code> and copies its qualification
 * from a source transition. The compatibility of the qualifications will
 * <i>not</i> be checked!
 *
 * @param _this Automaton instance
 * @param nUnit The unit to add the transition to
 * @param nIni  Unit relative initial state index of new transition
 * @param nTer  Unit relative terminal state index of new transition
 * @param itSrc Source automaton instance
 * @param nTsrc Absolute index of source transition in the source automaton's
 *              {@link td transition table} <code>itSrc->td</code>
 * @return      The non-negative absolute index of the new transition in the
 *              {@link td transition table} if successfull, a (negative) error
 *              code otherwise
 */
INT32 CGEN_PUBLIC CFst_AddtransCopy
(
  CFst*     _this,
  INT32      nUnit,
  FST_ITYPE nIni,
  FST_ITYPE nTer,
  CFst*     itSrc,
  FST_ITYPE nTsrc
)
{
  FST_ITYPE nTdst = -1;

  CHECK_THIS_RV(NOT_EXEC);
  DLPASSERT(CData_GetRecLen(AS(CData,_this->td))==CData_GetRecLen(AS(CData,itSrc->td)));

  IFCHECKEX(2)
    printf("\n  CFst_AddtransCopy(\"%s\",%ld,%ld,%ld,\"%s\",%ld)",
  		BASEINST(_this)->m_lpInstanceName,(long)nUnit,(long)nIni,(long)nTer,
  		BASEINST(itSrc)->m_lpInstanceName,(long)nTsrc);
  nTdst = CFst_Addtrans(_this,nUnit,nIni,nTer);
  if (CData_GetNComps(AS(CData,_this->td))>=IC_TD_DATA)
    dlp_memmove
    (
      CData_XAddr(AS(CData,_this->td),nTdst,IC_TD_DATA),
      CData_XAddr(AS(CData,itSrc->td),nTsrc,IC_TD_DATA),
      CData_GetRecLen(AS(CData,_this->td))
      - CDlpTable_GetCompOffset(AS(CData,_this->td)->m_lpTable,IC_TD_DATA)
    );

  return nTdst;
}

/*
 * Manual page at fst_man.def
 */
INT16 CGEN_PUBLIC CFst_Delunit(CFst* _this, INT32 nUnit)
{
  INT32 nU  = 0;
  INT32 nXS = 0;
  INT32 nXT = 0;

  /* Validation*/
  CHECK_THIS_RV(NOT_EXEC);
  CFst_Check(_this);
  if (nUnit<0 || nUnit>=UD_XXU(_this))
    return IERROR(_this,FST_BADID,"unit",nUnit,0);

  /* Initialization */
  nXS = UD_XS(_this,nUnit);
  nXT = UD_XT(_this,nUnit);

  /* Delete data content */
  CData_DeleteRecs(AS(CData,_this->sd),UD_FS(_this,nUnit),nXS);
  CData_DeleteRecs(AS(CData,_this->td),UD_FT(_this,nUnit),nXT);
  CData_DeleteRecs(AS(CData,_this->ud),nUnit             ,1  );

  /* Adjust unit descriptions */
  for (nU=nUnit; nU<UD_XXU(_this); nU++)
  {
    UD_FS(_this,nU)-=nXS;
    UD_FT(_this,nU)-=nXT;
  }

  CFst_Check(_this); /* TODO: Remove after debugging */
  return O_K;
}

/*
 * Manual page at fst_man.def
 */
INT16 CGEN_PUBLIC CFst_Delstate(CFst* _this, INT32 nUnit, INT32 nState)
{
  INT32 nU   = 0;
  INT32 nT   = 0;
  INT32 nFT  = 0;
  INT32 nXT  = 0;
  INT32 nIni = 0;
  INT32 nTer = 0;

  /* Validation*/
  CHECK_THIS_RV(NOT_EXEC);
  CFst_Check(_this);
  if (nUnit <0 || nUnit >=UD_XXU(_this      )) return IERROR(_this,FST_BADID,"unit" ,nUnit ,0);
  if (nState<0 || nState>=UD_XS (_this,nUnit)) return IERROR(_this,FST_BADID,"state",nState,0);

  /* Delete transitions starting or ending in nState */
  nFT = UD_FT(_this,nUnit);
  nXT = UD_XT(_this,nUnit);

  for(nT=nFT; nT<nFT+nXT; )
  {
    nIni = TD_INI(_this,nT);
    nTer = TD_TER(_this,nT);

    if ((nIni==nState)||(nTer==nState))
    {
      CFst_Deltrans(_this,nUnit,nT);
      nXT--;
    }
    else
    {
      if (nIni>nState) TD_INI(_this,nT)--;
      if (nTer>nState) TD_TER(_this,nT)--;
      nT++;
    }
  }

  /* Remove from state table */
  CData_DeleteRecs(AS(CData,_this->sd),UD_FS(_this,nUnit)+nState,1);

  /* Adjust unit descriptions */
  UD_XS(_this,nUnit)--;
  for (nU=nUnit+1; nU<UD_XXU(_this); nU++) UD_FS(_this,nU)--;

  CFst_Check(_this); /* TODO: Remove after debugging */
  return O_K;
}

/*
 * Manual page at fst_man.def
 */
INT16 CGEN_PUBLIC CFst_Deltrans(CFst* _this, INT32 nUnit, INT32 nTrans)
{
  INT32 nU = 0;

  /* Validation*/
  CHECK_THIS_RV(NOT_EXEC);
  CFst_Check(_this);
  if (nUnit<0 || nUnit>=UD_XXU(_this))
    return IERROR(_this,FST_BADID,"unit",nUnit,0);
  if (nTrans<UD_FT(_this,nUnit) || nTrans>=UD_FT(_this,nUnit)+UD_XT(_this,nUnit))
    return IERROR(_this,FST_BADID,"transition",nTrans,0);

  /* Remove from transition table */
  CData_DeleteRecs(AS(CData,_this->td),nTrans,1);

  /* Adjust unit descriptions */
  UD_XT(_this,nUnit)--;
  for (nU=nUnit+1; nU<UD_XXU(_this); nU++) UD_FT(_this,nU)--;

  CFst_Check(_this); /* TODO: Remove after debugging */
  return O_K;
}

/*
 * Manual page at fst_man.def
 */
INT16 CGEN_PUBLIC CFst_Reverse(CFst* _this, INT32 nUnit)
{
  INT32      nU             = 0;                                                /* Current unit                       */
  FST_ITYPE nS             = 0;                                                /* Current state                      */
  FST_ITYPE nFS            = 0;                                                /* First state of current unit        */
  FST_ITYPE nXS            = 0;                                                /* Number of states of current unit   */
  FST_ITYPE nT             = 0;                                                /* Current transition                 */
  FST_ITYPE nFT            = 0;                                                /* First transition of current unit   */
  FST_ITYPE nXT            = 0;                                                /* Number of tran. of current unit    */
  FST_ITYPE nIni           = 0;                                                /* Initial state of curr. transition  */
  FST_ITYPE nTer           = 0;                                                /* Terminal state of curr. transition */
  FST_ITYPE nAbsFinalState = 0;                                                /* The unified final state            */
  INT32      nIcRcs         = -1;                                               /* Index of ref. ctr. component in sd */
  INT32      nIcRct         = -1;                                               /* Index of ref. ctr. component in td */

  /* Validation*/
  CHECK_THIS_RV(NOT_EXEC);
  CFst_Check(_this);

  for (nU=nUnit<0?0:nUnit; nU<UD_XXU(_this); nU++)
  {
    /* Make sure there is a single final state */
    if(!_this->m_bPsr) _this->m_bLocal=TRUE;                                   /* set local option to prevent trim from changing probabilities */
    CFst_Fsunify(_this,nU);

    /* Initialize */
    nFS    = UD_FS(_this,nU);
    nXS    = UD_XS(_this,nU);
    nFT    = UD_FT(_this,nU);
    nXT    = UD_XT(_this,nU);
    nIcRcs = CData_FindComp(AS(CData,_this->sd),NC_SD_RC);
    nIcRct = CData_FindComp(AS(CData,_this->td),NC_TD_RC);

    /* Identify the final state */
    for (nS=nFS; nS<nFS+nXS; nS++)
    {
      if ((SD_FLG(_this,nS)&0x01)==0x01)
      {
        nAbsFinalState = nS-nFS;
        IFCHECKEX(1) printf("\n Found final state: %ld (absolute: %ld)",(long)(nS-UD_FS(_this,nU)),(long)nS);
        break;
      }
    }

    /* Probability semiring: Calculate (fictive) transition reference counters */
    if (CFst_Wsr_GetType(_this,NULL)==FST_WSR_PROB && _this->m_bPsr) CFst_Rcs(_this,nU,0.);

    /* swap from -> to */
    for (nT=nFT; nT<nFT+nXT; nT++)
    {
      nIni = TD_INI(_this,nT);
      nTer = TD_TER(_this,nT);

      /* Update start state and final state */
      if      (nIni == 0             ) nIni = nAbsFinalState;
      else if (nIni == nAbsFinalState) nIni = 0;

      if      (nTer == 0             ) nTer = nAbsFinalState;
      else if (nTer == nAbsFinalState) nTer = 0;

      /* Swap initial and terminal states */
      TD_INI(_this,nT) = nTer;
      TD_TER(_this,nT) = nIni;
    }

    /* Probability semiring: Recalculate transition probabilities */
    if (CFst_Wsr_GetType(_this,NULL)==FST_WSR_PROB && _this->m_bPsr) CFst_Probs(_this,nU);

    /* Stop in single unit mode */
    if (nUnit>=0) break;
  }

  /* Probability semiring: Delete temporary reference counter components */
  if (CFst_Wsr_GetType(_this,NULL)==FST_WSR_PROB && _this->m_bPsr)
  {
    if (nIcRcs<0 && (nIcRcs=CData_FindComp(AS(CData,_this->sd),NC_SD_RC))>=0)
      CData_DeleteComps(AS(CData,_this->sd),nIcRcs,1);
    if (nIcRct<0 && (nIcRct=CData_FindComp(AS(CData,_this->td),NC_TD_RC))>=0)
      CData_DeleteComps(AS(CData,_this->td),nIcRct,1);
  }

  CFst_Check(_this); /* TODO: Remove after debugging */
  return O_K;
}

/*
 * Manual page at fst_man.def
 */
INT16 CGEN_PUBLIC CFst_Trim(CFst* _this, INT32 nUnit, FLOAT64 nWlim)
{
  INT32      nU   = 0;
  FST_ITYPE nT   = 0;
  FST_ITYPE nFT  = 0;
  FST_ITYPE nXS  = 0;
  FST_WTYPE nW   = 0.;
  INT32      nIcW = 0;

  /* Protocol */
  IFCHECK
  {
    printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());
    printf("\n CFst_Trim(%s,%ld,%lg)",BASEINST(_this)->m_lpInstanceName,(long)nUnit,(double)nWlim);
    printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols()); printf("\n");
  }

  /* Validation*/
  CHECK_THIS_RV(NOT_EXEC);
  CFst_Check(_this);

  /* Initialize */
  CFst_Wsr_GetType(_this,&nIcW);

  /* Loop over units */
  for (nU=nUnit<0?0:nUnit; nU<UD_XXU(_this); nU++)
  {
    nFT = UD_FT(_this,nU);
    nXS = UD_XS(_this,nU);

    /* Check for invalid node ID's and transition weights */
    for (nT=nFT; nT<nFT+UD_XT(_this,nU); )
    {
      nW = nIcW>=0 ? CData_Dfetch(AS(CData,_this->td),nT,nIcW) : 0.;
      if
      (
        TD_INI(_this,nT)<0 || TD_INI(_this,nT)>nXS ||
        TD_TER(_this,nT)<0 || TD_TER(_this,nT)>nXS
      )
      {
        IFCHECK printf("\n Delete invalid transition %ld (%ld->%ld) from unit %ld",
            (long)nT,(long)TD_INI(_this,nT),(long)TD_TER(_this,nT),(long)nU);
        CFst_Deltrans(_this,nU,nT);
      }
      else if (nIcW>=0 && nWlim!=0. && CFst_Wsr_Op(_this,nW,nWlim,OP_LESS))
      {
        IFCHECK printf("\n Delete heavy transition %ld (%ld->%ld) from unit %ld",
                       (long)nT,(long)TD_INI(_this,nT),(long)TD_TER(_this,nT),(long)nU);
        CFst_Deltrans(_this,nU,nT);
      }
      else nT++;
    }

    /* Remove unconnected states */
    CFst_TrimStates(_this,nU);

    /* Check unit */
    if (UD_XT(_this,nU)==0) IERROR(_this,FST_UNITEMPTY,nU,0,0);

    /* Stop in single unit mode */
    if (nUnit>=0) break;
  }

  /* Recalc transition probabilities if exist, this behavior is defined and must not be changed! */
  if (!_this->m_bLocal && CFst_Wsr_GetType(_this,NULL)==FST_WSR_PROB) CFst_Probs(_this,-1);

  /* Protocol */
  IFCHECK
  {
    printf("\n\n CFst_Trim done.\n");
    dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());
    printf("\n");
  }
  CFst_Check(_this); /* TODO: Remove after debugging */
  return O_K;
}

/*
 * Manual page at fst_man.def
 */
INT16 CGEN_PUBLIC CFst_Chain(CFst* _this, INT32 nUnit)
{
  INT32      nU    = 0;
  FST_ITYPE nS    = 0;
  FST_ITYPE nT    = 0;
  BOOL bInit      = FALSE;
  char lpsInit[L_INPUTLINE];

  /* Validate */
  CHECK_THIS_RV(NOT_EXEC);
  CFst_Check(_this);
  if (nUnit>UD_XXU(_this)) return IERROR(_this,FST_BADID,"unit",nU,0);

  /* Read transition initializer */
  bInit = (CData_ReadInitializer(AS(CData,_this->td),lpsInit,L_INPUTLINE,FALSE)>0);

  /* Loop over units */
  for (nU=nUnit<0?0:nUnit; nU<UD_XXU(_this); nU++)
  {
    for (nS=0; nS<UD_XS(_this,nU)-1; nS++)
    {
      nT = CFst_Addtrans(_this,nU,nS,nS+1);
      if (nT>=0 && bInit)
        IF_NOK(CData_InitializeRecordEx(AS(CData,_this->td),lpsInit,nT,IC_TD_DATA))
          return IERROR(_this,FST_INTERNAL,__FILE__,__LINE__,"");
    }

    /* Stop in single unit mode */
    if (nUnit>=0) break;
  }

  CFst_Check(_this); /* TODO: Remove after debugging */
  return O_K;
}

/*
 * Manual page at fst_man.def
 */
INT16 CGEN_PUBLIC CFst_Loops(CFst* _this, INT32 nUnit)
{
  INT32      nU = 0;
  FST_ITYPE nS = 0;
  FST_ITYPE nT = 0;
  BOOL bInit   = FALSE;
  char lpsInit[L_INPUTLINE];

  /* Validate */
  CHECK_THIS_RV(NOT_EXEC);
  CFst_Check(_this);
/*printf("\n*** READING INITIALIZER :"); */
/*  _this->td->m_lpMic = NULL; */
  bInit = (CData_ReadInitializer(AS(CData,_this->td),lpsInit,L_INPUTLINE,FALSE)>0);
/*printf(" (%d) |%s|\n",bInit,lpsInit); */

  if (nUnit>UD_XXU(_this)) return IERROR(_this,FST_BADID,"unit",nU,0);

  /* Loop over units */
  for (nU=nUnit<0?0:nUnit; nU<UD_XXU(_this); nU++)
  {
    for (nS=0; nS<UD_XS(_this,nU); nS++)
    {
      nT = CFst_Addtrans(_this,nU,nS,nS);
      if (nT>=0 && bInit)
        IF_NOK(CData_InitializeRecordEx(AS(CData,_this->td),lpsInit,nT,IC_TD_DATA))
          return IERROR(_this,FST_INTERNAL,__FILE__,__LINE__,"");
    }

    /* Stop in single unit mode */
    if (nUnit>=0) break;
  }

  CFst_Check(_this); /* TODO: Remove after debugging */
  return O_K;
}

/*
 * Manual page at fst_man.def
 */
INT16 CGEN_PUBLIC CFst_Project(CFst* _this)
{
  /* Validate */
  CHECK_THIS_RV(NOT_EXEC);
  CFst_Check(_this);

  _this->m_nIcTos = CData_FindComp(AS(CData,_this->td),NC_TD_TOS);
  if (_this->m_nIcTos>=0) CData_DeleteComps(AS(CData,_this->td),_this->m_nIcTos,1);

  /* Clear output symbol table */
  CData_Reset(_this->os,TRUE);

  CFst_Check(_this); /* TODO: Remove after debugging */
  return O_K;
}

INT16 CGEN_PUBLIC CFst_Invert(CFst* _this, INT32 nUnit)
{
  FST_TID_TYPE* lpTI = NULL;
  FST_ITYPE     nT   = 0;
  FST_STYPE     nTis = 0;
  INT32          nU   = 0;
  CData*        idSymTab = NULL;

  /* Validate */
  CHECK_THIS_RV(NOT_EXEC);
  CFst_Check(_this);

  if (CData_FindComp(AS(CData,_this->td),NC_TD_TIS)<0)
    return IERROR(_this,FST_MISS,"input symbol component","","transition table");
  if (CData_FindComp(AS(CData,_this->td),NC_TD_TOS)<0)
    return IERROR(_this,FST_MISS,"output symbol component","","transition table");

  for (nU=nUnit<0?0:nUnit; nU<UD_XXU(_this); nU++)
  {
    lpTI = CFst_STI_Init(_this,nU,0);
    for (nT=lpTI->nFT; nT<lpTI->nFT+lpTI->nXT; nT++)
    {
      nTis = *CFst_STI_TTis(lpTI,CFst_STI_GetTransPtr(lpTI,nT));
      *CFst_STI_TTis(lpTI,CFst_STI_GetTransPtr(lpTI,nT)) =
        *CFst_STI_TTos(lpTI,CFst_STI_GetTransPtr(lpTI,nT));
      *CFst_STI_TTos(lpTI,CFst_STI_GetTransPtr(lpTI,nT)) = nTis;
    }
    CFst_STI_Done(lpTI);
    if (nUnit>=0) break;
  }

  /* Swap input and output symbol table */
  ICREATEEX(CData,idSymTab,"CFst_Invert~idSymTab",NULL);
  CData_Copy(BASEINST(idSymTab),_this->is);
  CData_Copy(_this->is,_this->os);
  CData_Copy(_this->os,BASEINST(idSymTab));
  IDESTROY(idSymTab);

  CFst_Check(_this); /* TODO: Remove after debugging */
  return O_K;
}

/*
 * Manual page at fst_man.def
 */
INT16 CGEN_PUBLIC CFst_Unweight(CFst* _this)
{
  /* Validate */
  CHECK_THIS_RV(NOT_EXEC);
  CFst_Check(_this);

  _this->m_nWsr = CFst_Wsr_GetType(_this,&_this->m_nIcW);
  if (_this->m_nIcW>=0)
    CData_DeleteComps(AS(CData,_this->td),_this->m_nIcW,1);

  CFst_Check(_this); /* TODO: Remove after debugging */
  return O_K;
}

/* EOF */

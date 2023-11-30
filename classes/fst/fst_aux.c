/* dLabPro class CFst (fst)
 * - Auxilary methods
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
 * When compiled in DEBUG mode this method checks the integrity of a fst
 * instance. If errors are found, the method causes an assertion failure. The
 * method will, however, not make any corrections to the instance.
 *
 * @param _this The automaton to be checked.
 */
void CGEN_PUBLIC CFst_Check(CFst* _this)
{
#ifdef _DEBUG
#ifndef __MAX_TYPE_32BIT

  INT32 nU   = 0;
  INT32 nXU  = 0;
  INT32 nC   = 0;
  FST_ITYPE nXS  = 0;
  FST_ITYPE nXSa = 0;
  FST_ITYPE nT   = 0;
  FST_ITYPE nFT  = 0;
  FST_ITYPE nXT  = 0;
  FST_ITYPE nXTa = 0;
  FST_ITYPE nIni = 0;
  FST_ITYPE nTer = 0;

  CHECK_THIS();

  /* Check instance and internal data tables */
  DLPASSERT(CFst_IsKindOf(BASEINST(_this),"fst"));
  DLPASSERT(_this->ud);
  DLPASSERT(_this->sd);
  DLPASSERT(_this->td);
  DLPASSERT(dlp_is_symbolic_type_code(CData_GetCompType(AS(CData,_this->ud),IC_UD_NAME)));
  DLPASSERT(CData_GetCompType(AS(CData,_this->ud),IC_UD_XS  )==DLP_TYPE(FST_ITYPE));
  DLPASSERT(CData_GetCompType(AS(CData,_this->ud),IC_UD_XT  )==DLP_TYPE(FST_ITYPE));
  DLPASSERT(CData_GetCompType(AS(CData,_this->ud),IC_UD_FS  )==DLP_TYPE(FST_ITYPE));
  DLPASSERT(CData_GetCompType(AS(CData,_this->ud),IC_UD_FT  )==DLP_TYPE(FST_ITYPE));
  DLPASSERT(CData_GetCompType(AS(CData,_this->sd),IC_SD_FLAG)==T_BYTE             );
  DLPASSERT(CData_GetCompType(AS(CData,_this->td),IC_TD_TER )==DLP_TYPE(FST_ITYPE));
  DLPASSERT(CData_GetCompType(AS(CData,_this->td),IC_TD_INI )==DLP_TYPE(FST_ITYPE));

  if ((nC=CData_FindComp(AS(CData,_this->td),NC_TD_TIS))>=0)
    DLPASSERT(CData_GetCompType(AS(CData,_this->td),nC)==DLP_TYPE(FST_STYPE));
  if ((nC=CData_FindComp(AS(CData,_this->td),NC_TD_TOS))>=0)
    DLPASSERT(CData_GetCompType(AS(CData,_this->td),nC)==DLP_TYPE(FST_STYPE));
  if ((nC=CData_FindComp(AS(CData,_this->td),NC_TD_PSR))>=0)
    DLPASSERT(CData_GetCompType(AS(CData,_this->td),nC)==DLP_TYPE(FST_WTYPE));
  if ((nC=CData_FindComp(AS(CData,_this->td),NC_TD_LSR))>=0)
    DLPASSERT(CData_GetCompType(AS(CData,_this->td),nC)==DLP_TYPE(FST_WTYPE));
  if ((nC=CData_FindComp(AS(CData,_this->td),NC_TD_TSR))>=0)
    DLPASSERT(CData_GetCompType(AS(CData,_this->td),nC)==DLP_TYPE(FST_WTYPE));

  if (CData_GetNComps(AS(CData,_this->is))>0)
    DLPASSERT(dlp_is_symbolic_type_code(CData_GetCompType(AS(CData,_this->is),0)));
  if (CData_GetNComps(AS(CData,_this->os))>0)
    DLPASSERT(dlp_is_symbolic_type_code(CData_GetCompType(AS(CData,_this->os),0)));

  /* Check units */
  nXU = CData_GetNRecs(AS(CData,_this->ud));
  for (nU=0,nXSa=0,nXTa=0; nU<nXU; nU++)
  {
    /* Check consistency of unit description table */
    DLPASSERT(UD_FS(_this,nU)==nXSa); nXSa+=UD_XS(_this,nU);
    DLPASSERT(UD_FT(_this,nU)==nXTa); nXTa+=UD_XT(_this,nU);

    /* Check initial and terminal states of transitions */
    nXS = UD_XS(_this,nU);
    nFT = UD_FT(_this,nU);
    nXT = UD_XT(_this,nU);
    for (nT=nFT; nT<nFT+nXT; nT++)
    {
      nIni = TD_INI(_this,nT);
      nTer = TD_TER(_this,nT);
      DLPASSERT(nIni>=0  );
      DLPASSERT(nIni< nXS);
      DLPASSERT(nTer>=0  );
      DLPASSERT(nTer< nXS);
    }
  }
  DLPASSERT(UD_XXS(_this)==nXSa);
  DLPASSERT(UD_XXT(_this)==nXTa);

  /* Ignore extra components in input & output symbol tables
  DLPASSERT(IS_XXS(_this)<=1 || IS_XXS(_this)==UD_XXU(_this));
  DLPASSERT(OS_XXS(_this)<=1 || OS_XXS(_this)==UD_XXU(_this));*/

  /* Force checking heap */
  /* TODO: This slows the check significantly down; you may want to comment it out --> */
/*  DLPASSERT(_CrtCheckMemory());*/
  /* <-- */
  DLP_CHECK_MEMINTEGRITY;
  DLP_CHECK_MEMLEAKS;

#endif
#endif /* #ifdef _DEBUG */
}

/*
 * Manual page at fst_man.def
 */
INT32 CGEN_PUBLIC CFst_GetType(CFst* _this, INT32 nMask)
{
  INT32 nProps = 0;

  CHECK_THIS_RV(0);

  if
  (
    CData_FindComp(AS(CData,_this->td),NC_TD_TIS)>=0 &&
    CData_FindComp(AS(CData,_this->td),NC_TD_TOS)>=0
  )
  {
    nProps |= FST_TRANSDUCER;
  }

  if (CData_FindComp(AS(CData,_this->td),NC_TD_TIS)>=0) nProps |= FST_ACCEPTOR;
  if (CFst_Wsr_GetType(_this,NULL)                    ) nProps |= FST_WEIGHTED;

  return nProps;
}

/*
 * Manual page at fst_man.def
 */
INT32 CGEN_PUBLIC CFst_Analyze(CFst* _this, INT32 nUnit, INT32 nMask)
{
  INT32 nProps = 0;

  CHECK_THIS_RV(0);

  if (nUnit>=CData_GetNRecs(AS(CData,_this->ud)))
    return IERROR(_this,FST_BADID,"unit",nUnit,0);
  if (nMask==0) nMask=0xFFFFFFFF;

  if (nUnit<0)
  {
    nProps = 0x8FFFFFFF;
    for (nUnit=0; nUnit<CData_GetNRecs(AS(CData,_this->ud)); nUnit++) nProps &= CFst_Analyze(_this,nUnit,nMask);
  }
  else
  {
    INT32  nS       = 0;
    INT32  nIS      = 0;
    INT32  nTS      = 0;
    INT32  nFS      = 0;
    INT32  nXS      = 0;
    INT32  nT       = 0;
    INT32  nFT      = 0;
    INT32  nXT      = 0;
    INT32* lpIniCtr = NULL;
    INT32* lpTerCtr = NULL;

    IFCHECK printf("\n Unit %ld",(long)nUnit);

    nFT      = UD_FT(_this,nUnit);
    nXT      = UD_XT(_this,nUnit);
    nFS      = UD_FS(_this,nUnit);
    nXS      = UD_XS(_this,nUnit);
    lpIniCtr = (INT32*)dlp_calloc(nXS,sizeof(INT32));
    lpTerCtr = (INT32*)dlp_calloc(nXS,sizeof(INT32));
    nProps   = FST_FWDCONN | FST_BKWCONN | FST_FWDTREE | FST_BKWTREE;

    for (nT=0; nT<nXT; nT++)
    {
      nIS = TD_INI(_this,nT+nFT);
      nTS = TD_TER(_this,nT+nFT);

      if (nIS==nTS)
      {
        IFCHECK if (!(nProps & FST_LOOPS)) printf("   Loop %ld -> %ld",(long)nIS);
        nProps |= FST_LOOPS;
      }
      else
      {
        if (nIS>=0 && nIS<nXS) lpIniCtr[nIS]++;
        if (nTS>=0 && nTS<nXS) lpTerCtr[nTS]++;
      }
    }

    for (nS=0; nS<nXS; nS++)
    {
      if (nS>0 && lpTerCtr[nS]==0)
      {
        IFCHECK if (nProps & FST_FWDCONN) printf("\n   No transition * -> %ld",(long)nS);
        nProps &= ~FST_FWDCONN;
      }
      if ((SD_FLG(_this,nFS+nS)&0x01)==0 && lpIniCtr[nS]==0)
      {
        IFCHECK if (nProps & FST_BKWCONN) printf("\n   No transition %ld -> *",(long)nS);
        nProps &= ~FST_BKWCONN;
      }
      if (lpTerCtr[nS]>1)
      {
        IFCHECK if (nProps & FST_FWDTREE) printf("\n   Multiple transitions * -> %ld",(long)nS);
        nProps &= ~FST_FWDTREE;
      }
      if (nS>0 && lpIniCtr[nS]>1)
      {
        IFCHECK if (nProps & FST_BKWTREE) printf("\n   Multiple transitions %ld -> *",(long)nS);
        nProps &= ~FST_BKWTREE;
      }
    }

    IFCHECK
    {
      printf("\n   Graph has %sloops"            ,nProps & FST_LOOPS  ?"":"NO " );
      printf("\n   Graph is %sforward connected" ,nProps & FST_FWDCONN?"":"NOT ");
      printf("\n   Graph is %sbackward connected",nProps & FST_BKWCONN?"":"NOT ");
      printf("\n   Graph is %sa forward tree"    ,nProps & FST_FWDTREE?"":"NOT ");
      printf("\n   Graph is %sa backward tree"   ,nProps & FST_BKWTREE?"":"NOT ");
    }

    dlp_free(lpTerCtr);
    dlp_free(lpIniCtr);
  }

  return (nProps|CFst_GetType(_this,nMask)) & nMask;
}

/*
 * Manual page at fst_man.def
 */
INT16 CGEN_PUBLIC CFst_Status(CFst* _this)
{
  INT16 nCheckSave = -1;
  INT32  nU         = 0;
  INT32  nProps     = 0;

  CHECK_THIS_RV(NOT_EXEC);
  dlp_init_printstop();
  nProps = CFst_GetType(_this,0xFFFFFFFF);

  printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());
  printf("\n   Status of instance");
  printf("\n   data %s",BASEINST(_this)->m_lpInstanceName);
  printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());
  printf("\n   Units      : %-4ld (unit table      : '%s')"  ,(long)CData_GetNRecs(AS(CData,_this->ud)),_this->ud->m_lpInstanceName);
  printf("\n   States     : %-4ld (state table     : '%s')"  ,(long)CData_GetNRecs(AS(CData,_this->sd)),_this->sd->m_lpInstanceName);
  printf("\n   Transitions: %-4ld (transition table: '%s')"  ,(long)CData_GetNRecs(AS(CData,_this->td)),_this->td->m_lpInstanceName);
  printf("\n   In-Symbols : %-4ld (input symbol t. : '%s')"  ,CData_IsEmpty(AS(CData,_this->is))?0:(long)CData_GetNRecs(AS(CData,_this->is)),_this->is->m_lpInstanceName);
  printf("\n   Out-Symbols: %-4ld (output symbol t.: '%s')\n",CData_IsEmpty(AS(CData,_this->os))?0:(long)CData_GetNRecs(AS(CData,_this->os)),_this->os->m_lpInstanceName);
  printf("\n   Acceptor   : %s",nProps & FST_ACCEPTOR  ?"yes":"no");
  printf("\n   Transducer : %s",(nProps & FST_TRANSDUCER)==FST_TRANSDUCER?"yes":"no");
  printf("\n   Weights    : ");
  switch (CFst_Wsr_GetType(_this,NULL))
  {
    case FST_WSR_PROB: printf("probability semiring"      ); break;
    case FST_WSR_LOG : printf("log semiring"              ); break;
    case FST_WSR_TROP: printf("tropical semiring"         ); break;
    default          : printf("none"                      );
  }
  printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());
  printf("\n   Units");
  dlp_inc_printlines(13);

  nCheckSave = BASEINST(_this)->m_nCheck;
  BASEINST(_this)->m_nCheck = 1;
  for (nU=0; nU<UD_XXU(_this); nU++)
  {
    printf("\n\n - - - - - - - - - -");
    CFst_Analyze(_this,nU,0xFFFFFFFF);
    dlp_inc_printlines(12);
    if (dlp_if_printstop()) break;
  }
  BASEINST(_this)->m_nCheck = nCheckSave;

  printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());
  printf("\nUse <-print> to view the data content.\n");
  return O_K;
}

/*
 * Manual page at fst_man.def
 */
INT16 CGEN_PUBLIC CFst_Print(CFst* _this)
{
  INT32 nU     = 0;
  INT32 nXU    = 0;
  INT32 nProps = 0;

  CHECK_THIS_RV(NOT_EXEC);
  CFst_Check(_this);

  nXU    = CData_GetNRecs(AS(CData,_this->ud));
  nProps = CFst_GetType(_this,0xFFFFFFFF);

  dlp_init_printstop();
  printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());
  printf("\n   Data content of instance");
  printf("\n   fst %s",BASEINST(_this)->m_lpInstanceName);
  printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());
  printf("\n   Units      : %-4ld (unit table      : '%s')"  ,(long)CData_GetNRecs(AS(CData,_this->ud)),_this->ud->m_lpInstanceName);
  printf("\n   States     : %-4ld (state table     : '%s')"  ,(long)CData_GetNRecs(AS(CData,_this->sd)),_this->sd->m_lpInstanceName);
  printf("\n   Transitions: %-4ld (transition table: '%s')"  ,(long)CData_GetNRecs(AS(CData,_this->td)),_this->td->m_lpInstanceName);
  printf("\n   In-Symbols : %-4ld (input symbol t. : '%s')"  ,CData_IsEmpty(AS(CData,_this->is))?0:(long)CData_GetNRecs(AS(CData,_this->is)),_this->is->m_lpInstanceName);
  printf("\n   Out-Symbols: %-4ld (output symbol t.: '%s')\n",CData_IsEmpty(AS(CData,_this->os))?0:(long)CData_GetNRecs(AS(CData,_this->os)),_this->os->m_lpInstanceName);
  printf("\n   Acceptor   : %s",nProps & FST_ACCEPTOR  ?"yes":"no");
  printf("\n   Transducer : %s",(nProps & FST_TRANSDUCER)==FST_TRANSDUCER?"yes":"no");
  printf("\n   Weights    : ");
  switch (CFst_Wsr_GetType(_this,NULL))
  {
    case FST_WSR_PROB: printf("probability semiring"      ); break;
    case FST_WSR_LOG : printf("log semiring"              ); break;
    case FST_WSR_TROP: printf("tropical semiring"         ); break;
    default          : printf("none"                      );
  }
  printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());
  dlp_inc_printlines(12);

  /* Loop over units */
  for (nU=0; nU<nXU; )
  {
    INT32  nS       = 0;
    INT32  nT       = 0;
    INT32  nXS      = 0;
    INT32  nFS      = 0;
    INT32  nXT      = 0;
    INT32  nFT      = 0;
    INT32  nXCN     = 0;
    INT32  nXCT     = 0;
    INT32  nIcTis   = -1;
    INT32  nIcTos   = -1;
    int    nIniMask = -1;
    int    nTerMask = -1;
    char*  tx       = NULL;
    CData* idIs     = NULL;
    CData* idOs     = NULL;
    short  bPrnLab  = FALSE;

    nXS      = UD_XS(_this,nU);
    nFS      = UD_FS(_this,nU);
    nXT      = UD_XT(_this,nU);
    nFT      = UD_FT(_this,nU);
    nXCN     = CData_GetNComps(AS(CData,_this->sd));
    nXCT     = CData_GetNComps(AS(CData,_this->td));
    nIcTis   = CData_FindComp(AS(CData,_this->td),NC_TD_TIS);
    nIcTos   = CData_FindComp(AS(CData,_this->td),NC_TD_TOS);
    idIs     = AS(CData,_this->is);
    idOs     = AS(CData,_this->os);
    nProps   = CFst_Analyze(_this,nU,0xFFFFFFFF);
    bPrnLab |= ( idIs && !CData_IsEmpty(idIs) && (nIcTis>=0));
    bPrnLab |= ( idOs && !CData_IsEmpty(idOs) && (nIcTos>=0));

    /* Print nodes */
    printf("\n\n - - - - - - - - - -"); dlp_inc_printlines(1);
    printf("\n Unit %d",nU); dlp_inc_printlines(1);
    printf("\n   Name              : '%s'",(char*)CData_XAddr(AS(CData,_this->ud),nU,IC_UD_NAME)); dlp_inc_printlines(1);
    printf("\n   States            : %-4d (offset: %d)",  (int)nXS,(int)nFS); dlp_inc_printlines(1);
    printf("\n   Transitions       : %-4d (offset: %d)\n",(int)nXT,(int)nFT); dlp_inc_printlines(2);
    printf("\n   Loops             : %s",nProps & FST_LOOPS  ?"yes":"no"   ); dlp_inc_printlines(1);
    printf("\n   Forward connected : %s",nProps & FST_FWDCONN?"yes":"no"   ); dlp_inc_printlines(1);
    printf("\n   Backward connected: %s",nProps & FST_BKWCONN?"yes":"no"   ); dlp_inc_printlines(1);
    printf("\n   Forward tree      : %s",nProps & FST_FWDTREE?"yes":"no"   ); dlp_inc_printlines(1);
    printf("\n   Backward tree     : %s",nProps & FST_BKWTREE?"yes":"no"   ); dlp_inc_printlines(1);

    if (nXCN<=IC_SD_DATA)
    {
      printf("\n\n   States"); dlp_inc_printlines(2);
      printf("\n     [no custom descriptors]");
      dlp_inc_printlines(1);
    }
    else for (nS=nFS; nS<nFS+nXS; )
    {
      /* Print headings */
      if (nS==nFS)
      {
        printf("\n\n   States"); dlp_inc_printlines(2);
        printf("\n            ");
        dlp_inc_printlines(CData_PrintRec(AS(CData,_this->sd),-1,IC_SD_DATA,nXCN-IC_SD_DATA,12));
      }

      /* Print values */
      printf("\n  %6d :  ",nS);
      dlp_inc_printlines(CData_PrintRec(AS(CData,_this->sd),nS,IC_SD_DATA,nXCN-IC_SD_DATA,12)-1);
      if((nS=dlp_printstop_nix(nS,"state",NULL))==-1) break;
    }

    /* Print transitions */
    dlp_init_printstop();
    nIniMask = -1;
    nTerMask = -1;

    for (nT=nFT; nT<nFT+nXT; )
    {
      INT32 nIni = -1;
      INT32 nTer = -1;
      char lpMask[24];
      dlp_strcpy(lpMask,"");

      /* Print headings */
      if (nT==nFT)
      {
        printf("\n\n   Transitions"); dlp_inc_printlines(2);
        printf("\n         :  initial -> terminal   type  ");
        if (bPrnLab) printf("labels      ");
        dlp_inc_printlines(CData_PrintRec(AS(CData,_this->td),-1,IC_TD_DATA,nXCT-IC_TD_DATA,38));
      }

      /* Print values */
      nIni = (INT32)TD_INI(_this,nT);
      nTer = (INT32)TD_TER(_this,nT);

      if
      (
        (nIniMask < 0 || nIniMask == nIni) &&
        (nTerMask < 0 || nTerMask == nTer)
      )
      {
        printf
        (
          "\n  %7ld: %s%6ld%s -> %s%6ld%s  ",(long)nT,
          (SD_FLG(_this,nIni+nFS)&0x01?"(":" "),(long)nIni,(SD_FLG(_this,nIni+nFS)&0x01?")":" "),
          (SD_FLG(_this,nTer+nFS)&0x01?"(":" "),(long)nTer,(SD_FLG(_this,nTer+nFS)&0x01?")":" ")
        );

        if      (SD_FLG(_this,nTer+nFS)&0x01) printf(" end   ");
        else if (nIni == nTer               ) printf(" loop  ");
        else if (nIni == nTer-1             ) printf(" next  ");
        else if (nIni == nTer-2             ) printf(" 2next ");
        else if (nIni == 0                  ) printf(" start ");
        else if (nIni  > nTer               ) printf(" back  ");
        else                                  printf("       ");

        if (bPrnLab)
        {
        	char  sTis[255] = "";
        	char  sTos[255] = "";
        	INT32 nTxs      = -1;
					if (nIcTis>=0 && !CData_IsEmpty(idIs))
					{
						nTxs = (INT32)CData_Dfetch(AS(CData,_this->td),nT,nIcTis);
						dlp_strcpy(sTis,CData_Sfetch(idIs,nTxs,CData_GetNComps(idIs)==UD_XXU(_this)?nU:0));
						dlp_strabbrv(sTis,sTis,5);
					}
					if (nIcTos>=0 && !CData_IsEmpty(idOs))
					{
						nTxs = (INT32)CData_Dfetch(AS(CData,_this->td),nT,nIcTos);
						dlp_strcpy(sTos,CData_Sfetch(idOs,nTxs,CData_GetNComps(idIs)==UD_XXU(_this)?nU:0));
						dlp_strabbrv(sTos,sTos,5);
					}
        	printf("%-5s:%-5s ",sTis,sTos);
        }

        dlp_inc_printlines(CData_PrintRec(AS(CData,_this->td),nT,IC_TD_DATA,nXCT-IC_TD_DATA,38)-1);
        if((nT=dlp_printstop_nix(nT,"transition",lpMask))==-1) break;
      }
      else nT++;

      /* Force a print stop */
      if (nXT>25 && nT==nFT+nXT)
      {
        dlp_inc_printlines(dlp_maxprintlines());
        if((nT=dlp_printstop_nix(nT,"transition",lpMask))==-1) break;
      }

      if (nT< nFT                             ) nT=nFT;
      if (nT>=nFT+nXT && dlp_strlen(lpMask)==0) break;

      /* Set display mask */
      if ((tx = strstr(lpMask,"->")))
      {
        *tx = 0; tx += 2;
        dlp_strtrimleft(lpMask); dlp_strtrimright(lpMask);
        dlp_strtrimleft(tx); dlp_strtrimright(tx);
        if (sscanf(lpMask,"%d",&nIniMask)!=1) nIniMask = -1;
        if (sscanf(tx    ,"%d",&nTerMask)!=1) nTerMask = -1;
        printf("\n Restart."      ); dlp_inc_printlines(1);
        printf("\n Display mask: "); dlp_inc_printlines(1);
        if (nIniMask==-1) printf("* -> "); else printf("%ld -> ",(long)nIniMask);
        if (nTerMask==-1) printf("*");     else printf("%ld"    ,(long)nTerMask);
        nT=nFT;
      }
    }

    /* Unit complete */
    if (nU==nXU-1) break;

    /* Force a print stop */
    dlp_inc_printlines(dlp_maxprintlines());
    if((nU=dlp_printstop_nix(nU,"unit",NULL))==-1) break;
  }

  printf("\n No more data - Stop.");
  printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());
  printf("\n");

  return O_K;
}

/**
 * Reset state flag depending on mask.
 *
 * @param _this The automaton to be processed.
 * @param nUnit The unit to be processed, if nUnit<0 all units are processed.
 * @param bMask The mask that selects the flags to reset.
 */
void CGEN_PUBLIC CFst_ResetStateFlag(CFst* _this, INT32 nUnit, BYTE bMask)
{
  INT32 nU  = 0;
  INT32 nS  = 0;
  INT32 nFS = 0;
  INT32 nXS = 0;

  /* Loop over units */
  for (nU=nUnit<0?0:nUnit; nU<UD_XXU(_this); nU++)
  {
    nFS = UD_FS(_this,nU);
    nXS = UD_XS(_this,nU);

    /* Loop over states and reset flag */
    for (nS=nFS; nS<nFS+nXS; nS++)
      SD_FLG(_this,nS)&=~bMask;

    /* Stop in single unit mode */
    if (nUnit>=0) break;
  }

  return;
}

/*
 * Manual page at fst.def
 */
INT16 CGEN_PUBLIC CFst_Typerepair(CFst* _this)
{
  struct tdef {
    CDlpObject *d;
    const char *cname;
    INT16 type;
  } *tdef,tdefs[]={
    {_this->ud, "~XS",  DLP_TYPE(FST_ITYPE)},
    {_this->ud, "~XT",  DLP_TYPE(FST_ITYPE)},
    {_this->ud, "~FS",  DLP_TYPE(FST_ITYPE)},
    {_this->ud, "~FT",  DLP_TYPE(FST_ITYPE)},
    {_this->sd, "~FLG", T_BYTE},
    {_this->td, "~TER", DLP_TYPE(FST_ITYPE)},
    {_this->td, "~INI", DLP_TYPE(FST_ITYPE)},
    {_this->td, "~TIS", DLP_TYPE(FST_STYPE)},
    {_this->td, "~TOS", DLP_TYPE(FST_STYPE)},
    {_this->td, "~STK", DLP_TYPE(FST_STYPE)},
    {_this->td, "~LSR", DLP_TYPE(FST_WTYPE)},
    {_this->td, "~PSR", DLP_TYPE(FST_WTYPE)},
    {_this->td, "~TSR", DLP_TYPE(FST_WTYPE)},
    {NULL,      NULL,   0},
  };
  CData *idD;
  INT32 nC;
  for(tdef=tdefs;tdef->d;tdef++) if((nC=CData_FindComp(idD=AS(CData,tdef->d),tdef->cname))>=0 && CData_GetCompType(idD,nC)!=tdef->type){
    CData_InsertComp(idD,tdef->cname,tdef->type,nC+1);
    CData_Xstore(idD,idD,nC,1,nC+1);
    CData_DeleteComps(idD,nC,1);
  }
  return O_K;
}
/* EOF */

/* dLabPro class CFst (fst)
 * - (Topological) ordering
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
 * Creates a topological state ordering map. The generated map can be used with
 * {@link Order CFst_Order} to create a topologically ordered automaton.
 *
 * @param _this Automaton instance
 * @param nUnit The index of the unit to processed or -1 for all units
 * @param idMap The resulting order map
 * @return      O_K if successfull, a (negative) error code otherwise
 */
INT16 CGEN_PROTECTED CFst_MakeTopoOrderMap(CFst* _this, INT32 nUnit, CData* idMap)
{
  INT32          nU     = 0;         /* Current unit                          */
  INT32          nL     = 0;         /* Current search layer                  */
  INT32          nS     = 0;         /* Current (initial) state               */
  INT32          nTer   = 0;         /* Current terminal state                */
  INT32          nM     = 0;         /* Current map index                     */
  INT32          nAN    = 0;         /* Beamwidth of current layer            */
  BOOL          bAll   = FALSE;     /* Process all units?                    */
  CData*        idUmap = NULL;      /* Map for current unit                  */
  FST_TID_TYPE* iTI    = NULL;      /* Graph iterator                        */
  BYTE*         lpLBrd = NULL;      /* Layer enrollment: read layer buffer   */
  BYTE*         lpLBwr = NULL;      /* Layer enrollment: write layer buffer  */
  BYTE*         lpCB   = NULL;      /* Copy buffer for layer buffer swapping */
  BYTE*         lpT    = NULL;      /* Pointer to current transition         */

  /* Validation */
  CHECK_THIS_RV(NOT_EXEC);
  CFst_Check(_this);

  /* Initialization */
  bAll = (nUnit<0 || nUnit>=UD_XXU(_this));
  ICREATEEX(CData,idUmap,"CFst_MakeTopoOrderMap~idUmap",NULL);
  CData_Reset(BASEINST(idMap),TRUE);

  /* Protocol */
  IFCHECK
  {
    printf("\n --------------------------------------------------------------------------------");
    printf("\n CFst_MakeTopoOrderMap"                                                           );
    printf("\n   mode: %s",bAll?"all units":"single unit"                                       );
    printf("\n --------------------------------------------------------------------------------");
    printf("\n"                                                                                 );
  }

  /* Loop over units */
  for (nU=bAll?0:nUnit; nU<(bAll?CData_GetNRecs(AS(CData,_this->ud)):nUnit+1); nU++)
  {
    IFCHECK printf("\n Unit %ld:",(long)nU);

    /* Initialize iterator for current unit */
    iTI=CFst_STI_Init(_this,nU,TRUE);

    /* Initialize topo order map for current unit */
    CData_Reset(BASEINST(idUmap),TRUE);
    CData_AddComp(idUmap,"map",T_LONG);  /* Old index of new node (map index) */
    CData_AddComp(idUmap,"idx",T_LONG);  /* New index of old node         */
    CData_AddComp(idUmap,"ord",T_LONG);  /* Topological order of old node */
    CData_Allocate(idUmap,iTI->nXS);
    CData_Fill(idUmap,CMPLX(-1),CMPLX(0));

    /* Initialize layer buffers */
    lpLBrd = (BYTE*)dlp_calloc(iTI->nXS,sizeof(BYTE));
    lpLBwr = (BYTE*)dlp_calloc(iTI->nXS,sizeof(BYTE));

    /* Layer enrollment */
    CData_Dstore(idUmap,0,0,0);  /* Map index of start state is 0                */
    CData_Dstore(idUmap,0,0,1);  /* New index of start state is 0                */
    CData_Dstore(idUmap,0,0,2);  /* Topological order of start state is 0        */
    nM        = 1;               /* First map index after start state is 1       */
    lpLBrd[0] = 1;               /* Graph traversal seed: begin with start state */
    for (nL=1; nL<_this->m_nMaxLen; nL++)
    {
      IFCHECK printf("\n   Layer %ld:",(long)nL);
      /* Follow transitions out of currently active states */
      nAN = 0;
      for (nS=0; nS<iTI->nXS; nS++)
        if (lpLBrd[nS])
        {
          lpT=NULL;
          while ((lpT=CFst_STI_TfromS(iTI,nS,lpT))!=NULL)
          {
            /* Get terminal node of transition */
            nTer=*CFst_STI_TTer(iTI,lpT);
            DLPASSERT(nTer>=0 && nTer<iTI->nXS);
            IFCHECK printf("\n     %ld -> %ld: ",(long)nS,(long)nTer);

            if (CData_Dfetch(idUmap,nTer,1)<0)
            {
              IFCHECK printf("mid=%ld, tporder=%ld",(long)nM,(long)nL);

              /* Remember map index and topological order */
              CData_Dstore(idUmap,nTer,nM,0);
              CData_Dstore(idUmap,nM,nTer,1);
              CData_Dstore(idUmap,nL,nTer,2);
              nM++;

              /* Propagate search beam
                 NOTE: Do not propagate beam if node has been visited earlier */
              lpLBwr[nTer]=1;
              nAN++;
            }
            else IFCHECK printf("X");
          }
        }
      
      /* No more active nodes? --> end of enrollment */
      IFCHECK printf("\n   End of layer %ld: beamwidth=%ld",(long)nL,(long)nAN);
      if (!nAN) break;

      /* Swap layer buffers and clear new write layer buffer */
      lpCB=lpLBwr; lpLBwr=lpLBrd; lpLBrd=lpCB;
      dlp_memset(lpLBwr,0,iTI->nXS);
    }

    /* Postprocess order map and append to output */
    for (nS=0; nS<iTI->nXS; nS++)
    {
      if (CData_Dfetch(idUmap,nS,1)<0)
      {
        /* Forward disconnected node */
        CData_Dstore(idUmap,nS,nM,0);
        CData_Dstore(idUmap,nM,nS,1);
        nM++;
      }

      if (bAll)
      {
        /* All units mode: offset map index by index of first node of unit */
        CData_Dstore(idUmap,CData_Dfetch(idUmap,nS,0)+iTI->nFS,nS,0);
        CData_Dstore(idUmap,CData_Dfetch(idUmap,nS,1)+iTI->nFS,nS,1);
      }

    }
    if (bAll && nU>0)
      /* All units mode: delete map index for zero node */
      CData_DeleteRecs(idUmap,0,1);
    
    CData_Cat(idMap,idUmap);

    /* Clean up */
    dlp_free(lpLBrd);
    dlp_free(lpLBwr);
    CFst_STI_Done(iTI);
    IFCHECK printf("\n End of unit %ld",(long)nU);
  }

  /* Clean up */
  IDESTROY(idUmap);
  IFCHECK
  {
    printf("\n\n CFst_MakeTopoOrderMap done."                                               );
    printf("\n --------------------------------------------------------------------------------");
    printf("\n"                                                                                 );
  }
  return O_K;
}

/**
 * 
 */
void CGEN_PRIVATE CFst_Rank_Rcsn
(
  FST_TID_TYPE* lpTI,
  INT32          nRank,
  FST_ITYPE     nS,
  CData*        idDst
)
{
  BYTE* lpT = NULL;                                                             /* Pointer to current transition     */
  while ((lpT=CFst_STI_TfromS(lpTI,nS,lpT))!=NULL)                              /* Enumerate trans. starting at nS   */
  {                                                                             /* >>                                */
    FST_ITYPE nTerS = *CFst_STI_TTer(lpTI,lpT);                                 /*   Terminal state of curr. trans.  */
    if (CData_Dfetch(idDst,nTerS,0)<(FLOAT64)nRank) continue;                    /*   We have been here earlier       */
    CData_Dstore(idDst,(FLOAT64)nRank,nTerS,0);                                  /*   Store rank of terminal state    */
    CFst_Rank_Rcsn(lpTI,nRank+1,nTerS,idDst);                                   /*   Continue recursion at term. st. */
  }                                                                             /* <<                                */
}

/**
 * ~
 */
INT16 CGEN_PROTECTED CFst_Rank_Unit(CFst* _this, INT32 nUnit, CData* idDst)
{
  FST_TID_TYPE* lpTI = NULL;                                                    /* Ptr. to transition iterator       */
  DLPASSERT(_this);                                                             /* Need this pointer                 */
  DLPASSERT(idDst);                                                             /* Need destination instance         */
  DLPASSERT(nUnit>=0 && nUnit<UD_XXU(_this));                                   /* Bad unit index                    */
  CData_Array(idDst,T_LONG,1,UD_XS(_this,nUnit));                               /* Allocate destination instance     */
  CData_SetCname(idDst,0,"RANK");                                               /* Set component name                */
  CData_Fill(idDst,CMPLX((FLOAT64)T_LONG_MAX),CMPLX(0.));                       /* Fill with "infinity"              */
  CData_Dstore(idDst,0.,0,0);                                                   /* Start state has rank 0            */
  lpTI = CFst_STI_Init(_this,nUnit,FSTI_SORTINI);                               /* Initialize transition iterator    */
  CFst_Rank_Rcsn(lpTI,1,0,idDst);                                               /* Start recursion                   */
  CFst_STI_Done(lpTI);                                                          /* Destroy transition iterator       */
  return O_K;                                                                   /* That's ok                         */
}

/*
 * Manual page in fst_man.def
 */
INT16 CGEN_PUBLIC CFst_Rank(CFst* _this, INT32 nUnit, CData* idDst)
{
  CData* idAux = NULL;
  INT32   nU    = 0;
  
  if (!idDst) return NOT_EXEC;
  CData_Reset(BASEINST(idDst),TRUE);
  if (nUnit>=UD_XXU(_this)) return O_K;
  ICREATEEX(CData,idAux,"CFst_Rank.~idAux",NULL);
  
  for (nU=nUnit<0?0:nUnit; nU<UD_XXU(_this); nU++)
  {
    CFst_Rank_Unit(_this,nU,idAux);
    CData_Cat(idDst,idAux);
    if (nUnit<0) break;
  }

  IDESTROY(idAux);
  return O_K;
}

/*
 * Manual page in fst_man.def
 */
INT16 CGEN_PUBLIC CFst_Order(CFst* _this, CFst* itSrc, CData* idMap, INT32 nComp, INT32 nUnit)
{
  INT32          i      = 0;
  INT16         nErr   = O_K;
  INT32          nU     = 0;
  FST_TID_TYPE* lpTI   = NULL;
  BYTE*         lpT    = NULL;
  CData*        idAux  = NULL;
  CData*        idAux2 = NULL;

  /* Validation */
  CHECK_THIS_RV(NOT_EXEC);
  CFst_Check(_this);
  CFst_Check(itSrc);

  /* NO RETURNS BEYOND THIS POINT! */
  CREATEVIRTUAL(CFst,itSrc,_this);
  ICREATEEX(CData,idAux ,"CFst_Order~idAux" ,NULL);
  ICREATEEX(CData,idAux2,"CFst_Order~idAux2",NULL);

  /* Copy source to destination */
  if (nUnit>=0 && nUnit<UD_XXU(itSrc))
    CFst_CopyUi(_this,itSrc,NULL,nUnit);
  else
    CFst_Copy(BASEINST(_this),BASEINST(itSrc));

  /* Create topological order map */
  if (!idMap)
  {
    ICREATEEX(CData,idMap,"CFst_Order~idMap",NULL);
    CFst_MakeTopoOrderMap(itSrc,nUnit,idMap);
    nComp=0;
    IFCHECKEX(2) CData_Print(idMap);
  }

  /* Validate map */
  CData_Select(idAux,idMap,nComp,1);
  CData_Sortup(idAux,idAux,0);
  for (i=0; i<CData_GetNRecs(idAux); i++)
    if (i!=(INT32)CData_Dfetch(idAux,i,0))
      break;

  if (i<CData_GetNRecs(idAux) || 0!=(INT32)CData_Dfetch(idMap,0,nComp))
  {
    nErr=NOT_EXEC;
    IERROR(_this,NOT_EXEC,0,0,0);
    goto L_CLEANUP;
  }

  /* Order states */
  if (CData_GetNComps(AS(CData,itSrc->sd))>0)
    CData_Lookup
    (
      AS(CData,_this->sd),
      idMap,
      nComp,
      AS(CData,itSrc->sd),
      0,
      CData_GetNComps(AS(CData,itSrc->sd))
    );

  /* Adjust transition table */
  /* 1. Convert transitions to global node indices */
  for (nU=0; nU<UD_XXU(_this); nU++)
  {
    lpTI = CFst_STI_Init(_this,nU,FALSE);
    lpT  = NULL;
    while ((lpT=CFst_STI_TfromS(lpTI,-1,lpT))!=NULL)
    {
      *(CFst_STI_TIni(lpTI,lpT))+=lpTI->nFS;
      *(CFst_STI_TTer(lpTI,lpT))+=lpTI->nFS;
    }
    CFst_STI_Done(lpTI);
  }

  /* 2. Transform by idMap */
  CData_GenIndex(idAux ,AS(CData,_this->td),idMap,IC_TD_INI,nComp);
  CData_GenIndex(idAux2,AS(CData,_this->td),idMap,IC_TD_TER,nComp);
  CData_Join(idAux2,idAux);
  CData_Select(idAux,AS(CData,_this->td),IC_TD_DATA,CData_GetNComps(AS(CData,_this->td))-IC_TD_DATA);
  CData_Join(idAux2,idAux);
  CData_Copy(_this->td,BASEINST(idAux2));
  CData_SetCname(AS(CData,_this->td),IC_TD_INI,"~INI");
  CData_SetCname(AS(CData,_this->td),IC_TD_TER,"~TER");

  /* 3. Convert transitions back to local node indices */
  for (nU=0; nU<UD_XXU(_this); nU++)
  {
    lpTI = CFst_STI_Init(_this,nU,FALSE);
    lpT  = NULL;
    while ((lpT=CFst_STI_TfromS(lpTI,-1,lpT))!=NULL)
    {
      *(CFst_STI_TIni(lpTI,lpT))-=lpTI->nFS;
      *(CFst_STI_TTer(lpTI,lpT))-=lpTI->nFS;
    }
    CFst_STI_Done(lpTI);
  }

  /* Clean up */
L_CLEANUP:
  if (idMap && dlp_strcmp(BASEINST(idMap)->m_lpInstanceName,"CFst_Order~idMap")==0) IDESTROY(idMap);
  IDESTROY(idAux );
  IDESTROY(idAux2);
  DESTROYVIRTUAL(itSrc,_this);
  CFst_Check(_this); /* TODO: Remove after debugging */
  return nErr;
}

/* EOF */

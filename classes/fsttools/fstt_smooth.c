/* dLabPro class CFsttools (fsttools)
 * - String and weight semirings
 *
 * AUTHOR : Sebastian Kluth
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
#include "dlp_fsttools.h"
#include "dlp_fst.h"

FST_WTYPE discount_wb(CFst* itFst, INT32 nState, INT32 nTrId, INT32 nU)
{
  INT32          nTok   = 0;                                     /* # of Tokens                       */
  INT32          nTyp   = 0;                                     /* # of Types                        */
  INT32          nRc    = 0;                                     /* Reference Counter                 */        
  FST_WTYPE     nNewRc = 0;                                     /* discounted RC                     */      
  INT32          nIcRct = -1;                                    /* Reference counter component in td */
  INT32          nIcSRC = -1;                                    /* Smoothed RC component in td       */
  INT32          nIcPsr = -1;                                    /* PSR weight component in td        */
  INT32          nIcTyp = -1;                                    /* # of Types component in sd        */
  INT32          nIcTok = -1;                                    /* # of Tokens component in sd       */
  INT32          nIcUni = -1;                                    /* Unigram compoment in sd           */
  FST_WTYPE     nDProb = 0;                                     /* discounted probability            */
  FST_WTYPE     nZ     = 0;                                     /* Number of zerograms               */
  
  nIcRct = CData_FindComp(AS(CData,itFst->td),NC_TD_RC);        /* Find reference counters           */
  nIcPsr = CData_FindComp(AS(CData,itFst->td),NC_TD_PSR);       /* Find prob. semiring comp.         */
  nIcSRC = CData_FindComp(AS(CData,itFst->td),"~SRC");          /* Find discounted counts comp.      */  
  nIcTok = CData_FindComp(AS(CData,itFst->sd),"~TOK");          /* Find token comp.                  */
  nIcTyp = CData_FindComp(AS(CData,itFst->sd),"~TYP");          /* Find types comp.                  */
  nIcUni = CData_FindComp(AS(CData,itFst->ud),"~UNI");          /* Find unigram comp.                */
  
  /* calculate discounts and probabilities */                   /* --------------------------------- */ 
  nRc = (INT32)CData_Dfetch(AS(CData,itFst->td),                 /* Get RC from trns.                 */
    nTrId,nIcRct);                                              /*      |                            */
  nTyp = (INT32)CData_Dfetch(AS(CData,itFst->sd),                /* Get # of types                    */
    nState,nIcTyp);                                             /*      |                            */
  nTok = (INT32)CData_Dfetch(AS(CData,itFst->sd),                /* Get # of tokens                   */
    nState,nIcTok);                                             /*      |                            */
  if(nRc != 0)                                                  /* If not zerogram                   */
  {                                                             /* >                                 */
    nNewRc = (FST_WTYPE)nRc*(FST_WTYPE)nTok/                    /* Calc smoothed probs               */
      ((FST_WTYPE)nTyp+(FST_WTYPE)nTok);                        /*      |                            */
  }                                                             /* <                                 */
  else                                                          /* otherwise                         */
  {                                                             /* >                                 */
    nZ = (FST_WTYPE)CData_Dfetch(AS(CData,itFst->ud),           /* calc. number of zerograms         */
      nU,nIcUni) - (FST_WTYPE)nTyp;                             /*      |                            */
    nNewRc = (FST_WTYPE)nTyp*(FST_WTYPE)nTok/                   /* adjust zero counts                */
      ((nZ)*((FST_WTYPE)nTyp+(FST_WTYPE)nTok));                 /*      |                            */
  }                                                             /* <                                 */
  nDProb = (FST_WTYPE)nNewRc/(FST_WTYPE)nTok;                   /* Calc disc. prob.                  */
  CData_Dstore(AS(CData,itFst->td),                             /*         store new disc. counts    */
    nNewRc,                                                     /*                |                  */
    nTrId,                                                      /*                |                  */
    nIcSRC);                                                    /*                |                  */
  CData_Dstore(AS(CData,itFst->td),                             /*         store new probs           */
    nDProb,                                                      /*                |                  */
    nTrId,                                                      /*                |                  */
    nIcPsr);                                                    /*                |                  */
  return nDProb;                                                /* Return smoothed prob.             */
}

FST_WTYPE discount_gt(CFst* itFst, INT32 nState, INT32 nTrId, INT32 ptr[], INT16 nKGt, INT32 nU)
{
  INT32          nTok   = 0;                                     /* # of Tokens                       */
  INT32          nTyp   = 0;                                     /* # of Types                        */
  INT32          nRc    = 0;                                     /* Reference Counter                 */        
  FST_WTYPE     nNewRc = 0;                                     /* discounted RC                     */      
  INT32          nIcRct = -1;                                    /* Reference counter component in td */
  INT32          nIcSRC = -1;                                    /* Smoothed RC component in td       */
  INT32          nIcPsr = -1;                                    /* PSR weight component in td        */
  INT32          nIcTok = -1;                                    /* # of Tokens component in sd       */
  INT32          nIcTyp = -1;                                    /* # of Types component in sd        */
  INT32          nIcUni = -1;                                    /* comp. of unigrams in ud           */
  FST_WTYPE     nZ     = 0;                                     /* Number of zerograms               */
  FST_WTYPE     nDProb = 0;                                     /* discounted probability            */

  nIcRct = CData_FindComp(AS(CData,itFst->td),NC_TD_RC);        /* Find reference counters           */
  nIcPsr = CData_FindComp(AS(CData,itFst->td),NC_TD_PSR);       /* Find prob. semiring comp.         */
  nIcSRC = CData_FindComp(AS(CData,itFst->td),"~SRC");          /* Find discounted counts comp.      */  
  nIcTok = CData_FindComp(AS(CData,itFst->sd),"~TOK");          /* Find token comp.                  */
  nIcTyp = CData_FindComp(AS(CData,itFst->sd),"~TYP");          /* Find types comp.                  */
  nIcUni = CData_FindComp(AS(CData,itFst->ud),"~UNI");          /* Find unigram comp.                */

  /* calculate discounts and probabilities */                   /* --------------------------------- */ 
  nRc = (INT32)CData_Dfetch(AS(CData,itFst->td),nTrId,nIcRct);   /* get RC from trns.                 */
  nTok = (INT32)CData_Dfetch(AS(CData,itFst->sd),nState,nIcTok); /* get # of types from state         */
  if(nRc <= nKGt)                                               /* If count -leq k -> smooth it      */
  {                                                             /* >                                 */
    if(nRc == 0)                                                /* If zerogram                       */
    {                                                           /* >                                 */
      nTyp = (INT32)CData_Dfetch(AS(CData,itFst->sd),            /* Get # of types                    */
        nState,nIcTyp);                                         /*      |                            */
      nZ = (FST_WTYPE)CData_Dfetch(AS(CData,itFst->ud),         /* Calc. number of zerograms         */
        nU,nIcUni) - (FST_WTYPE)nTyp;                           /*      |                            */
      nNewRc = (FST_WTYPE)ptr[0]/(FST_WTYPE)nZ;                 /* Calc. smoothed zero counts        */
  //    printf("\nChange it c = 0 \n\t c=%f, N=%i",nNewRc,nTok);

    }                                                           /* <                                 */
    else                                                        /* otherwise                         */
    {                                                           /* >                                 */
      nNewRc = (FST_WTYPE)(((FST_WTYPE)nRc+1)*(FST_WTYPE)ptr[nRc]/(FST_WTYPE)ptr[nRc-1]-
        (FST_WTYPE)nRc*(nKGt+1)*(FST_WTYPE)ptr[nKGt]/(FST_WTYPE)ptr[0])/
        (1-(nKGt+1)*ptr[nKGt]/(FST_WTYPE)ptr[0]);
  //  printf("\nChange it c > 0 \n\t c=%f, N=%i",nNewRc,nTok);

    }                                                           /* <                                 */
  }                                                             /* <                                 */
  else                                                          /* If count -gt k ...                */
  {                                                             /* >                                 */
    nNewRc = nRc;                                               /* ...keep it!                       */
  //  printf("\nKeep it: \n\t c=%f, N=%i",nNewRc,nTok);

  }                                                             /* <                                 */
  nDProb = (FST_WTYPE)nNewRc/nTok;                              /* calc disc. prob.                  */
  CData_Dstore(AS(CData,itFst->td),                             /*         store new disc. counts    */
    nNewRc,                                                     /*                |                  */
    nTrId,                                                      /*                |                  */
    nIcSRC);                                                    /*                |                  */
  CData_Dstore(AS(CData,itFst->td),                             /*         store new probs           */
    nDProb,                                                      /*                |                  */
    nTrId,                                                      /*                |                  */
    nIcPsr);                                                    /*                |                  */

  return nDProb;                                                /* Return smoothed prob.             */
}

FST_WTYPE discount_add(CFst* itFst, INT32 nState, INT32 nTrId, FLOAT64 nDelta, INT32 nUni)
{
  INT32 nTok        = 0;                                         /* Tokens @ state                    */ 
  FST_WTYPE nDProb = 0;                                         /* Trns. prob.                       */
  FST_WTYPE nNewRc = 0;                                         /* Discounted RC                     */
  FST_WTYPE nRc    = 0;                                         /* Current RC                        */
  INT32 nIcRct      =-1;                                         /* Comp. idx. RC                     */
  INT32 nIcSRC      =-1;                                         /* Comp. idx. smoothed RC            */
  INT32 nIcTok      =-1;                                         /* Comp. idx. tokens                 */
  INT32 nIcPsr      =-1;                                         /* Comp. idx. probs                  */
  nIcRct = CData_FindComp(AS(CData,itFst->td),NC_TD_RC);        /* Find reference counters           */
  nIcPsr = CData_FindComp(AS(CData,itFst->td),NC_TD_PSR);       /* Find prob. semiring comp.         */
  nIcSRC = CData_FindComp(AS(CData,itFst->td),"~SRC");          /* Find discounted counts comp.      */  
  nIcTok = CData_FindComp(AS(CData,itFst->sd),"~TOK");          /* Find token comp.                  */

  /* calculate discounts and probabilities */                   /* --------------------------------- */ 
  nRc = CData_Dfetch(AS(CData,itFst->td),nTrId,nIcRct);         /* get RC from trns.                 */
  nTok = (INT32)CData_Dfetch(AS(CData,itFst->sd),nState,nIcTok);       /* get # of types from state         */
  nNewRc = (FST_WTYPE)((FST_WTYPE)nRc + (FST_WTYPE)nDelta)*     /* Smooth RCs                        */
    (FST_WTYPE)nTok/                                            /*                |                  */
    ((FST_WTYPE)nTok+(FST_WTYPE)nDelta*(FST_WTYPE)nUni);        /*                |                  */
  nDProb = (FST_WTYPE)nNewRc/(FST_WTYPE)nTok;                   /* Calc. prob.                       */
  CData_Dstore(AS(CData,itFst->td),                             /*         store new disc. counts    */
    nNewRc,                                                     /*                |                  */
    nTrId,                                                      /*                |                  */
    nIcSRC);                                                    /*                |                  */
  CData_Dstore(AS(CData,itFst->td),                             /*         store new probs           */
    nDProb,                                                      /*                |                  */
    nTrId,                                                      /*                |                  */
    nIcPsr);                                                    /*                |                  */

  return nDProb;
}

INT16 prepare_fst_4_gt(CFst* itFst, INT32 nU, INT16 nKGt, INT32 ptr_u[], INT32 ptr_b[], INT32 ptr_t[])  
{
  FST_TID_TYPE* lpTI_1 = NULL;                                  /* Automaton iterator data structure */
  FST_TID_TYPE* lpTI_2 = NULL;                                  /* Automaton iterator data structure */
  FST_TID_TYPE* lpTI_3 = NULL;                                  /* Automaton iterator data structure */
  BYTE*         lpT_1  = NULL;                                  /* Current trns. (iteration)         */
  BYTE*         lpT_2  = NULL;                                  /* Current trns. (iteration)         */
  BYTE*         lpT_3  = NULL;                                  /* Current trns. (iteration)         */
  INT32          nRc    = 0;                                     /* RC                                */
  INT16         i      = 0;                                     /* Loop Cntr.                        */

  /* Setup array, which holds frequency of frequencies */
  for(i = 0; i <= nKGt; i++)                                    /* Set counts to zero                */ 
  {                                                             /* >>                                */
    ptr_u[i] = 0;
    ptr_b[i] = 0;                                                /* ...                               */
    ptr_t[i] = 0;                                               /* ...                               */
  }                                                             /* <<                                */
  /* Count frequency of frequencies */                          /* ---------------------------------- */
  lpTI_1 = CFst_STI_Init(itFst,nU,FSTI_SORTINI);                /*   Get sorted transition iterator   */
  lpTI_2 = CFst_STI_Init(itFst, nU, FSTI_SORTINI);              /*   Get sorted trns. ite.            */
  lpTI_3 = CFst_STI_Init(itFst, nU, FSTI_SORTINI);              /*   Get sorted trns. ite.            */
  while((lpT_1=CFst_STI_TfromS(lpTI_1,0,lpT_1))!=NULL)          /* loop over trns. at init. state     */
  {                                                             /* >>                                 */
      nRc = *CFst_STI_TRc(lpTI_1,lpT_1);                        /* Get RC (bigrams)                   */
      if((nRc <= (nKGt + 1)) && (nRc > 0))                      /* If RC -leq (threshold + 1)         */
      {                                                         /* >                                  */
        ptr_u[nRc-1]++;                                         /* inc count                          */
      }                                                         /* <                                  */
    while((lpT_2=CFst_STI_TfromS(lpTI_2,                        /* loop over bigram-trns.             */
      (INT32)*CFst_STI_TTer(lpTI_1,lpT_1),lpT_2))!=NULL)         /*              |                     */
    {                                                           /* >>                                 */
      nRc = *CFst_STI_TRc(lpTI_2,lpT_2);                        /* Get RC (bigrams)                   */
      if(nRc <= (nKGt + 1))                                     /* If RC -leq (threshold + 1)         */
      {                                                         /* >                                  */
        ptr_b[nRc-1]++;                                         /* inc count                          */
      }                                                         /* <                                  */
      while((lpT_3=CFst_STI_TfromS(lpTI_3,                      /* loop over trigram-trns.            */
        (INT32)*CFst_STI_TTer(lpTI_2,lpT_2),lpT_3))!=NULL)       /*              |                     */
      {                                                         /* >>                                 */
        nRc = *CFst_STI_TRc(lpTI_3,lpT_3);                      /* Get RC (trigrams)                  */
        if(nRc <= (nKGt + 1))                                   /* If RC -leq (threshold + 1)         */
        {                                                       /* >                                  */
          ptr_t[nRc-1]++;                                       /* inc count                          */
        }                                                       /* <                                  */
      }                                                         /* <<                                 */             
    }                                                           /* <<                                 */
  }                                                             /* <<                                 */

  /* Clean up */                                                /* --------------------------------- */
  CFst_STI_Done(lpTI_1);                                        /* Clean up iterator                 */  
  CFst_STI_Done(lpTI_2);                                        /* Clean up iterator                 */
  CFst_STI_Done(lpTI_3);                                        /* Clean up iterator                 */
  return O_K;
}

INT16 CGEN_PUBLIC CFsttools_Smooth(CFsttools *_this, INT16 nKGt, FLOAT64 nDelta, INT32 nUnit, CFst* itFst)
{
  INT32          nU     = 0;                                     /* Current unit                      */
  FST_ITYPE     nS     = 0;                                     /* Current state                     */
  INT32          nIcRct = -1;                                    /* Reference counter component in td */
  INT32          nIcSRC = -1;                                    /* smoothed RC comp.                 */
  INT32          nIcPsr = -1;                                    /* prob semiring comp.               */
  INT32          nIcUni = -1;                                    /* # of unigrams in unit comp.       */
  INT32          nIcBig = -1;                                    /* # of bigrams in unit comp.        */
  INT32          nIcTri = -1;                                    /* # of trigrams in unit comp.       */
  FST_WTYPE     nTyp_1 = 0;                                     /* Aux. ctr.                         */
  FST_WTYPE     nTok_1 = 0;                                     /* Aux. ctr.                         */
  FST_WTYPE     nTyp_2 = 0;                                     /* Aux. ctr.                         */
  FST_WTYPE     nTok_2 = 0;                                     /* Aux. ctr.                         */
  FST_WTYPE     nTyp_3 = 0;                                     /* Aux. ctr.                         */
  FST_WTYPE     nTok_3 = 0;                                     /* Aux. ctr.                         */
  CData*        idTd   = NULL;                                  /* Pointer to transition table       */
  FST_TID_TYPE* lpTI_1 = NULL;                                  /* Automaton iterator data structure */
  FST_TID_TYPE* lpTI_2 = NULL;                                  /* Automaton iterator data structure */
  FST_TID_TYPE* lpTI_3 = NULL;                                  /* Automaton iterator data structure */
  FST_TID_TYPE* lpTI_4 = NULL;                                  /* Automaton iterator data structure */
  FST_TID_TYPE* lpTI_5 = NULL;                                  /* Automaton iterator data structure */
  FST_TID_TYPE* lpTI_6 = NULL;                                  /* Automaton iterator data structure */
  BYTE*         lpT_1  = NULL;                                  /* Current transition (iteration)    */
  BYTE*         lpT_2  = NULL;                                  /* Current transition (iteration)    */
  BYTE*         lpT_3  = NULL;                                  /* Current transition (iteration)    */
  BYTE*         lpT_5  = NULL;                                  /* Current transition (iteration)    */
  BYTE*         lpT_6  = NULL;                                  /* Current transition (iteration)    */
  INT32          nIcTok = 0;                                     /* Token comp. of state              */
  INT32          nIcTyp = 0;                                     /* Type comp. of state               */
  INT32          nUniCnt= 0;                                     /* Unigram counter                   */
  INT32          nIcBet = 0;                                     /* Beta comp. of state               */
  INT32          nIcTis = 0;                                     /* TIS comp. of trns.                */     
  INT32          nBigCnt= 0;                                     /* Bigram counter                    */
  INT32          nTriCnt= 0;                                     /* Tri. ctr.                         */
  FST_WTYPE     nPAgg1 = 0.;                                    /* Aggregated probability            */
  FST_STYPE     nTis_1 = -1;                                    /* TIS for search                    */
  FST_STYPE     nTis_2 = -1;                                    /* TIS for search                    */
  FST_WTYPE     nPAgg2 = 0.;                                    /* Aggregated probability            */
  FST_WTYPE     nAlpha = 0.;                                    /* Alpha for backoff                 */
  INT32          nNewCnt= 0;                                     /* # of trns. to add                 */
  INT32          nNewTer= -1;                                    /* New ter. state                    */
  CData*        idAux = NULL;                                   /* Aux. data instance ptr.           */
  INT32          nIcAlpha=-1;                                    /* Alpha comp. in aux. data instance */
  INT32          nIcInit =-1;                                    /* Init. state in aux. data instance */
  INT32          nAuxId = -1;                                    /* New record id in idAux            */
  INT32          nNewTId= -1;                                    /* Id of added alpha-trns            */
  INT32          *ptr_u = NULL;                                  /* Ptr to freq. of freq. array uni.  */
  INT32          *ptr_b = NULL;                                  /* Ptr to freq. of freq. array big.  */
  INT32          *ptr_t = NULL;;                                 /* Ptr to freq. of freq. array tri.  */
  bool          bGt    = _this->m_bGt;                          /* Use Good-Turing                   */
  bool          bWb    = _this->m_bWb;                          /* Use Witten-Bell                   */
  bool          bAdd   = _this->m_bAdd;                         /* Use Additive smoothing            */

  if(((bGt && bWb) || (bGt && bAdd) || (bAdd && bWb))           /* More than 1 algorithm ...         */
    || (!bAdd && !bGt && !bWb))                                 /* ... or no algorithm ...           */
  {                                                             /* >                                 */
    printf("\n As written in the manual, you have to choose");  /* ERROR-MSG                         */
    printf(" ONE smoothing algorithm.");                        /* ERROR-MSG                         */
    printf("\n Nevertheless this process will continue with");  /* ERROR-MSG                         */
    printf(" default algrithm (Good-Turing).\n");               /* ERROR-MSG                         */
    bGt = 1;                                                    /* Set default                       */
    bWb = 0;                                                    /*             |                     */
    bAdd= 0;                                                    /*             |                     */
    nKGt= 5;
  }                                                             /* <                                 */
  if(bAdd && (nDelta <= 0 || nDelta > 1))                       /* If no delta or out of range       */
  {                                                             /* >                                 */
    printf("\n As written in the manual, you have to choose");  /* ERROR-MSG                         */
    printf(" some delta, if you want to use Add-Delta");        /* ERROR-MSG                         */
    printf(" smoothing.\n Nevertheless this process will");     /* ERROR-MSG                         */
    printf(" continue with default value delta=1.\n");          /* ERROR-MSG                         */
    nDelta = 1;                                                 /* Set default                       */    
  }                                                             /* <                                 */
  if(bGt && (nKGt <= 0))                                        /* If no k or out of range           */
  {                                                             /* >                                 */
    printf("\n As written in the manual, you have to choose");  /* ERROR-MSG                         */
    printf(" some k, if you want to use Good-Turing-Smoothing");/* ERROR-MSG                         */
    printf(" smoothing.\n Nevertheless this process will");     /* ERROR-MSG                         */
    printf(" continue with default value k=5.\n");              /* ERROR-MSG                         */
    nKGt = 5;                                                   /* default k=5                       */
  }                                                             /* <                                 */
  
  /* Validation */                                              /* --------------------------------- */
  CHECK_THIS_RV(NOT_EXEC);                                      /* Check this pointer                */
  CFst_Check(itFst);                                            /* Check FST(s)                      */
  
  /* Initialize */                                              /* --------------------------------- */
  idTd   = AS(CData,itFst->td);                                 /* Get pointer to transition table   */
  nIcRct = CData_FindComp(AS(CData,itFst->td),NC_TD_RC);        /* Find reference counters           */

  /* Add components to tables to store auxilary stuff... */     /* --------------------------------- */
  CData_AddComp(AS(CData,itFst->sd),"~TOK",DLP_TYPE(INT32));      /* # of tokens @ state               */
  CData_AddComp(AS(CData,itFst->sd),"~TYP",DLP_TYPE(INT32));      /* # of types @ state               */
  CData_AddComp(AS(CData,itFst->td),"~SRC",DLP_TYPE(FLOAT32));    /* Comp. for discounted counts       */ 
  CData_AddComp(AS(CData,itFst->ud),"~UNI",DLP_TYPE(INT32));     /* # of unigrams in LM               */
  CData_AddComp(AS(CData,itFst->ud),"~BIG",DLP_TYPE(INT32));     /* # of bigrams in LM                */
  CData_AddComp(AS(CData,itFst->ud),"~TRI",DLP_TYPE(INT32));     /* # of trigrams in LM               */
  CData_AddComp(AS(CData,itFst->sd),"~BETA",DLP_TYPE(FLOAT32));   /* Comp. for beta @ state            */
  if(CFst_Wsr_GetType(itFst, NULL) == FST_WSR_NONE){             /* If no semiring                    */  
    CData_AddComp(AS(CData,itFst->td),NC_TD_PSR,DLP_TYPE(FST_WTYPE));}                          /* Switch it to PSR                  */
  /* ...find those components */                                /* --------------------------------- */
  nIcTok = CData_FindComp(AS(CData,itFst->sd),"~TOK");          /* ...                               */
  nIcTyp = CData_FindComp(AS(CData,itFst->sd),"~TYP");          /* ...                               */
  nIcSRC = CData_FindComp(AS(CData,itFst->td),"~SRC");          /* ...                               */
  nIcPsr = CData_FindComp(AS(CData,itFst->td),NC_TD_PSR);       /* ...                               */
  nIcUni = CData_FindComp(AS(CData,itFst->ud),"~UNI");          /* ...                               */
  nIcBig = CData_FindComp(AS(CData,itFst->ud),"~BIG");          /* ...                               */
  nIcTri = CData_FindComp(AS(CData,itFst->ud),"~TRI");          /* ...                               */  
  nIcBet = CData_FindComp(AS(CData,itFst->sd),"~BETA");         /* ...                               */
  nIcTis = CData_FindComp(AS(CData,itFst->td),NC_TD_TIS);       /* ...                               */
  
  /* Loop over units */                                         /* --------------------------------- */
  for (nU=nUnit<0?0:nUnit; nU<UD_XXU(itFst); nU++)              /* For all units ...                 */
  {                                                             /* >>                                */
  
    /* Initialize */                                              /* --------------------------------- */  
    ICREATE(CData,idAux,NULL);                                    /* Create aux. table                 */
    CData_AddComp(idAux,"~alpha",DLP_TYPE(FST_WTYPE));            /* Add comp. 4 alpha                 */
    CData_AddComp(idAux,"~init",DLP_TYPE(FST_ITYPE));             /* Add comp. 4 state of new trns.    */
    nIcAlpha = CData_FindComp(idAux,"~alpha");                    /* Find comp.                        */
    nIcInit = CData_FindComp(idAux,"~init");                      /* Find comp.                        */
    if(bGt)                                                       /* If GT-smoothing                   */
    {                                                             /* Begin setup GT                    */
      ptr_u = (INT32*)malloc((nKGt+1)*sizeof(INT32));               /* Create array                      */
      ptr_b = (INT32*)malloc((nKGt+1)*sizeof(INT32));               /* Create array                      */
      ptr_t = (INT32*)malloc((nKGt+1)*sizeof(INT32));               /* Create array                      */
      prepare_fst_4_gt(itFst, nU, nKGt, ptr_u, ptr_b, ptr_t);     /* Prepare FST for GT-smoothing      */
    }                                                             /* End setup GT                      */
    lpTI_1 = CFst_STI_Init(itFst, nU, FSTI_SORTINI);              /*   Get sorted transition iterator  */
    lpTI_2 = CFst_STI_Init(itFst, nU, FSTI_SORTINI);              /*   Get sorted trns. ite.           */
    lpTI_3 = CFst_STI_Init(itFst, nU, FSTI_SORTINI);              /*   Get sortet trns. ite.           */
    nUniCnt = 0;
    nBigCnt = 0;
    nTriCnt = 0;
    while((lpT_1 = CFst_STI_TfromS(lpTI_1,0,lpT_1))!=NULL)        /* loop over all unigrams            */
    {                                                             /*                                   */
      nUniCnt++;                                                  /* increase number of unigrams       */
      if(*CFst_STI_TRc(lpTI_1,lpT_1) > 0)                         /* only seen n-gram is a type        */
      {                                                           /*           |                       */
        nTyp_1++;                                                 /*           |                       */
      }                                                           /*           |                       */
      nTok_1 = nTok_1 + *CFst_STI_TRc(lpTI_1,lpT_1);              /* Get RC for cur. trns.             */
      nTok_2 = 0;                                                 /* flush ctr                         */
      nTyp_2 = 0;                                                 /* flush ctr                         */
      while((lpT_2 = CFst_STI_TfromS(lpTI_2,(INT32)*CFst_STI_TTer(lpTI_1,lpT_1),lpT_2))!=NULL)
      {                                                           /* >>                                */ 
        nBigCnt++;                                                  /* increase number of bigrams        */
        nTyp_2++;                                                   /* assume every bigram was seen      */
        nTok_2 = nTok_2 + *CFst_STI_TRc(lpTI_2,lpT_2);              /* sum counts up                     */
        nTok_3 = 0;                                                 /* flush ctr.                        */
        nTyp_3 = 0;                                                 /* flush ctr.                        */
        while((lpT_3 = CFst_STI_TfromS(lpTI_3,(INT32)*CFst_STI_TTer(lpTI_2,lpT_2),lpT_3))!=NULL)
        {                                                           /* >>                                */
          nTriCnt++;                                                  /* increase number of trigrams       */
          nTyp_3++;                                                   /* assume every trigram was seen     */
          nTok_3 = nTok_3 + *CFst_STI_TRc(lpTI_3,lpT_3);              /* sum counts up                     */
        }                                                           /* <<                                */  
        CData_Dstore(AS(CData,itFst->sd),nTok_3,(INT32)*CFst_STI_TTer(lpTI_2,lpT_2),nIcTok);     /* store */
        CData_Dstore(AS(CData,itFst->sd),nTyp_3,(INT32)*CFst_STI_TTer(lpTI_2,lpT_2),nIcTyp);     /* store */
      }                                                           /* <<                                */  
      CData_Dstore(AS(CData,itFst->sd),nTok_2,(INT32)*CFst_STI_TTer(lpTI_1,lpT_1),nIcTok);     /* store */     
      CData_Dstore(AS(CData,itFst->sd),nTyp_2,(INT32)*CFst_STI_TTer(lpTI_1,lpT_1),nIcTyp);     /* store */
    }                                                           /* <<                                */
    CData_Dstore(AS(CData,itFst->sd),nTok_1,0,nIcTok);          /* store                             */
    CData_Dstore(AS(CData,itFst->sd),nTyp_1,0,nIcTyp);          /* store                             */

    CData_Dstore(AS(CData,itFst->ud),nUniCnt,nU,nIcUni);        /* store                             */    
    CData_Dstore(AS(CData,itFst->ud),nBigCnt,nU,nIcBig);        /* store                             */
    CData_Dstore(AS(CData,itFst->ud),nTriCnt,nU,nIcTri);        /* store                             */
    CFst_STI_Done(lpTI_1);                                        /* Clean up iterator                 */
    CFst_STI_Done(lpTI_2);                                        /* Clean up iterator                 */
    CFst_STI_Done(lpTI_3);                                        /* Clean up iterator                 */
    
    /* Calculate discounted RCs and smoothed probs for unig. */   /* --------------------------------- */                    
    lpTI_1 = CFst_STI_Init(itFst,nU,FSTI_SORTINI);                /*   Get sorted transition iterator  */
    lpTI_2 = CFst_STI_Init(itFst,nU,FSTI_SORTINI);                /*   Get sorted trns. iterator       */
    lpTI_3 = CFst_STI_Init(itFst,nU,FSTI_SORTINI);                /*   Get sorted trns. iterator       */
    while ((lpT_1=CFst_STI_TfromS(lpTI_1,0,lpT_1))!=NULL)         /* Enumerate trns. at 0              */  
    {                                                             /* >>                                */
      if(bAdd)                                                      /* If additive smoothing             */
      {                                                             /* >                                 */
        discount_add(itFst,                                         /*           |                       */
          0,                                                        /*           |                       */
          CFst_STI_GetTransId(lpTI_1,lpT_1),                        /*           |                       */
          nDelta,                                                   /*           |                       */
          nUniCnt);                                                 /*           |                       */
      }                                                             /* <                                 */
      if(bWb)                                                       /* If Witten-Bell-Smoothing          */
      {                                                             /* >                                 */
        discount_wb(itFst,                                          /*           |                       */
          0,                                                        /*           |                       */
          CFst_STI_GetTransId(lpTI_1,lpT_1),nU);                    /*           |                       */
      }                                                             /* <                                 */
      if(bGt)                                                       /* If Good-Turing-Smoothing          */
      {                                                             /* >                                 */
        discount_gt(itFst,                                          /*           |                       */
          0,                                                        /*           |                       */
          CFst_STI_GetTransId(lpTI_1,lpT_1),                        /*           |                       */
          ptr_u,                                                    /*           |                       */
          nKGt,nU);                                                 /*           |                       */
      }                                                             /* <                                 */
      
      /* Calculate c* and p* for bi- and trigrams */                /* --------------------------------- */
      nPAgg1=0;                                                     /* Flush aggregated prob.-mass       */ 
      while ((lpT_2=CFst_STI_TfromS(lpTI_2,                         /* Enumerate trns. at TER            */
        (INT32)*CFst_STI_TTer(lpTI_1,lpT_1),lpT_2))!=NULL)           /*               |                   */
      {                                                             /* >>                                */
        if(bWb)                                                       /* If WB-discounting                 */
          nPAgg1 = nPAgg1 +                                           /* aggregate                         */
            discount_wb(itFst,(INT32)*CFst_STI_TTer(lpTI_1,lpT_1),     /*               |                   */ 
            CFst_STI_GetTransId(lpTI_2,lpT_2),nU);                    /*               |                   */
        if(bGt)                                                       /* If GT-discounting                 */
          nPAgg1 = nPAgg1 +                                           /* aggregate                         */
          discount_gt(itFst,(INT32)*CFst_STI_TTer(lpTI_1,lpT_1),       /*               |                   */ 
          CFst_STI_GetTransId(lpTI_2,lpT_2),ptr_b,nKGt,nU);           /*               |                   */
        if(bAdd)                                                      /* If additive smoothing             */
              nPAgg1 = nPAgg1 +                                       /* aggregate                         */
              discount_add(itFst,                                     /*               |                   */ 
              (INT32)*CFst_STI_TTer(lpTI_1,lpT_1),                     /*               |                   */
              CFst_STI_GetTransId(lpTI_2,lpT_2),nDelta,nUniCnt);      /*               |                   */
        nPAgg2=0;                                                     /* Flush aggregated prob.-mass       */
        while ((lpT_3=CFst_STI_TfromS(lpTI_3,                         /* Enumerate trns. at TER            */
          (INT32)*CFst_STI_TTer(lpTI_2,lpT_2),lpT_3))!=NULL)           /*               |                   */
        {                                                             /* >>                                */
          if(bWb)                                                       /* IF WB-discounting                 */
            nPAgg2 = nPAgg2 +                                           /* aggregate                         */
              discount_wb(itFst,                                        /*               |                   */ 
              (INT32)*CFst_STI_TTer(lpTI_2,lpT_2),                       /*               |                   */
              CFst_STI_GetTransId(lpTI_3,lpT_3),nU);                    /*               |                   */
          if(bGt)                                                       /* If GT-discounting                 */
            nPAgg2 = nPAgg2 +                                           /* aggregate                         */
              discount_gt(itFst,                                        /*               |                   */ 
              (INT32)*CFst_STI_TTer(lpTI_2,lpT_2),                       /*               |                   */
              CFst_STI_GetTransId(lpTI_3,lpT_3),ptr_t,nKGt,nU);         /*               |                   */
          if(bAdd)                                                      /* If additive smoothing             */
              nPAgg2 = nPAgg2 +                                          /* aggregate                         */
              discount_add(itFst,                                        /*               |                   */ 
              (INT32)*CFst_STI_TTer(lpTI_2,lpT_2),                       /*               |                   */
              CFst_STI_GetTransId(lpTI_3,lpT_3),nDelta,nUniCnt);        /*               |                   */
        }                                                             /* <<                                */
        CData_Dstore(AS(CData,itFst->sd),                             /* store beta @ state (trigrams)     */
          (1-nPAgg2),                                                 /*               |                   */
          (INT32)*CFst_STI_TTer(lpTI_2,lpT_2),                         /*               |                   */
          nIcBet);                                                    /*               |                   */
      }                                                             /* <<                                */
      CData_Dstore(AS(CData,itFst->sd),                             /* store beta @ node (bigrams)       */
        (1-nPAgg1),                                                 /*               |                   */
        (INT32)*CFst_STI_TTer(lpTI_1,lpT_1),                         /*               |                   */
        nIcBet);                                                    /*               |                   */
    }                                                             /* <<                                */
    
    CFst_STI_Done(lpTI_1);                                        /* Clean up iterator                 */  
    CFst_STI_Done(lpTI_2);                                        /* Clean up iterator                 */
    CFst_STI_Done(lpTI_3);                                        /* Clean up iterator                 */
    
    /* calculate alphas */                                        /* --------------------------------- */
    lpTI_1 = CFst_STI_Init(itFst,nU,FSTI_SORTINI);                /*   Get sorted transition iterator  */
    lpTI_2 = CFst_STI_Init(itFst,nU,FSTI_SORTINI);                /*   Get sorted trns. iterator       */
    lpTI_3 = CFst_STI_Init(itFst,nU,FSTI_SORTINI);                /*   Get sorted trns. iterator       */
    lpTI_4 = CFst_STI_Init(itFst,nU,FSTI_SORTINI);                /*   Get sorted trns. iterator       */
    lpTI_5 = CFst_STI_Init(itFst,nU,FSTI_SORTINI);                /*   Get sorted trns. iterator       */
    lpTI_6 = CFst_STI_Init(itFst,nU,FSTI_SORTINI);                /*   Get sorted trns. iterator       */
  
    while ((lpT_1=CFst_STI_TfromS(lpTI_1,0,lpT_1))!=NULL)         /* Enumerate trns. at 0              */  
    {                                                             /* >>                                */
      nPAgg2 = 0;
      while ((lpT_2=CFst_STI_TfromS(lpTI_2,                         /* Enumerate trns. at TER            */
        *CFst_STI_TTer(lpTI_1,lpT_1),lpT_2))!=NULL)                 /*             |                     */ 
      {                                                             /* >>                                */
        nTis_1 = *CFst_STI_TTis(lpTI_2,lpT_2);                       /* get w_{n-1} for trigrams          */
        while ((lpT_6=CFst_STI_TfromS(lpTI_6,0,lpT_6))!=NULL)         /* loop over trns. at initial state  */
        {                                                             /* >>                                */
          if(*CFst_STI_TTis(lpTI_6,lpT_6)==nTis_1)                      /* if TIS found                      */
          {                                                             /* >                                 */
            nS = *CFst_STI_TTer(lpTI_6,lpT_6);                          /* get ter-state of trns.            */
            nPAgg2 = nPAgg2 + (FLOAT64)CData_Dfetch(idTd,                /* sum it up                         */
              CFst_STI_GetTransId(lpTI_6,lpT_6),                          /*           |                       */
              nIcPsr);
          }                                                             /* <                                 */
        }                                                             /* <<                                */
        nPAgg1 = 0;                                                   /* Flush                             */
        while ((lpT_3=CFst_STI_TfromS(lpTI_3,                         /* Enumerate trns. at TER            */
          *CFst_STI_TTer(lpTI_2,lpT_2),lpT_3))!=NULL)                 /*             |                     */
        {                                                             /* >>                                */
          nTis_2=*CFst_STI_TTis(lpTI_3,lpT_3);                          /* get w_{n}                         */
          while((lpT_5=CFst_STI_TfromS(lpTI_5,nS,lpT_5))!=NULL)         /* Find trns. with TIS=w_{n}         */
          {                                                             /* >>                                */
            if(*CFst_STI_TTis(lpTI_5,lpT_5)==nTis_2)                      /* ...                               */
            {                                                             /* >                                 */   
              nPAgg1 = nPAgg1 + (FLOAT64)CData_Dfetch(idTd,                /* sum it up                         */
              CFst_STI_GetTransId(lpTI_5,lpT_5),                          /*           |                       */
            nIcPsr);                                                      /*           |                       */
            }                                                             /* <                                 */
          }                                                             /* <<                                */
        }                                                             /* <<                                */
          nAlpha = (                                                    /* Calc. alphas 4 trigrams           */
            (FLOAT64)CData_Dfetch(AS(CData,itFst->sd),                           /*           |                       */
              *CFst_STI_TTer(lpTI_2,lpT_2),nIcBet)                       /*           |                       */
              )/(1-nPAgg1);                                             /*           |                       */
          nAuxId=CData_AddRecs(idAux,1,1);                              /* Allocate new record in aux.-tab.  */
          CData_Dstore(idAux,nAlpha,nAuxId,nIcAlpha);                   /* Store alpha                       */
          CData_Dstore(idAux,(FST_ITYPE)*CFst_STI_TTer(lpTI_2,lpT_2),   /* Store state id 4 alpha            */
            nAuxId,nIcInit);                                            /*           |                       */
      }                                                             /* <<                                */
      //  nPAgg2 = (FST_WTYPE)CData_Dfetch(idTd,CFst_STI_GetTransId(lpTI_1,lpT_1),nIcPsr);
        nAlpha = (                                                    /* Calc. alphas 4 bigrams            */
          (FLOAT64)CData_Dfetch(AS(CData,itFst->sd),                   /*           |                       */
          *CFst_STI_TTer(lpTI_1,lpT_1),nIcBet)                        /*           |                       */
          )/(1-nPAgg2);                                               /*           |                       */
        nAuxId=CData_AddRecs(idAux,1,1);                              /* Allocate new record in aux.-tab.  */
        CData_Dstore(idAux,nAlpha,nAuxId,nIcAlpha);                   /* Store alpha                       */
        CData_Dstore(idAux,(INT32)*CFst_STI_TTer(lpTI_1,lpT_1),        /* Store state id 4 alpha            */ 
          nAuxId,nIcInit);                                            /*             |                     */
    }                                                             /* <<                                */
  
    /* Add new states and trns. and alphas */                     /* --------------------------------- */
    nNewCnt = CData_GetMaxRecs(idAux);                            /* # of new states to add            */
    nNewTer = CFst_Addstates(itFst,nU,nNewCnt,1);                 /* Add states, get idx of 1st new s. */
    for(INT32 i = 0; i < nNewCnt;i++)                               /* For all Recs in idAux             */
    {                                                             /* >>                                */
      nNewTId=CFst_Addtrans(itFst,nU,                               /* Add new trns. & get id            */
        (FST_ITYPE)CData_Dfetch(idAux,i,nIcInit),(FST_ITYPE)(nNewTer+i));             /*              |                    */
      nAlpha = CData_Dfetch(idAux,i,nIcAlpha);
      if(nAlpha == 0) 
        printf("\n\n WARNING, BACKOFF TRANSITION HAS NO PROBABILITY! TID=%li\n\n",nNewTId);
      CData_Dstore(idTd,nAlpha,                                     /* Store alpha for new trns.         */
        nNewTId,nIcPsr);                                            /*              |                    */
      CData_Dstore(idTd,2,nNewTId,nIcTis);
    }                                                             /* <<                                */
    /* Clean up */                                                /* --------------------------------- */
    CFst_STI_Done(lpTI_1);                                        /* Clean up iterator                 */  
    CFst_STI_Done(lpTI_2);                                        /* Clean up iterator                 */
    CFst_STI_Done(lpTI_3);                                        /* Clean up iterator                 */
    CFst_STI_Done(lpTI_5);                                        /* Clean up iterator                 */
    CFst_STI_Done(lpTI_6);                                        /* Clean up iterator                 */
    IDESTROY(idAux);                                              /* destroy aux. table                */
    free(ptr_u);                                                  /* Free allocated mem.               */
    free(ptr_b);                                                  /* Free allocated mem.               */
    free(ptr_t);                                                  /* Free allocated mem.               */
  }                                                             /* <<                                */

  return O_K;
}

FLOAT64 CGEN_PUBLIC CFsttools_Getlogprob(CFsttools *_this, INT16 nSeqLen, CData* idSeq, INT32 nUnit, CFst* itFst)
{
  FLOAT64 nLogProb = 0;
  FST_TID_TYPE* lpTI_1 = NULL;                                  /* Automaton iterator data structure */
  BYTE*         lpT_1  = NULL;                                  /* Current transition (iteration)    */
  INT32          nIcLsr = -1;                                    /* Semiring Component                */
  INT32          nS     = -1;                                    /* Current state                     */
  INT32          nU     = 0;                                     /* Current unit                      */
  INT16         nSeqStart = 0;                                  /* Sequence start in idSeq           */
  bool          bUni   = 0;                                     /* Found unigram?                    */
  bool          bTri   = 0;                                     /* Found bigram?                     */
  bool          bBig   = 0;                                     /* Found trigram?                    */
  FLOAT64        nAlpha = 0;                                     /* Alpha (backoff)                   */
  bool          NOT_FOUND = 1;                                  /* FOUND PROB ?                      */
  
  /* Validation */                                              /* --------------------------------- */
  CHECK_THIS_RV(NULL);                                          /* Check this pointer                */
  CFst_Check(itFst);                                            /* Check FST(s)                      */
  if(CFst_Wsr_GetType(itFst,NULL) != FST_WSR_LOG)               /* If not LSR                        */
  {                                                             /* >                                 */
    CFst_Wsr_Convert(itFst,FST_WSR_LOG);                        /* Convert                           */
  }                                                             /* <                                 */
  nIcLsr = CData_FindComp(AS(CData,itFst->td),NC_TD_LSR);       /* Find comp.                        */
  
  /* If not uni-, bi- or trigram: error */                      /* --------------------------------- */
  if((nSeqLen < 1) || (nSeqLen > 3))                            /* If nSeqLen: warn & go on          */
  {                                                             /* >                                 */
    printf("\n As written in the manual, nSeqLen must be an");  /* warn...                           */
    printf(" integer in [1,3]");                                /*            |                      */
    printf("\n Nevertheless it will go on with nSeqLen=1");     /*            |                      */
    nSeqLen = 1;                                                /*            |                      */
  }                                                             /* <                                 */
  
  /* Find prob. */                                              /* --------------------------------- */
  for (nU=nUnit<0?0:nUnit; nU<UD_XXU(itFst); nU++)              /* For all units ...                 */
  {                                                             /* >>                                */

      lpTI_1 = CFst_STI_Init(itFst, nU, FSTI_SORTINI);          /*   Get sorted transition iterator  */
    while(NOT_FOUND)                                            /* Found Prob?                       */
    {                                                           /* >>                                */
      INT32 nTIS = (INT32) CData_Dfetch(idSeq,nSeqStart,0);
      lpT_1=NULL;
      while((lpT_1=CFst_STI_TfromS(lpTI_1,0,lpT_1))!=NULL && !bUni)      /* Enumerate trns. at 0              */  
      {                                                         /* >>                                */
        if((INT32)*CFst_STI_TTis(lpTI_1,lpT_1) == nTIS)          /* Found TIS                         */
        {                                                       /* >                                 */
          nS = *CFst_STI_TTer(lpTI_1,lpT_1);                    /* Init state for bigram             */
          nLogProb = (FLOAT64)CData_Dfetch(AS(CData,itFst->td),  /* Get unigram-prob.                 */
            CFst_STI_GetTransId(lpTI_1,lpT_1),nIcLsr);          /*          |                        */
          bUni = 1;                                             /* Found unigram                     */
        }                                                       /* <                                 */ 
      }                                                          /* <<                                */  

      if((nSeqLen == 1) && bUni)                                /* If looked for unigram && found    */
      {                                                         /* >                                 */
        NOT_FOUND = 0;                                          /* Can leave loop                    */
        break;
      }                                                         /* <                                 */
          
      lpT_1=NULL;
      while((lpT_1=CFst_STI_TfromS(lpTI_1,nS,lpT_1))!=NULL      /* Enumerate at ter of unig.-trns.   */
        && (nSeqLen > 1) && bUni && !bBig)                               /*          |                        */
      {                                                         /* >>                                */
        if((INT32)*CFst_STI_TTis(lpTI_1,lpT_1) ==                /* Found TIS                         */
          (INT32)CData_Dfetch(idSeq,nSeqStart+1,0))              /*          |                        */
        {                                                       /* >                                 */
          nS = *CFst_STI_TTer(lpTI_1,lpT_1);                    /* Init state for trigram            */
          nLogProb = (FLOAT64)CData_Dfetch(AS(CData,itFst->td),  /* Get bigram-prob.                  */
            CFst_STI_GetTransId(lpTI_1,lpT_1),nIcLsr);          /*          |                        */
          bBig = 1;                                             /* Bigram exists                     */
        }                                                       /* <                                 */
      }                                                         /* <<                                */

      if((nSeqLen == 2) && bBig)                                /* If looked for bigram && found     */
      {                                                         /* >                                 */
        NOT_FOUND = 0;                                          /* Can leave loop                    */
        break;
      }                                                         /* <                                 */

      if(nSeqLen == 2 && !bBig && bUni)                         /* Looked for bigram && found none   */
      {                                                         /* >                                 */
        lpT_1=NULL;
        while((lpT_1=CFst_STI_TfromS(lpTI_1,nS,lpT_1))!=NULL)   /* Enumerate at ter of big.-trns.    */
        {                                                       /* >>                                */
          if((INT32)*CFst_STI_TTis(lpTI_1,lpT_1) == 2)          /* Found TIS                         */
          {                                                     /* >                                 */
            nAlpha = (FLOAT64)CData_Dfetch(AS(CData,              /* Get alpha                         */
              itFst->td),CFst_STI_GetTransId(lpTI_1,lpT_1),     /*          |                        */
              nIcLsr);                                          /*          |                        */
          }                                                     /* <                                 */
        }                                                       /* <<                                */
        nSeqStart++;                                            /* Don't care about first word       */
        nSeqLen--;                                              /* So sequence becoms unigram        */
      }                                                         /* <                                 */
      
      if(!bBig && (nSeqLen > 2))
      {
        nSeqLen--;
        nSeqStart++;
      }

      lpT_1=NULL;
      while((lpT_1=CFst_STI_TfromS(lpTI_1,nS,lpT_1))!=NULL      /* Enumerate at ter of bi.-trns.     */
        && nSeqLen > 2 && bBig)                                 /*          |                        */
      {                                                         /* >>                                */
        if((INT32)*CFst_STI_TTis(lpTI_1,lpT_1) ==                /* Found TIS                         */
          (INT32)CData_Dfetch(idSeq,nSeqStart+2,0))              /*          |                        */
        {                                                       /* >                                 */
          nLogProb = (FLOAT64)CData_Dfetch(AS(CData,itFst->td),  /* Get trigram-prob.                 */
            CFst_STI_GetTransId(lpTI_1,lpT_1),nIcLsr);          /*          |                        */  
          bTri = 1;                                             /* Trigram exists                    */
        }                                                       /* <                                 */
      }                                                         /* <<                                */

      if(bTri && nSeqLen == 3)                                  /* If looked for trigram && found    */
      {                                                         /* >                                 */
        NOT_FOUND = 0;                                          /* Can leave loop                    */
        break;
      }                                                         /* <                                 */
  
      if(!bTri && (nSeqLen == 3) && bBig)                         /* If there is no certain trigram    */
      {                                                         /* >                                 */
        lpT_1=NULL;
        while((lpT_1=CFst_STI_TfromS(lpTI_1,nS,lpT_1))!=NULL)   /* Enumerate at ter of big.-trns.    */
        {                                                       /* >>                                */
          if((INT32)*CFst_STI_TTis(lpTI_1,lpT_1) == 2)          /* Found TIS                         */
          {                                                     /* >                                 */
            nAlpha = (FLOAT64)CData_Dfetch(AS(CData,    /* Get alpha                         */
              itFst->td),CFst_STI_GetTransId(lpTI_1,lpT_1),     /*          |                        */
              nIcLsr);                                          /*          |                        */
          }                                                     /* <                                 */
        }                                                       /* <<                                */
        nSeqStart++;                                            /* Don't care about first wird       */
        nSeqLen--;                                              /* So sequence becoms bigram         */
      }                                                         /* <                                 */

      bUni = 0;                                                 /* Forget seen things                */
      bBig = 0;                                                 /*          |                        */
      bTri = 0;                                                 /*          |                        */
    }                                                           /* <<                                */
      CFst_STI_Done(lpTI_1);                                    /* Tidy up                           */
  }                                                             /* <<                                */

  MIC_PUT_N((FLOAT64)(nLogProb+nAlpha));                         /* Put result on dLabPro-stack       */
  return O_K;                                                   /* All right                         */
}

/* EOF */


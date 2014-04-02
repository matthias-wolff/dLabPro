/* dLabPro class CFst (fst)
 * - String (de)compilation
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
#include "kzl_hash.h"

#define CS_PUSH(A,B,C) {INT32 l;for(l=C;l>0;l--)A[l]=A[l-1];A[0]=B;}

/*
 * Destroys unit-lookup-table
 * @param lpLookupTable lookup table to destroy
 *
 */
INT16 CGEN_PRIVATE CFst_ClearUnitLookupTable(hash_t* lpLookupTable)
{
  hscan_t  hs;
  hnode_t* hn;

  /* Perform clearing */
  while(!hash_isempty(lpLookupTable))                                           /* While still nodes in table        */
  {                                                                             /* >>                                */
    hash_scan_begin(&hs,lpLookupTable);                                         /*    Reset node scanner             */
    if ((hn = hash_scan_next(&hs))!=NULL)                                       /*    Still elements in table ?      */
    {                                                                           /*    >>                             */
      INT32* lpVal = (INT32*) hnode_get(hn);                                      /*       Get pointer to current value*/
      char* lpKey = (char*) hnode_getkey(hn);                                   /*       Get pointer to current key  */
      hash_scan_delfree(lpLookupTable,hn);                                      /*     Delete hash node              */
      dlp_free(lpVal);                                                          /*     Delete value                  */
      dlp_free(lpKey);                                                          /*     Delete key                    */
    }                                                                           /*    <<                             */
  }                                                                             /* <<                                */
  hash_destroy(lpLookupTable);                                                  /* Destroy lookup table              */
  return O_K;                                                                   /* (Hopefully) everything is fine    */
}

/*
 * Manual page at fst_man.def
 */
INT16 CGEN_PUBLIC CFst_Compile
(
  CFst*  _this,
  CData* idSrc,
  INT32   nIcStr,
  INT32   nIcName,
  CData* idAlp
)
{
  INT32  nIcW           = -1;                                                    /* Comp. of weight in idSrc          */
  FLOAT64 nW            = 0.;                                                    /* Current path weight               */
  INT32  nIcA           = -1;                                                    /* Comp.in idAlp cont.alphabet symbs.*/
  INT32  nU             = -1;                                                    /* Current unit in dest. fst         */
  INT32  nSnw           = -1;                                                    /* First newly added state           */
  INT32  nRi            = 0;                                                     /* Index of current input string     */
  INT32  nXRi           = 0;                                                     /* Number of input strings           */
  INT32  nRa            = 0;                                                     /* Curr. symb. (=record) in alphabet */
  INT32  nXRa           = 0;                                                     /* Number of symbols alphabet        */
  INT32  nRla           = 0;                                                     /* Record length in alphabet inst.   */
  INT32  nL             = 0;                                                     /* Current segment length            */
  INT32  nXL            = -1;                                                    /* Maximal symbol length             */
  char* lpA0           = NULL;                                                  /* Ptr. to first alphabet symbol     */
  INT32  nT             = 0;                                                     /* Current transition                */
  INT32  nLen           = 0;                                                     /* Length of lpS                     */
  char* lpS            = NULL;                                                  /* Curr. char. in string to compile  */
  char  lpsBuf[L_SSTR] = "";                                                    /* Auxilary string buffer            */
  INT16 nCheck         = 0;                                                     /* Check level                       */
  BOOL  bGreedy        = FALSE;                                                 /* Copy of m_bLmatch option          */
  BOOL  bIndex         = FALSE;                                                 /* Copy of m_bIndex option           */
  BOOL  bPush          = FALSE;                                                 /* Copy of m_bPush option            */
  hash_t* lpUnitLookupTable = NULL;                                             /* Lookup-table for unit names       */
  hnode_t* lpHNode     = NULL;                                                  /* Current hash node                 */
  INT32* lpVal          = NULL;                                                  /* Current value                     */
  char* lpKey          = NULL;                                                  /* Current key                       */

  /* Prepare destination */                                                     /* --------------------------------- */
  CHECK_THIS_RV(NOT_EXEC);                                                      /* Check this pointer                */
  CFst_Check(_this);                                                            /* Check this instance               */
  bGreedy = _this->m_bGreedy;                                                   /* Remember greedy flag              */
  bIndex  = _this->m_bIndex;                                                    /* Remember index flag               */
  bPush   = _this->m_bPush;                                                     /* Remember push flag                */
  nCheck  = BASEINST(_this)->m_nCheck;                                          /* Remember check level              */
  IFCHECKEX(1)                                                                  /* On verbose level 1                */
  {                                                                             /* >>                                */
    printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());             /*   Print protocol header           */
    printf("\n CFst_Compile(%s,%s,%ld,%ld,%s)",                                 /*   ...                             */
      BASEINST(_this)->m_lpInstanceName,                                        /*   ...                             */
      idSrc?BASEINST(idSrc)->m_lpInstanceName:"NULL",                           /*   ...                             */
      (long)nIcStr,nIcName,idAlp?BASEINST(idAlp)->m_lpInstanceName:"NULL");     /*   ...                             */
    dlp_strcpy(lpsBuf,"\n ");                                                   /*   ...                             */
    if (bGreedy) dlp_strcat(lpsBuf,"/greedy ");                                 /*   ...                             */
    if (bIndex ) dlp_strcat(lpsBuf,"/index " );                                 /*   ...                             */
    if (bPush  ) dlp_strcat(lpsBuf,"/push "  );                                 /*   ...                             */
    if (dlp_strlen(lpsBuf)>2) printf(lpsBuf);                                   /*   ...                             */
    printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());             /*   ...                             */
    printf("\n");                                                               /*   ...                             */
  }                                                                             /* <<                                */
  CFst_Reset(BASEINST(_this),TRUE);                                             /* Reset this instance               */
  BASEINST(_this)->m_nCheck = nCheck;                                           /* Restore check level               */
  CData_AddComp(AS(CData,_this->td),NC_TD_TIS,DLP_TYPE(FST_ITYPE));             /* Add input symbol component        */
  if (bIndex) CData_AddComp(AS(CData,_this->td),NC_TD_TOS,DLP_TYPE(FST_ITYPE)); /* Making FST add output symbol comp.*/

  /* Seek and analyze alphabet symbols */                                       /* --------------------------------- */
  nXRa = CData_GetNRecs(idAlp);                                                 /* Get number of alphabet symbols    */
  nRla = CData_GetRecLen(idAlp);                                                /* Get record length                 */
  for (nIcA=0; nIcA<CData_GetNComps(idAlp); nIcA++)                             /* Loop over components of idAlp     */
    if (dlp_is_symbolic_type_code(CData_GetCompType(idAlp,nIcA)))               /*   Find first symbolic one         */
      break;                                                                    /*     Gotcha!                       */
  if (nIcA>=CData_GetNRecs(idAlp))                                              /* Or didn't I?                      */
    return IERROR(_this,FST_MISS,"symbolic component","","idAlp");              /*   Error                           */
  lpA0 = (char*)CData_XAddr(idAlp,0,nIcA);                                      /* Get ptr. to first alphabet symbol */
  for(nRa=0;nRa<nXRa;nRa++)                                                     /* Loop over alphabet symbols        */
    if((nL=(INT32)dlp_strlen(lpA0+nRa*nRla))>nXL) nXL=nL;                       /*   Remember longest symbol so far  */
  IFCHECKEX(1)                                                                  /* On verbose level 1                */
  {                                                                             /* >>                                */
    printf("\n Alphabet");                                                      /*   Tell a bit about the alphabet   */
    printf("\n - Symbol component  : %ld",(long)nIcA);                          /*   ...                             */
    printf("\n - Max. symbol length: %ld",(long)nXL );                          /*   ...                             */
  }                                                                             /* <<                                */

  /* Validate source instance */                                                /* --------------------------------- */
  if (!dlp_is_symbolic_type_code(CData_GetCompType(idSrc,nIcName)))             /* Name component not symbolic       */
  {                                                                             /* >>                                */
    nIcName=-1;                                                                 /*   Silently forget it              */
    IFCHECK printf("\n Ignoring bad unit name component!");                     /*   Ok, not so silently :)          */
  }                                                                             /* <<                                */
  if (!dlp_is_symbolic_type_code(CData_GetCompType(idSrc,nIcStr)))              /* Source string comp. not symbolic  */
    return IERROR(_this,FST_BADCTYPE,"string",nIcStr,0);                        /*   That's a problem ...            */
  nXRi = CData_GetNRecs(idSrc);                                                 /* Get number of input strings       */
  nIcW = CData_FindComp(idSrc,"W");                                             /* Find weight component             */
  if (nIcW>=0)CData_AddComp(AS(CData,_this->td),NC_TD_LSR,DLP_TYPE(FST_WTYPE)); /* Making wFST add weight comp.      */

  /* Create hash lookup table */                                                /* --------------------------------- */
  IFCHECK printf("\n\n Creating lookup table");                                 /* Protocol                          */
  lpUnitLookupTable = hash_create(CData_GetNRecs(idSrc),NULL,NULL,NULL);        /*Initialize lookup table            */

  /* Loop over input records */                                                 /* --------------------------------- */
  IFCHECK printf("\n\n Compiling %ld string%s",(long)nXRi,nXRi==1?"":"s");      /* Protocol                          */
  for (nRi=0; nRi<nXRi; nRi++)                                                  /* For every input string            */
  {                                                                             /* >>                                */
    lpS = (char*)CData_XAddr(idSrc,nRi,nIcStr);                                 /*   Get ptr. to input string        */
    nW  = nIcW>=0 ? CData_Dfetch(idSrc,nRi,nIcW) : 0.;                          /*   Get current path weight         */
    IFCHECK printf("\n Entry: %d \t - \"%s\"",(int)nRi,lpS);                    /*   Protocol                        */

    /* Find or add unit */                                                      /*   - - - - - - - - - - - - - - - - */
    nU = -1;                                                                    /*   Output unit unknown yet         */
    if (nIcName>=0)                                                             /*   Have name component             */
    {                                                                           /*   >>                              */
      strcpy(lpsBuf,(char*)CData_XAddr(idSrc,nRi,nIcName));                     /*     Copy unit name                */
      lpKey = NULL;                                                             /*     Reset key                     */
      lpVal = NULL;                                                             /*     Reset value                   */
      lpHNode = NULL;                                                           /*     Reset hash-node               */
      lpHNode = hash_lookup(lpUnitLookupTable, lpsBuf);                         /*     Lookup curr. entry (unit name)*/
      if (!lpHNode)                                                             /*     New unit name?                */
      {                                                                         /*     >>                            */
        lpVal  = (INT32*) dlp_calloc(1,sizeof(INT32));                            /*       Alloc. memory for new value */
        *lpVal = ((INT32)UD_XXU(_this));                                        /*       Set value to curr.unit index*/
        lpKey  = (char*) dlp_calloc(L_SSTR,sizeof(char));                       /*       Allocate memory for key     */
        strcpy(lpKey,lpsBuf);                                                   /*       Set key to current name     */
        hash_alloc_insert(lpUnitLookupTable,lpKey,lpVal);                       /*       Add (key,value) pair        */
        nU = -1;                                                                /*       This unit is a new one      */
      }                                                                         /*     <<                            */
      else                                                                      /*     Already in table              */
      {                                                                         /*     >>                            */
        lpVal = (INT32*)hnode_get(lpHNode);                                     /*      Get unit index               */
        nU = *lpVal;                                                            /*      Not a new unit               */
      }                                                                         /*     <<                            */
    }                                                                           /*   <<                              */
    else                                                                        /*   Have no name component          */
      sprintf(lpsBuf,"%s_%ld",BASEINST(idSrc)->m_lpInstanceName,(long)nRi);     /*     Create dummy unit name        */
    if (nU<0)                                                                   /*   No suitable unit found          */
    {                                                                           /*   >>                              */
      nU = CFst_Addunit(_this,lpsBuf);                                          /*     Create new one                */
      CFst_Addstates(_this,nU,1,FALSE);                                         /*     Add start state               */
    }                                                                           /*   <<                              */
    IFCHECK printf("\t = unit %ld",(long)nU);                                   /*   Protocol                        */
    if (!dlp_strlen(lpS)) continue;                                             /*   No input string -> forget it    */

    /* Compile string */                                                        /*   - - - - - - - - - - - - - - - - */
    if ((nLen=(INT32)dlp_strlen(lpS))>1)                                        /*   Input string longer than 1 >>   */
      nSnw = CFst_Addstates(_this,nU,nLen-1,FALSE)-UD_FS(_this,nU);             /*     Add sufficient no. of states  */
    CFst_Addstates(_this,nU,1,TRUE);                                            /*   Plus a final one                */
    if (nLen==1) nSnw = UD_XS(_this,nU)-1;                                      /*   The final state was the 1st one */

    for(nT=0;nT<nLen;nT++) if(lpS[nT]=='('){                                    /*   Loop over bracket begins >>     */
      INT32 nE;                                                                 /*   End of bracket segment          */
      INT32 nI;                                                                 /*   Pos within bracket segment      */
      INT32 nDepth;                                                             /*   Bracket depth                   */
      CFst_AddtransEx(_this,nU,nT==0?0:nSnw+nT-1,nSnw+nT,-1,-1,nT?0.:nW);       /*   Add transition over '('         */
      for(nDepth=0,nE=nT+1;nE<nLen;nE++){                                       /*   Find end of bracket segment >>  */
        if(lpS[nE]=='(') nDepth++;                                              /*     Increase depth                */
        if(lpS[nE]==')' && !nDepth--) break;                                    /*     Decrease depth or found end   */
      }                                                                         /*   <<                              */
      if(nE==nLen){                                                             /*   No segment end found ? >>       */
        IERROR(_this,FST_SYMBOLNOTFOUND,"')' for '('",0,0);                     /*     ERROR                         */
        continue;                                                               /*     Nothing to do                 */
      }                                                                         /*   <<                              */
      CFst_AddtransEx(_this,nU,nSnw+nE-1,nSnw+nE,-1,-1,0.);                     /*   Add transition over ')'         */
      for(nDepth=0,nI=nT+1;nI<nE;nI++){                                         /*   Find alternatives '|' >>        */
        if(lpS[nI]=='(') nDepth++;                                              /*     Increase depth                */
        if(lpS[nI]==')') nDepth--;                                              /*     Decrease depth                */
        if(lpS[nI]=='|' && !nDepth){                                            /*     Found '|' in current depth >> */
          CFst_AddtransEx(_this,nU,nT==0?0:nSnw+nT-1,nSnw+nI,-1,-1,0.);         /*       Add trans. from '(' to '|'  */
          CFst_AddtransEx(_this,nU,nSnw+nI-1,nSnw+nE,-1,-1,0.);                 /*       Add trans. from '|' to ')'  */
        }                                                                       /*     <<                            */
      }                                                                         /*   <<                              */
    }                                                                           /* <<                                */

    for(nT=0;*lpS;lpS++,nT++){                                                  /*   Loop over segment offsets >>    */
      if(lpS[0]=='(' || lpS[0]==')' || lpS[0]=='|') continue;                   /*     Skip meta characters          */
      for(nL=MIN(nXL,(INT32)dlp_strlen(lpS));nL;nL--){                          /*     Loop over segments lengths >> */
        IFCHECKEX(2){                                                           /*       Protocol >>                 */
          dlp_memmove(lpsBuf,lpS,nL); lpsBuf[nL]='\0';                          /*         Copy segment              */
          printf("\n   - Segment \"%s\"",lpsBuf);                               /*         Protocol                  */
        }                                                                       /*       <<                          */
        for(nRa=0;nRa<nXRa;nRa++)                                               /*       Loop over alphabet symbols  */
          if(!dlp_strncmp(lpA0+nRa*nRla,lpS,nL)) break;                         /*         Symbol=segment => Gotcha! */
        if (nRa<nXRa)                                                           /*       Gotcha?                     */
        {                                                                       /*       >>                          */
          IFCHECKEX(2) printf(" = symbol %ld",(long)nRa);                       /*         Protocol                  */
          CFst_AddtransEx(_this,nU,nT==0?0:nSnw+nT-1,nSnw+nT+nL-1,nRa,-1,nT?0.:nW); /*     Add a new transition      */
          if (bGreedy) { lpS+=nL-1; nT+=nL-1; break; }                          /*         Greedy match              */
        }                                                                       /*       <<                          */
      }                                                                         /*     <<                            */
      if(!nL && bGreedy){                                                       /*     Greedy match >>               */
        dlp_memmove(lpsBuf,lpS,nXL); lpsBuf[nXL]='\0';                          /*       Store forthcoming segment  */
        IERROR(_this,FST_SYMBOLNOTFOUND,lpsBuf,0,0);                            /*       it's all over :((           */
      }                                                                         /*     <<                            */
    }                                                                           /*   <<                              */
  }                                                                             /* <<                                */

  /* Clear unit lookup table*/                                                  /* --------------------------------- */
  IFCHECK printf("\n\nClearing unit lookup table");                             /* Protocol                          */
  CFst_ClearUnitLookupTable(lpUnitLookupTable);                                 /* Clear lookup table                */

  /* Trim */                                                                    /* --------------------------------- */
  IFCHECK printf("\n Trimming FSMs");                                           /* Protocol                          */
  BASEINST(_this)->m_nCheck = 0;                                                /* Switch protocol off               */
  for (nU=0; nU<UD_XXU(_this); nU++) CFst_TrimStates(_this,nU);                 /* Trim all units                    */
  BASEINST(_this)->m_nCheck = nCheck;                                           /* Switch protocol back on           */

  /* Add output symbol */                                                       /* --------------------------------- */
  IFCHECK printf("\n Adding output symbols");                                   /* Protocol                          */
  if (bIndex)                                                                   /* When adding output symbols        */
    for (nU=0; nU<UD_XXU(_this); nU++)                                          /*   Loop over units                 */
      for (nT=UD_FT(_this,nU); nT<UD_FT(_this,nU)+UD_XT(_this,nU); nT++)        /*     Loop over transitions         */
        if                                                                      /*       If ...                      */
        (                                                                       /*       |                           */
          (bPush && SD_FLG(_this,UD_FS(_this,nU)+TD_TER(_this,nT))&SD_FLG_FINAL)/*       | Pushing and final trans.  */
          || (!bPush && TD_INI(_this,nT)==0)                                    /*       | OR not push.and ini.trans.*/
        )                                                                       /*       |                           */
        {                                                                       /*       >>                          */
          CData_Dstore(AS(CData,_this->td),(FLOAT64)nU,nT,IC_TD_DATA+1);        /*         Store unit index          */
        }                                                                       /*       <<                          */

  /* Add symbol tables */                                                       /* --------------------------------- */
  CData_SelectComps(AS(CData,_this->is),idAlp,nIcA,1);                          /* Copy input symbol table           */
  if(bIndex) CData_SelectComps(AS(CData,_this->os),AS(CData,_this->ud),0,1);    /* Copy output symbol table          */

  /* Check result */                                                            /* --------------------------------- */
  CFst_Check(_this);                                                            /* Check this instance               */
  IFCHECKEX(1)                                                                  /* On verbose level 1                */
  {                                                                             /* >>                                */
    printf("\n\n CFst_Compile done.\n");                                        /*   Print protocol footer           */
    dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());                           /*   ...                             */
    printf("\n");                                                               /*   ...                             */
  }                                                                             /* <<                                */
  return O_K;                                                                   /* Yo...                             */
}

/* EOF */

/* dLabPro class CFsttools (fsttools)
 * - optimization methods
 *
 * AUTHOR : Frank Duckhorn
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

/*
 * Manual page at fst.def
 */
INT16 CGEN_PUBLIC CFsttools_SumCompId(CFsttools* _this,CData* Src,INT32 Cid,INT32 DstRecs,CData* Dst)
{
  INT32 i,j;                                                                     /* Loop indizies                     */
  FLOAT64 *lpSrc=NULL;                                                           /* Buffer for source matrix          */
  FLOAT64 *lpDst=NULL;                                                           /* Buffer for destination matrix     */
  INT32 N;                                                                       /* Number of records in source mat.  */
  INT32 C;                                                                       /* Number of components in src. mat. */
  INT32 id;                                                                      /* Id value                          */

  C=CData_GetNComps(Src);                                                       /* Get number of components          */
  N=CData_GetNRecs(Src);                                                        /* Get number of records             */

  /* Validation */                                                              /* --------------------------------- */
  if (CData_IsEmpty(Src)) return IERROR(Src,DATA_EMPTY,"idSrc",0,0);            /* Need source matrix                */
  if (CData_GetNBlocks(Src)>1) return IERROR(Src,DATA_MDIM,"idSrc",0,0);        /* Only one block implemented        */
  if (Cid<0 || Cid>=C) return IERROR(Src,ERR_INVALARG,"nCid",0,0);              /* Cid should be a valid comp.-index */

  lpSrc=(FLOAT64*)dlp_calloc(C*N,sizeof(FLOAT64));                                /* Alloc memory of source matrix     */
  lpDst=(FLOAT64*)dlp_calloc(C*DstRecs,sizeof(FLOAT64));                          /* Alloc memory of destination mat.  */

  CData_DblockFetch(Src,lpSrc,0,C,N,-1);                                        /* Get first block of source mat.    */
  CData_Copy(Dst,Src);                                                          /* Copy source mat. to dst. mat.     */
  CData_AllocateUninitialized(Dst,DstRecs);                                     /* Realloc dst. mat.                 */
  CData_Fill(Dst,CMPLX(0),CMPLX(0));                                            /* Clear dst. mat.                   */
  for(i=0;i<N;i++){                                                             /* Loop over all rec. in src. mat. >>*/
    id=(INT32)lpSrc[i*C+Cid];                                                    /*   Get Id value                    */
    if(id>=0 && id<DstRecs) for(j=0;j<C;j++) if(Cid!=j)                         /*   If Id valid, loop over comp.'s  */
      lpDst[id*C+j]+=lpSrc[i*C+j];                                              /*     Add up comp. values to dst.   */
  }                                                                             /* <<                                */
  CData_DblockStore(Dst,lpDst,0,C,DstRecs,-1);                                  /* Save block in dst. mat.           */
  dlp_free(lpSrc);                                                              /* Free source matrix buffer         */
  dlp_free(lpDst);                                                              /* Free dst. matrix buffer           */
  CData_DeleteComps(Dst,Cid,1);                                                 /* Delete Id component from dst. mat.*/
  return O_K;                                                                   /* All done                          */
}

/**
 * Read unigram and bigram RC's per word from language model fst 
 *
 * @param _this     Pointer to fsttools object
 * @param itLM      Pointer to language model as fst
 * @param nNWord    Number of words in language model
 * @param lpUnigram Pointer to buffer for unigram RC's per word (initialized with zeros)
 * @param lpBigram  Pointer to buffer for bigram RC's per word (initialized with zeros)
 *
 * @return Success of conversion
 */
INT16 lcc_lm2ngram(CFsttools *_this,CFst *itLM,INT32 nNWord,INT32 *lpUnigram,INT32 *lpBigram)
{
  INT32 nNState;                                                                 /* Number of states in LM            */
  INT32 nNTrans;                                                                 /* Number of transitions in LM       */
  INT32 nCTIS;                                                                   /* Comp. index of input symbol       */
  INT32 nCRC;                                                                    /* Comp. index of RC                 */
  INT32 nT;                                                                      /* Current transition index          */
  INT32 nW;                                                                      /* Current word index                */
  INT32 nIni;                                                                    /* Current initial state index       */
  INT32 nTer;                                                                    /* Current terminal state index      */
  INT32 nRC;                                                                     /* RC at current transition          */
  INT32 *lpStateW;                                                               /* Buffer for map state -> word idx. */

  /* Initialization */                                                          /* --------------------------------- */
  nNState=CData_GetNRecs(itLM->sd);                                             /* Get number of states              */
  nNTrans=CData_GetNRecs(itLM->td);                                             /* Get number of transitions         */
  lpStateW=(INT32 *)dlp_malloc(sizeof(INT32)*nNState);                            /* Memory for state -> word map      */
  dlp_memset(lpStateW,-1,sizeof(INT32)*nNState);                                 /* Clear state -> word map           */
  nCTIS=CData_FindComp(itLM->td,NC_TD_TIS);                                     /* Get comp. index of input symbol   */
  nCRC=CData_FindComp(itLM->td,NC_TD_RC);                                       /* Get comp. index of RC             */
  CData_Sortup(itLM->td,itLM->td,IC_TD_INI);                                    /* Sort transitions by input symbol  */

  /* Read ngram RC's from LM */                                                 /* --------------------------------- */
  for(nT=0;nT<nNTrans;nT++){                                                    /* Loop over all transitions >>      */
    nIni=(INT32)CData_Dfetch(itLM->td,nT,IC_TD_INI);                             /*   Get current initial state       */
    nW=(INT32)CData_Dfetch(itLM->td,nT,nCTIS);                                   /*   Get current input word          */
    nRC=(INT32)CData_Dfetch(itLM->td,nT,nCRC);                                   /*   Get current RC                  */
    if(nIni==0){                                                                /*   Trans. is unigram trans. ? >>   */
      nTer=(INT32)CData_Dfetch(itLM->td,nT,IC_TD_TER);                           /*     Get current terminal state    */
      if(nTer>=nNState) return 1;                                               /*     Error if ter. state out of n. */
      lpStateW[nTer]=nW;                                                        /*     Save unigram word for state   */
      lpUnigram[nW]+=nRC;                                                       /*     Save unigram RC for word      */
    }else{                                                                      /*   << Else bigram trans. >>        */
      if(nIni>=nNState || nIni<0 || lpStateW[nIni]<0) return 1;                 /*     Error if ini. state out of n. */
      lpBigram[lpStateW[nIni]*nNWord+nW]+=nRC;                                  /*     Save bigram RC for words      */
    }                                                                           /*   <<                              */
  }                                                                             /* <<                                */

  /* Finialization */                                                           /* --------------------------------- */
  dlp_free(lpStateW);                                                           /* Free memory                       */

  return 0;                                                                     /* All done                          */
}

#define LCC_RC2PROB(rc)    ((rc>0) ? (FLOAT64)(rc)*log10(rc) : 0)                 /* Calc log probability from RC      */

/**
 * Calculate likelihood of training text for current classes
 *
 * @param nNClass    Number of classes
 * @param lpCUnigram Unigram RC's per class
 * @param lpCBigram  Bigram RC's per class
 *
 * @return likelihood of training text
 */
FLOAT64 lcc_calcprob(INT32 nNClass,INT32 *lpCUnigram,INT32 *lpCBigram){
  INT32   nC1;                                                                   /* Current class index #1            */
  INT32   nC2;                                                                   /* Current class index #2            */
  FLOAT64 nProb = 0.;                                                            /* Likelihood aggregator             */

  for(nC1=0;nC1<nNClass;nC1++) if(lpCUnigram[nC1]){                             /* Loop over all classes with RC>0 >>*/
    for(nC2=0;nC2<nNClass;nC2++) nProb+=LCC_RC2PROB(lpCBigram[nC1*nNClass+nC2]);/*   Upd. likelihood by all bigrams  */
    nProb-=2*LCC_RC2PROB(lpCUnigram[nC1]);                                      /*   Upd. likelihood by unigram RC   */
  }                                                                             /* <<                                */

  return nProb;                                                                 /* Return likelihood                 */
}

/**
 * Generate startup class map (one class for word 0..2, all other words in class 3)
 *
 * @param nNWord     Number of word in language model
 * @param lpUnigram  Unigram RC's per word
 * @param lpBigram   Bigram RC's per word
 * @param nNClass    Number of classes
 * @param lpClass    Class map (word -> class)
 * @param lpCUnigram Unigram RC's per class (initialized with zeros)
 * @param lpCBigram  Unigram RC's per class (initialized with zeros)
 */
void lcc_initclass(INT32 nNWord,INT32 *lpUnigram,INT32 *lpBigram,INT32 nNClass,INT32 *lpClass,INT32 *lpCUnigram,INT32 *lpCBigram)
{
  INT32 nW1;                                                                     /* Current word index #1             */
  INT32 nW2;                                                                     /* Current word index #2             */

  for(nW1=0;nW1<nNWord;nW1++) lpClass[nW1]=nW1<3?nW1:3;                         /* Generate class map                */
  for(nW1=0;nW1<nNWord;nW1++){                                                  /* Loop over all words #1 >>         */
    lpCUnigram[lpClass[nW1]]+=lpUnigram[nW1];                                   /*   Upd. unigram RC per class       */
    for(nW2=0;nW2<nNWord;nW2++)                                                 /*   Loop over all words #2          */
      lpCBigram[lpClass[nW1]*nNClass+lpClass[nW2]]+=lpBigram[nW1*nNWord+nW2];   /*     Upd. bigram RC per class      */
  }                                                                             /* <<                                */
}

/**
 * Move one word from one class to another and update
 * ngram RC's per class and likelihood of training text
 *
 * @param nW         Word index of moving word
 * @param nC_dst     Destination class index
 * @param nNWord     Number of words in language model
 * @param lpUnigram  Unigram RC's per word
 * @param lpBigram   Bigram RC's per word
 * @param nNClass    Number of classes
 * @param lpClass    Class map (word -> class)
 * @param lpCUnigram Unigram RC's per class
 * @param lpCBigram  Bigram RC's per class
 * @param nProb      Likelihood of training text
 */
void lcc_moveword(INT32 nW,INT32 nC_dst,INT32 nNWord,INT32 *lpUnigram,INT32 *lpBigram,INT32 nNClass,INT32 *lpClass,INT32 *lpCUnigram,INT32 *lpCBigram,FLOAT64 *nProb)
{
  INT32  nC_src=lpClass[nW];                                                     /* Source class index                */
  INT32  nC;                                                                     /* Current class index               */
  INT32  nW2;                                                                    /* Current word index                */
  INT32* lpC1;                                                                   /* Pointer in class bigram buffer #1 */
  INT32* lpC2;                                                                   /* Pointer in class bigram buffer #2 */
  INT32* lpB;                                                                    /* Pointer in word bigram buffer     */

  /* Initialization */                                                          /* --------------------------------- */
  if(nC_src==nC_dst) return;                                                    /* Src. and dst. equal => do nothing */
  lpClass[nW]=nC_dst;                                                           /* Update class map                  */

  /* Update unigram RC's per class */                                           /* --------------------------------- */
  *nProb+=2*LCC_RC2PROB(lpCUnigram[nC_src]);                                    /* Remove src. class likelihood      */
  *nProb+=2*LCC_RC2PROB(lpCUnigram[nC_dst]);                                    /* Remove dst. class likelihood      */
  lpCUnigram[nC_src]-=lpUnigram[nW];                                            /* Update unigram RC of src. class   */
  lpCUnigram[nC_dst]+=lpUnigram[nW];                                            /* Update unigram RC of dst. class   */
  *nProb-=2*LCC_RC2PROB(lpCUnigram[nC_src]);                                    /* Add src. class likelihood         */
  *nProb-=2*LCC_RC2PROB(lpCUnigram[nC_dst]);                                    /* Add dst. class likelihood         */

  /* Update bigram RC's per class with moving word as second word */            /* --------------------------------- */
  for(nC=0;nC<nNClass;nC++) if(lpCUnigram[nC]){                                 /* Loop over all classes with RC>0 >>*/
    *nProb-=LCC_RC2PROB(lpCBigram[nC*nNClass+nC_src]);                          /*   Remove src. class likelihood    */
    *nProb-=LCC_RC2PROB(lpCBigram[nC*nNClass+nC_dst]);                          /*   Remove dst. class likelihood    */
  }                                                                             /* <<                                */
  lpCBigram[nC_src*nNClass+nC_src]-=lpBigram[nW*nNWord+nW];                     /* Update bigram RC of src. class    */
  lpCBigram[nC_dst*nNClass+nC_dst]+=lpBigram[nW*nNWord+nW];                     /* Update bigram RC of dst. class    */
  lpB=lpBigram+nW;                                                              /* Initialize word bigram pointer    */
  lpC1=lpCBigram+nC_src;                                                        /* Initialize src. class bigram pnt. */
  lpC2=lpCBigram+nC_dst;                                                        /* Initialize dst. class bigram pnt. */
  for(nW2=0;nW2<nNWord;nW2++,lpB+=nNWord) if(nW!=nW2 && *lpB){                  /* Loop over all words with RC>0 >>  */
    lpC1[lpClass[nW2]*nNClass]-=*lpB;                                           /*   Update bigram RC of src. class  */
    lpC2[lpClass[nW2]*nNClass]+=*lpB;                                           /*   Update bigram RC of dst. class  */
  }                                                                             /* <<                                */
  for(nC=0;nC<nNClass;nC++) if(lpCUnigram[nC]){                                 /* Loop over all classes with RC>0 >>*/
    *nProb+=LCC_RC2PROB(lpCBigram[nC*nNClass+nC_src]);                          /*   Add src. class likelihood       */
    *nProb+=LCC_RC2PROB(lpCBigram[nC*nNClass+nC_dst]);                          /*   Add dst. class likelihood       */
  }                                                                             /* <<                                */

  /* Update bigram RC's per class with moving word as first word */             /* --------------------------------- */
  for(nC=0;nC<nNClass;nC++) if(lpCUnigram[nC]){                                 /* Loop over all classes with RC>0 >>*/
    *nProb-=LCC_RC2PROB(lpCBigram[nC_src*nNClass+nC]);                          /*   Remove src. class likelihood    */
    *nProb-=LCC_RC2PROB(lpCBigram[nC_dst*nNClass+nC]);                          /*   Remove dst. class likelihood    */
  }                                                                             /* <<                                */
  lpB=lpBigram+nW*nNWord;                                                       /* Initialize word bigram pointer    */
  lpC1=lpCBigram+nC_src*nNClass;                                                /* Initialize src. class bigram pnt. */
  lpC2=lpCBigram+nC_dst*nNClass;                                                /* Initialize dst. class bigram pnt. */
  for(nW2=0;nW2<nNWord;nW2++,lpB++) if(nW!=nW2 && *lpB){                        /* Loop over all words with RC>0 >>  */
    lpC1[lpClass[nW2]]-=*lpB;                                                   /*   Update bigram RC of src. class  */
    lpC2[lpClass[nW2]]+=*lpB;                                                   /*   Update bigram RC of dst. class  */
  }                                                                             /* <<                                */
  for(nC=0;nC<nNClass;nC++) if(lpCUnigram[nC]){                                 /* Loop over all classes with RC>0 >>*/
    *nProb+=LCC_RC2PROB(lpCBigram[nC_src*nNClass+nC]);                          /*   Add src. class likelihood       */
    *nProb+=LCC_RC2PROB(lpCBigram[nC_dst*nNClass+nC]);                          /*   Add dst. class likelihood       */
  }                                                                             /* <<                                */
}

/**
 * Move one word to that class where the likelihood of
 * training text is maximal
 *
 * @param nW         Word index of moving word
 * @param nNWord     Number of words in language model
 * @param lpUnigram  Unigram RC's per word
 * @param lpBigram   Bigram RC's per word
 * @param nNClass    Number of classes
 * @param lpClass    Class map (word -> class)
 * @param lpCUnigram Unigram RC's per class
 * @param lpCBigram  Bigram RC's per class
 *
 * @return 1 if word was moved, otherwise 0
 */
INT16 lcc_optmoveword(INT32 nW,INT32 nNWord,INT32 *lpUnigram,INT32 *lpBigram,INT32 nNClass,INT32 *lpClass,INT32 *lpCUnigram,INT32 *lpCBigram)
{
  INT32   nC;                                                                    /* Current class index               */
  INT32   nC_src=lpClass[nW];                                                    /* Source class index                */
  INT32   nC_max;                                                                /* Class index with max. likelihood  */
  FLOAT64 nProb;                                                                 /* Likelihood for word in cur. class */
  FLOAT64 nProb_max;                                                             /* Maximal likelihood                */

  nProb_max=nProb=lcc_calcprob(nNClass,lpCUnigram,lpCBigram);                   /* Calc. likelihood in src. class    */
  nC_max=nC_src;                                                                /* Max. up to now is src. class      */
  for(nC=3;nC<nNClass;nC++) if(nC!=nC_src){                                     /* Loop over all possible classes >> */
    lcc_moveword(nW,nC,nNWord,lpUnigram,lpBigram,                               /*   Move word to that class & upd.  */
      nNClass,lpClass,lpCUnigram,lpCBigram,&nProb);                             /*   | ngram RC's and likelihood     */
    if(nProb>nProb_max){                                                        /*   If likelihood > maximal lkl. >> */
      nProb_max=nProb;                                                          /*     Save max. likelihood          */
      nC_max=nC;                                                                /*     Save class index for max.     */
    }                                                                           /*   <<                              */
  }                                                                             /* <<                                */
  if((nW+1)%(nNWord/20)==0) printf(".");                                        /* Protocol 20 points over all words */
  lcc_moveword(nW,nC_max,nNWord,lpUnigram,lpBigram,                             /* Move word to dst. class & update  */
    nNClass,lpClass,lpCUnigram,lpCBigram,&nProb);                               /* | ngram RC's and likelihood       */

  return nC_src!=nC_max;                                                        /* Return wheter word was moved      */
}

/**
 * Do one clustering loop iteration (successively move all words
 * to optimal classes)
 *
 * @param nL         Index of current loop iteration
 * @param nNWord     Number of words in language model
 * @param lpUnigram  Unigram RC's per word
 * @param lpBigram   Bigram RC's per word
 * @param nNClass    Number of classes
 * @param lpClass    Class map (word -> class)
 * @param lpCUnigram Unigram RC's per class
 * @param lpCBigram  Bigram RC's per class
 *
 * @return Number of words moved to different classes
 */
INT16 lcc_iter(INT32 nL,INT32 nNWord,INT32 *lpUnigram,INT32 *lpBigram,INT32 nNClass,INT32 *lpClass,INT32 *lpCUnigram,INT32 *lpCBigram)
{
  INT32 nW;                                                                      /* Current word index                */
  INT32 nNWmove=0;                                                               /* Number of words moved             */

  printf("\n LM Cluster: doing iteration %i ",nL+1);                            /* Protocol current iteration        */
  for(nW=3;nW<nNWord;nW++) if(lcc_optmoveword(nW,nNWord,lpUnigram,lpBigram,     /* Successively move all words to    */
    nNClass,lpClass,lpCUnigram,lpCBigram)) nNWmove++;                           /* | optimal classes & upd. nNWmove  */
  printf(" done. (moved words: %i/%i)",nNWmove,nNWord);                         /* Protocol number of words moved    */

  return nNWmove;                                                               /* Return number of words moved      */
}

/**
 * Create class map data object from internal class map
 *
 * @param nWWord     Number of words in language model
 * @param nNClass    Number of classes
 * @param lpClass    Internal class map
 * @param lpUnigram  Unigram RC's per word
 * @param idClassMap External class map data object
 */
void lcc_ngram2classmap(INT32 nNWord,INT32 nNClass,INT32 *lpClass,INT32 *lpUnigram,CData *idClassMap)
{
  INT32 nW;                                                                      /* Current word index                */
  INT32 *lpMap;                                                                  /* Pointer to data of class map obj. */

  CData_Array(idClassMap,T_LONG,2,nNWord);                                      /* Create class map object for words */
  lpMap=(INT32 *)CData_XAddr(idClassMap,0,0);                                    /* Get data pointer for object       */
  for(nW=0;nW<nNWord;nW++){                                                     /* Loop over all words >>            */
    lpMap[nW*2]=lpClass[nW];                                                    /*    Copy class index to obj.       */
    lpMap[nW*2+1]=lpUnigram[nW];                                                /*    Copy unigram RC to obj.        */
  }                                                                             /* <<                                */
  CData_SetCname(idClassMap,0,"~CLA");                                          /* Set comp. name of class indizies  */
  CData_SetCname(idClassMap,1,"~RC");                                           /* Set comp. name of RC's            */
}

/*
 * Manual page at fst.def
 */
INT16 CGEN_PUBLIC CFsttools_LmClusterClasses(CFsttools* _this,CFst* itLM,INT32 nNClass,INT32 nNLoops,CData* idClassMap)
{
  INT32   nNWord;                                                                /* Number of words in lang. mod.     */
  INT32   nNTrans;                                                               /* Number of transitions in LM       */
  INT32   nCTIS;                                                                 /* Comp. index of input symbol       */
  INT32   nT;                                                                    /* Current trans. index              */
  INT32   nW;                                                                    /* Current word index                */
  INT32   nL;                                                                    /* Current loop index                */
  INT32*  lpUnigram;                                                             /* Buffer for unigram RC's per word  */
  INT32*  lpBigram;                                                              /* Buffer for bigram RC's per word   */
  INT32*  lpClass;                                                               /* Buffer for class map              */
  INT32*  lpCUnigram;                                                            /* Buffer for unigram RC's per class */
  INT32*  lpCBigram;                                                             /* Buffer for bigram RC's per class  */

  /* Initialization */                                                          /* --------------------------------- */
  nNTrans=CData_GetNRecs(itLM->td);                                             /* Get number of transitions         */
  nCTIS=CData_FindComp(itLM->td,NC_TD_TIS);                                     /* Get comp. index of input symbol   */
  for(nNWord=nT=0;nT<nNTrans;nT++){                                             /* Loop over all transitions >>      */
    nW=(INT32)CData_Dfetch(itLM->td,nT,nCTIS);                                   /*   Get input word index            */
    if(nW>nNWord) nNWord=nW;                                                    /*   Find maximum word index         */
  }                                                                             /* <<                                */
  nNWord++;                                                                     /* Number of words is max. idx. + 1  */
  lpUnigram=(INT32 *)dlp_calloc(nNWord,sizeof(INT32));                            /* Memory for unigram RC's per word  */
  lpBigram=(INT32 *)dlp_calloc(nNWord*nNWord,sizeof(INT32));                      /* Memory for bigram RC's per word   */
  lpClass=(INT32 *)dlp_malloc(nNWord*sizeof(INT32));                              /* Memory for class map              */
  lpCUnigram=(INT32 *)dlp_calloc(nNClass,sizeof(INT32));                          /* Memory for unigram RC's per class */
  lpCBigram=(INT32 *)dlp_calloc(nNClass*nNClass,sizeof(INT32));                   /* Memory for bigram RC's per class  */

  /* Generate startup class map */                                              /* --------------------------------- */
  if(lcc_lm2ngram(_this,itLM,nNWord,lpUnigram,lpBigram))                        /* Get ngram RC's per word from LM   */
    return IERROR(itLM,ERR_NOTSUPPORTED,"",0,0);                                /*   Error if something is wrong     */
  lcc_initclass(nNWord,lpUnigram,lpBigram,nNClass,lpClass,lpCUnigram,lpCBigram);/* Generate startup classes          */

  /* Class clustering of loop iterations */                                     /* --------------------------------- */
  for(nL=0;nL<nNLoops;nL++) if(!lcc_iter(nL,nNWord,lpUnigram,lpBigram,          /* For all loops do one clustering   */
    nNClass,lpClass,lpCUnigram,lpCBigram)) break;                               /* | iteration over classes          */
                                                                                /* | break if no words were moved    */

  /* Create class map object */                                                 /* --------------------------------- */
  lcc_ngram2classmap(nNWord,nNClass,lpClass,lpUnigram,idClassMap);              /* Create class map object           */

  /* Finialization */                                                           /* --------------------------------- */
  dlp_free(lpUnigram);                                                          /* Free memory                       */
  dlp_free(lpBigram);                                                           /* Free memory                       */
  dlp_free(lpClass);                                                            /* Free memory                       */
  dlp_free(lpCUnigram);                                                         /* Free memory                       */
  dlp_free(lpCBigram);                                                          /* Free memory                       */

  return O_K;                                                                   /* All done                          */
}

/* EOF */

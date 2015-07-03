/* dLabPro class CFst (fst)
 * - Best-N
 *
 * AUTHOR : Maximiliano Cuevas, Matthias Wolff
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
#include "dlp_math.h"

/* TODO: Move to fst.def, HEADERCODE: --> */
typedef struct tag_FST_PQU_TYPE
{
  FST_ITYPE nXini;
  FST_ITYPE nXter;
  FST_ITYPE nYini;
  FST_ITYPE nYter;
  FST_ITYPE nTran;
  FST_ITYPE nPlen;
  FST_WTYPE nCacc;
  FST_WTYPE nCtot;
} FST_PQU_TYPE;
/* <-- */

void CGEN_PRIVATE CFst_Cps_HashPrint(CFst* _this)
{
  hscan_t  hs;
  hnode_t* hn;
  FST_ITYPE nS;

  printf("\n -- CFst_Cps_HashPrint: %ld entries --",(long)hash_count((hash_t*)_this->m_lpCpsHash));
  hash_scan_begin(&hs,(hash_t*)_this->m_lpCpsHash);
  while ((hn = hash_scan_next(&hs))!=NULL)
  {
  	nS = (FST_ITYPE)((int)((long)hnode_get(hn)&0xffffffff));
  	printf("\n   %ld",nS);
  }
  printf("\n -------------------------------------");

}

/**
 * <p>Restores the source state indices and the epsilon filter mode from a hash
 * key.</p>
 * <p>The composition algorithm maintains a hash map <i>M</i>:
 * (<code>nSX</code>, <code>nSY</code>, <code>nFlagXY</code>) &rarr;
 * <code>nS</code> which associates a state index <code>nS</code> in the
 * composed transducer with the respective state indices in the left operand
 * (<code>nSX</code>) and in the right operand (<code>nSY</code>) and with an
 * epsilon transition filter mode <code>nFlagXY</code>.</p>
 * @param _this Pointer to composed transducer instance
 * @param lpKey    The hash key to be resolved
 * @param lpSX     Pointer to a buffer to be filled with state index in left operand
 * @param lpSY     Pointer to a buffer to be filled with state index in left operand
 * @param lpFlagXY Pointer to a buffer to be filled epsilon transition filter mode
 * @see Compose CFst_Compose
 */
void CGEN_PRIVATE CFst_Cps_HashResolveKey
(
  CFst*       _this,
  const void* lpKey,
  FST_ITYPE*  lpSX,
  FST_ITYPE*  lpSY,
  BYTE*       lpFlagXY
)
{
  FST_ITYPE nS = (FST_ITYPE)((int)((long)lpKey & 0xffffffff));

  if (nS>=0)
  {
    /* TODO: replace by SD_SX, SD_SY and SD_FLAGYX macros */
    *lpSX     = *(FST_ITYPE*)CData_XAddr(AS(CData,_this->sd),(INT32)nS,_this->m_nIcSdAux  );
    *lpSY     = *(FST_ITYPE*)CData_XAddr(AS(CData,_this->sd),(INT32)nS,_this->m_nIcSdAux+1);
    *lpFlagXY = *(BYTE*     )CData_XAddr(AS(CData,_this->sd),(INT32)nS,_this->m_nIcSdAux+2);
  }
  else
  {
    *lpSX     = _this->m_lpCpsKeybuf[0];
    *lpSY     = _this->m_lpCpsKeybuf[1];
    *lpFlagXY = (BYTE)_this->m_lpCpsKeybuf[2];
  }
}

/**
 * Creates a temporary hash key (for a hash lookup).
 */
void* CGEN_PRIVATE CFst_Cps_HashMakeTmpKey
(
  CFst* _this,
  FST_ITYPE nSX,
  FST_ITYPE nSY,
  FST_ITYPE nFlagXY
)
{
  _this->m_lpCpsKeybuf[0] = nSX;
  _this->m_lpCpsKeybuf[1] = nSY;
  _this->m_lpCpsKeybuf[2] = nFlagXY;
  return (void*)(-1);
}

/**
 * Hash function for composed state hash map.
 */
hash_val_t CGEN_SPRIVATE CFst_Cps_HashFn(const void* lpKey, void* lpContext)
{
  FST_ITYPE  nSX     = 0;
  FST_ITYPE  nSY     = 0;
  BYTE       nFlagXY = 0;
  INT32        i       = 0;
  hash_val_t nMask   = 0;
  hash_val_t nHash   = 0x00000000;

  CFst_Cps_HashResolveKey((CFst*)lpContext,lpKey,&nSX,&nSY,&nFlagXY);

/* MSB hash function
  for (i=0; i<15; i++)
  {
    if (nSX&(1<<i)) nHash |= 1<<(3+2*(14-i));
    if (nSY&(1<<i)) nHash |= 1<<(2+2*(14-i));
  }
  nHash |= (nFlagXY & 0x03);
*/

  /* Interlace least significant bits of nSX and nSY */
/*  for (i=0,nMask=1; i<15; i++,nMask<<=1)
  {
    if (nSX&nMask) nHash |= 1<<(2*i  );
    if (nSY&nMask) nHash |= 1<<(2*i+1);
  }
  nHash |= ((nFlagXY & 0x03) << 30);*/

  /* Use xor of 30 least significant bits of nSX and bit-reversed nSY */
  for(i=0,nMask=1;i<30;i++,nMask<<=1){
    if(nSX&nMask) nHash ^= nMask;
    if(nSY&nMask) nHash ^= 1<<(29-i);
  }
  nHash ^= (nFlagXY&0x03) << 30;

  return nHash;
}

/**
 * Comparison function for composed state hash map.
 */
int CGEN_SPRIVATE CFst_Cps_HashCmp(const void* lpKey1, const void* lpKey2, void* lpContext)
{
  FST_ITYPE  nSX1     = 0;
  FST_ITYPE  nSX2     = 0;
  FST_ITYPE  nSY1     = 0;
  FST_ITYPE  nSY2     = 0;
  BYTE       nFlagXY1 = 0;
  BYTE       nFlagXY2 = 0;
  int        nResult  = 0;

  if (lpKey1==lpKey2) return 0;

  CFst_Cps_HashResolveKey((CFst*)lpContext,lpKey1,&nSX1,&nSY1,&nFlagXY1);
  CFst_Cps_HashResolveKey((CFst*)lpContext,lpKey2,&nSX2,&nSY2,&nFlagXY2);

  if      (nSX1           <nSX2           ) nResult = -1;
  else if (nSX1           >nSX2           ) nResult =  1;
  else if (nSY1           <nSY2           ) nResult = -1;
  else if (nSY1           >nSY2           ) nResult =  1;
  else if ((nFlagXY1&0x03)<(nFlagXY2&0x03)) nResult = -1;
  else if ((nFlagXY1&0x03)>(nFlagXY2&0x03)) nResult =  1;
  else                                      nResult =  0;

  if (nResult!=0 /*&& ((CFst*)lpHint)->m_nCheck>0*/)
    printf("\n     HASH COLLISION (%ld,%ld,%d)=0x%08lX <--> (%ld,%ld,%d)=0x%08lX !",
      (long)nSX1,(long)nSY1,(int)nFlagXY1,(unsigned int)CFst_Cps_HashFn(lpKey1,lpContext),
      (long)nSX2,(long)nSY2,(int)nFlagXY2,(unsigned int)CFst_Cps_HashFn(lpKey2,lpContext));

  return nResult;
}

/**
 * Composed state hash map node allocation function. Hash nodes are taken from
 * a pool: {@link grany m_nGrany} nodes are allocated at a time to save memory
 * allocations.
 */
hnode_t* CFst_Cps_HashAllocNode(void* lpContext)
{
  CFst*    _this = NULL;
  INT32    nPool = 0;
  INT32    nItem = 0;
  hnode_t* lpRet = NULL;

  /* Get this pointer and locate next hash node */
  _this = (CFst*)lpContext;
  nPool = _this->m_nCpsHnpoolSize / _this->m_nGrany;
  nItem = _this->m_nCpsHnpoolSize - nPool*_this->m_nGrany;

  /* Grow size of hash node pool table if necessary */
  if (nPool>=(INT32)(dlp_size(_this->m_lpCpsHnpool)/sizeof(hnode_t*)))
    _this->m_lpCpsHnpool =
      (void**)dlp_realloc(_this->m_lpCpsHnpool,nPool+100,sizeof(hnode_t*));

  /* Allocate a new hash node pool if necessary */
  if (_this->m_lpCpsHnpool[nPool]==NULL)
    ((hnode_t**)_this->m_lpCpsHnpool)[nPool] =
    	(hnode_t*)dlp_calloc(_this->m_nGrany,sizeof(hnode_t));

  /* Increment pool size and return pointer to new hash node */
  _this->m_nCpsHnpoolSize++;
  lpRet = &((hnode_t**)_this->m_lpCpsHnpool)[nPool][nItem];
  return lpRet;
}

/**
 * Composed state hash map node deallocation function.
 */
void CFst_Cps_HashFreeNode(hnode_t* lpNode, void* lpContext)
{
  /* Nothing to be done; nodes will be freed all together by CFst_Cps_DelSdAux */
}

/**
 * Adds three auxiliary components to the state table of this instance. These
 * components are used by {@link -compose CFst_Compose} and
 * {@link -best_n CFst_BestN} to associate a composed state with its source
 * states and an epsilon filter mode. The method sets the field
 * {@link ic_sd_aux m_nIcSdAux} to the first component added.
 *
 * @param _this Pointer to automaton instance
 * @see Cps_DelSdAux CFst_Cps_DelSdAux
 * @see Cps_SetSdAux CFst_Cps_SetSdAux
 * @see Cps_FindState CFst_Cps_FindState
 */
void CGEN_PRIVATE CFst_Cps_AddSdAux(CFst* _this)
{
  /* Add auxiliary components to state table */
  CData_AddComp(AS(CData,_this->sd),"X"  ,DLP_TYPE(FST_ITYPE));
  CData_AddComp(AS(CData,_this->sd),"Y"  ,DLP_TYPE(FST_ITYPE));
  CData_AddComp(AS(CData,_this->sd),"Flg",T_BYTE             );

  DLPASSERT((_this->m_nIcSdAux = CData_FindComp(AS(CData,_this->sd),"X"  ))>=0);
  DLPASSERT(                     CData_FindComp(AS(CData,_this->sd),"Y"  ) >=0);
  DLPASSERT(                     CData_FindComp(AS(CData,_this->sd),"Flg") >=0);

  /* Initialize hash node pool */
  DLPASSERT(_this->m_lpCpsHnpool   ==NULL);
  DLPASSERT(_this->m_nCpsHnpoolSize==0   );
  _this->m_lpCpsHnpool    = (void**)dlp_calloc(100,sizeof(hnode_t*));
  _this->m_nCpsHnpoolSize = 0;
  dlp_memset(_this->m_lpCpsKeybuf,0,3*sizeof(FST_ITYPE));

  /* Initialize composed state hash map */
  DLPASSERT(_this->m_lpCpsHash==NULL);
  _this->m_lpCpsHash = hash_create(HASHCOUNT_T_MAX,CFst_Cps_HashCmp,CFst_Cps_HashFn,_this);
  hash_set_allocator((hash_t*)_this->m_lpCpsHash,CFst_Cps_HashAllocNode,CFst_Cps_HashFreeNode,_this);
}

/**
 * Deletes auxiliary components created by CFst_Cps_AddSdAux from the state table.
 *
 * @param _this Pointer to automaton instance
 * @see Cps_AddSdAux CFst_Cps_AddSdAux
 * @see Cps_SetSdAux CFst_Cps_SetSdAux
 * @see Cps_FindState CFst_Cps_FindState
 */
void CGEN_PRIVATE CFst_Cps_DelSdAux(CFst* _this)
{
  INT32 i = 0;

  /* Destroy composed state hash map */
  /* NOTE: MUST be done before deleting auxiliary state components! */
  hash_free_nodes((hash_t*)_this->m_lpCpsHash);
  hash_destroy((hash_t*)_this->m_lpCpsHash);
  _this->m_lpCpsHash = NULL;

  /* Destroy hash node pool */
  if (_this->m_lpCpsHnpool)
    for (i=0; i<(INT32)(dlp_size(_this->m_lpCpsHnpool)/sizeof(hnode_t*)); i++)
      dlp_free(_this->m_lpCpsHnpool[i]);
  dlp_free(_this->m_lpCpsHnpool);
  _this->m_lpCpsHnpool    = NULL;
  _this->m_nCpsHnpoolSize = 0;

  /* Delete auxiliary components from state table */
  CData_DeleteComps(AS(CData,_this->sd),_this->m_nIcSdAux,3);
  _this->m_nIcSdAux = -1;
}

/**
 * Associates the composed state <code>nS</code> with its source states
 * <code>nSX</code> (left operand) and <code>nSY</code> (right operand) and
 * with its epsilon filter mode <code>nFlagXY</code>.
 *
 * @param _this   Pointer to composed (destination) automaton instance
 * @param nS      Unit relative composed (destination) state index
 * @param nSX     Unit relative source state index in left operand (<code>itSrc1</code>)
 * @param nSY     Unit relative source state index in right operand (<code>itSrc2</code>)
 * @param nFlagXY Epsilon filter mode, one of the <code>CPS_EFLT_XXX</code> constants
 * @see Cps_FindState CFst_Cps_FindState
 * @see Cps_AddSdAux CFst_Cps_AddSdAux
 * @see Cps_DelSdAux CFst_Cps_DelSdAux
 */
void CGEN_PRIVATE CFst_Cps_SetSdAux
(
  CFst*     _this,
  FST_ITYPE nS,
  FST_ITYPE nSX,
  FST_ITYPE nSY,
  BYTE      nFlagXY
)
{
  IFCHECKEX(2)
    printf("\n       nS=%ld: Set [X=%ld, Y=%ld, Flg=%d]",(long)nS,(long)nSX,(long)nSY,(int)nFlagXY);

  /* Store state's auxiliary info */
  /* TODO: replace by SD_SX, SD_SY and SD_FLAGYX macros */
  *(FST_ITYPE*)(CData_XAddr(AS(CData,_this->sd),nS,_this->m_nIcSdAux  )) = nSX;
  *(FST_ITYPE*)(CData_XAddr(AS(CData,_this->sd),nS,_this->m_nIcSdAux+1)) = nSY;
  *(BYTE*     )(CData_XAddr(AS(CData,_this->sd),nS,_this->m_nIcSdAux+2)) = nFlagXY;

  /* Insert state into state hashmap for reverse lookup */
  hash_alloc_insert((hash_t*)_this->m_lpCpsHash,(void*)(long)nS,(void*)(long)nS);
}

/**
 * Returns the unit relative index of the composed state associated with the
 * source states <code>nSX</code> (left operand) and <code>nSY</code> (right
 * operand) and with the epsilon filter mode <code>nFlagXY</code> or -1 if no
 * such state exists.
 * @param _this   Pointer to composed (destination) automaton instance
 * @param nSX     Unit relative source state index in left operand (<code>itSrc1</code>)
 * @param nSY     Unit relative source state index in right operand (<code>itSrc2</code>)
 * @param nFlagXY Epsilon filter mode, one of the <code>CPS_EFLT_XXX</code> constants
 * @return        The unit relative state index or -1 if not found
 */
FST_ITYPE CGEN_PRIVATE CFst_Cps_FindState
(
  CFst*     _this,
  FST_ITYPE nSX,
  FST_ITYPE nSY,
  BYTE      nFlagXY
)
{
  FST_ITYPE nS    = 0;
  void*     lpKey = NULL;
  hnode_t*  lpHn  = NULL;

  lpKey = CFst_Cps_HashMakeTmpKey(_this,nSX,nSY,nFlagXY);
  lpHn  = hash_lookup((hash_t*)_this->m_lpCpsHash,lpKey);
  nS    = lpHn ? (FST_ITYPE)((int)((long)hnode_get(lpHn)&0xffffffff)) : -1;
  IFCHECKEX(2)
    printf("\n       Get [X=%ld, Y=%ld, Flg=%ld]: nS=%ld",(long)nSX,(long)nSY,(long)nFlagXY,(long)nS);

  /* HACK: Compare result of hash lookup with result of old algorithm --> * /
  {
    FST_ITYPE nSo = 0;
    FST_ITYPE nFS = 0;
    FST_ITYPE nXS = 0;

    nFS = 0;
    nXS = CData_GetNRecs(AS(CData,_this->sd));

    IFCHECKEX(2) printf("\n       Get [X=%ld, Y=%ld, Flg=%ld]:",(long)nSX,(long)nSY,(long)nFlagXY);

    for(nSo=nFS; nSo<nFS+nXS; nSo++)
      if (nSX == *(FST_ITYPE*)(CData_XAddr(AS(CData,_this->sd),nSo,_this->m_nIcSdAux)))
        if (nSY == *(FST_ITYPE*)(CData_XAddr(AS(CData,_this->sd),nSo,_this->m_nIcSdAux+1)))
          if (nFlagXY == *(BYTE*)(CData_XAddr(AS(CData,_this->sd),nSo,_this->m_nIcSdAux+2)))
          {
            IFCHECKEX(2) printf(" nS=%ld",(long)nSo);
            break;
          }

    if (nSo==nFS+nXS)
    {
      nSo=-1;
      IFCHECKEX(2) printf(" not found");
    }

    if (nS!=nSo)
      printf("\n  HASH MAP FAILURE looking up (%ld,%ld,%d); old=%ld <--> new=%ld",
        (long)nSX,(long)nSY,(int)nFlagXY,(long)nSo,(long)nS);
    DLPASSERT(nS==nSo);
  }
  / * <-- */

  return nS;
}

/**
 * Internally used by best-N-paths algorithm when sorting priority queue.
 * @see Bsn_CompDown CFst_Bsn_CompDown
 * @see BestNUnit CFst_BestNUnit
 */
int CGEN_SPRIVATE CFst_Bsn_CompUp(const void* lpElem1, const void* lpElem2)
{
  if (((FST_PQU_TYPE*)lpElem1)->nCtot < ((FST_PQU_TYPE*)lpElem2)->nCtot) return -1;
  if (((FST_PQU_TYPE*)lpElem1)->nCtot > ((FST_PQU_TYPE*)lpElem2)->nCtot) return  1;
  return 0;
}

/**
 * Internally used by best-N-paths algorithm when sorting priority queue.
 * @see Bsn_CompUp CFst_Bsn_CompUp
 * @see BestNUnit CFst_BestNUnit
 */
int CGEN_SPRIVATE CFst_Bsn_CompDown(const void* lpElem1, const void* lpElem2)
{
  if (((FST_PQU_TYPE*)lpElem1)->nCtot > ((FST_PQU_TYPE*)lpElem2)->nCtot) return -1;
  if (((FST_PQU_TYPE*)lpElem1)->nCtot < ((FST_PQU_TYPE*)lpElem2)->nCtot) return  1;
  return 0;
}

/**
 * Creates a tree with the best <code>nPaths</code> paths (minimum cost) taken from <code>itSrc</code>. There are no
 * checks performed.
 *
 * @param _this  Pointer to this (destination) automaton instance
 * @param itSrc  Pointer to source automaton instance
 * @param nUnit  Index of unit to process
 * @param nPaths Number of paths to be extracted
 * @param nPathlength Length of paths to be extracted
 * @return O_K if successfull, a (negative) error code otherwise
 * @see BestN CFst_BestN
 */
INT16 CGEN_PROTECTED CFst_BestNUnit(CFst* _this, CFst* itSrc, INT32 nUnit, INT32 nPaths, INT32 nPathlength)
{
  FST_PQU_TYPE* lpPQ      = NULL;                                              /* Priority queue data struct         */
  FST_TID_TYPE* lpTIX     = NULL;                                              /* Source graph iterator data struct  */
  BYTE*         lpTX      = NULL;                                              /* Ptr. to current source transition  */
  FST_ITYPE     nFS       = 0;                                                 /* First state of source unit         */
  INT32          nArrivals = 0;                                                 /* Paths having reached a final state */
  FST_WTYPE     nNeAdd    = 0.;                                                /* Neutral element of addition        */
  FST_WTYPE     nNeMult   = 0.;                                                /* Neutral element of multiplication  */
  FST_ITYPE     nPQsize   = 0;                                                 /* Priority queue size                */
  FST_ITYPE     nT        = 0;                                                 /* Index of current source transition */
  FST_ITYPE     nNewT     = 0;                                                 /* Index of newly inserted dst. trans.*/
  FST_ITYPE     nSini     = 0;                                                 /* Ini. state of curr. dest. trans.   */
  FST_ITYPE     nXini     = 0;                                                 /* Ini. state of curr. source trans.  */
  FST_ITYPE     nYini     = 0;                                                 /* Ini. state of curr. aux. trans.    */
  FST_ITYPE     nSter     = 0;                                                 /* Ter. state of curr. dest. trans.   */
  FST_ITYPE     nXter     = 0;                                                 /* Ter. state of curr. source trans.  */
  FST_ITYPE     nYter     = 0;                                                 /* Ter. state of curr. aux. trans.    */
  FST_ITYPE     nPlen     = 0;                                                 /* length of the path up to here      */
  FST_WTYPE     nWX       = 0.0;                                               /* Weight of current src. transition  */
  FST_WTYPE     nPot      = 0.0;                                               /* Potential of current src. state    */
  FST_WTYPE     nCacc     = 0.0;                                               /* Accumulated cost from start state  */
  FST_WTYPE     nCtot     = 0.0;                                               /* Total cost to final state          */
  FST_WTYPE     nCmax     = 0.0;                                               /* Maximal cost in priority queue     */
  INT32          nNewY     = 0;                                                 /* New auxiliary state                */
  INT32          nIcP      = 0;                                                 /* Comp. index of src. state potential*/
  INT16         nCheck    = 0;                                                 /* Copy buffer for verbose level      */
  BOOL          bPush     = FALSE;                                             /* Copy of m_bPush option             */
  INT32          k         = 0;                                                 /* Auxilary loop counter              */

  /* Validate */
  DLPASSERT(_this!=itSrc);
  DLPASSERT(nUnit>=0 && nUnit<UD_XXU(itSrc));

  /* Initialize destination */
  nCheck = BASEINST(_this)->m_nCheck;
  bPush  = _this->m_bPush;
  CFst_Reset(BASEINST(_this),TRUE);
  BASEINST(_this)->m_nCheck=nCheck;
  CData_Scopy(AS(CData,_this->sd),AS(CData,itSrc->sd));
  CData_Scopy(AS(CData,_this->td),AS(CData,itSrc->td));
  CData_SelectRecs(AS(CData,_this->ud),AS(CData,itSrc->ud),nUnit,1);
  UD_FS(_this,0)  = 0;
  UD_FT(_this,0)  = 0;
  UD_XS(_this,0)  = 0;
  UD_XT(_this,0)  = 0;
  _this->m_nGrany = itSrc->m_nGrany;
  _this->m_nWsr   = CFst_Wsr_GetType(itSrc,&_this->m_nIcW);
  BASEINST(_this)->m_nCheck = BASEINST(_this)->m_nCheck>BASEINST(itSrc)->m_nCheck?BASEINST(_this)->m_nCheck:BASEINST(itSrc)->m_nCheck;

  CFst_Cps_AddSdAux(_this);
  nNeMult = CFst_Wsr_NeMult(_this->m_nWsr);
  nNeAdd  = CFst_Wsr_NeAdd (_this->m_nWsr);

  /* Add in itSrc the component Extracted that stores how many times this node has been visited */
  nIcP = CData_FindComp(AS(CData,itSrc->sd),NC_SD_POT);
  DLPASSERT(nIcP         >=0);
  DLPASSERT(_this->m_nIcW>=0);

  /* Initialize - Create priority queue */
  lpPQ = (FST_PQU_TYPE*)dlp_calloc(nPaths+1,sizeof(FST_PQU_TYPE));
  for (k=0; k<nPaths+1; k++) lpPQ[k].nCtot = nNeAdd;

  /* Initialize - Graph iterator */
  lpTIX = CFst_STI_Init(itSrc,nUnit,FSTI_SORTINI);
  nFS   = UD_FS(itSrc,nUnit);

  /* Create initial state in _this */
  nXter = CFst_Addstates(_this,0,1,SD_FLG(itSrc,nFS)&0x01);
  CFst_Cps_SetSdAux(_this,nXter,0,0,0);

  /* Initialize local variables */
  nXter = 0;
  nYter = 0;
  nCacc = nNeMult;

  /* Best-N computation */
  for(;;)
  {
    /* For all transitions leaving nXter */
    lpTX = NULL;
    while ((lpTX=CFst_STI_TfromS(lpTIX,nXter,lpTX))!=NULL)
    {
      /* Compute nCtot following the current transition */
      nT    = CFst_STI_GetTransId(lpTIX,lpTX);
      nWX   = *CFst_STI_TW(lpTIX,lpTX);
      nPot  = *(FST_WTYPE*)(CData_XAddr(AS(CData,itSrc->sd),TD_TER(itSrc,nT)+lpTIX->nFS,nIcP));
      nCtot = CFst_Wsr_Op(_this,nCacc,CFst_Wsr_Op(_this,nPot,nWX,OP_MULT),OP_MULT); /* nCacc + nWX + nPot; */
      IFCHECKEX(1)
        printf("\n state: %d \t Cacc: %f\t W: %f \t Pot: %f \t Ctot: %f\t\n",
          (int)nXter,(float)nCacc,(float)nWX,(float)nPot,(float)nCtot);

      /* Insert into priority queue? */
      if
      (
        nPQsize < (nPaths - nArrivals)              ||
        (
          nPQsize==(nPaths - nArrivals)             &&
          CFst_Wsr_Op(_this,nCtot,nCmax,OP_GREATER)
        )
      )
      {
        nNewY++;

        /* Push into priority queue */
        lpPQ[nPQsize].nXini = nXter;
        lpPQ[nPQsize].nYini = nYter;
        lpPQ[nPQsize].nXter = *CFst_STI_TTer(lpTIX,lpTX);
        lpPQ[nPQsize].nYter = nNewY;
        lpPQ[nPQsize].nTran = nT;
        lpPQ[nPQsize].nPlen = nPlen+1;
        lpPQ[nPQsize].nCacc = CFst_Wsr_Op(_this,nCacc,nWX,OP_MULT); /* nWX + nCacc; */
        lpPQ[nPQsize].nCtot = nCtot;
        nPQsize++;

        IFCHECKEX(1)
          printf("\n PriQsize:%d PUSH: [X:%d Y:%d] --- Cacc:%f Tran:%d Plen:%d Ctot:%f ----> [X:%d Y:%d]\n",
            (int)nPQsize,(int)nXter,(int)nYter,(float)nCacc,(int)nT,(int)nPlen,
            (float)nCtot,(int)*CFst_STI_TTer(lpTIX,lpTX),(int)nNewY);

        /* Sort priority queue */
        if (_this->m_nWsr == FST_WSR_PROB)
          qsort(lpPQ,nPaths+1,sizeof(FST_PQU_TYPE),CFst_Bsn_CompDown);
        else
          qsort(lpPQ,nPaths+1,sizeof(FST_PQU_TYPE),CFst_Bsn_CompUp);

        /* Delete last element from queue (if neccesary) to keep its size smaller than nPaths+1 */
        if (nPQsize == nPaths+1-nArrivals)
        {
          lpPQ[nPQsize-1].nCtot = nNeAdd;
          nPQsize--;
        }
        nCmax = lpPQ[nPQsize-1].nCtot;

        IFCHECKEX(2)
          for (k=0; k<nPQsize; k++)
            printf("Q %d : [X:%d Y:%d] ---  Tran:%d  ----> [X:%d Y:%d] Plen:%d Cacc:%f ",
              (int)k,(int)lpPQ[k].nXini,(int)lpPQ[k].nYini,(int)lpPQ[k].nTran,
              (int)lpPQ[k].nXter,(int)lpPQ[k].nYter,(int)lpPQ[k].nPlen,(float)lpPQ[k].nCacc),
          printf("Ctot:%f \n",lpPQ[k].nCtot);
      }
    }

    /* Write transition and state in _this */
    DLPASSERT(nPQsize>=0);
    if (nPQsize>0)
    {
      /* Pop first record data from priority queue */
      nXini = lpPQ[0].nXini;
      nYini = lpPQ[0].nYini;
      nXter = lpPQ[0].nXter;
      nYter = lpPQ[0].nYter;
      nT    = lpPQ[0].nTran;
      nPlen = lpPQ[0].nPlen;
      nCacc = lpPQ[0].nCacc;
      nCtot = lpPQ[0].nCtot;

      dlp_memmove(lpPQ,&lpPQ[1],nPaths*sizeof(FST_PQU_TYPE));
      lpPQ[nPQsize-1].nCtot = nNeAdd;
      nPQsize--;
      IFCHECKEX(1) printf(" Pop priority queue\n");
      IFCHECKEX(2)
        for (k=0; k<nPQsize; k++)
          printf("Q %d : [X:%d Y:%d] ---  Tran:%d  ----> [X:%d Y:%d] Plen:%d Cacc:%f ",
            (int)k,(int)lpPQ[k].nXini,(int)lpPQ[k].nYini,(int)lpPQ[k].nTran,
            (int)lpPQ[k].nXter,(int)lpPQ[k].nYter,(int)lpPQ[k].nPlen,(float)lpPQ[k].nCacc),
          printf("Ctot:%f \n",(float)lpPQ[k].nCtot);

      /* Add state and trans in _this */
      nSini = CFst_Cps_FindState(_this,nXini,nYini,0);
      nSter = CFst_Addstates(_this,0,1,(nPathlength<=0||nPlen>=nPathlength)?(SD_FLG(itSrc,nXter+nFS)&0x01):0); /* FS-Flag ist only used if Path is long enough */
      CFst_Cps_SetSdAux(_this,nSter,nXter,nYter,0);

      /* If the path in itSrc is finished and path length exceeded increment nArrivals */
      if ((SD_FLG(itSrc,nXter+nFS)&0x01) && (nPathlength<=0 || nPlen>=nPathlength))
      {
        nArrivals++;
        nWX = nCtot;
      }
      else nWX = nNeMult;

      nNewT = CFst_AddtransCopy(_this,0,nSini,nSter,itSrc,nT);
      if (bPush) *(FST_WTYPE*)CData_XAddr(AS(CData,_this->td),nNewT,_this->m_nIcW) = nWX;

      IFCHECKEX(1)
        printf("\n AddTrans: [X:%d Y:%d] --- Cacc:%f Tran:%d Plen:%d Ctot:%f ----> [X:%d Y:%d]\n",
          (int)nXini,(int)nYini,(float)nCacc,(int)nT,(int)nPlen,(float)nCtot,(int)nXter,(int)nYter);
    }
    else break;
  }

  /* Clean up */
  CFst_Cps_DelSdAux(_this);
  CFst_STI_Done(lpTIX);
  dlp_free(lpPQ);
  return O_K;
}

/*
 * Manual page at fst_man.def
 */
INT16 CGEN_PUBLIC CFst_BestN(CFst* _this, CFst* itSrc, INT32 nUnit, INT32 nPaths, INT32 nPathlength)
{
  CFst* itUnit = NULL;                                                         /* Current unit                      */
  INT32  nU     = 0;                                                            /* Current unit index                */
  INT32  nIcW   = -1;                                                           /* Comp. index of src.trans. weights */
  INT32  nIcP   = -1;                                                           /* Comp. index of src.state potential*/
  BOOL  bPush  = FALSE;                                                        /* Copy of m_bPush option            */
  BOOL  bLocal = FALSE;                                                        /* Copy of m_bLocal option           */

  /* Validation */
  CHECK_THIS_RV(NOT_EXEC);
  CFst_Check(_this);
  CFst_Check(itSrc);
  if (nUnit>=UD_XXU(itSrc)) return IERROR(_this,FST_BADID,"unit",nUnit,0);

  /* BestN needs transitions weights and state potentials */
  CFst_Wsr_GetType(itSrc,&nIcW);
  if (nIcW<0) return IERROR(_this,FST_MISS,"weight component","","transition table");
  nIcP = CData_FindComp(AS(CData,itSrc->sd),NC_SD_POT);
  CFst_Potential(itSrc,nUnit);
  DLPASSERT(CData_FindComp(AS(CData,itSrc->sd),NC_SD_POT)>=0);                 /* CFst_Potential failed?             */

  bPush  = _this->m_bPush;
  bLocal = _this->m_bLocal;

  /* NO RETURNS BEYOND THIS POINT! */
  CREATEVIRTUAL(CFst,itSrc,_this);
  CFst_Reset(BASEINST(_this),TRUE);
  if (nUnit<0) { ICREATEEX(CFst,itUnit,"CFst_BestN~itUnit",NULL); }
  else         itUnit = _this;

  if (BASEINST(_this)->m_nCheck>BASEINST(itUnit)->m_nCheck) BASEINST(itUnit)->m_nCheck=BASEINST(_this)->m_nCheck;
  if (BASEINST(itSrc)->m_nCheck>BASEINST(itUnit)->m_nCheck) BASEINST(itUnit)->m_nCheck=BASEINST(itSrc)->m_nCheck;

  /* BestN of units */
  for (nU=nUnit<0?0:nUnit; nU<UD_XXU(itSrc); nU++)
  {
    itUnit->m_bPush = bPush;
    if (bLocal) CFst_BestNUnitLocal(itUnit,itSrc,nU,nPaths);
    else        CFst_BestNUnit     (itUnit,itSrc,nU,nPaths,nPathlength);
    if (nUnit>=0) break;
    CFst_Cat(_this,itUnit);
  }

  /* Copy input and output symbol table */                                      /* --------------------------------- */
  CData_Copy(_this->is,itSrc->is);                                              /* Copy input symbol table           */
  CData_Copy(_this->os,itSrc->os);                                              /* Copy output symbol table          */

  /* Clean up */
  if (nUnit<0) IDESTROY(itUnit);
  DESTROYVIRTUAL(itSrc,_this);
  if (nIcP<0) CData_DeleteComps(AS(CData,itSrc->sd),nIcP,1);
  CData_DeleteComps(AS(CData,_this->sd),CData_FindComp(AS(CData,_this->sd),NC_SD_POT),1);
  if(nPathlength>0) CFst_Trim(_this,-1,0);                                  /* remove short paths without endstate */
  CFst_Check(_this);                                                           /* TODO: Remove after debugging      */
  return O_K;
}

/* EOF */

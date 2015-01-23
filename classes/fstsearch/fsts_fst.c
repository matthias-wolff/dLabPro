/* dLabPro class CFstsearch (fstsearch)
 * - Internal transducers
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

#include <math.h>

#include "fsts_glob.h"

/* Check source transducer function
 *
 * This function checks the source transducer if it is usable for decoding.
 *
 * @param itSrc  Source transducer
 * @return <code>NULL</code> if successfull, the error string otherwise
 */
const char *fsts_check(CFst *itSrc,INT32 uid){
  INT32 i,ctis,ctos,cstk,stk;
  if(UD_XXU(itSrc)<=0) return FSTSERR("no units in itSrc");
  if(uid>=UD_XXU(itSrc)) return FSTSERR("selected source unit does not exists in itSrc");
  if(UD_XXU(itSrc)>2147483646) return FSTSERR("more than 2^31-2 units");
  if(CData_GetNRecs(AS(CData,itSrc->td))>2147483646) return FSTSERR("more than 2^31-2 transitions");
  for(i=0;i<UD_XXU(itSrc);i++) if(UD_XS(itSrc,i)>2147483646) return FSTSERR("more than 2^31-2 states in one of the units");
  ctis=CData_FindComp(AS(CData,itSrc->td),NC_TD_TIS);
  ctos=CData_FindComp(AS(CData,itSrc->td),NC_TD_TOS);
  cstk=CData_FindComp(AS(CData,itSrc->td),"~STK");
  for(i=0;i<CData_GetNRecs(AS(CData,itSrc->td));i++){
    if(ctis>=0 && *(FST_STYPE*)CData_XAddr(AS(CData,itSrc->td),i,ctis)>2147483646) return FSTSERR("input symbol index larger than 2^31-2");
    if(ctos>=0 && *(FST_STYPE*)CData_XAddr(AS(CData,itSrc->td),i,ctos)>2147483646) return FSTSERR("output symbol index larger than 2^31-2");
    if(cstk>=0){
      stk=*(FST_STYPE*)CData_XAddr(AS(CData,itSrc->td),i,cstk);
      if(stk<-65535 || stk>65535) return FSTSERR("pushdown symbol out of -65535..65535");
    }
  }
  return NULL;
}

/* Load source transducer function
 *
 * This function converts the source transducer into the
 * internal structure.
 *
 * @param src    Pointer to the internal transducer structure
 * @param itSrc  Source transducer
 * @param uid    Unit to use or negative for on-the-fly composition
 * @param fast   If TRUE the source transducer may get unsable in operation
 * @return <code>NULL</code> if successfull, the error string otherwise
 */
const char *fsts_load(struct fsts_fst *src,CFst *itSrc,INT32 uid,UINT8 fast){
  INT32 ui;
  INT32 csub;
  INT32 cstk;
  UINT8 psr;
  const char *err;
  if((err=fsts_check(itSrc,uid))) return err;
  src->nunits = uid<0 ? UD_XXU(itSrc) : 1;
  csub = uid<0 ? CData_FindComp(AS(CData,itSrc->ud),"~SUB") : -1;
  psr  = CFst_Wsr_GetType(itSrc,NULL)==FST_WSR_PROB;
  cstk = CData_FindComp(AS(CData,itSrc->td),"~STK");
  src->stk=0;
  if(!(src->units=(struct fsts_unit *)calloc(src->nunits,sizeof(struct fsts_unit)))) return FSTSERR("out of memory");
  for(ui=uid<0?0:uid ; uid<0 ? ui<src->nunits : ui==uid ; ui++){
    struct fsts_unit *u=src->units+(uid<0?ui:0);
    UINT32 si;
    FST_TID_TYPE *lpTI;
    u->pot0=0.;
    if(!(lpTI=CFst_STI_Init(itSrc,ui,FSTI_PTR))) return FSTSERR("fst iterator creation failed");
    u->sub = csub<0 ? 0 : CData_Dfetch(AS(CData,itSrc->ud),ui,csub);
    u->ns=UD_XS(itSrc,ui);
    if(!(u->tfroms=(struct fsts_t **)calloc(u->ns,sizeof(struct fsts_t *)))) return FSTSERR("out of memory");
    if(!(u->ttos  =(struct fsts_t **)calloc(u->ns,sizeof(struct fsts_t *)))) return FSTSERR("out of memory");
    if(!(u->sfin  =(UINT8 *         )calloc(u->ns,sizeof(UINT8          )))) return FSTSERR("out of memory");
    for(si=0;si<u->ns;si++){
      BYTE *lpT=NULL;
      while((lpT=CFst_STI_TfromS(lpTI,si,lpT))){
        struct fsts_t *t;
        if(!(t=(struct fsts_t *)malloc(sizeof(struct fsts_t)))) return FSTSERR("out of memory";) 
        t->id =CFst_STI_GetTransId(lpTI,lpT);
        t->ini=*CFst_STI_TIni(lpTI,lpT);
        t->ter=*CFst_STI_TTer(lpTI,lpT);
        t->is =lpTI->nOfTTis>0 ? *CFst_STI_TTis(lpTI,lpT) : -1;
        t->os =lpTI->nOfTTos>0 ? *CFst_STI_TTos(lpTI,lpT) : -1;
        t->stk=cstk>0 ? CData_Dfetch(AS(CData,itSrc->td),t->id,cstk) : 0;
        if(t->stk) src->stk=1;
        t->w  =lpTI->nOfTW>0   ? *CFst_STI_TW(lpTI,lpT)   :  0;
        if(psr) t->w=-1.*log(t->w);
        t->nxt=u->tfroms[t->ini]; u->tfroms[t->ini]=t;
        t->prv=u->ttos[t->ter];   u->ttos[t->ter]=t;
      }
      u->sfin[si]=SD_FLG(itSrc,si+lpTI->nFS)&SD_FLG_FINAL;
    }
    CFst_STI_Done(lpTI);
  }
  if(!fast){
    ICREATEEX(CFst,src->itSrc,"SRC",NULL);
    CDlpObject_Copy(BASEINST(src->itSrc),BASEINST(itSrc));
  }
  return NULL;
}

/* Unload source transducer function
 *
 * This function unloads the internal source transducer
 * and frees the memory.
 *
 * @param src    Pointer to the internal transducer structure
 * @return <code>NULL</code> if successfull, the error string otherwise
 */
const char *fsts_unload(struct fsts_fst *src){
  if(src->itSrc) IDESTROY(src->itSrc);
  if(src->units){
    INT32 ui;
    for(ui=0;ui<src->nunits;ui++){
      struct fsts_unit *u=src->units+ui;
      UINT32 si;
      for(si=0;si<u->ns;si++) while(u->tfroms[si]){
        struct fsts_t *t=u->tfroms[si];
        u->tfroms[si]=t->nxt;
        free(t);
      }
      free(u->tfroms);
      free(u->ttos);
      free(u->sfin);
    }
    free(src->units);
  }
  return NULL;
}

/* Transducer push weights function
 *
 * This function pushes all transition weights
 * of all units towards initial states by calculation
 * the potential of each state and adding the potential
 * to all ingoing transition and subtracting it
 * from all outgoing ones. It results in a
 * weight offset for all paths which is stored
 * in pot0 of each unit.
 *
 * @param src  The transducer
 * @return <code>NULL</code> if successfull, the error string otherwise
 */
const char *fsts_fstpw(struct fsts_fst *src){
  struct fsts_unit *u=src->units;
  INT32 ui;
  for(ui=0;ui<src->nunits;ui++,u++) if(!u->pot0){
    struct fsts_pot {
      struct fsts_pot *nxt;
      FLOAT64 w;
    } *pot=(struct fsts_pot*)calloc(u->ns,sizeof(struct fsts_pot)), *qs=NULL, *qe=NULL;
    UINT32 si;
    if(!pot) return FSTSERR("out of memory");
    for(si=0;si<u->ns;si++) if(u->sfin[si]){
      pot[si].w=0.;
      if(!qe) qs=qe=pot+si;
      else{
        qe->nxt=pot+si;
        qe=pot+si;
      }
    }else pot[si].w=T_DOUBLE_MAX;
    while(qs){
      INT32 si=qs-pot;
      struct fsts_t *t=u->ttos[si];
      for(;t;t=t->prv) if(pot[t->ini].w>qs->w+t->w){
        pot[t->ini].w=qs->w+t->w;
        if(!pot[t->ini].nxt){
          qe->nxt=pot+t->ini;
          qe=pot+t->ini;
        }
      }
      qs=qs->nxt;
      pot[si].nxt=NULL;
    }
    for(si=0;si<u->ns;si++){
      struct fsts_t *t=u->tfroms[si];
      for(;t;t=t->nxt){
        t->w+=pot[t->ter].w;
        t->w-=pot[t->ini].w;
      }
    }
    u->pot0=pot[0].w;
    free(pot);
  }
  return NULL;
}


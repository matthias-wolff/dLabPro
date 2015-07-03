/* dLabPro class CFstsearch (fstsearch)
 * - A* state storage
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

#include "fsts_glob.h"
#include "fsts_as_algo.h"

/* A* state storage initialization function
 *
 * This function initializes the state storage.
 *
 * @param ls   The memory 
 * @param btm  Backtrack memory
 * @param cfg  Configuration
 * @return <code>NULL</code> if successfull, the error string otherwise
 */
const char *fsts_as_lsinit(struct fsts_as_ls *ls,struct fsts_btm *btm,struct fsts_cfg *cfg){
  const char *err;
  ls->numpaths=cfg->numpaths;
  ls->btm=btm;
  if((err=fsts_hinit(&ls->h,sizeof(struct fsts_as_s),cfg->numpaths,cfg->stkprn?NULL:fsts_as_scmpstk))) return err;
  if((err=fsts_as_qinit(&ls->q,cfg->as.qsize))) return err;
  return NULL;
}

/* A* state storage free function
 *
 * This function free's the state storage and all it's subordinated memories.
 *
 * @param ls  The queue
 */
void fsts_as_lsfree(struct fsts_as_ls *ls){
  fsts_hfree(&ls->h);
  fsts_as_qfree(&ls->q);
}

/* A* active state add function
 *
 * This function adds a new active state into the state storage
 *
 * @param ls  The memory 
 * @param s   The active state
 * @param dbg Debug level
 * @return <code>NULL</code> if successfull, the error string otherwise
 */
const char *fsts_as_lsadd(struct fsts_as_ls *ls,struct fsts_as_s *s,UINT8 dbg){
  struct fsts_hs *hs;
  const char *err;
  if((hs=fsts_hfind(&ls->h,s))) if(ls->numpaths==1){
    /* state visited before + only one path => replace state if weight improved */
    struct fsts_as_s *sh=(struct fsts_as_s*)fsts_hs2s(hs);
    #ifdef _DEBUG
    if(dbg>=4) printf(" => %s0 (w: %8.1f os: 0x%08x)",sh->w<=s->w?"del":"rpl",sh->w,sh->bt.os?BTHASH(sh->bt.os):0);
    #endif
    if(sh->w<=s->w) fsts_as_sfree(s,sh,ls->btm); else{
      #ifdef OPT_AVL
      struct fsts_as_qs *qp=sh->qs;
      #else
      UINT32 qp=sh->qp;
      #endif
      fsts_as_sfree(sh,s,ls->btm);
      *sh=*s;
      if((err=fsts_as_qrpl(&ls->q,sh,qp))) return err;
    }
  }else{
    /* state visited before + multiple paths */
    struct fsts_as_s *sh=(struct fsts_as_s*)fsts_hs2s(hs);
    UINT16 i;
    /* search for hypo with equal history */
    for(i=0;i<hs->use;i++,sh++) if(fsts_bteql(sh->bt.os,s->bt.os)){
      /* hypo with equal history => replace sate if weight improved */
      #ifdef _DEBUG
      if(dbg>=4) printf(" => %s%i (w: %8.1f os: 0x%08x)",sh->w<=s->w?"del":"rpl",i,sh->w,BTHASH(sh->bt.os));
      #endif
      if(sh->w<=s->w) fsts_as_sfree(s,sh,ls->btm); else{
        #ifdef OPT_AVL
        struct fsts_as_qs *qp=sh->qs;
        #else
        UINT32 qp=sh->qp;
        #endif
        fsts_as_sfree(sh,s,ls->btm);
        *sh=*s;                                         /* insert the state */
        if((err=fsts_as_qrpl(&ls->q,sh,qp))) return err;
      }
      break;
    }
    if(i==hs->use){
      /* no hypo with equal history */
      if(hs->use<ls->numpaths){
        #ifdef _DEBUG
        if(dbg>=4) printf(" => add%i",hs->use);
        #endif
        /* not full => add state */
        *sh=*s;
        hs->use++;
        if((err=fsts_as_qrpl(&ls->q,sh,0))) return err;
      }else{
        /* full => replace maximal weight if possible */
        struct fsts_as_s *sh=(struct fsts_as_s*)fsts_hs2s(hs);
        struct fsts_as_s *shmax=sh;
        for(i=1,sh++;i<hs->use;i++,sh++) if(sh->w>shmax->w) shmax=sh;
        #ifdef _DEBUG
        if(dbg>=4) printf(" => %sX (w: %8.1f os: 0x%08x)",shmax->w<=s->w?"del":"rpl",shmax->w,BTHASH(shmax->bt.os));
        #endif
        if(shmax->w<=s->w) fsts_as_sfree(s,shmax,ls->btm); else {
          #ifdef OPT_AVL
          struct fsts_as_qs *qp=shmax->qs;
          #else
          UINT32 qp=shmax->qp;
          #endif
          fsts_as_sfree(shmax,s,ls->btm);
          *shmax=*s;                                         /* insert the state */
          if((err=fsts_as_qrpl(&ls->q,shmax,qp))) return err;
        }
      }
    }
  }else{
    /* state not visited before => insert */
    #ifdef _DEBUG
    if(dbg>=4) printf(" => add0");
    #endif
    if(!(s=(struct fsts_as_s*)fsts_hins(&ls->h,s))) return FSTSERR("hmem: out of memory");
    if((err=fsts_as_qrpl(&ls->q,s,0))) return err;
  }
  return NULL;
}

void fsts_as_lsfins(struct fsts_as_ls *ls,struct fsts_as_s *s,UINT8 prn){
  struct fsts_hs *hs;
  if((hs=fsts_hfins(&ls->h,s,prn))){
    UINT16 i;
    for(i=0,s=(struct fsts_as_s*)fsts_hs2s(hs);i<hs->use;i++,s++){
      #ifdef OPT_AVL
      if(s->qs) fsts_as_qpop(&ls->q,s->qs);
      #else
      if(s->qp) fsts_as_qpop(&ls->q,s->qp);
      #endif
      fsts_as_sfree(s,NULL,ls->btm);
    }
    fsts_as_mapon(ls->map,hs->id);
  }
}

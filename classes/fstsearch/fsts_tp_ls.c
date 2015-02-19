/* dLabPro class CFstsearch (fstsearch)
 * - TP state storage
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
#include "fsts_tp_algo.h"

/* TP active state queue initialization function
 *
 * This function initializes an active state queue.
 *
 * @param ls       The queue
 * @param btm      Backtrack memory
 * @param cfg      Configuration
 * @param wmin     Minimal weight in last frame
 * @return <code>NULL</code> if successfull, the error string otherwise
 */
const char *fsts_tp_lsinit(struct fsts_tp_ls *ls,struct fsts_btm *btm,struct fsts_cfg *cfg,FLOAT64 wmin){
  if(btm) ls->btm=btm;
  if(cfg) ls->numpaths=cfg->numpaths;
  ls->qs=ls->qe=NULL;
  fsts_tp_histinit(&ls->hist,wmin);
  ls->wmax=0.;
  ls->wmin=T_DOUBLE_MAX;
  return fsts_hinit(&ls->h,sizeof(struct fsts_tp_s),cfg?cfg->numpaths:0,cfg && !cfg->stkprn?fsts_tp_scmpstk:NULL);
}

/* TP active state queue free function
 *
 * This function free's the active state queue and all it's subordinated memories.
 *
 * @param ls  The queue
 */
void fsts_tp_lsfree(struct fsts_tp_ls *ls){
  struct fsts_tp_s *s;
  if(ls->qs) while((s=fsts_tp_lsdel(ls))) fsts_tp_sfree(s,NULL,ls->btm);
  fsts_hfree(&ls->h);
}

/* This macro defines the deactivated next state pointer */
#define SNXT_NONE ((struct fsts_tp_s *)-1)
/* This macro inserts a new state into the connected list of the queue */
#define fsts_tp_lsins(ls,s) { \
  if((ls)->qe) (ls)->qe->nxt=(s); \
  else (ls)->qs=(s); \
  (ls)->qe=(s); \
  (s)->nxt=NULL; \
}

/* TP active state queue add function
 *
 * This function adds a new active state into the queue
 *
 * @param ls  The queue
 * @param s   The active state
 * @param dbg Debug level
 * @return <code>NULL</code> if successfull, the error string otherwise
 */
const char *fsts_tp_lsadd(struct fsts_tp_ls *ls,struct fsts_tp_s *s,UINT8 dbg){
  struct fsts_hs *hs;
  if(s->wn>ls->wmax) ls->wmax=s->wn;
  if(s->wn<ls->wmin) ls->wmin=s->wn;
  fsts_tp_histins(&ls->hist,s->wn);
  /* state visitied before ? */
  if((hs=fsts_hfind(&ls->h,s))) if(ls->numpaths==1){
    /* state visited before + only one path => replace state if weight improved */
    struct fsts_tp_s *sh=(struct fsts_tp_s*)fsts_hs2s(hs);
    #ifdef _DEBUG
    if(dbg>=4) printf(" => %s0 (wn: %8.1f os: 0x%08x)",sh->wn<=s->wn?"del":"rpl",sh->wn,sh->bt.os?BTHASH(sh->bt.os):0);
    #endif
    if(sh->wn<=s->wn) fsts_tp_sfree(s,sh,ls->btm); else{
      s->nxt=sh->nxt;
      fsts_tp_sfree(sh,s,ls->btm);
      *sh=*s;
      if(sh->nxt==SNXT_NONE) fsts_tp_lsins(ls,sh); /* insert in queue if not done */
    }
  }else{
    /* state visited before + multiple paths */
    struct fsts_tp_s *sh=(struct fsts_tp_s*)fsts_hs2s(hs);
    UINT16 i;
    /* search for hypo with equal history */
    for(i=0;i<hs->use;i++,sh++) if(fsts_bteql(sh->bt.os,s->bt.os)){
      /* hypo with equal history => replace sate if weight improved */
      #ifdef _DEBUG
      if(dbg>=4) printf(" => %s%i (wn: %8.1f os: 0x%08x)",sh->wn<=s->wn?"del":"rpl",i,sh->wn,BTHASH(sh->bt.os));
      #endif
      if(sh->wn<=s->wn) fsts_tp_sfree(s,sh,ls->btm); else{
        s->nxt=sh->nxt;
        fsts_tp_sfree(sh,s,ls->btm);
        *sh=*s;                                         /* insert the state */
        if(sh->nxt==SNXT_NONE) fsts_tp_lsins(ls,sh);    /* insert in queue if not done */
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
        fsts_tp_lsins(ls,sh);
        hs->use++;
      }else{
        /* full => replace maximal weight if possible */
        struct fsts_tp_s *sh=(struct fsts_tp_s*)fsts_hs2s(hs);
        struct fsts_tp_s *shmax=sh;
        for(i=1,sh++;i<hs->use;i++,sh++) if(sh->wn>shmax->wn) shmax=sh;
        #ifdef _DEBUG
        if(dbg>=4) printf(" => %sX (wn: %8.1f os: 0x%08x)",shmax->wn<=s->wn?"del":"rpl",shmax->wn,BTHASH(shmax->bt.os));
        #endif
        if(shmax->wn<=s->wn) fsts_tp_sfree(s,shmax,ls->btm); else {
          s->nxt=shmax->nxt;
          fsts_tp_sfree(shmax,s,ls->btm);
          *shmax=*s;                                         /* insert the state */
          if(shmax->nxt==SNXT_NONE) fsts_tp_lsins(ls,shmax);    /* insert in queue if not done */
        }
      }
    }
  }else{
    /* state not visited before => insert */
    #ifdef _DEBUG
    if(dbg>=4) printf(" => add0");
    #endif
    if(!(s=(struct fsts_tp_s*)fsts_hins(&ls->h,s))) return FSTSERR("hmem: out of memory");
    fsts_tp_lsins(ls,s);
  }
  return NULL;
}

/* TP active state queue remove function
 *
 * This function removes an active state from the queue
 * and returns the removed state.
 *
 * @param ls  The queue
 * @return    The removed state or <code>NULL</code> if the queue is empty
 */
struct fsts_tp_s *fsts_tp_lsdel(struct fsts_tp_ls *ls){
  struct fsts_tp_s *s=ls->qs;
  if(s){
    if(!(ls->qs=s->nxt)) ls->qe=NULL;
    s->nxt=SNXT_NONE;
  }
  return s;
}

/* TP active state queue remove best function
 *
 * This function removes the best active state (minimal weight)
 * from the queue and returns the removed state.
 *
 * @param ls  The queue
 * @return    The removed state or <code>NULL</code> if the queue is empty
 */
struct fsts_tp_s *fsts_tp_lsbest(struct fsts_tp_ls *ls,UINT8 del){
  struct fsts_tp_s **s, **best=NULL, *ret;
  for(s=&ls->qs;s[0];s=&s[0]->nxt)
    if(!best || s[0]->wn<best[0]->wn) best=s;
  if(!best) return NULL;
  if(!del) return best[0];
  ls->qe=NULL;
  ret=best[0];
  best[0]=ret->nxt;
  return ret;
}

/* dLabPro class CFstsearch (fstsearch)
 * - A* main programm
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

/* These are string representations of configure enums */
#define E(X)  #X
const char *fsts_as_aheustr[]={ AS_AH, NULL };
const char *fsts_as_sheustr[]={ AS_SH, NULL };
#undef E

/* A* decoder config function
 *
 * The function copies the configuration from fstsearch fields
 * to the internal structure.
 *
 * @param cfg   Pointer to A* configuration
 * @param _this Pointer to fstsearch instance
 * @return <code>NULL</code> if successfull, the error string otherwise
 */
const char *fsts_as_cfg(struct fsts_cfg *cfg,CFstsearch *_this){
  cfg->as.qsize=_this->m_nAsQsize;
  cfg->as.prnf =_this->m_nAsPrnf;
  cfg->as.prnw =_this->m_nAsPrnw;
  cfg->as.aheu =(enum fsts_as_aheu)fsts_cfg_enum(_this->m_lpsAsAheutype,fsts_as_aheustr);
  cfg->as.sheu =(enum fsts_as_sheu)fsts_cfg_enum(_this->m_lpsAsSheutype,fsts_as_sheustr);
  if(cfg->as.prnw<0.) return FSTSERR("negative value for as_prnw");
  if(cfg->as.aheu<0)  return FSTSERR("unknown value for as_aheutype");
  if(cfg->as.sheu<0)  return FSTSERR("unknown value for as_sheutype");
  return NULL;
}

/* A* decoder initialize function
 *
 * This function initializes the decoder.
 * It prepares the internal memory.
 *
 * @param glob  Pointer to the global memory structure
 * @return <code>NULL</code> if successfull, the error string otherwise
 */
const char *fsts_as_init(struct fsts_glob *glob){
  struct fsts_as_algo *algo;
  struct fsts_as_s s;
  const char *err;
  if(glob->src.nunits!=1) return FSTSERR("not implemented for multiple units");
  if(glob->cfg.bt==BT_T && !glob->src.itSrc) return FSTSERR("fast loading not possible with tp_bt=\"t\"");
/*  if((err=fsts_tp_chksrc(&glob->src))) return err; TODO */
  if(glob->cfg.as.aheu==AS_AH_POT && (err=fsts_fstpw(&glob->src))) return err;
  if(!(algo=(struct fsts_as_algo *)calloc(1,sizeof(struct fsts_as_algo)))) return FSTSERR("out of memory");
  glob->algo=algo;
  if((err=fsts_btm_init(&algo->btm,&glob->cfg))) return err;
  if((err=fsts_as_lsinit(&algo->ls,&algo->btm,&glob->cfg))) return err;
  if((err=fsts_as_sgen(&s,NULL,NULL,NULL,&algo->btm))) return err;
  s.w=glob->src.units[0].pot0;
  if(!(algo->res=(struct fsts_as_s**)calloc(glob->cfg.numpaths,sizeof(struct fsts_as_s*)))) return FSTSERR("out of memory");
  return fsts_as_lsadd(&algo->ls,&s,0);
}

/* A* decoder free function
 *
 * This function frees the internal memory of the decoder.
 *
 * @param glob  Pointer to the global memory structure
 */
void fsts_as_free(struct fsts_glob *glob){
  struct fsts_as_algo *algo=(struct fsts_as_algo *)glob->algo;
  if(!glob->algo) return;
  fsts_as_lsfree(&algo->ls);
  fsts_btm_free(&algo->btm);
  free(algo->res);
  free(glob->algo);
  glob->algo=NULL;
}

/* A* timevariant heuristic
 *
 * This function reduces the timevariant weights
 * by the frame mininal weight. The offset is
 * stored in w->wo.
 *
 * @param w  Timevariant weights
 */
void fsts_as_sheumin(struct fsts_w *w){
  INT32 f;
  for(f=0;f<w->nf;f++){
    INT32 s;
    FLOAT64 w0=w->w[w->ns*f];
    for(s=1;s<w->ns;s++) if(w->w[w->ns*f+s]<w0) w0=w->w[w->ns*f+s];
    for(s=0;s<w->ns;s++) w->w[w->ns*f+s]-=w0;
    w->w0+=w0;
  }
}

/* A* decoder search function
 *
 * This function decodes using the time variant weights.
 * If the weight array is empty timeinvariant decoding is performed.
 *
 * @param glob  Pointer to the global memory structure
 * @param w     Pointer to the time variant weight array
 * @return <code>NULL</code> if successfull, the error string otherwise
 */
const char *fsts_as_isearch(struct fsts_glob *glob,struct fsts_w *w){
  const char *err;
  struct fsts_as_algo *algo=(struct fsts_as_algo *)glob->algo;
  struct fsts_as_s *s1;
  struct fsts_unit *u=glob->src.units;
  FLOAT64 *wmin;
  INT32 f;
  UINT16 nres=0;
  algo->ls.h.schg[0]=(void**)&s1;
  fsts_as_mapinit(algo->ls.map,(w->nf+1)*glob->src.units[0].ns);
  if(glob->cfg.as.sheu==AS_SH_MIN) fsts_as_sheumin(w);
  wmin=(FLOAT64*)malloc((w->nf+1)*sizeof(FLOAT64));
  for(f=0;f<=w->nf;f++) wmin[f]=T_DOUBLE_MAX;
  algo->f=0;
  #ifdef OPT_AVL
  while(nres<glob->cfg.numpaths && (s1=fsts_as_qpop(&algo->ls.q,NULL)))
  #else
  while(nres<glob->cfg.numpaths && (s1=fsts_as_qpop(&algo->ls.q,1)))
  #endif
  {
    struct fsts_t *t=u->tfroms[s1->s];
    struct fsts_w wf;
    UINT8 prn;
    if(glob->debug>=2 && s1->f>algo->f) printf("next frame %4i: %s %s %s\n",algo->f,
          fsts_as_qdbg(&algo->ls.q,0),
          fsts_hdbg(&algo->ls.h,0),
          fsts_btmdbg(&algo->btm,0));
    if(s1->f>algo->f) wmin[(algo->f=s1->f)]=s1->w;
    if((prn=((!glob->cfg.as.prnf || algo->f-s1->f<=glob->cfg.as.prnf) && ((!glob->cfg.as.prnw || s1->w-glob->cfg.as.prnw<wmin[s1->f]))))){
      #ifdef _DEBUG
      if(glob->debug>=3) printf(" state: f %4i s %8i w %8.1f os: 0x%08x\n",s1->f,s1->s,s1->w,glob->cfg.numpaths>1?BTHASH(s1->bt.os):0);
      #endif
      algo->nstates++;
      fsts_wf(w,s1->f,&wf);
      for(;t;t=t->nxt){
        struct fsts_as_s s2;
        if(t->is>=0 && !wf.w && s1->f) continue;
        s2.f=s1->f;
        if(t->is>=0 && wf.w) s2.f++;
        if(t->stk<0 && (!s1->nstk || s1->stk[s1->nstk-1]!=-t->stk)) continue;
        s2.s=t->ter;
        s2.id=s2.f*u->ns+s2.s;
        if(fsts_as_mapget(algo->ls.map,s2.id)) continue;
        if((err=fsts_as_sgen(&s2,s1,t,&wf,&algo->btm))) return err;
        if(glob->cfg.as.prnw && s2.w-glob->cfg.as.prnw>=wmin[s2.f]){
          fsts_as_sfree(&s2,NULL,&algo->btm);
          continue;
        }
        #ifdef _DEBUG
        if(glob->debug>=4) printf("  => t: f %4i s %8i w %8.1f os: 0x%08x",
          s2.f,s2.s,s2.w,glob->cfg.numpaths>1?BTHASH(s2.bt.os):0);
        #endif
        if((err=fsts_as_lsadd(&algo->ls,&s2,glob->debug))) return err;
        #ifdef _DEBUG
        if(glob->debug>=4) printf("\n");
        #endif
      }
      if(u->sfin[s1->s] && !wf.w && !s1->nstk){
        struct fsts_as_s *s2=(struct fsts_as_s *)fsts_memget(&algo->ls.h.mem);
        if((err=fsts_as_sgen(s2,s1,NULL,NULL,&algo->btm))) return err;
        #ifdef _DEBUG
        if(glob->debug>=4) printf("  >fin: f %4i s %8i w %8.1f os: 0x%08x\n",s2->f,s2->s,s2->w,glob->cfg.numpaths>1?BTHASH(s2->bt.os):0);
        #endif
        if(w) s2->w+=w->w0;
        algo->res[nres++]=s2;
      }
    }
    fsts_as_lsfins(&algo->ls,s1,prn);
  }
  if(glob->debug>=1){
    if(glob->debug>=2) printf("Info:\n");
    printf("  nstates %4i\n",algo->nstates);
    printf("  %s\n",fsts_as_qdbg(&algo->ls.q,1));
    printf("  %s\n",fsts_hdbg(&algo->ls.h,1));
    printf("  %s\n",fsts_btmdbg(&algo->btm,1));
    printf("  map %4iMB\n",FSTSMB(algo->ls.map.n));
  }
  glob->mem = 
    sizeof(struct fsts_as_algo) +
    fsts_as_qmem(&algo->ls.q) +
    fsts_hmem(&algo->ls.h) +
    fsts_as_mapmem(algo->ls.map) +
    fsts_btmmem1(&algo->btm);
  fsts_as_mapfree(algo->ls.map);
  free(wmin);
  return NULL;
}

/* A* decoder backtrack function
 *
 * This function performes the backtracking after decoding.
 *
 * @param glob  Pointer to the global memory structure
 * @param itDst Destination transducer for backtracking
 * @return <code>NULL</code> if successfull, the error string otherwise
 */
const char *fsts_as_backtrack(struct fsts_glob *glob,CFst *itDst){
  struct fsts_as_algo *algo=(struct fsts_as_algo *)glob->algo;
  struct fsts_btinfo bti;
  struct fsts_as_s *s;
  const char *err;
  UINT32 i;
  if((err=fsts_btstart(&bti,glob,&algo->btm,itDst))) return err;
  for(i=0;i<glob->cfg.numpaths && (s=algo->res[i]);i++){
    fsts_btpath(&bti,s->w,&s->bt);
    if(glob->state==FS_SEARCHING) break;
    fsts_as_sfree(s,NULL,&algo->btm);
  }
  if(glob->debug>=1 && algo->btm.lat.head) printf("  lat %4iMB\n",FSTSMB(fsts_latsize(&algo->btm.lat)));
  glob->mem += fsts_btmmem2(&algo->btm);
  return NULL;
}


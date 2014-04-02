/* dLabPro class CFstsearch (fstsearch)
 * - TP search state
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

/* TP active state free function
 *
 * This function free's the backtrack data of an active state
 *
 * @param s     Active state which will be free'd
 * @param sref  State with which s was recombinded (for lattice generation)
 * @param btm   Bactrack memory
 */
void fsts_tp_sfree(struct fsts_tp_s *s,struct fsts_tp_s *sref,struct fsts_btm *btm){
  if(s->btfree) return;
  s->btfree=1;
  fsts_btsfree(&s->bt,sref?&sref->bt:NULL,sref?s->w-sref->w:0.,btm);
}

/* TP active state generation function
 *
 * This function generates a new active state from a previous one.
 *
 * @param s     The new active state
 * @param sref  The previous active state
 * @param t     The transition used
 * @param src   The source transducer
 * @param w     The timevariant weight array
 * @param btm   Bactrack memory
 * @param sub   Switch indicating lowest level for on-the-fly composition
 * @param ui0   Switch indicating the highest level
 * @return <code>NULL</code> if successfull, the error string otherwise
 */
const char *fsts_tp_sgen(struct fsts_tp_s *s,struct fsts_tp_s *sref,struct fsts_t *t,struct fsts_fst *src,struct fsts_w *w,struct fsts_btm *btm,UINT8 sub,UINT8 ui0){
  INT32 d;
  uint64_t mid;
  const char *err;
  if(!sref){
    s->ds=s->s[0]=0;
    s->nstk=0;
    s->w=0.;
    s->id=0;
    s->btfree=0;
/*    s->mid=1;*/
    return fsts_btsgen(&s->bt,NULL,NULL,0,0.,btm);
  }
  s->ds=sref->ds;
  for(d=0;d<=s->ds;d++) s->s[d]=sref->s[d];
  for(d=0;d<s->ds;d++)  s->u[d]=sref->u[d];
  s->w=sref->w;
/*  s->mid=sref->mid;*/
  s->btfree=0;
  if((err=fsts_btsgen(&s->bt,&sref->bt,t,ui0,s->w,btm))) return err;
  s->nstk=sref->nstk;
  if(!t){
    if(--s->ds<0){
      s->id=0;
    }else{
      mid=src->units[0].ns;
      for(d=0;d<s->ds;d++) mid*=src->nunits*src->units[s->u[d]].ns;
/*      s->mid/=src->nunits;*/
      s->id = sref->id%mid;
/*      s->mid/=src->units[s->ds?s->u[s->ds-1]:0].ns;*/
    }
    return NULL;
  }
  for(d=0;d<s->nstk;d++) s->stk[d]=sref->stk[d];
  if(t->stk>0){
    if(s->nstk+1==MAXSTK) return FSTSERR("stk: out of memory");
    s->stk[s->nstk++]=t->stk;
  }else if(t->stk<0) s->nstk--;
  s->w+=t->w;
  s->s[s->ds]=t->ter;
  mid=1;
  for(d=-1;d<s->ds-1;d++) mid*=src->nunits*src->units[d>=0?s->u[d]:0].ns;
  s->id = sref->id%mid + s->s[s->ds]*mid;
  if(t->is>=0){
    if(!sub){
      if(w) s->w+=w->w[t->is];
    }else{
      if(++s->ds==MAXLAYER) return FSTSERR("out of layers");
      s->u[s->ds-1]=t->is;
      s->s[s->ds]=0;
      mid*=src->units[s->ds>1?s->u[s->ds-2]:0].ns;
/*      s->mid*=src->units[s->ds>1?s->u[s->ds-2]:0].ns;*/
      s->id += t->is*mid;
/*      s->mid*=src->nunits;*/
    }
  }
  return NULL;
}

/* TP state stack compare function
 *
 * This function compares the pushdown memory
 * of two states.
 *
 * @param a  The first state
 * @param a  The second state
 * @return   0: not equal, 1: equal
 */
UINT8 fsts_tp_scmpstk(void *a,void *b){
  struct fsts_tp_s *as=(struct fsts_tp_s *)a;
  struct fsts_tp_s *bs=(struct fsts_tp_s *)b;
  INT32 i;
  if(bs->nstk!=as->nstk) return 0;
  for(i=0;i<as->nstk;i++) if(as->stk[i]!=bs->stk[i]) return 0;
  return 1;
}


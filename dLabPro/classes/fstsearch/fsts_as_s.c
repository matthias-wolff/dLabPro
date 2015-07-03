/* dLabPro class CFstsearch (fstsearch)
 * - A* search state
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

/* A* active state free function
 *
 * This function free's the backtrack data of an active state
 *
 * @param s     Active state which will be free'd
 * @param sref  State with which s was recombinded (for lattice generation)
 * @param btm   Bactrack memory
 */
void fsts_as_sfree(struct fsts_as_s *s,struct fsts_as_s *sref,struct fsts_btm *btm){
  fsts_btsfree(&s->bt,sref?&sref->bt:NULL,sref?s->w-sref->w:0.,btm);
}

/* A* active state generation function
 *
 * This function generates a new active state from a previous one.
 *
 * @param s     The new active state
 * @param sref  The previous active state
 * @param t     The transition used
 * @param w     The timevariant weight array
 * @param btm   Bactrack memory
 * @return <code>NULL</code> if successfull, the error string otherwise
 */
const char *fsts_as_sgen(struct fsts_as_s *s,struct fsts_as_s *sref,struct fsts_t *t,struct fsts_w *w,struct fsts_btm *btm){
  INT32 d;
  const char *err;
  if(!sref){
    s->id=0;
    s->s=s->f=0;
    s->nstk=0;
    s->w=0.;
    return fsts_btsgen(&s->bt,NULL,NULL,0,0.,btm);
  }
  if((err=fsts_btsgen(&s->bt,&sref->bt,t,1,s->w,btm))) return err;
  s->nstk=sref->nstk;
  for(d=0;d<s->nstk;d++) s->stk[d]=sref->stk[d];
  if(!t) s->w=sref->w; else{
    if(t->stk>0){
      if(s->nstk+1==MAXSTK) return FSTSERR("stk: out of memory");
      s->stk[s->nstk++]=t->stk;
    }else if(t->stk<0) s->nstk--;
    s->w=sref->w+t->w;
    if(t->is>=0 && w->w) s->w+=w->w[t->is];
  }
  return NULL;
}

/* A* state stack compare function
 *
 * This function compares the pushdown memory
 * of two states.
 *
 * @param a  The first state
 * @param a  The second state
 * @return   0: not equal, 1: equal
 */
UINT8 fsts_as_scmpstk(void *a,void *b){
  struct fsts_as_s *as=(struct fsts_as_s *)a;
  struct fsts_as_s *bs=(struct fsts_as_s *)b;
  INT32 i;
  if(bs->nstk!=as->nstk) return 0;
  for(i=0;i<as->nstk;i++) if(as->stk[i]!=bs->stk[i]) return 0;
  return 1;
}


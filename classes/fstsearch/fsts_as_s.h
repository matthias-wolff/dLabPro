/* dLabPro class CFstsearch (fstsearch)
 * - A* search state header
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
#ifndef _FSTS_AS_S_H
#define _FSTS_AS_S_H

/* A* internal active state */
struct fsts_as_s {
  uint64_t        id;          /* State id = f * numstates + s   */
  struct fsts_bts bt;          /* State backtrack node           */
  FLOAT64         w;           /* Current path weight            */
  UINT32          f,s;         /* Current frame and state number */
  #ifdef OPT_AVL
  struct fsts_as_qs *qs;
  #else
  UINT32          qp;          /* Position in active state queue */
  #endif
  INT8            nstk;        /* Usage of pushdown stack memory */
  UINT16          stk[MAXSTK]; /* Pushdown stack memory          */
};

void fsts_as_sfree(struct fsts_as_s *s,struct fsts_as_s *sref,struct fsts_btm *btm);
const char *fsts_as_sgen(struct fsts_as_s *s,struct fsts_as_s *sref,struct fsts_t *t,struct fsts_w *w,struct fsts_btm *btm);
UINT8 fsts_as_scmp(void *a,void *b);
UINT8 fsts_as_scmpstk(void *a,void *b);

#endif

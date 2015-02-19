/* dLabPro class CFstsearch (fstsearch)
 * - TP search state header
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
#ifndef _FSTS_TP_S_H
#define _FSTS_TP_S_H

/* TP internal active state */
struct fsts_tp_s {
  uint64_t id;             /* State id                               */
  struct fsts_tp_s *nxt;   /* Next active state in queue             */
  struct fsts_bts bt;      /* State backtrack node                   */
  FLOAT64 wc;              /* Current path weight                    */
  FLOAT64 wn;              /* Normalized current path weight         */
  INT32   l;               /* Path length                            */
  INT8    ds;              /* Depth level for on-the-fly composition */
  INT8    nstk;            /* Usage of pushdown stack memory         */
  INT8    btfree;
  INT32   s[MAXLAYER];     /* Transducer state index                 */
  INT32   u[MAXLAYER-1];   /* Transducer unit index                  */
  UINT16  stk[MAXSTK];     /* Pushdown stack memory                  */
};

void fsts_tp_sfree(struct fsts_tp_s *s,struct fsts_tp_s *sref,struct fsts_btm *btm);
const char *fsts_tp_sgen(struct fsts_tp_s *s,struct fsts_tp_s *sref,struct fsts_t *t,struct fsts_fst *src,struct fsts_w *w,struct fsts_btm *btm,UINT8 sub,UINT8 ui0);
UINT8 fsts_tp_scmpstk(void *a,void *b);

#endif

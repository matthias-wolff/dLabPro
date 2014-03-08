/* dLabPro class CFstsearch (fstsearch)
 * - TP state storage header
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
#ifndef _FSTS_TP_LS_H
#define _FSTS_TP_LS_H

/* Internal active state queue */
struct fsts_tp_ls {
  UINT32 numpaths;           /* Number of paths to decode                 */
  FLOAT64 wmax,wmin;         /* Minimal and maximal weight                */
  struct fsts_btm  *btm;     /* Backtrack memory                          */
  struct fsts_tp_s *qs, *qe; /* Begin and End of the connected state list */
  struct fsts_h h;           /* Active state hash memory                  */
  struct fsts_tp_hist hist;  /* Histogram for hypothesis pruning          */
};

const char *fsts_tp_lsinit(struct fsts_tp_ls *ls,struct fsts_btm *btm,struct fsts_cfg *cfg,FLOAT64 wmin);
void fsts_tp_lsfree(struct fsts_tp_ls *ls);
const char *fsts_tp_lsadd(struct fsts_tp_ls *ls,struct fsts_tp_s *s,UINT8 dbg);
struct fsts_tp_s *fsts_tp_lsdel(struct fsts_tp_ls *ls);
struct fsts_tp_s *fsts_tp_lsbest(struct fsts_tp_ls *ls,UINT8 del);

#endif

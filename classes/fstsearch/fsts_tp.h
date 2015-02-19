/* dLabPro class CFstsearch (fstsearch)
 * - TP main header
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
#ifndef _FSTS_TP_H
#define _FSTS_TP_H

/* TP deocoder config structure */
struct fsts_tp_cfg {
  INT32   jobs;        /* Number of jobs                     */
  INT32   prnh;        /* Hypthesis number pruning threshold */
  FLOAT64 prnw;        /* Weight pruning threshold           */
};

const char *fsts_tp_cfg(struct fsts_cfg *cfg,CFstsearch *_this);
const char *fsts_tp_init(struct fsts_glob *glob);
void fsts_tp_free(struct fsts_glob *glob);
const char *fsts_tp_isearch(struct fsts_glob *glob,struct fsts_w *w,UINT8 final,UINT8 start);
const char *fsts_tp_backtrack(struct fsts_glob *glob,CFst *itDst,UINT8 final);

#endif

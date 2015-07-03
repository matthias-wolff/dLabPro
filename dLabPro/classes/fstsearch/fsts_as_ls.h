/* dLabPro class CFstsearch (fstsearch)
 * - A* state storage header
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
#ifndef _FSTS_AS_LS_H
#define _FSTS_AS_LS_H

/* A* state storage memory */
struct fsts_as_ls {
  UINT32 numpaths;         /* Number of paths to decode */
  struct fsts_btm    *btm; /* Backtrack memory          */
  struct fsts_as_q   q;    /* Active state queue        */
  struct fsts_h      h;    /* Visitied states hash      */
  struct fsts_as_map map;  /* State done map            */
};

const char *fsts_as_lsinit(struct fsts_as_ls *ls,struct fsts_btm *btm,struct fsts_cfg *cfg);
void fsts_as_lsfree(struct fsts_as_ls *ls);
const char *fsts_as_lsadd(struct fsts_as_ls *ls,struct fsts_as_s *s,UINT8 dbg);
void fsts_as_lsfins(struct fsts_as_ls *ls,struct fsts_as_s *s,UINT8 prn);

#endif

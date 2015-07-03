/* dLabPro class CFstsearch (fstsearch)
 * - A* global header
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
#ifndef _FSTS_AS_ALGO_H
#define _FSTS_AS_ALGO_H

struct fsts_as_algo;

/* Switch queue type between binary heap and avl tree */
/*#define OPT_AVL*/

#include "fsts_as_map.h"
#include "fsts_as_s.h"
#include "fsts_as_q.h"
#include "fsts_as_ls.h"

/* TP internal memory structure */
struct fsts_as_algo {
  UINT32             f;       /* Current highest frame index */
  UINT64             nstates; /* Number of expanded states   */
  struct fsts_as_ls  ls;      /* State storage               */
  struct fsts_btm    btm;     /* Backtrack memory            */
  struct fsts_as_s **res;     /* Path end states             */
};

#endif

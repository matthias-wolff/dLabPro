/* dLabPro class CFstsearch (fstsearch)
 * - A* main header
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
#ifndef _FSTS_AS_H
#define _FSTS_AS_H

#define AS_AH    E(NONE), E(EXIST), E(POT) /* TODO: aheu exists not implemented */
#define AS_SH    E(NONE), E(EXIST), E(MIN), E(MINISU) /* TODO: sheu exists + minisu not implemented */

#define E(X)  AS_AH_##X
enum fsts_as_aheu { AS_AH };
#undef E
#define E(X)  AS_SH_##X
enum fsts_as_sheu { AS_SH };
#undef E
extern const char *fsts_as_aheustr[];
extern const char *fsts_as_sheustr[];

/* A* deocoder config structure */
struct fsts_as_cfg {
  UINT32            qsize; /* Size of the queue (0=auto)   */
  UINT32            prnf;  /* Frame pruning threshold      */
  FLOAT64           prnw;  /* Weight pruning threshold     */ /* TODO: prnw not implemented */
  enum fsts_as_aheu aheu;  /* Timeinvariant heuristic type */
  enum fsts_as_sheu sheu;  /* Timevariant heuristic type   */
};

const char *fsts_as_cfg(struct fsts_cfg *cfg,CFstsearch *_this);
const char *fsts_as_init(struct fsts_glob *glob);
void fsts_as_free(struct fsts_glob *glob);
const char *fsts_as_isearch(struct fsts_glob *glob,struct fsts_w *w);
const char *fsts_as_backtrack(struct fsts_glob *glob,CFst *itDst);

#endif

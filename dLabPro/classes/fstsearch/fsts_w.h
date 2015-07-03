/* dLabPro class CFstsearch (fstsearch)
 * - Internal timevariant weights header
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
#ifndef _FSTS_W_H
#define _FSTS_W_H

/* Internal weight array structure */
struct fsts_w {
  INT32 ns,nf; /* Number of states and frames  */
  FLOAT64 *w;  /* Pointer to the weights       */
  FLOAT64 w0;
  CData *idW;  /* Original source weight array */
};

const char *fsts_wgen(struct fsts_w *w,CData *idWeights);
void fsts_wf(struct fsts_w *w,INT32 f,struct fsts_w *wf);
void fsts_wfree(struct fsts_w *w);

#endif

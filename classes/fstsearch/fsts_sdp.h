/* dLabPro class CFstsearch (fstsearch)
 * - DP main header
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
#ifndef _FSTS_SDP_H
#define _FSTS_SDP_H

/* DP deocoder config structure */
struct fsts_sdp_cfg {
  FLOAT64 prn;       /* Pruning threshold */
  UINT8   epsremove; /* Perform eps removal after backtracking */
  UINT8   fwd;       /* Do forward algorihtm instead of DP */
};

const char *fsts_sdp_cfg(struct fsts_sdp_cfg *cfg,CFstsearch *_this);
const char *fsts_sdp_init(struct fsts_glob *glob);
void fsts_sdp_free(struct fsts_glob *glob);
const char *fsts_sdp_isearch(struct fsts_glob *glob,struct fsts_w *w);
const char *fsts_sdp_backtrack(struct fsts_glob *glob,CFst *itDst);

#endif

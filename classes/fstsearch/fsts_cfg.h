/* dLabPro class CFstsearch (fstsearch)
 * - Configuration header
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
#ifndef _FSTS_CFG_H
#define _FSTS_CFG_H

#define FA  E(TP), E(AS), E(SDP)
#define E(X)  FA_##X
enum fsts_algo {FA};
#undef E

#define BT    E(T), E(OS), E(LAT)
#define E(X)  BT_##X
enum fsts_ebt {BT};
#undef E

/* Global config structure */
struct fsts_cfg {
  enum fsts_algo algo;     /* Algorithm selector        */
  enum fsts_ebt bt;        /* Backtrack type            */
  UINT16 numpaths;         /* Number of paths to decode */
  UINT8 stkprn;            /* Stack pruning             */
  FLOAT64 latprn;          /* Lattice pruning threshold */
  struct fsts_as_cfg  as;  /* A* decoder config         */
  struct fsts_tp_cfg  tp;  /* TP decoder config         */
  struct fsts_sdp_cfg sdp; /* DP decoder config         */
};

const char *fsts_cfg(struct fsts_cfg *cfg,CFstsearch *_this);
INT32 fsts_cfg_enum(const char *val,const char **ref);

#endif

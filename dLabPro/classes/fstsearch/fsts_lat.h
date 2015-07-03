/* dLabPro class CFstsearch (fstsearch)
 * - Lattice generation header
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
#ifndef _FSTS_LAT_H
#define _FSTS_LAT_H

#define HMASK   0x0000ffff
#define OSMASK  0x000000ff
#define ASHMASK 0x0000ffff

/* Lattice propagate hash element */
struct fsts_latash {
  struct fsts_latash *nxt; /* Next hash element    */
  struct fsts_latas  *as;  /* Current active state */
  FLOAT64 w;               /* Remaining weight     */
};

/* Lattice residual for determinization */
struct fsts_latres {
  struct fsts_latres *nxt; /* Next residual for determined state */
  struct fsts_latas  *as;  /* Current lattice active state       */
  FLOAT64 w;               /* Remaining weight                   */
  FLOAT64 wa;              /* Total weight (for pruning)         */
};

/* Lattice determined state */
struct fsts_latds {
  struct fsts_latds  *hnxt; /* Next hash element        */
  struct fsts_latds  *qnxt; /* Next queue element       */
  struct fsts_latres *res;  /* Residuals for that state */
  INT32   nres;             /* Number of residuals      */
  UINT64  hash;             /* Hash key                 */
  INT32   os;               /* Output symbol            */
  INT32   s;                /* State in itDst           */
  FLOAT64 w;                /* Minimal residual weight  */
};

/* Lattice active state */
struct fsts_latas {
  struct fsts_latas *pa;  /* Parent active state with weight 0      */
  struct fsts_latas *paw; /* Parent active state with weight w      */
  INT32   os;             /* Output symbol                          */
  INT32   s;              /* State in itDst without determinization */
  FLOAT64 w;              /* Weight for transition to *paw          */
};

/* Lattice storage structure */
struct fsts_lat {
  struct fsts_latas *head;  /* Initial lattice state                   */
  struct fsts_mem   asmem;  /* Memory for active states                */
  struct fsts_mem   dsmem;  /* Memory for determined states            */
  struct fsts_mem   resmem; /* Memory for residuals in determinization */
  struct fsts_latds **hds;  /* Hash for determined states              */
  struct fsts_latds *qds;   /* Queue for determined states             */
  FLOAT64            prn;   /* Lattice pruning threshold               */
};

/* Return maximal amount of memory used for decoding */
#define fsts_latmem1(lat)   fsts_memmem(&(lat)->asmem)
/* Return maximal amount of memory used for backtracking */
#define fsts_latmem2(lat)   fsts_memmem(&(lat)->dsmem) + fsts_memmem(&(lat)->resmem)

const char *fsts_latinit(struct fsts_lat *lat,FLOAT64 prn);
void fsts_latfree(struct fsts_lat *lat);
UINT32 fsts_latsize(struct fsts_lat *lat);
struct fsts_latas *fsts_latgen(struct fsts_latas *asref,INT32 os,struct fsts_lat *lat);
struct fsts_latas *fsts_latjoin(struct fsts_latas *as,struct fsts_latas *asref,FLOAT64 wd,struct fsts_lat *lat);
const char *fsts_latbt(struct fsts_latas *as,struct fsts_lat *lat,CFst *itDst,INT32 ui,FLOAT64 w);

#endif

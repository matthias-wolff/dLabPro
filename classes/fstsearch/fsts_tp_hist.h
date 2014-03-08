/* dLabPro class CFstsearch (fstsearch)
 * - TP weight histogram header
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
#ifndef _FSTS_TP_HIST_H
#define _FSTS_TP_HIST_H

#define HISTSIZE  128
#define HISTLEN   500

/* Internal histogram pruning structure */
struct fsts_tp_hist {
  UINT8   on;          /* Histogram active     */
  FLOAT64 wmin;        /* Last minimal weight  */
  FLOAT64 step;        /* Histogram step width */
  INT32   h[HISTSIZE]; /* Histogram counters   */
};

/* Macro inserting a new weight */
#define fsts_tp_histins(hist,w) if((hist)->on){ \
  INT32 hi=(INT32)(((w)-(hist)->wmin)/(hist)->step+0.5); \
  if(hi<0) hi=0; \
  if(hi<HISTSIZE) (hist)->h[hi]++; \
}

void fsts_tp_histinit(struct fsts_tp_hist *hist,FLOAT64 wmin);
double fsts_tp_histthr(struct fsts_tp_hist *hist,INT32 prnhyp);


#endif

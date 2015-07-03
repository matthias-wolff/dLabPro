/* dLabPro class CFstsearch (fstsearch)
 * - TP weight histogram
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

#include "fsts_glob.h"
#include "fsts_tp_algo.h"

/* TP histogram initialization function
 *
 * This function initializes the histogram used for hypothesis number pruning.
 *
 * @param hist  Histogram memory
 * @param wmin  Minimal weight of last frame
 */
void fsts_tp_histinit(struct fsts_tp_hist *hist,FLOAT64 wmin){
  INT32 hi;
  if(wmin==T_DOUBLE_MIN) hist->on=0;
  else{
    hist->on=1;
    hist->wmin=wmin;
    hist->step=HISTLEN/HISTSIZE;
    for(hi=0;hi<HISTSIZE;hi++) hist->h[hi]=0;
  }
}

/* TP histogram finalization function
 *
 * This function finalizes the histogram used for hypothesis number pruning
 * and returns the new pruning threshold.
 *
 * @param hist   Histogram memory
 * @param prnhyp Number of hypothesis remaining after pruning
 * @return       New pruning threshold
 */
double fsts_tp_histthr(struct fsts_tp_hist *hist,INT32 prnhyp){
  INT32 hi;
  if(!hist->on) return 0.;
  hist->on=0;
  for(hi=0;hi<HISTSIZE && prnhyp>0;hi++) prnhyp-=hist->h[hi];
  if(hi==HISTSIZE) return 0.;
  return hist->wmin+hi*hist->step;
}


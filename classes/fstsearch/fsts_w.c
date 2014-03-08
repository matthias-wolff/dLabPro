/* dLabPro class CFstsearch (fstsearch)
 * - Internal timevariant weights
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

/* Weight convert function
 *
 * This function converts the timevariant weight array form
 * data instance to the internal structure.
 *
 * @param w          Destination weight array structure
 * @param idWeights  Source data instance
 * @return <code>NULL</code> if successfull, the error string otherwise
 */
const char *fsts_wgen(struct fsts_w *w,CData *idWeights){
  INT32 f,s;
  w->ns=w->nf=0; w->w=NULL;
  w->w0=0.;
  w->idW=idWeights;
  if(!idWeights) return NULL;
  w->ns=CData_GetNComps(idWeights);
  w->nf=CData_GetNRecs(idWeights);
  if(!w->nf) return NULL;
  if(CData_IsHomogen(idWeights)==T_DOUBLE){
    w->w=(FLOAT64*)CData_XAddr(idWeights,0,0);
    return NULL;
  }
  if(!(w->w=(FLOAT64*)malloc(w->ns*w->nf*sizeof(FLOAT64)))) return FSTSERR("out of memory");
  for(f=0;f<w->nf;f++) for(s=0;s<w->ns;s++)
    w->w[f*w->ns+s]=CData_Dfetch(idWeights,f,s);
  return NULL;
}

/* Get the timevariant weight vectore for one specific frame
 *
 * This function extracts from the weight array a new weight array
 * with only one frame. If the frame is not existing, wf->w will
 * be NULL.
 *
 * @param w   Source weight array
 * @param f   Frame index
 * @param wf  Destination weight array (vector)
 */
void fsts_wf(struct fsts_w *w,INT32 f,struct fsts_w *wf){
  if(!w || !w->w || f>=w->nf) wf->w=NULL; else {
    wf->nf=1;
    wf->ns=w->ns;
    wf->w=w->w+w->ns*f;
  }
}

/* Free weight array
 *
 * This function frees the internal weight array structure.
 *
 * @param w   Weight array
 */
void fsts_wfree(struct fsts_w *w){
  if(w->w && w->w!=(FLOAT64*)CData_XAddr(w->idW,0,0)) free(w->w);
}

/* dLabPro class CFstsearch (fstsearch)
 * - DP main programm
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
#include "fsts_sdp_imp.h"

/* DP decoder config function
 *
 * The function copies the configuration from fstsearch fields
 * to the internal structure.
 *
 * @param cfg   Pointer to DP configuration
 * @param _this Pointer to fstsearch instance
 * @return <code>NULL</code> if successfull, the error string otherwise
 */
const char *fsts_sdp_cfg(struct fsts_sdp_cfg *cfg,CFstsearch *_this){
  cfg->prn=_this->m_nSdpPrn;
  cfg->epsremove=_this->m_bSdpEpsremove;
  cfg->fwd=_this->m_bSdpFwd;
  if(cfg->prn<0. || cfg->prn>1.) return FSTSERR("pruning constant out of bounds");
  return NULL;
}

/* DP decoder initialize function
 *
 * This function initializes the decoder.
 * It prepares the internal memory.
 *
 * @param glob  Pointer to the global memory structure
 * @return <code>NULL</code> if successfull, the error string otherwise
 */
const char *fsts_sdp_init(struct fsts_glob *glob){
  CFst *itDst;
  if(glob->cfg.numpaths!=1) return FSTSERR("not implemented for multiple paths");
  if(glob->src.nunits!=1) return FSTSERR("not implemented for multiple units");
  if(!glob->src.itSrc) return FSTSERR("this algo does not work with fast loading");
  ICREATEEX(CFst,itDst,"fsts_sdp_dst",NULL);
  if(!itDst) return FSTSERR("creation of destination object failed");
  glob->algo=itDst;
  return NULL;
}

/* DP decoder free function
 *
 * This function frees the internal memory of the decoder.
 *
 * @param glob  Pointer to the global memory structure
 */
void fsts_sdp_free(struct fsts_glob *glob){
  CFst *itDst=(CFst*)glob->algo;
  if(!glob->algo) return;
  IDESTROY(itDst);
  glob->algo=NULL;
}

/* DP decoder search function
 *
 * This function decodes using the time variant weights.
 *
 * @param glob  Pointer to the global memory structure
 * @param w     Pointer to the time variant weight array
 * @return <code>NULL</code> if successfull, the error string otherwise
 */
const char *fsts_sdp_isearch(struct fsts_glob *glob,struct fsts_w *w){
  CFst *itDst=(CFst*)glob->algo;
  if(glob->cfg.sdp.prn){
    itDst->m_nPrnConst=glob->cfg.sdp.prn;
    itDst->m_bPrune=TRUE;
  }
  if(glob->cfg.sdp.epsremove) itDst->m_bEpsremove=TRUE;
  if(glob->cfg.sdp.fwd)       itDst->m_bFwd=TRUE;
  if(CFst_Sdp((CFst*)glob->algo,glob->src.itSrc,0,w->idW)!=O_K) return FSTSERR("sdp failed");
  glob->mem =
    UD_XS(glob->src.itSrc,0)*2*sizeof(FST_LB_TYPE) + /* lpLBrd, lpLBwr */
     sizeof(FST_BT_TYPE) + CData_GetDescr(AS(CData,((CFst*)glob->algo)->ud),DESCR4)*sizeof(BYTE*) /* BT */;
  return NULL;
}

/* DP decoder backtrack function
 *
 * This function performes the backtracking after decoding.
 *
 * @param glob  Pointer to the global memory structure
 * @param itDst Destination transducer for backtracking
 * @return <code>NULL</code> if successfull, the error string otherwise
 */
const char *fsts_sdp_backtrack(struct fsts_glob *glob,CFst *itDst){
  INT32 gwi;
  CDlpObject_Copy(BASEINST(itDst),BASEINST(((CFst*)glob->algo)));
  if((gwi=CData_FindComp(AS(CData,itDst->ud),"~GW"))<0){
    CData_AddComp(AS(CData,itDst->ud),"~GW",T_DOUBLE);
    gwi=CData_GetNComps(AS(CData,itDst->ud))-1;
  }
  CData_Dstore(AS(CData,itDst->ud),itDst->m_nGw,0,gwi);
  return NULL;
}


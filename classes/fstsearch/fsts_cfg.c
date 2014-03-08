/* dLabPro class CFstsearch (fstsearch)
 * - Configuration
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

#define E(X)  #X
const char *fsts_algostr[]={FA, NULL};
#undef E

#define E(X)  #X
const char *fsts_ebtstr[]={BT, NULL};
#undef E

/* Config function
 *
 * This function configures the selected decoding algorithm.
 * It copies the options from fstsearch fields to the internal
 * config structure.
 *
 * @param cfg   Pointer to configuration structure
 * @param _this Pointer to fstsearch instance
 * @return <code>NULL</code> if successfull, the error string otherwise
 */
const char *fsts_cfg(struct fsts_cfg *cfg,CFstsearch *_this){
  const char *err=NULL;
  cfg->algo=(enum fsts_algo)fsts_cfg_enum(_this->m_lpsAlgo,fsts_algostr);
  cfg->bt  =(enum fsts_ebt )fsts_cfg_enum(_this->m_lpsBt  ,fsts_ebtstr );
  cfg->numpaths=_this->m_nNumpaths;
  cfg->stkprn=_this->m_bStkprn;
  cfg->latprn=_this->m_nLatprn;
  if(_this->m_nNumpaths>65535) return FSTSERR("numpaths greater than 65535");
  if(cfg->algo<0)   return FSTSERR("algo unkown");
  if(cfg->bt  <0)   return FSTSERR("backtrack type (bt) unkown");
  if(cfg->latprn<0) return FSTSERR("negative value for latprn");
  switch(cfg->algo){
  case FA_TP:  err=fsts_tp_cfg(cfg,_this);   break;
  case FA_AS:  err=fsts_as_cfg(cfg,_this);   break;
  case FA_SDP: err=fsts_sdp_cfg(&cfg->sdp,_this); break;
  }
  return err;
}

/* Convert string to enum
 *
 * This function converts a string in value config option
 * in an enumartion type.
 *
 * @param val  The value for conversion
 * @param ref  List of possible value terminated by <code>NULL</code>
 * @return     The index of the given value or -1 if not found
 */
INT32 fsts_cfg_enum(const char *val,const char **ref){
  INT32 vali=0;
  while(ref[0] && dlp_strnicmp(ref[0],val,strlen(ref[0]))){
    ref++;
    vali++;
  }
  if(!ref[0]) return -1;
  return vali;
}

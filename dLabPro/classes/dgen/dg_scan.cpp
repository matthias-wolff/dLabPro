// dLabPro class CDgen (DGen)
// - Common scanner methods
//
// AUTHOR : Matthias Wolff
// PACKAGE: dLabPro/classes
// 
// Copyright 2013 dLabPro contributors and others (see COPYRIGHT file) 
// - Chair of System Theory and Speech Technology, TU Dresden
// - Chair of Communications Engineering, BTU Cottbus
// 
// This file is part of dLabPro.
// 
// dLabPro is free software: you can redistribute it and/or modify it under the
// terms of the GNU Lesser General Public License as published by the Free
// Software Foundation, either version 3 of the License, or (at your option)
// any later version.
// 
// dLabPro is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
// details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with dLabPro. If not, see <http://www.gnu.org/licenses/>.

#include "dlp_dgen.h"

/**
 * Determines if the string lpsBuffer starts with the "key" string lpsKey and,
 * if this is the case, returns a pointer to the "value" string following the
 * key.
 *
 * @param lpsBuffer Pointer to a string to scan
 * @param lpsKey    POinter to the key string
 * @return Pointer to value string
 */
const char* CGEN_PROTECTED CDgen::ScanKey(const char* lpsBuffer, const char* lpsKey)
{
  if (!dlp_strlen(lpsKey)                ) return NULL;
  if (!dlp_strlen(lpsBuffer)             ) return NULL;
  if (strstr(lpsBuffer,lpsKey)!=lpsBuffer) return NULL;

  const char* tx = &lpsBuffer[strlen(lpsKey)];
  while (*tx && iswspace(*tx)) tx++;
  if (*tx++!=':') return NULL;
  while (*tx && iswspace(*tx)) tx++;

  return tx;
}

/**
 * Stores a key/value pair in the scanned document's property list. The property
 * string will be stored in record nPos or at the end of the list if nPos is
 * negative. Any previous values will be overwritten.
 *
 * @param lpsKey   Property name (max. 32 characters)
 * @param lpsValue Property string (max. 255 characters)
 * @param nPos     Ordinal number of property
 * @return         The non-negative record number in m_idSpl in which the property
 *                 was stored or a negative error code
 */
INT32 CGEN_PROTECTED CDgen::StoreProperty(const char* lpsKey, const char* lpsValue, INT32 nPos DEFAULT(-1))
{
  if (nPos<0) nPos=m_idSpl->GetNRecs();
  if (nPos>=m_idSpl->GetMaxRecs()) m_idSpl->Realloc(nPos+1);

  if ((INT32)dlp_strlen(lpsKey  )+1>m_idSpl->GetCompSize(0)) IERROR(this,DG_TOOLONG2,"Property key"  ,lpsKey  , 32);
  if ((INT32)dlp_strlen(lpsValue)+1>m_idSpl->GetCompSize(1)) IERROR(this,DG_TOOLONG2,"Property value",lpsValue,255);

  if (m_idSpl->GetNRecs()<nPos+1) m_idSpl->SetNRecs(nPos+1);
  m_idSpl->Sstore(lpsKey  ,nPos,0);
  m_idSpl->Sstore(lpsValue,nPos,1);

  return nPos;
}

/**
 * Stores additional text in the document's texts container and returns the
 * number of records appended. The method stores the text in field {@link txt}
 * in blocks of maximal 255 characters. Every call to <code>StoreText</code>
 * will append at least one new record to {@link txt}.
 * 
 * @param lpsTxt
 *          Pointer to the text to be stored
 * @param lpnFrec
 *          Pointer to a long to be filled with the index of the first new
 *          record index in {@link txt} (may be <code>NULL</code> or omitted)
 * @return The number of records appended to {@link txt} or 0 in case of errors.
 */
INT32 CGEN_PROTECTED CDgen::TxtStore
(
  const char* lpsTxt,
  INT32*       lpnFrec DEFAULT(0)
)
{
  INT32        nR     = 0;
  INT32        nFR    = CData_GetNRecs(m_idTxt);
  INT32        nXR    = (INT32)(dlp_strlen(lpsTxt)/255)+1;
  INT32        nLen   = 0;
  const char* lpsSrc = NULL;
  char*       lpsDst = NULL;

  DLPASSERT(CData_GetNComps(m_idTxt)==1);
  CData_AddRecs(m_idTxt,nXR,nXR);
  for (nR=0,lpsSrc=lpsTxt; nR<nXR; nR++,lpsSrc+=255)
  {
    nLen   = MIN((INT32)dlp_strlen(lpsSrc),255);
    lpsDst = (char*)CData_XAddr(m_idTxt,nR+nFR,0);
    dlp_memmove(lpsDst,lpsSrc,nLen);
    lpsDst[nLen]='\0';
  }

  if (lpnFrec) *lpnFrec = nFR;
  return nXR;
}

// EOF

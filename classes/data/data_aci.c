/* dLabPro class CData (data)
 * - Additional C/C++ methods
 *
 * AUTHOR : Matthias Eichner
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

#include "dlp_cscope.h" /* Indicate C scope */
#include "dlp_data.h"

/**
 * Returns a pointer to the data element in record nRec and
 * component nComp. If this element does not exists, NULL will
 * be returned.
 *
 * @param _this This instance
 * @param nRec  Record index of data element
 * @param nComp Component index of data element
 * @return A pointer to the element, or NULL in case of any errors
 */
BYTE* CGEN_PUBLIC CData_XAddr(CData* _this, INT32 nRec, INT32 nComp)
{
  CHECK_THIS_RV(NULL);
  return CDlpTable_XAddr(_this->m_lpTable,nRec,nComp);
}

/**
 * Allocates memory for nRecs records of the current record structure
 * (component setup). In contrast to CData_Allocate, this method does
 * not mark the allocated records valid. Consequently, after executing
 * this method the instance contains 0 valid records.
 * The memory will be zeroed.
 *
 * NOTE: CData_Alloc will destroy all data content.
 *
 * @param _this This instance
 * @param nRecs The number of records to allocate
 * @return O_K if successfull, an error code otherwise
 * @cgen:option /fast
 */
INT16 CGEN_PUBLIC CData_Alloc(CData* _this, INT32 nRecs)
{
  CHECK_THIS_RV(NOT_EXEC);

  if(_this->m_bFast == TRUE) {
    if(O_K != CDlpTable_AllocUninitialized(_this->m_lpTable,nRecs)) return NOT_EXEC;
  } else {
    if(O_K != CDlpTable_Alloc(_this->m_lpTable,nRecs)) return NOT_EXEC;
  }
  return O_K;
}

/**
 * Allocates memory for nRecs records of the current record structure
 * (component setup). In contrast to CData_AllocateUninitialized, this
 * method does not mark the allocated records valid. Consequently,
 * after executing this method the instance contains 0 valid records.
 * The memory will NOT be zeroed.
 *
 * NOTE: CData_Alloc will destroy all data content.
 *
 * @param _this This instance
 * @param nRecs The number of records to allocate
 * @return O_K if successfull, an error code otherwise
 * @cgen:option /fast
 */
INT16 CGEN_PUBLIC CData_AllocUninitialized(CData* _this, INT32 nRecs)
{
  CHECK_THIS_RV(NOT_EXEC);

  if(O_K != CDlpTable_AllocUninitialized(_this->m_lpTable,nRecs)) return NOT_EXEC;

  return O_K;
}

/**
 * Allocates memory for nRecs records of the current record structure
 * (component setup). In contrast to CData_AllocUninitialized, this
 * method mark the allocated records valid. Consequently, after executing
 * this method the instance contains <nRecs> valid records.
 * The memory will NOT be zeroed.
 *
 * NOTE: CData_Alloc will destroy all data content.
 *
 * @param _this This instance
 * @param nRecs The number of records to allocate
 * @return O_K if successfull, an error code otherwise
 * @cgen:option /fast
 */
INT16 CGEN_PUBLIC CData_AllocateUninitialized(CData* _this, INT32 nRecs)
{
  CHECK_THIS_RV(NOT_EXEC);

  if(O_K != CDlpTable_AllocateUninitialized(_this->m_lpTable,nRecs)) return NOT_EXEC;

  return O_K;
}

/**
 * Reallocates the memory block for the data content. The new
 * memory block can hold nRecs records of the current record
 * structure (component setup).
 *
 * ATTENTION: After the reallocation ALL pointers into the
 *            data content become invalid!
 *
 * @param _this This instance
 * @param nRecs New size (in records) of the data block
 * @return O_K of successfull, an error code otherwise
 */
INT16 CGEN_PUBLIC CData_Realloc(CData* _this, INT32 nRecs)
{
  CHECK_THIS_RV(NOT_EXEC);
  return CDlpTable_Realloc(_this->m_lpTable,nRecs);
}

/**
 * Appends nRecs valid records to the end of the table and returns the record
 * index of the first new record. If nRealloc is greater than 0, the table's
 * memory block will be reallocated if necessary to hold the new records. If
 * nRealloc is 0, the method will NOT reallocate the memory. If the present
 * allocated memory cannot hold nRecs additional recerds, the method will
 * return -1.
 *
 * WARNING: If nRealloc is greater 0 the method may reallocate the memory.
 * Pointers into the memory block become invalid after a reallocation!
 *
 * @param _this    This instance
 * @param nRecs    Number of records to append
 * @param nRealloc Capacity increment on reallocation (at least nRecs)
 * @return The index of the first new record or -1 if no more memory
 */
INT32 CGEN_PUBLIC CData_AddRecs(CData* _this, INT32 nRecs, INT32 nRealloc)
{
  CHECK_THIS_RV(NOT_EXEC);
  return CDlpTable_AddRecs(_this->m_lpTable,nRecs,nRealloc);
}

/**
 * Inserts nRecs valid records at record index nInsertAr and returns the record
 * index of the first new record. If nRealloc is greater than 0, the table's
 * memory block will be reallocated if necessary to hold the new records. If
 * nRealloc is 0, the method will NOT reallocate the memory. If the present
 * allocated memory cannot hold nRecs additional recerds, the method will
 * return -1.
 *
 * WARNING: If nRealloc is greater 0 the method may reallocate the memory.
 * Pointers into the memory block become invalid after a reallocation!
 *
 * @param _this     _this pointer to CDlpTable instance
 * @param nInsertAt Component index of the first record to insert
 * @param nRecs     Number of records to append
 * @param nRealloc  Capacity increment on reallocation (at least nRecs)
 * @return The index of the first new record or -1 if no more memory
 */
INT32 CGEN_PUBLIC CData_InsertRecs(CData* _this, INT32 nInsertAt, INT32 nRecs, INT32 nRealloc)
{
  CHECK_THIS_RV(NOT_EXEC);
  return CDlpTable_InsertRecs(_this->m_lpTable,nInsertAt,nRecs,nRealloc);
}

/**
 * Returns the number of valid records. Most of the algorithms
 * use this method to determine how many records need to be
 * processed. The number of valid records is always smaller or
 * equal than the capacity (see CData_GetMaxRecs).
 *
 * @param _this This instance
 * @return The number of valid records
 */
INT32 CGEN_PUBLIC CData_GetNRecs(CData* _this)
{
  CHECK_THIS_RV(0);
  return CDlpTable_GetNRecs(_this->m_lpTable);
}

/**
 * Returns the number of components in the record structure.
 *
 * @param _this This instance
 * @return The number of components of each record
 */
INT32 CGEN_PUBLIC CData_GetNComps(CData* _this)
{
  CHECK_THIS_RV(0);
  return CDlpTable_GetNComps(_this->m_lpTable);
}

/**
 * Returns the continuition rate.
 *
 * @param _this This instance
 * @return continuition rate.
 */
FLOAT64 CGEN_PUBLIC CData_GetFsr(CData* _this)
{
  CHECK_THIS_RV(0);
  return CDlpTable_GetFsr(_this->m_lpTable);
}

/**
 * Returns the number of numeric components in the record
 * structure. Numeric components are all components which
 * are not character arrays. The number of numeric components
 * is sometimes called the "numeric dimension" of the data
 * instance.
 *
 * @param _this This instance
 * @return The number of numeric components of each record
 */
INT32 CGEN_PUBLIC CData_GetNNumericComps(CData* _this)
{
  INT32 i      = 0;
  INT32 nComps = 0;

  CHECK_THIS_RV(0);

  for (i=0; i<CData_GetNComps(_this); i++)
    if (dlp_is_numeric_type_code(CData_GetCompType(_this,i))) nComps++;

  return nComps;
}

/**
 * Returns the number of complex numeric components in the record
 * structure. Complex numeric components are all components which
 * are explicitly of type complex.
 *
 * @param _this This instance
 * @return The number of complex numeric components of each record
 */
INT32 CGEN_PUBLIC CData_GetNComplexComps(CData* _this)
{
  INT32 i      = 0;
  INT32 nComps = 0;

  CHECK_THIS_RV(0);

  for (i=0; i<CData_GetNComps(_this); i++)
    if (dlp_is_complex_type_code(CData_GetCompType(_this,i))) nComps++;

  return nComps;
}

/**
 * Returns the number of blocks in the data instance.
 * Blocks are a virtual subdevision of data instances.
 * One data instance may be considered as a sequence of
 * equally sized blocks (think of a tensor or of a matrix
 * sequence).
 *
 * @param _this This instance
 * @return The number of blocks or 0 if <code>_this</code> is <code>NULL</code>
 */
INT32 CGEN_PUBLIC CData_GetNBlocks(CData* _this)
{
  CHECK_THIS_RV(0);
  DLPASSERT(_this->m_nblock>=0)
  if (_this->m_nblock==0) return 1;
  return _this->m_nblock;
}

/**
 * Returns the number of records in one block of the data instance.
 * Blocks are a virtual subdevision of data instances.
 * One data instance may be considered as a sequence of
 * equally sized blocks (think of a tensor or of a matrix
 * sequence).
 *
 * @param _this This instance
 * @return The number records in each block or 0, if there are
 *         no blocks defined
 */
INT32 CGEN_PUBLIC CData_GetNRecsPerBlock(CData* _this)
{
  CHECK_THIS_RV(0);
  DLPASSERT(_this->m_nblock>=0)
  if (CData_GetNBlocks(_this)==0) return 0;
  return CData_GetNRecs(_this) / CData_GetNBlocks(_this);
}

/**
 * Sets the number of blocks contained in the data instance.
 * Blocks are a virtual subdevision of data instances.
 * One data instance may be considered as a sequence of
 * equally sized blocks (think of a tensor or of a matrix
 * sequence).
 * If nBlocks is less or qual zero, nothing is done (i.e. the
 * previous number of blocks remains unchanged). If nBlocks
 * is greater than the capacity (maximal number of records),
 * the number of blocks is set to the capacity.
 *
 * @param _this   This instance
 * @param nBlocks The new block number
 * @return The actual number of blocks after executing the method
 */
INT32 CGEN_PUBLIC CData_SetNBlocks(CData* _this, INT32 nBlocks)
{
  CHECK_THIS_RV(0);
  if (nBlocks < 0) return _this->m_nblock;

  _this->m_nblock = nBlocks>CData_GetMaxRecs(_this) ? CData_GetMaxRecs(_this) : nBlocks;
  return _this->m_nblock;
}

/**
 * Increments or decrements the number of valid records by nRecs.
 * The resulting number of valid records is checked, i.e. it cannot
 * exceed the capacity (see CData_GetMaxRecs) or fall below zero.
 *
 * @param _this This instance
 * @param nRecs The increment of the number of valid records (may be
 *              zero or negative)
 * @return The actual number of valid records after executing the method
 */
INT32 CGEN_PUBLIC CData_IncNRecs(CData* _this, INT32 nRecs)
{
  CHECK_THIS_RV(NOT_EXEC);
  return CDlpTable_IncNRecs(_this->m_lpTable, nRecs);
}

/**
 * Sets the number of valid records to nRecs. The resulting number
 * of valid records is checked, i.e. it cannot exceed the capacity
 * (see CData_GetMaxRecs) or fall below zero.
 *
 * @param _this This instance
 * @param nRecs The new number of valid records
 * @return The actual number of valid records after executing the method
 */
INT32 CGEN_PUBLIC CData_SetNRecs(CData* _this, INT32 nRecs)
{
  CHECK_THIS_RV(NOT_EXEC);
  return CDlpTable_SetNRecs(_this->m_lpTable, nRecs);
}

/**
 * Returns the capacity of the data instance. The capacity is the maximal
 * number of records which can be stored in the currenly allocated memory
 * block.
 *
 * @param _this This instance
 * @return The capacity (in records) of the allocated memory
 */
INT32 CGEN_PUBLIC CData_GetMaxRecs(CData* _this)
{
  CHECK_THIS_RV(0);
  return CDlpTable_GetMaxRecs(_this->m_lpTable);
}

/**
 * Returns the size of data type of component nComp in bytes.
 *
 * @param _this This instance
 * @param nComp The zero-based index of the component in question
 * @return The size (in bytes) of the variable type stored in
 *         component nComp
 */
INT32 CGEN_PUBLIC CData_GetCompSize(CData* _this, INT32 nComp)
{
  CHECK_THIS_RV(0);
  return CDlpTable_GetCompSize(_this->m_lpTable, nComp);
}

/**
 * Returns the offset of component nComp in bytes from the first byte of the
 * records in bytes.
 *
 * @param _this This instance
 * @param nComp The zero-based index of the component in question
 * @return The offset (in bytes) of component nComp
 */
INT32 CGEN_PUBLIC CData_GetCompOffset(CData* _this, INT32 nComp)
{
  CHECK_THIS_RV(0);
  return CDlpTable_GetCompOffset(_this->m_lpTable, nComp);
}

/**
 * Returns the length of one record of the data instance in bytes.
 * This length is equal to the sum of all component sizes (see
 * CData_GetCompSize). There is NO memory alignment!
 *
 * @param _this This instance
 * @return The size (in bytes) of one record
 */
INT32 CGEN_PUBLIC CData_GetRecLen(CData* _this)
{
  CHECK_THIS_RV(0);
  return CDlpTable_GetRecLen(_this->m_lpTable);
}

/**
 * ??
 *
 * @param _this This instance
 * @param nComp The zero-based index of the component carrying the search
 *              criterion
 * @param sWhat The search criterion
 * @return The zero based record index of the first record which matches
 *         the search criterion or -1 if no matching record was found
 */
INT32 CGEN_PUBLIC CData_FindRec(CData* _this, INT32 nComp, char* sWhat)
{
  CHECK_THIS_RV(NOT_EXEC);
  return CData_Find(_this,0,CData_GetNRecs(_this),1,(long)nComp,sWhat);
}

/**
 * Fetches an entire numeric component
 *
 * @param _this    This instance
 * @param dBuffer  Vector address, must contain nMaxComps elements
 * @param nComp    Index of component to fetch
 * @param nMaxRecs Maximal number of records to fetch
 * @return The number of actually fetched values
 */
INT32 CGEN_PUBLIC CData_CcompFetch(CData* _this, COMPLEX64* dBuffer, INT32 nComp, INT32 nMaxRecs)
{
  INT32 i = 0;

  CHECK_THIS_RV(0);

  if(!dBuffer)                       return 0;
  if(nComp>=CData_GetNComps(_this))  return 0;
  if(nMaxRecs<=0)                    return 0;
  if(nComp<0)                        return 0;
  if(dlp_is_numeric_type_code(CData_GetCompType(_this,nComp))!=TRUE) return 0;

  nMaxRecs=((nMaxRecs>CData_GetMaxRecs(_this))?CData_GetMaxRecs(_this):nMaxRecs);

  for(i=0;i<nMaxRecs;i++) *(dBuffer+i)=CData_Cfetch(_this,i,nComp);

  /* HACK: This is not the number of actually fetched values!!! (??) */
  return nMaxRecs;
}

/**
 * Fetches the real part of an entire numeric component
 *
 * @param _this    This instance
 * @param dBuffer  Vector address, must contain nMaxComps elements
 * @param nComp    Index of component to fetch
 * @param nMaxRecs Maximal number of records to fetch
 * @return The number of actually fetched values
 */
INT32 CGEN_PUBLIC CData_DcompFetch(CData* _this, FLOAT64* dBuffer, INT32 nComp, INT32 nMaxRecs)
{
  INT32 i = 0;

  CHECK_THIS_RV(0);

  if(!dBuffer)                       return 0;
  if(nComp>=CData_GetNComps(_this)) return 0;
  if(nMaxRecs<=0)                    return 0;
  if(nComp<0)                        return 0;
  if(dlp_is_numeric_type_code(CData_GetCompType(_this,nComp))!=TRUE) return 0;

  nMaxRecs=((nMaxRecs>CData_GetMaxRecs(_this))?CData_GetMaxRecs(_this):nMaxRecs);

  for(i=0;i<nMaxRecs;i++) *(dBuffer+i)=CData_Dfetch(_this,i,nComp);

  /* HACK: This is not the number of actually fetched values!!! (??) */
  return nMaxRecs;
}

/**
 * Stores a numeric component
 *
 * @param _this        This instance
 * @param dBuffer      Vector address, must contain nMaxComps elements
 * @param nRec         Index of record to store
 * @param nMaxComps    Maximal number of components to store
 * @return Number of actually stored values
 */
INT32 CGEN_PUBLIC CData_CcompStore(CData* _this, COMPLEX64* dBuffer, INT32 nComp, INT32 nMaxRecs)
{
  INT32 i = 0;

  CHECK_THIS_RV(0);

  if(!dBuffer)                       return 0;
  if(nComp>=CData_GetNComps(_this))  return 0;
  if(nMaxRecs<=0)                    return 0;
  if(nComp<0)                        return 0;
  if(dlp_is_numeric_type_code(CData_GetCompType(_this,nComp))!=TRUE) return 0;

  nMaxRecs=((nMaxRecs>CData_GetMaxRecs(_this))?CData_GetMaxRecs(_this):nMaxRecs);

  for(i=0;i<nMaxRecs;i++) CData_Cstore(_this,dBuffer[i],i,nComp);

  /* HACK: This is not the number of actually stored values!!! (??) */
  return nMaxRecs;
}

/**
 * Stores the real part of a numeric component
 *
 * @param _this        This instance
 * @param dBuffer      Vector address, must contain nMaxComps elements
 * @param nRec         Index of record to store
 * @param nMaxComps    Maximal number of components to store
 * @return Number of actually stored values
 */
INT32 CGEN_PUBLIC CData_DcompStore(CData* _this, FLOAT64* dBuffer, INT32 nComp, INT32 nMaxRecs)
{
  INT32 i = 0;

  CHECK_THIS_RV(0);

  if(!dBuffer)                       return 0;
  if(nComp>=CData_GetNComps(_this))  return 0;
  if(nMaxRecs<=0)                    return 0;
  if(nComp<0)                        return 0;
  if(dlp_is_numeric_type_code(CData_GetCompType(_this,nComp))!=TRUE) return 0;

  nMaxRecs=((nMaxRecs>CData_GetMaxRecs(_this))?CData_GetMaxRecs(_this):nMaxRecs);

  for(i=0;i<nMaxRecs;i++) CData_Dstore(_this,dBuffer[i],i,nComp);

  /* HACK: This is not the number of actually stored values!!! (??) */
  return nMaxRecs;
}

/**
 * Fetches a numeric vector from numeric components
 *
 * @param _this        This instance
 * @param lpBuffer     Vector address, must contain nMaxComps elements
 * @param nRec         Index of record to fetch
 * @param nMaxComps    Maximal number of components to fetch
 * @param nCompIgnore  Index of component to ignore, use nCompIgnore=-1,
 *                     if you don't want to ignore a certain component
 * @return Number of components in vector
 */
INT32 CGEN_PUBLIC CData_CrecFetch
(
  CData*    _this,
  COMPLEX64* lpBuffer,
  INT32      nRec,
  INT32      nMaxComps,
  INT32      nCompIgnore
)
{
  INT32  i   = 0;
  INT32  j   = 0;

  CHECK_THIS_RV(0);
  if (_this->m_lpTable &&
      _this->m_lpTable->m_compDescrList &&
      (nRec>=0) &&
      (nRec<CData_GetNRecs(_this)) &&
      (nMaxComps>0)) {
    for (i=0,j=0; i<_this->m_lpTable->m_dim; i++) {
      if (j>=nMaxComps  ) break;
      if (i==nCompIgnore) continue;
      if (dlp_is_numeric_type_code(_this->m_lpTable->m_compDescrList[i].ctype)) {
        lpBuffer[j++] = CDlpTable_Cfetch(_this->m_lpTable,nRec,i);
      }
    }
  } else {
    dlp_memset(lpBuffer,0,nMaxComps*sizeof(COMPLEX64));
    return 0;
  }

  dlp_memset(lpBuffer+j, 0, (nMaxComps-j)*sizeof(COMPLEX64));
  return j;
}

/**
 * Fetches the real part a numeric vector from numeric components
 *
 * @param _this        This instance
 * @param lpBuffer     Vector address, must contain nMaxComps elements
 * @param nRec         Index of record to fetch
 * @param nMaxComps    Maximal number of components to fetch
 * @param nCompIgnore  Index of component to ignore, use nCompIgnore=-1,
 *                     if you don't want to ignore a certain component
 * @return Number of components in vector
 */
INT32 CGEN_PUBLIC CData_DrecFetch
(
  CData*  _this,
  FLOAT64* lpBuffer,
  INT32    nRec,
  INT32    nMaxComps,
  INT32    nCompIgnore
)
{
  INT32  i   = 0;
  INT32  j   = 0;

  CHECK_THIS_RV(0);
  if (_this->m_lpTable &&
      _this->m_lpTable->m_compDescrList &&
      (nRec>=0) &&
      (nRec<CData_GetNRecs(_this)) &&
      (nMaxComps>0)) {
    for (i=0,j=0; i<_this->m_lpTable->m_dim; i++) {
      if (j>=nMaxComps  ) break;
      if (i==nCompIgnore) continue;
      if (dlp_is_numeric_type_code(_this->m_lpTable->m_compDescrList[i].ctype)) {
        lpBuffer[j++] = CDlpTable_Dfetch(_this->m_lpTable,nRec,i);
      }
    }
  } else {
    dlp_memset(lpBuffer,0,nMaxComps*sizeof(FLOAT64));
    return 0;
  }

  dlp_memset(lpBuffer+j, 0, (nMaxComps-j)*sizeof(FLOAT64));
  return j;
}

/**
 * Fetches a interpolated numeric vector from numeric components
 * (Record index may be fractional)
 *
 * @param _this        This instance
 * @param dBuffer      Vector address, must contain nMaxComps elements
 * @param nRec         Index of record to fetch (may be fractional)
 * @param nMaxComps    Maximal number of components to fetch
 * @param nCompIgnore  Index of component to ignore, use nCompIgnore=-1,
 *                     if you don't want to ignore a certain component
 * @param nMode        Interpolation mode
 *                     0: linear interpolation
 *
 * @return O_K if sucessfull, NOT_EXEC otherwise.
 */
INT32 CGEN_PUBLIC CData_CrecFetchInterpol(CData* _this, COMPLEX64 *dBuffer, FLOAT64 nRec, INT32 nMaxComps, INT32 nCompIgnore, INT16 nMode)
{
  INT16     nType  = 0;
  INT32     i      = 0;
  INT32     j      = 0;
  INT32     nLRec  = 0;
  INT32     nComps = 0;
  COMPLEX64 nValue = CMPLX(0);
  COMPLEX64 nNext  = CMPLX(0);
  BYTE*     lpVal  = NULL;

  CHECK_THIS_RV(0);

  if(nMaxComps <= 0)               return O_K;
  if(CData_IsEmpty(_this) == TRUE) return O_K;
  if (nRec >= CData_GetNRecs(_this) || nRec<0)
  {
    dlp_memset(dBuffer,0,nMaxComps*sizeof(FLOAT64));
    return O_K;
  }

  nComps = CData_GetNComps(_this);
  nLRec  = (INT32)nRec;
  if((FLOAT64)nLRec == nRec)
    return CData_CrecFetch(_this,dBuffer,nLRec,nMaxComps,nCompIgnore);

    switch(nMode)
  {
  case 0:
    for (i=0; i < nComps; i++)
    {
      nType = CData_GetCompType(_this,i);

      if (nType > 256 && i != nCompIgnore && j < nMaxComps)
      {
        lpVal = CData_XAddr(_this,nLRec,i);
        if(lpVal) nValue = dlp_fetch(lpVal,nType);
        if ((FLOAT64)nLRec != nRec)
        {
          lpVal = CData_XAddr(_this,nLRec+1,i);
          if(lpVal) nNext = dlp_fetch(lpVal,nType);
          nValue = CMPLX_PLUS(nValue, CMPLX_MULT(CMPLX_MINUS(nNext,nValue),CMPLX(nRec-(FLOAT64)nLRec)));
        }
        dBuffer[j++] = nValue;
      }
    }
    break;
  default:
    DLPASSERT(FMSG("Unknow interpolation mode."));
  }

  return j;
}

/**
 * Stores a numeric vector in this instance
 *
 * @param _this        This instance
 * @param dBuffer      Vector address, must contain nMaxComps elements
 * @param nRec         Index of record to store
 * @param nMaxComps    Maximal number of components to store
 * @param nCompIgnore Index of component to ignore, use nCompIgnore=-1,
 *                    if you don't want to ignore a certain component
 * @return Number of actually stored values
 */
INT32 CGEN_PUBLIC CData_CrecStore
(
  CData*    _this,
  COMPLEX64* dBuffer,
  INT32      nRec,
  INT32      nMaxComps,
  INT32      nCompIgnore
)
{
  INT32 i       = 0;
  INT32 nResult = 0;

  CHECK_THIS_RV(0);
  if (!dBuffer)                                 return 0;
  if (nRec > CData_GetNRecs(_this) || nRec < 0) return 0;

  for (i=0; i<CData_GetNComps(_this); i++)
    if (CData_GetCompType(_this,i) > 256 && i != nCompIgnore && nResult < nMaxComps)
    {
      CData_Cstore(_this,dBuffer[nResult],nRec,i);
      nResult++;
    }

  return nResult;
}

/**
 * Stores a numeric vector in the real part of this instance
 *
 * @param _this        This instance
 * @param dBuffer      Vector address, must contain nMaxComps elements
 * @param nRec         Index of record to store
 * @param nMaxComps    Maximal number of components to store
 * @param nCompIgnore Index of component to ignore, use nCompIgnore=-1,
 *                    if you don't want to ignore a certain component
 * @return Number of actually stored values
 */
INT32 CGEN_PUBLIC CData_DrecStore
(
  CData*  _this,
  FLOAT64* dBuffer,
  INT32    nRec,
  INT32    nMaxComps,
  INT32    nCompIgnore
)
{
  INT32 i       = 0;
  INT32 nResult = 0;

  CHECK_THIS_RV(0);
  if (!dBuffer)                                 return 0;
  if (nRec > CData_GetNRecs(_this) || nRec < 0) return 0;

  for (i=0; i<CData_GetNComps(_this); i++)
    if (CData_GetCompType(_this,i) > 256 && i != nCompIgnore && nResult < nMaxComps)
    {
      CData_Dstore(_this,dBuffer[nResult],nRec,i);
      nResult++;
    }

  return nResult;
}

/**
 * Fetches a numeric block from a data instance.
 *
 * @param _this       This instance
 * @param dBuffer     Block buffer, must contain nDim * nBrec elements
 * @param nBlock      Block index
 * @param nMaxComps   Maximal number of components
 * @param nMaxBrecs   Maximal number of records per block
 * @param nCompIgnore Index of component to ignore, use nCompIgnore=-1,
 *                    if you don't want to ignore a certain component
 * @return Actual dimension (number of components) of block
 */
INT32 CGEN_PUBLIC CData_CblockFetch
(
  CData*    _this,
  COMPLEX64* dBuffer,
  INT32      nBlock,
  INT32      nMaxComps,
  INT32      nMaxBrecs,
  INT32      nCompIgnore
)
{
  INT32 i    = 0;
  INT32 nDim = 0;

  CHECK_THIS_RV(0);
  if (!dBuffer) return 0;

  if (nBlock<0 || nBlock>=CData_GetNBlocks(_this))
  {
    dlp_memset(dBuffer,0,nMaxComps*nMaxBrecs*sizeof(FLOAT64));
    return 0;
  }

  if (nMaxBrecs > CData_GetNRecsPerBlock(_this))
    nMaxBrecs = CData_GetNRecsPerBlock(_this);

  for (i=0; i < nMaxBrecs; i++)
    nDim = CData_CrecFetch(_this,&dBuffer[i*nMaxComps],i+nBlock*nMaxBrecs,nMaxComps,nCompIgnore);

  return nDim;
}

/**
 * Fetches the real part of a numeric block from a data instance.
 *
 * @param _this       This instance
 * @param dBuffer     Block buffer, must contain nDim * nBrec elements
 * @param nBlock      Block index
 * @param nMaxComps   Maximal number of components
 * @param nMaxBrecs   Maximal number of records per block
 * @param nCompIgnore Index of component to ignore, use nCompIgnore=-1,
 *                    if you don't want to ignore a certain component
 * @return Actual dimension (number of components) of block
 */
INT32 CGEN_PUBLIC CData_DblockFetch
(
  CData*  _this,
  FLOAT64* dBuffer,
  INT32    nBlock,
  INT32    nMaxComps,
  INT32    nMaxBrecs,
  INT32    nCompIgnore
)
{
  INT32 i    = 0;
  INT32 nDim = 0;

  CHECK_THIS_RV(0);
  if (!dBuffer) return 0;

  if (nBlock<0 || nBlock>=CData_GetNBlocks(_this))
  {
    dlp_memset(dBuffer,0,nMaxComps*nMaxBrecs*sizeof(FLOAT64));
    return 0;
  }

  if (nMaxBrecs > CData_GetNRecsPerBlock(_this))
    nMaxBrecs = CData_GetNRecsPerBlock(_this);

  for (i=0; i < nMaxBrecs; i++)
    nDim = CData_DrecFetch(_this,&dBuffer[i*nMaxComps],i+nBlock*nMaxBrecs,nMaxComps,nCompIgnore);

  return nDim;
}

/**
 * Stores a numeric block in a data instance.
 *
 * @param _this       This instance
 * @param dBuffer     Block buffer, must contain nDim * nBrec elements
 * @param nBlock      Block index
 * @param nMaxComps   Maximal number of components
 * @param nMaxBrecs   Maximal number of records per block
 * @param nCompIgnore Index of component to ignore, use nCompIgnore=-1,
 *                    if you don't want to ignore a certain component
 * @return Number of actually stores values
 */
INT32 CGEN_PUBLIC CData_CblockStore
(
  CData*    _this,
  COMPLEX64* dBuffer,
  INT32      nBlock,
  INT32      nMaxComps,
  INT32      nMaxBrecs,
  INT32      nCompIgnore
)
{
  INT32 i    = 0;
  INT32 nDim = 0;

  CHECK_THIS_RV(0);
  if (!dBuffer)                         return 0;
  if (nBlock > CData_GetNBlocks(_this)) return 0;

  if (nMaxBrecs > CData_GetNRecsPerBlock(_this))
    nMaxBrecs = CData_GetNRecsPerBlock(_this);

  for (i=0; i < nMaxBrecs; i++)
    nDim = CData_CrecStore(_this,&dBuffer[i*nMaxComps],i+nBlock*nMaxBrecs,nMaxComps,nCompIgnore);

  return nDim*nMaxBrecs;
}

/**
 * Stores a numeric block containing the real parts in a data instance.
 *
 * @param _this       This instance
 * @param dBuffer     Block buffer, must contain nDim * nBrec elements
 * @param nBlock      Block index
 * @param nMaxComps   Maximal number of components
 * @param nMaxBrecs   Maximal number of records per block
 * @param nCompIgnore Index of component to ignore, use nCompIgnore=-1,
 *                    if you don't want to ignore a certain component
 * @return Number of actually stores values
 */
INT32 CGEN_PUBLIC CData_DblockStore
(
  CData*  _this,
  FLOAT64* dBuffer,
  INT32    nBlock,
  INT32    nMaxComps,
  INT32    nMaxBrecs,
  INT32    nCompIgnore
)
{
  INT32 i    = 0;
  INT32 nDim = 0;

  CHECK_THIS_RV(0);
  if (!dBuffer)                         return 0;
  if (nBlock > CData_GetNBlocks(_this)) return 0;

  if (nMaxBrecs > CData_GetNRecsPerBlock(_this))
    nMaxBrecs = CData_GetNRecsPerBlock(_this);

  for (i=0; i < nMaxBrecs; i++)
    nDim = CData_DrecStore(_this,&dBuffer[i*nMaxComps],i+nBlock*nMaxBrecs,nMaxComps,nCompIgnore);

  return nDim*nMaxBrecs;
}

/**
 * Fetch cell nRec,nComp in all blocks
 *
 * @param _this       This instance
 * @param dBuffer     Destination buffer, must contain nMaxBlocks elements
 * @param nRec        Record index
 * @param nComp       Component index
 * @param nMaxBlocks  Maximal number blocks
 * @return Number of fetched cells
 */
INT32 CGEN_PUBLIC CData_DijFetch(CData* _this, FLOAT64* dBuffer, INT32 nRec, INT32 nComp, INT32 nMaxBlocks)
{
  INT16 nTypeCode = 0;
  INT32  i         = 0;
  INT32  nBlocks   = 0;
  INT32  nBlockLen = 0;
  BYTE* lpData    = NULL;

  CHECK_THIS_RV(0);

  if (CData_IsEmpty(_this) == TRUE) return 0;
  if (!dBuffer)                               return 0;
  if (nRec  >= CData_GetNRecsPerBlock(_this)) return 0;
  if (nComp >= CData_GetNComps(_this))        return 0;

  nTypeCode = CData_GetCompType(_this,nComp);
  if (dlp_is_numeric_type_code(nTypeCode)!=TRUE)    return 0;

  lpData    = CData_XAddr(_this,nRec,nComp);
  nBlockLen = CData_GetRecLen(_this) * CData_GetNRecsPerBlock(_this);
  nBlocks   = CData_GetNBlocks(_this);

  if (nBlocks > nMaxBlocks) nBlocks = nMaxBlocks;

  for(i=0; i<nBlocks; i++)
  {
    dBuffer[i] = dlp_fetch(lpData,nTypeCode).x;
    lpData += nBlockLen;
  }

  return nBlocks;
}

/*
 * Complex variant of CData_DijFetch
 */
INT32 CGEN_PUBLIC CData_CijFetch(CData* _this, COMPLEX64* dBuffer, INT32 nRec, INT32 nComp, INT32 nMaxBlocks)
{
  INT16  nTypeCode = 0;
  INT32  i         = 0;
  INT32  nBlocks   = 0;
  INT32  nBlockLen = 0;
  BYTE*  lpData    = NULL;

  CHECK_THIS_RV(0);

  if (CData_IsEmpty(_this) == TRUE) return 0;
  if (!dBuffer)                               return 0;
  if (nRec  >= CData_GetNRecsPerBlock(_this)) return 0;
  if (nComp >= CData_GetNComps(_this))        return 0;

  nTypeCode = CData_GetCompType(_this,nComp);
  if (dlp_is_numeric_type_code(nTypeCode)!=TRUE)    return 0;

  lpData    = CData_XAddr(_this,nRec,nComp);
  nBlockLen = CData_GetRecLen(_this) * CData_GetNRecsPerBlock(_this);
  nBlocks   = CData_GetNBlocks(_this);

  if (nBlocks > nMaxBlocks) nBlocks = nMaxBlocks;

  for(i=0; i<nBlocks; i++)
  {
    dBuffer[i] = dlp_fetch(lpData,nTypeCode);
    lpData += nBlockLen;
  }

  return nBlocks;
}

/**
 * Fetch interpolated value nRec,nComp (nRec may be fractional)
 *
 * @param _this      This instance
 * @param nRec       Record index (may be fractional)
 * @param nComp      Component index
 * @param nMode      Interpolation mode
 *                   0: linear interpolation
 * @return (Interpolated) value or 0.0 if not sucessfull
 */
COMPLEX64 CGEN_PUBLIC CData_CfetchInterpol(CData* _this, FLOAT64 nRec, INT32 nComp, INT16 nMode)
{
  INT16     nType  = 0;
  INT32     nLRec  = 0;
  COMPLEX64 nValue = CMPLX(0);
  COMPLEX64 nPrev  = CMPLX(0);
  COMPLEX64 nNext  = CMPLX(0);
  BYTE*     lpVal  = NULL;

  CHECK_THIS_RV(CMPLX(0));

  if(CData_IsEmpty(_this) == TRUE)     return CMPLX(0);
  if(nRec  >= CData_GetNRecs(_this))   return CMPLX(0);
  if(nComp >= CData_GetNComps(_this))  return CMPLX(0);
  nType = CData_GetCompType(_this,nComp);
  if(!dlp_is_numeric_type_code(nType)) return CMPLX(0);

  nLRec = (INT32)nRec;
  lpVal = CData_XAddr(_this,nLRec,nComp);
  nPrev = dlp_fetch(lpVal,nType);
  if ((FLOAT64)nLRec == nRec)
  {
    nValue = nPrev;
  }
  else
  {
    lpVal = CData_XAddr(_this,++nLRec,nComp);
    nNext = dlp_fetch(lpVal,nType);

    switch(nMode)
    {
    case 0:
      nValue.x  = nPrev.x + (nNext.x-nPrev.x)*(nRec-(FLOAT64)nLRec);
      nValue.y  = nPrev.y + (nNext.y-nPrev.y)*(nRec-(FLOAT64)nLRec);
      break;
    default:
      DLPASSERT(FMSG(Unknow interpolation mode.));
    }
  }

  return nValue;
}

/**
 *
 *
 */
INT16 CGEN_PUBLIC CData_SelectRecs(CData* _this, data* iSrc, INT32 from, INT32 count)
{
  /* Validate */
  if(!iSrc) return NOT_EXEC;
  CHECK_THIS_RV(NOT_EXEC);
  CHECK_DATA(_this);
  CHECK_DATA(iSrc);

  CREATEVIRTUAL(CData,iSrc,_this);

  if (from<0) from=0;
  if (count<0 || from+count>CData_GetNRecs(iSrc))
    count=CData_GetNRecs(iSrc)-from;
  if (count<0) count=0;

  /* Prepare destination instance */
  CData_Scopy(_this,iSrc);
  CData_AllocateUninitialized(_this,count);

    /* Move data */
  dlp_memmove
  (
      CData_XAddr(_this,0,0),
      CData_XAddr(iSrc,from,0),
      count*CData_GetRecLen(_this)
  );

  DESTROYVIRTUAL(iSrc,_this);

  return O_K;
}

/**
 *
 *
 */
INT16 CGEN_PUBLIC CData_SelectBlocks(CData* _this, data* iSrc, INT32 from, INT32 count)
{
  INT32  to;
  INT32  nRecsInFullBlocks = 0;
  INT16 retVal0 = O_K;
  INT16 retVal1 = O_K;
  INT16 retVal2 = O_K;

  CHECK_THIS_RV(NOT_EXEC);
  CHECK_DATA(_this);

  if(!iSrc) return NOT_EXEC;
  CHECK_DATA(iSrc);

  if(from<0) from=0;
  if(count<0 || from+count>CData_GetNBlocks(iSrc))
    count=CData_GetNBlocks(iSrc)-from;
  if(count<0) count=0;
  to=from+count;

  if(_this!=iSrc)
  {
    INT32 nRecPerBlock = 0;
    BYTE* lpSrc       = NULL;
    BYTE* lpDest      = NULL;

    nRecPerBlock = CData_GetNRecsPerBlock(iSrc);
    CData_Scopy(_this, iSrc);
    CData_AllocateUninitialized(_this,count*nRecPerBlock);

    lpSrc  = CData_XAddr(iSrc,from*nRecPerBlock,0);
    lpDest = CData_XAddr(_this,0,0);

    dlp_memmove(lpDest,lpSrc,count*nRecPerBlock*CData_GetRecLen(iSrc));
    CData_SetNBlocks(_this,count);

    DLP_CHECK_MEMINTEGRITY;
  }
  else
  {
    nRecsInFullBlocks = CData_GetNBlocks(_this)*CData_GetNRecsPerBlock(_this);

    retVal0 = CData_DeleteRecs(_this,nRecsInFullBlocks,CData_GetNRecs(_this)-nRecsInFullBlocks);
    retVal1 = CData_DeleteBlocks(_this,to,CData_GetNBlocks(_this)-to);
    retVal2 = CData_DeleteBlocks(_this,0,from);

    if (NOK(retVal0) || NOK(retVal1) || NOK(retVal2))
    {
      DLPASSERT(FALSE);
      return IERROR(_this,DATA_MTHINCOMPL,0,0,0);
    }
  }

  return O_K;
}

/**
 *
 *
 */
INT16 CGEN_PUBLIC CData_SelectComps(CData* _this, data* iSrc, INT32 from, INT32 count)
{
  INT32  to;
  INT16 retVal1 = O_K;
  INT16 retVal2 = O_K;

  CHECK_THIS_RV(NOT_EXEC);
  CHECK_DATA(_this);

  if(!iSrc) return NOT_EXEC;
  CHECK_DATA(iSrc);

  CData_Copy(BASEINST(_this), BASEINST(iSrc));

  if(from<0) from=0;
  if(from>CData_GetNComps(iSrc)) from=CData_GetNComps(iSrc);
  to=from+count;
  if (count<0 || to>CData_GetNComps(iSrc)) to=CData_GetNComps(iSrc);

  retVal1 = CData_DeleteComps(_this,to,CData_GetNComps(_this)-to);
  retVal2 = CData_DeleteComps(_this,0,from);

  if (NOK(retVal1) || NOK(retVal2)) return IERROR(_this,DATA_MTHINCOMPL,0,0,0);
  return O_K;
}

/**
 *
 *
 */
INT16 CGEN_PUBLIC CData_DeleteRecs(CData* _this, INT32 from, INT32 count)
{
  CHECK_THIS_RV(NOT_EXEC);

  return CDlpTable_DeleteRecs(_this->m_lpTable, _this->m_lpTable, from, count);
}

/**
 *
 *
 */
INT16 CGEN_PUBLIC CData_DeleteBlocks(CData* _this, INT32 from, INT32 count)
{
  INT32  nBLen  = 0;
  BYTE* lpSrc  = NULL;
  BYTE* lpDest = NULL;

  CHECK_THIS_RV(NOT_EXEC);

  if(from<0||count<0||from+count>CData_GetNBlocks(_this)) return NOT_EXEC;

  nBLen = CData_GetNRecsPerBlock(_this);

  lpDest = CData_XAddr(_this,from*nBLen,0);
  lpSrc  = CData_XAddr(_this,(from+count)*nBLen,0);

  dlp_memmove(lpDest,lpSrc,(CData_GetNRecs(_this)-(from+count)*nBLen)*CData_GetRecLen(_this));

  CData_SetNRecs(_this,CData_GetNRecs(_this)-(count*nBLen));
  CData_SetNBlocks(_this,CData_GetNBlocks(_this)-count);

  DLP_CHECK_MEMINTEGRITY;

  return O_K;
}

/**
 * Deletes components.
 *
 * @param _this  Pointer to this instance
 * @param nFirst First component to delete
 * @param nCount Number of components to delete
 * @return O_K if successfull, a (negative) error code otherwise
 */
INT16 CGEN_PUBLIC CData_DeleteComps(CData* _this, INT32 nFirst, INT32 nCount)
{
  CHECK_THIS_RV(NOT_EXEC);                                                      /* Check this pointer                */
  return CDlpTable_DeleteComps(_this->m_lpTable,_this->m_lpTable,nFirst,nCount);/* Call underlying table method      */
}

/**
 * Copies the marked or unmarked records or components. <code>_this</code> and <code>idSrc</code>
 * may be identical. Cell and block mark modes are <em>not</em> supported.
 *
 * @param _this     Pointer to destination instance
 * @param idSrc     Pointer to source instance
 * @param bPositive If <code>TRUE</code> copy marked elements, if <code>FALSE</code> copy
 *                  unmarked elements.
 * @return          <code>O_K</code> if successfull, a (negative) error code otherwise
 */
INT16 CGEN_PUBLIC CData_CopyMarked(CData* _this, CData* idSrc, BOOL bPositive)
{
  CData* idAux = NULL;
  INT32   nE    = 0;
  INT32   nXE   = 0;
  INT16  nErr  = O_K;

  if (_this->m_bMark && idSrc->m_markMap)
  {
    CREATEVIRTUAL(CData,idSrc,_this);
    ICREATE(CData,idAux,NULL);
    CData_Reset(BASEINST(_this),TRUE);

    switch(idSrc->m_markMode)
    {
    case CDATA_MARK_RECS:
      for (nE=0; nE<CData_GetNRecs(idSrc); )
        if (CData_RecIsMarked(idSrc,nE)==bPositive)
        {
          for (nXE=1; nE+nXE<CData_GetNRecs(idSrc); nXE++)
            if (CData_RecIsMarked(idSrc,nE+nXE)!=bPositive)
              break;

          CData_SelectRecs(idAux,idSrc,nE,nXE);
          CData_Cat(_this,idAux);

          nE+=nXE;
        }
        else nE++;
      break;
    case CDATA_MARK_COMPS:
      for (nE=0; nE<CData_GetNComps(idSrc); )
        if (CData_CompIsMarked(idSrc,nE)==bPositive)
        {
          for (nXE=1; nE+nXE<CData_GetNComps(idSrc); nXE++)
            if (CData_CompIsMarked(idSrc,nE+nXE)!=bPositive)
              break;

          CData_SelectComps(idAux,idSrc,nE,nXE);
          CData_Join(_this,idAux);

          nE+=nXE;
        }
        else nE++;
      break;
    case CDATA_MARK_BLOCKS: nErr=DATA_BADMARK; IERROR(_this,DATA_BADMARK,"blocks",0,0); break;
    case CDATA_MARK_CELLS : nErr=DATA_BADMARK; IERROR(_this,DATA_BADMARK,"cells" ,0,0); break;
    default               : DLPASSERT(FMSG("Unknown mark mode."));
    }

    DESTROYVIRTUAL(idSrc,_this);
    IDESTROY(idAux);
    CData_Unmark(_this);
  }
  else
  {
    /* /mark not set or no mark map present */
    if (!bPositive && _this!=idSrc)
      CData_Copy(BASEINST(_this),BASEINST(idSrc));
    if (bPositive)
      CData_Reset(BASEINST(_this),TRUE);
  }

  return nErr;
}

/**
 * Check type of components
 *
 * @param _this     This instance
 * @param nType     Type to check for
 * @return TRUE if all components are of type 'nType'
 *         FALSE if no components defined or one of them not
 *         of type 'nType'
 */
INT16 CGEN_PUBLIC CData_CheckCompType(CData* _this, INT16 nType)
{
  INT32 i = 0;

  CHECK_THIS_RV(NOT_EXEC);

  if (CData_GetNComps(_this) == 0) return FALSE;

  for (i=0;i<CData_GetNComps(_this); i++)
    if (CData_GetCompType(_this,i) != nType)
      return FALSE;

  return TRUE;
}

/**
 * Get data desriptor
 *
 * @param  _this  This instance
 * @param  nDescr Descriptor to fetch, one of the following constants:
 *                <code>DESCR0</code>, <code>DESCR1</code>,
 *                <code>DESCR2</code>, <code>DESCR3</code>,
 *                <code>DESCR4</code>, <code>FSR</code>, <code>ZF</code>,
 *                <code>OFS</code>.
 * @return Descriptor value or 0.0 if 'nDescr' is
 *         not a valid descriptor
 */
FLOAT64 CGEN_PUBLIC CData_GetDescr(CData* _this, INT16 nDescr)
{
  CHECK_THIS_RV(0.0);

  switch (nDescr)
  {
  case DESCR0 : return _this->m_lpTable->m_descr0;
  case DESCR1 : return _this->m_lpTable->m_descr1;
  case DESCR2 : return _this->m_lpTable->m_descr2;
  case DESCR3 : return _this->m_lpTable->m_descr3;
  case DESCR4 : return _this->m_lpTable->m_descr4;
  case RINC   : return _this->m_lpTable->m_fsr;
  case RWID   : return _this->m_lpTable->m_zf;
  case ROFS   : return _this->m_lpTable->m_ofs;
  default: return (0.0);
  }
}


/**
 * Set data desriptor
 *
 * @param _this   This instance
 * @param  nDescr Descriptor to set, one of the following constants:
 *                <code>DESCR0</code>, <code>DESCR1</code>,
 *                <code>DESCR2</code>, <code>DESCR3</code>,
 *                <code>DESCR4</code>, <code>FSR</code>, <code>ZF</code>,
 *                <code>OFS</code>.
 * @param nValue  New value for descriptor
 */
void CGEN_PUBLIC CData_SetDescr(CData* _this, INT16 nDescr, FLOAT64 nValue)
{
  if(!_this) return;

  switch (nDescr)
  {
  case DESCR0 : _this->m_lpTable->m_descr0 = nValue; break;
  case DESCR1 : _this->m_lpTable->m_descr1 = nValue; break;
  case DESCR2 : _this->m_lpTable->m_descr2 = nValue; break;
  case DESCR3 : _this->m_lpTable->m_descr3 = nValue; break;
  case DESCR4 : _this->m_lpTable->m_descr4 = nValue; break;
  case RINC   : _this->m_lpTable->m_fsr    = nValue; break;
  case RWID   : _this->m_lpTable->m_zf     = nValue; break;
  case ROFS   : _this->m_lpTable->m_ofs    = nValue; break;
  }
}


/**
 * extracts an indexlist from the data object if preset
 *
 * _this - result: indexlist if present
 * iSrc     - data, may contain an index or label  component
 * iTab   - label-reference table or index list
 * iLTab  - label table container
 * nIdx    - index component
 */
INT32 CGEN_PUBLIC CData_GenIndexList (data *_this, data *iSrc, data *iTab, data *iLTab, INT32 nIdx)
{
  INT32 i,T,Tl;

  CHECK_THIS_RV(NOT_EXEC);

  if (CData_IsEmpty(iSrc)==TRUE) return(NOT_EXEC);
  T = CData_GetNRecs(iSrc);

  /* --- index component present */

  if (nIdx > -1 )
  {
    CData_Reset(BASEINST(_this),TRUE);
    CData_AddComp(_this,"indx",T_LONG);
    CData_AllocateUninitialized(_this, T);
    for (i=0; i<T;i++)
      CData_Dstore(_this,CData_Dfetch(iSrc,i,nIdx),i,0);
    return O_K;
  }

    if (CData_IsEmpty(iTab) == TRUE)
  {
    CData_Reset(BASEINST(_this),TRUE);
    return O_K;
  }

  if (CData_GetCompType(iTab,0) <= 256)
  {
    for (i=0; i<CData_GetNComps(iSrc); i++)
    {
      if (dlp_is_symbolic_type_code(CData_GetCompType(iSrc,i)) == TRUE)
      {
/*    	  j=i;*/
/*        CData_CorrectPhdLabel(iSrc,j);*/
      }
    }
    CData_GenLabIndex (iSrc, NULL, _this, iTab);
    if (iLTab != NULL)
      CData_SelectComps(iLTab,iTab,0,1);
    return(O_K);
  }


  if (_this == NULL) return(NOT_EXEC);
  CData_Reset(BASEINST(_this),TRUE);
  CData_AddComp(_this,"indx",T_LONG);
  CData_AllocateUninitialized(_this, T);
  CData_Fill(_this,CMPLX(-1),CMPLX(0)),
    Tl = CData_GetNRecs(iTab);
  if (Tl > T) Tl=T;
  for (i=0; i<Tl;i++)
    CData_Dstore(_this,CData_Dfetch(iTab,i,0),i,0);

  if (CData_GetNRecs(_this)<T) return IERROR(_this,DATA_INTERNAL,T,0,0);
  return O_K;
}

/**
 *  select label from x-data and convert to index, if table tab given
 *
 *  _this  - input data containing label component
 *  iLabel - output label data
 *  iIndex - output index data      (may be NULL)
 *  iLTab  - label lookup table     (may be NULL)
 *
 *  remarks:
 *  - only the first label component is selected
 *  - if iLTab is defined and iIndex is not NULL, the label are
 *    coded to index sequence
 *  - if iLabel is NULL, no labels are generated
 *
 *  returns: O_K      - if at least label or index generated
 *      EMPTY    - if x is NULL or empty
 *           NOT_EXEC - if nothing is generated
 */
INT16 CGEN_PUBLIC CData_GenLabIndex(CData* _this, CData* iLabel, CData* iIndex, CData* iLTab)
{
  INT16  ierr=O_K;
  INT32    cl=0;
  INT32    i,j=-1;

  if (CData_IsEmpty(_this))
    return IERROR(_this,DATA_EMPTY,"_this in CData_GenLabIndex",0,0);

  for (i=0; i<CData_GetNComps(_this); i++)
  {
    if (dlp_is_symbolic_type_code(CData_GetCompType(_this,i)) == TRUE)
    {
      j=i;
      break;
    }
  }
  if (j == -1) return (NOT_EXEC);

  if (iLabel != NULL)
  {
    CData_Reset(BASEINST(iLabel),TRUE);
    CData_AddComp(iLabel,CData_GetCname(_this,j),CData_GetCompType(_this,j));
    ierr=CData_AllocateUninitialized(iLabel,CData_GetNRecs(_this));
    cl = CData_GetCompSize(_this,j);

    for (i=0; i<CData_GetNRecs(_this); i++)
      dlp_memmove(CData_XAddr(iLabel,i,0),CData_XAddr(_this,i,j),cl);
  }

  if (iIndex != NULL && iLTab != NULL && ierr == O_K)
    ierr=CData_GenIndex(iIndex,_this,iLTab,j,0);

  return (ierr);
}

/**
 *  copies n components from iSrc to y, starting with i;
 *  the corresponding components in y must be present !
 *
 *  _this  - target object
 *  iSrc   - source object
 *  is     - first source component
 *  it     - first target component
 *  n      - number of components to select
 */
INT16 CGEN_PUBLIC CData_CopyComps(CData* _this, CData *iSrc, INT32 is, INT32 it, INT32 n)
{
  INT32  i,j,k,n1;

  CHECK_THIS_RV(NOT_EXEC);

  if (is < 0 || is >= CData_GetNComps(iSrc))
    return IERROR(_this,DATA_BADCOMP,(int)is,BASEINST(iSrc)->m_lpInstanceName,0);
  if (iSrc == _this) return O_K;

  if (CData_GetNRecs(_this) == 0)
    if (CData_AllocateUninitialized(_this, CData_GetNRecs(iSrc)) != O_K) return NOT_EXEC;

  n1 = n;
  if (is+n > CData_GetNComps(iSrc)) n1 = CData_GetNComps(iSrc) - is;
  k = it;

  for (j=is; j<is+n1; j++)
  {
    if ( k > CData_GetNComps(_this)) break;
    if (dlp_is_numeric_type_code(CData_GetCompType(iSrc,j)) == TRUE)
    {
      for (i=0; i<CData_GetNRecs(_this); i++) CData_Dstore(_this,CData_Dfetch(iSrc,i,j),i,k);
    }
    if (dlp_is_symbolic_type_code(CData_GetCompType(iSrc,j)) == TRUE)
    {
      for (i=0; i<CData_GetNRecs(_this); i++) CData_Sstore (_this,(char*)CData_XAddr(iSrc,i,j),i,k);
    }
    k++;
  }

  CData_CopyDescr(_this,iSrc);

  return (O_K);
}


/**
 *  Resample vector sequence.
 *
 * @param _this        target object
 * @param iSrc         source object
 * @param nRate        target rate
 * @param nCompIgnore  Index of component to ignore, use nCompIgnore=-1,
 *                     if you don't want to ignore a certain component
 * @param nMode        Interpolation mode
 *                     0: linear interpolation
 * @return O_K if sucessfull, not exec otherwise
 */
INT16 CGEN_PUBLIC CData_ResampleInt(CData* _this, CData *iSrc, FLOAT64 nRate, INT16 nMode)
{
  INT16      bSymb  = FALSE;
  INT32      i      = 0;
  INT32      j      = 0;
  INT32      nComps = 0;
  INT32      nDim   = 0;
  FLOAT64    nIdx   = 0;
  FLOAT64    nRecs  = 0.;
  COMPLEX64* lpRec  = NULL;

  CHECK_THIS_RV(NOT_EXEC);
  if(CData_IsEmpty(iSrc) == TRUE)    return NOT_EXEC;
  if(nRate <= 0.)                    return NOT_EXEC;
  if(nRate == 1.) return (CData_Copy(BASEINST(_this),BASEINST(iSrc)));

  CREATEVIRTUAL(CData,iSrc,_this);

  nRate  = 1/nRate;
  nComps = CData_GetNComps(iSrc);
  nRecs  = (FLOAT64)CData_GetNRecs(iSrc);

  /* Check for symbolic components */
  for(i=0; i<nComps; i++)
  {
    if(dlp_is_symbolic_type_code(CData_GetCompType(iSrc,i)))
    {
      bSymb = TRUE;
    }
  }

  CData_Scopy(_this,iSrc);
  CData_Allocate(_this,(INT32)(nRecs/nRate+1.));

  lpRec = (COMPLEX64*)dlp_malloc(nComps*sizeof(COMPLEX64));
  if (lpRec == NULL)
  {
    DESTROYVIRTUAL(iSrc,_this);
    return NOT_EXEC;
  }

  nDim = CData_CrecFetch(iSrc,lpRec,0,nComps,-1);

  if(bSymb == FALSE)
  {
    do {
      CData_CrecFetchInterpol(iSrc,lpRec,nIdx,nDim,-1,nMode);
      CData_CrecStore(_this,lpRec,j,nDim,-1);
      j++;
      nIdx += nRate;
    } while(nIdx<nRecs);
  }
  else
  {
    do {
      CData_CrecFetchInterpol(iSrc,lpRec,nIdx,nDim,-1,nMode);
      CData_CrecStore(_this,lpRec,j,nDim,-1);
      for (i=0; i<nComps; i++)
        if (dlp_is_symbolic_type_code(CData_GetCompType(iSrc,i))==TRUE)
          CData_Sstore(_this,(char*)CData_XAddr(iSrc,(INT32)nIdx,i),j,i);
      nIdx += nRate;
      j++;
    } while(nIdx<nRecs);
  }

  CData_CopyDescr(_this,iSrc);
  CData_SetDescr(_this,RINC,CData_GetDescr(iSrc,RINC)*nRate);
  CData_SetNRecs(_this,j);

  /* clean up */
  DESTROYVIRTUAL(iSrc,_this);
  dlp_free(lpRec);

  return(O_K);
}

/* EOF */

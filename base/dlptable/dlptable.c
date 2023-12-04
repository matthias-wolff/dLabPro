/* dLabPro data table library
 * - Implementation file
 *
 * AUTHOR : Matthias Eichner
 * PACKAGE: dLabPro/base
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

#include "dlp_cscope.h"
#include "dlp_kernel.h"
#include "dlp_table.h"

#define IFCHECK if(0)

/**
 * Create CDlpTable instance
 *
 * @return pointer to new instance
 */
CDlpTable* CDlpTable_CreateInstance(void)
{
  CDlpTable* lpiNewTable = NULL;

  lpiNewTable = (CDlpTable*)dlp_calloc(1,sizeof(CDlpTable));

  if(CDlpTable_Constructor(lpiNewTable) == NOT_EXEC)
  {
    CDlpTable_Destructor(lpiNewTable);
    return NULL;
  }

  return lpiNewTable;
}

/**
 * Destroy CDlpTable instance
 *
 * @param _this pointer to instance of CDlpTable to destroy
 * @return O_K if successful NOT_EXEC otherwise
 */
INT16 CDlpTable_DestroyInstance(CDlpTable* _this)
{
  if(!_this) return NOT_EXEC;

  CDlpTable_Destructor(_this);
  dlp_free(_this);
  _this = NULL;

  return O_K;
}

/**
 * Constructor of class CDlpTable
 *
 * @param _this pointer to CDlpTable instance to construct (this)
 * @return O_K if successful NOT_EXEC otherwise
 */
INT16 CDlpTable_Constructor(CDlpTable* _this)
{
  if(!_this) return NOT_EXEC;

  /* Create component descriptor list */
  _this->m_compDescrList = (SDlpTableComp*)dlp_calloc(COMP_ALLOC,sizeof(SDlpTableComp));
  if (NULL == _this->m_compDescrList) return NOT_EXEC;

  /* Initialize fields */
  _this->m_dim            = 0;
  _this->m_maxdim         = COMP_ALLOC;
  _this->m_nrec           = 0;
  _this->m_maxrec         = 0;
  _this->m_reclen         = 0;
  _this->m_descr0         = 0;
  _this->m_descr1         = 0;
  _this->m_descr2         = 0;
  _this->m_descr3         = 0;
  _this->m_descr4         = 0;
  _this->m_fsr            = 0;
  _this->m_zf             = 0;
  _this->m_ofs            = 0;
  _this->m_rtext          = NULL;
  _this->m_vrtext         = NULL;
  _this->m_theDataPointer = NULL;

  return O_K;
}

/**
 * Destructor of class CDlpTable
 *
 * @param  _this pointer to CDlpTable instance to destruct (this)
 * @return O_K if successful NOT_EXEC otherwise
 */
INT16 CDlpTable_Destructor(CDlpTable* _this)
{
  if(!_this) return NOT_EXEC;

  CDlpTable_Reset(_this);
  dlp_free(_this->m_compDescrList);

  return O_K;
}

/**
 * Initialization of class CDlpTable
 *
 * @param _this pointer to CDlpTable instance to initialize (this)
 * @return O_K if successful NOT_EXEC otherwise
 */
INT16 CDlpTable_Init(CDlpTable* _this)
{
  return (NULL==_this)?NOT_EXEC:CDlpTable_Reset(_this);
}

/**
 * Reset the data content (records and component structure) of a CDlpTable instance.
 * Does NOT reset user fields: m_rtext, m_vrtext, m_descr0, m_descr1, m_descr2,
 * m_descr3, m_descr4, m_fsr, m_zf and m_ofs.
 *
 * @param _this pointer to CDlpTable instance to reset (this)
 * @return O_K if successful NOT_EXEC otherwise
 */
INT16 CDlpTable_SReset(CDlpTable* _this)
{
  if(!_this) return NOT_EXEC;

  /* Reset data content */
  if(_this->m_theDataPointer)
  {
    dlp_free(_this->m_theDataPointer);
    _this->m_theDataPointer = NULL;
  }

  if(_this->m_compDescrList)
  {
    dlp_free(_this->m_compDescrList);
    _this->m_compDescrList = NULL;
  }

  _this->m_compDescrList  = (SDlpTableComp*)dlp_calloc(COMP_ALLOC,sizeof(SDlpTableComp));

  _this->m_dim            = 0;
  _this->m_maxdim         = COMP_ALLOC;
  _this->m_nrec           = 0;
  _this->m_maxrec         = 0;
  _this->m_reclen         = 0;

  return O_K;
}

/**
 * Reset of class CDlpTable
 *
 * @param _this pointer to CDlpTable instance to reset (this)
 * @return O_K if successful NOT_EXEC otherwise
 */
INT16 CDlpTable_Reset(CDlpTable* _this)
{
  if(!_this) return NOT_EXEC;

  /* Reset data content */
  CDlpTable_SReset(_this);

  /* Reset fields */
  if(_this->m_rtext)
  {
    dlp_free(_this->m_rtext);
    _this->m_rtext = NULL;
  }

  if(_this->m_vrtext)
  {
    dlp_free(_this->m_vrtext);
    _this->m_vrtext = NULL;
  }

  _this->m_descr0         = 0;
  _this->m_descr1         = 0;
  _this->m_descr2         = 0;
  _this->m_descr3         = 0;
  _this->m_descr4         = 0;
  _this->m_fsr            = 0;
  _this->m_zf             = 0;
  _this->m_ofs            = 0;

  return O_K;
}

/**
 * Copy table (structure and data)
 *
 * @param _this     pointer to destination instance (this)
 * @param lpiSrc    pointer to source instance
 * @param nFirstRec Record to start copying
 * @param nRecs     Number of records to copy
 * @return O_K if successful NOT_EXEC otherwise
 */
INT16 CDlpTable_Copy(CDlpTable* _this, CDlpTable* lpiSrc, INT32 nFirstRec, INT32 nRecs)
{
  if(!_this                              ) return NOT_EXEC;
  if(!lpiSrc                             ) return NOT_EXEC;
  if(!lpiSrc->m_compDescrList            ) return NOT_EXEC;
  if(_this==lpiSrc                       ) return O_K;
  if(CDlpTable_Reset(_this)         !=O_K) return NOT_EXEC;
  if(CDlpTable_Scopy(_this,lpiSrc)  !=O_K) return NOT_EXEC;
  if(CDlpTable_Allocate(_this,nRecs)!=O_K) return NOT_EXEC;

  dlp_memmove(_this->m_theDataPointer,
              lpiSrc->m_theDataPointer + ((INT64)_this->m_reclen * (INT64)nFirstRec),
              (INT64)nRecs * (INT64)_this->m_reclen);

  if((nFirstRec + nRecs) > CDlpTable_GetNRecs(lpiSrc))
    CDlpTable_SetNRecs(_this,CDlpTable_GetNRecs(lpiSrc) - nFirstRec);

  /* Copy description fields */
  _this->m_descr0 = lpiSrc->m_descr0;
  _this->m_descr1 = lpiSrc->m_descr1;
  _this->m_descr2 = lpiSrc->m_descr2;
  _this->m_descr3 = lpiSrc->m_descr3;
  _this->m_descr4 = lpiSrc->m_descr4;
  _this->m_fsr    = lpiSrc->m_fsr;
  _this->m_zf     = lpiSrc->m_zf;
  _this->m_ofs    = lpiSrc->m_ofs;
  if (dlp_strlen(lpiSrc->m_rtext))
  {
    _this->m_rtext = (char*)dlp_malloc((dlp_strlen(lpiSrc->m_rtext)+1)*sizeof(char));
    dlp_strcpy(_this->m_rtext,lpiSrc->m_rtext);
  }
  if (dlp_strlen(lpiSrc->m_vrtext))
  {
    _this->m_vrtext = (char*)dlp_malloc((dlp_strlen(lpiSrc->m_vrtext)+1)*sizeof(char));
    dlp_strcpy(_this->m_vrtext,lpiSrc->m_vrtext);
  }

  return O_K;
}

/**
 * Insert table components.
 *
 * @param _this     Pointer to CDlpTable instance (this)
 * @param lpsName   Pointer to string containing name of new components
 * @param nType     Type of new components
 * @param nInsertAt Component index of the component to insert
 * @param nComps    Number of components to insert
 * @return O_K if successful NOT_EXEC otherwise
 */
INT16 CDlpTable_InsertNcomps(CDlpTable* _this, const char* lpsName, INT16 nType, INT32 nInsertAt, INT32 nComps)
{
  INT32          i         = 0;
  INT32          nNRecs    = 0;
  INT64          nSize1    = 0;
  INT64          nSize2    = 0;
  INT32          nReclen   = 0;
  INT32          nOldRln   = 0;
  INT32          nOffset   = 0;
  BYTE*          lpNewData = NULL;
  BYTE*          lpData    = _this->m_theDataPointer;
  SDlpTableComp* newComp   = NULL;

  /* Verify */
  if (!_this->m_compDescrList) return NOT_EXEC;
  if (!lpsName               ) return NOT_EXEC;
  if (nComps <= 0            ) return O_K;

  /* Save old record size */
  nOldRln = _this->m_reclen;

  /* Need more memory? */
  if (_this->m_dim+nComps >= _this->m_maxdim)
  {
    _this->m_compDescrList = (SDlpTableComp*)dlp_realloc(_this->m_compDescrList,_this->m_maxdim+nComps+COMP_ALLOC,sizeof(SDlpTableComp));
    if (!_this->m_compDescrList) return NOT_EXEC;
    _this->m_maxdim=_this->m_maxdim+nComps+COMP_ALLOC;
  }

  /* Adjust descriptions */
  if (nInsertAt>=0 && nInsertAt<_this->m_dim)
  {
    /* insert */
    dlp_memmove
    (
      &_this->m_compDescrList[nInsertAt+nComps],
      &_this->m_compDescrList[nInsertAt],
      (_this->m_dim-nInsertAt)*sizeof(SDlpTableComp)
    );
  }
  /* Append components if nInsertAt is greater than dimension or less than zero */
  else nInsertAt = _this->m_dim;

  for(i=nInsertAt; i<nInsertAt+nComps; i++)
  {
    /* Initialize new items */
    newComp = &_this->m_compDescrList[i];
    dlp_strncpy(newComp->lpName,lpsName,COMP_DESCR_LEN);
    newComp->ctype = nType;
    newComp->size  = dlp_get_type_size(nType);
  }

  /* Adjust dimension */
  _this->m_dim += nComps;

  /* Re-calculate all offsets */
  for (i=0,nOffset=0; i<_this->m_dim; i++)
  {
    _this->m_compDescrList[i].offset = nOffset;
    nOffset+=_this->m_compDescrList[i].size;
  }

  _this->m_reclen = CDlpTable_GetRecLen(_this);
  nReclen         = _this->m_reclen;
  nNRecs          = _this->m_nrec;

  /* Adjust data block */
  if(lpData)
  {
#ifndef __NOXALLOC
    DLPASSERT(dlp_size(lpData) && _this->m_reclen!=0 && _this->m_maxrec!=0);
#endif
    /* Assertion: There is a data pointer, but no data :( */

    lpNewData = (BYTE*)dlp_calloc((size_t)_this->m_maxrec*_this->m_reclen,sizeof(BYTE));
    if (!lpNewData) return NOT_EXEC;

    for(i=0               ; i<nInsertAt   ; i++) nSize1 += CDlpTable_GetCompSize(_this,i);
    for(i=nInsertAt+nComps; i<_this->m_dim; i++) nSize2 += CDlpTable_GetCompSize(_this,i);

    dlp_memmove(lpNewData,lpData,nSize1);

    for(i=1;i<_this->m_maxrec;i++)
    {
      dlp_memmove(lpNewData+(size_t)i*nReclen-nSize2,lpData+(size_t)i*nOldRln-nSize2,nSize1+nSize2);
    }
    dlp_memmove(lpNewData+(size_t)nNRecs*nReclen-nSize2,lpData+(size_t)nNRecs*nOldRln-nSize2,nSize2);

    dlp_free(lpData);
    _this->m_theDataPointer=lpNewData;
  }

  DLP_CHECK_MEMINTEGRITY;
  return O_K;
}

/**
 * Inserts one table component.
 *
 * @param _this     Pointer to CDlpTable instance (this)
 * @param lpsName   Pointer to string containing name of new component
 * @param nType     Type of new component
 * @param nInsertAt Component index of the component to insert
 * @return O_K if successful NOT_EXEC otherwise
 */
INT16 CDlpTable_InsertComp(CDlpTable* _this, const char* lpsName, INT16 nType, INT32 nInsertAt)
{
  return CDlpTable_InsertNcomps(_this,lpsName,nType,nInsertAt,1);
}

/**
 * Appends table components.
 *
 * @param _this  Pointer to CDlpTable instance (this)
 * @param nType  Type of new components
 * @param nComps Number of components to append
 * @return O_K if successful NOT_EXEC otherwise
 */
INT16 CDlpTable_AddNcomps(CDlpTable* _this, INT16 nType, INT32 nComps)
{
  return CDlpTable_InsertNcomps(_this,"",nType,-1,nComps);
}

/**
 * Appends one table component.
 *
 * @param _this   Pointer to CDlpTable instance (this)
 * @param lpsName Pointer to string containing name of new component
 * @param nType   Type of new component, one of the T_XXX constans or 1 through
 *                <code>L_SSTR</code> (=256) for character arrays (pointer and
 *                instance types are not allowed!)
 * @return <code>O_K</code> if successful, <code>NOT_EXEC</code> otherwise
 */
INT16 CDlpTable_AddComp(CDlpTable* _this, const char* lpsName, INT16 nType)
{
  return (NULL==_this->m_compDescrList)?NOT_EXEC:CDlpTable_InsertComp(_this,lpsName,nType,-1);
}

/**
 * Copy table structure (number and type of components, but no data)
 *
 * @param _this  Pointer to destination instance (this)
 * @param lpiSrc Pointer to source instance
 * @return O_K if successful NOT_EXEC otherwise
 */
INT16 CDlpTable_Scopy(CDlpTable* _this, CDlpTable* lpiSrc)
{
  if(!_this)                      return NOT_EXEC;
  if(!lpiSrc)                     return NOT_EXEC;
  if(!lpiSrc->m_compDescrList)    return NOT_EXEC;
  if(CDlpTable_Reset(_this)!=O_K) return NOT_EXEC;
  if(_this==lpiSrc)               return O_K;

  dlp_free(_this->m_compDescrList);
  _this->m_compDescrList = (SDlpTableComp*)dlp_malloc(lpiSrc->m_maxdim*sizeof(SDlpTableComp));
  if (!_this->m_compDescrList) return NOT_EXEC;

  dlp_memmove(_this->m_compDescrList,lpiSrc->m_compDescrList,lpiSrc->m_maxdim*sizeof(SDlpTableComp));

  _this->m_dim    = lpiSrc->m_dim;
  _this->m_maxdim = lpiSrc->m_maxdim;
  _this->m_reclen = lpiSrc->m_reclen;

  if((_this->m_dim != lpiSrc->m_dim)||(_this->m_reclen != lpiSrc->m_reclen))
  {
    CDlpTable_Reset(_this);
    return NOT_EXEC;
  }
  return O_K;
}

/**
 * Copy table description fields. Structure and data will remain unchanged.
 *
 * @param _this  Pointer to destination instance (this)
 * @param lpiSrc Pointer to source instance
 * @return O_K if successful NOT_EXEC otherwise
 */
INT16 CDlpTable_Dcopy(CDlpTable* _this, CDlpTable* lpiSrc)
{
  if(!_this       ) return NOT_EXEC;
  if(!lpiSrc      ) return NOT_EXEC;
  if(_this==lpiSrc) return O_K;

  _this->m_descr0 = lpiSrc->m_descr0;
  _this->m_descr1 = lpiSrc->m_descr1;
  _this->m_descr2 = lpiSrc->m_descr2;
  _this->m_descr3 = lpiSrc->m_descr3;
  _this->m_descr4 = lpiSrc->m_descr4;
  _this->m_fsr    = lpiSrc->m_fsr;
  _this->m_zf     = lpiSrc->m_zf;
  _this->m_ofs    = lpiSrc->m_ofs;

  if (_this->m_rtext ) dlp_free(_this->m_rtext );
  if (lpiSrc->m_rtext)
  {
  	_this->m_rtext = (char*)dlp_calloc(dlp_strlen(lpiSrc->m_rtext)+1,sizeof(char));
  	dlp_strcpy(_this->m_rtext,lpiSrc->m_rtext);
  }
  if (_this->m_vrtext) dlp_free(_this->m_vrtext);
  if (lpiSrc->m_vrtext)
  {
  	_this->m_vrtext=(char*)dlp_calloc(dlp_strlen(lpiSrc->m_vrtext)+1,sizeof(char));
  	dlp_strcpy(_this->m_vrtext,lpiSrc->m_vrtext);
  }

  return O_K;
}

/**
 * Allocates memory for the given number of records memory. The method
 * preserves the component structure but destroys the data content of
 * the table. The number of valid records will be set to nRecs. The
 * capacity of the table (the maximal number of records) will be nRecs.
 *
 * @param _this Pointer to CDlpTable instance
 * @param nRecs The number of records to allocate
 * @return O_K if successfull, a negative error code otherwise
 */
INT16 CDlpTable_AllocateUninitialized(CDlpTable* _this, INT32 nRecs)
{
  if(!_this->m_compDescrList) return NOT_EXEC;
  if(nRecs<0) nRecs = 0;

  if(_this->m_theDataPointer)
  {
    dlp_free(_this->m_theDataPointer);
    _this->m_theDataPointer = NULL;
  }

  if(nRecs>0)
  {
    _this->m_reclen = CDlpTable_GetRecLen(_this);

    if(!_this->m_reclen) return NOT_EXEC;

    _this->m_theDataPointer = (BYTE*)dlp_malloc((size_t)nRecs*(size_t)_this->m_reclen);

    if(!_this->m_theDataPointer) return NOT_EXEC;
  }

  _this->m_nrec   = nRecs;
  _this->m_maxrec = nRecs;

  return O_K;
}

/**
 * Allocates memory for the given number of records and zero-initializes the
 * memory. The method preserves the component structure but destroys the data
 * content of the table. The number of valid records will be set to nRecs. The
 * capacity of the table (the maximal number of records) will be nRecs.
 *
 * @param _this Pointer to CDlpTable instance
 * @param nRecs The number of records to allocate
 * @return O_K if successfull, a negative error code otherwise
 */
INT16 CDlpTable_Allocate(CDlpTable* _this, INT32 nRecs)
{
  INT16 nRetval;

  nRetval = CDlpTable_AllocateUninitialized(_this, nRecs);

#ifdef __OPTIMIZE_ALLOC
  if(nRetval == O_K) {
    dlp_memset(_this->m_theDataPointer, 0L, nRecs*(size_t)_this->m_reclen);
  }
#endif

  return nRetval;
}

/**
 * Allocates memory for the given number of records. The method preserves
 * the component structure but destroys the data content of the table. The
 * number of valid records will be set to zero. The capacity of the table
 * (the maximal number of records) will be nRecs.
 *
 * @param _this Pointer to CDlpTable instance
 * @param nRecs The number of records to allocate
 * @return O_K if successfull, a negative error code otherwise
 */
INT16 CDlpTable_AllocUninitialized(CDlpTable* _this, INT32 nRecs)
{
  INT16 nRetval;

  if(!_this                 ) return NOT_EXEC;
  if(!_this->m_compDescrList) return NOT_EXEC;
  if(nRecs<0                ) nRecs=0;

  nRetval = CDlpTable_AllocateUninitialized(_this,nRecs);
  CDlpTable_SetNRecs(_this,0);

  return nRetval;
}

/**
 * Allocates memory for the given number of records and zero-initializes the
 * memory. The method preserves the component structure but destroys the data
 * content of the table. The number of valid records will be set to zero. The
 * capacity of the table (the maximal number of records) will be nRecs.
 *
 * @param _this Pointer to CDlpTable instance
 * @param nRecs The number of records to allocate
 * @return O_K if successfull, a negative error code otherwise
 */
INT16 CDlpTable_Alloc(CDlpTable* _this, INT32 nRecs)
{
  INT16 nRetval;

  nRetval = CDlpTable_AllocUninitialized(_this,nRecs);
  if(nRetval == O_K) {
    dlp_memset(_this->m_theDataPointer, 0L, (size_t)nRecs*_this->m_reclen*sizeof(BYTE));
  }

  return nRetval;
}

/**
 * Increases or decreases the capacity of the table to the given number of
 * records. The method preserves the structure and the data content of the
 * table. However, if nRecs is smaller than the number of valid records before
 * calling CDlpTable_Realloc, the data content will be truncated. Sets the
 * number of valid records to nRec.
 *
 * @param _this Pointer to CDlpTable instance
 * @param nRecs The number of records to allocate
 * @return O_K if successfull, a negative error code otherwise
 */
INT16 CDlpTable_Realloc(CDlpTable* _this, INT32 nRecs)
{
  BYTE* lpNewDataPtr = NULL;

  if(!_this                  ) return NOT_EXEC;
  if(!_this->m_compDescrList ) return NOT_EXEC;
  if(nRecs == 0              ) return CDlpTable_Alloc(_this,nRecs);
  if(!_this->m_theDataPointer) return CDlpTable_Alloc(_this,nRecs);
  if(nRecs < 0               ) return NOT_EXEC;
  if(nRecs == _this->m_nrec  ) return O_K;
  if(_this->m_reclen == 0    ) return NOT_EXEC;

  lpNewDataPtr = (BYTE*)dlp_realloc(_this->m_theDataPointer,nRecs,_this->m_reclen);
  if(!lpNewDataPtr) return NOT_EXEC;

  _this->m_maxrec = nRecs;
  if(_this->m_nrec>nRecs) _this->m_nrec = nRecs;
  _this->m_theDataPointer = lpNewDataPtr;

  /* Check size of reallocate memory */
#ifndef __NOXALLOC
  DLPASSERT((INT64)dlp_size(_this->m_theDataPointer)==(INT64)(CDlpTable_GetMaxRecs(_this)*(size_t)CDlpTable_GetRecLen(_this)));
#endif

  return O_K;
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
 * @param _this    _this pointer to CDlpTable instance
 * @param nRecs    Number of records to append
 * @param nRealloc Capacity increment on reallocation (at least nRecs)
 * @return The index of the first new record or -1 if no more memory
 */
INT32 CDlpTable_AddRecs(CDlpTable* _this, INT32 nRecs, INT32 nRealloc)
{
  INT32 nR = CDlpTable_GetNRecs(_this);
  if (nRealloc>0 && nRealloc<nRecs) nRealloc = nRecs;

  if (nR+nRecs>CDlpTable_GetMaxRecs(_this))
  {
    if (nRealloc>0)
    {
      IF_NOK(CDlpTable_Realloc(_this,nR+nRealloc)) return -1;
    }
    else return -1;
  }
  CDlpTable_IncNRecs(_this,nRecs);
  return nR;
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
INT32 CDlpTable_InsertRecs(CDlpTable* _this, INT32 nInsertAt, INT32 nRecs, INT32 nRealloc)
{
  INT32 nR   = -1;       /* Index of first appended record */
  INT32 nRl  = 0;        /* Record length in bytes         */
  INT64 nXXR = 0;        /* Capacity after reallocation    */

  if ((nR=CDlpTable_AddRecs(_this,nRecs,nRealloc))<0) return -1;
  if (nInsertAt< 0 ) nInsertAt=0;
  if (nInsertAt>=nR) return nR;

  nXXR = CDlpTable_GetMaxRecs(_this);
  nRl  = CDlpTable_GetRecLen (_this);

  if (nRl>0)
  {
    dlp_memmove
    (
      CDlpTable_XAddr(_this,nInsertAt+nRecs,0),
      CDlpTable_XAddr(_this,nInsertAt      ,0),
      (nXXR-nInsertAt-nRecs)*(INT64)nRl
    );
    dlp_memset(CDlpTable_XAddr(_this,nInsertAt,0),0,(INT64)nRecs*(INT64)nRl);
  }

  return nInsertAt;
}

/**
 * Clear table (set all cells to zero), the structure of the table
 * remains unchanged
 *
 * @param _this pointer to CDlpTable instance to clear (this)
 * @return O_K if successful NOT_EXEC otherwise
 */
INT16 CDlpTable_Clear(CDlpTable* _this)
{
  if(_this->m_compDescrList==NULL) return NOT_EXEC;
  if(!_this->m_theDataPointer) return NOT_EXEC;

  dlp_memset(_this->m_theDataPointer,0,_this->m_nrec*(size_t)_this->m_reclen);

  return O_K;
}

/**
 * Returns TRUE if there are neither components nor records in the table,
 * FALSE otherwise.
 *
 * @param  _this pointer to CDlpTable instance
 */
INT16 CDlpTable_IsEmpty(CDlpTable* _this)
{
  return ((_this->m_dim==0)||(_this->m_nrec==0)) ? TRUE : FALSE;
}

/**
 * Returns the number of valid records in the table.
 *
 * @param  _this Pointer to CDlpTable instance
 */
INT32 CDlpTable_GetNRecs(CDlpTable* _this)
{
  return _this->m_nrec;
}

/**
 * Increments the number of valid records of the table. If the resulting
 * number of valid records exceeds the table's capacity, the number of
 * valid records will be set to the capacity.
 *
 * @param  _this Pointer to CDlpTable instance
 * @param  nRecs The increment
 * @return The new number of valid records
 */
INT32 CDlpTable_IncNRecs(CDlpTable* _this, INT32 nRecs)
{
  if((_this->m_nrec + nRecs) < 0)               _this->m_nrec = 0;
  if((_this->m_nrec + nRecs) > _this->m_maxrec) _this->m_nrec = _this->m_maxrec;
  else _this->m_nrec += nRecs;
  return _this->m_nrec;
}

/**
 * Sets the number of valid records of the table. If the resulting
 * number of valid records exceeds the table's capacity, the number of
 * valid records will be set to the capacity.
 *
 * @param  _this Pointer to CDlpTable instance
 * @param  nRecs The increment
 * @return The new number of valid records
 */
INT32 CDlpTable_SetNRecs(CDlpTable* _this, INT32 nRecs)
{
  if(nRecs < 0)                    _this->m_nrec = 0;
  else if(nRecs > _this->m_maxrec) _this->m_nrec = _this->m_maxrec;
  else _this->m_nrec = nRecs;
  return _this->m_nrec;
}

/**
 * Returns the capacity (the maximal number of records) of the table.
 *
 * @param  _this Pointer to CDlpTable instance
 */
INT32 CDlpTable_GetMaxRecs(CDlpTable* _this)
{
  return _this->m_maxrec;
}

/**
 * Returns the length (in bytes) of one record.
 *
 * @param  _this pointer to CDlpTable instance
 */
INT32 CDlpTable_GetRecLen(CDlpTable* _this)
{
  INT32 i = 0;

  if(_this->m_compDescrList==NULL) return NOT_EXEC;

  _this->m_reclen=0;
  for (i=0; i<_this->m_dim; i++)
    _this->m_reclen+=_this->m_compDescrList[i].size;

  return _this->m_reclen;
}

/**
 * Returns the number of components in the table.
 *
 * @param  _this Pointer to CDlpTable instance
 */
INT32 CDlpTable_GetNComps(CDlpTable* _this)
{
  return _this->m_dim;
}

/**
 * Returns the size (in bytes) of one component of the table.
 *
 * @param  _this pointer to CDlpTable instance
 * @param nComp Index of component
 */
INT32 CDlpTable_GetCompSize(CDlpTable* _this, INT32 nComp)
{
  if (_this->m_compDescrList==NULL) return 0;
  if (nComp <0                    ) return 0;
  if (nComp>=_this->m_dim         ) return 0;
  return _this->m_compDescrList[nComp].size;
}

/**
 * Returns the offset (in bytes) of one component of the table from the
 * first byte of the record..
 *
 * @param  _this pointer to CDlpTable instance
 * @param nComp Index of component
 */
INT64 CDlpTable_GetCompOffset(CDlpTable* _this, INT32 nComp)
{
  if (_this->m_compDescrList==NULL) return 0;
  if (nComp <0                    ) return 0;
  if (nComp>=_this->m_dim         ) return 0;
  return _this->m_compDescrList[nComp].offset;
}

/**
 * Returns the type of one component of the table.
 *
 * @param  _this pointer to CDlpTable instance
 * @param nComp Index of component
 */
INT16 CDlpTable_GetCompType(CDlpTable* _this, INT32 nComp)
{
  if (_this->m_compDescrList==NULL) return 0;
  if (nComp <0                    ) return 0;
  if (nComp>=_this->m_dim         ) return 0;
  return (INT16)_this->m_compDescrList[nComp].ctype;
}

/**
 * Returns a pointer to the name of one component.
 *
 * @param  _this Pointer to CDlpTable instance
 * @param nComp  The component index
 */
char* CDlpTable_GetCompName(CDlpTable* _this, INT32 nComp)
{
  if (_this->m_compDescrList==NULL) return NULL;
  if (nComp <0                    ) return NULL;
  if (nComp>=_this->m_dim         ) return NULL;
  return _this->m_compDescrList[nComp].lpName;
}

/**
 * Sets the name of a component.
 *
 * @param  _this  Pointer to CDlpTable instance
 * @param nComp   The component index
 * @param lpsName The new name of the component
 * @return O_K if successfull, a negative error code otherwise
 */
INT16 CDlpTable_SetCompName(CDlpTable* _this, INT32 nComp, const char* lpsName)
{
  SDlpTableComp* lpComp = NULL;

  if ((lpComp = CDlpTable_FindCompByIdx(_this,nComp))==NULL) return NOT_EXEC;
  dlp_strncpy(lpComp->lpName,lpsName,COMP_DESCR_LEN-1);

  return O_K;
}

/**
 * Returns the continuation rate specified for the table (field m_fsr).
 *
 * @param  _this Pointer to CDlpTable instance
 */
FLOAT64 CDlpTable_GetFsr(CDlpTable* _this)
{
  return _this->m_fsr;
}

/**
 * Returns a pointer to the SDlpTableComp structure describing the (first)
 * component of the table whose name is lpsName. If no such component exists,
 * the method will return NULL.
 *
 * @param _this   Pointer to CDlpTable instance
 * @param lpsName The component name
 * @return A pointer to a CDlpTableComp structure or NULL in case of an error
 * @see CDlpTable_FindCompByIdx
 * @see CDlpTable_CompNameToIdx
 */
SDlpTableComp* CDlpTable_FindCompByName(CDlpTable* _this, const char* lpsName)
{
  INT32 i = 0;

  if(!lpsName               ) return NULL;
  if(!_this                 ) return NULL;
  if(!_this->m_compDescrList) return NULL;

  for (i=0; i<_this->m_dim; i++)
  {
    if (dlp_strcmp(_this->m_compDescrList[i].lpName,lpsName)==0)
      return &_this->m_compDescrList[i];
  }
  return NULL;
}

/**
 * Returns a pointer to the SDlpTableComp structure describing the nComp'th
 * component of the table. If no such component exists, the method will return
 * NULL.
 *
 * @param _this Pointer to CDlpTable instance
 * @param nComp The component index
 * @return A pointer to a CDlpTableComp structure or NULL in case of an error
 * @see CDlpTable_FindCompByName
 * @see CDlpTable_CompNameToIdx
 */
SDlpTableComp* CDlpTable_FindCompByIdx(CDlpTable* _this, INT32 nComp)
{
  if(nComp<0 || nComp>=_this->m_dim) return NULL;
  if(!_this                        ) return NULL;
  if(!_this->m_compDescrList       ) return NULL;
  return &_this->m_compDescrList[nComp];
}

/**
 * Returns index of the (first) component of the table whose name is lpsName.
 * If no such component exists, the method will return NOT_EXEC.
 *
 * @param _this   Pointer to CDlpTable instance
 * @param lpsName The component name
 * @return The component index  or NOT_EXEC in case of an error
 * @see CDlpTable_FindCompByIdx
 * @see CDlpTable_FindCompByName
 */
INT32 CDlpTable_CompNameToIdx(CDlpTable* _this, const char* lpsName)
{
  INT32 i = 0;

  if(!lpsName               ) return NOT_EXEC;
  if(!_this                 ) return NOT_EXEC;
  if(!_this->m_compDescrList) return NOT_EXEC;

  for (i=0; i<_this->m_dim; i++)
  {
    if (dlp_strcmp(_this->m_compDescrList[i].lpName,lpsName)==0)
      return i;
  }
  return NOT_EXEC;
}

/**
 * Returns a pointer to a cell in the table. If the specified cell does
 * not exist the function returns NULL.
 *
 * @param  _this Pointer to CDlpTable instance
 * @param  nRec  Record index of cell
 * @param  nComp Component index of cell
 */
BYTE* CDlpTable_XAddr(CDlpTable* _this, INT32 nRec, INT32 nComp)
{
  if (_this==NULL                 ) return NULL;
  if (_this->m_compDescrList==NULL) return NULL;
  if (nRec==0 && nComp==0) return _this->m_theDataPointer;

  if((nRec>=0)&&(nRec<_this->m_maxrec)&&(nComp>=0)&&(nComp<_this->m_dim))
  {
    return _this->m_theDataPointer             +
           (INT64)nRec*(INT64)_this->m_reclen  +
           (INT64)_this->m_compDescrList[nComp].offset;
  }

  return NULL;
}

/**
 * Returns the real part of numeric value of cell [nRec,nComp]. If the type of
 * component nComp is not numeric, the method will return 0. No value is
 * reserved to indicate an error!
 *
 * @param _this Pointer to CDlpTable instance
 * @param nRec  The record index of the cell to read
 * @param nComp The component index of the cell to read
 * @see CDlpTable_Dstore
 * @see CDlpTable_Sstore
 * @see CDlpTable_Sfetch
 */
FLOAT64 CDlpTable_Dfetch(CDlpTable* _this, INT32 nRec, INT32 nComp)
{
  return CDlpTable_Cfetch(_this, nRec, nComp).x;
}

/**
 * Returns the numeric value of cell [nRec,nComp]. If the type of component
 * nComp is not numeric, the method will return 0. No value is reserved to
 * indicate an error!
 *
 * @param _this Pointer to CDlpTable instance
 * @param nRec  The record index of the cell to read
 * @param nComp The component index of the cell to read
 * @see CDlpTable_Cfetch
 * @see CDlpTable_Sfetch
 * @see CDlpTable_Cstore
 * @see CDlpTable_Dstore
 * @see CDlpTable_Sstore
 */
COMPLEX64 CDlpTable_Cfetch(CDlpTable* _this, INT32 nRec, INT32 nComp)
{
  SDlpTableComp* c = NULL;
  BYTE*          p = NULL;
  INT16          t = 0;

  if (!_this                             ) return CMPLX(0);
  if (!_this->m_compDescrList            ) return CMPLX(0);
  if ((nRec <0)||(nRec >=_this->m_maxrec)) return CMPLX(0);
  if ((nComp<0)||(nComp>=_this->m_dim   )) return CMPLX(0);

  c = &_this->m_compDescrList[nComp];
  t = c->ctype;
  p = _this->m_theDataPointer+(size_t)nRec*(size_t)_this->m_reclen+(size_t)c->offset;

  return dlp_fetch(p,t);
}

/**
 * Stores the real part of numeric value dVal into cell [nRec,nComp]. If the
 * type of component nComp is not numeric, the method will do nothing and return
 * NOT_EXEC.
 *
 * @param _this Pointer to CDlpTable instance
 * @param nVal  The numeric value to store
 * @param nRec  The record index of the cell to write
 * @param nComp The component index of the cell to write
 * @return O_K if successfull, a negative error code otherwise
 * @see CDlpTable_Cfetch
 * @see CDlpTable_Sfetch
 * @see CDlpTable_Cstore
 * @see CDlpTable_Dstore
 * @see CDlpTable_Sstore
 */
INT16 CDlpTable_Dstore(CDlpTable* _this, FLOAT64 nVal, INT32 nRec, INT32 nComp)
{
  return CDlpTable_Cstore(_this, CMPLX(nVal), nRec, nComp);
}

/**
 * Stores the numeric value dVal into cell [nRec,nComp]. If the type of
 * component nComp is not numeric, the method will do nothing and return
 * NOT_EXEC.
 *
 * @param _this Pointer to CDlpTable instance
 * @param nVal  The numeric value to store
 * @param nRec  The record index of the cell to write
 * @param nComp The component index of the cell to write
 * @return O_K if successfull, a negative error code otherwise
 * @see CDlpTable_Cfetch
 * @see CDlpTable_Sfetch
 * @see CDlpTable_Cstore
 * @see CDlpTable_Dstore
 * @see CDlpTable_Sstore
 */
INT16 CDlpTable_Cstore(CDlpTable* _this, COMPLEX64 nVal, INT32 nRec, INT32 nComp)
{
  SDlpTableComp* c = NULL;
  BYTE*          p = NULL;
  INT16          t = 0;

  if (!_this                             ) return NOT_EXEC;
  if (!_this->m_compDescrList            ) return NOT_EXEC;
  if ((nRec <0)||(nRec >=_this->m_maxrec)) return NOT_EXEC;
  if ((nComp<0)||(nComp>=_this->m_dim   )) return NOT_EXEC;

  c = &_this->m_compDescrList[nComp];
  t = c->ctype;
  p = _this->m_theDataPointer+(INT64)nRec*(INT64)_this->m_reclen+(INT64)c->offset;

  return dlp_store(nVal,p,t);
}

/**
 * <p>Returns the pointer value of cell [<code>nRec</code>,<code>nComp</code>].
 * If the type of component <code>nComp</code> is not pointer, the method will
 * return <code>NULL</code>.</p>
 * <h3>Note</h3>
 * <p>The returned address points to the actual data content of the cell.</p>
 *
 * @param _this Pointer to CDlpTable instance
 * @param nRec  The record index of the cell to read
 * @param nComp The component index of the cell to read
 * @return A pointer to a string if successfull, <code>NULL</code> otherwise
 * @see CDlpTable_Pstore
 * @see CDlpTable_Dfetch
 * @see CDlpTable_Dstore
 * @see CDlpTable_Sfetch
 * @see CDlpTable_Sstore
 */
void* CDlpTable_Pfetch(CDlpTable* _this, INT32 nRec, INT32 nComp)
{
  if(!_this                                                       ) return NULL;
  if(!dlp_is_pointer_type_code(CDlpTable_GetCompType(_this,nComp))) return NULL;
  return (void*)CDlpTable_XAddr(_this,nRec,nComp);
}

/**
 * Stores the pointer lpVal into cell [nRec,nComp]. If the type of component
 * nComp is not pointer, the method will do nothing and return NOT_EXEC.
 *
 * @param _this  Pointer to CDlpTable instance
 * @param lpVal  The pointer to store
 * @param nRec   The record index of the cell to write
 * @param nComp  The component index of the cell to write
 * @return O_K if successfull, a negative error code otherwise
 * @see CDlpTable_Pfetch
 * @see CDlpTable_Dfetch
 * @see CDlpTable_Dstore
 * @see CDlpTable_Sfetch
 */
INT16 CDlpTable_Pstore(CDlpTable* _this, void* lpVal, INT32 nRec, INT32 nComp)
{
  char* p = NULL;
  INT16 t = 0;

  if(!_this                                                           ) return NOT_EXEC;
  if(!(p = (char*)CDlpTable_XAddr(_this,nRec,nComp))                  ) return NOT_EXEC;
  if(!dlp_is_pointer_type_code(t = CDlpTable_GetCompType(_this,nComp))) return NOT_EXEC;

  *(BYTE**)CDlpTable_XAddr(_this,nRec,nComp)=(BYTE*)lpVal;

  return O_K;
}

/**
 * <p>Returns the string value of cell [<code>nRec</code>,<code>nComp</code>].
 * If the type of component <code>nComp</code> is not symbolic, the method will
 * return <code>NULL</code>.</p>
 * <h3>Note</h3>
 * <p>The returned address points to the actual data content of the cell. You
 * may use this pointer to write data into the table, however, this is not
 * recommended. Use {@link CDlpTable_Sstore} instead!</p>
 *
 * @param _this Pointer to CDlpTable instance
 * @param nRec  The record index of the cell to read
 * @param nComp The component index of the cell to read
 * @return A pointer to a string if successfull, <code>NULL</code> otherwise
 * @see CDlpTable_Dfetch
 * @see CDlpTable_Dstore
 * @see CDlpTable_Sstore
 */
char* CDlpTable_Sfetch(CDlpTable* _this, INT32 nRec, INT32 nComp)
{
  if(!_this                                   ) return NULL;
  if(CDlpTable_GetCompType(_this,nComp)>L_SSTR) return NULL;
  return (char*)CDlpTable_XAddr(_this,nRec,nComp);
}

/**
 * Stores the string lpsVal into cell [nRec,nComp]. If the type of component
 * nComp is not symbolic, the method will do nothing and return NOT_EXEC.
 *
 * @param _this  Pointer to CDlpTable instance
 * @param lpsVal The string to store
 * @param nRec   The record index of the cell to write
 * @param nComp  The component index of the cell to write
 * @return O_K if successfull, a negative error code otherwise
 * @see CDlpTable_Dfetch
 * @see CDlpTable_Dstore
 * @see CDlpTable_Sfetch
 */
INT16 CDlpTable_Sstore(CDlpTable* _this, const char* lpsVal, INT32 nRec, INT32 nComp)
{
  char* p = NULL;
  INT16 t = 0;

  if(!_this                                         ) return NOT_EXEC;
  if(!lpsVal                                        ) return NOT_EXEC;
  if(!(p = (char*)CDlpTable_XAddr(_this,nRec,nComp))) return NOT_EXEC;
  if((t = CDlpTable_GetCompType(_this,nComp))>L_SSTR) return NOT_EXEC;

  dlp_strncpy(p,lpsVal,t-1);
  p[t-1]=0;

  return O_K;
}

/**
 * <p>Deletes records from a source table and stores the result into a
 * destination table (this instance).</p>
 * <h4>Remarks</h4>
 * <ul>
 *   <li>The method does <em>not</em> reduce the capacity of this instance
 *   compared to <code>lpiSrc</code>. If you want this instance to be physically
 *   smaller, use {@link CDlpTable_Realloc}.</li>
 * </ul>
 *
 * @param _this
 *          Pointer to destination instance (this)
 * @param lpiSrc
 *          Pointer to source instance
 * @param nFirstRec
 *          First record to delete
 * @param nRecs
 *          Number of records to delete
 * @return <code>O_K</code> if successfull, a (negative) error code otherwise.
 */
INT16 CDlpTable_DeleteRecs
(
  CDlpTable* _this,
  CDlpTable* lpiSrc,
  INT32       nFirstRec,
  INT32       nRecs
)
{
  if (!_this                        ) return NOT_EXEC;
  if (!lpiSrc                       ) return NOT_EXEC;
  if (!lpiSrc->m_compDescrList      ) return NOT_EXEC;
  if (nFirstRec<0                   ) return O_K;
  if (nFirstRec>=lpiSrc->m_nrec     ) return O_K;
  if (nFirstRec+nRecs>lpiSrc->m_nrec) nRecs = lpiSrc->m_nrec-nFirstRec;
  if (nRecs<=0                      ) return O_K;
  CDlpTable_Copy(_this,lpiSrc,0,lpiSrc->m_maxrec);
  dlp_memmove
  (
    _this->m_theDataPointer + nFirstRec*(size_t)_this->m_reclen,
    _this->m_theDataPointer + (nFirstRec+nRecs)*(size_t)_this->m_reclen,
    ((size_t)lpiSrc->m_maxrec-nFirstRec-nRecs)*(size_t)_this->m_reclen
  );
  _this->m_nrec = lpiSrc->m_nrec-nRecs;
  return O_K;
}

/**
 * Deletes components from a source table and stores the result into a destination
 * table (this instance).
 *
 * @param _this      Pointer to destination instance (this)
 * @param lpiSrc     Pointer to source instance
 * @param nFirstComp First component to delete
 * @param nComps     Number of components to delete
 * @return O_K if successfull, a negative error code otherwise
 */
INT16 CDlpTable_DeleteComps(CDlpTable* _this, CDlpTable* lpiSrc, INT32 nFirstComp, INT32 nComps)
{
  INT32  i            = 0;
  INT64  nSize1       = 0;
  INT64  nSize2       = 0;
  INT64  nSize3       = 0;
  INT32  nNewRecLen   = 0;
  INT32  nOffset      = 0;
  INT32  nTo          = 0;
  BYTE* lpNewDataPtr = NULL;

  if(!_this                  ) return NOT_EXEC;
  if(!lpiSrc                 ) return NOT_EXEC;
  if(!lpiSrc->m_compDescrList) return NOT_EXEC;
  if(nComps <= 0             ) return O_K;

  if (nFirstComp+nComps>lpiSrc->m_dim) nComps = lpiSrc->m_dim - nFirstComp;
  nTo = nFirstComp + nComps;

  if(nFirstComp < 0 || nFirstComp >= lpiSrc->m_dim) return O_K;
  if(nTo <= nFirstComp || nTo > lpiSrc->m_dim     ) return O_K;

  CDlpTable_Copy(_this,lpiSrc,0,lpiSrc->m_nrec);

  for(i=0         ; i<nFirstComp  ; i++) nSize1+=CDlpTable_GetCompSize(_this,i);
  for(i=nFirstComp; i<nTo         ; i++) nSize2+=CDlpTable_GetCompSize(_this,i);
  for(i=nTo       ; i<_this->m_dim; i++) nSize3+=CDlpTable_GetCompSize(_this,i);

  nNewRecLen = nSize1 + nSize3;

  if (_this->m_nrec>0)
  {
    if(((size_t)_this->m_nrec * (size_t)nNewRecLen) == 0) { CDlpTable_SReset(_this); return O_K; }

    lpNewDataPtr = (BYTE*)dlp_malloc((size_t)_this->m_maxrec * (size_t)nNewRecLen * sizeof(BYTE));
    if(!lpNewDataPtr) return NOT_EXEC;

    dlp_memmove(lpNewDataPtr, _this->m_theDataPointer, nSize1);

    for(i=1; i<_this->m_maxrec; i++)
      dlp_memmove
      (
        lpNewDataPtr + (size_t)i * (size_t)nNewRecLen - (size_t)nSize3,
        _this->m_theDataPointer + (size_t)i * (size_t)_this->m_reclen - (size_t)nSize3,
        nNewRecLen
      );

    dlp_memmove
    (
      lpNewDataPtr + (size_t)_this->m_maxrec * (size_t)nNewRecLen - (size_t)nSize3,
      _this->m_theDataPointer + (size_t)_this->m_maxrec * (size_t)_this->m_reclen - (size_t)nSize3,
      nSize3
    );

    dlp_free(_this->m_theDataPointer);
    _this->m_theDataPointer = lpNewDataPtr;
  }

  /* Adjust component list */
  dlp_memmove
  (
    &_this->m_compDescrList[nFirstComp],
    &_this->m_compDescrList[nTo],
    (_this->m_maxdim-nTo)*sizeof(SDlpTableComp)
  );

  _this->m_dim   -= nComps;
  _this->m_reclen = nNewRecLen;

  /* Re-calculate all offsets */
  for (i=0,nOffset=0; i<_this->m_dim; i++)
  {
    _this->m_compDescrList[i].offset = nOffset;
    nOffset+=_this->m_compDescrList[i].size;
  }

  return O_K;
}

/**
 * Append (join) all components of source table to destination table.
 *
 * @param  _this  Pointer to destination instance (this)
 * @param  lpiSrc Pointer to source instance
 * @return O_K if successful, ERR_TRUNCATE if record number in lpiSrc is
 *         greater than record number in _this and NOT_EXEC otherwise
 */
INT16 CDlpTable_Join(CDlpTable* _this, CDlpTable* lpiSrc)
{
  INT32  nC     = 0;                                                             /* Current component                 */
  INT32  nR     = 0;                                                             /* Current record                    */
  INT32  nXRD   = 0;                                                             /* Number of records in this (=dest) */
  INT32  nXRS   = 0;                                                             /* Number of records in source       */
  INT32  nRlT   = 0;                                                             /* Record length in this             */
  INT32  nRlS   = 0;                                                             /* Record length in source           */
  INT32  nRlD   = 0;                                                             /* Record length in destination      */
  BYTE* lpTd   = NULL;                                                          /* This write pointer                */
  BYTE* lpSd   = NULL;                                                          /* Source write pointer              */
  BYTE* lpTs   = NULL;                                                          /* This read pointer                 */
  BYTE* lpSs   = NULL;                                                          /* Source read pointer               */
  BYTE* lpData = NULL;                                                          /* The new data pointer              */

  /* Validation */                                                              /* --------------------------------- */
  if (!_this                  || !lpiSrc                 ) return NOT_EXEC;     /* This and source pointer           */
  if (!_this->m_compDescrList || !lpiSrc->m_compDescrList) return NOT_EXEC;     /* This and src. comp. descr. arrays */
  if (CDlpTable_IsEmpty(lpiSrc)                          ) return O_K;          /* Src. empty -> nothing to be done  */
  if (CDlpTable_IsEmpty(_this))                                                 /* This table empty                  */
    return CDlpTable_Copy(_this,lpiSrc,0,lpiSrc->m_nrec);                       /*   Copy source                     */

  /* Initialize */                                                              /* --------------------------------- */
  nXRD   = _this->m_nrec;                                                       /* Number of this and dest. records  */
  nXRS   = lpiSrc->m_nrec;                                                      /* Number of source records          */
  nRlT   = _this->m_reclen;                                                     /* This record length                */
  nRlS   = lpiSrc->m_reclen;                                                    /* Source record length              */
  nRlD   = nRlT+nRlS;                                                           /* Destination record length         */

  /* Assemble destination data */                                               /* --------------------------------- */
  lpData = (BYTE*)dlp_malloc((INT64)nRlD*(INT64)nXRD);                          /* Get new data memory               */
  for                                                                           /* Loop over destination records     */
  (                                                                             /* |                                 */
    nR=0, lpTd=lpData, lpSd=lpData+nRlT,                                        /* |                                 */
    lpTs=_this->m_theDataPointer, lpSs=lpiSrc->m_theDataPointer;                /* |                                 */
    nR<nXRD;                                                                    /* |                                 */
    nR++, lpTd+=nRlD, lpSd+=nRlD, lpTs+=nRlT, lpSs+=nRlS                        /* |                                 */
  )                                                                             /* |                                 */
  {                                                                             /* >>                                */
    dlp_memmove(lpTd,lpTs,nRlT);                                                /*   Copy this data                  */
    if (nR<nXRS) dlp_memmove(lpSd,lpSs,nRlS);                                   /*   Copy source data or ...         */
    else         dlp_memset (lpSd,0   ,nRlS);                                   /*   ... clear destination data      */
  }                                                                             /* <<                                */
  dlp_free(_this->m_theDataPointer);                                            /* Free old this data                */
  _this->m_theDataPointer = lpData;                                             /* Use new this data                 */

  /* Assemble destination component description */                              /* --------------------------------- */
  _this->m_compDescrList = (SDlpTableComp*)dlp_realloc(_this->m_compDescrList,  /* Reallocate component descriptions */
    _this->m_dim+lpiSrc->m_dim+COMP_ALLOC,sizeof(SDlpTableComp));               /* |                                 */
  dlp_memmove(&_this->m_compDescrList[_this->m_dim],lpiSrc->m_compDescrList,    /* Append source comp. descriptions  */
    lpiSrc->m_dim*sizeof(SDlpTableComp));                                       /* |                                 */
  for (nC=_this->m_dim; nC<_this->m_dim+lpiSrc->m_dim; nC++)                    /* Loop over appended components     */
    _this->m_compDescrList[nC].offset += nRlT;                                  /*   Adjust component offset         */

  /* Adjust fields */                                                           /* --------------------------------- */
  _this->m_dim    += lpiSrc->m_dim;                                             /* Dimension                         */
  _this->m_maxdim  = _this->m_dim+COMP_ALLOC;                                   /* Maximal dimension                 */
  _this->m_maxrec  = _this->m_nrec;                                             /* Maximal number of records         */
  _this->m_reclen  = nRlD;                                                      /* Record length                     */

  /* Check up */                                                                /* --------------------------------- */
  DLPASSERT(_this->m_maxdim ==                                                  /* Component description storage ... */
    (INT32)(dlp_size(_this->m_compDescrList)/sizeof(SDlpTableComp)));            /* | ... fits maximal dimension      */
  DLPASSERT(_this->m_maxrec ==                                                  /* Data storage fits maximal ...     */
    (INT32)(dlp_size(_this->m_theDataPointer)/_this->m_reclen));                 /* | ... number of records           */
  DLPASSERT(_this->m_reclen == CDlpTable_GetRecLen(_this));                     /* Record length as expected         */

  /* Finally... */                                                              /* --------------------------------- */
  if(_this->m_nrec < lpiSrc->m_nrec) return ERR_TRUNCATE;                       /* Source has been truncated...      */
  else return O_K;                                                              /* Everything ok                     */
}

/* 2008-03-14 MWX: Optimized to save memoves --> */
INT16 CDlpTable_Join_(CDlpTable* _this, CDlpTable* lpiSrc)
{
  INT32          i          = 0;
  INT32          nRecLenOld = 0;
  void*          lpD        = NULL;
  void*          lpS        = NULL;
  SDlpTableComp* lpComp     = NULL;

  if(!_this || !lpiSrc       ) return NOT_EXEC;
  if(!lpiSrc->m_compDescrList) return NOT_EXEC;
  if(!_this->m_compDescrList ) return NOT_EXEC;
  if(!lpiSrc->m_dim          ) return O_K;

  /* If target table is empty then copy table */
  if(CDlpTable_IsEmpty(_this))
  {
    CDlpTable_Copy(_this,lpiSrc,0,lpiSrc->m_nrec);
    return O_K;
  }

  nRecLenOld = _this->m_reclen;

  /* Create new components in target instance */
  for(i=0;i<lpiSrc->m_dim;i++)
  {
    lpComp = CDlpTable_FindCompByIdx(lpiSrc,i);
    if(lpComp) CDlpTable_AddComp(_this,lpComp->lpName,lpComp->ctype);
  }

  /* Copy data */
  lpD = _this->m_theDataPointer + nRecLenOld;
  lpS = lpiSrc->m_theDataPointer;
  i = MIN(_this->m_nrec,lpiSrc->m_nrec);
  while(i--)
  {
    dlp_memmove(lpD,lpS,lpiSrc->m_reclen);
    lpD += _this->m_reclen;
    lpS += lpiSrc->m_reclen;
  }

  if(_this->m_nrec < lpiSrc->m_nrec) return ERR_TRUNCATE;
  else return O_K;
}
/* <-- */

/**
 * <p>Appends (concatenates) all records of source table to destination table.</p>
 *
 * @param _this  Pointer to destination instance (this)
 * @param lpiSrc Pointer to source instance
 * @return O_K if successful, ERR_TRUNCATE if source dimension greater
 *         than target dimension and NOT_EXEC otherwise
 * @see CDlpTable_CatEx
 */
INT16 CDlpTable_Cat(CDlpTable* _this, CDlpTable* lpiSrc)
{
  if(!_this || !lpiSrc        ) return NOT_EXEC;
  if(CDlpTable_IsEmpty(lpiSrc)) return NOT_EXEC;

  return CDlpTable_CatEx(_this,lpiSrc,0,lpiSrc->m_nrec);
}

/**
 * <p>Appends (concatenates) <code>nCount</code> records of source table to
 * destination table starting with record <code>nFirstRec</code>.</p>
 * <h3>Note</h3>
 * <p>If source and destination instance have the same number and types of
 * components, the operation is performed fast using dlp_memmove. If both
 * tables have different structures, the data is appended cell by cell and
 * conversion of numeric data to the target type is performed. Symbolic
 * component length is adjusted to the longest of both components. If the
 * target type is symbolic and the source type numeric (or vice versa) the
 * component is skipped. If source table has more components than target table,
 * the remaining data is truncated. If the target table has a higher dimension
 * the remaining components are initialized by zero.</p>
 *
 * @param _this     Pointer to destination instance (this)
 * @param lpiSrc    Pointer to source instance
 * @param nFirstRec Index of first record to append
 * @param nCount    Number of records to append
 * @return O_K if successful, ERR_TRUNCATE if source dimension greater
 *         than target dimension and NOT_EXEC otherwise
 * @see CDlpTable_Cat
 */
INT16 CDlpTable_CatEx(CDlpTable* _this, CDlpTable* lpiSrc, INT32 nFirstRec, INT32 nCount)
{
  BOOL  bIdentical = FALSE;
  INT32  i          = 0;
  INT32  j          = 0;
  INT32  nNRecOld   = 0;
  INT16 nRetVal    = O_K;

  /* Validate */
  if(!_this || !lpiSrc        ) return NOT_EXEC;
  if(CDlpTable_IsEmpty(lpiSrc)) return NOT_EXEC;

  if (nFirstRec       < 0             ) nFirstRec = 0;
  if (nFirstRec+nCount> lpiSrc->m_nrec) nCount    = lpiSrc->m_nrec-nFirstRec;
  if (nCount          <=0             ) return NOT_EXEC;

  /* If destination is empty and has no declared components --> copy */
  if((_this->m_maxrec==0) && (_this->m_dim == 0)) return CDlpTable_Copy(_this,lpiSrc,nFirstRec,nCount);

  /* Initialize */
  nNRecOld = _this->m_nrec;

  if((_this->m_nrec+nCount)>_this->m_maxrec)
    CDlpTable_Realloc(_this,_this->m_nrec + nCount);
  _this->m_nrec = _this->m_nrec + nCount;

  /* Check if tables have the same number and types of components */
  if(_this->m_dim==lpiSrc->m_dim)
    for(bIdentical=TRUE,i=0; i<_this->m_dim; i++)
      if(CDlpTable_GetCompType(_this,i)!=CDlpTable_GetCompType(lpiSrc,i))
      {
        bIdentical=FALSE;
        break;
      }

  if(bIdentical)
  {
    IFCHECK printf("\n Source and destination instance have the same number and types, the operation is performed fast using dlp_memmove.");
    dlp_memmove
    (
      _this->m_theDataPointer+(nNRecOld*(size_t)_this->m_reclen),
      lpiSrc->m_theDataPointer+(nFirstRec*(size_t)lpiSrc->m_reclen),
      nCount*(size_t)lpiSrc->m_reclen
    );
  }
  else
  {
    IFCHECK printf("\n Tables have different structures, the data is appended cell by cell.");
    for(j=0; j<lpiSrc->m_dim; j++)
    {
      INT16 nMatchingTypes = -1;

      /* expand target table if necessary * /
      / * NOTE: We do not do this to be compatible to sig$ma$Lab * /
      / *       But we do return ERR_TRUNCATE to release a warning * /
      if(_this->m_dim <= j)
      {
        SDlpTableComp* lpComp = NULL;
        lpComp = CDlpTable_FindCompByIdx(lpiSrc,j);
        if(lpComp) CDlpTable_Addcomp(_this,lpComp->lpName,lpComp->ctype);
      }*/
      if(_this->m_dim <= j) nRetVal = ERR_TRUNCATE;

      /* Check if types match */
      if
      (
        (dlp_is_numeric_type_code (CDlpTable_GetCompType(lpiSrc,j))  &&
         dlp_is_numeric_type_code (CDlpTable_GetCompType(_this ,j))) ||
        (dlp_is_symbolic_type_code(CDlpTable_GetCompType(lpiSrc,j))  &&
         dlp_is_symbolic_type_code(CDlpTable_GetCompType(_this ,j)))
      )
      {
        nMatchingTypes=CDlpTable_GetCompType(lpiSrc,j);
      }

      /* Adjust symbolic type to longest of both components. */
      if(dlp_is_symbolic_type_code(nMatchingTypes)&&nMatchingTypes>CDlpTable_GetCompType(_this,j))
      {
        INT16 nCLn;
        INT32  nRIdx;
        char  sCompName[COMP_DESCR_LEN];
        BYTE* lpComp;

        IFCHECK printf("\n Adjust length of symbolic component to %d.",(int)nMatchingTypes);

        nCLn   = CDlpTable_GetCompType(_this,j);
        lpComp = (unsigned char*)dlp_malloc(nCLn*(size_t)CDlpTable_GetNRecs(_this)*sizeof(char));

        for(nRIdx=0; nRIdx<CDlpTable_GetNRecs(_this); nRIdx++)
          dlp_strncpy((char*)(lpComp+nRIdx*(size_t)nCLn),
            (char*)CDlpTable_XAddr(_this,nRIdx,j),nCLn);

        dlp_strcpy(sCompName,CDlpTable_GetCompName(_this,j));
        CDlpTable_InsertComp(_this,sCompName,nMatchingTypes,j);
        CDlpTable_DeleteComps(_this,_this,j+1,1);

        for(nRIdx=0;nRIdx<CDlpTable_GetNRecs(_this);nRIdx++)
          dlp_strncpy((char*)CDlpTable_XAddr(_this,nRIdx,j),
            (char*)(lpComp+nRIdx*(size_t)nCLn),nCLn);

        dlp_free(lpComp);
      }

      /* Copy cell by cell */
      for(i=nFirstRec; i<nFirstRec+nCount; i++)
      {
        if(dlp_is_numeric_type_code(nMatchingTypes))
          CDlpTable_Dstore(_this,CDlpTable_Dfetch(lpiSrc,i,j),nNRecOld+i,j);
        else if(dlp_is_symbolic_type_code(nMatchingTypes))
          CDlpTable_Sstore(_this,CDlpTable_Sfetch(lpiSrc,i,j),nNRecOld+i,j);
      }
    }
  }

  DLP_CHECK_MEMINTEGRITY;
  return nRetVal;
}
/**
 * Prints the component structure of the table at stdout.
 *
 * @param _this Pointer to CDlpTable instance
 * @return O_K if successfull, a negative error code otherwise
 */
INT16 CDlpTable_Descr(CDlpTable* _this)
{
  INT64           nSize     = 0;
  INT32           i         = 0;
  INT32           nCompSize = 0;
  INT16          nCompType = 0;
  SDlpTableComp* lpComp    = NULL;

  if(!_this)                  return NULL;
  if(!_this->m_compDescrList) return NOT_EXEC;

  printf("\n\n---- Data Table Description \n");
  printf("max. record number    : %13ld\n"         ,(  long)_this->m_maxrec                );
  printf("valid record number   : %13ld\n"         ,(  long)_this->m_nrec                  );
  printf("record components     : %13ld\n"         ,(  long)_this->m_dim                   );
  printf("record bytes          : %13ld\n"         ,(  long)_this->m_reclen                );
  printf("descr0                : %13.8f\n"        ,(double)_this->m_descr0                );
  printf("descr1                : %13.8f\n"        ,(double)_this->m_descr1                );
  printf("descr2                : %13.8f\n"        ,(double)_this->m_descr2                );
  printf("descr3                : %13.8f\n"        ,(double)_this->m_descr3                );
  printf("descr4                : %13.8f\n"        ,(double)_this->m_descr4                );
  printf("window length     [ms]: %13.8f\n"        ,(double)_this->m_zf                    );
  printf("continuation rate [ms]: %13.8f\n"        ,(double)_this->m_fsr                   );
  printf("offset of 1.rec   [ms]: %13.8f\n"        ,(double)_this->m_ofs                   );
  printf("realization descr.    : %13p -> \"%s\"\n",_this->m_rtext,_this->m_rtext  );
  printf("realization attribute : %13p -> \"%s\"\n",_this->m_vrtext,_this->m_vrtext);
  printf("records pointer       : %13p\n"          ,_this->m_theDataPointer        );
  printf("component list pointer: %13p\n"          ,_this->m_compDescrList         );
  printf("\ncomponent description table:\n");
  printf("component     name  offset   bytes   type\n");

  for(i=0;i<_this->m_dim;i++)
  {
    nCompSize = CDlpTable_GetCompSize(_this,i);
    nCompType = CDlpTable_GetCompType(_this,i);
    lpComp    = CDlpTable_FindCompByIdx(_this,i);
    if(!lpComp) return NOT_EXEC;
    printf("%8ld: %8s %7ld %7ld %6ld",(long)i,lpComp->lpName,(long)nSize,(long)nCompSize,(long)nCompType);

    switch (nCompType)
    {
      case T_UCHAR  : printf(" unsigned char");  break;
      case T_CHAR   : printf(" signed char");    break;
      case T_USHORT : printf(" unsigned short"); break;
      case T_SHORT  : printf(" signed short");   break;
      case T_UINT   : printf(" unsigned int");   break;
      case T_INT    : printf(" signed int");     break;
      case T_ULONG  : printf(" unsigned long");  break;
      case T_LONG   : printf(" signed long");    break;
      case T_FLOAT  : printf(" float");          break;
      case T_DOUBLE : printf(" double");         break;
      case T_COMPLEX: printf(" complex");        break;
      default       : printf(" ascii");          break;
    }
    printf("\n");
    nSize+=nCompSize;
  }
  printf("\n\n---- end of table \n");

  return O_K;
}

/**
 * Prints the data content of the table at stdout.
 *
 * @param _this Pointer to CDlpTable instance
 * @return O_K if successfull, a negative error code otherwise
 */
INT16 CDlpTable_Print(CDlpTable* _this)
{
  INT32 i             = 0;
  INT32 j             = 0;
  INT32 lines         = 0;
  char dGetCharacter = '\0';
  char buff[512];
  char buffer[512];

  if(!_this                 ) return NULL;
  if(!_this->m_compDescrList) return NULL;

  printf("\n\n---- Data Record Sequence  \n");

  dlp_strcpy(buffer,"   nr.  ");

  for(i=0; i<_this->m_dim; i++)
  {
    INT16 nTypeCode = CDlpTable_GetCompType(_this,i);
    char* lpName    = CDlpTable_GetCompName(_this,i);

    switch (nTypeCode)
    {
      case T_UCHAR  : sprintf(buff,"%4s ", lpName);  break;
      case T_CHAR   : sprintf(buff,"%4s ", lpName);  break;
      case T_USHORT : sprintf(buff,"%6s ", lpName);  break;
      case T_SHORT  : sprintf(buff,"%6s ", lpName);  break;
      case T_UINT   : sprintf(buff,"%12s ",lpName);  break;
      case T_INT    : sprintf(buff,"%12s ",lpName);  break;
      case T_ULONG  : sprintf(buff,"%12s ",lpName);  break;
      case T_LONG   : sprintf(buff,"%12s ",lpName);  break;
      case T_FLOAT  : sprintf(buff,"%15s ",lpName);  break;
      case T_DOUBLE : sprintf(buff,"%24s ",lpName);  break;
      case T_COMPLEX: sprintf(buff,"%24s ",lpName);  break;
      default       : sprintf(buff,"%20s ",lpName);  break;
    }
    strcat(buffer,buff);
  }

  printf("%s\n",buffer);

  if(_this->m_nrec <= 0)
  {
    printf("---- no data available ");
    return O_K;
  }

  for(i=0; i<_this->m_nrec; i++)
  {
    sprintf(buffer,"%5ld: ",(long)i);
    printf("%s ",buffer);

    for(j=0; j<_this->m_dim; j++)
    {
      INT16 nTypeCode = CDlpTable_GetCompType(_this, j);

      switch (nTypeCode)
      {
        case T_UCHAR  : sprintf(buffer,"%4hhu "         ,(unsigned char )CDlpTable_Dfetch(_this,i,j)); break;
        case T_CHAR   : sprintf(buffer,"%4hhd "         ,(char          )CDlpTable_Dfetch(_this,i,j)); break;
        case T_USHORT : sprintf(buffer,"%6hu "          ,(unsigned short)CDlpTable_Dfetch(_this,i,j)); break;
        case T_SHORT  : sprintf(buffer,"%6hd "          ,(int           )CDlpTable_Dfetch(_this,i,j)); break;
        case T_UINT   : sprintf(buffer,"%12u "          ,(unsigned int  )CDlpTable_Dfetch(_this,i,j)); break;
        case T_INT    : sprintf(buffer,"%12d "          ,(int           )CDlpTable_Dfetch(_this,i,j)); break;
        case T_ULONG  : sprintf(buffer,"%12lu "         ,(unsigned long )CDlpTable_Dfetch(_this,i,j)); break;
        case T_LONG   : sprintf(buffer,"%12ld "         ,(long          )CDlpTable_Dfetch(_this,i,j)); break;
        case T_FLOAT  : sprintf(buffer,"%12.3f "        ,(double        )CDlpTable_Dfetch(_this,i,j)); break;
        case T_DOUBLE : /* FALL THROUGH */
        case T_COMPLEX: dlp_sprintc(buffer,CDlpTable_Cfetch(_this,i,j),TRUE); break;
        default       : sprintf(buffer,"%20s "          ,(char*         )CDlpTable_Sfetch(_this,i,j)); break;
      }

      printf("%s",buffer);
    }
    printf("\n");lines++;

    if(lines >= 40)
    {
      printf("--cont: <cr>, abort: <q>");

      if((scanf("%c",&dGetCharacter) == 1) && (dGetCharacter == 'q'))
      {
        printf("\n");
        return O_K;
      }

      printf("\n");;
      lines=0;
    }
    lines++;
  }

  printf("---- END of Record Sequence \n\n");

  return O_K;
}

/* EOF */

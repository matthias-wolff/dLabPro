/* dLabPro base library
 * - Support for deprecated identifiers and macros
 *
 * AUTHOR : Matthias Wolff
 * PACKAGE: dLabPro/sdk
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

#ifndef __OBSOLETE_H
#define __OBSOLETE_H

/* Deprecated macros, helpers & syntactic sugar */
#define IDENTIFY(A)
#define DLASSERT(A)  DLPASSERT(A)

/* Deprecated CGen scanner marks */
#define __PROT

/* Deprecated error codes */
#define NOMEM    ERR_NOMEM
#define CALLERR  ERR_BADARG

/* Deprecated return macros */
#define REFUSE_PARA return FALSE;

/* Deprecated dlab.lib (alias dlpbase) functions */
#define dl_calloc(A,B,C)           __dlp_calloc(A,B,__FILE__,__LINE__,"obsolete call",NULL)
#define dl_realloc(A,B,C,D)        __dlp_realloc(A,B,C,__FILE__,__LINE__,"obsolete call: "D,NULL)
#define dl_free(A)                 dlp_free(A)
#define dl_isNumericType(A)        dlp_is_numeric_type_code((INT16)A)
#define dl_isSymbolicType(A)       dlp_is_symbolic_type_code((INT16)A)
#define dl_memmove(A,B,C)          dlp_memmove(A,B,C)
#define dl_memset(A,B,C)           dlp_memset(A,B,C)
#define dl_aggrop_code(A)          dlp_aggrop_code(A)
#define dl_scalop(A,B,C)           dlp_scalop(A,B,C)
#define dl_scalop_code(A)          dlp_scalop_code(A)
#define dl_scalop_name(A)          dlp_scalop_name(A)
#define dl_strlen(A)               dlp_strlen(A)
#define dl_strcpy(A,B)             dlp_strcpy(A,B)
#define dl_strncpy(A,B,C)          dlp_strncpy(A,B,C)
#define dl_strncmp(A,B,C)          dlp_strncmp(A,B,C)
#define dl_initPrintStop()         dlp_init_printstop()
#define dl_ifPrintStop()           dlp_if_printstop()
#define dl_printDashedLine(A)      dlp_fprint_x_line(stdout,'-',A)
#define dl_getx(A,B)               dlp_getx(B,A)
#define dl_fetch(A,B)              dlp_fetch(A,B)

/* Deprecated data methods */
#define comp_text(A,B)             CData_GetCname(A,B)
#define comp_type(A,B)             CData_GetCompType(A,B)
#define comp_def(A,B,C)            CData_AddComp(A,B,(INT16)C)
#define comp_mdef(A,B,C)           CData_AddNcomps(A,(INT16)C,B)
#define copy_comp_text(A,B,C,D,E)  {INT64 i; for (i=0;i<E;i++) CData_SetCname(C,D+i,CData_GetCname(A,B+i));}
#define copy_data_descr(A,B)       CData_CopyDescr(B,A)        /* HACK: Correct order? */
#define data_alloc(A,B)            CData_Alloc(A,B)
#define data_arr_alloc(A,B)        CData_Allocate(A,B)
#define data_array(A,B,C,D)        CData_Array(A,D,B,C)
#define data_clear(A)              CData_Clear(A)
#define data_copy(A,B)             CData_Copy(B,A)             /* HACK: Correct order? */
#define data_copy_lcomp(A,B)       CData_CopyLabels(B,A)       /* HACK: Correct order? */
#define data_create(A)             CData_CreateInstance(A)
#define data_destroy(A)            IDESTROY(A)
#define data_dim(A)                CData_GetNComps(A)
#define data_ndim(A)               CData_GetNNumericComps(A)
#define data_empty(A)              CData_IsEmpty(A)
#define data_fill(A,B,C)           CData_Fill(A,B,C)
#define data_maxrec(A)             CData_GetMaxRecs(A)
#define data_nblock(A)             CData_GetNBlocks(A)
#define data_nrec(A)               CData_GetNRecs(A)
#define data_nrecblock(A)          CData_GetNRecsPerBlock(A)
#define data_print(A)              CData_Print(A)
#define data_realloc(A,B)          CData_Realloc(A,B)
#define data_reset(A)              CData_Reset(BASEINST(A),1)
#define data_scalop(A,B,C,D,E,F,G) CData_Scalop_Int(B,A,C,D,E,F,G)
#define data_scopy(A,B)            CData_Scopy(B,A)            /* HACK: Correct order? */
#define dcomp_fetch(A,B,C,D)       CData_DcompFetch(B,A,C,D)
#define dblock_fetch(A,B,C,D,E,F)  CData_DblockFetch(B,A,C,D,E,F)
#define dblock_store(A,B,C,D,E,F)  CData_DblockStore(B,A,C,D,E,F)
#define dfetch(A,B,C)              CData_Dfetch(A,B,C)
#define dstore(A,B,C,D)            CData_Dstore(B,A,C,D)
#define find_comp(A,B)             CData_FindComp(A,B)
#define sstore(A,B,C,D)            CData_Sstore(B,A,C,D)
#define dvec_fetch(A,B,C,D,E)      CData_DrecFetch(B,A,C,D,E)
#define dvec_store(A,B,C,D,E)      CData_DrecStore(B,A,C,D,E)
#define inc_data_nrec(A,B)         CData_IncNRecs(A,B)
#define set_data_nblock(A,B)       CData_SetNBlocks(A,B)
#define xaddr(A,B,C)               (char*)CData_XAddr(A,B,C)
#define data_ptr(A)                (char*)CData_XAddr(A,0,0)
#define data_chtype(A,B)           CData_CheckCompType(A,B)
#define data_descr(A,B)            CData_GetDescr(A,B)
#define set_data_descr(A,B,C)      CData_SetDescr(A,B,C)
#define set_data_nrec(A,B)         CData_SetNRecs(A,B)
#define data_reclen(A)             CData_GetRecLen(A)
#define data_indexlist(A,B,C,D,E)  CData_GenIndexList(D,A,B,C,E)
#define data_indexgen(A,B,C,D,E)   CData_GenIndex(C,A,B,D,E)
#define data_labindex(A,B,C,D)     CData_GenLabIndex(A,B,D,C)
#define lcomp_take(A,B)            {INT64 i;for(i=0;i<CData_GetNComps(A);i++)if(dlp_is_symbolic_type_code(CData_GetCompType(A,i)))CData_AddComp(B,CData_GetCname(A,i),CData_GetCompType(A,i));}
#define comp_len(A,B)              CData_GetCompSize(A,B)
#define d_operate(A,B,C,D,E)       CData_Dstore(B,dlp_scalop(CData_Dfetch(B,C,D),A,E),C,D)
#define data_copy_comp(A,B,C,D,E)  CData_CopyComps(B,A,C,D,E)
#define data_sort_up(A,B,C,D)      CData_SortInt(B,A,C,D,CDATA_SORT_UP)
#define data_sort_down(A,B,C,D)    CData_SortInt(B,A,C,D,CDATA_SORT_DOWN)
#define data_tconvert(A,B,C,D)     {DLPASSERT(D==-1);CData_Tconvert(B,A,C);}
#define set_comp_text(A,B,C)       CData_SetCname(A,C,B)
#define data_join(A,B,C,D)         CData_NJoin(B,A,C,D)
#define data_sel_comp(A,B,C,D)     CData_SelectComps(B,A,C,D)

#define CompDef(A,B)               AddComp(A,B)
#define CompMDef(A,B)              AddNcomps(B,A)
#define DFetch(A,B)                Dfetch(A,B)
#define DStore(A,B,C)              Dstore(A,B,C)
#define ArrAlloc(A)                Allocate(A)
#define GetCompText(A)             GetCname(A)
#define SStore(A,B,C)              Sstore(A,B,C)
#define DRecFetch(A,B,C,D)         DrecFetch(A,B,C,D)

/* Deprecated structure methods */
#define CalcTransProbabsUnit(A)    CalcTp(A)
#define GetNodeCount(A)            ud->Dfetch(A,OF_NNODES)     /* HACK: Does not behave exactly as GetNodeCount! */
#define GetNodeOffset(A)           ud->Dfetch(A,OF_FIRSTNODE)  /* HACK: Does not behave exactly as GetNodeOffset! */
#define GetTransCount(A)           ud->Dfetch(A,OF_NTRANS)     /* HACK: Does not behave exactly as GetTransCount! */
#define GetTransOffset(A)          ud->Dfetch(A,OF_FIRSTTRANS) /* HACK: Does not behave exactly as GetTransOffset! */

/* Deprecated base class members */
#define CSlCmdTarget     CDlpObject
#define SlcOfKind        CDlpObject_OfKind
#define SlcFindInstance  CDlpObject_FindInstance
#define SlcPushLogic     CDlpObject_PushLogic
#define m_instanceName   m_lpInstanceName
#define objectname       m_lpInstanceName

#endif /* if !defined __OBSOLETE_H */

/* EOF */

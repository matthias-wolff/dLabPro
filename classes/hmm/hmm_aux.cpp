// dLabPro class CHmm (hmm)
// - Auxilary methods
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

#include "dlp_hmm.h"                                                            // Include class header file

/**
 * Copies HMM units. The method copies the HMM topologies, the Gaussians and the
 * mixture map.
 *
 * <h4>Remarks</h4>
 * <ul>
 *   <li><span class="warning">Copying the mixture map is not yet implemented!
 *      </span></li>
 *   <li>The method resolves any covariance tying.</li>
 *   <li>The method destroys all internal statistics instances.</li>
 * </ul>
 *
 * @param itSrc
 *          Source instance
 * @param idIndex
 *          Sequence of HMM unit indices in <code>itSrc</code> to be copied, may
 *          be <code>NULL</code> indicating the method should only copy unit
 *          <code>nPar</code>.
 * @param nPar
 *          Interpretation depends on <code>idIndex</code>: if
 *          <code>idIndex</code> is <code>NULL</code>, <code>nPar</code> denotes
 *          the index of the HMM unit in <code>itSrc</code> to be copied,
 *          otherwise it denotes the component in <code>idIndex</code> that
 *          contains the unit indices to be copied.
 * @param bCat
 *          If <code>TRUE</code> the method will append the copied HMM units to
 *          this instance, if <code>FALSE</code> this instance will containe the
 *          copied HMM units <em>only</em>.
 * @return <code>O_K</code> if successful, a (negative) error code otherwise.
 */
INT16 CGEN_PUBLIC CHmm::CopyUiEx
(
  CHmm*  itSrc,
  CData* idIndex,
  INT32   nPar,
  BOOL   bCat
)
{
  CHmm*  itAux  = NULL;
  CData* idMsrc = NULL;                                                         // Buffer for FST units being copied
  CData* idMdst = NULL;                                                         // Mean vectors (dest)
  CData* idIsrc = NULL;                                                         // Inverse covariance matrices (source)
  CData* idIdst = NULL;                                                         // Inverse covariance matrices (dest)
  CData* idGmap = NULL;                                                         // Gaussian map
  CData* idAux1 = NULL;                                                         // Auxilary data instance #1
  CData* idAux2 = NULL;                                                         // Auxilary data instance #1
  INT32   N      = 0;                                                            // Feature space dim. of Gaussians
  INT32   nKoffs = 0;                                                            // New Gaussian index offset
  INT32   k      = -1;                                                           // Current source Gaussian
  INT32   i      = 0;                                                            // Generic loop counter
  INT32   nIcTis = -1;                                                           // Input symbol component
  INT16  nCheck = 0;                                                            // Buffer for check level
  INT16  nErr   = O_K;                                                          // Error state

  IFCHECK printf("\n   CHmm::CopyUiEx in %s mode",bCat?"cat":"copy");           // Protocol
  ICREATEEX(CHmm ,itAux ,"CHmm::CopyUiEx.itAux" ,NULL);                         // Create FST copy buffer
  ICREATEEX(CData,idMdst,"CHmm::CopyUiEx.idMdst",NULL);                         // Create mean vector buffer (dest.)
  ICREATEEX(CData,idIdst,"CHmm::CopyUiEx.idIdst",NULL);                         // Create inv. cov. mats. buffer (dst.)
  IFCHECK printf("\n   Copying HMMs");                                          // Protocol
  if (OK(nErr=CFst_CopyUi(itAux,itSrc,idIndex,nPar)) && itSrc->m_iGm)           // CopyUi of FST ok and have Gaussians
  {                                                                             // >>
    // Initialize                                                               //   - - - - - - - - - - - - - - - - -
    nIcTis = CData_FindComp(itAux->td,NC_TD_TIS);                               //   Find transducer input symbols
    N      = CGmm_GetDim(itSrc->m_iGm);                                         //   Get Gaussian dimensionality
    IFCHECK  printf("\n   Transducer input symbols in component %d",nIcTis);     //   Protocol
    if (nIcTis<0) DLPTHROW(NOT_EXEC);                                           //   No input syms.-> suspicious but ok
    if (itSrc->m_iGm->m_iMmap)                                                  //   NOT YET IMPLEMENTED (MIXTURE MAPS)
    {                                                                           //   >>
      IERROR(this,ERR_GENERIC,"Not implemented with mixture map",0,0);          //     Error
      DLPTHROW(ERR_GENERIC);                                                    //     Leave it
    }                                                                           //   <<
    ICREATEEX(CData,idMsrc,"CHmm::CopyUiEx.idMsrc",NULL);                       //   Create mean vector buffer (source)
    ICREATEEX(CData,idIsrc,"CHmm::CopyUiEx.idIsrc",NULL);                       //   Create inv.cov.mats. buffer (src.)
    ICREATEEX(CData,idGmap,"CHmm::CopyUiEx.idGmap",NULL);                       //   Create Gaussian map
    ICREATEEX(CData,idAux1,"CHmm::CopyUiEx.idAux1",NULL);                       //   Create auxilary data instance #1
    ICREATEEX(CData,idAux2,"CHmm::CopyUiEx.idAux2",NULL);                       //   Create auxilary data instance #2

    // Copy Gaussians                                                           //   - - - - - - - - - - - - - - - - -
    IFCHECK  printf("\n   Copying Gaussians");                                   //   Protocol
    if (bCat && this!=itSrc) CGmm_Extract(m_iGm,idMdst,idIdst);                 //   Extract dest. means and inv. covs.
    nKoffs = CData_GetNRecs(idMdst);                                            //   Get new Gaussian index offset
    CGmm_Extract(itSrc->m_iGm,idMsrc,idIsrc);                                   //   Extract source means and inv.covs.
    CData_Sortup(idGmap,itAux->td,nIcTis);                                      //   Sort used Gaussian indices
    CData_Compress(idGmap,idGmap,nIcTis);                                       //   RLE compress
    CData_DeleteComps(idGmap,1,2);                                              //   Delete RLE components
    for (i=0; i<CData_GetNRecs(idGmap); )                                       //   For all Gaussians to be copied
    {                                                                           //   >>
      k = (INT32)CData_Dfetch(idGmap,i,0);                                       //     Get source Gaussian index
      CData_SelectRecs  (idAux1,idMsrc,k,1);                                    //     Get mean vector
      CData_SelectBlocks(idAux2,idIsrc,k,1);                                    //     Get inverse covariance matrix
      if                                                                        //     Skip and delete map entry if ...
      (                                                                         //     |
        k<0 || k>=CGmm_GetNGauss(itSrc->m_iGm) ||                               //     | epsilon or invalid input symb.
        CData_GetNRecs(idAux1)!=1 || CData_GetNRecs(idAux2)!=N                  //     | invalid mean or inv.cov.matr.
      )                                                                         //     |
      {                                                                         //     >>
        CData_DeleteRecs(idGmap,i,1);                                           //       Remove map entry
        continue;                                                               //       Skip
      }                                                                         //     <<
      IFCHECK  printf("\n   - src: %ld -> dst: %ld",(long)k,(long)(nKoffs+i));  //     Protocol
      CData_Cat(idMdst,idAux1);                                                 //     Append mean vector to dest.
      CData_Cat(idIdst,idAux2);                                                 //     Append inv. cov. matrix to dest.
      CData_SetNBlocks(idIdst,CData_GetNRecs(idIdst)/CData_GetNComps(idIdst));  //     Adjust block number
      DLPASSERT(CData_GetNRecs(idMdst)==CData_GetNBlocks(idIdst));              //     Check consistency
      i++;                                                                      //     Increment loop counter
    }                                                                           //   <<

    // Map Gaussian indices                                                     //   - - - - - - - - - - - - - - - - -
    IFCHECK  printf("\n   Mapping transducer input symbols (Gaussian IDs)");    //   Protocol
    CData_GenIndex(idAux1,itAux->td,idGmap,nIcTis,0);                           //   Pass source Gaussian IDs thru map
    for (i=0; i<CData_GetNRecs(itAux->td); i++)                                 //   Loop over trans. of copied HMMs
    {                                                                           //   >>
      k = (INT32)CData_Dfetch(idAux1,i,0);                                      //     Get Gaussian index
      if (k>=0) CData_Dstore(itAux->td,k+nKoffs,i,nIcTis);                      //     Map Gaussian index
    }                                                                           //   <<

    // Clean up                                                                 //   - - - - - - - - - - - - - - - - -
    IDESTROY(idMsrc);                                                           //   Destroy mean vector buffer (src.)
    IDESTROY(idIsrc);                                                           //   Destroy inv.cov.mats.buff. (src.)
    IDESTROY(idGmap);                                                           //   Destroy Gaussian map
    IDESTROY(idAux1);                                                           //   Destroy auxilary data instance #1
    IDESTROY(idAux2);                                                           //   Destroy auxilary data instance #2
  }                                                                             // <<

DLPCATCH(NOT_EXEC)                                                              // == Catch silent exception
DLPCATCH(ERR_GENERIC)                                                           // == Catch ERR_GENERIC exception
  IFCHECK printf("\n   %s HMMs",bCat?"Catting":"Copying");                      // Protocol

  // Cat or copy HMM units                                                      // ------------------------------------
  nCheck = m_nCheck;                                                            // Save check level
  if (bCat) CFst_Cat(this,itAux); else CFst_Copy(this,itAux);                   // Cat or copy HMM units
  m_nCheck = nCheck;                                                            // Restore check level

  // Setup new Gaussians                                                        // ------------------------------------
  if (!CData_IsEmpty(idMdst) && !CData_IsEmpty(idIdst))                         // Have new Gaussians >>
  {                                                                             // >>
    IFCHECK  printf("\n   Setting up new Gaussians");                            //   Protocol
    IFIELD_RESET(CGmm,"gm");                                                    //   Reset or create dest. Gaussians
    ISETOPTION(m_iGm,"/icov");                                                  //   Set /icov option
    CGmm_Setup(m_iGm,idMdst,idIdst,NULL);                                       //   Setup destination Gaussians
    IRESETOPTIONS(m_iGm);                                                       //   Clear /icov option
  }                                                                             // <<

  // Finsih result and clean up                                                 // ------------------------------------
  IFCHECK  printf("\n   Clearing statistics'");                                  // Protocol
  CStatistics_Reset(m_iPfsm,TRUE);                                              // Clear most sign. feature stats.
  CStatistics_Reset(m_iPfsl,TRUE);                                              // Clear least sign. feature stats.
  CStatistics_Reset(m_iPms ,TRUE);                                              // Clear per models stats.
  CStatistics_Reset(m_iPss ,TRUE);                                              // Clear per state stats.
  IDESTROY(itAux );                                                             // Destroy FST copy buffer
  IDESTROY(idMdst);                                                             // Destroy mean vector buffer (dst.)
  IDESTROY(idIdst);                                                             // Destroy inv.cov.mats.buffer (dst.)
  return nErr;                                                                  // Return error state
}

/**
 * Wrapper method for the HMM implementation of all automaton operations.
 * <h4>Remarks</h4>
 * <ul>
 *   <li>Gaussians, statistics and all other HMM-specific fields will be copied
 *     from <code>itSrc1</code> if this operand is an instance of class
 *     <code>hmm</code>.</li>
 *   <li>If the first operand (<code>itSrc1</code>) is <em>not</em> an
 *     <code>hmm</code>-instance, Gaussians, statistics and all other
 *     HMM-specific fields will be copied from <code>itSrc2</code> if that
 *     operand is an instance of class <code>hmm</code>.</li>
 *   <li>If neither <code>itSrc1</code> nor <code>itSrc2</code> are
 *     <code>hmm</code> instances, the method will keep the HMM specific fields
 *     of <code>this</code> instance.
 * </ul>
 *
 * @param itSrc1 Pointer to operand 1 (CHmm or CFst, may be <code>NULL</code>)
 * @param itSrc2 Pointer to operand 2 (CHmm or CFst, may be <code>NULL</code>)
 * @param nPar1  Numeric parameter 1, meaning depends on <code>nOpc</code>
 * @param nPar2  Numeric parameter 2, meaning depends on <code>nOpc</code>
 * @param nOpc   Operation code, one of the <code>AOP_XXX</code> constants
 * @return <code>O_K</code> if successfull, a (negative) error code otherwise
 */
INT16 CGEN_PUBLIC CHmm::Op
(
  CFst*  itSrc1,
  CFst*  itSrc2,
  FLOAT64 nPar1,
  FLOAT64 nPar2,
  INT16  nOpc
)
{
  CFst* itDst = NULL;
  INT16 nErr  = O_K;

  ICREATEEX(CFst,itDst,"CHmm::Op~itDst",NULL);
  CFst_Copy(itDst,this); CDlpObject_CopyAllOptions(itDst,this);
  if      ((CHmm*)OfKind("hmm",itSrc1)!=this) Copy((CHmm*)OfKind("hmm",itSrc1));
  else if ((CHmm*)OfKind("hmm",itSrc2)!=this) Copy((CHmm*)OfKind("hmm",itSrc2));
  switch(nOpc)
  {
  case AOP_UNION : nErr=CFst_Union      (itDst,itSrc1                               ); break;
  case AOP_INTERS: nErr=CFst_Intersect  (itDst,itSrc1,itSrc2,(INT32)nPar1,(INT32)nPar2); break;
  case AOP_CLOSE : nErr=CFst_Close      (itDst,itSrc1,(INT32)nPar1                   ); break;
  case AOP_PROD  : nErr=CFst_Product    (itDst,itSrc1,itSrc2,(INT32)nPar1,(INT32)nPar2); break;
  case AOP_CMPS  : nErr=CFst_Compose    (itDst,itSrc1,itSrc2,(INT32)nPar1,(INT32)nPar2); break;
  case AOP_DET   : nErr=CFst_Determinize(itDst,itSrc1,(INT32)nPar1                   ); break;
  case AOP_MIN   : nErr=CFst_Minimize   (itDst,itSrc1,(INT32)nPar1                   ); break;
  case AOP_ERM   : nErr=CFst_Epsremove  (itDst,itSrc1,(INT32)nPar1                   ); break;
  case AOP_HMM   : nErr=CFst_Hmm        (itDst,itSrc1,(INT32)nPar1                   ); break;
  case AOP_TREE  : nErr=CFst_Tree       (itDst,itSrc1,(INT32)nPar1                   ); break;
  case AOP_BESTN : nErr=CFst_BestN      (itDst,itSrc1,(INT32)nPar1,(INT32)nPar2,*((INT32*)itSrc2)); break;
  case AOP_INV   : CFst_Copy(itDst,itSrc1); nErr=CFst_Invert  (itDst,(INT32)nPar1    ); break;
  case AOP_PROJ  : CFst_Copy(itDst,itSrc1); nErr=CFst_Project (itDst                ); break;
  case AOP_WRM   : CFst_Copy(itDst,itSrc1); nErr=CFst_Unweight(itDst                ); break;
  default: DLPASSERT(FMSG("Unknown automaton operation"));
  }

  if (OK(nErr)) CopyFst(itDst); else Reset();
  IDESTROY(itDst);
  return nErr;
}

// EOF

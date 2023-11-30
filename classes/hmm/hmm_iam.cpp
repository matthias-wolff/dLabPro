// dLabPro class CHmm (hmm)
// - Interactive methods
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

/*
 * Manual page at hmm.def
 */
INT16 CGEN_PUBLIC CHmm::SetupEx
(
  INT32   nMsf,
  CData* idHmms,
  INT32   nLsf,
  INT32   nPlf,
  INT32   nPsf,
  INT32   nPmf
)
{
  INT32 nU  = 0;                                                                 // Current unit (HMM)
  INT32 nS  = 0;                                                                 // Current state
  INT32 nXS = 0;                                                                 // Number of states
  INT32 K   = 0;                                                                 // Number of Gaussians

  // Zero-initialize                                                            // ------------------------------------
  Reset();                                                                      // Reset HMM instance
  CData_AddComp(sd,"~LYR"   ,T_LONG             );                              // HMM layer
  CData_AddComp(td,NC_TD_TIS,DLP_TYPE(FST_STYPE));                              // Add input symbol component
  CData_AddComp(td,NC_TD_TOS,DLP_TYPE(FST_STYPE));                              // Add output symbol component
  CData_AddComp(td,NC_TD_LSR,DLP_TYPE(FST_WTYPE));                              // Add weight component
  CData_AddComp(td,NC_TD_RC ,T_LONG             );                              // Add reference counter component

  // Validate                                                                   // ------------------------------------
  if (CData_IsEmpty(idHmms)) return IERROR(this,ERR_INVALARG,"idHmms",0,0);     // No HMM list, no service
  if (!dlp_is_symbolic_type_code(CData_GetCompType(idHmms,0)))                  // Missing model name component
    return IERROR(this,FST_BADCTYPE,"HMM",0,0);                                 //   -> Error
  if (!dlp_is_numeric_type_code(CData_GetCompType(idHmms,1)))                   // Missing number-of-states component
    return IERROR(this,FST_BADCTYPE,"HMM",1,0);                                 //   -> Error

  // Create units                                                               // ------------------------------------
  for (nU=0; nU<CData_GetNRecs(idHmms); nU++)                                   // Loop over model list
  {                                                                             // >>
    nXS = (INT32)CData_Dfetch(idHmms,nU,1);                                      //   Get number of states
    CFst_Addunit(this,(const char*)CData_XAddr(idHmms,nU,0));                   //   Add a unit
    if (nXS<=0) continue;                                                       //   Just a label, don't create an HMM
    CFst_Addstates(this,nU,nXS+2,FALSE);                                        //   Add states
    SD_FLG(this,UD_XXS(this)-1)|=SD_FLG_FINAL;                                  //   Make last state final
    CData_Dstore(sd,-1.,UD_FS(this,nU),IC_SD_DATA);                             //   Store HMM layer
    for (nS=1; nS<UD_XS(this,nU)-1; nS++,K++)                                   //   Loop over states
    {                                                                           //   >>
      CData_Dstore(sd,nS-1.,UD_FS(this,nU)+nS,IC_SD_DATA);                      //     Store HMM layer
      CFst_AddtransEx(this,nU,(FST_ITYPE)nS,(FST_ITYPE)nS,(FST_STYPE)K,-1,0.);                                 //     Add loop transiton
      CFst_AddtransEx(this,nU,(FST_ITYPE)(nS-1),(FST_ITYPE)nS,(FST_STYPE)K,-1,0.);                                 //     Add layer transition
    }                                                                           //   >>
    CData_Dstore(sd,-1.,UD_FS(this,nU)+nS,IC_SD_DATA);                          //   Store HMM layer
    CFst_AddtransEx(this,nU,(FST_ITYPE)(nS-1),(FST_ITYPE)nS,-1,(FST_STYPE)nU,0.);                                  //   Add exit transiton
  }                                                                             // <<

  // Create statistics                                                          // ------------------------------------
  if (nMsf>0)                                                                   // Use most significant features?
  {                                                                             // >>
    IFIELD_RESET(CStatistics,"pfsm");                                           //   Create/reset statistics
    CStatistics_Setup(m_iPfsm,4,nMsf,K,NULL,0);                                 //   Setup statistics
  }                                                                             // <<
  if (nLsf>0)                                                                   // Use least significant features?
  {                                                                             // >>
    IFIELD_RESET(CStatistics,"pfsl");                                           //   Create/reset statistics
    CStatistics_Setup(m_iPfsl,4,nLsf,K,NULL,0);                                 //   Setup statistics
  }                                                                             // <<
  if (nPlf>0)                                                                   // Use per HMM layer features?
  {                                                                             // >>
    IFIELD_RESET(CStatistics,"pls");                                            //   Create/reset statistics
    CStatistics_Setup(m_iPls,4,nPlf,K,NULL,0);                                  //   Setup statistics
  }                                                                             // <<
  if (nPsf>0)                                                                   // Use per HMM state features?
  {                                                                             // >>
    IFIELD_RESET(CStatistics,"pss");                                            //   Create/reset statistics
    CStatistics_Setup(m_iPss,4,nPsf,K,NULL,0);                                  //   Setup statistics
  }                                                                             // <<
  if (nPmf>0)                                                                   // Use per HMM layer features?
  {                                                                             // >>
    IFIELD_RESET(CStatistics,"pms");                                            //   Create/reset statistics
    CStatistics_Setup(m_iPms,4,nPmf,nU,NULL,0);                                 //   Setup statistics
  }                                                                             // <<

  return O_K;                                                                   // All done.
}

INT16 CGEN_PRIVATE CHmm::GrcNormalize()
{
  INT32 nR, nC;
  INT32 nNR, nNC;
  FST_WTYPE *lpGrc;
  FST_WTYPE nSum;
  INT16 nWsr;
  if(m_idGrc->IsHomogen()!=DLP_TYPE(FST_WTYPE))
    m_idGrc->Tconvert(m_idGrc,DLP_TYPE(FST_WTYPE));
  nNR=m_idGrc->GetNRecs();
  nNC=m_idGrc->GetNComps();
  lpGrc=(FST_WTYPE*)m_idGrc->XAddr(0,0);
  nWsr=Wsr_GetType(NULL);
  for(nR=0;nR<nNR;nR++){
    nSum=Wsr_NeAdd(nWsr);
    for(nC=0;nC<nNC;nC++)
      if(nWsr==FST_WSR_LOG || nWsr==FST_WSR_TROP)
        nSum = dlp_scalop(nSum, lpGrc[nR*nNC+nC], OP_LSADD);
      else nSum += lpGrc[nR*nNC+nC];
    for(nC=0;nC<nNC;nC++)
      if(nWsr==FST_WSR_LOG || nWsr==FST_WSR_TROP) lpGrc[nR*nNC+nC] -= nSum;
      else lpGrc[nR*nNC+nC] /= nSum;
  }
  return O_K;
}

/*
 * Manual page at hmm.def
 */
INT16 CGEN_PUBLIC CHmm::SetupGmm(FLOAT64 nMindet)
{
  BOOL         bFullCov = TRUE;                                                 // Full covariances flag
  INT32        C        = 0;                                                    // Number of indep. covariance matrices
  INT32        K        = 0;                                                    // Number of Gaussians/stats. classes
  CData*       idMean   = NULL;                                                 // Mean vector set
  CData*       idCov    = NULL;                                                 // Covariance matrix set
  CVmap*       iMmap    = NULL;                                                 // Gaussian mixture map
  CData*       idCmap   = NULL;                                                 // Covariance tying map
  CData*       idVar    = NULL;                                                 // Individual variance vector set
  CStatistics* iPool    = NULL;                                                 // Pooled stats. for tied cov. matrices

  IFCHECK                                                                       // On verbose level 1
  {                                                                             // >>
    printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());             //   Protocol header
    printf("\n   Method -setup_gmm\n   hmm %s",_this->m_lpInstanceName);        //   ...
    printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());             //   ...
  }                                                                             // <<

  // Validation                                                                 // ------------------------------------
  if (!m_iPfsm || !m_iPfsm->GetNSamples())                                      // No most significant feature stats.
    return IERROR(this,FST_MISS,"statistics data","","field pfsm");             //   ... no service!

  // Extract feature statistics data                                            // ------------------------------------
  ICREATEEX(CData,idMean,"CHmm::SetupGmm.~idMean",NULL);                        // Create mean vector set
  ICREATEEX(CData,idCov ,"CHmm::SetupGmm.~idCov" ,NULL);                        // Create covariance matrix set
  K = CStatistics_GetNClasses(m_iPfsm);                                         // Get number of statistics classes
  IFCHECK printf("\n   - Estimating %ld new single Gaussians",(long)K);         // Protocol (verbose level 1)
  m_iPfsm->m_nCheck = m_nCheck;                                                 // Propagate verbose level
  IFCHECK printf("\n\n>> Calling CStatistics_Mean");                            // Protocol (verbose level 1)
  CStatistics_Mean(m_iPfsm,idMean);                                             // Estimate mean vectors
  if (m_bVar)
  {
  	IFCHECK printf("\n<<\n\n>> Calling CStatistics_Var");                       //   Protocol (verbose level 1)
  	CStatistics_Var(m_iPfsm,idCov);                                             //   Estimate variance vectors
  }
  else
  {
  	IFCHECK printf("\n<<\n\n>> Calling CStatistics_Cov");                       //   Protocol (verbose level 1)
  	CStatistics_Cov(m_iPfsm,idCov);                                             //   Estimate covariance matrices
  }
  IFCHECK printf("\n<<\n");                                                     // Protocol (verbose level 1)
  m_iPfsm->m_nCheck = 0;                                                        // Reset verbose level

  // Extract Gaussian properties                                                // ------------------------------------
  if (m_iGm && OK(CGmm_Check(m_iGm)) && K==CGmm_GetNGauss(m_iGm))               // Are there intact Gaussians?
  {                                                                             // >> (Yes)
    // Handle mixture map                                                       //   - - - - - - - - - - - - - - - - -
    if (m_iGm->m_iMmap)                                                         //   Have mixture map?
    {                                                                           //   >> (Yes)
      if(m_idGrc && m_idGrc->GetNComps()==K)                                    //     Have valid Gaussian RC's
      {                                                                         //     >>
        IFCHECK printf("\n   - Generating new mixture map");                    //       Protocol (verbose level 1)
        ICREATEEX(CVmap,iMmap,"CHmm::SetupGmm.~iMmap",NULL);                    //       Create
        GrcNormalize();                                                         //       Make comp. sum one
        CVmap_Setup(iMmap,m_idGrc,                                              //       Setup with G. RC's
          dlp_scalop_sym(m_iGm->m_iMmap->m_nAop),                               //       |
          dlp_scalop_sym(m_iGm->m_iMmap->m_nWop),                               //       |
          m_iGm->m_iMmap->m_nZero);                                             //       |
      }                                                                         //     <<
      else                                                                      //     No valid Gaussians RC's
      {                                                                         //     >>
        IFCHECK printf("\n   - Copying mixture map");                           //       Protocol (verbose level 1)
        ICREATEEX(CVmap,iMmap,"CHmm::SetupGmm.~iMmap",NULL);                    //       Make a copy
        CVmap_Copy(BASEINST(iMmap),BASEINST(m_iGm->m_iMmap));                   //       ...
      }                                                                         //     <<
    }                                                                           //   <<

    // Estimate individual variance vectors                                     //   - - - - - - - - - - - - - - - - -
    if                                                                          //   Individual var. vectors present
    (                                                                           //   |
      !CData_IsEmpty(m_iGm->m_idIvar) &&                                        //   |
      m_iGm->m_idIvar->m_lpTable->m_descr0==1                                   //   |
    )                                                                           //   |
    {                                                                           //   >>
      IFCHECK printf("\n   - Estimating individual variance vectors");          //     Protocol (verbose level 1)
      ICREATEEX(CData,idVar,"CHmm::SetupGmm.~idVar",NULL);                      //     Create variance vector set
      CStatistics_Var(m_iPfsm,idVar);                                           //     Estimate variance vectors
    }                                                                           //   <<

    bFullCov = !CData_IsEmpty(AS(CData,m_iGm->m_idIcov));                       // Have full covariance matrices?

    // Pool covariance matrices                                                 //   - - - - - - - - - - - - - - - - -
    C = bFullCov ? CGmm_GetNIcov(m_iGm) : CData_GetNRecs(m_iGm->m_idIvar);      //   Get number of indep. cov. matrs.
    if (C<K)                                                                    //   Covariance tying
    {                                                                           //   >>
      ICREATEEX(CStatistics,iPool,"CHmm::SetupGmm.~iPool",NULL);                //     Create pooled statistics
      CStatistics_Pool(iPool,m_iPfsm,C==1?NULL:m_iGm->m_idCmap);                //     Pool statistics
      if (bFullCov)                                                             //     Full covariance matrices
      {                                                                         //     >>
        IFCHECK printf("\n   - Estimating %ld tied covariance matrices",(long)C);//       Protocol (verbose level 1)
        CStatistics_Cov(iPool,idCov);                                           //       Get tied covariance matrices
      }                                                                         //     <<
      else                                                                      //     Diagonal covariance matrices
      {                                                                         //     >>
        IFCHECK printf("\n   - Estimating %ld tied variance vectors",(long)C);  //       Protocol (verbose level 1)
        CStatistics_Var(iPool,idCov);                                           //       Get tied variance vectors
      }                                                                         //     <<
      IDESTROY(iPool);                                                          //     Destroy pooled statistics
      ICREATEEX(CData,idCmap,"CHmm::SetupGmm.~idCmap",NULL);                    //     Create covariance tying map
      CData_Copy(BASEINST(idCmap),BASEINST(m_iGm->m_idCmap));                   //     Copy from GMMs
    }                                                                           //   <<

    // Handle diagonal covariance matrices                                      //   - - - - - - - - - - - - - - - - -
    else if (!bFullCov && K==C)                                                 //   No covariance tying
    {                                                                           //   >>
      IFCHECK printf("\n   - Estimating %ld variance vectors",(long)C);         //     Protocol (verbose level 1)
      CStatistics_Var(m_iPfsm,idCov);                                           //     Get variance vectors
    }                                                                           //   <<
    else if (K!=C) DLPASSERT(FMSG("GMM covariance tying corrupt"));             //   This cannot happen
  }                                                                             // <<
  else if (m_iGm && K!=CGmm_GetNGauss(m_iGm))                                   // Wrong number of Gaussians
    IERROR(this,HMM_DISCARDGMM," (wrong number of GMMs)",0,0);                  //   Can't use 'em -> error message
  else if (m_iGm)                                                               // There are only corrupt Gaussians
    IERROR(this,HMM_DISCARDGMM," (GMMs corrupt)",0,0);                          //   Can't use 'em -> error message

  // Setup Gaussians                                                            // ------------------------------------
  IFCHECK printf("\n   - Setting up GMMs\n\n>> Calling CGmm_SetupEx");          // Protocol (verbose level 1)
  IFIELD_RESET(CGmm,"gm");                                                      // Create/reset GMM's
  m_iGm->m_nCheck=m_nCheck;                                                     // Propagate verbose level
  if (nMindet>0) m_iGm->m_nMindet = nMindet;                                                   // Set min. cov. matrix determinant
  CGmm_SetupEx(m_iGm,idMean,idCov,iMmap,idCmap,idVar);                          // Setup Gaussians
  m_iGm->m_nCheck=0;                                                            // Reset verbose level
  IFCHECK printf("\n<<\n");                                                     // Protocol (verbose level 1)

  // Clean up                                                                   // ------------------------------------
  IFCHECK printf("\n   - Cleaning up");                                         // Protocol (verbose level 1)
  IDESTROY(idMean);                                                             // Destroy mean vector set
  IDESTROY(idVar );                                                             // Destroy individual variance vec. set
  IDESTROY(idCov );                                                             // Destroy covariance matrix set
  IDESTROY(iMmap );                                                             // Destroy Gaussian mixture map
  IDESTROY(idCmap);                                                             // Destroy covariance tying map
  IFCHECK                                                                       // On verbose level 1
  {                                                                             // >>
    printf("\n");dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());printf("\n"); //   Protocol footer
  }                                                                             // <<
  return O_K;                                                                   // Ok
}

struct sTperG                                                                   // Structure for one trans. for Gauss.
{                                                                               // >>
  INT32 nT;                                                                      //   Absolute trans. index
  INT32 nU;                                                                      //   Unit index
};                                                                              // >>

struct ssTperG                                                                  // Structure for trans.'s for Gauss.
{                                                                               // >>
  struct sTperG* lpT;                                                           //   Array of trans. for Gauss.
  INT32 nNT;                                                                     //   Number of trans.'s
};                                                                              // >>

/*
 * Compare two Gaussians according there transitions, where they occur.
 */
char CGEN_PRIVATE CHmm::GmmMix_CompareGauss(struct ssTperG *lpTperG1, struct ssTperG *lpTperG2)
{
  INT32   nT1;                                                                   // Trans. index for Gaussian #1
  INT32   nT2;                                                                   // Trans. index for Gaussian #2
  char   bFoundT = 1;                                                           // Found similiar trans.
  char*  lpT2used;                                                              // Trans. used at Gaussian #2

  if(lpTperG1->nNT!=lpTperG2->nNT) return 0;                                    // Number of trans. doesn't match
  lpT2used = (char *)dlp_calloc(lpTperG1->nNT,sizeof(char));                    // Get memory for used-array

  for(nT1=0;nT1<lpTperG1->nNT && bFoundT;nT1++)                                 // Loop over all trans. of Gaussian #1
  {                                                                             // >>
    bFoundT = 0;                                                                //   No similiar trans. found until now
    for(nT2=0;nT2<lpTperG1->nNT && !bFoundT;nT2++) if(!lpT2used[nT2])           //   Loop over unused trans. of G. #2
    {                                                                           //   >>
      bFoundT = lpTperG1->lpT[nT1].nU==lpTperG2->lpT[nT2].nU &&                 //     Compare trans. accoring unit,
        TD_TER(_this,lpTperG1->lpT[nT1].nT) ==                                  //     | terminal state
        TD_TER(_this,lpTperG2->lpT[nT2].nT) &&                                  //     | and
        TD_INI(_this,lpTperG1->lpT[nT1].nT) ==                                  //     | initial state
        TD_INI(_this,lpTperG2->lpT[nT2].nT);                                    //     |
      lpT2used[nT2] = bFoundT;                                                  //     Update used trans.
    }                                                                           //   <<
  }                                                                             // <<

  dlp_free(lpT2used);                                                           // Free memory for used-array
  return bFoundT;                                                               // Return whether all trans. match
}

/*
 * Create a mixture for a set of Gaussians (definded by lpGinMix[x]==nG)
 */
void CGEN_PRIVATE CHmm::GmmMix_CreateMixture(INT32 *lpGinMix,INT32 nNG,INT32 nG,struct ssTperG* lpTperG,INT32 nIcW,INT16 nWsrt,CData *idTmx,INT32 nNGnew,char *lpTDel){
  INT32       nNM=0;                                                             // Number of Gaussians in mixture
  INT32       nNT;                                                               // Number of transitions where G. occur
  INT32       nM;                                                                // Index of mixture component
  INT32       nT;                                                                // Index of transition
  INT32*      lpG;                                                               // Map from mixt. index to old G. index
  FST_WTYPE* lpWold;                                                            // Old trans. weights
  FST_WTYPE* lpRnew;                                                            // New mixture rate relations
  FST_WTYPE  nRnewSum;                                                          // Sum of relations
  FST_WTYPE* lpMnew;                                                            // New mixture rates
  FST_WTYPE* lpWnew;                                                            // New transition weights
  INT32       nRbase;                                                            // G. index as relation base
  char       bFound;                                                            // Detection boolean

  nNT = lpTperG[nG].nNT;                                                        // Get number of transitions

  // Get number and index of Gaussians to mix                                   // ------------------------------------
  lpG = (INT32 *)dlp_calloc(nNG-nG,sizeof(INT32));                                // Alloc memory for G. map
  for(nM=nG;nM<nNG;nM++) if(lpGinMix[nM]==nG) lpG[nNM++]=nM;                    // Create G. map and get number of G.

  // Read and Convert old transition weights                                    // ------------------------------------
  lpWold = (FST_WTYPE *)dlp_calloc(nNM*nNT,sizeof(FST_WTYPE));                  // Alloc memory for old weights
  for(nM=0;nM<nNM;nM++) for(nT=0;nT<nNT;nT++)                                   // Loop over all old transitions
  {                                                                             // >>
    lpWold[nM*nNT+nT] = CData_Dfetch(td,lpTperG[lpG[nM]].lpT[nT].nT,nIcW);      //   Read weights from trans. table
    switch(nWsrt)                                                                //   Check WSR type (Convert weights)
    {                                                                           //   >>
      case FST_WSR_PROB: break;                                                 //     probability -> nothing to do
      case FST_WSR_LOG :                                                        //     logarithmic -> see tropical
      case FST_WSR_TROP: lpWold[nM*nNT+nT]=exp(-lpWold[nM*nNT+nT]); break;      //     tropical -> convert weight
      case FST_WSR_NONE: DLPASSERT(FMSG(Unweighted semiring type));break;       //     Unweighted
      default          : DLPASSERT(FMSG(Unknown weight semiring type));         //     NEW WEIGHT SEMIRING TYPE?
    }                                                                           //   <<
  }                                                                             // <<
  IFCHECKEX(2) for(nT=0;nT<nNM*nNT;nT++)                                        // Protocol
    fprintf(stdout,"%c%6.4f",nT%nNT==0?'\n':' ',(double)lpWold[nT]);                    // |

  // Find a non-zero base weight for relations                                  // ------------------------------------
  for(nRbase=0;nRbase<nNM-1;nRbase++)                                           // Loop over all mixture components
  {                                                                             // >>
    bFound=1;                                                                   //   Until now all trans. ok
    for(nT=0;nT<nNT && bFound;nT++) if(lpWold[nRbase*nNT+nT]==0.) bFound=0;     //   Trans. weight ok?
    if(bFound) break;                                                           //   All trans. weig. ok -> found base
  }                                                                             // <<
  IFCHECKEX(2) printf("\n%3i",nRbase);                                          // Protocol

  // Calc Relation between new mixture rates                                    // ------------------------------------
  lpRnew = (FST_WTYPE *)dlp_calloc(nNM,sizeof(FST_WTYPE));                      // Alloc memory for relations
  nRnewSum = 0;                                                                 // Initializise sum
  for(nM=0;nM<nNM;nM++) if(nM!=nRbase)                                          // Loop over all mix. comp. expect base
  {                                                                             // >>
    lpRnew[nM] = 1.;                                                            //   Init relation
    bFound = 0;                                                                 //   Calc nothing until now
    for(nT=0;nT<nNT;nT++) if(lpWold[nRbase*nNT+nT]!=0. && lpWold[nM*nNT+nT]!=0.)//   Loop over trans. with weig. != 0
    {                                                                           //   >>
      FST_WTYPE nRatio = 1./(FST_WTYPE)nNT;  // TODO: something intelegent ??   //     Set ratio for trans.
      lpRnew[nM] *= pow(lpWold[nM*nNT+nT]/lpWold[nRbase*nNT+nT],nRatio);        //     Add trans. rela. to mix. rela.
      bFound = 1;                                                               //     Calc'ed something
    }                                                                           //   <<
    if(!bFound) lpRnew[nM] = 0.;                                                //   Nothing calc'ed -> rela. = 0
    nRnewSum += lpRnew[nM];                                                     //   Update sum
  }                                                                             // <<
  IFCHECKEX(2) for(nM=0;nM<nNM;nM++)                                            // Protocol
    fprintf(stdout,"%c%6.2f",nM==1?'\n':' ',(double)lpRnew[nM-1]);                      // |

  // Calc new mixture rates                                                     // ------------------------------------
  lpMnew = (FST_WTYPE *)dlp_calloc(nNM,sizeof(FST_WTYPE));                      // Alloc memory for mixture rates
  lpMnew[nRbase] = 1./(1.+nRnewSum);                                            // Calc base rate from rela. sum
  for(nM=0;nM<nNM;nM++) if(nM!=nRbase) lpMnew[nM] = lpRnew[nM]*lpMnew[nRbase];  // Calc other rates from relations
  IFCHECKEX(2) for(nM=0;nM<nNM;nM++)                                            // Protocol
    fprintf(stdout,"%c%6.2f",nM==0?'\n':' ',(double)lpMnew[nM]);                        // |
  dlp_free(lpRnew);                                                             // Free memory for relations

  // Store new mixture rates                                                    // ------------------------------------
  for(nM=0;nM<nNM;nM++) CData_Dstore(idTmx,lpMnew[nM],nNGnew,lpG[nM]);          // Copy mixture rates to data object
  dlp_free(lpMnew);                                                             // Free memory for mixture rates

  // Calc new transition weights                                                // ------------------------------------
  lpWnew = (FST_WTYPE *)dlp_calloc(nNT,sizeof(FST_WTYPE));                      // Alloc memory for new trans. weights
  for(nT=0;nT<nNT;nT++)                                                         // Loop over all trans.
  {                                                                             // >>
    lpWnew[nT] = 0.;                                                            //   Init new weight
    for(nM=0;nM<nNM;nM++) lpWnew[nT] += lpWold[nM*nNT+nT];                      //   Get sum of old weights for new one
  }                                                                             // <<
  IFCHECKEX(2) for(nT=0;nT<nNT;nT++)                                            // Protocol
    fprintf(stdout,"%c%6.2f",nT==0?'\n':' ',(double)lpWnew[nT]);                        // |
  dlp_free(lpWold);                                                             // Free memory for old trans. weights

  // Convert and Store new transition weights                                   // ------------------------------------
  for(nT=0;nT<nNT;nT++)                                                         // Loop over all transitions
  {                                                                             // >>
    switch(nWsrt)                                                               //   Check WSR type (Convert weights)
    {                                                                           //   >>
      case FST_WSR_PROB: break;                                                 //     probability -> nothing to do
      case FST_WSR_LOG :                                                        //     logarithmic -> see tropical
      case FST_WSR_TROP: lpWnew[nT]=-log(lpWnew[nT]); break;                    //     tropical -> convert weight
      case FST_WSR_NONE: DLPASSERT(FMSG(Unweighted semiring type));break;       //     Unweighted
      default          : DLPASSERT(FMSG(Unknown weight semiring type));         //     NEW WEIGHT SEMIRING TYPE?
    }                                                                           //   <<
    CData_Dstore(td,lpWnew[nT],lpTperG[nG].lpT[nT].nT,nIcW);                    //   Store new weight in trans. table
  }                                                                             // <<
  IFCHECKEX(2) for(nT=0;nT<nNT;nT++)                                            // Protocol
    fprintf(stdout,"%c%6.2f",nT==0?'\n':' ',(double)lpWnew[nT]);                        // |
  dlp_free(lpWnew);                                                             // Free memory for new trans. weights

  // Mark edges for deletion                                                    // ------------------------------------
  for(nM=1;nM<nNM;nM++) for(nT=0;nT<nNT;nT++)                                   // Loop over mixtures and trans.'s
    lpTDel[lpTperG[lpG[nM]].lpT[nT].nT] = 1;                                    //   Mark trans. to delete
  dlp_free(lpG);                                                                // Free memory for G. map
}

/*
 * Manual page at hmm.def
 */
INT16 CGEN_PUBLIC CHmm::GmmMix()
{
  INT32       nIcW   = -1;                                                       // Weight trans. quali. comp.
  INT32       nG;                                                                // Gaussian index
  INT32       nG2;                                                               // Gaussian index slave
  INT32       nU;                                                                // Unit index
  INT32       nT;                                                                // Transition index
  INT32       nNG;                                                               // Number of old Gaussians
  INT32       nNU;                                                               // Number of units
  INT32       nNT;                                                               // Number of transitions
  INT32       nNGnew;                                                            // Number of new Mixtures
  INT32       nNGnewSize;                                                        // Size of table for new Mixtures
  INT16      nWsrt  = 0;                                                        // Weight semiring type
  INT32       nIcTis = -1;                                                       // Input symbol trans. quali. comp.
  INT32*      lpGinMix = NULL;                                                   // Gaussian used in Mixture
  struct ssTperG* lpTperG  = NULL;                                              // Transitions where G. occur
  CData*     idTmx    = NULL;                                                   // Mixture map table
  CVmap*     iMap     = NULL;                                                   // Gaussian mixture map
  char*      lpTDel   = NULL;                                                   // Transitions to delete
  CData*     idMean   = NULL;                                                   // Mean vectors
  CData*     idIcov   = NULL;                                                   // Inverse covariances
  FLOAT64     nTmp;                                                              // Temporary double

  if (m_iGm->m_iMmap)                                                           // Mixture map available?
    return IERROR(this,GMM_INVALD,"has mixture map","","gmm");                  //   Error
  nNG = CGmm_GetNGauss(m_iGm);                                                  // Get number of single Gaussians
  nNU = UD_XXU(_this);                                                          // Get number of units
  nWsrt = Wsr_GetType(&nIcW);                                                   // Get weight type and component
  nIcTis = CData_FindComp(td,NC_TD_TIS);                                        // Find input symbol component
  if (nIcTis<0)                                                                 // Can't do w/o input symbols!
    return IERROR(this,FST_MISS,"input symbol","component","transition table"); //   Error
  nNT = CData_GetNRecs(td);                                                     // Get number of transitions

  ICREATEEX(CVmap,iMap,"CHmm::GmmMix.~iMap",NULL);                              // Create instance for map
  ICREATEEX(CData,idTmx,"CHmm::GmmMix.~idTmx",NULL);                            // Create instance for map table
  ICREATEEX(CData,idMean,"CHmm::GmmMix.~idMean",NULL);                          // Create instance for means
  ICREATEEX(CData,idIcov,"CHmm::GmmMix.~idIcov",NULL);                          // Create instance for covs
  nNGnewSize=16;                                                                // Initialize new Mix. size
  CData_Array(idTmx,DLP_TYPE(FST_WTYPE),nNG,nNGnewSize);                        // Create map table
  lpTperG = (struct ssTperG *)dlp_calloc(nNG,sizeof(struct ssTperG));           // Alloc memory for trans. table
  lpGinMix = (INT32 *)dlp_calloc(nNG,sizeof(INT32));                              // Alloc memory for G. map
  lpTDel = (char *)dlp_calloc(nNT,sizeof(char));                                // Alloc memory for t. delete map
  dlp_memset(lpGinMix,0xff,nNG*sizeof(INT32));                                   // Clear Gaussian map

  // Create lpTperG Map                                                         // ------------------------------------
  for(nU=0;nU<nNU;nU++)                                                         // Loop over all units
  {                                                                             // >>
    for(nT=UD_FT(_this,nU);nT<UD_FT(_this,nU)+UD_XT(_this,nU);nT++)             //   Loop over trans. in unit
    {                                                                           //   >>
      nG=(INT32)CData_Dfetch(td,nT,nIcTis);                                      //     Get input symbol
      if(nG<0 || nG>=nNG) continue;                                             //     No Gaussian -> nothing to do
      lpTperG[nG].lpT=(struct sTperG *)dlp_realloc(lpTperG[nG].lpT,             //     Realloc memory for trans. table
        lpTperG[nG].nNT+1,sizeof(struct sTperG));                               //     |
      lpTperG[nG].lpT[lpTperG[nG].nNT].nT=nT;                                   //     Set trans. index
      lpTperG[nG].lpT[lpTperG[nG].nNT].nU=nU;                                   //     Set unit index
      lpTperG[nG].nNT++;                                                        //     Update numb. of trans. per G.
    }                                                                           //   <<
  }                                                                             // <<

  // Check for Gaussian to mix                                                  // ------------------------------------
  nNGnew=0;                                                                     // Init number of new mixtures
  for(nG=0;nG<nNG;nG++) if(lpGinMix[nG]<0)                                      // Loop over all G. not used
  {                                                                             // >>
    if(nNGnew>=nNGnewSize)                                                      //   Not enough space ?
    {                                                                           //   >>
      nNGnewSize+=16;                                                           //     Increment size
      CData_Reallocate(idTmx,nNGnewSize);                                       //     Get new memory
    }                                                                           //   <<
    for(nG2=nG+1;nG2<nNG;nG2++)                                                 //   Loop over all following Gaussians
    {                                                                           //   >>
      if(!GmmMix_CompareGauss(lpTperG+nG,lpTperG+nG2)) continue;                //     Gaussians don't match -> next
      IFCHECKEX(3) printf("\nfound similiar Gaussians: %5i, %5i",nG,nG2);       //     Protocol
      lpGinMix[nG]=lpGinMix[nG2]=nG;                                            //     Mark G. to group in mixture
    }                                                                           //   <<
    if(lpGinMix[nG]<0)                                                          //   No similiar G. found
    {                                                                           //   >>
      IFCHECKEX(2) printf("\n%5i: single Gaussian: %5i",nNGnew,nG);             //     Protocol
      CData_Dstore(idTmx,1.,nNGnew,nG);                                         //     Create single Gaussian
    } else {                                                                    //   << else >>
      IFCHECK{
        printf("\n%5i: mixed Gaussians: %5i",nNGnew,nG);                        //     Protocol
        for(nG2=nG+1;nG2<nNG;nG2++) if(lpGinMix[nG2]==nG) printf(", %5i",nG2);  //     |
      }
      GmmMix_CreateMixture(lpGinMix,nNG,nG,lpTperG,nIcW,nWsrt,                  //     Create mixture Gaussian
        idTmx,nNGnew,lpTDel);                                                   //     |
    }                                                                           //   <<
    for(nT=0;nT<lpTperG[nG].nNT;nT++)                                           //   Loop over all trans.
      CData_Dstore(td,nNGnew,lpTperG[nG].lpT[nT].nT,nIcTis);                    //     Store new input symbol
    nNGnew++;                                                                   //   Update numb. of new mixtures
  }                                                                             // <<

  // Resize idTmx                                                               // ------------------------------------
  if(nNGnew<nNGnewSize) CData_DeleteRecs(idTmx,nNGnew,nNGnewSize-nNGnew);       // Resize mixture map table

  // Convert idTmx                                                              // ------------------------------------
  switch (nWsrt)                                                                // Branch for weight semiring type
  {                                                                             // >>
    case FST_WSR_PROB: break;                                                   //   probability -> nothing to do
    case FST_WSR_LOG :                                                          //   logarithmic -> see tropical
    case FST_WSR_TROP:                                                          //   tropical -> convert mix. map table
      CMatrix_Op(idTmx,idTmx,T_INSTANCE,NULL,T_INSTANCE,OP_LN_EL);              //   |
      nTmp = -1; CMatrix_Op(idTmx,idTmx,T_INSTANCE,&nTmp,T_DOUBLE,OP_MULT_EL);  //   |
    break;                                                                      //   |
    case FST_WSR_NONE: DLPASSERT(FMSG(Unweighted semiring type));break;         //   Unweighted
    default          : DLPASSERT(FMSG(Unknown weight semiring type));           //   NEW WEIGHT SEMIRING TYPE?
  }                                                                             // <<
  IFCHECKEX(1) CData_Print(idTmx);                                              // Protocol

  // Setup iMap                                                                 // ------------------------------------
  switch (nWsrt)                                                                // Branch for weight semiring type
  {                                                                             // >>
    case FST_WSR_PROB: CVmap_Setup(iMap,idTmx,"add","mult",0);break;            //   Create probability map
    case FST_WSR_LOG : CVmap_Setup(iMap,idTmx,"lsadd","add",T_DOUBLE_MAX);break;//   Create logarithmic weight map
    case FST_WSR_TROP: CVmap_Setup(iMap,idTmx,"min","add",T_DOUBLE_MAX);break;  //   Create tropical weight map
    case FST_WSR_NONE: DLPASSERT(FMSG(Unweighted semiring type));break;         //   Unweighted
    default          : DLPASSERT(FMSG(Unknown weight semiring type));           //   NEW WEIGHT SEMIRING TYPE?
  }                                                                             // <<

  IFCHECK CVmap_Status(iMap);                                                   // Protocol

  // Delete Transitions                                                         // ------------------------------------
  nU=nNU-1;                                                                     // Start with last unit
  for(nT=nNT-1;nT>=0;nT--) if(lpTDel[nT])                                       // Loop backward over all trans. to del
  {                                                                             // >>
    while(nT<UD_FT(_this,nU)) nU--;                                             //   Update unit index
    Deltrans(nU,nT);                                                            //   Delete trans.
  }                                                                             // <<

  // Setup Gmm with mixture map                                                 // ------------------------------------
  IFIELD_RESET_EX(m_iGm,CVmap,"mmap");                                          // Reset Mixture map
  m_iGm->m_iMmap->Copy(iMap);                                                   // Copy Mixture map
  IF_NOK(CGmm_CheckMmap(m_iGm))                                                 /*   Check the map                   */
    IERROR(m_iGm,GMM_NOTSETUP," (mixture map corrupt)",0,0);                    /*     Error message                 */

  // Clean up                                                                   // ------------------------------------
  IDESTROY(idTmx);                                                              // Destroy mixture map table
  IDESTROY(iMap);                                                               // Destroy mixture map
  IDESTROY(idMean);                                                             // Destroy means
  IDESTROY(idIcov);                                                             // Destroy covs
  dlp_free(lpTDel);                                                             // Free memory for trans. to delete
  dlp_free(lpGinMix);                                                           // Free memory for G. map
  for(nG=0;nG<nNG;nG++) if(lpTperG[nG].lpT) dlp_free(lpTperG[nG].lpT);          // Free memory for trans. table
  dlp_free(lpTperG);                                                            // Free memory for trans. table

  return O_K;                                                                   // All done
}

static INT16 CHmm_GmmUnmixComp(CHmm *h){
  if (!h->m_iGm->m_iMmap) return IERROR(h,GMM_INVALD,"has no mixture map","","gmm");
  CData* tmx=h->m_iGm->m_iMmap->m_idTmx;
  if(!CTmx_IsCompressed(tmx)) return IERROR(h,GMM_INVALD,"mixture map is not compressed","","gmm");
  if(CData_GetNComps(h->is)!=3 || CData_IsHomogen(h->is)!=T_DOUBLE) return IERROR(h,GMM_INVALD,"only implemented with tid map","","gmm");

  BYTE *bi0=(BYTE*)CData_XAddr(tmx,0,0),*bi;
  BYTE *bo0=(BYTE*)CData_XAddr(tmx,0,1),*bo;
  BYTE *bw0=(BYTE*)CData_XAddr(tmx,0,2),*bw;
  INT64 n0=CData_GetNRecs(tmx),n;
  INT32 nr=CData_GetRecLen(tmx);
  INT64 no=0;
  for(n=n0,bo=bo0;n;n--,bo+=nr)
    if(*(INT64*)bo>=no) no=(*(INT64*)bo)+1;
  struct tmxmap {
    INT64 i;
    FLOAT64 w;
    struct tmxmap *nxt;
  } **tmxmap,*tmxmaps,*tmi;
  if(!(tmxmap=(struct tmxmap **)calloc(no,sizeof(*tmxmap)))) return IERROR(h,GMM_INVALD,"alloc failed","","gmm");
  if(!(tmxmaps=(struct tmxmap *)malloc(n0*sizeof(*tmxmaps)))) return IERROR(h,GMM_INVALD,"alloc failed","","gmm");
  for(n=n0,bi=bi0,bo=bo0,bw=bw0,tmi=tmxmaps;n;n--,bi+=nr,bo+=nr,bw+=nr,tmi++){
    INT64 o=*(INT64*)bo;
    tmi->i=*(INT64*)bi;
    tmi->w=*(FLOAT64*)bw;
    tmi->nxt=tmxmap[o];
    tmxmap[o]=tmi;
  }

  FLOAT64 *tids=(FLOAT64*)CData_XAddr(h->is,0,0);
  INT64 xtid0=CData_GetNRecs(h->is),tidi,tido;
  INT64 xtid1=0;
  for(tidi=0;tidi<xtid0;tidi++)
    for(tmi=tmxmap[(INT64)tids[tidi*3]];tmi;tmi=tmi->nxt) xtid1++;
  
  struct tidmap { INT64 s,e; } *tidmap;
  if(!(tidmap=(struct tidmap*)malloc(xtid0*sizeof(*tidmap)))) return IERROR(h,GMM_INVALD,"alloc failed","","gmm");
  CData_InsertRecs(h->is,0,xtid1-xtid0,1);
  tids=(FLOAT64*)CData_XAddr(h->is,0,0);
  for(tidi=xtid1-xtid0,tido=0;tidi<xtid1;tidi++){
    tidmap[tidi+xtid0-xtid1].s=tido;
    for(tmi=tmxmap[(INT64)tids[tidi*3]];tmi;tmi=tmi->nxt,tido++){
      tids[tido*3+0]=tmi->i;
      tids[tido*3+1]=tids[tidi*3+1]-tmi->w;
      tids[tido*3+2]=tids[tidi*3+2]-tmi->w;
    }
    tidmap[tidi+xtid0-xtid1].e=tido;
  }
  
  INT32 nIcTis,nIcTos,nIcW;
  INT64 nRL;
  BYTE *td,*tdtid;
  if((nIcTis=CData_FindComp(h->td,NC_TD_TIS))<0) return IERROR(h,FST_MISS,"input symbol","component","transition table");
  if((nIcTos=CData_FindComp(h->td,NC_TD_TOS))<0) return IERROR(h,FST_MISS,"output symbol","component","transition table");
  if(h->Wsr_GetType(&nIcW)!=FST_WSR_LOG) return IERROR(h,FST_INTERNAL,"currently only LOG WSR implemented","","");
  nRL=CData_GetRecLen(h->td);
  tdtid=h->td->XAddr(0,nIcTis);

  INT32 ti,xt0=CData_GetNRecs(h->td),xt1=0;
  for(ti=0;ti<xt0;ti++){
    INT32 tid=*(FST_STYPE*)(tdtid+nRL*ti);
    INT32 xt1l=xt1;
    if(tid<0) xt1++;
    else if(tid>=xtid0) return IERROR(h,GMM_INVALD,"tid exceeds tid0","","gmm");
    else xt1+=tidmap[tid].e-tidmap[tid].s;
    if(xt1<xt1l) return IERROR(h,GMM_INVALD,"xt1 exceeds type size of INT32","","gmm");
  }

  CData_InsertRecs(h->td,0,xt1-xt0,1);
  tdtid=h->td->XAddr(0,nIcTis);
  td=h->td->XAddr(0,0);

  INT32 ui,xu=UD_XXU(h),to=0;
  for(ui=0;ui<xu;ui++){
    INT32 uft=UD_FT(h,ui), uxt=UD_XT(h,ui);
    UD_FT(h,ui)=to;
    for(ti=uft+xt1-xt0;ti<uft+uxt+xt1-xt0;ti++){
      INT32 tid=*(FST_STYPE*)(tdtid+nRL*ti);
      if(tid<0) memcpy(td+nRL*to++,td+nRL*ti,nRL);
      else if(tid>=xtid0) return IERROR(h,GMM_INVALD,"tid exceeds tid0","","gmm");
      else for(tido=tidmap[tid].s;tido<tidmap[tid].e;tido++,to++){
        memcpy(td+nRL*to,td+nRL*ti,nRL);
        *(FST_STYPE*)(tdtid+nRL*to)=tido;
      }
    }
    UD_XT(h,ui)=to-UD_FT(h,ui);
  }

  free(tidmap);
  free(tmxmap);
  free(tmxmaps);

  IDESTROY(h->m_iGm->m_iMmap);
  h->m_iGm->m_iMmap=NULL;

  return O_K;
}

/*
 * Manual page at hmm.def
 */
INT16 CGEN_PUBLIC CHmm::GmmUnmix()
{
  INT32       nIcW   = -1;                                                       // Weight trans. quali. comp.
  INT32       nU;                                                                // Unit index
  INT32       nT;                                                                // Transition index
  INT32       nG;
  INT32       nM;
  BOOL        bT;
  FLOAT64     nW;
  FLOAT64     nMW;
  INT32       nNG;                                                               // Number of old Gaussians
  INT32       nNM;
  INT32       nNU;                                                               // Number of units
  INT32       nXT;                                                               // Number of units
  INT16      nWsrt  = 0;                                                        // Weight semiring type
  INT32       nIcTis = -1;                                                       // Input symbol trans. quali. comp.
  INT32       nIcTos = -1;                                                       // Input symbol trans. quali. comp.
  CData*     idTmx    = NULL;                                                   // Mixture map table

  if (!m_iGm->m_iMmap)                                                           // Mixture map available?
    return IERROR(this,GMM_INVALD,"has no mixture map","","gmm");                  //   Error
  idTmx = m_iGm->m_iMmap->m_idTmx;
  if(CTmx_IsCompressed(idTmx)) return CHmm_GmmUnmixComp(this);

  nNG = idTmx->GetNComps();
  nNM = idTmx->GetNRecs();
  nNU = UD_XXU(_this);                                                          // Get number of units
  nWsrt = Wsr_GetType(&nIcW);                                                   // Get weight type and component
  if(nWsrt!=FST_WSR_LOG) return IERROR(this,FST_INTERNAL,"currently only LOG WSR implemented","","");
  nIcTis = CData_FindComp(td,NC_TD_TIS);                                        // Find input symbol component
  if (nIcTis<0)                                                                 // Can't do w/o input symbols!
    return IERROR(this,FST_MISS,"input symbol","component","transition table"); //   Error
  nIcTos = CData_FindComp(td,NC_TD_TOS);
  if (nIcTos<0) return IERROR(this,FST_MISS,"output symbol","component","transition table");

  // Expand Transitions
  for(nU=0;nU<nNU;nU++)                                                         // Loop over all units
  {                                                                             // >>
    nXT=UD_FT(_this,nU)+UD_XT(_this,nU);
    for(nT=UD_FT(_this,nU);nT<nXT;nT++){                                                                           //   >>
      nM=*(FST_STYPE*)td->XAddr(nT,nIcTis);                                      //     Get input symbol
      nW=*(FST_WTYPE*)td->XAddr(nT,nIcW);
      bT=TRUE;
      for(nG=0;nG<nNG;nG++){
        if((nMW=CData_Dfetch(idTmx,nM,nG))==m_iGm->m_iMmap->m_nZero) continue;
        if(bT){
          *(FST_STYPE*)td->XAddr(nT,nIcTis)=nG;
          bT=FALSE;
          *(FST_WTYPE*)td->XAddr(nT,nIcW)=nW+nMW;
        }else{
          _this->AddtransEx(
            nU,
            *(FST_ITYPE*)td->XAddr(nT,IC_TD_INI),
            *(FST_ITYPE*)td->XAddr(nT,IC_TD_TER),
            nG,
            *(FST_STYPE*)td->XAddr(nT,nIcTos),
            nW+nMW
          );
        }
      }
      if(bT) *(FST_WTYPE*)td->XAddr(nT,nIcW)=m_iGm->m_iMmap->m_nZero;
    }                                                                           //   <<
  }                                                                             // <<

  IDESTROY(m_iGm->m_iMmap);
  m_iGm->m_iMmap=NULL;

  return O_K;                                                                   // All done
}

/*
 * Manual page at hmm.def
 */
INT16 CGEN_PUBLIC CHmm::ResetStats()
{
  INT32 nT     = 0;                                                              // Current transition
  INT32 nIcTrc = -1;                                                             // Transition reference counter
  INT32 K      = 0;                                                              // Number of single Gaussians
  INT32 nXXU   = 0;                                                              // Number of HMM's
  INT32 nXXS   = 0;                                                              // Total number of HMM states
  INT32 nXXL   = 0;                                                              // Total number of HMM layers

  if (!m_iGm) return NOT_EXEC;                                                  // Need valid Gaussians

  // Clear transition reference counters                                        // ------------------------------------
  if ((nIcTrc = CData_FindComp(td,NC_TD_RC))>=0)                                // Find reference counter comp
    for (nT=0; nT<UD_XXT(this); nT++)                                           //   Loop over all transitions
      CData_Dstore(td,0,nT,nIcTrc);                                             //     Clear reference counter

  // Clear internal statistics                                                  // ------------------------------------
  if      (m_iGm  ) K = CGmm_GetNGauss(m_iGm);                                  // Get number of single Gaussians
  else if (m_iPfsm) K = CData_GetNBlocks(m_iPfsm->m_idDat);                     // ...
  else              K = 0;                                                      // ...
  nXXU = UD_XXU(this);                                                          // Get number of HMM's
  nXXS = UD_XXS(this);                                                          // Get total number of HMM states
  nXXL = m_iPls ? CData_GetNBlocks(m_iPls->m_idDat) : 0;                        // Get total number of HMM layers
  if (m_iPfsm) m_iPfsm->Setup(4,CData_GetNComps(m_iPfsm->m_idDat),K   ,NULL,-1);// Resetup most sign. feature stats.
  if (m_iPfsl) m_iPfsl->Setup(4,CData_GetNComps(m_iPfsl->m_idDat),K   ,NULL,-1);// Resetup least sign. feature stats.
  if (m_iPls ) m_iPls ->Setup(4,CData_GetNComps(m_iPls ->m_idDat),nXXL,NULL,-1);// Resetup per-HMM-layer statistics
  if (m_iPss ) m_iPss ->Setup(4,CData_GetNComps(m_iPss ->m_idDat),nXXS,NULL,-1);// Resetup per-HMM-state statistics
  if (m_iPms ) m_iPms ->Setup(4,CData_GetNComps(m_iPms ->m_idDat),nXXU,NULL,-1);// Resetup per-HMM statistics
  if (m_idGrc) m_idGrc->Reset(TRUE);                                            // Resetup Gaussian RC's

  return O_K;
}

/**
 * Internal use only!
 */
INT16 CGEN_PRIVATE CHmm::MergeStatsInt(CHmm* iSrc, const char* lpsFid)
{
	DLPASSERT(iSrc);
	CStatistics* isSrc = (CStatistics*)iSrc->FindInstance(lpsFid);
	if (!isSrc) return O_K;
	CStatistics* isDst = (CStatistics*)FindInstance(lpsFid);
	if (!isDst)
	{
		IFIELD_RESET(CStatistics,lpsFid);
		isDst = (CStatistics*)FindInstance(lpsFid);
	}
	DLPASSERT(isSrc && isDst);
	CStatistics_Merge(isDst,isSrc);
	return O_K;
}

/*
 * Manual page at hmm.def
 */
INT16 CGEN_PUBLIC CHmm::MergeStats(CHmm* iSrc)
{
	if (!iSrc) return O_K;

	MergeStatsInt(iSrc,"pfsm");
	MergeStatsInt(iSrc,"pfsl");
	MergeStatsInt(iSrc,"pls" );
	MergeStatsInt(iSrc,"pss" );
	MergeStatsInt(iSrc,"pms" );

	// Merge Gaussian reference counters
	if (iSrc->m_idGrc && m_idGrc)
	{
		if
		(
	    (CData_GetNRecs (iSrc->m_idGrc)==CData_GetNRecs (m_idGrc)) &&
	    (CData_GetNComps(iSrc->m_idGrc)==CData_GetNComps(m_idGrc))
		)
		{
			CData_Scalop(m_idGrc,iSrc->m_idGrc,CMPLX(0),"add");
		}
		else
			IERROR(this,HMM_INCOMPAT,0,0,0);
	}

	// Merge transition reference counters
	INT32 nIcSrc = CData_FindComp(iSrc->td,"~RC");
	INT32 nIcDst = CData_FindComp(td      ,"~RC");
	if (CData_GetNRecs(iSrc->td)==CData_GetNRecs(td) && nIcSrc>=0 && nIcDst>=0)
	{
		for (INT32 nR=0; nR<CData_GetNRecs(td); nR++)
			CData_Dstore
			(
			  td,
	      CData_Dfetch(td,nR,nIcSrc)+CData_Dfetch(iSrc->td,nR,nIcSrc),
	      nR,
	      nIcDst
	    );
	}
	else if (nIcSrc>=0 || nIcDst>=0)
		IERROR(this,HMM_INCOMPAT,0,0,0);

  return O_K;
}

/**
 * Updates an internal statistics with a set of labelled feature vectors.
 *
 * @param iStat
 *          The statistics instance
 * @param idVec
 *          The feature vector set
 * @param idLab
 *          Set of numeric statistics class labels corresponding with
 *          <code>idVec</code>
 * @return <code>O_K</code> if successfull, a (negative) error code otherwise
 */
INT16 CGEN_PRIVATE CHmm::UpdateStatInt
(
  CStatistics* iStat,
  CData*       idVec,
  CData*       idLab
)
{
  CData* idAux  = NULL;                                                         // Auxilary data instance
  INT32   nXV    = 0;                                                            // Number of feature vectors
  INT32   nXL    = 0;                                                            // Number of labels
  INT32   nIcLab = -1;                                                           // Label component index (stat. update)

  if (!iStat              ) return NOT_EXEC;                                    // Need statistics instance
  if (CData_IsEmpty(idVec)) return NOT_EXEC;                                    // Need feature vector set
  if (CData_IsEmpty(idLab)) return NOT_EXEC;                                    // Need stats. class (Gaussian) labels

  nXV = CData_GetNRecs(idVec);                                                  // Count number of feature vectors
  nXL = CData_GetNRecs(idLab);                                                  // Count number of labels
  if (nXV<nXL)                                                                  // Fewer vectors than labels
  {                                                                             // >>
    IERROR(this,HMM_BADNOLAB,nXL,nXV,"feature sequence");                       //   Warning
    ICREATEEX(CData,idAux,"~CHmm::UpdateStatInt.idAux",NULL);                   //   Create a temporary data instance
    CData_SelectRecs(idAux,idVec,0,nXL);                                        //   Truncate vector sequence
    UpdateStatInt(iStat,idAux,idLab);                                           //   Recursive call to UpdateStatInt
    IDESTROY(idAux);                                                            //   Destroy temporaray data instance
  }                                                                             // <<
  else if (nXV>nXL)                                                             // More vectors than labels
  {                                                                             // >>
    IERROR(this,HMM_BADNOLAB,nXL,nXV,"label sequence");                         //   Warning
    ICREATEEX(CData,idAux,"~CHmm::UpdateStatInt.idAux",NULL);                   //   Create a temporary data instance
    CData_SelectRecs(idAux,idLab,0,nXV);                                        //   Truncate label sequence
    UpdateStatInt(iStat,idVec,idAux);                                           //   Recursive call to UpdateStatInt
    IDESTROY(idAux);                                                            //   Destroy temporaray data instance
  }                                                                             // <<
  else                                                                          // Equal number of vectors and labels
  {                                                                             // >>
    CData_Join(idVec,idLab);                                                    //   Join Gaussian labs. to ftr. vecs.
    nIcLab = CData_GetNComps(idVec)-1;                                          //   Determine label component index
    CStatistics_Update(iStat,idVec,nIcLab,NULL);                                //   Update feature statistics
    CData_DeleteComps(idVec,nIcLab,1);                                          //   Remove label component
  }                                                                             // <<

  return O_K;                                                                   // Yo!
}

/*
 * Manual page at hmm.def
 */
INT16 CGEN_PUBLIC CHmm::Update
(
  CData* idSrc,
  INT32  nIcTis,
  INT32  nIcTer,
  CData* idMsf,
  CData* idLsf,
  INT32  nUnit
)
{
  CData* idLab = NULL;                                                          // Gaussian label seq. (stat. update)
  INT32   nF    = 0;                                                             // Current feature vector
  INT32   nCML  = 0;                                                             // Current value of field max_len

  // Validate                                                                   // ------------------------------------
  if (CData_IsEmpty(idSrc)) return IERROR(this,ERR_NULLINST,"idSrc",0,0);       // Need an update sequence
  if (CData_IsEmpty(idMsf)) return IERROR(this,ERR_NULLINST,"idMsf",0,0);       // Need a most significant feature seq.
  if (nUnit<0 || nUnit>UD_XXU(this))                                            // Check unit index
    return IERROR(this,FST_BADID,"unit",nUnit,0);                               // |
  if (nIcTis<0 || nIcTis>=CData_GetNComps(idSrc))                               // Check Gaussian index component
    return IERROR(this,FST_BADID,"source component",nIcTis,0);                  // |

  // Add path to HMM transducer                                                 // ------------------------------------
  nCML = m_nMaxLen;                                                             // Save current value of field max_len
  ISETFIELD_RVALUE(this,"max_len",CData_GetNRecs(idSrc)+1);                          // Set max_len to len. of update seq.+1
  CFst_Addseq(this,idSrc,nIcTis,/*nIcTer*/-1,-1,nUnit);                         // Call FST update
  ISETFIELD_LVALUE(this,"max_len",nCML);                                             // Reset field max_len to prev. value

  // Remove epsilon symbols from Gaussian sequence                              // ------------------------------------
  // NO RETURNS BEYOND THIS POINT!                                              //
  if (_this->m_bFast)                                                           // In quick and dirty mode
  {                                                                             // >>
    idLab = idSrc;                                                              //   Operate on source instance
    CData_DeleteComps(idLab,0,nIcTis);                                          //   Delete heading components
    CData_DeleteComps(idLab,1,CData_GetNComps(idLab)-1);                        //   Delete trailing components
  }                                                                             // <<
  else                                                                          // In normal mode
  {                                                                             // >>
    ICREATEEX(CData,idLab,"~CHmm::Update.idLab",NULL);                          //   Create temporary label instance
    CData_Select(idLab,idSrc,nIcTis,1);                                         //   Copy label component
  }                                                                             // <<
  for (nF=0; nF<CData_GetNRecs(idLab); )                                        // Loop over labels
    if (CData_Dfetch(idLab,nF,0)<0.)                                            //   Epsilon label
      CData_DeleteRecs(idLab,nF,1);                                             //     Remove it
    else                                                                        //   Non-epsilon label
      nF++;                                                                     //     Continue with next label

  // Update feature statistics                                                  // ------------------------------------
  UpdateStatInt(m_iPfsm,idMsf,idLab);                                           // Update most sign. ftrs. stats.
  UpdateStatInt(m_iPfsl,idLsf,idLab);                                           // Update least sign. ftrs. stats.

  // Clean up                                                                   // ------------------------------------
  if (!_this->m_bFast) IDESTROY(idLab);                                         // Destroy temporary label instance
  return O_K;                                                                   // Ok
}

/*
 * Manual page at hmm.def
 */
INT16 CGEN_PUBLIC CHmm::Bwupdate
(
  CData* idAlpha,
  CData* idMsf,
  CData* idLsf,
  INT32   nUnit
)
{
  INT32 nK,nTG;
  INT32 nNK,nNTG,nNM,nNG;
  INT32 nFT;
  INT16 nWsr;
  FST_WTYPE nNeuAdd;
  INT32 nIcTis,nIcRc;
  FST_WTYPE *lpAlpha;
  FST_WTYPE *lpPG;

  /* Check */
  if (nUnit<0 || nUnit>UD_XXU(this))                                            // Check unit index
    return IERROR(this,FST_BADID,"unit",nUnit,0);                               // |
  if(idAlpha->IsHomogen()!=DLP_TYPE(FST_WTYPE))
    return IERROR(_this,FST_BADCTYPE,"idAlpha",0,0);

  /* Init */
  nNK=idAlpha->GetNRecs()-3;
  nNTG=idAlpha->GetNComps();
  nNM=m_iGm->GetNMix();
  nNG=m_iGm->GetNGauss();
  nFT=UD_FT(this,nUnit);
  nWsr=this->Wsr_GetType(NULL);
  nNeuAdd=this->Wsr_NeAdd(nWsr);
  nIcTis=td->FindComp(NC_TD_TIS);
  nIcRc=td->FindComp(NC_TD_RC);
  lpAlpha=(FST_WTYPE*)idAlpha->XAddr(0,0);

  /* Convert transition reference counter to FST_WTYPE */
  if(CData_GetCompType(td,nIcRc)!=DLP_TYPE(FST_WTYPE)){
    CData_InsertComp(td,NC_TD_RC,DLP_TYPE(FST_WTYPE),nIcRc+1);
    CData_Xstore(td,td,nIcRc,1,nIcRc+1);
    CData_DeleteComps(td,nIcRc,1);
  }

  /* Update transition + Gaussian reference counter */
  for(nTG=0;nTG<nNTG;nTG++){
    INT32 nT=(INT32)lpAlpha[0*nNTG+nTG];
    INT32 nG=(INT32)lpAlpha[1*nNTG+nTG];
    FST_WTYPE nRc = *(FST_WTYPE*)this->td->XAddr(nT+nFT,nIcRc);
    for(nK=(nG<0?0:1);nK<=nNK;nK++){
      FST_WTYPE nA=lpAlpha[(nK+2)*nNTG+nTG];
      if(nWsr==FST_WSR_LOG || nWsr==FST_WSR_TROP) nA=exp(-nA);
      nRc += nA;
    }
    *(FST_WTYPE*)this->td->XAddr(nT+nFT,nIcRc)=nRc;
  }

  /* Update gaussian reference counter */
  if(m_iGm->m_iMmap){
    FST_WTYPE *lpGrc;
    if(CData_IsEmpty(m_idGrc) || m_idGrc->GetNComps()!=nNG || m_idGrc->GetNRecs()!=nNM){
      IFIELD_RESET(CData,"grc");
      m_idGrc->Array(DLP_TYPE(FST_WTYPE),nNG,nNM);
      m_idGrc->Fill(CMPLX(m_iGm->m_iMmap->m_nZero),CMPLX(0));
    }
    lpGrc=(FST_WTYPE *)m_idGrc->XAddr(0,0);
    for(nTG=0;nTG<nNTG;nTG++){
      INT32 nT=(INT32)lpAlpha[0*nNTG+nTG];
      INT32 nG=(INT32)lpAlpha[1*nNTG+nTG];
      INT32 nM=*(FST_STYPE*)td->XAddr(nT+nFT,nIcTis);
      if(nM<0 || nG<0) continue;
      FST_WTYPE nGRc = lpGrc[nM*nNG+nG];
      for(nK=1;nK<=nNK;nK++){
        FST_WTYPE nA=lpAlpha[(nK+2)*nNTG+nTG];
        if(nWsr==FST_WSR_LOG || nWsr==FST_WSR_TROP) nGRc = dlp_scalop(nGRc,nA,OP_LSADD); else nGRc+=nA;
      }
      lpGrc[nM*nNG+nG]=nGRc;
    }
  }

  /* Calc gaussian probs */
  INT32 nNXm=m_iPfsm->GetDim();
  INT32 nNXl=m_iPfsl->GetDim();
  FLOAT64 *lpXm=(FLOAT64*)dlp_malloc(nNXm*sizeof(FLOAT64));
  FLOAT64 *lpXl=(FLOAT64*)dlp_malloc(nNXl*sizeof(FLOAT64));
  lpPG=(FST_WTYPE*)dlp_malloc(nNG*sizeof(FST_WTYPE));
  for(nK=0;nK<nNK;nK++){
    FST_WTYPE nSum=nNeuAdd;
    INT32 nG;
    for(nG=0;nG<nNG;nG++) lpPG[nG]=nNeuAdd;
    for(nTG=0;nTG<nNTG;nTG++){
      nG=(INT32)lpAlpha[1*nNTG+nTG];
      if(nG>=0){
        lpPG[nG] = this->Wsr_Op(lpPG[nG],lpAlpha[(nK+3)*nNTG+nTG],OP_ADD);
        nSum     = this->Wsr_Op(nSum,    lpAlpha[(nK+3)*nNTG+nTG],OP_ADD);
      }
    }
    idMsf->DrecFetch(lpXm,nK,nNXm,-1);
    idLsf->DrecFetch(lpXl,nK,nNXl,-1);
    for(nG=0;nG<nNG;nG++){
      FST_WTYPE nW=lpPG[nG];
      if(nW!=nNeuAdd){
        if(nWsr==FST_WSR_LOG || nWsr==FST_WSR_TROP) nW=exp(-nW);
        CStatistics_UpdateVector(m_iPfsm,lpXm,nG,nW);
        if (m_iPfsl) CStatistics_UpdateVector(m_iPfsl,lpXl,nG,nW);
      }
    }
  }
  dlp_free(lpPG);
  dlp_free(lpXm);
  dlp_free(lpXl);

  /* Done */
  return O_K;                                                                   // Happy happy joy joy!
}

/*
 * Manual page at hmm.def
 */
INT16 CGEN_PUBLIC CHmm::Split(FLOAT64 nMinRc, INT32 nMaxCnt, CData* idMap)
{
  CData*    idMapGm  = NULL;                                                    // Split map for gm
  INT32      nM     = 0;                                                         // Current map entry
  INT32      nMcount= 0;                                                         // Stored map entries
  INT32      nCtr   = 0;                                                         // Split counter
  INT32      nU     = 0;                                                         // Current unit
  INT32      nT     = 0;                                                         // Current (old) transition
  INT32      nTn    = 0;                                                         // Current new transition
  INT32      nFT    = 0;                                                         // First transition of current unit
  INT32      nXT    = 0;                                                         // Number of transitions of curr. unit
  INT32      nIcTis = -1;                                                        // Input symbol trans. quali. comp.
  INT32      nIcW   = -1;                                                        // Weight trans. quali. comp.
  INT32      nIcTrc = -1;                                                        // Reference counter trans. quali.comp.
  INT16     nWsrt  = 0;                                                         // Weight semiring type
  FST_ITYPE nIni   = -1;                                                        // Initial state of current transition
  FST_ITYPE nTer   = -1;                                                        // Terminal state of current transition
  FST_STYPE nTis   = -1;                                                        // Input symbol of current transition
  FST_WTYPE nW     = 0.;                                                        // Weight of current transition
  INT32      nMaxCntRC  = m_bByrc == FALSE ? -1 : nMaxCnt;                       // max Gaussians to split, selected by RC
  INT32      nMaxCntVar = m_bByrc == FALSE ? nMaxCnt : -1;                       // max Gaussians to split, selected by Variance
  if(nMaxCntVar == 0) nMaxCntVar = -1;                                          // make it compatible with nMaxCntRC

  // Initialize                                                                 // ------------------------------------
  nIcTis = CData_FindComp(td,NC_TD_TIS);                                        // Find input symbol component
  if (nIcTis<0)                                                                 // Can't do w/o input symbols!
    return IERROR(this,FST_MISS,"input symbol","component","transition table"); // ...
  nIcTrc = CData_FindComp(td,NC_TD_RC);                                         // Find trans. reference counter comp.
  nWsrt = Wsr_GetType(&nIcW);                                                   // Get weight type and component
  if (nMaxCntRC<=0) nMaxCntRC = CGmm_GetNGauss(m_iGm);                          // Split maximal numbe of Gaussians

  // Make splitting map                                                         // -- NO RETURNS BEYOND THIS POINT! ---
  ICREATEEX(CData,idMapGm,"~CHmm::Split.idMap",NULL);                           // Create split map
  if (nMinRc>0 || nMaxCntRC<CGmm_GetNGauss(m_iGm) || idMap!=NULL)               // Not blindly splitting all Gaussians
  {
    CData_Array(idMapGm,T_LONG,2,CGmm_GetNGauss(m_iGm));                        //   Allocate split map
    for (nM=0; nM<CData_GetNRecs(idMapGm); nM++) CData_Dstore(idMapGm,nM,nM,1); //   Initialize Gaussian index
    for (nT=0; nT<UD_XXT(_this); nT++)                                          //   Get total Gaussian reference ctrs.
    {                                                                           //   >>
      nTis = (INT32)CData_Dfetch(td,nT,nIcTis);                                  //     Get GMM index
      CData_Dstore(idMapGm,CData_Dfetch(idMapGm,nTis,0)+CData_Dfetch(td,nT,nIcTrc), //     Sum up GMM reference counters
        nTis,0);                                                                //     |
    }                                                                           //   <<
    CData_Sortdown(idMapGm,idMapGm,0);                                          //   Sort descending by RC
                                                                                // >>
    for (nM=nMcount=0; nM<CData_GetNRecs(idMapGm); nM++)                        //   Loop over map entries (Gaussians)
      if(nMcount<nMaxCntRC && (INT32)CData_Dfetch(idMapGm,nM,0)>=nMinRc &&        //  nMaxCnt most frequent AND RC >= nMinRc AND
        (idMap==NULL || CData_GetNRecs(idMap)==0 ||                             //  | idMap(nM)!=0
        CData_Dfetch(idMap,(INT32)CData_Dfetch(idMapGm,nM,1),0)))
      {
        CData_Dstore(idMapGm,1.,nM,0);                                          //  Store 1
        nMcount++;                                                              //  increment counter
      }else CData_Dstore(idMapGm,0.,nM,0);                                      //  Store 0
    CData_Sortup(idMapGm,idMapGm,1);                                            //   Sort descending by index
    CData_DeleteComps(idMapGm,1,1);                                             //   Delete index component

    // Analyze generated map                                                    //   - - - - - - - - - - - - - - - - -
    for (nM=0,nCtr=0; nM<CData_GetNRecs(idMapGm); nM++)                         //   Count Gaussians to be split
      nCtr += (INT32)CData_Dfetch(idMapGm,nM,0);                                 //   ...
    if (nCtr==0)                                                                //   No Gaussians to be split
    {                                                                           //   >>
      IDESTROY(idMapGm);                                                        //     Destroy map
      IERROR(this,HMM_NOSPLIT,0,0,0);                                           //     Print warning
      return O_K;                                                               //     But return O_K
    }                                                                           //   <<
  }                                                                             // <<

  // Split Gaussians                                                            // ------------------------------------
  CGmm_Split(m_iGm,nMaxCntVar,idMapGm);                                         // Split Gaussians
  ResetStats();                                                                 // Reset internal statistics

  if(!m_iGm->m_iMmap)                                                           // Check for mixture map
  {                                                                             // >>
    // Ajust HMM topology                                                       //   ----------------------------------
    for (nU=0; nU<UD_XXU(this); nU++)                                           //   Loop over units
    {                                                                           //   >>
      nFT = UD_FT(this,nU);                                                     //     Get first transition of unit
      nXT = UD_XT(this,nU);                                                     //     Get number of transitions of unit
      for (nT=nFT; nT<nFT+nXT; nT++)                                            //     Loop over OLD transitions
      {                                                                         //     >>
        nIni = TD_INI(this,nT);                                                 //       Get initial state
        nTer = TD_TER(this,nT);                                                 //       Get terminal state
        nTis = (FST_ITYPE)CData_Dfetch(td,nT,nIcTis);                           //       Get input symbol (=GMM index)
        for (nM=0,nCtr=0; nM<CData_GetNRecs(idMapGm); nM++)                     //       Loop over map entries
        {                                                                       //       >>
          if ((FST_ITYPE)CData_Dfetch(idMapGm,nM,0)!=nTis) continue;            //         Does not map to curr. Gaussian
          nTn = (nCtr==0 ? nT : AddtransCopy(nU,(FST_ITYPE)nIni,(FST_ITYPE)nTer,this,(FST_ITYPE)nT));            //         Recycle old or add new trans.
          CData_Dstore(td,(FLOAT64)nM,nTn,nIcTis);                               //         Store new GMM index
          nCtr++;                                                               //         Increment
        }                                                                       //       <<
        if (nCtr<=1) continue;                                                  //       Eps. or unsplit trans. --> done
        nW = CData_Dfetch(td,nT,nIcW);                                          //       Get old weight
        switch (nWsrt)                                                          //       Branch for weight semiring type
        {                                                                       //       >>
          case FST_WSR_NONE: nW = 0.; break;                                    //         Unweighted
          case FST_WSR_PROB: nW /= nCtr; break;                                 //         Adjust probability
          case FST_WSR_LOG : // Fall through                                    //         Adjust logarithmic weight
          case FST_WSR_TROP: nW += (FST_WTYPE)log((FLOAT64)nCtr); break;         //         Adjust tropical weight
          default          : DLPASSERT(FMSG(Unknown weight semiring type));     //         NEW WEIGHT SEMIRING TYPE?
        }                                                                       //       <<
        CData_Dstore(td,nW,nT,nIcW);                                            //       Adjust weight of recycled trans.
        for (nCtr--; nCtr>0; nCtr--)                                            //       Adjust weight of added trans.
          CData_Dstore(td,nW,nFT+UD_XT(this,nU)-nCtr,nIcW);                     //       ...
      }                                                                         //     <<
    }                                                                           //   <<
  }                                                                             // <<

  // Clean up                                                                   // ------------------------------------
  IDESTROY(idMapGm);                                                            // Destroy split map
  return O_K;                                                                   // Happy happy joy joy!
}

/**
 * Manual page at hmm.def
 */
INT16 CGEN_PUBLIC CHmm::CopyFst(CFst* itSrc)
{
  hscan_t  hs;
  hnode_t* hn;
  SWord*   lpWordSrc = NULL;
  SWord*   lpWordDst = NULL;

  if (!itSrc) return NOT_EXEC;
  hash_scan_begin(&hs,itSrc->m_lpDictionary);
  while ((hn = hash_scan_next(&hs))!=NULL)
  {
    DLPASSERT((lpWordSrc = (SWord*)hnode_get(hn))!=NULL);
    lpWordDst = CDlpObject_FindWord(this,lpWordSrc->lpName,WL_TYPE_FIELD);
    if (lpWordDst && lpWordDst->ex.fld.nType==lpWordSrc->ex.fld.nType)
      CDlpObject_CopyField(this,lpWordDst,itSrc);
  }
  m_nCheck = itSrc->m_nCheck;
  return O_K;
}

/*
 * Manual page at hmm.def
 */
INT32 CGEN_PUBLIC CHmm::GetDim()
{
  if (m_iGm  ) return CGmm_GetDim(m_iGm);
  if (m_iPfsm) return CStatistics_GetDim(m_iPfsm);
  return 0;
}

/*
 * Manual page at hmm.def
 */
INT16 CGEN_PUBLIC CHmm::GenMap
(
  CData* idLab,
  INT32   nIcLab,
  INT32   nIcOfs,
  INT32   nIcLen,
  CData* idMap
)
{
  CData* idLsq  = NULL;                                                         // Decompressed numeric label sequence
  INT32   nL     = 0;                                                            // Current label
  INT32   nXL    = 0;                                                            // Number of labels
  INT32   nNM    = 0;                                                            // Number of mixtures
  INT32   nNG    = 0;                                                            // Number of Gaussians / mixtures
  INT32   nU     = 0;                                                            // Current unit
  INT32   nT     = 0;                                                            // Current transition
  INT32   nM     = 0;                                                            // Current mixture
  INT32   nIcTis = -1;                                                           // Transducer input symbol trans. comp.
  char  *lpMap;                                                                 // Current map vector

  // Validation                                                                 // ------------------------------------
  if (!idMap) return NOT_EXEC;                                                  // No destination, no service
  if (CData_IsEmpty(idLab)) return NOT_EXEC;                                    // No labels, no service
  if (nIcLab<0 || nIcLab>CData_GetNComps(idLab))                                // Check label component index
    return IERROR(this,FST_BADID,"component",nIcLab,0);                         //   Must be valid!
  if (nIcOfs>CData_GetNComps(idLab) || (nIcOfs<0 && nIcLen>=0))                 // Check RLE offset component index
    return IERROR(this,FST_BADID,"component",nIcOfs,0);                         //   Must be negative or valid!
  if (nIcLen>CData_GetNComps(idLab) || (nIcLen<0 && nIcOfs>=0))                 // Check RLE length component index
    return IERROR(this,FST_BADID,"component",nIcLen,0);                         //   Must be negative or valid!
  nIcTis = CData_FindComp(td,NC_TD_TIS);                                        // Get input symbol trans. comp.
  if (nIcTis<0)                                                                 // Need it!
    return IERROR(this,FST_MISS,"input symbol","component","transition table"); //   Otherwise error

  CREATEVIRTUAL(CData,idLab,idMap);

  // Initialize                                                                 // ------------------------------------
  ICREATEEX(CData,idLsq,"~CHmm::GenMap.idLsq",NULL);
  if (nIcLen>=0 && nIcLab>=0)                                                   // Label sequence RLE compressed
    CData_Expand(idLsq,idLab,nIcLab,nIcOfs,nIcLen);                             //   Decompress
  else                                                                          // Otherwise
    CData_SelectComps(idLsq,idLab,nIcLab,1);                                    //   Copy uncompressed labels
  if (dlp_is_symbolic_type_code(CData_GetCompType(idLsq,0)))                    // Labels symbolic
    CData_GenIndex(idLsq,idLsq,ud,0,0);                                         //   Convert to numeric
  nXL = CData_GetNRecs(idLsq);                                                  // Get number of labels
  nNM = CGmm_GetNMix(m_iGm);                                                    // Get number of mixtures
  nNG = m_bNomix && m_iGm->m_iMmap ? CGmm_GetNGauss(m_iGm) : nNM;               // Get number of Gaussians / mixtures
  CData_Array(idMap,T_BYTE,nNG,nXL);                                            // Prepare destination instance
  lpMap = (char *)dlp_calloc(nNM,sizeof(char));                                 // Mixture use buffer

  // Create map                                                                 // ------------------------------------
  for (nL=0; nL<nXL; nL++)                                                      // Loop over labels
  {                                                                             // >>
    dlp_memset(lpMap,0,nNM*sizeof(char));                                       //   Reset mixture use buffer
    nU = (INT32)CData_Dfetch(idLsq,nL,0);                                        //   Get label(=HMM) index
    if (nU<0) continue;                                                         //   Must be valid!
    for (nT=UD_FT(this,nU); nT<UD_FT(this,nU)+UD_XT(this,nU); nT++){            //   Loop over transitions >>
      nM=(INT32)CData_Dfetch(td,nT,nIcTis);                                      //     Get mixture index
      if(nM>=0 && nM<nNM) lpMap[nM]=1;                                          //     Store map point
    }                                                                            //   <<
    for(nM=0;nM<nNM;nM++) if(m_bNomix && m_iGm->m_iMmap)                        //   Revert mixture map
    {                                                                           //   >>
      INT32 nG;                                                                  //     Current Gaussian
      if(!CData_IsEmpty(m_iGm->m_iMmap->m_idWeakTmx)){                          //     Use weak tmx matrix
        for(nG=0;nG<m_iGm->m_iMmap->m_idWeakTmx->GetNRecs();nG++){              //       Loop over records >>
          INT32 nGr=(INT32)m_iGm->m_iMmap->m_idWeakTmx->Dfetch(nG,nM*2);          //         Get Gaussian index
          if(nGr>=0) CData_Dstore(idMap,1,nL,nGr);                              //         Update new map
        }                                                                       //       <<
      }else if(m_iGm->m_iMmap->m_idTmx){                                        //     Use tmx matrix
        for(nG=0;nG<m_iGm->m_iMmap->m_idTmx->GetNRecs();nG++)                   //       Loop over Gaussians
          if(m_iGm->m_iMmap->m_idTmx->Dfetch(nM,nG)!=m_iGm->m_iMmap->m_nZero)   //         If Gaussian is used
            CData_Dstore(idMap,1,nL,nG);                                        //           Update new map
      }else CData_Dstore(idMap,lpMap[nM],nL,nM);                                 //   Else copy map vector
    }                                                                           //   <<
    else CData_Dstore(idMap,lpMap[nM],nL,nM);                                   //   Else copy map vector
  }                                                                             // <<

  // Clean up                                                                   // ------------------------------------
  dlp_free(lpMap);
  IDESTROY(idLsq);
  DESTROYVIRTUAL(idLab,idMap);
  return O_K;
}

/*
 * Manual page at hmm.def
 */
INT16 CGEN_PUBLIC CHmm::Cleanup(CHmm *itSrc)
{
  INT32 nT;                                                                      /* current transition index          */
  INT32 *lpNewGmMap;                                                             /* map from old gm to new gm index   */
  INT32 nNewGmCount;                                                             /* count of new gm's                 */
  INT32 nOldGmCount;                                                             /* count of old gm's                 */
  INT32 nGm;                                                                     /* current gm index                  */
  CData *idMean1 = NULL;                                                        /* old mean vectors                  */
  CData *idMean2 = NULL;                                                        /* new mean vectors                  */
  CData *idICov1 = NULL;                                                        /* old inverse covariance matricies  */
  CData *idICov2 = NULL;                                                        /* new inverse covariance matricies  */
  INT32 nCTIS;                                                                   /* component index for TIS           */
  CFst_Trim(_this,-1,0.);                                                       /* del unused trans. + states        */
  if(m_iGm->m_iMmap || m_iGm->m_idCmap) return O_K;                             /* no operation with mixtures/tying  */
  nCTIS=CData_FindComp(this->td,NC_TD_TIS);                                     /* get component index for TIS       */
  nOldGmCount=CData_GetNRecs(this->m_iGm->m_idMean);                            /* get number of old gm's            */
  DLPASSERT((lpNewGmMap=(INT32*)malloc(sizeof(INT32)*nOldGmCount)));              /* alloc membory for lpNewGmMap      */
  for(nGm=0;nGm<nOldGmCount;nGm++) lpNewGmMap[nGm]=-1;                          /* initialize lpNewGmMap[] = -1      */
  nNewGmCount=0;                                                                /* initialize nNewGmCount = 0        */
  for(nT=0;nT<CData_GetNRecs(this->td);nT++){                                   /* loop over all transitions         */
    if((nGm=(INT32)CData_Dfetch(this->td,nT,nCTIS))<0) continue;                 /*   cont. if trans. has no TIS      */
    if(lpNewGmMap[nGm]<0) lpNewGmMap[nGm]=nNewGmCount++;                        /*   create new gm if old hasn't one */
    CData_Dstore(this->td,lpNewGmMap[nGm],nT,nCTIS);                            /*   store new gm for trans          */
  }                                                                             /* <<                                */
  ICREATEEX(CData,idMean1,"CHmm:Hmm2.idMean1",NULL);                            /* create idMean1                    */
  ICREATEEX(CData,idMean2,"CHmm:Hmm2.idMean2",NULL);                            /* create idMean2                    */
  ICREATEEX(CData,idICov1,"CHmm:Hmm2.idICov1",NULL);                            /* create idICov1                    */
  ICREATEEX(CData,idICov2,"CHmm:Hmm2.idICov2",NULL);                            /* create idICov2                    */
  CGmm_Extract(this->m_iGm,idMean1,idICov1);                                    /* get old mean's and covariance's   */
  CData_Array(idMean2,CData_GetCompType(idMean1,0),                             /* alloc new means'                  */
    CData_GetNComps(idMean1),nNewGmCount);                                      /* |                                 */
  CData_Array(idICov2,CData_GetCompType(idICov1,0),                             /* alloc new covarince's             */
    CData_GetNComps(idICov1),CData_GetNComps(idICov1)*nNewGmCount);             /* |                                 */
  CData_SetNBlocks(idICov2,nNewGmCount);                                        /* set block size for new cov's      */
  for(nGm=0;nGm<nOldGmCount;nGm++) if(lpNewGmMap[nGm]>=0){                      /* loop over old gm's not to delete  */
    idMean2->m_bRec=TRUE; CData_Xstore(idMean2,idMean1,nGm,1,lpNewGmMap[nGm]);  /*   copy old mean's to new ones     */
    idICov2->m_bBlock=TRUE; CData_Xstore(idICov2,idICov1,nGm,1,lpNewGmMap[nGm]);/*   copy old covariance's to new    */
  }                                                                             /* <<                                */
  free(lpNewGmMap);                                                             /* free lpNewGmMap                   */
  this->m_iGm->m_bIcov=TRUE; CGmm_Setup(this->m_iGm,idMean2,idICov2,NULL);      /* setup gmm with new data           */
  IDESTROY(idMean1);                                                            /* destroy idMean1                   */
  IDESTROY(idMean2);                                                            /* destroy idMean2                   */
  IDESTROY(idICov1);                                                            /* destroy idICov1                   */
  IDESTROY(idICov2);                                                            /* destroy idICov2                   */
  return O_K;                                                                   /* all done                          */
}

/*
 * Manual page at hmm_man.def
 */
INT16 CGEN_PUBLIC CHmm::Bwalpha
(
  INT32   nUnit,
  CData* idWeights,
  CData* idAlpha
)
{
  CData *idG;
  CData *idH;
  CFst  *iFstRev;
  INT32 nK,nT,nTG;
  INT32 nNK,nNM,nNG,nNS,nNT,nNTG;
  INT32 nFT;
  INT32 nIcTis,nIcW,nIcRId;
  INT16 nWsr;
  FST_WTYPE nNeuAdd,nNeuMult;
  FST_WTYPE nObs;
  INT32 *lpStateMap;
  CData *idTmx = NULL;
  CData *idWeakTmx = NULL;
  FST_WTYPE *lpG;
  FST_WTYPE *lpH;
  FST_WTYPE *lpWeights;
  FST_WTYPE *lpWeightsMaped = NULL;
  FST_WTYPE *lpAlpha;

  if (nUnit<0 || nUnit>UD_XXU(_this))                                           /* Check unit index         */
    return IERROR(_this,FST_BADID,"unit",nUnit,0);                              /* |                        */

  /* unify final state of source uint */
  CFst_Fsunify(_this,nUnit);

  /* Init */
  if(_this->m_idBwalphaG) idG=AS(CData,_this->m_idBwalphaG); else {
    ICREATEEX(CData,idG,"bwalpha_g",CDlpObject_FindWord(BASEINST(_this),"bwalpha_g",WL_TYPE_FIELD));
    _this->m_idBwalphaG=BASEINST(idG);
  }
  if(_this->m_idBwalphaH) idH=AS(CData,_this->m_idBwalphaH); else {
    ICREATEEX(CData,idH,"bwalpha_h",CDlpObject_FindWord(BASEINST(_this),"bwalpha_h",WL_TYPE_FIELD));
    _this->m_idBwalphaH=BASEINST(idH);
  }
  if(_this->m_iBwalphaFstRev) iFstRev=AS(CFst,_this->m_iBwalphaFstRev); else {
    ICREATEEX(CFst,iFstRev,"bwalpha_fstrev",CDlpObject_FindWord(BASEINST(_this),"bwalpha_fstrev",WL_TYPE_FIELD));
    _this->m_iBwalphaFstRev=BASEINST(iFstRev);
  }
  CData_Tconvert(idWeights,idWeights,DLP_TYPE(FST_WTYPE));
  nNK=CData_GetNRecs(idWeights);
  nNG=CData_GetNComps(idWeights);
  nNS=UD_XS(_this,nUnit);
  nNT=UD_XT(_this,nUnit);
  nFT=UD_FT(_this,nUnit);
  nIcTis=CData_FindComp(AS(CData,_this->td),NC_TD_TIS);
  nWsr=CFst_Wsr_GetType(_this,&nIcW);
  nNeuAdd=CFst_Wsr_NeAdd(nWsr);
  nNeuMult=CFst_Wsr_NeMult(nWsr);
  if(_this->m_iGm->m_iMmap){
    CVmap *iVM=_this->m_iGm->m_iMmap;
    if(CData_IsEmpty(iVM->m_idWeakTmx)) idTmx=iVM->m_idTmx; else idWeakTmx=iVM->m_idWeakTmx;
    nNM=CVmap_GetOutDim(AS(CHmm,_this)->m_iGm->m_iMmap);
  }else nNM=nNG;

  /* Copy sync weights (or eventually map them) */
  if(idTmx || idWeakTmx){
    CVmap_Map(_this->m_iGm->m_iMmap,idWeights,idG);
    lpWeightsMaped=(FST_WTYPE*)dlp_malloc(nNM*sizeof(FST_WTYPE));
    CData_DrecFetch(idG,lpWeightsMaped,0,nNM,-1);
  }else
  CData_Copy(BASEINST(idG),BASEINST(idWeights));
  CData_Tconvert(idG,idG,DLP_TYPE(FST_WTYPE));
  CData_Array(idH,DLP_TYPE(FST_WTYPE),nNM,nNK);

  /* Reverse weights for backward calculation */
  lpG=(FST_WTYPE *)CData_XAddr(idG,0,0);
  lpH=(FST_WTYPE *)CData_XAddr(idH,0,0);
  for(nK=0;nK<(nNK+1)/2;nK++){
    INT32 nM;
    for(nM=0;nM<nNM;nM++){
      lpH[(nNK-1-nK)*nNM+nM]=lpG[(nK)*nNM+nM];
      lpH[(nK)*nNM+nM]=lpG[(nNK-1-nK)*nNM+nM];
    }
  }

  /* Reverse transducer */
  CFst_CopyUi(iFstRev,_this,NULL,nUnit);
  CData_Rindex(AS(CData,iFstRev->td),"~RID",nIcRId=CData_GetNComps(AS(CData,iFstRev->td)));
  CFst_Reverse(iFstRev,0);

  /* Create state map */
  lpStateMap=(INT32 *)dlp_malloc(nNS*sizeof(INT32));
  for(nT=0;nT<nNT;nT++){
    INT32 nTo=*(INT32*)CData_XAddr(AS(CData,iFstRev->td),nT,nIcRId);
    lpStateMap[TD_INI(_this,nTo+nFT)]=TD_TER(iFstRev,nT);
    lpStateMap[TD_TER(_this,nTo+nFT)]=TD_INI(iFstRev,nT);
  }

  /* Do forward + backward calculation */
  ISETOPTION(_this,"/fwd");
  ISETOPTION(iFstRev,"/fwd");
  CFst_Sdp(_this,_this,nUnit,idG);
  CFst_Sdp(iFstRev,iFstRev,0,idH);
  if(CData_GetNComps(idG)!=nNS) return IERROR(_this,FST_BADCTYPE,"idG",0,0);
  if(CData_GetNComps(idH)!=nNS) return IERROR(_this,FST_BADCTYPE,"idH",0,0);
  lpWeights=(FST_WTYPE*)CData_XAddr(idWeights,0,0);
  lpG=(FST_WTYPE*)CData_XAddr(idG,0,0);
  lpH=(FST_WTYPE*)CData_XAddr(idH,0,0);

  /* Calc observation probability */
  nObs = nNeuAdd;
  for(nT=0;nT<nNT;nT++){
    INT32 nM=*(FST_STYPE*)CData_XAddr(AS(CData,_this->td),nT+nFT,nIcTis);
    if(nM>=0){
      INT32 nIni=TD_INI(_this,nT+nFT);
      INT32 nTer=TD_TER(_this,nT+nFT);
      FST_WTYPE nAW = *(FST_WTYPE*)CData_XAddr(AS(CData,_this->td), nT+nFT, nIcW);
      FST_WTYPE nA = lpG[0*nNS+nIni];
      nA=CFst_Wsr_Op(_this, nA, nAW, OP_MULT);
      nA=CFst_Wsr_Op(_this, nA, lpWeightsMaped ? lpWeightsMaped[nM] : lpWeights[0*nNG+nM], OP_MULT);
      nA=CFst_Wsr_Op(_this, nA, lpH[(nNK-1)*nNS+lpStateMap[nTer]], OP_MULT);
      nObs = CFst_Wsr_Op(_this, nObs, nA, OP_ADD);
      }
  }
  if(lpWeightsMaped) dlp_free(lpWeightsMaped);

  /* Get number of different Gaussians per transitions */
  if(idTmx || idWeakTmx){
    INT32 *lpNMG=(INT32*)dlp_calloc(nNM,sizeof(INT32));
    for(nNTG=nT=0;nT<nNT;nT++){
      INT32 nM=*(FST_STYPE*)CData_XAddr(AS(CData,_this->td),nT+nFT,nIcTis);
      if(nM<0) nNTG++; else {
        if(!lpNMG[nM]){
          INT32 nG;
          if(idWeakTmx){
            INT32 nNG=CData_GetNRecs(idWeakTmx);
            for(nG=0;nG<nNG;nG++) if((INT32)CData_Dfetch(idWeakTmx,nG,nM*2)>=0) lpNMG[nM]++;
          }else{
            for(nG=0;nG<nNG;nG++) if(CData_Dfetch(idTmx,nM,nG)!=nNeuAdd) lpNMG[nM]++;
          }
        }
        nNTG+=lpNMG[nM];
      }
    }
    dlp_free(lpNMG);
  }else nNTG=nNT;

  /* Calc alpha values */
  CData_Array(idAlpha,DLP_TYPE(FST_WTYPE),nNTG,nNK+3);
  lpAlpha=(FST_WTYPE *)CData_XAddr(idAlpha,0,0);
  for(nT=nTG=0;nT<nNT;nT++){
    INT32 nM=*(FST_STYPE*)CData_XAddr(AS(CData,_this->td),nT+nFT,nIcTis);
    INT32 nIni=TD_INI(_this,nT+nFT);
    INT32 nTer=TD_TER(_this,nT+nFT);
    FST_WTYPE nAW = *(FST_WTYPE*)CData_XAddr(AS(CData,_this->td), nT+nFT, nIcW);
    INT32 nGmax = 1;
    INT32 nG;
    if(nM>=0){ if(idWeakTmx) nGmax=CData_GetNRecs(idWeakTmx); else if(idTmx) nGmax=nNG; }
    for(nG=0;nG<nGmax;nG++){
      FST_WTYPE nGW = nNeuMult;
      INT32 nGr;
      if(nM>=0){
        if(idWeakTmx){
          if((nGr=(INT32)CData_Dfetch(idWeakTmx,nG,nM*2))<0) continue;
          nGW=CData_Dfetch(idWeakTmx,nG,nM*2+1);
        }else if(idTmx){
          if((nGW=CData_Dfetch(idTmx,nM,nG))==nNeuAdd) continue;
          nGr=nG;
        }else nGr=nM;
      }else nGr=-1;
      lpAlpha[0*nNTG+nTG]=nT;
      lpAlpha[1*nNTG+nTG]=nGr;
      for(nK=0;nK<=nNK;nK++){
        FST_WTYPE nA;
        if(!nK && nM>=0) nA=nNeuAdd; else{
          nA=lpG[(nM<0?nK:nK-1)*nNS+nIni];
          nA=CFst_Wsr_Op(_this, nA, nAW, OP_MULT);
          if(nM>=0){
            nA=CFst_Wsr_Op(_this, nA, lpWeights[(nK-1)*nNG+nGr], OP_MULT);
            nA=CFst_Wsr_Op(_this, nA, nGW, OP_MULT);
          }
          nA=CFst_Wsr_Op(_this, nA, lpH[(nNK-nK)*nNS+lpStateMap[nTer]], OP_MULT);
          nA = CFst_Wsr_Op(_this, nA, nObs, OP_DIV);
          if(nWsr==FST_WSR_TROP) nA = fabs(nA-nNeuMult)<1E-9 ? nNeuMult : nNeuAdd;
        }
        if(nA>FST_WTYPE_MAX) nA=FST_WTYPE_MAX;
        lpAlpha[(nK+2)*nNTG+nTG]=nA;
      }
      nTG++;
      }
  }
  dlp_free(lpStateMap);

  CData_Reset(BASEINST(idG),TRUE);
  CData_Reset(BASEINST(idH),TRUE);
  CFst_Reset(BASEINST(iFstRev),TRUE);

  return O_K;
}

// EOF

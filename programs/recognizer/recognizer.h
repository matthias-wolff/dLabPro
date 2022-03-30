/* dLabPro program recognizer (dLabPro recognizer)
 * - Header file
 *
 * AUTHOR : Frank Duckhorn
 * PACKAGE: dLabPro/programs
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
#ifndef _CFG_C_SETOPT
#ifndef _RECOGNIZER_H
#define _RECOGNIZER_H

#include "dlp_config.h"
#include "dlp_cscope.h"
#include "dlp_base.h"
#include "dlp_gmm.h"
#include "dlp_fst.h"
#include "dlp_fstsearch.h"
#include "dlp_file.h"
#include "dlp_fvrtools.h"

#ifdef __TMS
  #undef long
  typedef unsigned long _ulong;
  #define long INT32
#endif

#undef dlp_malloc
#undef dlp_calloc
#undef dlp_realloc
#define dlp_malloc(A) __dlp_malloc(A,__FILE__,__LINE__,"kernel",NULL)
#define dlp_calloc(A,B) __dlp_calloc(A,B,__FILE__,__LINE__,"kernel",NULL)
#define dlp_realloc(A,B,C) __dlp_realloc(A,B,C,__FILE__,__LINE__,"kernel",NULL)

#define STR_LEN   2048

#define RECOOUT       E(res), E(cmd), E(sta), E(dbg), E(gui), E(vad), E(all)
#define RECOIN        E(none), E(cmd), E(fea)
#define RECOSEARCHTYP E(tp), E(as)
#define RECOREJTYP    E(off), E(phn), E(two)

#ifdef _CFG_C
#define E(X)  #X
const char *recoout_str[] = {RECOOUT,NULL};
const char *recoin_str[] = {RECOIN,NULL};
const char *recosearchtyp_str[] = {RECOSEARCHTYP,NULL};
const char *recorejtyp_str[] = {RECOREJTYP,NULL};
#undef E
#endif

extern const char *recoout_str[];
extern const char *recoin_str[];
extern const char *recosearchtyp_str[];
extern const char *recorejtyp_str[];
#define E(X)  O_##X
enum recoout {RECOOUT};
#undef E
#define E(X)  I_##X
enum recoin {RECOIN};
#undef E
#define E(X)  RS_##X
enum recosearchtyp {RECOSEARCHTYP};
#undef E
#define E(X)  RR_##X
enum recorejtyp {RECOREJTYP};
#undef E


struct recosearch {
  enum recosearchtyp eTyp;
  BOOL    bPrn;
  BOOL    bIter;
  INT32   nDebug;
  INT32   nASPrn1;
  INT32   nASPrn2;
  INT32   nAS2Prn;
  FLOAT32 nTPPrnW;
  INT32   nTPPrnH;
  INT32   nThreads;
  BOOL    bPermanent;
};

struct recorej {
  enum recorejtyp eTyp;
  FLOAT32 nTAD;
  FLOAT32 nTED;
  FLOAT32 nASTAD;
  FLOAT32 nTWOTWD;
  FLOAT32 nTWOTNWD;
  FLOAT32 nFVRTED;
  FLOAT32 nFVRLAM;
};

struct recovad {
  BOOL  bOffline;
  BOOL  bNoLimit;
  INT32 nMinSp;
  INT32 nMaxSp;
  INT32 nSigMin;
  struct dlm_vad_param lpBas;
  enum { VAD_NONE, VAD_ENG, VAD_GMM } nVadType;
  FLOAT32     nSigThr; /* Signal energy threshold */
  FLOAT32     nPfaThr; /* PFA energy threshold */
  FLOAT32     nGmmThr; /* Gaussian voice threshold (0..1) */
};

struct recofile {
  char lpsFName[STR_LEN];
  char lpsLab[STR_LEN];
}; 

struct recosig {
  char*    lpsLab;
  INT32    nLen;
  FLOAT32* lpSamples;
  INT32    nNldNum;
  INT32    nNldDim;
  FLOAT32* lpNld;
};


struct recoflst {
  INT32  nNum;
  INT32  nSize;
  struct recofile *lpF;
};

struct recopfa {
  INT32         nCoeff;
  FLOAT32       nSrate;
  MLP_CNVC_TYPE lpCnvc;
  FLOAT32      *lpWindow;
  struct dlm_fba_doframing_param lpFba;
};

struct recodfea {
  INT32    nPfaDim;
  INT32    nDeltaDim;
  INT32    nSfaDim;
  FLOAT32* lpDeltaT; /* Delta selection table (size: 2*nPfaDim) */
  FLOAT32* lpDeltaW; /* Delta weighting vector buffer (size: 2*nDeltaWL+1) */
  INT32    nDeltaWL; /* Delta window length */
  FLOAT32* lpX;      /* Normalization vector (size: nDeltaDim) */
  FLOAT32* lpW;      /* PCA Matrix (size: nDeltaDim*nSfaDim) */
};

struct recodses {
  CFst*       itGP;
  CFst*       itRN;
  CFst*       itRNr;
  CGmm*       itGM;
  CFstsearch* itSP;
  CFstsearch* itSPr;
};

struct recodvad {
  INT32    nPfaDim;
  INT32    nSfaDim;
  FLOAT32* lpX;      /* Normalization vector (size: nPfaDim) */
  FLOAT32* lpW;      /* PCA Matrix (size: nPfaDim*nSfaDim) */
  CGmm*    itGM;
};

struct recoddlg {
  CFst*         itDlg;
  FST_TID_TYPE* lpTI;
};

struct recores {
  UINT32 nTP;
  UINT32 nFP;
  UINT32 nTN;
  UINT32 nFN;
  UINT32 nTO;
  UINT32 nFO;
  UINT32 nN;
  UINT32 nNO;
  char   sLastRes[1024];
};

struct recotmp {
  CDlpFile*      iFile;
  CData*         idSig;
  CData*         idFea;
  CData*         idNld;
  CData*         idVFea;
  CData*         idVNld;
  INT32          nNVadForce;
  UINT8*         lpVadForce;
  FLOAT32        nDuration;
  struct recores rRes;
  INT32          nFstSel;
  const char*    sSigFname;
  FLOAT32*       lpColSig;
  INT32          nColSigLen;
  CFvrtools*     iFvr;
};

struct recocfg {
  BOOL              bHelp;
  BOOL              bOptionsList;
  enum recoout      eOut;
  enum recoin       eIn;
  char              sPostProc[STR_LEN];
  struct recosearch rSearch;
  struct recorej    rRej;
  BOOL              bFSTForce;
  FLOAT32           nFstSleep;
  struct recovad    rVAD;
  BOOL              bVADForce;
  BOOL              bNoiseRdc;
  INT32             nNoiseRdcLen;
  FLOAT32           nNoiseRdcPrc;
  BOOL              bMeasureTime;
  INT32             nAudioDev;
  BOOL              bAudioDevList;
  BOOL              bBinData;
  BOOL              bSkipNld;
  BOOL              bForce;
  BOOL              bCache;
  BOOL              bExit;
  INT32             nSigSampleRate;
  INT32             nSigChannel;
  struct recopfa    rPfa;
  struct recodfea   rDFea;
  struct recodses   rDSession;
  struct recodvad   rDVAD;
  struct recoddlg   rDDlg;
  struct recoflst   rFlst;
  char             *lpsIgn;
};

enum cfgload { CL_FEA,CL_SES,CL_GMM,CL_VAD,CL_DLG,CL_BIN,CL_N };

extern struct recotmp rTmp;
extern struct recocfg rCfg;

extern char cfgload[CL_N][STR_LEN];

#define IDESTROYBASE(A) if(A){ \
  void *lpBuf=A->m_lpDerivedInstance; \
  A->Destructor(A); \
  dlp_free(lpBuf); \
  A=NULL; \
}

#define IDESTROYFST(A) { \
  IDESTROYBASE(A->ud); \
  IDESTROYBASE(A->td); \
  IDESTROYBASE(A->sd); \
  IDESTROY(A); \
}

#define IDESTROYFILE(A) { \
  IDESTROYBASE(A->m_idFlistData); \
  IDESTROY(A); \
}  

/* recognizer.c */
void audio_devlist();
void rerror(const char *lpsErr,...);
void routput(enum recoout eOut,char bMark,const char *lpsMsg,...);
void dlg_upd(const char *lpsRes);

/* cfg.c */
BOOL setoption(char *lpsOpt,char *lpsVal,char bOnline);
BOOL cfginit(int argc,char** argv);
void cfgdone();
void opt_tmpcfg(const char *sFN);
void optdump();
void searchload(INT32 nFstSel);

/* hlp.c */
INT16 restore(CDlpObject *iInst, const char *lpsFilename, INT16 (*deserialize)(CDlpObject* __this, CDN3Stream* lpSrc), INT16 (*deserializeXml)(CDlpObject* __this, CXmlStream* lpSrc));
char *env_replace(char *lpsVal);

/* cmd.c */
void *cmdthread(void *lpDat);
void cmdqueueget();

#endif /* _RECOGNIZER_H */

#else /* _CFG_C_SETOPT */

typedef void (*recosetopt)(const char*,const char*,void*);
enum recoopttyp { OT_TRUE, OT_BOOL, OT_ENUM, OT_INT, OT_FLOAT, OT_STR, OT_LOAD, OT_IGN, OT_NUM };
struct {
  const char* lpsName;
  BOOL         bArg;
  recosetopt   lpFnc;
} rRecoOptTyp[OT_NUM]={
  { "no-arg",   FALSE, setopttrue  },
  { "yes/no",   TRUE,  setoptbool  },
  { "switch",   TRUE,  setoptenum  },
  { "int",      TRUE,  setoptint   },
  { "float",    TRUE,  setoptfloat },
  { "string",   TRUE,  setoptstr   },
  { "filename", TRUE,  setoptload  },
  { "ignore",   TRUE,  setoptignore},
};
struct recoopt {
  const char *lpsName;
  enum recoopttyp eTyp;
  BOOL bOnline;
  void *lpDst;
} rRecoOpts[]={
  { "h",               OT_TRUE,  FALSE, &rCfg.bHelp             },
  { "opts",            OT_TRUE,  FALSE, &rCfg.bOptionsList      },
  { "output",          OT_ENUM,  TRUE,  &rCfg.eOut              },
  { "out",             OT_ENUM,  TRUE,  &rCfg.eOut              },
  { "input",           OT_ENUM,  FALSE, &rCfg.eIn               },
  { "in",              OT_ENUM,  FALSE, &rCfg.eIn               },
  { "cache",           OT_BOOL,  TRUE,  &rCfg.bCache            },
  { "postproc.cmd",    OT_STR,   FALSE, &rCfg.sPostProc         },
  { "search.typ",      OT_ENUM,  FALSE, &rCfg.rSearch.eTyp      },
  { "search.debug",    OT_INT,   FALSE, &rCfg.rSearch.nDebug    },
  { "search.iterative",OT_BOOL,  FALSE, &rCfg.rSearch.bIter     },
  { "search.prn",      OT_BOOL,  FALSE, &rCfg.rSearch.bPrn      },
  { "search.tpprnw",   OT_FLOAT, FALSE, &rCfg.rSearch.nTPPrnW   },
  { "search.tpprnh",   OT_FLOAT, FALSE, &rCfg.rSearch.nTPPrnH   },
  { "search.asprn1",   OT_INT,   FALSE, &rCfg.rSearch.nASPrn1   },
  { "search.asprn2",   OT_INT,   FALSE, &rCfg.rSearch.nASPrn2   },
  { "search.as2prn",   OT_INT,   FALSE, &rCfg.rSearch.nAS2Prn   },
  { "search.threads",  OT_INT,   FALSE, &rCfg.rSearch.nThreads  },
  { "search.permanent",OT_BOOL,  FALSE, &rCfg.rSearch.bPermanent},
  { "rej.typ",         OT_ENUM,  FALSE, &rCfg.rRej.eTyp         },
  { "rej.tad",         OT_FLOAT, FALSE, &rCfg.rRej.nTAD         },
  { "rej.ted",         OT_FLOAT, FALSE, &rCfg.rRej.nTED         },
  { "rej.as.tad",      OT_FLOAT, FALSE, &rCfg.rRej.nASTAD       },
  { "rej.two.twd",     OT_FLOAT, FALSE, &rCfg.rRej.nTWOTWD      },
  { "rej.two.tnwd",    OT_FLOAT, FALSE, &rCfg.rRej.nTWOTNWD     },
  { "rej.fvr.ted",     OT_FLOAT, FALSE, &rCfg.rRej.nFVRTED      },
  { "rej.fvr.lambda",  OT_FLOAT, FALSE, &rCfg.rRej.nFVRLAM      },
  { "fst.force",       OT_BOOL,  FALSE, &rCfg.bFSTForce         },
  { "fst.sel",         OT_INT,   TRUE,  &rTmp.nFstSel           },
  { "fst.sleep",       OT_FLOAT, FALSE, &rCfg.nFstSleep         },
  { "vad.offline",     OT_BOOL,  TRUE,  &rCfg.rVAD.bOffline     },
  { "vad.nolimit",     OT_BOOL,  FALSE, &rCfg.rVAD.bNoLimit     },
  { "vad.minsp",       OT_INT,   FALSE, &rCfg.rVAD.nMinSp       },
  { "vad.maxsp",       OT_INT,   FALSE, &rCfg.rVAD.nMaxSp       },
  { "vad.sigmin",      OT_INT,   FALSE, &rCfg.rVAD.nSigMin      },
  { "vad.force",       OT_BOOL,  FALSE, &rCfg.bVADForce         },
  { "noise_reduce",    OT_BOOL,  FALSE, &rCfg.bNoiseRdc         },
  { "noise_reduce.len",OT_INT,   FALSE, &rCfg.nNoiseRdcLen      },
  { "noise_reduce.prc",OT_FLOAT, FALSE, &rCfg.nNoiseRdcPrc      },
  { "measure_time",    OT_BOOL,  FALSE, &rCfg.bMeasureTime      },
  { "sig.sample_rate", OT_INT,   FALSE, &rCfg.nSigSampleRate    },
  { "sig.sel_channel", OT_INT,   FALSE, &rCfg.nSigChannel       },
  { "audio.dev",       OT_INT,   FALSE, &rCfg.nAudioDev         },
  { "d",               OT_INT,   FALSE, &rCfg.nAudioDev         },
  { "audio.dev_list",  OT_BOOL,  FALSE, &rCfg.bAudioDevList     },
  { "l",               OT_TRUE,  FALSE, &rCfg.bAudioDevList     },
  { "data.bin",        OT_LOAD,  FALSE, (void*)CL_BIN           },
  { "skipnld",         OT_BOOL,  FALSE, &rCfg.bSkipNld          },
  { "force",           OT_BOOL,  FALSE, &rCfg.bForce            },
  { "data.feainfo",    OT_LOAD,  TRUE,  (void*)CL_FEA           },
  { "data.sesinfo",    OT_LOAD,  TRUE,  (void*)CL_SES           },
  { "data.gmm",        OT_LOAD,  TRUE,  (void*)CL_GMM           },
  { "data.vadinfo",    OT_LOAD,  FALSE, (void*)CL_VAD           },
  { "data.dialog",     OT_LOAD,  FALSE, (void*)CL_DLG           },
  { "ignore.*",        OT_IGN,   FALSE, &rCfg.lpsIgn            },
  { "uasr.*",          OT_IGN,   FALSE, &rCfg.lpsIgn            },
  { NULL, NULL, FALSE, NULL },
};

#endif


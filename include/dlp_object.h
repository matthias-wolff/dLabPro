/* dLabPro class CDlpObject (object)
 * - Header file
 *
 * AUTHOR : Matthias Wolff
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

#ifndef __DLPOBJECT_H
#define __DLPOBJECT_H

#include "dlp_base.h"
#include "kzl_hash.h"
#include "dlp_config.h"

#ifndef __NODN3STREAM
#include "dlp_dn3stream.h"
#else /* #ifndef __NODN3STREAM */
typedef struct CDN3Stream CDN3Stream;
#endif /*#ifndef __NODN3STREAM */

#ifndef __NOXMLSTREAM
#include "dlp_xmlstream.h"
#else /* #ifndef __NOXMLSTREAM */
typedef struct CXmlStream CXmlStream;
#endif /* #ifndef __NOXMLSTREAM */

/* Constants - Notify running dLabPro Classes */
#define _DLC

/* Constants - Backward compatibilty */
#define _SLC
#define _SLC22

/* Constants - Error levels */
#define EL_SILENT   -1
#define EL_FATAL     0
#define EL_ERROR     1
#define EL_WARNING   2
#define EL_WARNING2  3
#define EL_WARNING3  4
#define EL_ANY       255

/* Constants - Message levels */
#define DM_KERNEL   -1
#define ML_SILENT   -1
#define ML_IDENTIFY  1
#define ML_CHECK     2
#define ML_ANY       255

/* Constants - Errors */
#define ERR_DEBUG            -10000
#define ERR_REGWORD          -10001
#define ERR_DOUBLEDEF        -10002
#define ERR_NOTFIELD         -10003
#define ERR_BADARG           -10004
#define ERR_OBSOLETEID       -10005
#define ERR_ILLEGALQUALI     -10006
#define ERR_NOMEM            -10007
#define ERR_NULLINST         -10008
#define ERR_NOTOFKIND        -10009
#define ERR_DN3              -10010
#define ERR_NULLARG          -10011
#define ERR_INVALARG         -10012
#define ERR_FILEOPEN         -10013
#define ERR_FILECLOSE        -10014
#define ERR_SERIALIZE        -10015
#define ERR_DESERIALIZE      -10016
#define ERR_DESTROY          -10017
#define ERR_STREAMOBJ        -10018
#define ERR_NOTSUPPORTED     -10019
#define ERR_CREATEINSTANCE   -10020
#define ERR_CHDIR            -10021
#define ERR_GETCWD           -10022
#define ERR_GENERIC          -10023
#define ERR_BADPTR           -10024
#define ERR_INTERNAL         -10025
#define ERR_EXCEPTION        -10026
#define ERR_ILLEGALMEMBERVAL -10027
#define ERR_DANGEROUS        -10028

/* Constants - Word list */
#define WL_TYPE_METHOD       0x0001
#define WL_TYPE_FIELD        0x0002
#define WL_TYPE_OPTION       0x0004
#define WL_TYPE_ERROR        0x0008
#define WL_TYPE_INSTANCE     0x0010
#define WL_TYPE_FACTORY      0x0020
#define WL_TYPE_OPERATOR     0x0040
#define WL_TYPE_DONTCARE     0x7FFF

/* Constants - Class styles */
#define CS_AUTOINSTANCE      0x0001  /* Automatically instantiates              */
#define CS_AUTOACTIVATE      0x0002  /* Automatically activates                 */
#define CS_SINGLETON         0x0004  /* Only one instance allowed               */
#define CS_JEALOUS           0x0004  /* -- DEPRECATED --                        */
#define CS_SECONDARY         0x0008  /* To be placed into the secondary handler */
#define CS_CCLASS            0x0010  /* Use C call-backs                        */

/* Constants - Instance styles */
#define IS_GLOBAL            0x0001  /* Instance has global scope               */

/* Constants - Method flags */
#define MF_NONDISTINCT       0x0001

/* Constants - Field flags */
#define FF_NOSET             0x0001
#define FF_HIDDEN            0x0002  /* Implies no set */
#define FF_NONAUTOMATIC      0x0004
#define FF_NOSAVE            0x0008

/* Constants - Option flags */
#define OF_NONAUTOMATIC      0x0004

/* Constants - Class proc execution modes */
#define CPM_NORMAL           0
#define CPM_INSTALL          1
#define CPM_HELP             2

/* Constants - Check function operation modes */
#define CFM_RECURSIVE        1

/* Constants - PrintWordList */
#define PWL_WORDS            1
#define PWL_HELP             2
#define PWL_SYNTAX           3

/* Constants - Serialization of fields to strings */
#define C_FIELDSEP           ' '
#define C_FIELDDEL           "^"

/* Constants - File format for CDlpObject_Save */
#define SV_DEFAULT           0x0000
#define SV_XML               0x0001
#define SV_DN3               0x0002
#define SV_FILEFORMAT        0x0003  /* Mask for SV_XML, SV_DN3 */
#define SV_ZIP               0x0004

/* Instance pointer validation */
#define CHECK_IPTR(A,B) if (!CDlpObject_CheckInstancePtr(A,B)) (A)=NULL;

/* C/C++ language abstraction macros */
#ifdef __DLP_CSCOPE

  #define CHECK(A)                                                  \
    if (!A)                                                         \
    {                                                               \
      if (dlp_get_kernel_debug())                                   \
        printf("\n %s:%d; FAILURE: this-pointer is NULL or invalid",\
          __FILE__,__LINE__);                                       \
      return;                                                       \
    }
  #define CHECK_RV(A,B)                                             \
    if (!A)                                                         \
    {                                                               \
      if (dlp_get_kernel_debug())                                   \
        printf("\n %s:%d; FAILURE: this-pointer is NULL or invalid",\
          __FILE__,__LINE__);                                       \
      return B;                                                     \
    }
  #define CHECK_THIS()     CHECK(_this)
  #define CHECK_THIS_RV(A) CHECK_RV(_this,(A))

  #ifdef __cplusplus
    #define LPMF(A,B)                     &A::B
    #define LPMV(A)                       &(_this->A)
    #define BASEINST(A)                   A
    #define BASEINST_WORDTYPE(A)          A
    #define AS(A,B)                       ((A*)B)

    #define INVOKE_VIRTUAL_0(A  )         _this->A()
    #define INVOKE_VIRTUAL_1(A,B)         _this->A(B)

    #define INVOKE_BASEINST_0(A    )      CDlpObject_##A(_this)
    #define INVOKE_BASEINST_1(A,B  )      CDlpObject_##A(_this,B)
    #define INVOKE_BASEINST_2(A,B,C)      CDlpObject_##A(_this,B,C)

    #define GET_THIS_VIRTUAL(A)           A* _this = (A*)__this; CHECK_THIS();
    #define GET_THIS_VIRTUAL_RV(A,B)      A* _this = (A*)__this; CHECK_THIS_RV(B);

    #define GET_VIRTUAL(A,B,C)            A = (B*)C; CHECK(A);
    #define GET_VIRTUAL_RV(A,B,C,D)       A = (B*)C; CHECK_RV(A,D);

  #else /* #ifdef __cplusplus */
    #define LPMF(A,B)                     &A##_##B
    #define LPMV(A)                       &(_this->A)
    #define BASEINST(A)                   A->m_lpBaseInstance
    #define BASEINST_WORDTYPE(A)          ( strncmp(lpWord->ex.fld.lpType,"DlpObject",10) ? ((CDlpObject**)(A))[0] : (A) )
    #define AS(A,B)                       ((A*)(B->m_lpDerivedInstance))

    #define INVOKE_VIRTUAL_0(A  )         BASEINST(_this)->A(BASEINST(_this))
    #define INVOKE_VIRTUAL_1(A,B)         BASEINST(_this)->A(BASEINST(_this),B)

    #define INVOKE_BASEINST_0(A    )      CDlpObject_##A(BASEINST(_this))
    #define INVOKE_BASEINST_1(A,B  )      CDlpObject_##A(BASEINST(_this),B)
    #define INVOKE_BASEINST_2(A,B,C)      CDlpObject_##A(BASEINST(_this),B,C)

    #define GET_THIS_VIRTUAL(A)           A* _this = (A*)__this->m_lpDerivedInstance; CHECK_THIS();
    #define GET_THIS_VIRTUAL_RV(A,B)      A* _this = (A*)__this->m_lpDerivedInstance; CHECK_THIS_RV(B);

    #define GET_VIRTUAL(A,B,C)            A = (B*)(C->m_lpDerivedInstance); CHECK(A);
    #define GET_VIRTUAL_RV(A,B,C,D)       A = (B*)(C->m_lpDerivedInstance); CHECK_RV(A,D);

  #endif /* #ifdef __cplusplus */

  #define INVOKE_BASEINST_STATIC_1(A,B)   CDlpObject_##A(B)

#else /* #ifdef __DLP_CSCOPE */
  #ifdef __cplusplus
    #define LPMF(A,B)                     &A::B
    #define LPMV(A)                       &A
    #define BASEINST(A)                   A
    #define BASEINST_WORDTYPE(A)          A
    #define AS(A,B)                       ((A*)B)
    #define _this                         this

    #define INVOKE_VIRTUAL_0(A  )         A()
    #define INVOKE_VIRTUAL_1(A,B)         A(B)

    #define INVOKE_BASEINST_0(A    )      inherited::A()
    #define INVOKE_BASEINST_1(A,B  )      inherited::A(B)
    #define INVOKE_BASEINST_2(A,B,C)      inherited::A(B,C)

    #define INVOKE_BASEINST_STATIC_1(A,B) inherited::A(B)

  #else
    #error C scope not declared. For C projects, include "dlp_cscope.h" at the very beginning of each C source file.

  #endif /* #ifdef __cplusplus */

#endif /* #ifdef __DLP_CSCOPE */

/* Invoking callback functions */
#if defined __cplusplus
  #define INVOKE_CALLBACK(A,B) (A->*(B))()

#else /* #if (defined __cplusplus && defined __DLP_CSCOPE) */
  #define INVOKE_CALLBACK(A,B) (*(B))(A)

#endif /* #if (defined __cplusplus && defined __DLP_CSCOPE) */

/* Profiling macros */
#ifdef __MSVC
  #define BEGIN_TIMECRITICAL() {MMTIME __time0; \
                                timeGetSystemTime(&__time0,sizeof(MMTIME));

  #define END_TIMECRITICAL(A)  MMTIME __time1; \
                               timeGetSystemTime(&__time1,sizeof(MMTIME)); \
                               printf("\n %s time: %lu ms",A,(unsigned long)(__time1.u.ms-__time0.u.ms));}

  #define BEGIN_MEASURETIME() {MMTIME __time0; \
                                timeGetSystemTime(&__time0,sizeof(MMTIME));

  #define END_MEASURETIME(A)   MMTIME __time1; \
                               timeGetSystemTime(&__time1,sizeof(MMTIME)); \
                               A += __time1.u.ms-__time0.u.ms;}

#else  /* UNIX */
  #define BEGIN_TIMECRITICAL() {tms __tms0; INT64 __time0=times(&__tms0);
  #define END_TIMECRITICAL(A)  INT64 __time1; __time1=times(&__tms0); \
                               printf("\n %s Time: %lu ms",A,(unsigned long)((__time1-__time0)*10)); }

#endif

/* Register class factories (only to be used in main function) */
#ifdef __cplusplus

  #define UNREGISTER_CLASS(A)
  #define REGISTER_CLASS(A) \
  { \
    SWord w; A::GetClassInfo(&w); \
    CDlpObject_RegisterClass(&w); \
  }

#else /* #ifdef __cplusplus */

  #define UNREGISTER_CLASS(A)
  #define REGISTER_CLASS(A) \
  { \
    SWord w; A##_GetClassInfo(&w); \
    CDlpObject_RegisterClass(&w); \
  }

#endif /* #ifdef __cplusplus */

#define REGISTER_FIELD(A,B,C,D,E,F,G,H,I,J) REGISTER_FIELD_EX(_this,A,B,C,D,E,F,G,H,I,(J))

#ifdef __NOITP

#define REGISTER_METHOD(A,B,C,D,E,F,G)

#else /* #ifdef __NOITP */

/* Register words (GENERATED CODE ONLY!) */
#define REGISTER_METHOD(A,B,C,D,E,F,G) \
{ \
  SWord w; memset(&w,0,sizeof(SWord)); \
  dlp_strcpy(w.lpName,A); \
  dlp_strcpy(w.lpObsname,B); \
  w.nWordType          = WL_TYPE_METHOD; \
  w.lpComment          = D; \
  w.nFlags             = E; \
  w.ex.mth.lpfCallback = (LP_PMIC_FUNC)C; \
  w.ex.mth.lpSyntax    = F; \
  w.ex.mth.lpPostsyn   = G; \
  CDlpObject_RegisterWord(BASEINST(_this),&w); \
}

#endif /* #ifdef __NOITP */

#ifdef __NORTTI

#define REGISTER_FIELD_EX(_this,A,B,C,D,E,F,G,H,I,J) CDlpObject_SetFieldNoWord(BASEINST(_this),C,F,G,I,A,J);
#define REGISTER_INSTANCE(_this,A,B,C,D,E,F)
#define REGISTER_OPTION(A,B,C,D,E,F)
#define REGISTER_ERROR(A,B,C,D)

#else /* #ifdef __NORTTI */

#define REGISTER_FIELD_EX(_this,A,B,C,D,E,F,G,H,I,J) \
{ \
  SWord w; memset(&w,0,sizeof(SWord)); \
  dlp_strcpy(w.lpName,A); \
  dlp_strcpy(w.lpObsname,B); \
  w.nWordType          = WL_TYPE_FIELD; \
  w.lpData             = (void*)(C); \
  w.lpComment          = E; \
  w.nFlags             = F; \
  w.ex.fld.lpfCallback = (LP_FCC_FUNC)D; \
  w.ex.fld.nType       = G; \
  w.ex.fld.nArrlen     = H; \
  w.ex.fld.lpType      = I; \
  CDlpObject_RegisterWord(BASEINST(_this),&w,J); \
}

#define REGISTER_INSTANCE(_this,A,B,C,D,E,F) \
{ \
  SWord w; memset(&w,0,sizeof(SWord)); \
  dlp_strcpy(w.lpName,A); \
  dlp_strcpy(w.lpObsname,B); \
  w.nWordType          = WL_TYPE_INSTANCE; \
  w.lpData             = (void*)C; \
  w.lpComment          = D; \
  w.nFlags             = E; \
  CDlpObject_RegisterWord(BASEINST(_this),&w,F); \
}

#define REGISTER_OPTION(A,B,C,D,E,F) \
{ \
  SWord w; memset(&w,0,sizeof(SWord)); \
  dlp_strcpy(w.lpName,A); \
  dlp_strcpy(w.lpObsname,B); \
  w.nWordType          = WL_TYPE_OPTION; \
  w.lpData             = (void*)C; \
  w.lpComment          = E; \
  w.nFlags             = F; \
  w.ex.opt.lpfCallback = (LP_FCC_FUNC)D; \
  CDlpObject_RegisterWord(BASEINST(_this),&w); \
}

#define REGISTER_ERROR(A,B,C,D) \
{ \
  SWord w; memset(&w,0,sizeof(SWord)); \
  dlp_strcpy(w.lpName,A); \
  w.nWordType       = WL_TYPE_ERROR; \
  w.lpComment       = D; \
  w.nFlags          = B; \
  w.ex.err.nErrorID = C; \
  CDlpObject_RegisterWord(BASEINST(_this),&w); \
}

#define REGISTER_OPERATOR(A,B,C,D,E,F,G) \
{ \
  SWord w; memset(&w,0,sizeof(SWord)); \
  dlp_strcpy(w.lpName,A); \
  w.nWordType          = WL_TYPE_OPERATOR; \
  w.lpComment          = G; \
  w.ex.op.lpfCallback  = (void*)B; \
  w.ex.op.nOpc         = C; \
  w.ex.op.nRes         = D; \
  w.ex.op.nOps         = E; \
  w.ex.op.lpsSig       = F; \
  CDlpObject_RegisterOperator(BASEINST(_this),&w); \
}

#endif /* #ifdef __NORTTI */

/* Fields */
/* NOTE: 'B->lpfCallback != NULL' cannot be shortened (MS C++ compiler bug!)*/
/* ME 2003-08-12: second argument of va_arg makro depends on type:
     If the type in question is one that would normally be promoted, the pro-
       moted type should be used as the argument to va_arg().  The following de-
       scribes which types should be promoted (and to what):
       -   short is promoted to int
       -   float is promoted to double
       -   char is promoted to int */
#define REFUSE_FIELD_CHANGE return FALSE;
#define DOFIELDUPDATE(A,B,C) \
  { \
    A ghost = *(A*)B->lpData; \
    *(A*)B->lpData = *(A*)C; \
    if (B->ex.fld.lpfCallback != NULL && \
      NOK(INVOKE_CALLBACK(_this,B->ex.fld.lpfCallback))) \
    { \
      *(A*)B->lpData = ghost; \
    } \
  }

#ifdef __NORTTI

#define REMAP_FIELD(A,B)
#define UNREMAP_FIELD(A)

#else /* #ifdef __NORTTI */

#define REMAP_FIELD(A,B) \
  { \
    SWord* lpWord; \
    DLPASSERT((lpWord = CDlpObject_FindWord(BASEINST(_this),A,WL_TYPE_FIELD))!=NULL); \
    lpWord->lpData = (void*)&(B); \
  }
#define UNREMAP_FIELD(A) \
  { \
    SWord* lpWord; \
    DLPASSERT((lpWord = CDlpObject_FindWord(BASEINST(_this),A,WL_TYPE_FIELD))!=NULL); \
    lpWord->lpData = NULL; \
  }

#endif /* #ifdef __NORTTI */

/* Default system functions */
#define INIT      INVOKE_BASEINST_1(Init,0)
#define DONE
#define RESET       INVOKE_BASEINST_1(Reset,bResetMembers)
#define SAVE_DN3    INVOKE_BASEINST_1(Serialize,lpDest)
#define SAVE_XML    INVOKE_BASEINST_1(SerializeXml,lpDest)
#define RESTORE_DN3 INVOKE_BASEINST_1(Deserialize,lpSrc)
#define RESTORE_XML INVOKE_BASEINST_1(DeserializeXml,lpSrc)
#define COPY        INVOKE_BASEINST_1(Copy,__iSrc)
#define CLASSPROC   INVOKE_BASEINST_0(ClassProc)
#define INSTALL     INVOKE_BASEINST_STATIC_1(InstallProc,lpItp)

/* Error macros */
#define IERROR(INST,ERR,A,B,C) \
  CDlpObject_Error(BASEINST(INST),__FILE__,__LINE__,ERR,A,B,C)
#define IERRORAT(INST,FILE,LINE,ERR,A,B,C) \
  { \
    char __sBuffer[L_PATH]; \
    INT32 __nLine; \
    CDlpObject_GetErrorPos(__sBuffer,&__nLine); \
    CDlpObject_SetErrorPos(FILE,LINE); \
    IERROR(INST,ERR,A,B,C); \
    CDlpObject_SetErrorPos(__sBuffer,__nLine); \
  }
#define CERROR(CLASS,ERR,A,B,C) \
  { \
    CLASS* iTmp; ICREATEEX(CLASS,iTmp,"~CERROR",NULL); \
    CDlpObject_Error(BASEINST(iTmp),__FILE__,__LINE__,ERR,A,B,C); \
    IDESTROY(iTmp); \
  }

/* Message macros */
#define IFCHECK        if (BASEINST(_this)->m_nCheck >  0)
#define IFCHECKEX(A)   if (BASEINST(_this)->m_nCheck >= A)
#define MSG(A,B,C,D,E) if (BASEINST(_this)->m_nCheck >= A) dlp_message(B,C,D,E);

/* Exception macros */
#define DLPTHROW(A) { \
  if (A!=NOT_EXEC) IERROR(_this,ERR_EXCEPTION,A,__FILE__,__LINE__); \
  goto L_EXCEPTION_##A; \
}
#define DLPCATCH(A) L_EXCEPTION_##A:

/* C/C++ helpers */
#ifdef __cplusplus

#define ICREATEEX(CLASS,INSTANCE,NAME,CONT) {\
  INSTANCE=new CLASS(NAME); \
  if (INSTANCE) INSTANCE->m_lpContainer = CONT; \
}
#define IDESTROY(INSTANCE) { delete INSTANCE; INSTANCE=NULL; }

/* TODO: ? Make this the standard procedure in CDlpObject_ResetField ? */
#define IFIELD_RESET(A,B) IFIELD_RESET_EX(_this,A,B)

#define IFIELD_RESET_EX(A,B,C) \
  { \
    SWord* lpWord = CDlpObject_FindWord(A,C,WL_TYPE_FIELD); \
    if (lpWord && lpWord->ex.fld.nType==T_INSTANCE) \
    { \
      if (!*(CDlpObject**)lpWord->lpData) \
      { \
        CDlpObject* iGhost = CDlpObject_CreateInstanceOf(#B,C); \
        if (!iGhost) return NOT_EXEC; \
        *(CDlpObject**)lpWord->lpData = iGhost; \
        (*(CDlpObject**)lpWord->lpData)->m_lpContainer=lpWord; \
        lpWord->ex.fld.nISerialNum = iGhost->m_nSerialNum; \
      } \
      else (*(CDlpObject**)lpWord->lpData)->Reset(); \
    } \
    else IERROR(A,ERR_INTERNAL," (undef. field identifier)",__FILE__,__LINE__); \
  }
/*
#define IFIELD_RESET_EX(A,B,C) \
  { \
    SWord* lpWord = CDlpObject_FindWord(A,C,WL_TYPE_FIELD); \
    if (lpWord && lpWord->ex.fld.nType==T_INSTANCE) \
    { \
      if (!*(CDlpObject**)lpWord->lpData) \
      { \
        B* lpGhost; \
        ICREATEEX(B,lpGhost,C); \
        if (!lpGhost) return NOT_EXEC; \
        *(CDlpObject**)lpWord->lpData=BASEINST(lpGhost); \
        (*(CDlpObject**)lpWord->lpData)->m_lpContainer=lpWord; \
      } \
      else (*(CDlpObject**)lpWord->lpData)->Reset(); \
    } \
  }
*/
/* Overlapping argument helpers */
/* Rewritten because old version did not allow nested calls of CREATEVIRTUAL/DESTROYVIRTUAL
   New version of CREATEVIRTUAL tests if OUT is already a virtual instance (instance name is '#')
   and appends a dot ('.') at the instance name if this is the case. Otherwise it just creates a
   virtual instance. DESTROYVIRTUAL looks at the instance name and does destroy the instance
   only if there is no dot at the end of the name. If the last character is a dot it removes
   the last character. Only L_NAMES nested calls a of CREATEVIRTUAL/DESTROYVIRTUAL are possible
   due to limited size of m_lpInstanceName. If this number is exceeded an assertion occures.
*/
#define CREATEVIRTUAL(CLASS,IN,OUT) \
  if (OUT == IN) \
  { \
    OUT = new(CLASS)("#"); \
    if (!OUT) return IERROR(_this,ERR_NOMEM,0,0,0); \
  } \
  else if(OUT->m_lpInstanceName[0]=='#') \
  { \
    DLPASSERT(dlp_strlen(OUT->m_lpInstanceName)<L_NAMES-2); \
    dlp_strcat(OUT->m_lpInstanceName,"."); \
  }

#define DESTROYVIRTUAL(IN,OUT) \
  if (strncmp(OUT->m_lpInstanceName,"#",L_NAMES)==0) \
  { \
    IN->Copy(OUT); \
    delete OUT; \
    OUT=IN; \
  } \
  else if(OUT->m_lpInstanceName[0]=='#') \
    OUT->m_lpInstanceName[dlp_strlen(OUT->m_lpInstanceName)-1]=0;

#else /* #ifdef __cplusplus */

#define ICREATEEX(CLASS,INSTANCE,NAME,CONT) \
  {\
    INSTANCE=(CLASS*)calloc(1,sizeof(CLASS));\
    if (INSTANCE) { CLASS##_Constructor(INSTANCE,NAME,1); BASEINST(INSTANCE)->m_lpContainer = CONT; }\
  }
/*#define ICREATEEXV(A,B,C)  { B=(A*)__dlp_calloc(1,sizeof(A),__FILE__,__LINE__,"C-ICREATEEX",#C); if (B) A##_Constructor(B,C,0); }*/
#define IDESTROY(A)        { BASEINST(A)->Destructor(BASEINST(A)); dlp_free(A); A=NULL; }

/* TODO: ? Make this the standard procedure in CDlpObject_ResetField ? */
#define IFIELD_RESET(A,B) IFIELD_RESET_EX(_this,A,B) \

#define IFIELD_RESET_EX(A,B,C) \
  { \
    SWord* lpWord = CDlpObject_FindWord(BASEINST(A),C,WL_TYPE_FIELD); \
    if (lpWord && lpWord->ex.fld.nType==T_INSTANCE) \
    { \
      if (!*(CDlpObject**)lpWord->lpData) \
      { \
        CDlpObject* iGhost = CDlpObject_CreateInstanceOf(#B,C); \
        if (!iGhost) return NOT_EXEC; \
        iGhost = BASEINST_WORDTYPE(iGhost); \
        *(CDlpObject**)lpWord->lpData=iGhost; \
        (*(CDlpObject**)lpWord->lpData)->m_lpContainer=lpWord; \
      } \
      else (*(CDlpObject**)lpWord->lpData)->Reset(*(CDlpObject**)lpWord->lpData,TRUE); \
    } \
  }

/* Overlapping argument helpers */
#define CREATEVIRTUAL(CLASS,IN,OUT) \
  if (OUT == IN) \
  { \
    ICREATEEX(CLASS,OUT,"#",NULL); \
    if (!OUT) return IERROR(_this,ERR_NOMEM,0,0,0); \
  } \
  else if(BASEINST(OUT)->m_lpInstanceName[0]=='#') \
  { \
    DLPASSERT(dlp_strlen(BASEINST(OUT)->m_lpInstanceName)<L_NAMES-2); \
    dlp_strcat(BASEINST(OUT)->m_lpInstanceName,"."); \
  }

#define DESTROYVIRTUAL(IN,OUT) \
  if (strncmp(BASEINST(OUT)->m_lpInstanceName,"#",L_NAMES)==0) \
  { \
    BASEINST(IN)->Copy(BASEINST(IN),BASEINST(OUT));  \
    IDESTROY(OUT);  \
    OUT=IN; \
  } \
  else if (BASEINST(OUT)->m_lpInstanceName[0]=='#') \
    BASEINST(OUT)->m_lpInstanceName[dlp_strlen(BASEINST(OUT)->m_lpInstanceName)-1]=0;

#endif /* #ifdef __cplusplus */

#define ICREATE(A,B,CONT) ICREATEEX(A,B,#B,CONT)

#ifdef __cplusplus

  #define IRESETOPTIONS(A) A->ResetAllOptions(FALSE);

#else /* #ifdef __cplusplus */

  #define IRESETOPTIONS(A) BASEINST(A)->ResetAllOptions(BASEINST(A),FALSE);

#endif /* #ifdef __cplusplus */

#ifdef __DLP_CSCOPE

    #define ISETFIELD(A,B,C)  CDlpObject_SetField(BASEINST(A),B,C)
    #define ISETOPTION(A,B)   CDlpObject_SetOption(BASEINST(A),CDlpObject_FindWord(BASEINST(A),B,WL_TYPE_OPTION));

#else /* #ifdef __DLP_CSCOPE */

  #define ISETFIELD(A,B,C)  A->SetField(B,C)
  #define ISETOPTION(A,B)   A->SetOption(A->FindWord(B,WL_TYPE_OPTION));

#endif

  #define ISETFIELD_LVALUE(A,B,C)  ISETFIELD(A,CDlpObject_FindWord(BASEINST(A),B,WL_TYPE_FIELD),(void*)&(C))
  #define ISETFIELD_SVALUE(A,B,C)  { const char* lpsTmp=C;ISETFIELD(A,CDlpObject_FindWord(BASEINST(A),B,WL_TYPE_FIELD),(void*)&lpsTmp); }
  #define ISETFIELD_RVALUE(A,B,C)  { \
    SWord* lpWord = CDlpObject_FindWord(BASEINST(A),B,WL_TYPE_FIELD); \
    switch(lpWord->ex.fld.nType) { \
      case T_BOOL:   {      BOOL bTmp=(     BOOL)(C);ISETFIELD(A,lpWord,(void*)&bTmp); } break; \
      case T_UCHAR:  {     UINT8 nTmp=(    UINT8)(C);ISETFIELD(A,lpWord,(void*)&nTmp); } break; \
      case T_CHAR:   {      INT8 nTmp=(     INT8)(C);ISETFIELD(A,lpWord,(void*)&nTmp); } break; \
      case T_USHORT: {    UINT16 nTmp=(   UINT16)(C);ISETFIELD(A,lpWord,(void*)&nTmp); } break; \
      case T_SHORT:  {     INT16 nTmp=(    INT16)(C);ISETFIELD(A,lpWord,(void*)&nTmp); } break; \
      case T_UINT:   {    UINT32 nTmp=(   UINT32)(C);ISETFIELD(A,lpWord,(void*)&nTmp); } break; \
      case T_INT:    {     INT32 nTmp=(    INT32)(C);ISETFIELD(A,lpWord,(void*)&nTmp); } break; \
      case T_ULONG:  {    UINT64 nTmp=(   UINT64)(C);ISETFIELD(A,lpWord,(void*)&nTmp); } break; \
      case T_LONG:   {     INT64 nTmp=(    INT64)(C);ISETFIELD(A,lpWord,(void*)&nTmp); } break; \
      case T_FLOAT:  {   FLOAT32 nTmp=(  FLOAT32)(C);ISETFIELD(A,lpWord,(void*)&nTmp); } break; \
      case T_DOUBLE: {   FLOAT64 nTmp=(  FLOAT64)(C);ISETFIELD(A,lpWord,(void*)&nTmp); } break; \
    } }
  #define ISETFIELD_CVALUE(A,B,C)  { \
    SWord* lpWord = CDlpObject_FindWord(BASEINST(A),B,WL_TYPE_FIELD); \
    switch(lpWord->ex.fld.nType) { \
        case T_COMPLEX:{ COMPLEX64 nTmp=(COMPLEX64)(C);ISETFIELD(A,lpWord,(void*)&nTmp); } break; \
    } }

/* Function types and forward declarations */
#ifdef __cplusplus

  class CDlpObject;
  class CDlpClFactory;

#else /* #ifdef __cplusplus */

  struct CDlpObject;

#endif /* #ifdef __cplusplus */

/* Stack item struct */
typedef struct _tag_StkItm
{
  INT16 nType;
  union
  {
    BOOL        b;
    COMPLEX64   n;
    char*       s;
#ifdef __cplusplus
    CDlpObject* i;
#else /* #ifdef __cplusplus */
    struct CDlpObject* i;
#endif /* #ifdef __cplusplus */
  } val;
} StkItm;

/* Method invocation context */
#ifdef __cplusplus
typedef StkItm*     (* LPF_MIC_GETX)(CDlpObject*, INT16, StkItm*);
typedef BOOL        (* LPF_MIC_GETB)(CDlpObject*, INT16, INT16  );
typedef COMPLEX64   (* LPF_MIC_GETN)(CDlpObject*, INT16, INT16  );
typedef CDlpObject* (* LPF_MIC_GETI)(CDlpObject*, INT16, INT16  );
typedef char*       (* LPF_MIC_GETS)(CDlpObject*, INT16, INT16  );
typedef void        (* LPF_MIC_PUTB)(CDlpObject*, BOOL          );
typedef void        (* LPF_MIC_PUTN)(CDlpObject*, COMPLEX64     );
typedef void        (* LPF_MIC_PUTI)(CDlpObject*, CDlpObject*   );
typedef void        (* LPF_MIC_PUTS)(CDlpObject*, const char*   );
typedef const char* (* LPF_MIC_NXTK)(CDlpObject*, BOOL          );
typedef void        (* LPF_MIC_RFTK)(CDlpObject*                );
typedef const char* (* LPF_MIC_NXTD)(CDlpObject*                );
#else /* #ifdef __cplusplus */
typedef StkItm*     (* LPF_MIC_GETX)(struct CDlpObject*, INT16, StkItm*);
typedef BOOL        (* LPF_MIC_GETB)(struct CDlpObject*, INT16, INT16  );
typedef COMPLEX64   (* LPF_MIC_GETN)(struct CDlpObject*, INT16, INT16  );
typedef struct CDlpObject* (* LPF_MIC_GETI)(struct CDlpObject*, INT16, INT16  );
typedef char*       (* LPF_MIC_GETS)(struct CDlpObject*, INT16, INT16  );
typedef void        (* LPF_MIC_PUTB)(struct CDlpObject*, BOOL          );
typedef void        (* LPF_MIC_PUTN)(struct CDlpObject*, COMPLEX64     );
typedef void        (* LPF_MIC_PUTI)(struct CDlpObject*, struct CDlpObject*   );
typedef void        (* LPF_MIC_PUTS)(struct CDlpObject*, const char*   );
typedef const char* (* LPF_MIC_NXTK)(struct CDlpObject*, BOOL          );
typedef void        (* LPF_MIC_RFTK)(struct CDlpObject*                );
typedef const char* (* LPF_MIC_NXTD)(struct CDlpObject*                );
#endif /* #ifdef __cplusplus */

typedef struct __tag_SMic
{
#ifdef __cplusplus
  CDlpObject*   iCaller;
#else /* #ifdef __cplusplus */
  struct CDlpObject* iCaller;
#endif /* #ifdef __cplusplus */
  LPF_MIC_GETX  GetX;
  LPF_MIC_GETB  GetB;
  LPF_MIC_GETN  GetN;
  LPF_MIC_GETI  GetI;
  LPF_MIC_GETS  GetS;
  LPF_MIC_PUTB  PutB;
  LPF_MIC_PUTN  PutN;
  LPF_MIC_PUTI  PutI;
  LPF_MIC_PUTS  PutS;
  LPF_MIC_NXTK  NextToken;
  LPF_MIC_RFTK  RefuseToken;
  LPF_MIC_NXTD  NextTokenDel;
} SMic;

#define MIC_GET_X(ARG,LPSI) CDlpObject_MicGetX(BASEINST(_this),ARG,LPSI)
#define MIC_GET_B(ARG,POS)  CDlpObject_MicGetB(BASEINST(_this),ARG,POS)
#define MIC_GET_N(ARG,POS)  CDlpObject_MicGetN(BASEINST(_this),ARG,POS).x
#define MIC_GET_C(ARG,POS)  CDlpObject_MicGetN(BASEINST(_this),ARG,POS)
#define MIC_GET_S(ARG,POS)  CDlpObject_MicGetS(BASEINST(_this),ARG,POS)
#define MIC_GET_I(ARG,POS)  CDlpObject_MicGetI(BASEINST(_this),ARG,POS)
#define MIC_PUT_X(LPSI)     CDlpObject_MicPutX(BASEINST(_this),LPSI)
#define MIC_PUT_B(VAL)      CDlpObject_MicPutB(BASEINST(_this),VAL)
#define MIC_PUT_N(VAL)      CDlpObject_MicPutN(BASEINST(_this),CMPLX(VAL))
#define MIC_PUT_C(VAL)      CDlpObject_MicPutN(BASEINST(_this),VAL)
#define MIC_PUT_S(VAL)      CDlpObject_MicPutS(BASEINST(_this),VAL)
#define MIC_PUT_I(VAL)      CDlpObject_MicPutI(BASEINST(_this),VAL)

#ifdef __cplusplus
#  define MIC_GET_I_EX(INST,CLASS,ARG,POS) \
    (CLASS*)MIC_GET_I(ARG,POS); \
    if (INST && !INST->IsKindOf(""#CLASS"")) \
      return IERROR(_this,ERR_BADARG,ARG,""#CLASS"",0);
#else /* #ifdef __cplusplus */
#  define MIC_GET_I_EX(INST,CLASS,ARG,POS) \
    (CLASS*)MIC_GET_I(ARG,POS); \
    if (INST && !BASEINST(INST)->IsKindOf(BASEINST(INST),""#CLASS"")) \
      return IERROR(_this,ERR_BADARG,ARG,""#CLASS"",0);
#endif /* #ifdef __cplusplus */
/*
#define MIC_GET_I_EX(INST,CLASS,ARG,POS) \
  (CLASS*)CDlpObject_MicGetI(BASEINST(_this),ARG,POS); \
  if (INST && !CDlpObject_IsKindOf(BASEINST(INST),""#CLASS"")) \
    return IERROR(_this,ERR_BADARG,""#CLASS"",0,0);
*/

#define MIC_CHECK \
  if (!CDlpObject_MicGet(BASEINST(_this))) \
    return IERROR(_this,ERR_GENERIC,"No invocation context",0,0);

#define MIC_NEXTTOKEN       CDlpObject_MicNextToken(BASEINST(_this),TRUE )
#define MIC_NEXTTOKEN_FORCE CDlpObject_MicNextToken(BASEINST(_this),FALSE)
#define MIC_CMDLINE         CDlpObject_MicCmdLine(_this)
#define MIC_REFUSETOKEN     CDlpObject_MicRefuseToken(BASEINST(_this))

/* Function types and forward declarations */
#ifdef __cplusplus

  typedef INT16       (CDlpObject::* LP_PMIC_FUNC)(void       );                /* C++ Primary method invocation     */
  typedef INT16       (CDlpObject::* LP_FCC_FUNC )(void       );                /* C++ Field changed                 */
  typedef INT16       (CDlpObject::* LP_OCC_FUNC )(void       );                /* C++ Option changed                */
  typedef INT16       (* LP_INSTALL_PROC         )(void*      );                /* Class installation function       */
  typedef CDlpObject* (* LP_FACTORY_PROC         )(const char*);                /* Instanciation function            */
  typedef INT16       (* LP_FOP_FUNC             )(INT16,StkItm*,StkItm*);      /* C++ signal operation function    */
  typedef INT16       (* LP_MOP_FUNC             )(CDlpObject*,void*,           /* C++ matrix operation function     */
                                                   INT16,void*,INT16,INT16);    /* |                                 */

#else /* #ifdef __cplusplus */

  typedef INT16 (* LP_PMIC_FUNC   )(struct CDlpObject*);                        /* C Primary method invocation       */
  typedef INT16 (* LP_FCC_FUNC    )(struct CDlpObject*);                        /* C Field changed                   */
  typedef INT16 (* LP_OCC_FUNC    )(struct CDlpObject*);                        /* C Option changed                  */
  typedef INT16 (* LP_INSTALL_PROC)(void*             );                        /* Class installation function       */
  typedef void* (* LP_FACTORY_PROC)(const char*       );                        /* Instanciation function            */
  typedef INT16 (* LP_FOP_FUNC    )(INT16,StkItm*,StkItm*);                     /* C signal operation function      */
  typedef INT16 (* LP_MOP_FUNC    )(struct CDlpObject*,void*,                   /* C matrix operation function       */
                                    INT16,void*,INT16,INT16);                   /* |                                 */

#endif /* #ifdef __cplusplus */

#ifdef __cplusplus
typedef INT16 (CALLBACK* LPF_FORMEX)                                            /* Formula execution function        */
  (const SMic*, CDlpObject*, const char*, char*);                               /* |                                 */
#else /* #ifdef __cplusplus */
typedef INT16 (CALLBACK* LPF_FORMEX)                                            /* Formula execution function        */
  (const SMic*, struct CDlpObject*, const char*, char*);                               /* |                                 */
#endif /* #ifdef __cplusplus */

/* Struct SVersion */
typedef struct SVersion
{
  char no[20];
  char date[12];
} SVersion;

/* Struct SWord */
typedef struct
{
#ifdef __cplusplus
  CDlpObject* lpContainer;                                                      /* Container instance                */
#else
  struct CDlpObject* lpContainer;                                               /* Container instance                */
#endif
  INT16         nWordType;                                                      /* WL_TYPE_XXX                       */
  char          lpName[L_NAMES];                                                /* Name                              */
  char          lpObsname[L_NAMES];                                             /* Obsolete name                     */
  void*         lpData;                                                         /* Pointer to member or instance     */
  const char*   lpComment;                                                      /* Pointer to brief help text        */
  INT32         nFlags;                                                         /* Depend on nWordType               */
  union
  {
    struct                                                                      /* -- WL_TYPE_METHOD --------------- */
    {
      LP_PMIC_FUNC    lpfCallback;                                              /* Primary method invocation function*/
      const char*     lpSyntax;                                                 /* Syntax description                */
      const char*     lpPostsyn;                                                /* Post syntax description           */
    } mth;
    struct
    {
      LP_OCC_FUNC     lpfCallback;                                              /* Option changed function           */
    } opt;
    struct                                                                      /* -- WL_TYPE_FIELD ---------------- */
    {
      LP_FCC_FUNC     lpfCallback;                                              /* Field changed function            */
      INT16           nType;                                                    /* Variable type                     */
      INT32           nArrlen;                                                  /* Array size (1 of no array)        */
      const char*     lpType;                                                   /* INSTANCE: dLabPro class id (RTTI) */
                                                                                /* POINTER : C/C++ type name         */
      union                                                                     /* Copy of init value                */
      {
        INT64     n;
        COMPLEX64 c;
        char*     s;
        void*     p;
      } lpInit;
      UINT64          nISerialNum;                                              /* Serial number of an instance      */
    } fld;
    struct                                                                      /* -- WL_TYPE_ERROR ---------------- */
    {
      INT32           nErrorID;                                                 /* Numeric error ID                  */
    } err;
    struct                                                                      /* -- WL_TYPE_FACTORY -------------- */
    {
      LP_INSTALL_PROC lpfInstall;                                               /* Class installation function       */
      LP_FACTORY_PROC lpfFactory;                                               /* Instanciation function            */
      const char*     lpProject;                                                /* Project code                      */
      const char*     lpBaseClass;                                              /* Base class identifier             */
      const char*     lpAutoname;                                               /* Auto instanciation identifier     */
      const char*     lpCname;                                                  /* C/C++ identifier                  */
      SVersion        version;                                                  /* Version information               */
      const char*     lpAuthor;                                                 /* Author of class                   */
    } fct;
    struct                                                                      /* -- WL_TYPE_OPERATOR               */
    {
      void*           lpfCallback;                                              /* Operation method invocation function*/
      INT16           nOpc;                                                     /* Operation code                    */
      INT16           nRes;                                                     /* Number of results                 */
      INT16           nOps;                                                     /* Number of operands                */
      const char*     lpsSig;                                                   /* Signature                         */
    } op;
  } ex;
} SWord;

/* Class CDlpObject */
#ifdef __cplusplus

class CDlpObject
{
  /* Constructors and destructors (dlpinst.c) */
  public:
  CDlpObject(const char* lpsInstanceName, BOOL bCallVirtual = 1);
  virtual ~CDlpObject();

  /* Virtual and static system member functions (dlpinst.c) */
  public:
  virtual INT16       AutoRegisterWords();
  virtual INT16       Reset(BOOL bResetMembers = 1);
  virtual INT16       Init(BOOL bCallVirtual = 0);
  virtual INT16       Serialize(CDN3Stream* lpiDst);
  virtual INT16       Deserialize(CDN3Stream* lpiSrc);
  virtual INT16       SerializeXml(CXmlStream* lpiDst);
  virtual INT16       DeserializeXml(CXmlStream* lpiSrc);
  virtual INT16       Copy(CDlpObject* iSrc);
  virtual INT16       ClassProc();
  static  INT16       InstallProc(void* iItp);
  static  CDlpObject* CreateInstance(const char* lpsName);
  static  INT16       GetClassInfo(SWord* lpClassWord);
  virtual INT16       GetInstanceInfo(SWord* lpClassWord);
  virtual BOOL        IsKindOf(const char* lpsClassName);
  virtual INT16       ResetAllOptions(BOOL bInit = 0);
          INT16       Check(INT32 nMode);

  /* Dictionary functions (dlpi_dict.c) */
  public:
  SWord*       RegisterWord(const SWord* lpWord,...);
  SWord*       RegisterOperator(const SWord* lpWord);
  INT16        UnregisterWord(SWord* lpWord, INT16 bDeleteInstance = 1);
  INT16        UnregisterAllWords();
  INT16        UnregisterAllOperators();
  SWord*       FindWordInternal(const char* lpsName, INT16 nMask = WL_TYPE_DONTCARE);
  SWord*       FindWord(const char* lpsName, INT16 nMask = WL_TYPE_DONTCARE);
  CDlpObject*  FindInstanceWord(const char *lpsInstanceName, const char* lpsClassName = NULL);
  SWord*       FindFactoryWord(const char *lpsName);
  SWord*       FindFieldPtr(const void* lpData, BOOL bRecursive);
  SWord*       FindOperator(const char* lpsName);
  char*        GetFQName(char* lpName, BOOL bForceArray = 0);
  CDlpObject*  Instantiate(const char* lpsClassName, const char* lpsInstanceName, BOOL bGlobal);

  /* Handling of class members (dlpi_memb.c) */
  public:
  INT16 SetField(SWord* lpWord,void*);
  INT16 ResetField(SWord* lpWord, BOOL bDestroying = 0);
  INT16 ResetAllFields(BOOL bInit = 0);
  INT16 FieldToString(char* lpsBuffer, size_t nBufferLen, SWord* lpWord);
  INT16 FieldToDouble(SWord* lpWord, FLOAT64* lpBuffer);
  INT16 FieldFromString(SWord* lpWord, const char* lpsBuffer);
  INT16 SetOption(SWord* lpWord);
  INT16 CopyAllOptions(CDlpObject* iSrc);

  /* Printing (dlpi_prt.c) */
  private:
  void    PrintMember(SWord* lpWord, INT16 nHow, BOOL bLast = 0);
  void    PrintAllMembers(INT16 nWordType, INT16 nHow);
  public:
  INT16   PrintField(const char* lpsFieldName, BOOL bInline = 0);
  void    PrintVersionInfo();
  void    PrintDictionary(INT16 nHow);

  /* Static functions (system information)(dlpi_stat.c) */
  public:
  static void*       GetStaticFieldPtr(const char* lpsFieldName);
  static void        SetFormexFunc(LPF_FORMEX FormExFunc, CDlpObject* iFormExInst);
  static void        GetFormexFunc(LPF_FORMEX* FormExFunc, CDlpObject** lpiFormExInst);
  static void        RegisterClass(const SWord* lpClassWord);
  static void        UnregisterAllClasses();
         INT16       LoadClassRegistry(BOOL bAutoInst);
  static CDlpObject* OfKind(const char* lpsClassName, CDlpObject* iInst);
  static CDlpObject* CreateInstanceOf(const char* lpsClassName, const char* lpsInstanceName);
  static BOOL        OfKindStr(const char* lpsClassName, const char* lpsBaseClassName);
         CDlpObject* FindInstance(const char* lpsInstanceName);
  static INT16       SetErrorLevel(INT16 nErrorLevel);
  static INT16       GetErrorLevel();
  static void        SetTraceError(const char* lpTraceError);
  static void        GetTraceError(char* lpBuffer, INT16 nBufferLen);
  static INT32       GetErrorCount();
  static void        SetLastError(INT16 nError, CDlpObject* iErrorInst);
  static void        SetErrorPos(const char* lpInFile, INT32 nInLine);
  static void        GetErrorPos(char* lpInFile, INT32* lpInLine);
  static INT16       Error(CDlpObject* iInst, const char* lpsFilename, INT32 nLine, INT16 nErrorID, ...);
  static void        ErrorLog();
  static CDlpObject* CheckInstancePtr(CDlpObject* iInst, UINT64 nSerialNum);
         CDlpObject* GetParent();
         CDlpObject* GetRoot();

  /* Fields */
  public:

#else  /* #ifdef __cplusplus */

typedef struct CDlpObject
{
  /* Pointers to C virtual member functions */
  INT16 (*AutoRegisterWords)(struct CDlpObject*                    );
  INT16 (*Reset            )(struct CDlpObject*, BOOL              );
  INT16 (*Init             )(struct CDlpObject*, BOOL              );
  INT16 (*Serialize        )(struct CDlpObject*, struct CDN3Stream*);
  INT16 (*SerializeXml     )(struct CDlpObject*, struct CXmlStream*);
  INT16 (*Deserialize      )(struct CDlpObject*, struct CDN3Stream*);
  INT16 (*DeserializeXml   )(struct CDlpObject*, struct CXmlStream*);
  INT16 (*Copy             )(struct CDlpObject*, struct CDlpObject*);
  INT16 (*ClassProc        )(struct CDlpObject*                    );
  INT16 (*GetInstanceInfo  )(struct CDlpObject*, SWord* lpClassWord);
  BOOL  (*IsKindOf         )(struct CDlpObject*, const char*       );
  void  (*Destructor       )(struct CDlpObject*                    );
  INT16 (*ResetAllOptions  )(struct CDlpObject*, BOOL              );

  /* Pointer to C base/derived instance */
  struct CDlpObject* m_lpBaseInstance;
  void*              m_lpDerivedInstance;


#endif /* #ifdef __cplusplus */

  char        m_lpClassName[L_NAMES];                                           /* dLabPro class name                */
  char        m_lpObsoleteName[L_NAMES];                                        /* Obsolete dLabPro class name (opt.)*/
  char        m_lpProjectName[L_NAMES];                                         /* Project code                      */
  char        m_lpInstanceName[L_SSTR];                                         /* Instance name                     */
  INT64       m_nClStyle;                                                       /* Class style (comb. of CS_XXX)     */
  INT64       m_nInStyle;                                                       /* Inst. style (comb. of IS_XXX)     */
  SVersion    m_version;                                                        /* Class version                     */
  hash_t*     m_lpDictionary;                                                   /* Word list                         */
  hash_t*     m_lpObsIds;                                                       /* List of obsolete identifiers      */
  hash_t*     m_lpOpDict;                                                       /* Operator list                     */
  INT16       m_nCheck;                                                         /* Message level                     */
  INT16       m_nRC;                                                            /* REMOVE TOGETHER WITH CItp         */
  UINT64      m_nSerialNum;                                                     /* Serial number of this object      */
  SWord*      m_lpContainer;                                                    /* Container dictionary entry        */
#ifdef __cplusplus
  CDlpObject* m_iAliasInst;                                                     /* ONLY to be used by CVar!          */
#else
  struct CDlpObject* m_iAliasInst;                                              /* ONLY to be used by CVar!          */
#endif
  const SMic* m_lpMic;                                                          /* Method invocation context         */
}

#ifndef __cplusplus
CDlpObject
#endif
;

/* C functions - Constructors and destructors (dlpinst.c) */
void        CDlpObject_Constructor(CDlpObject*, const char* lpsInstanceName, BOOL bCallVirtual);
void        CDlpObject_Destructor(CDlpObject*);

/* C functions - Virtual system member functions (dlpinst.c) */
INT16       CDlpObject_AutoRegisterWords(CDlpObject*);
INT16       CDlpObject_Reset(CDlpObject*, BOOL bResetMembers);
INT16       CDlpObject_Init(CDlpObject*, BOOL bCallVirtual);
INT16       CDlpObject_Serialize(CDlpObject*, struct CDN3Stream* lpiDst);
INT16       CDlpObject_Deserialize(CDlpObject*, struct CDN3Stream* lpiSrc);
INT16       CDlpObject_Copy(CDlpObject*, CDlpObject* __iSrc);
INT16       CDlpObject_CopySelective(CDlpObject*, CDlpObject* iSrc, INT16 nWhat);
INT16       CDlpObject_ClassProc(CDlpObject*);
INT16       CDlpObject_InstallProc(void* lpItp);
CDlpObject* CDlpObject_CreateInstance(const char* lpsName);
INT16       CDlpObject_GetClassInfo(SWord* lpClassWord);
INT16       CDlpObject_GetInstanceInfo(CDlpObject*, SWord* lpClassWord);
BOOL        CDlpObject_IsKindOf(CDlpObject*, const char* lpsClassName);
INT16       CDlpObject_ResetAllOptions(CDlpObject*, BOOL bInit);
INT16       CDlpObject_Check(CDlpObject*, INT32 nMode);

/* C functions - Dictionary functions (dlpi_dict.c) */
SWord*      CDlpObject_RegisterWord(CDlpObject*, const SWord* lpWord,...);
SWord*      CDlpObject_RegisterOperator(CDlpObject*, const SWord* lpWord);
INT16       CDlpObject_UnregisterWord(CDlpObject*, SWord* lpWord, INT16 bDeleteInstance);
INT16       CDlpObject_UnregisterAllWords(CDlpObject*);
INT16       CDlpObject_UnregisterOperator(CDlpObject*, SWord*);
INT16       CDlpObject_UnregisterAllOperators(CDlpObject*);
SWord*      CDlpObject_FindWordInternal(CDlpObject*, const char* lpsName, INT16 nMask);
SWord*      CDlpObject_FindWord(CDlpObject*, const char* lpsName, INT16 nMask);
CDlpObject* CDlpObject_FindInstanceWord(CDlpObject*, const char *lpsInstanceName, const char* lpClassName);
SWord*      CDlpObject_FindFactoryWord(CDlpObject*, const char *lpsName);
SWord*      CDlpObject_FindFieldPtr(CDlpObject*, const void* lpData, BOOL bRecursive);
SWord*      CDlpObject_FindOperator(CDlpObject*, const char* lpIdentifier);
char*       CDlpObject_GetFQName(CDlpObject*, char* lpsName, BOOL bForceArray);
CDlpObject* CDlpObject_CheckInstancePtr(CDlpObject*, UINT64);
CDlpObject* CDlpObject_GetParent(CDlpObject*);
CDlpObject* CDlpObject_GetRoot(CDlpObject*);
CDlpObject* CDlpObject_Instantiate(CDlpObject*, const char* lpsClassName, const char* lpsInstanceName, BOOL bGlobal);
int         CDlpObject_CompareWordsByName(const void* lpWord1, const void* lpWord2);

/* C functions - Handling of class members (dlpi_memb.c) */
INT16       CDlpObject_SetField(CDlpObject*, SWord* lpWord,void*);
INT16       CDlpObject_SetFieldNoWord(CDlpObject* , void *lpDst, INT32 nFlags, INT16 nType, const char *lpsTName, const char *lpsIName,...);
INT16       CDlpObject_ResetField(CDlpObject*, SWord* lpWord, BOOL bDestroying);
INT16       CDlpObject_ResetAllFields(CDlpObject*, BOOL bInit);
INT16       CDlpObject_FieldToString(CDlpObject*, char* lpsBuffer, size_t nBufferLen, SWord* lpWord);
INT16       CDlpObject_FieldToDouble(CDlpObject*, SWord* lpWord, FLOAT64* lpBuffer);
INT16       CDlpObject_FieldFromString(CDlpObject*, SWord* lpWord, const char* lpsBuffer);
INT16       CDlpObject_SetOption(CDlpObject*, SWord* lpWord);
INT16       CDlpObject_CopyAllWords(CDlpObject*, CDlpObject* iSrc, INT16 nWordType);
INT16       CDlpObject_CopyAllOptions(CDlpObject*, CDlpObject* iSrc);

/* C functions - Method invocation context functions (dlp_mic.c) */
const SMic* CDlpObject_MicSet(CDlpObject*, const SMic* lpMic);
const SMic* CDlpObject_MicGet(CDlpObject*);
StkItm*     CDlpObject_MicGetX(CDlpObject*, INT16 nArg, StkItm* lpSi);
BOOL        CDlpObject_MicGetB(CDlpObject*, INT16 nArg, INT16 nPos);
COMPLEX64   CDlpObject_MicGetN(CDlpObject*, INT16 nArg, INT16 nPos);
CDlpObject* CDlpObject_MicGetI(CDlpObject*, INT16 nArg, INT16 nPos);
char*       CDlpObject_MicGetS(CDlpObject*, INT16 nArg, INT16 nPos);
void        CDlpObject_MicPutX(CDlpObject*, StkItm* lpSi);
void        CDlpObject_MicPutB(CDlpObject*, BOOL bVal);
void        CDlpObject_MicPutN(CDlpObject*, COMPLEX64 nVal);
void        CDlpObject_MicPutI(CDlpObject*, CDlpObject* iVal);
void        CDlpObject_MicPutS(CDlpObject*, const char* lpsVal);
const char* CDlpObject_MicNextToken(CDlpObject*, BOOL bSameLine);
const char* CDlpObject_MicCmdLine(CDlpObject*);
void        CDlpObject_MicRefuseToken(CDlpObject*);
const char* CDlpObject_MicNextTokenDel(CDlpObject*);
SWord*      CDlpObject_MicFindWord(CDlpObject*, const char* lpsId);

/* C functions - Printing (dlpi_prt.c) */
INT16       CDlpObject_PrintField(CDlpObject*, const char* lpName, BOOL bInline);
void        CDlpObject_PrintMember(CDlpObject*, SWord* lpWord, INT16 nHow, BOOL bLast);
void        CDlpObject_PrintAllMembers(CDlpObject*, INT16 nWordType, INT16 nHow);
void        CDlpObject_PrintVersionInfo(CDlpObject*);
void        CDlpObject_PrintDictionary(CDlpObject*, INT16 nHow);

/* C functions - Static functions (system information)(dlpi_stat.c) */
void*       CDlpObject_GetStaticFieldPtr(const char* lpFieldIdentifier);
void        CDlpObject_SetFormexFunc(LPF_FORMEX FormExFunc, CDlpObject* iFormExInst);
void        CDlpObject_GetFormexFunc(LPF_FORMEX* lpFormExFunc, CDlpObject** lpiFormExInst);
void        CDlpObject_RegisterClass(const SWord* lpClassWord);
void        CDlpObject_UnregisterAllClasses();
void        CDlpObject_PrintClassRegistry();
INT16       CDlpObject_LoadClassRegistry(CDlpObject*, BOOL bAutoInst);
CDlpObject* CDlpObject_CreateInstanceOf(const char* lpsClassName, const char* lpsInstanceName);
CDlpObject* CDlpObject_OfKind(const char* lpsClassName, CDlpObject* iInst);
BOOL        CDlpObject_OfKindStr(const char* lpsClassName, const char* lpsBaseClassName);
CDlpObject* CDlpObject_FindInstance(CDlpObject*, const char* lpName);
INT16       CDlpObject_SetErrorLevel(INT16 nErrorLevel);
INT16       CDlpObject_GetErrorLevel();
void        CDlpObject_SetTraceError(const char* lpsTraceError);
void        CDlpObject_GetTraceError(char* lpsBuffer, INT16 nBufferLen);
INT32       CDlpObject_GetErrorCount();
void        CDlpObject_SetLastError(INT16 nError, CDlpObject* iErrorInst);
void        CDlpObject_SetErrorPos(const char* lpInFile, INT32 nInLine);
void        CDlpObject_GetErrorPos(char* lpInFile, INT32* lpInLine);
INT16       CDlpObject_Error(CDlpObject* iInst, const char* lpsFilename, INT32 nLine, INT16 nErrorID, ...);
void        CDlpObject_ErrorLog();
UINT64      CDlpObject_GetNextSerialNum();
INT16       CDlpObject_SerializeField(CDlpObject*, struct CDN3Stream* lpiDst, SWord* lpWord);
INT16       CDlpObject_DeserializeField(CDlpObject*, struct CDN3Stream* lpiDst, SWord* lpWord);
INT16       CDlpObject_CopyField(CDlpObject*, SWord* lpWord, CDlpObject* iSrc);

/* C functions - Common serialization / deserialization (dlpi_stm.c) */
INT16       CDlpObject_Save(CDlpObject*, const char* lpsFilename, INT16 nFormat);
INT16       CDlpObject_SaveBuffer(CDlpObject*, void **buf, size_t *si, INT16 nFormat);
INT16       CDlpObject_Restore(CDlpObject*, const char* lpsFilename, INT16 nFormat);
INT16       CDlpObject_RestoreBuffer(CDlpObject*, void *buf, size_t si);

/* C functions - XML serialization / deserialization (dlpi_xml.c) */
INT16       CDlpObject_SerializeXml(CDlpObject*, struct CXmlStream* lpiDst);
INT16       CDlpObject_DeserializeXml(CDlpObject*, struct CXmlStream* lpiSrc);
INT16       CDlpObject_SerializeFieldXml(CDlpObject*, struct CXmlStream* lpiDst, SWord* lpWord);
INT16       CDlpObject_DeserializeFieldXml(CDlpObject*, struct CXmlStream* lpiSrc, SWord* lpWord);

#endif /* ifndef __DLPOBJECT_H */

/* EOF */

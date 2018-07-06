cdef extern from "dlabpro_init.hpp":
    cdef void dlabpro_init()

dlabpro_init()

cdef extern from "dlp_base.h":
    cdef char dlp_xalloc_init(int flags)

cdef extern from "dlp_object.h":
    cdef cppclass CDlpObject:
        CDlpObject(char*,char)
        short ResetAllOptions(char)
        short SetOption(void*)
        void* FindWord(char*,short mask)
    cdef short CDlpObject_Save(CDlpObject*,char*,short)
    cdef short CDlpObject_Restore(CDlpObject*,char*,short)

cdef class PObject:
    cdef CDlpObject *optr
    cdef char _init
    def __cinit__(self,str name="",init=True):
        self._init=False
        if type(self) is PObject and init:
            self._init=True
            self.optr=new CDlpObject(name.encode(),1)
    def __dealloc__(self):
        if type(self) is PObject and self._init: del self.optr
    def RESETOPTIONS(self): return self.optr.ResetAllOptions(0)
    def SETOPTION(self,str name): return self.optr.SetOption(self.optr.FindWord(name.encode(),0x4))
    def Save(self,str filename,int fmt=0): return CDlpObject_Save(self.optr,filename.encode(),fmt)
    def Restore(self,str filename,int fmt=0): return CDlpObject_Restore(self.optr,filename.encode(),fmt)

cdef extern from "dlp_data.h":
    cdef cppclass CData(CDlpObject):
        CData(char *,char)
        short Print()
        short Status()
        short Select(CData*,int,int)
        short Delete(CData*,int,int)
        short Array(short,int,int)
        short AddNcomps(short,int)
        short InsertNcomps(short,int,int)

cdef extern from "dlabpro_numpy.hpp":
    cdef void numpy2data(object,CData*)
    cdef object data2numpy(CData*)

cdef class PData(PObject):
    cdef CData *dptr
    def __cinit__(self,str name="",init=True):
        if type(self) is PData and init:
            self._init=True
            self.optr=self.dptr=new CData(name.encode(),1)
    def __dealloc__(self):
        if type(self) is PData and self._init: del self.dptr
    def Print(self): return self.dptr.Print()
    def Status(self): return self.dptr.Status()
    def Select(self,PData src,int first,int count): return self.dptr.Select(src.dptr,first,count)
    def Delete(self,PData src,int first,int count): return self.dptr.Delete(src.dptr,first,count)
    def AddNcomps(self,int ctype,int count): return self.dptr.AddNcomps(ctype,count)
    def InsertNcomps(self,int ctype,int insertat,int count): return self.dptr.InsertNcomps(ctype,insertat,count)
    def Array(self,int ctype,int comps,int recs): return self.dptr.Array(ctype,comps,recs)
    def fromnumpy(self,object n): numpy2data(n,self.dptr)
    def tonumpy(self): return data2numpy(self.dptr)

cdef extern from "dlp_fst.h":
    cdef cppclass CFst(CDlpObject):
        CData *ud
        CData *sd
        CData *td
        CFst(char *,char)
        short Status()
        short Print()
        short Probs(int)
        short Sdp(CFst*,int,CData*)

cdef class PFst(PObject):
    cdef CFst *fptr
    cdef PData udptr
    cdef PData sdptr
    cdef PData tdptr
    def __cinit__(self,str name=""):
        if type(self) is PFst:
            self.fptr=self.optr=new CFst(name.encode(),1)
            self.udptr=PData("",init=False)
            self.sdptr=PData("",init=False)
            self.tdptr=PData("",init=False)
            self.udptr.optr=self.udptr.dptr=self.fptr.ud
            self.sdptr.optr=self.sdptr.dptr=self.fptr.sd
            self.tdptr.optr=self.tdptr.dptr=self.fptr.td
    def __dealloc__(self):
        if type(self) is PFst: del self.fptr
    def Status(self): return self.fptr.Status()
    def Print(self): return self.fptr.Print()
    def Probs(self,int unit): return self.fptr.Probs(unit)
    def Sdp(self,PFst src,int unit,PData weights): return self.fptr.Sdp(src.fptr,unit,weights.dptr)
    def ud(self): return self.udptr
    def sd(self): return self.sdptr
    def td(self): return self.tdptr

cdef extern from "dlp_gmm.h":
    cdef cppclass CGmm(CDlpObject):
        CGmm(char *,char)
        short Density(CData*,CData*,CData*)
        short Status()

cdef class PGmm(PObject):
    cdef CGmm *gptr
    def __cinit__(self,str name="",init=True):
        if type(self) is PGmm and init:
            self._init=True
            self.optr=self.gptr=new CGmm(name.encode(),1)
    def __dealloc__(self):
        if type(self) is PGmm and self._init: del self.gptr
    def Status(self): return self.gptr.Status()
    def Density(self,PData x,PData xmap,PData dens):
        return self.gptr.Density(x.dptr,xmap.dptr if not xmap is None else NULL,dens.dptr)

cdef extern from "dlp_hmm.h":
    cdef cppclass CHmm(CFst):
        CHmm(char *,char)
        CGmm *m_iGm
        short Setup(int,CData*)
        short Update(CData*,int,int,CData*,CData*,int)
        short SetupGmm(int)
        short Bwalpha(int,CData*,CData*)
        short Bwupdate(CData*,CData*,CData*,int)

cdef class PHmm(PFst):
    cdef CHmm *hptr
    def __cinit__(self,str name=""):
        if type(self) is PHmm:
            self.hptr=self.fptr=self.optr=new CHmm(name.encode(),1)
    def __dealloc__(self):
        if type(self) is PHmm: del self.hptr
    def Setup(self,int mfs,PData hmms): return self.hptr.Setup(mfs,hmms.dptr)
    def Update(self,PData src,int ictis,int icter,PData msf,PData lsf,int unit):
        return self.hptr.Update(src.dptr,ictis,icter,msf.dptr,lsf.dptr if not lsf is None else NULL,unit)
    def SetupGmm(self,int mindet): return self.hptr.SetupGmm(mindet)
    def Bwalpha(self,int unit,PData weights,PData alpha): return self.hptr.Bwalpha(unit,weights.dptr,alpha.dptr)
    def Bwupdate(self,PData alpha,PData msf,PData lsf,int unit):
        return self.hptr.Bwupdate(alpha.dptr,msf.dptr,lsf.dptr if not lsf is None else NULL,unit)
    def gm(self):
        ret=PGmm("",init=False)
        ret.optr=ret.gptr=self.hptr.m_iGm
        return ret

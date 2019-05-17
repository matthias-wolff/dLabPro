from libc.string cimport strncpy

cdef extern from "dlp_base.h":
    cdef void* __dlp_malloc(int,char*,int,char*,char*)
cdef void* dlp_malloc(int size):
    return __dlp_malloc(size,"dlabpro.pyx",0,"python",NULL)

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
        short Copy(CDlpObject*)
    cdef short CDlpObject_Save(CDlpObject*,char*,short)
    cdef short CDlpObject_Restore(CDlpObject*,char*,short)

cdef class PObject:
    cdef CDlpObject *optr
    cdef char _init
    def __cinit__(self,str name="object",init=True):
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
    def Copy(self,PObject src): return self.optr.Copy(src.optr)

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
        int GetNRecs()
        int GetNComps()
        double Dfetch(int,int)
        short Dstore(double,int,int)
    cdef short CData_ChecksumInt(CData*,char*,int)

cdef extern from "dlabpro_numpy.hpp":
    cdef void numpy2data(object,CData*)
    cdef object data2numpy(CData*)

cdef class PData(PObject):
    cdef CData *dptr
    def __cinit__(self,str name="data",init=True):
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
    def fromnumpy(self,object n):
        if not n.flags['C_CONTIGUOUS']: n=n.copy(order='C')
        numpy2data(n,self.dptr)
    def tonumpy(self): return data2numpy(self.dptr)
    def nrec(self): return self.dptr.GetNRecs()
    def dim(self): return self.dptr.GetNComps()
    def Dfetch(self,int rec,int comp): return self.dptr.Dfetch(rec,comp)
    def Dstore(self,float val,int rec,int comp): return self.dptr.Dstore(val,rec,comp)
    def hash(self): return CData_ChecksumInt(self.dptr,'CRC-32'.encode(),-1)

cdef extern from "dlp_fst.h":
    cdef cppclass CFst(CDlpObject):
        CFst(char *,char)
        CData *ud
        CData *sd
        CData *td
        CData *os
        short Status()
        short Print()
        short Probs(int)
        short Sdp(CFst*,int,CData*)
        short Addunit(char*)
        short Determinize(CFst*,int)
        short Minimize(CFst*,int)
        short Compose(CFst*,CFst*,int,int)
        short Close(CFst*,int)
        short Union(CFst*)
        short AddtransEx(int,int,int,int,int,double)

cdef class PFst(PObject):
    cdef CFst *fptr
    cdef PData udptr
    cdef PData sdptr
    cdef PData tdptr
    cdef PData osptr
    def __cinit__(self,str name="fst"):
        if type(self) is PFst:
            self.fptr=self.optr=new CFst(name.encode(),1)
            self.init_fst_fields()
    def __dealloc__(self):
        if type(self) is PFst: del self.fptr
    def init_fst_fields(self):
        self.udptr=PData("",init=False)
        self.sdptr=PData("",init=False)
        self.tdptr=PData("",init=False)
        self.osptr=PData("",init=False)
        self.udptr.optr=self.udptr.dptr=self.fptr.ud
        self.sdptr.optr=self.sdptr.dptr=self.fptr.sd
        self.tdptr.optr=self.tdptr.dptr=self.fptr.td
        self.osptr.optr=self.osptr.dptr=self.fptr.os
    def ud(self): return self.udptr
    def sd(self): return self.sdptr
    def td(self): return self.tdptr
    def os(self): return self.osptr
    def Status(self): return self.fptr.Status()
    def Print(self): return self.fptr.Print()
    def Probs(self,int unit): return self.fptr.Probs(unit)
    def Sdp(self,PFst src,int unit,PData weights): return self.fptr.Sdp(src.fptr,unit,weights.dptr)
    def Addunit(self,str name): return self.fptr.Addunit(name.encode())
    def Minimize(self,PFst src,int unit): return self.fptr.Minimize(src.fptr,unit)
    def Determinize(self,PFst src,int unit): return self.fptr.Determinize(src.fptr,unit)
    def Compose(self,PFst src1,PFst src2,int unit1,int unit2): return self.fptr.Compose(src1.fptr,src2.fptr,unit1,unit2)
    def Close(self,PFst src,int unit): return self.fptr.Close(src.fptr,unit)
    def Union(self,PFst src): return self.fptr.Union(src.fptr)
    def AddtransEx(self,int unit,int ini,int ter,int tis,int tos,float w): return self.fptr.AddtransEx(unit,ini,ter,tis,tos,w)
    def LoopsEx(self,int unit,int tis,int tos,float w):
        xs=int(self.ud().Dfetch(unit,1))
        fs=int(self.ud().Dfetch(unit,3))
        for si in range(fs,fs+xs): self.AddtransEx(unit,si,si,tis,tos,w)
    def compose_fast(self,PFst src1,PFst src2,int unit1=0,int unit2=0):
        src1.AddtransEx(unit1,0,0,-1,-1,0)
        src2.LoopsEx(unit2,-1,-1,0)
        self.SETOPTION('/noeps')
        self.SETOPTION('/noint')
        self.Compose(src1,src2,unit1,unit2)
    def lazy_minimize(self,PFst src=None,int unit=0):
        if src is None: src=self
        self.SETOPTION('/lazy')
        self.Minimize(src,unit)


cdef extern from "dlp_gmm.h":
    cdef cppclass CGmm(CDlpObject):
        CGmm(char *,char)
        short Status()
        int GetNGauss()
        short Density(CData*,CData*,CData*)

cdef class PGmm(PObject):
    cdef CGmm *gptr
    def __cinit__(self,str name="gmm",init=True):
        if type(self) is PGmm and init:
            self._init=True
            self.optr=self.gptr=new CGmm(name.encode(),1)
    def __dealloc__(self):
        if type(self) is PGmm and self._init: del self.gptr
    def Status(self): return self.gptr.Status()
    def GetNGauss(self): return self.gptr.GetNGauss()
    def Density(self,PData x,PData xmap,PData dens):
        return self.gptr.Density(x.dptr,xmap.dptr if not xmap is None else NULL,dens.dptr)

cdef extern from "dlp_hmm.h":
    cdef cppclass CHmm(CFst):
        CHmm(char *,char)
        CGmm *m_iGm
        short Setup(int,CData*)
        short Update(CData*,int,int,CData*,CData*,int)
        short SetupGmm(double)
        short Bwalpha(int,CData*,CData*)
        short Bwupdate(CData*,CData*,CData*,int)
        short Split(double,int,CData*)
        short GmmMix()

cdef class PHmm(PFst):
    cdef CHmm *hptr
    def __cinit__(self,str name="hmm"):
        if type(self) is PHmm:
            self.hptr=self.fptr=self.optr=new CHmm(name.encode(),1)
            self.init_fst_fields()
    def __dealloc__(self):
        if type(self) is PHmm: del self.hptr
    def Setup(self,int mfs,PData hmms): return self.hptr.Setup(mfs,hmms.dptr)
    def Update(self,PData src,int ictis,int icter,PData msf,PData lsf,int unit):
        return self.hptr.Update(src.dptr,ictis,icter,msf.dptr,lsf.dptr if not lsf is None else NULL,unit)
    def SetupGmm(self,double mindet): return self.hptr.SetupGmm(mindet)
    def Bwalpha(self,int unit,PData weights,PData alpha): return self.hptr.Bwalpha(unit,weights.dptr,alpha.dptr)
    def Bwupdate(self,PData alpha,PData msf,PData lsf,int unit):
        return self.hptr.Bwupdate(alpha.dptr,msf.dptr,lsf.dptr if not lsf is None else NULL,unit)
    def Split(self,double minrc=-1,int maxcnt=0,PData map=None):
        return self.hptr.Split(minrc,maxcnt,map.dptr if not map is None else NULL)
    def GmmMix(self): return self.hptr.GmmMix()
    def gm(self):
        ret=PGmm("",init=False)
        ret.optr=ret.gptr=self.hptr.m_iGm
        return ret

cdef extern from "dlp_fstsearch.h":
    cdef cppclass CFstsearch(CDlpObject):
        CFstsearch(char *,char)
        char* m_lpsBt
        short Status()
        short Search(CFst*,long,CData*,CFst*)

cdef class PFstsearch(PObject):
    cdef CFstsearch *fptr
    def __cinit__(self,str name="fstsearch"):
        if type(self) is PFstsearch: self.fptr=self.optr=new CFstsearch(name.encode(),1)
    def __dealloc__(self):
        if type(self) is PFstsearch: del self.fptr
    def Status(self): return self.fptr.Status()
    def Search(self,PFst src,int unit,PData weights,PFst dst):
        return self.fptr.Search(src.fptr,unit,weights.dptr if not weights is None else NULL,dst.fptr)
    def backtrack(self,val=None):
        if not val is None: 
            self.fptr.m_lpsBt=<char*>dlp_malloc(32)
            strncpy(self.fptr.m_lpsBt,val.encode(),32)
        return self.fptr.m_lpsBt.decode('utf-8')

cdef extern from "dlp_file.h":
    cdef cppclass CDlpFile(CDlpObject):
        CDlpFile(char *,char)
        short Export(char*,char*,CDlpObject*)
        short Import(char*,char*,CDlpObject*)

cdef class PFile(PObject):
    cdef CDlpFile *fptr
    def __cinit__(self,str name="file"):
        if type(self) is PFile: self.fptr=self.optr=new CDlpFile(name.encode(),1)
    def __dealloc__(self):
        if type(self) is PFile: del self.fptr
    def Export(self,str filename,str filter,PObject inst):
        return self.fptr.Export(filename.encode(),filter.encode(),inst.optr)
    def Import(self,str filename,str filter,PObject inst):
        return self.fptr.Import(filename.encode(),filter.encode(),inst.optr)

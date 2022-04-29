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
    cdef CDlpObject* CDlpObject_FindInstanceWord(CDlpObject*,char*,char*)
    cdef CDlpObject* CDlpObject_Instantiate(CDlpObject*,char*,char*,char)

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
    def Save(self,str filename,int fmt=0,zip=False):
        if zip: fmt=fmt|4
        return CDlpObject_Save(self.optr,filename.encode(),fmt)
    def Restore(self,str filename,int fmt=0): return CDlpObject_Restore(self.optr,filename.encode(),fmt)
    def Copy(self,PObject src): return self.optr.Copy(src.optr)
    def FindDataWord(self,str name):
        obj=CDlpObject_FindInstanceWord(self.optr,name.encode(),b'data')
        if obj==NULL: return None
        dat=PData("")
        dat.optr.Copy(obj)
        return dat
    def FindFstWord(self,str name):
        obj=CDlpObject_FindInstanceWord(self.optr,name.encode(),b'fst')
        if obj==NULL: return None
        dat=PFst("")
        dat.optr.Copy(obj)
        return dat
    def AddDataWord(self,str name,PData dat=None):
        obj=CDlpObject_Instantiate(self.optr,b'data',name.encode(),1)
        if obj!=NULL and not dat is None: obj.Copy(dat.optr)
    def AddFstWord(self,str name,PFst dat=None):
        obj=CDlpObject_Instantiate(self.optr,b'fst',name.encode(),1)
        if obj!=NULL and not dat is None: obj.Copy(dat.optr)

cdef extern from "dlp_data.h":
    cdef cppclass CData(CDlpObject):
        CData(char *,char)
        short Print()
        short Status()
        short Select(CData*,int,int)
        short Delete(CData*,int,int)
        short Array(short,int,int)
        short Reallocate(int)
        short AddComp(char*,short)
        short AddNcomps(short,int)
        short InsertComp(char*,short,int)
        short InsertNcomps(short,int,int)
        int FindComp(char*)
        int GetNRecs()
        int GetNComps()
        double Dfetch(int,int)
        short Dstore(double,int,int)
        char Xstore(CData*,int,int,int)
        short Join(CData*)
        short GetCompType(int)
        short Tconvert(CData*,short)
        short Quantize(CData*)
        short Dequantize(CData*)
        short Rindex(char*,int)
        const char* GetCname(int)
        short SetCname(int,char*)
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
    def AddComp(self,str name,int ctype): return self.dptr.AddComp(name.encode(),ctype)
    def AddNcomps(self,int ctype,int count): return self.dptr.AddNcomps(ctype,count)
    def InsertComp(self,str name,int ctype,int insertat): return self.dptr.InsertComp(name.encode(),ctype,insertat)
    def InsertNcomps(self,int ctype,int insertat,int count): return self.dptr.InsertNcomps(ctype,insertat,count)
    def FindComp(self,str name): return self.dptr.FindComp(name.encode())
    def Array(self,int ctype,int comps,int recs): return self.dptr.Array(ctype,comps,recs)
    def Reallocate(self,int nrecs): return self.dptr.Reallocate(nrecs)
    def fromnumpy(self,object n):
        import copy
        if not n.flags['C_CONTIGUOUS']: n=n.copy(order='C')
        if not n.flags['OWNDATA']: n=copy.deepcopy(n)
        numpy2data(n,self.dptr)
        import pdb
        if self.dim()==0: pdb.set_trace()
    def tonumpy(self): return data2numpy(self.dptr)
    def nrec(self): return self.dptr.GetNRecs()
    def dim(self): return self.dptr.GetNComps()
    def Dfetch(self,int rec,int comp): return self.dptr.Dfetch(rec,comp)
    def Dstore(self,float val,int rec,int comp): return self.dptr.Dstore(val,rec,comp)
    def Xstore(self,PData src,int first,int count,int pos): return self.dptr.Xstore(src.dptr,first,count,pos)
    def Join(self,PData x): return self.dptr.Join(x.dptr)
    def hash(self): return CData_ChecksumInt(self.dptr,'CRC-32'.encode(),-1)
    def GetCompType(self,comp): return self.dptr.GetCompType(comp)
    def Tconvert(self,PData src,short ctype): return self.dptr.Tconvert(src.dptr,ctype)
    def Quantize(self,PData src): return self.dptr.Quantize(src.dptr)
    def Dequantize(self,PData src): return self.dptr.Dequantize(src.dptr)
    def Rindex(self,str cname,int ic): return self.dptr.Rindex(cname.encode(),ic)
    def GetCname(self,int ic): return self.dptr.GetCname(ic).decode('utf-8')
    def SetCname(self,int ic,str name): return self.dptr.SetCname(ic,name.encode())
    @classmethod
    def newfromnumpy(cls,object n):
        self=cls()
        self.fromnumpy(n)
        return self

cdef extern from "dlp_fst.h":
    cdef cppclass CFst(CDlpObject):
        CFst(char *,char)
        CData *ud
        CData *sd
        CData *td
        CData *os
        CData *isx
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
        short Cat(CFst*)
        short AddtransEx(int,int,int,int,int,double)
        short CopyUi(CFst*,CData*,int)

cdef class PFst(PObject):
    cdef CFst *fptr
    cdef PData udptr
    cdef PData sdptr
    cdef PData tdptr
    cdef PData osptr
    cdef PData isptr
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
        self.isptr=PData("",init=False)
        self.udptr.optr=self.udptr.dptr=self.fptr.ud
        self.sdptr.optr=self.sdptr.dptr=self.fptr.sd
        self.tdptr.optr=self.tdptr.dptr=self.fptr.td
        self.osptr.optr=self.osptr.dptr=self.fptr.os
        self.isptr.optr=self.isptr.dptr=self.fptr.isx
    def ud(self): return self.udptr
    def sd(self): return self.sdptr
    def td(self): return self.tdptr
    def os(self): return self.osptr
    def is_(self): return self.isptr
    def stk(self): return self.FindDataWord('stk')
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
    def Cat(self,PFst src): return self.fptr.Cat(src.fptr)
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
    def CopyUi(self,PFst src,PData index,int par): return self.fptr.CopyUi(src.fptr,index.dptr,par)


cdef extern from "dlp_vmap.h":
    cdef cppclass CVmap(CDlpObject):
        CData* m_idTmx
        CData* m_idWeakTmx
        CVmap(char *,char)
        short Status()
        short Setup(CData*,char*,char*,float)
        short Map(CData*,CData*)

cdef class PVmap(PObject):
    cdef CVmap *vptr
    cdef PData tmxptr
    cdef PData weaktmxptr
    def __cinit__(self,str name="vmap",init=True):
        if type(self) is PVmap and init:
            self._init=True
            self.optr=self.vptr=new CVmap(name.encode(),1)
            self.init_vmap_fields()
    def init_vmap_fields(self):
        self.tmxptr=PData("",init=False)
        self.weaktmxptr=PData("",init=False)
        self.tmxptr.optr=self.tmxptr.dptr=self.vptr.m_idTmx
        self.weaktmxptr.optr=self.weaktmxptr.dptr=self.vptr.m_idWeakTmx
    def __dealloc__(self):
        if type(self) is PVmap and self._init: del self.vptr
    def tmx(self): return self.tmxptr
    def weaktmx(self): return self.weaktmxptr
    def Status(self): return self.vptr.Status()
    def Setup(self,PData tmx,str aop,str wop,float zero):
        import sys
        if zero==float('inf'): zero=sys.float_info.max
        return self.vptr.Setup(tmx.dptr,aop.encode(),wop.encode(),zero)
    def Map(self,PData src,PData dst): return self.vptr.Map(src.dptr,dst.dptr)

cdef extern from "dlp_gmm.h":
    cdef cppclass CGmm(CDlpObject):
        CData *m_idMean
        CData *m_idIvar
        CData *m_idIcov
        CData *m_idCdet
        CVmap *m_iMmap;
        double m_nDceil
        CGmm(char *,char)
        short Status()
        int GetNGauss()
        short Density(CData*,CData*,CData*)
        short Extract(CData*,CData*)
        short Setup(CData*,CData*,CVmap*)

cdef class PGmm(PObject):
    cdef CGmm *gptr
    cdef PData meanptr
    cdef PData ivarptr
    cdef PData icovptr
    cdef PData cdetptr
    def __cinit__(self,str name="gmm",init=True):
        if type(self) is PGmm and init:
            self._init=True
            self.optr=self.gptr=new CGmm(name.encode(),1)
            self.init_gmm_fields()
    def init_gmm_fields(self):
        self.meanptr=PData("",init=False)
        self.ivarptr=PData("",init=False)
        self.icovptr=PData("",init=False)
        self.cdetptr=PData("",init=False)
        self.meanptr.optr=self.meanptr.dptr=self.gptr.m_idMean
        self.ivarptr.optr=self.ivarptr.dptr=self.gptr.m_idIvar
        self.icovptr.optr=self.icovptr.dptr=self.gptr.m_idIcov
        self.cdetptr.optr=self.cdetptr.dptr=self.gptr.m_idCdet
    def __dealloc__(self):
        if type(self) is PGmm and self._init: del self.gptr
    def mean(self): return self.meanptr
    def ivar(self): return self.ivarptr
    def icov(self): return self.icovptr
    def cdet(self): return self.cdetptr
    def mmap(self):
        if self.gptr.m_iMmap==NULL: return None
        ret=PVmap("",init=False)
        ret.optr=ret.vptr=self.gptr.m_iMmap
        ret.init_vmap_fields()
        return ret
    def Status(self): return self.gptr.Status()
    def GetNGauss(self): return self.gptr.GetNGauss()
    def Density(self,PData x,PData xmap,PData dens):
        return self.gptr.Density(x.dptr,xmap.dptr if not xmap is None else NULL,dens.dptr)
    def Extract(self,PData mean,PData icov): return self.gptr.Extract(mean.dptr,icov.dptr)
    def Setup(self,PData mean,PData cov,PVmap mmap=None):
        return self.gptr.Setup(mean.dptr,cov.dptr,mmap.vptr if not mmap is None else NULL)
    def setdceil(self,dceil): self.gptr.m_nDceil=dceil

cdef extern from "dlp_statistics.h":
    cdef cppclass CStatistics(CDlpObject):
        CStatistics(char *,char)
        short Status()
        short Setup(int,int,int,CData*,int)
        short Update(CData*,int,CData*)
        short Mean(CData*)
        short Cov(CData*)
        short Var(CData*)
        short Freq(CData*)

cdef class PStatistics(PObject):
    cdef CStatistics *sptr
    def __cinit__(self,str name="hmm"):
        if type(self) is PStatistics:
            self._init=True
            self.sptr=self.optr=new CStatistics(name.encode(),1)
    def __dealloc__(self):
        if type(self) is PStatistics and self._init: del self.sptr
    def Status(self): return self.sptr.Status()
    def Setup(self,int order,int dim,int cls,PData ltb,int icltb): return self.sptr.Setup(order,dim,cls,ltb.dptr if not ltb is None else NULL,icltb)
    def Update(self,PData vec,int iclab,PData w): return self.sptr.Update(vec.dptr,iclab,w.dptr if not w is None else NULL)
    def Mean(self,PData dst): return self.sptr.Mean(dst.dptr)
    def Var(self,PData dst): return self.sptr.Var(dst.dptr)
    def Cov(self,PData dst): return self.sptr.Cov(dst.dptr)
    def Freq(self,PData dst): return self.sptr.Freq(dst.dptr)

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
        short CopyFst(CFst*)

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
    def CopyFst(self,PFst src): return self.hptr.CopyFst(src.fptr)
    def gm(self):
        if self.hptr.m_iGm==NULL: return None
        ret=PGmm("",init=False)
        ret.optr=ret.gptr=self.hptr.m_iGm
        ret.init_gmm_fields()
        return ret
    def addgm(self): self.hptr.m_iGm=new CGmm(b'gm',1)

cdef extern from "dlp_fstsearch.h":
    cdef cppclass CFstsearch(CDlpObject):
        CFstsearch(char *,char)
        char* m_lpsBt
        double m_nTpPrnw
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
    def prnw(self,val=None):
        if not val is None: self.fptr.m_nTpPrnw=val
        return self.fptr.m_nTpPrnw

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

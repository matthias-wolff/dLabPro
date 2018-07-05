cdef extern from "dlabpro_init.hpp":
    cdef void dlabpro_init()

dlabpro_init()

cdef extern from "dlp_base.h":
    cdef char dlp_xalloc_init(int flags)

cdef extern from "dlp_object.h":
    cdef cppclass CDlpObject:
        CDlpObject(char *,char)
    cdef short CDlpObject_Save(CDlpObject*,char*,short)
    cdef short CDlpObject_Restore(CDlpObject*,char*,short)

cdef class PObject:
    cdef CDlpObject *optr
    def __cinit__(self,str name=""):
        if type(self) is PObject: self.optr=new CDlpObject(name.encode(),1)
    def __dealloc__(self):
        if type(self) is PObject: del self.optr
    def Save(self,str filename,int fmt): return CDlpObject_Save(self.optr,filename.encode(),fmt)
    def Restore(self,str filename,int fmt): return CDlpObject_Restore(self.optr,filename.encode(),fmt)

cdef extern from "dlp_data.h":
    cdef cppclass CData(CDlpObject):
        CData(char *,char)
        short Print()
        short Status()
        short Array(short,int,int)
        short AddNcomps(short,int)
        short InsertNcomps(short,int,int)

cdef class PData(PObject):
    cdef CData *dptr
    def __cinit__(self,str name=""):
        if type(self) is PData: self.optr=self.dptr=new CData(name.encode(),1)
    def __dealloc__(self):
        if type(self) is PData: del self.dptr
    def Print(self): return self.dptr.Print()
    def Status(self): return self.dptr.Status()
    def AddNcomps(self,int ctype,int count): return self.dptr.AddNcomps(ctype,count)
    def InsertNcomps(self,int ctype,int insertat,int count): return self.dptr.InsertNcomps(ctype,insertat,count)
    def Array(self,int ctype,int comps,int recs): return self.dptr.Array(ctype,comps,recs)

cdef extern from "dlp_fst.h":
    cdef cppclass CFst(CDlpObject):
        CFst(char *,char)
        CData *ud

cdef class PFst(PObject):
    cdef CFst *fptr
    def __cinit__(self,str name=""):
        if type(self) is PFst: self.fptr=self.optr=new CFst(name.encode(),1)
    def __dealloc__(self):
        if type(self) is PFst: del self.fptr
    def ud(self):
        ret=PData("")
        ret.dptr=self.fptr.ud
        return ret

cdef extern from "dlp_hmm.h":
    cdef cppclass CHmm(CFst):
        CHmm(char *,char)
        short Status()
        short Setup(int,CData*)

cdef class PHmm(PFst):
    cdef CHmm *hptr
    def __cinit__(self,str name=""):
        if type(self) is PHmm: self.hptr=self.fptr=self.optr=new CHmm(name.encode(),1)
    def __dealloc__(self):
        if type(self) is PHmm: del self.hptr
    def Status(self): return self.hptr.Status()
    def Setup(self,int mfs,PData hmms): return self.hptr.Setup(mfs,hmms.dptr)


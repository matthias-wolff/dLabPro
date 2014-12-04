/* dLabPro base library
 * - Basic arithmetic functions
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

#include "dlp_kernel.h"
#include "dlp_base.h"
#include "dlp_math.h"

/* Static variables */
static FLOAT64*   __hbuff  = NULL;
static COMPLEX64* __hbuffC  = NULL;
static INT64      __nhbuff = 0;

/**
 * Cleanup static pointers allocated by arith.c
 */
void dlp_arith_cleanup()
{
	dlp_free(__hbuff);
	__nhbuff = 0;
}

#ifdef __OPTIMIZE_LSADD

#define DLP_LSADD_DIFF(nParam1,nParam2)      (MIN(nParam1,nParam2)-MAX(nParam1,nParam2))
#define DLP_LSADD_ERRORLIN(nParam1,nParam2)  ((DLP_LSADD_DIFF(nParam1,nParam2)<DLP_LSADD_LIN_MIN)?0.0:dlp_lsadd_errorlin(DLP_LSADD_DIFF(nParam1,nParam2)))
#define DLP_LSADD_LIN_MIN    -10.0
#define DLP_LSADD_LIN_NUM    128
#define DLP_LSADD_LIN_INC    (DLP_LSADD_LIN_MIN/DLP_LSADD_LIN_NUM)
const FLOAT64 dlp_lsadd_lin_ybegin[]={
-0.693147180559945286226763982995L,
-0.654847426066446636205853337742L,
-0.618070839007268113540760623437L,
-0.582810476884710659817301348085L,
-0.549054862270668131429829372792L,
-0.516788120398108619113486383867L,
-0.485990166880682250560852253329L,
-0.456636940907461486016671869947L,
-0.428700678276518643006198772127L,
-0.402150217876254489901555189135L,
-0.376951334716886410358682724109L,
-0.353067092361850576320847494571L,
-0.330458207602220721543773152007L,
-0.309083420436873923531351238125L,
-0.288899862837948273597277193403L,
-0.269863420359272576654063868773L,
-0.251929081345372951616923273832L,
-0.235051269279768271225350417808L,
-0.219184154634490951929493007810L,
-0.204281943412441269192214576833L,
-0.190299140379555919988519008257L,
-0.177190785739814005150805087396L,
-0.164912664693908139756928221686L,
-0.153421489929056908962934357987L,
-0.142675057605601590671184908388L,
-0.132632377833224068819362173599L,
-0.123253780967468612450588238971L,
-0.114501001310482880546359751861L,
-0.106337239975603584096752740606L,
-0.098727208782160749689360557113L,
-0.091637157094043963923013507156L,
-0.085034883512789219417449260163L,
-0.078889734292549557048701558415L,
-0.073172590269181478350724034954L,
-0.067855843996778214544107754591L,
-0.062913368669467215066326559736L,
-0.058320480280248887527960732768L,
-0.054053894337221936039483694003L,
-0.050091678324936057820071511060L,
-0.046413200968103404675524359391L,
-0.042999079229087848463830567880L,
-0.039831123851266039825880227454L,
-0.036892284148855897241148227295L,
-0.034166592640870373231987144891L,
-0.031639110032915036818668141905L,
-0.029295870965678504072027266147L,
-0.027123830872991468859956043502L,
-0.025110814224961596624607906847L,
-0.023245464372425105048503013450L,
-0.021517195157270106037339374438L,
-0.019916144408496851136991523390L,
-0.018433129405544586981724464181L,
-0.017059604357851830547243565661L,
-0.015787619922200867017547665228L,
-0.014609784756570058997238170662L,
-0.013519229090428418887936956594L,
-0.012509570276173371736194717130L,
-0.011574880274240759506221287722L,
-0.010709655014929037389337374009L,
-0.009908785572759312015866939305L,
-0.009167531083942194805591618945L,
-0.008481493333893905250953082486L,
-0.007846592939527522944653803449L,
-0.007259047049955590254211035273L,
-0.006715348489117966938877213323L,
-0.006212246264487618699345894413L,
-0.005746727367274213731096477176L,
-0.005315999791303611249349003742L,
-0.004917476699886012478368790823L,
-0.004548761672412979586987979985L,
-0.004207634965057618914308079638L,
-0.003892040722719800494716624328L,
-0.003600075082226413935443387615L,
-0.003329975109689551487912595462L,
-0.003080108517836825307578685340L,
-0.002848964111996527340253670957L,
-0.002635142916257974480404024931L,
-0.002437349934075071743178053651L,
-0.002254386500262519927445525880L,
-0.002085143183909898400435478649L,
-0.001928593204219393279774674710L,
-0.001783786323642485953377967434L,
-0.001649843184953359498043368170L,
-0.001525950061044297314041817692L,
-0.001411353988272717090504682069L,
-0.001305358256112017396877167386L,
-0.001207318227684914518221814816L,
-0.001116637467472827993653083922L,
-0.001032764154109798741473791317L,
-0.000955187757691891570210884765L,
-0.000883435962445536910291055133L,
-0.000817071816948827431802138133L,
-0.000755691095333504070027941246L,
-0.000698919854074298040925994346L,
-0.000646412170056661857832358553L,
-0.000597848046637691863010311000L,
-0.000552931475360745553621066861L,
-0.000511388641875646699809510043L,
-0.000472966265432947442944044969L,
-0.000437430062099316240269847311L,
-0.000404563322548251490598514302L,
-0.000374165595947081430338448937L,
-0.000346051472082704143166248523L,
-0.000320049454437128699704068202L,
-0.000296000917462524882218383171L,
-0.000273759141798378641090294661L,
-0.000253188421634527099576827958L,
-0.000234163238847689525023304524L,
-0.000216567498943157841747100822L,
-0.000200293824186990751740414241L,
-0.000185242899671879325974171993L,
-0.000171322868359380966725782902L,
-0.000158448771446150773415523227L,
-0.000146542030665979271285628838L,
-0.000135529969395550549480372604L,
-0.000125345369659076827069735249L,
-0.000115926062352737733736167403L,
-0.000107214548195867423853802625L,
-0.000099157647116162331507940864L,
-0.000091706173934580950376019293L,
-0.000084814638383289757492949912L,
-0.000078440967633615962112392239L,
-0.000072546249645767538552498799L,
-0.000067094495781360821728446509L,
-0.000062052421234639327358127703L,
-0.000057389241945524286487266552L,
-0.000053076486758392766207755031L,
-0.000049087823686484842596825656L,
};
const FLOAT64 dlp_lsadd_lin_anstieg[]={
-0.490236857516782698063195766736L,
-0.470740314357485090113186743110L,
-0.451332635168735396558048478255L,
-0.432071867059744374461871530002L,
-0.413014295968761768751420504486L,
-0.394213805023057528575947117133L,
-0.375721292457225797267739153540L,
-0.357584161676068379431825405845L,
-0.339845893123381148637207616048L,
-0.322545704439911429250997798590L,
-0.305718302144458664582060691828L,
-0.289393724923262152248781831076L,
-0.273597275716438992354540005181L,
-0.258349537266248319156147772446L,
-0.243666463727048920873130555265L,
-0.229559539377915206026514738369L,
-0.216035994439739920114362803361L,
-0.203099067459549698089205094220L,
-0.190748303642235939037163916510L,
-0.178979878820932469807303277776L,
-0.167786939388696515473853310141L,
-0.157159949387595088143854127338L,
-0.147087036990095759714236578475L,
-0.137554333740228074134392954875L,
-0.128546301086432279703331005294L,
-0.120046039881669841520306363236L,
-0.112035579609417373925239758137L,
-0.104496145086454994554969744058L,
-0.097408399276068285965735071841L,
-0.090752661607894857809242239455L,
-0.084509101840060732446779923066L,
-0.078657910019067675544413020816L,
-0.073179443499111401782997177179L,
-0.068054352286761779500245950203L,
-0.063263684189580796091156855709L,
-0.058788971381994591103303804402L,
-0.054612300070744981828063657758L,
-0.050716364957259243984033503239L,
-0.047084510167457960250203541364L,
-0.043700758259399119509680531337L,
-0.040549828836119149177985576671L,
-0.037617148190849825084569602041L,
-0.034888851302214710092819416332L,
-0.032351777381828303314925676659L,
-0.029993460060627620544781990475L,
-0.027802113186394049326732869076L,
-0.025766613094782363224677368407L,
-0.023876478112467090786363854704L,
-0.022121845953983986649005188951L,
-0.020493449584297662724452493421L,
-0.018982592037788979799639577095L,
-0.017581120610467283055244891443L,
-0.016281400776332333180107525550L,
-0.015076290120074341966072140053L,
-0.013959112526612993746000235262L,
-0.012923632822464604236190055531L,
-0.011964032024737437237549286806L,
-0.011074883319190043096114095533L,
-0.010251128859772484086532173819L,
-0.009488057456859100291524100612L,
-0.008781283200618106299373266665L,
-0.008126725047889693520630771673L,
-0.007520587386520738611139780261L,
-0.006959341578721578436272920953L,
-0.006439708475268457119056186855L,
-0.005958641884331583420120193040L,
-0.005513312972423712113312355143L,
-0.005101095570145264268546725361L,
-0.004719552351654820836202031131L,
-0.004366421854148616957247419634L,
-0.004039606301924075423825932774L,
-0.003737160198315347871961256132L,
-0.003457279648471839328394139557L,
-0.003198292375714895108274049562L,
-0.002958648394755813895024010307L,
-0.002736911305453476606075469135L,
-0.002531750171941155036492432373L,
-0.002341931952800663241376355472L,
-0.002166314449313555458992430758L,
-0.002003839740038465717930638021L,
-0.001853528071384413691141679337L,
-0.001714472175220818628282870577L,
-0.001585831986035995911851759210L,
-0.001466829731476226817907249078L,
-0.001356745371656955991696014152L,
-0.001254912363866916803420425985L,
-0.001160713730714707557847842345L,
-0.001073578411046774471263032247L,
-0.000992977874149211705429030062L,
-0.000918422979153339690341906199L,
-0.000849461062357881368026224500L,
-0.000785673236676139052393763595L,
-0.000726671888117837215873007217L,
-0.000672098355425743186966625053L,
-0.000621620779762815977090295583L,
-0.000574932112344912781866368423L,
-0.000531748268609265328787927274L,
-0.000491806418466550487877952946L,
-0.000454863402670479383387708294L,
-0.000420694266253628784951040798L,
-0.000389090900494976793012880112L,
-0.000359860785464029254120121859L,
-0.000332825825863365676315908104L,
-0.000307821273274928842132724949L,
-0.000284694728501071908123576382L,
-0.000263305218097299742214395524L,
-0.000243522339671520948864089084L,
-0.000225225470778005540514396521L,
-0.000208303036878938746664577386L,
-0.000192651833793426238965879049L,
-0.000178176400799979003800391220L,
-0.000164788440489346463529302111L,
-0.000152406281986195221841637326L,
-0.000140954384261487639107279790L,
-0.000130362876626863652277169003L,
-0.000120567133521140391959163007L,
-0.000111507381207939971915280020L,
-0.000103128333820225184737535973L,
-0.000095378856724241673067585245L,
-0.000088211655056527263482277212L,
-0.000081582985595824578160632778L,
-0.000075452390244459816145625175L,
-0.000069782449464405978058374747L,
-0.000064538554198035130650586144L,
-0.000059688694900672521791770014L,
-0.000055203266395283456867242033L,
-0.000051054887320421419509390570L,
-0.000047218233211063132708559115L,
};
FLOAT64 dlp_lsadd_errorlin(FLOAT64 diff){
  register INT32 idx;
  register FLOAT64 xbegin;
  idx=diff/DLP_LSADD_LIN_INC;
  xbegin=idx*DLP_LSADD_LIN_INC;
  return dlp_lsadd_lin_ybegin[idx]+(diff-xbegin)*dlp_lsadd_lin_anstieg[idx];
}

#endif /* __OPTIMIZE_LSADD */

#define ARITH_FTYPE_CODE  T_FLOAT
#include "dlp_arith_core.c"
#undef ARITH_FTYPE_CODE
#define ARITH_FTYPE_CODE  T_DOUBLE
#include "dlp_arith_core.c"
#undef ARITH_FTYPE_CODE

/**
 * Performs scalar vector operations (aggregation of vector components).
 *
 * @param  lpVec     Vector buffer, must contain nDim values
 * @param  lpMask    Coefficient vector, may be NULL or must contain nDim values
 * @param  nParam    Parameter
 * @param  nDim      Number of vector components
 * @param  nFirst    Index of first vector in buffer
 * @param  nOffs     Index offset to next vector comp.
 * @param  nOpcode   Operation code
 * @param  lpnResult The result to return.
 * @return O_K, NOT_EXEC if nOpcode is not valid or not supported.
 * @see    dlp_get_type_size
 */
INT16 dlp_aggrop
(
  FLOAT64* lpVec,
  FLOAT64* lpMask,
  FLOAT64  nParam,
  INT32    nDim,
  INT32    nFirst,
  INT32    nOffs,
  INT16    nOpcode,
  FLOAT64* lpnResult
)
{
  register INT32   i    = 0;
  register INT32   j    = 0;
  register INT32   flag = 0;
  register INT32   k    = nOffs*nDim;
  register INT16   op   = OP_RANK;
  register FLOAT64 a    = 0.;
  register FLOAT64 b    = 0.;
  register FLOAT64 d    = 0.;
  register FLOAT64 g    = 0.;
  INT16            r    = (INT16)nParam;

  *lpnResult = 0.0;

  switch (nOpcode)
  {
    case OP_QUANTIL: r=(INT16)(nDim * nParam); break;
    case OP_QUARTIL: r=(INT16)(nDim * 0.25  ); break;
    case OP_MED    : r=(INT16)(nDim * 0.5   ); break;
    default        : op=nOpcode;               break;
  }

  /* Without mask */
  if (lpMask == NULL)
  {
    switch (op)
    {
    case OP_DIFF:
      *lpnResult=lpVec[nFirst]-lpVec[nFirst+(nDim-1)*nOffs];
      return O_K;
    case OP_SUM:
      for (i=nFirst; i<k; i+=nOffs) *lpnResult += lpVec[i];
      return O_K;
    case OP_LSSUM:
    case OP_LSMEAN:
      *lpnResult = T_FLOAT_MAX;
      for (i=nFirst; i<k; i+=nOffs) *lpnResult = dlp_scalop(*lpnResult,lpVec[i],OP_LSADD);
      if(op==OP_LSMEAN) *lpnResult += log((FLOAT64)nDim);
      return O_K;
    case OP_PROD:
      *lpnResult = lpVec[nFirst];
      for (i=nFirst+nOffs; i<k; i+=nOffs) *lpnResult *= lpVec[i];
      return O_K;
    case OP_MAX:
      *lpnResult = lpVec[nFirst];
      for (i=nFirst+nOffs; i<k; i+=nOffs) if (lpVec[i] > *lpnResult) *lpnResult = lpVec[i];
      return O_K;
    case OP_IMAX:
      a = lpVec[nFirst];
      j=nFirst;
      for (i=nFirst+nOffs; i<k; i+=nOffs) if (lpVec[i] > a)
      {
        j=i;
        a = lpVec[i];
      }
      *lpnResult = (j-nFirst)/nOffs;
      return O_K;
    case OP_MIN:
      *lpnResult = lpVec[nFirst];
      for (i=nFirst+nOffs; i<k; i+=nOffs) if (lpVec[i] < *lpnResult) *lpnResult = lpVec[i];
      return O_K;
    case OP_IMIN:
      a = lpVec[nFirst];
      j=nFirst;
      for (i=nFirst+nOffs; i<k; i+=nOffs) if (lpVec[i] < a)
      {
        j=i;
        a = lpVec[i];
      }
      *lpnResult = (j-nFirst)/nOffs;
      return O_K;
    case OP_SPAN:
      a = lpVec[nFirst];
      d = a;
      for (i=nFirst+nOffs; i<k; i+=nOffs)
      {
        if (lpVec[i] < a) a = lpVec[i];
        if (lpVec[i] > d) d = lpVec[i];
      }
      *lpnResult = d-a;
      return O_K;
    case OP_MEAN:
      for (i=nFirst; i<k; i+=nOffs) a += lpVec[i];
      *lpnResult = a/(FLOAT64)nDim;
      return O_K;
    case OP_AMEAN:
      for (i=nFirst; i<k; i+=nOffs)
      {
        if ((g=lpVec[i]) < 0) g=-g;
        a+=g;
      }
      *lpnResult = a/(FLOAT64)nDim;
      return O_K;
    case OP_QMEAN:
      for (i=nFirst; i<k; i+=nOffs) a += lpVec[i]*lpVec[i];
      *lpnResult = a/(FLOAT64)nDim;
      return O_K;
    case OP_MOMENT:
      for (i=nFirst; i<k; i+=nOffs) a += dlm_pow(lpVec[i],nParam);
      *lpnResult = a/(FLOAT64)nDim;
      return O_K;
    case OP_CMOMENT:
      for (i=nFirst; i<k; i+=nOffs) a += lpVec[i];
      a /= (FLOAT64)nDim;
      for (i=nFirst; i<k; i+=nOffs) b += dlm_pow(lpVec[i]-a,nParam);
      *lpnResult = b/(FLOAT64)nDim;
      return O_K;
    case OP_GMEAN:
      for (i=nFirst; i<k; i+=nOffs) a += log(lpVec[i]);
      a /= (FLOAT64)nDim;
      *lpnResult = exp(a);
      return O_K;
    case OP_HMEAN:
      for (i=nFirst; i<k; i+=nOffs) a+= 1./lpVec[i];
      *lpnResult = 1/a;
      return O_K;
    case OP_RANK:
      if (nDim > __nhbuff)
      {
        dlp_free(__hbuff);
        __hbuff=(FLOAT64*)dlp_calloc(nDim+1,sizeof(FLOAT64));
        __nhbuff=nDim+1;
      }
      for (i=nFirst; i<k; i+=nOffs) __hbuff[j++]= lpVec[i];
      do {
        flag = 0;
        for (i=1; i<nDim; i++) if (__hbuff[i-1] > __hbuff[i])
        {
          a = __hbuff[i-1];
          __hbuff[i-1] = __hbuff[i];
          __hbuff[i]   = a;
          flag = 1;
        }
      } while (flag != 0);
      *lpnResult = __hbuff[r];
      return O_K;
    case OP_VAR:
    case OP_STDEV:
      for (i=nFirst; i<k; i+=nOffs)
      {
        d = lpVec[i];
        g += d*d;
        a += d;
      }
      b = a/((FLOAT64)nDim -1);
      a /= (FLOAT64)nDim;
      g /= ((FLOAT64)nDim-1);
      *lpnResult = (op==OP_VAR)?g-a*b:sqrt(g-a*b);
      return O_K;
    case OP_SKEW:
      for (i=nFirst; i<k; i+=nOffs) a += lpVec[i];
      a /= (FLOAT64)nDim;
      for (i=nFirst; i<k; i+=nOffs)
      {
        d = lpVec[i]-a;
        g += d*d*d;
        b += d*d;
      }
      g /= (FLOAT64)nDim;
      b /= (FLOAT64)nDim;
      b = dlm_pow(b,3./2.);
      *lpnResult = g/b;
      return O_K;
    case OP_EXC:
      for (i=nFirst; i<k; i+=nOffs) a += lpVec[i];
      a /= (FLOAT64)nDim;
      for (i=nFirst; i<k; i+=nOffs)
      {
        d = lpVec[i]-a;
        g += d*d*d*d;
      }
      g /= (FLOAT64)nDim;
      b /= (FLOAT64)nDim;
      b = dlm_pow(b,3./2.);
      *lpnResult = g/b - 3;
      return O_K;
    case OP_MINK:
    case OP_MINKPOW:
      for (i=nFirst; i<k; i+=nOffs) a += dlm_pow(lpVec[i],nParam);
      *lpnResult = (op==OP_MINKPOW)?a:dlm_pow(a,1./nParam);
      return O_K;
    default:
      DLPASSERT(FMSG("Unknown aggregation operation code."));
      return NOT_EXEC;
    }
  }
  /* With mask */
  else
  {
    switch (op)
    {
    case OP_DIFF:
      *lpnResult=lpVec[nFirst]*lpMask[nDim-1]-lpVec[nFirst+(nDim-1)*nOffs] *lpMask[0];
      return O_K;
    case OP_SUM:
      for (i=nFirst; i<k; i+=nOffs)
      {
        g= lpMask[j++] * lpVec[i];
        *lpnResult += g;
      }
      return O_K;
    case OP_LSSUM:
    case OP_LSMEAN:
      *lpnResult = T_FLOAT_MAX;
      for (i=nFirst; i<k; i+=nOffs, j++) if(lpMask[j]<T_FLOAT_MAX)
      {
        g = lpMask[j] + lpVec[i];
        *lpnResult = dlp_scalop(*lpnResult,g,OP_LSADD);
      }
      if(op==OP_LSMEAN) *lpnResult += log((FLOAT64)nDim);
      return O_K;
    case OP_PROD:
      *lpnResult = lpMask[j++] * lpVec[nFirst];
      for (i=nFirst+nOffs; i<k; i+=nOffs)
      {
        g= lpMask[j++] * lpVec[i];
        *lpnResult += g;
      }
      return O_K;
    case OP_MAX:
      *lpnResult =  lpMask[j++]*lpVec[nFirst];
      for (i=nFirst+nOffs; i< k; i+=nOffs)
      if ((g=lpMask[j++]*lpVec[i]) > *lpnResult) *lpnResult = g;
      return O_K;
    /* TODO: Implement OP_IMAX */
    case OP_MIN:
      *lpnResult =  lpMask[j++]*lpVec[nFirst];
      for (i=nFirst+nOffs; i< k; i+=nOffs) if ((g=lpMask[j++]*lpVec[i]) < *lpnResult) *lpnResult = g;
      return O_K;
    /* TODO: Implement OP_IMIN */
    case OP_SPAN:
      a =  lpMask[j++]*lpVec[nFirst];
      d = a;
      for (i=nFirst+nOffs; i< k; i+=nOffs)
      {
        if ((g=lpMask[j++]*lpVec[i]) < a) a = g;
        if (g > d)                     d = g;
      }
      *lpnResult =  d-a;
      return O_K;
    case OP_MEAN:
      for (i=nFirst; i<k; i+=nOffs)
      {
        g= lpMask[j++] * lpVec[i];
        a += g;
      }
      *lpnResult = a/(FLOAT64)nDim;
      return O_K;
    case OP_AMEAN:
      for (i=nFirst; i< k; i+=nOffs)
      {
        if ((g = lpMask[j++] * lpVec[i]) < 0.) g=-g;
        a += g;
      }
      *lpnResult = a/(FLOAT64)nDim;
      return O_K;
    case OP_QMEAN:
      for (i=nFirst; i<k; i+=nOffs)
      {
        g= lpMask[j++] * lpVec[i];
        a += dlm_pow(g,2.);
      }
      *lpnResult = a/(FLOAT64)nDim;
      return O_K;
    case OP_MOMENT:
      for (i=nFirst; i<k; i+=nOffs)
      {
        g= lpMask[j++] * lpVec[i];
        a += dlm_pow(g,nParam);
      }
      *lpnResult = a/(FLOAT64)nDim;
      return O_K;
    case OP_CMOMENT:
      for (i=nFirst; i<k; i+=nOffs)  a += (lpMask[j++] * lpVec[i]);
      a /= (FLOAT64)nDim;
      for (i=nFirst; i<k; i+=nOffs)
      {
        g= lpMask[j++] * lpVec[i] - a;
        b += dlm_pow(g, nParam);
      }
      *lpnResult = b/(FLOAT64)nDim;
      return O_K;
    case OP_GMEAN:
      for (i=nFirst; i<k; i+=nOffs)
      {
        g= lpMask[j++] * lpVec[i];
        a += log(g);
      }
      a /= (FLOAT64)nDim;
      *lpnResult = exp(a);
      return O_K;
    case OP_HMEAN:
      for (i=nFirst; i<k; i+=nOffs)
      {
        g= lpMask[j++] * lpVec[i];
        a = 1/g;
      }
      a /= (FLOAT64)nDim;
      *lpnResult = 1/a;
      return O_K;
    case OP_RANK:
      if (nDim > __nhbuff)
      {
        dlp_free(__hbuff);
        __hbuff=(FLOAT64*)dlp_calloc(nDim+1,sizeof(FLOAT64));
        __nhbuff=nDim+1;
      }
      /* MW 2004-08-09: Because of gcc warning "operation on `j' may be undefined" -->
      for (i=nFirst; i<k; i+=nOffs) __hbuff[j]=  lpMask[j++] * lpVec[i]; */
      for (i=nFirst,j=0; i<k; i+=nOffs,j++) __hbuff[j]=  lpMask[j+1] * lpVec[i];
      /* <-- */
      do {
        flag = 0;
        for (i=1; i<nDim; i++) if (__hbuff[i-1] > __hbuff[i])
        {
          a = __hbuff[i-1];
          __hbuff[i-1] = __hbuff[i];
          __hbuff[i]   = a;
          flag = 1;
        }
      } while (flag != 0);
      *lpnResult = __hbuff[r];
      return O_K;
    case OP_VAR:
      g=0.;
      for (i=nFirst; i< k; i+=nOffs)
      {
        d =  lpMask[j++] * lpVec[i];
        g += d*d;
        a += d;
      }
      b = a/((FLOAT64)nDim-1);
      a /= (FLOAT64)nDim;
      g = g/((FLOAT64)nDim-1);
      *lpnResult = g-a*b;
      return O_K;
    case OP_STDEV:
      g=0.;
      for (i=nFirst; i< k; i+=nOffs)
      {
        d =  lpMask[j++] * lpVec[i];
        g += d*d;
        a += d;
      }
      b = a/((FLOAT64)nDim-1);
      a /= (FLOAT64)nDim;
      g = g/((FLOAT64)nDim-1);
      *lpnResult = sqrt(g-a*b);
      return O_K;
    case OP_SKEW:
      for (i=nFirst; i<k; i+=nOffs) a += lpMask[j++] * lpVec[i];
      a /= (FLOAT64)nDim;
      j=0;
      for (i=nFirst; i<k; i+=nOffs)
      {
        d = lpMask[j++] * lpVec[i]-a;
        g += d*d*d;
        b += d*d;
      }
      g /= (FLOAT64)nDim;
      b /= (FLOAT64)nDim;
      b = dlm_pow(b, 3./2.);
      *lpnResult = g/b;
      return O_K;
    case OP_EXC:
      for (i=nFirst; i<k; i+=nOffs) a += lpMask[j++] * lpVec[i];
      a /= (FLOAT64)nDim;
      j=0;
      for (i=nFirst; i<k; i+=nOffs)
      {
        d = lpMask[j++] * lpVec[i]-a;
        g += d*d*d*d;
        b += d*d;
      }
      g /= (FLOAT64)nDim;
      b /= (FLOAT64)nDim;
      b = dlm_pow(b,3./2.);
      *lpnResult = g/b - 3;
      return O_K;
    case OP_MINK:
    case OP_MINKPOW:
      for (i=nFirst; i<k; i+=nOffs)
      {
        g = lpMask[j++] * lpVec[i];
        a += dlm_pow(g,nParam);
      }
      *lpnResult = (op==OP_MINKPOW)?a:dlm_pow(a,1./nParam);
      return O_K;
    default:
      DLPASSERT(FMSG("Unknown aggregation operation code."));
      return NOT_EXEC;
    }
  }
}

INT16 CGEN_IGNORE dlp_aggropC
(
  COMPLEX64* lpVec,
  COMPLEX64* lpMask,
  COMPLEX64  nParam,
  INT32      nDim,
  INT32      nFirst,
  INT32      nOffs,
  INT16      nOpcode,
  COMPLEX64* lpnResult
)
{
  register INT32     i    = 0;
  register INT32     j    = 0;
  register INT32     flag = 0;
  register INT32     k    = nOffs*nDim;
  register INT16     op   = OP_RANK;
  register COMPLEX64 a    = CMPLX(0.);
  register COMPLEX64 b    = CMPLX(0.);
  register COMPLEX64 d    = CMPLX(0.);
  register COMPLEX64 g    = CMPLX(0.);
  INT16              r    = (INT16)nParam.x;

  *lpnResult = CMPLX(0.);

  switch (nOpcode)
  {
    case OP_QUANTIL: r=(INT16)(nDim * nParam.x); break;
    case OP_QUARTIL: r=(INT16)(nDim * 0.25    ); break;
    case OP_MED    : r=(INT16)(nDim * 0.5     ); break;
    default        : op=nOpcode;               break;
  }

  /* Without mask */
  if (lpMask == NULL)
  {
    switch (op)
    {
    case OP_DIFF:
      *lpnResult=CMPLX_MINUS(lpVec[nFirst],lpVec[nFirst+(nDim-1)*nOffs]);
      return O_K;
    case OP_SUM:
      for (i=nFirst; i<k; i+=nOffs) *lpnResult = CMPLX_PLUS(*lpnResult,lpVec[i]);
      return O_K;
    case OP_LSSUM:
    case OP_LSMEAN:
      *lpnResult = CMPLX(T_FLOAT_MAX);
      for (i=nFirst; i<k; i+=nOffs) *lpnResult = dlp_scalopC(*lpnResult,lpVec[i],OP_LSADD);
      if(op==OP_LSMEAN) *lpnResult=CMPLX_PLUS(*lpnResult,CMPLX(log((FLOAT64)nDim)));
      return O_K;
    case OP_PROD:
      *lpnResult = lpVec[nFirst];
      for (i=nFirst+nOffs; i<k; i+=nOffs) *lpnResult = CMPLX_MULT(*lpnResult,lpVec[i]);
      return O_K;
    case OP_MAX:
      *lpnResult = lpVec[nFirst];
      for (i=nFirst+nOffs; i<k; i+=nOffs) if (CMPLX_GREATER(lpVec[i],*lpnResult)) *lpnResult = lpVec[i];
      return O_K;
    case OP_IMAX:
      a = lpVec[nFirst];
      j=nFirst;
      for (i=nFirst+nOffs; i<k; i+=nOffs) if (CMPLX_GREATER(lpVec[i],a)) {
        j=i;
        a = lpVec[i];
      }
      *lpnResult = CMPLX((j-nFirst)/nOffs);
      return O_K;
    case OP_MIN:
      *lpnResult = lpVec[nFirst];
      for (i=nFirst+nOffs; i<k; i+=nOffs) if (CMPLX_LESS(lpVec[i],*lpnResult)) *lpnResult = lpVec[i];
      return O_K;
    case OP_IMIN:
      a = lpVec[nFirst];
      j=nFirst;
      for (i=nFirst+nOffs; i<k; i+=nOffs) if (CMPLX_LESS(lpVec[i],a)) {
        j=i;
        a = lpVec[i];
      }
      *lpnResult = CMPLX((j-nFirst)/nOffs);
      return O_K;
    case OP_SPAN:
      a = lpVec[nFirst];
      d = a;
      for (i=nFirst+nOffs; i<k; i+=nOffs) {
        if (CMPLX_LESS(lpVec[i],a)) a = lpVec[i];
        if (CMPLX_GREATER(lpVec[i],d)) d = lpVec[i];
      }
      *lpnResult = CMPLX_MINUS(d,a);
      return O_K;
    case OP_MEAN:
      for (i=nFirst; i<k; i+=nOffs) a = CMPLX_PLUS(a,lpVec[i]);
      *lpnResult = CMPLX_DIV_R(a,nDim);
      return O_K;
    case OP_AMEAN:
      for (i=nFirst; i<k; i+=nOffs) {
        if (CMPLX_LESS((g=lpVec[i]),CMPLX(0))) g=CMPLX_NEG(g);
        a = CMPLX_PLUS(a,g);
      }
      *lpnResult = CMPLX_DIV_R(a,nDim);
      return O_K;
    case OP_QMEAN:
      for (i=nFirst; i<k; i+=nOffs) a = CMPLX_PLUS(a,CMPLX_MULT(lpVec[i],lpVec[i]));
      *lpnResult = CMPLX_DIV_R(a,nDim);
      return O_K;
    case OP_MOMENT:
      for (i=nFirst; i<k; i+=nOffs) a = CMPLX_PLUS(a,dlm_powC(lpVec[i],nParam));
      *lpnResult = CMPLX_DIV_R(a,nDim);
      return O_K;
    case OP_CMOMENT:
      for (i=nFirst; i<k; i+=nOffs) a = CMPLX_PLUS(a,lpVec[i]);
      a = CMPLX_DIV_R(a,nDim);
      for (i=nFirst; i<k; i+=nOffs) b = CMPLX_PLUS(b,dlm_powC(CMPLX_MINUS(lpVec[i],a),nParam));
      *lpnResult = CMPLX_DIV_R(b,nDim);
      return O_K;
    case OP_GMEAN:
      for (i=nFirst; i<k; i+=nOffs) a = CMPLX_PLUS(a,dlm_logC(lpVec[i]));
      a = CMPLX_DIV_R(a,nDim);
      *lpnResult = dlm_expC(a);
      return O_K;
    case OP_HMEAN:
      for (i=nFirst; i<k; i+=nOffs) a = CMPLX_PLUS(a,CMPLX_INVT(lpVec[i]));
      *lpnResult = CMPLX_INVT(a);
      return O_K;
    case OP_RANK:
      if (nDim > __nhbuff) {
        dlp_free(__hbuff);
        __hbuffC=(COMPLEX64*)dlp_calloc(nDim+1,sizeof(COMPLEX64));
        __nhbuff=nDim+1;
      }
      for (i=nFirst; i<k; i+=nOffs) __hbuffC[j++] = lpVec[i];
      do {
        flag = 0;
        for (i=1; i<nDim; i++) if (CMPLX_GREATER(__hbuffC[i-1],__hbuffC[i])) {
          a = __hbuffC[i-1];
          __hbuffC[i-1] = __hbuffC[i];
          __hbuffC[i]   = a;
          flag = 1;
        }
      } while (flag != 0);
      *lpnResult = __hbuffC[r];
      return O_K;
    case OP_VAR:
    case OP_STDEV:
      for (i=nFirst; i<k; i+=nOffs) {
        d = lpVec[i];
        g = CMPLX_PLUS(g,CMPLX_MULT(d,d));
        a = CMPLX_PLUS(a,d);
      }
      b = CMPLX_DIV_R(a,nDim-1);
      a = CMPLX_DIV_R(a,nDim);
      g = CMPLX_DIV_R(g,nDim-1);
      *lpnResult = (op==OP_VAR)?CMPLX_MINUS(g,CMPLX_MULT(a,b)):dlp_scalopC(CMPLX_MINUS(g,CMPLX_MULT(a,b)),CMPLX(0),OP_SQRT);
      return O_K;
    case OP_SKEW:
      for (i=nFirst; i<k; i+=nOffs) a = CMPLX_PLUS(a,lpVec[i]);
      a = CMPLX_DIV_R(a,nDim);
      for (i=nFirst; i<k; i+=nOffs) {
        d = CMPLX_MINUS(lpVec[i],a);
        g = CMPLX_PLUS(g,CMPLX_MULT(d,CMPLX_MULT(d,d)));
        b = CMPLX_PLUS(b,CMPLX_MULT(d,d));
      }
      g = CMPLX_DIV_R(g,nDim);
      b = CMPLX_DIV_R(b,nDim);
      b = dlm_powC(b,CMPLX(3./2.));
      *lpnResult = CMPLX_DIV(g,b);
      return O_K;
    case OP_EXC:
      for (i=nFirst; i<k; i+=nOffs) a = CMPLX_PLUS(a,lpVec[i]);
      a = CMPLX_DIV_R(a,nDim);
      for (i=nFirst; i<k; i+=nOffs) {
        d = CMPLX_MINUS(lpVec[i],a);
        g = CMPLX_PLUS(g,CMPLX_MULT(d,CMPLX_MULT(d,CMPLX_MULT(d,d))));
      }
      g = CMPLX_DIV_R(g,nDim);
      b = CMPLX_DIV_R(b,nDim);
      b = dlm_powC(b,CMPLX(3./2.));
      *lpnResult = CMPLX_MINUS(CMPLX_DIV(g,b),CMPLX(3));
      return O_K;
    case OP_MINK:
    case OP_MINKPOW:
      for (i=nFirst; i<k; i+=nOffs) a = CMPLX_PLUS(a,dlm_powC(lpVec[i],nParam));
      *lpnResult = (op==OP_MINKPOW)?a:dlm_powC(a,CMPLX_INVT(nParam));
      return O_K;
    default:
      DLPASSERT(FMSG("Unknown aggregation operation code."));
      return NOT_EXEC;
    }
  }
  /* With mask */
  else
  {
    return NOT_EXEC;
  }
}

COMPLEX64 CGEN_IGNORE dlp_scalopC(COMPLEX64 nParam1, COMPLEX64 nParam2, INT16 nOpcode) {

  switch (nOpcode) {
    case OP_NOOP     : { return nParam1; }
    case OP_REAL     : { return CMPLX(nParam1.x); }
    case OP_IMAG     : { return CMPLX(nParam1.y); }
    case OP_CONJ     : { return CMPLX_CONJ(nParam1); }
    case OP_NEG      : { return CMPLX_NEG(nParam1); }
    case OP_ANGLE    : { return CMPLX(CMPLX_ANGLE(nParam1)); }
    case OP_SQR      : { return CMPLXY(nParam1.x*nParam1.x-nParam1.y*nParam1.y,2*nParam1.x*nParam1.y); }
    case OP_ABS      : {
      FLOAT64 x,y,w,r;
      x = fabs(nParam1.x);
      y = fabs(nParam1.y);
      if(x == 0.0)      { w = y; }
      else if(y == 0.0) { w = x; }
      else if(x > y)    { r = y/x; w = x*sqrt(1.0+r*r); }
      else              { r = x/y; w = y*sqrt(1.0+r*r); }
      return CMPLX(w); }
    case OP_SIGN     : { return CMPLX_DIV_R(nParam1,CMPLX_ABS(nParam1)); }
    case OP_ENT      : { return CMPLX_ENT(nParam1); }
    case OP_FLOOR    : { return CMPLX_FLOOR(nParam1); }
    case OP_CEIL     : { return CMPLX_CEIL(nParam1); }
    case OP_INC      : { return CMPLX_INC(nParam1); }
    case OP_DEC      : { return CMPLX_DEC(nParam1); }
    case OP_INVT     : {
      FLOAT64 r,den;
      if(nParam1.y == 0.0) {
        return CMPLXY(1.0/nParam1.x,0.0);
      } else if(fabs(nParam1.x)>=fabs(nParam1.y)) {
        r = nParam1.y / nParam1.x;
        den = nParam1.x + r*nParam1.y;
        return CMPLXY(1.0/den,-r/den);
      } else {
        r = nParam1.x / nParam1.y;
        den = nParam1.y + r*nParam1.x;
        return CMPLXY(r/den,-1/den);
      } }
    case OP_LN       : { return dlm_logC(nParam1); }
    case OP_EXP      : { return dlm_expC(nParam1); }
    case OP_SQRT     : {
      FLOAT64 x,y,w,r;
      if((nParam1.x == 0.0) && (nParam1.y == 0.0)) return CMPLX(0);
      x = fabs(nParam1.x);
      y = fabs(nParam1.y);
      if(x >= y) { r = y/x; w = sqrt(x)*sqrt(0.5*(1.0+sqrt(1.0+r*r))); }
      else       { r = x/y; w = sqrt(y)*sqrt(0.5*(1.0+sqrt(1.0+r*r))); }
      if (nParam1.x>=0) return CMPLXY(w, nParam1.y/(2.0*w));
      else if (nParam1.y<0) w = -w;
      return CMPLXY(nParam1.y/(2.0*w), w); }
    case OP_LOG      : { return dlm_log_bC(CMPLX(10),nParam1); }
    case OP_LOG2     : { return dlm_log_bC(CMPLX(2),nParam1); }
    case OP_SIN      : {
      FLOAT64 x1, y1;
      FLOAT64 x2, y2;
      FLOAT64 s;
      s = exp(-nParam1.y);
      x1 = s * cos(nParam1.x);
      y1 = s * sin(nParam1.x);
      s = exp(nParam1.y);
      x2 = s * cos(-nParam1.x);
      y2 = s * sin(-nParam1.x);
      x1 -= x2;
      y1 -= y2;
      return CMPLXY(0.5*y1, -0.5*x1);}
    case OP_SINC     : {
      COMPLEX64 z;
      if (nParam1.x==0 && nParam1.y==0) return CMPLX(1);
      z = dlp_scalopC(nParam1,CMPLX(0),OP_SIN);
      return dlp_scalopC(z,nParam1,OP_DIV);
    }
    case OP_ASIN     : {
      FLOAT64 x, y;
      COMPLEX64 z;
      x =  1.0 - ( (nParam1.x*nParam1.x) - (nParam1.y*nParam1.y) );
      y =  0.0 - ( (nParam1.x*nParam1.y) + (nParam1.y*nParam1.x) );
      z =  CMPLXY(x, y);
      z = dlp_scalopC(z,CMPLX(0),OP_SQRT);
      z.x += -nParam1.y;
      z.y +=  nParam1.x;
      x = log(CMPLX_ABS(z));
      y = CMPLX_ANGLE(z);
      z.x =   y;
      z.y = - x;
      return z; }
    case OP_SINH     : {
      FLOAT64 x1, y1;
      FLOAT64 x2, y2;
      FLOAT64 s;
      s = exp(nParam1.x);
      x1 = s * cos(nParam1.y);
      y1 = s * sin(nParam1.y);
      s = exp(-nParam1.x);
      x2 = s * cos(-nParam1.y);
      y2 = s * sin(-nParam1.y);
      x1 -= x2;
      y1 -= y2;
      return CMPLXY(0.5*y1, 0.5*x1); }
    case OP_ASINH: {
      FLOAT64 t;
      COMPLEX64 z;
      z = CMPLXY(((nParam1.x*nParam1.x) - (nParam1.y*nParam1.y)) + 1, (nParam1.x*nParam1.y) + (nParam1.y*nParam1.x));
      z = dlp_scalopC(z,CMPLX(0),OP_SQRT);
      z.x += nParam1.x;
      z.y += nParam1.y;
      t = CMPLX_ANGLE(z);
      z.x = log(CMPLX_ABS(z));
      z.y = t;
      return z; }
    case OP_COS      : {
      FLOAT64 x1, y1;
      FLOAT64 x2, y2;
      FLOAT64 s;
      s = exp(-nParam1.y);
      x1 = s * cos(nParam1.x);
      y1 = s * sin(nParam1.x);
      s = exp(nParam1.y);
      x2 = s * cos(-nParam1.x);
      y2 = s * sin(-nParam1.x);
      x1 += x2;
      y1 += y2;
      return CMPLXY(0.5*x1, 0.5*y1);
      break; }
    case OP_ACOS     : {
      FLOAT64 x, y;
      COMPLEX64 z;
      x =  1.0 - ( (nParam1.x*nParam1.x) - (nParam1.y*nParam1.y) );
      y =  0.0 - ( (nParam1.x*nParam1.y) + (nParam1.y*nParam1.x) );
      z =  CMPLXY(x, y);
      z = dlp_scalopC(z,CMPLX(0),OP_SQRT);
      x = z.y;
      y = z.x;
      z.x = nParam1.x - x;
      z.y = nParam1.y + y;
      return CMPLXY(log(CMPLX_ABS(z)),CMPLX_ANGLE(z)); }
    case OP_COSH     : {
      FLOAT64 x1, y1;
      FLOAT64 x2, y2;
      FLOAT64 s;
      s = exp(nParam1.x);
      x1 = s * cos(nParam1.y);
      y1 = s * sin(nParam1.y);
      s = exp(-nParam1.x);
      x2 = s * cos(-nParam1.y);
      y2 = s * sin(-nParam1.y);
      x1 += x2;
      y1 += y2;
      return CMPLXY(0.5*y1, 0.5*x1); }
    case OP_ACOSH: {
      FLOAT64 t;
      COMPLEX64 z;
      z = CMPLXY(((nParam1.x*nParam1.x) - (nParam1.y*nParam1.y)) - 1, (nParam1.x*nParam1.y) + (nParam1.y*nParam1.x));
      z = dlp_scalopC(z,CMPLX(0),OP_SQRT);
      z.x += nParam1.x;
      z.y += nParam1.y;
      t = CMPLX_ANGLE(z);
      z.x = log(CMPLX_ABS(z));
      z.y = t;
      return z; }
    case OP_TAN      : {
      return dlp_scalopC(dlp_scalopC(nParam1,CMPLX(0),OP_SIN),dlp_scalopC(nParam1,CMPLX(0),OP_COS),OP_DIV); }
    case OP_ATAN     : {
      FLOAT64 x, y;
      COMPLEX64 z = CMPLXY(-nParam1.x, 1.0 - nParam1.y);
      x = nParam1.x;
      y = 1.0 + nParam1.y;
      z = dlp_scalopC(z,CMPLXY(x,y),OP_DIV);
      x = log(CMPLX_ABS(z));
      y = CMPLX_ANGLE(z);
      z.x =  0.5*y;
      z.y = -0.5*x;
      return z; }
    case OP_TANH     : {
      return dlp_scalopC(dlp_scalopC(nParam1,CMPLX(0),OP_SINH),dlp_scalopC(nParam1,CMPLX(0),OP_COSH),OP_DIV); }
    case OP_ATANH    : {
      FLOAT64 x, y;
      COMPLEX64 z = CMPLXY(1.0 + nParam1.x, nParam1.y);
      x = 1.0 - nParam1.x;
      y =     - nParam1.y;
      z = dlp_scalopC(z, CMPLXY(x, y), OP_DIV);
      x = log(CMPLX_ABS(z));
      y = CMPLX_ANGLE(z);
      z.x = 0.5*x;
      z.y = 0.5*y;
      return z; }
    case OP_SET      : { return nParam1; }
    case OP_ADD      : { return CMPLX_PLUS(nParam1,nParam2); }
    case OP_LSADD    : { return CMPLX_MINUS(CMPLX_MIN(nParam1,nParam2),dlp_scalopC(CMPLX_INC(dlp_scalopC(CMPLX_MINUS(CMPLX_MIN(nParam1,nParam2),CMPLX_MAX(nParam1,nParam2)),CMPLX(0),OP_EXP)),CMPLX(0),OP_LOG)); }
    case OP_EXPADD   : { return CMPLX_PLUS(CMPLX_MAX(nParam1,nParam2),dlp_scalopC(CMPLX_INC(dlp_scalopC(CMPLX_MINUS(CMPLX_MIN(nParam1,nParam2),CMPLX_MAX(nParam1,nParam2)),CMPLX(0),OP_EXP)),CMPLX(0),OP_LOG)); }
    case OP_DIFF     : { return CMPLX_MINUS(nParam1,nParam2); }
    case OP_QDIFF    : { COMPLEX64 tmp = CMPLX_MINUS(nParam1,nParam2); return dlp_scalopC(tmp, tmp, OP_MULT); }
    case OP_ABSDIFF  : { return dlp_scalopC(CMPLX_MINUS(nParam1,nParam2), CMPLX(0), OP_ABS); }
    case OP_QABSDIFF : { COMPLEX64 tmp = CMPLX_MINUS(nParam1,nParam2); return dlp_scalopC(tmp, CMPLX_CONJ(tmp), OP_MULT); }
    case OP_MULT     : { return CMPLX_MULT(nParam1,nParam2); }
    case OP_DIV      : {
      FLOAT64 r,den;
      if(fabs(nParam2.x)>=fabs(nParam2.y)) {
        r = nParam2.y / nParam2.x;
        den = nParam2.x + r*nParam2.y;
        return CMPLXY((nParam1.x+r*nParam1.y)/den,(nParam1.y-r*nParam1.x)/den);
      } else {
        r = nParam2.x / nParam2.y;
        den = nParam2.y + r*nParam2.x;
        return CMPLXY((nParam1.x*r+nParam1.y)/den,(nParam1.y*r-nParam1.x)/den);
      } }
    case OP_DIV1     : { return CMPLX_DIV(nParam1,CMPLX_INC(nParam2)); }
    case OP_LNL      : { return CMPLX_LESS(dlm_logC(nParam1),nParam2)?nParam2:dlm_logC(nParam1); }
    case OP_POW      : { return dlm_powC(nParam1,nParam2); }
    case OP_GAUSS    : { return dlm_expC(CMPLX_NEG(CMPLX_DIV(CMPLX_MULT(nParam1,nParam1),CMPLX_MULT(nParam2,nParam2)))); }
    case OP_EQUAL    : { return CMPLX(CMPLX_EQUAL(nParam1,nParam2)?1.:0.); }
    case OP_NEQUAL   : { return CMPLX(CMPLX_EQUAL(nParam1,nParam2)?0.:1.); }
    case OP_LESS     : { return CMPLX(CMPLX_LESS(nParam1,nParam2)?1.:0.); }
    case OP_GREATER  : { return CMPLX(CMPLX_GREATER(nParam1,nParam2)?1.:0.); }
    case OP_LEQ      : { return CMPLX(CMPLX_LEQ(nParam1,nParam2)?1.:0.); }
    case OP_GEQ      : { return CMPLX(CMPLX_GEQ(nParam1,nParam2)?1.:0.); }
    case OP_ISNAN    : { return CMPLX(CMPLX_ISNAN(nParam1)?1.:0.); }
    case OP_MAX      : { return CMPLX_MAX(nParam1,nParam2); }
    case OP_AMAX     : { return CMPLX((CMPLX_ABS(nParam1)>CMPLX_ABS(nParam2))?CMPLX_ABS(nParam1):CMPLX_ABS(nParam2)); }
    case OP_SMAX     : { return (CMPLX_ABS(nParam1)>CMPLX_ABS(nParam2))?nParam1:nParam2; }
    case OP_MIN      : { return CMPLX_MIN(nParam1,nParam2); }
    case OP_AMIN     : { return CMPLX((CMPLX_ABS(nParam1)<CMPLX_ABS(nParam2))?CMPLX_ABS(nParam1):CMPLX_ABS(nParam2)); }
    case OP_SMIN     : { return (CMPLX_ABS(nParam1)<CMPLX_ABS(nParam2))?nParam1:nParam2; }
    case OP_ROUND    : {
      FLOAT64 x, y;
      x = dlp_isnan(nParam1.x) ? nParam1.x : (INT64)round(nParam1.x);
      y = dlp_isnan(nParam1.y) ? nParam1.y : (INT64)round(nParam1.y);
      return CMPLXY(x,y);
    }

    /* TODO: The following real functions process the real part of the argument(s) only. */
    case OP_NOVERK  : { return CMPLX(dlm_n_over_k((INT32)nParam1.x,(INT32)nParam2.x)); }
    case OP_FCTRL   : {
      FLOAT64 x;
      INT32   i;
      for (i=2,x=1.; i<=(INT32)nParam1.x; i++) x*=(FLOAT64)i;
      return CMPLX(x);
    }
    case OP_GAMMA   : { return CMPLX(dlm_gamma(nParam1.x)); }
    case OP_LGAMMA  : { return CMPLX(dlm_lgamma(nParam1.x)); }
    case OP_BETA    : { return CMPLX(dlm_beta(nParam1.x,nParam2.x)); }
    case OP_STUDT   : { return CMPLX(dlm_studt(nParam1.x,nParam2.x)); }
    case OP_ERF     : { return CMPLX(erf(nParam1.x)); }
    case OP_ERFC    : { return CMPLX(erfc(nParam1.x)); }
    case OP_AND     :
    case OP_NOT     :
    case OP_OR      :
    case OP_POTENTIAL:
    case OP_MOD     : { return CMPLX(dlp_scalop(nParam1.x,nParam2.x,nOpcode)); }

    default: { DLPASSERT(FMSG("Unknown scalar operation code")); return CMPLX(0.); }
  }
}

COMPLEX64 CGEN_IGNORE dlp_scalopC3(COMPLEX64 nParam1, COMPLEX64 nParam2, COMPLEX64 nParam3, INT16 nOpcode)
{
  switch (nOpcode)
  {
    /* TODO: The following real functions process the real part of the argument(s) only. */
    case OP_BETADENS : { return CMPLX(dlm_betadens(nParam1.x,nParam2.x,nParam3.x)); }
    case OP_BETAQUANT: { return CMPLX(dlm_betaquant(nParam1.x,nParam2.x,nParam3.x)); }

    default: { DLPASSERT(FMSG("Unknown scalar operation code")); return CMPLX(0.); }
  }
}

/* TODO: Implement scalar operation wrapper for arbitrary signatures.
COMPLEX64* CGEN_IGNORE dlp_scalopCN(COMPLEX* args, COMPLEX* res, INT16 nOpcode)
{
}
*/

/* EOF */

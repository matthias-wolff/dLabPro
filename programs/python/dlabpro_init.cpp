#include "dlp_base.h"
#include "dlp_object.h"
#include "dlp_data.h"
#include "dlp_fst.h"
#include "dlp_gmm.h"
#include "dlp_hmm.h"
#include "dlp_vmap.h"
#include "dlp_statistics.h"
#include "dlp_profile.h"
#include "dlabpro_init.hpp"

void dlabpro_init(){
  dlp_xalloc_init(0);
  REGISTER_CLASS(CDlpObject);
  REGISTER_CLASS(CData);
  REGISTER_CLASS(CVmap);
  REGISTER_CLASS(CStatistics);
  REGISTER_CLASS(CProfile);
  REGISTER_CLASS(CFst);
  REGISTER_CLASS(CGmm);
  REGISTER_CLASS(CHmm);
}


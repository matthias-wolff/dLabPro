/* dLabPro class CSvm (svm)
 * - Data functions
 *
 * AUTHOR : Robert Schubert
 * PACKAGE: dLabPro/classes
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
#include "dlp_cscope.h"
#include "dlp_svm.h"
#include "../../ext/libsvm/libsvm.h"

INT32 CSvm_getClassIndex(CSvm *_this, INT32 label);
INT32 CSvm_getClassLabel(CSvm *_this, INT32 index);
FLOAT64 CSvm_getClassWeightByLabel(CSvm *_this, INT32 label);
FLOAT64 CSvm_getClassWeightByIndex(CSvm *_this, INT32 index);
void CSvm_toParameters(CSvm *_this, struct svm_parameter* param);
bool CSvm_toProblemSet(CSvm *_this, CData* trainset, struct svm_problem* prob);
bool CSvm_fromModel(CSvm *_this, struct svm_model* newmodel);
bool CSvm_toModel(CSvm *_this, struct svm_model* model);
void freeProblemSet(struct svm_problem* prob);
void freeParameters(struct svm_parameter* param);
void freeModel(struct svm_model* model);
struct svm_node* double_array2svm_node_array(FLOAT64 *input, UINT32 nr_elems, struct svm_node *output);
FLOAT64* svm_node_array2double_array(struct svm_node* input, UINT32 nr_elems, FLOAT64 *output);


/** looks up the {@link model_classes} field
 * @param label the class integer
 * @return the record number of label
 */
INT32 CSvm_getClassIndex(CSvm *_this, INT32 label)
{
    INT16 idx, maxidx = CData_GetNRecs(_this->m_idModelClasses);
/* Robert Schubert, Feb 2006: this was way too slow
    CData *subjpat = NULL, *indxres = NULL;
    ICREATE(CData, subjpat);
    ICREATE(CData, indxres);

    CData_Array(subjpat, T_INT, 1, 1);

    CData_Dstore(subjpat, (FLOAT64) label, 0, 0);
    CData_GenIndex(indxres, subjpat, _this->m_idModelClasses, 0, 0);
    index = (INT32) CData_Dfetch(indxres, 0, 0);

    IDESTROY(subjpat);
    IDESTROY(indxres);
*/
    for (idx = 0; idx < maxidx; idx++)
  if (label == (INT32) CData_Dfetch(_this->m_idModelClasses, idx, 0))
      return idx;
    return -1;

}

/** looks up the {@link model_classes} field
 * @param index the record number
 * @return the class integer
 */
INT32 CSvm_getClassLabel(CSvm *_this, INT32 index)
{
    if (! _this->m_idModelClasses)
  return (0);

    return ((INT32)CData_Dfetch(_this->m_idModelClasses, index, 0));
}

/** looks up the {@link param_C_weights} field
 * @param label the class integer
 * @return the weight factor (1.0 if unset)
 */
FLOAT64 CSvm_getClassWeightByLabel(CSvm *_this, INT32 label)
{
    if (! _this->m_idParamCWeights)
  return (1.0);

    INT16 idx, maxidx = CData_GetNRecs(_this->m_idParamCWeights);
/* Robert Schubert, Feb 2006: this was way too slow
    CData *subjpat = NULL, *indxres = NULL;
    ICREATE(CData, subjpat);
    ICREATE(CData, indxres);

    CData_Array(subjpat, T_INT, 1, 1);

    CData_Dstore(subjpat, (FLOAT64) label, 0, 0);
    CData_GenIndex(indxres, subjpat, _this->m_idParamCWeights, 0, 0);
    index = (INT32) CData_Dfetch(indxres, 0, 0);

    IDESTROY(subjpat);
    IDESTROY(indxres);
*/
    for (idx = 0; idx < maxidx; idx++)
  if (label == (INT32) CData_Dfetch(_this->m_idParamCWeights, idx, 0))
      return (CData_Dfetch(_this->m_idParamCWeights, idx, 1));
    return 1.0;
}

/** looks up the {@link param_C_weights} and {@link model_classes} fields
 * @param index the class index ({@link model_classes} record number)
 * @return the weight factor (1.0 if unset)
 */
FLOAT64 CSvm_getClassWeightByIndex(CSvm *_this, INT32 index)
{
    if (! _this->m_idParamCWeights)
  return (1.0);

    return (CSvm_getClassWeightByLabel(_this, (INT32)CData_Dfetch(_this->m_idModelClasses, index, 0)));
}

void CGEN_EXPORT PrintParams(struct svm_parameter* param)
{
    printf("kernel type: %d\n", param->kernel_type);
    printf("degree: %lf\n", param->degree);
    printf("gradient: %lf\n", param->gamma);
    printf("offset: %lf\n", param->coef0);
    printf("epsilon: %lf\n", param->eps);
    printf("C: %lf\n", param->C);
}

/**  fills a struct svm_parameter instance from this classes fields
 * @param param the transient libsvm data structure to write the results to
 */
void CGEN_PRIVATE CSvm_toParameters(CSvm *_this, struct svm_parameter* param)
{
    INT32 n;

    /***** field settings *************/
    param->kernel_type = _this->m_nParamKnlType;
    param->degree = (FLOAT64) _this->m_nParamKnlDegree;
    param->gamma = _this->m_nParamKnlGradient ? _this->m_nParamKnlGradient : (_this->m_nModelNrfeatures ? 1.0/_this->m_nModelNrfeatures : 1.0 );
    param->coef0 = _this->m_nParamKnlOffset;
    param->eps = _this->m_nParamEpsilon;
    param->shrinking = ! _this->m_bNoShrinking;
    param->probability = _this->m_bProbabilities;
    param->C = _this->m_bHardMargin ? T_DOUBLE_MAX : _this->m_nParamC;
    if (_this->m_idParamCWeights && ! _this->m_bHardMargin) {
  param->nr_weight = CData_GetNRecs(_this->m_idParamCWeights);
  param->weight_label = (INT32*) dlp_calloc(param->nr_weight, sizeof(INT32));
  for (n=0; n<param->nr_weight; n++)
      param->weight_label[n] = (INT32) CData_Dfetch(_this->m_idParamCWeights, n, 0);
  param->weight = (FLOAT64*) dlp_calloc(param->nr_weight, sizeof(FLOAT64));
  for (n=0; n<param->nr_weight; n++)
      param->weight[n] = CData_Dfetch(_this->m_idParamCWeights, n, 1);
    }
    /****** some defaults *************/
    else {
  param->nr_weight = 0;
  param->weight_label = NULL;
  param->weight = NULL;
    }
    param->svm_type = 0;
#ifdef __NEW_LIBSVM_INTERFACE
    param->softsvm_type = _this->m_b2norm ? 2 : 1;
    param->threshold = _this->m_nParamThreshold;
#endif
    param->nu = 0.5;
    param->cache_size = SVM_CACHESIZE;
    param->p = 0.1;

}

#define debugprobx(sample, feature) 'DEBUGMSG(0, "prob->x['sample']['feature']={%d,%lf}\n", prob->x['sample']['feature'].index, prob->x['sample']['feature'].value, 0)'

/** transforms CData operand into svm_problem
 * @param trainset the complete sample vectors (rows) together with their class targets (last component)
 * @param prob the transient libsvm data structure to write the results to
 * @return success status
 */
bool CGEN_PRIVATE CSvm_toProblemSet(CSvm *_this, CData* idTset, struct svm_problem* lpPrblm)
{
  UINT32 nXV, nV, nXC;

  if (idTset == NULL || CData_IsEmpty(idTset))
    return FALSE;

  lpPrblm->l = nXV = CData_GetNRecs(idTset);
  _this->m_nModelNrfeatures = nXC = CData_GetNComps(idTset)-1;
  IFCHECK
    printf("\nfound problem set of %d total sample vectors with %d input "
      "features\n",nXV,nXC);

  lpPrblm->y = (FLOAT64*)dlp_calloc(nXV,sizeof(FLOAT64));

  /* the last component is the target class */
  if (nXV != (unsigned)CData_DcompFetch(idTset,lpPrblm->y,nXC,nXV))
  {
    IERROR(_this,SVM_LABELS,"could not read all class labels in last column",0,0);
    return 0;
  }
  for (nV = 0; nV<nXV; nV++)
  {
    if ((FLOAT64)(INT32)lpPrblm->y[nV] != lpPrblm->y[nV])
    {
      IERROR(_this,SVM_LABELS,"%d: non-integer class labels not allowed: %lf\n",nV, lpPrblm->y[nV]);
      return 0;
    }
  }

  lpPrblm->x = (struct svm_node**)dlp_calloc(nXV,sizeof(struct svm_node*)); /* allocate rows */
  FLOAT64* lpVec = (FLOAT64*)dlp_calloc(nXC,sizeof(FLOAT64));
  if (!lpVec) return FALSE;
  for (nV=0; nV<nXV; nV++)
  {
    if (nXC!=(unsigned)CData_DrecFetch(idTset,lpVec,nV,nXC,-1))
    {
      IERROR(_this,SVM_CONVERT,"could not read all vector columns for sample no. ",nV,"");
      return 1;
    }
    lpPrblm->x[nV] = (svm_node*)dlp_calloc(nXC+1,sizeof(svm_node));
    if (!double_array2svm_node_array(lpVec,nXC,lpPrblm->x[nV]))
    {
      IERROR(_this,SVM_CONVERT,"could not convert all vector columns for sample no. ",nV,"");
      return 0;
    }
  }
  dlp_free(lpVec);

  return TRUE;
}



/** saves an svm_model to the instance's fields
 * @param newmodel the transient libsvm data structure to extract the new field values from
 * @return success status
 */
bool CGEN_PRIVATE CSvm_fromModel(CSvm *_this, struct svm_model* newmodel)
{
    INT32 nrsvs = 0, currentsv = 0;
    INT32 nrclasses, currentclass, currentclass2, currentmachine;

    if (newmodel == NULL)
    {
      IERROR(_this,SVM_INTERNAL,"Internal SVM data NULL",0,0);
      return FALSE;
    }
    if (newmodel->l  < 1)
    {
      IERROR(_this,SVM_INTERNAL,"Internal SVM data do not contain support vectors",0,0);
      return FALSE;
    }

    nrclasses = newmodel->nr_class;
    nrsvs = newmodel->l;
    IFIELD_RESET(CData,"model_classes");
    CData_Array(_this->m_idModelClasses, T_LONG, 1, nrclasses);
    for (currentclass = 0; currentclass < nrclasses; currentclass++)
  CData_Dstore(_this->m_idModelClasses, newmodel->label[currentclass], currentclass, 0);
  IFIELD_RESET(CData,"model_SVs");
    CData_Array(_this->m_idModelSVs, T_DOUBLE, _this->m_nModelNrfeatures + 1, nrsvs);
    CData_SetCname(_this->m_idModelSVs, _this->m_nModelNrfeatures, "LABL");
  IFIELD_RESET(CData,"model_alphas");
    CData_Array(_this->m_idModelAlphas, T_DOUBLE, nrclasses*(nrclasses-1)/2, nrsvs);

    INT32 *svstart = (INT32*) dlp_calloc(nrclasses+1, sizeof(INT32));
    FLOAT64 *fullrow = (FLOAT64*) dlp_malloc(_this->m_nModelNrfeatures * sizeof(FLOAT64));
    if (! svstart || ! fullrow) return FALSE;
    for (currentclass = 0; currentclass <= nrclasses; currentclass++)
  svstart[currentclass] = currentclass ? (svstart[currentclass-1] + newmodel->nSV[currentclass-1]) : 0;
    currentclass = 0;
    for (currentsv = 0; currentsv < nrsvs; currentsv++) {
  if (currentclass < nrclasses -1 && currentsv >= svstart[currentclass+1])
      ++currentclass;
  // store support vector itself, zero padded by the above Array()
  if (! svm_node_array2double_array(newmodel->SV[currentsv], _this->m_nModelNrfeatures, fullrow))
  {
      IERROR(_this,SVM_CONVERT, "could not convert all vector columns of support vector no. ", currentsv, "");
      return 1;
  }
  if (_this->m_nModelNrfeatures != (unsigned) CData_DrecStore(_this->m_idModelSVs, fullrow, currentsv, _this->m_nModelNrfeatures, -1))
  {
      IERROR(_this,SVM_CONVERT, "could not store all vector columns of support vector no. ", currentsv, "");
      return 1;
  }
  // store the support vectors' target class in the last component, as in the input data
  CData_Dstore(_this->m_idModelSVs, newmodel->label[currentclass], currentsv, _this->m_nModelNrfeatures);
    }
    currentmachine = 0;
    for (currentclass = 0; currentclass < nrclasses; ++currentclass)
  for (currentclass2 = currentclass + 1; currentclass2 < nrclasses; ++currentclass2)
  {
      for (currentsv = 0; currentsv < nrsvs; ++currentsv)
      {
    if (currentsv >= svstart[currentclass] && currentsv < svstart[currentclass + 1]) {
        /* classifier(i,j): store the alphas of that SV for i */
        CData_Dstore(_this->m_idModelAlphas, newmodel->sv_coef[currentclass2 - 1][currentsv], currentsv, currentmachine);
    } else if (currentsv >= svstart[currentclass2] && currentsv < svstart[currentclass2 + 1]) {
        /* classifier(i,j): store the alphas of that SV for j */
        CData_Dstore(_this->m_idModelAlphas, -1.0 * newmodel->sv_coef[currentclass][currentsv], currentsv, currentmachine);
    }
      }
      ++currentmachine;
  }


    dlp_free(svstart);
    dlp_free(fullrow);

  IFIELD_RESET(CData,"model_b");
    CData_Array(_this->m_idModelB, T_DOUBLE, 1, nrclasses*(nrclasses-1)/2);

    CData_DcompStore(_this->m_idModelB, newmodel->rho, 0, nrclasses*(nrclasses-1)/2);
    CData_Scalop_C(_this->m_idModelB, CMPLX(-1), OP_MULT, 0); // rho = -b

    if (_this->m_bProbabilities) {
      IFIELD_RESET(CData,"model_prob_A");
  CData_Array(_this->m_idModelProbA, T_DOUBLE, 1, nrclasses*(nrclasses-1)/2);

  IFIELD_RESET(CData,"model_prob_B");
  CData_Array(_this->m_idModelProbB, T_DOUBLE, 1, nrclasses*(nrclasses-1)/2);

  CData_DcompStore(_this->m_idModelProbA, newmodel->probA, 0, nrclasses*(nrclasses-1)/2);
  CData_DcompStore(_this->m_idModelProbB, newmodel->probB, 0, nrclasses*(nrclasses-1)/2);
    }

    return TRUE;
}

/** fills a struct svm_model from this instance's fields
 * @param model
 * @return success status
 */
bool CGEN_PRIVATE CSvm_toModel(CSvm *_this, struct svm_model* model)
{
  UINT32 nrsvs, currentsv,
    nrclasses, currentclass, currentclass2, currentmachine,
    nrfeatures;

  /* retrieve simple fields  _nr_class_, _l_, and _param_ */
  model->nr_class = nrclasses = CData_GetNRecs(_this->m_idModelClasses);
  model->l = nrsvs = CData_GetNRecs(_this->m_idModelSVs);
  nrfeatures = _this->m_nModelNrfeatures;
  CSvm_toParameters(_this, &model->param);

  /* retrieve double array field _label_ */
  model->label = (INT32*) dlp_calloc(nrclasses, sizeof(INT32));
  for (currentclass = 0; currentclass < nrclasses; ++currentclass)
    model->label[currentclass] = (INT32) CData_Dfetch(_this->m_idModelClasses, currentclass, 0);

  /* retrieve double array fields _biases_, _probA_, and _probB_, prepare for _nSV_ */
  model->rho = (FLOAT64*) dlp_calloc(nrclasses*(nrclasses-1)/2, sizeof(FLOAT64));
  CData_Scalop_C(_this->m_idModelB, CMPLX(-1), OP_MULT, 0); // rho = -b
  if (nrclasses*(nrclasses-1)/2 != (unsigned) CData_DcompFetch(_this->m_idModelB, model->rho, 0, nrclasses*(nrclasses-1)/2))
  {
    CData_Scalop_C(_this->m_idModelB, CMPLX(-1), OP_MULT, 0); // rho = -b
    IERROR(_this,SVM_CONVERT, "could not find all ",nrclasses*(nrclasses-1)/2," biases again");
    return 1;
  }
  CData_Scalop_C(_this->m_idModelB, CMPLX(-1), OP_MULT, 0); // rho = -b
  if (_this->m_bProbabilities)
  {
    model->probA = (FLOAT64*) dlp_calloc(nrclasses*(nrclasses-1)/2, sizeof(FLOAT64));
    if (nrclasses*(nrclasses-1)/2 != (unsigned) CData_DcompFetch(_this->m_idModelProbA, model->probA, 0, nrclasses*(nrclasses-1)/2))
    {
      IERROR(_this,SVM_CONVERT, "could not find all ",nrclasses*(nrclasses-1)/2," pdf parameters again");
      return 1;
    }
    model->probB = (FLOAT64*) dlp_calloc(nrclasses*(nrclasses-1)/2, sizeof(FLOAT64));
    if (nrclasses*(nrclasses-1)/2 != (unsigned) CData_DcompFetch(_this->m_idModelProbB, model->probB, 0, nrclasses*(nrclasses-1)/2))
    {
      IERROR(_this,SVM_CONVERT, "could not find all ",nrclasses*(nrclasses-1)/2," pdf parameters again");
      return 1;
    }
  }
  else
  {
    model->probA = NULL;
    model->probB = NULL;
  }
  model->nSV = (INT32*) dlp_calloc(nrsvs, sizeof(INT32));

  /* retrieve int array field _nSV_ in order to arrange for a redistribution of the 2dim array fields _SV_ and _sv_coef_ sorted appropriately by the LABL column of _support_vectors_ (in case they are misordered) */
  UINT32 *svclass = (UINT32*) dlp_calloc(nrsvs, sizeof(UINT32)); /* label indices by sv */
  UINT32 *svstart = (UINT32*) dlp_calloc(nrclasses, sizeof(UINT32)); /* first-index of svs by label index */
  if (! svclass || ! svstart) return FALSE;
  for (currentsv = 0; currentsv < nrsvs; currentsv++)
  {
    // find the class label of support vector currentsv and retrieve its number
    currentclass = CSvm_getClassIndex(_this, (INT32) CData_Dfetch(_this->m_idModelSVs, currentsv, nrfeatures));
/*    if (currentclass < 0) return FALSE; */ /* will never be negative */

    // contribute to nSV (number of support vectors per class)
    ++model->nSV[currentclass];
    // remember currentclass for currentsv
    svclass[currentsv] = currentclass;
    for (currentclass++; currentclass < nrclasses; currentclass++)
      ++svstart[currentclass];
  }

  /* now retrieve _SV_ (get rid of the zero value nodes) - sort in at their appropriate class block - and prepare for _sv_coef_ */
  UINT32 *svcount = (UINT32*) dlp_calloc(nrclasses, sizeof(UINT32)); /* amount of svs by label index */
  FLOAT64 *fullrow = (FLOAT64*)dlp_calloc(nrfeatures, sizeof(FLOAT64));
  if (! svcount || ! fullrow) return FALSE;
  model->SV = (svm_node**)dlp_calloc(nrsvs, sizeof(svm_node*));
  model->sv_coef = (FLOAT64**)dlp_calloc(nrclasses-1, sizeof(FLOAT64*));

  for (currentclass = 0; currentclass < nrclasses; currentclass++)
  {
    if (currentclass)
      model->sv_coef[currentclass-1] = (FLOAT64*) dlp_calloc(nrsvs, sizeof(FLOAT64));

    for (currentsv = 0; currentsv < nrsvs; currentsv++)
    {
      if (currentclass == svclass[currentsv])
      {
        /* retrieve the support vector itself */
        CData_DrecFetch(_this->m_idModelSVs, fullrow, currentsv, nrfeatures, -1);

        /* save the support vector at the end of the current classes block */
        model->SV[svstart[currentclass]+svcount[currentclass]] = (struct svm_node*)dlp_calloc(nrfeatures+1,sizeof(struct svm_node));
        if (! double_array2svm_node_array(fullrow, nrfeatures, model->SV[svstart[currentclass]+svcount[currentclass]]))
        {
          IERROR(_this,SVM_CONVERT, "could not store all vectors columns of support vector ", currentsv,"");
          return 1;
        }
        ++svcount[currentclass];
      }
    }
  }

  /* now retrieve _sv_coef_:
   * this represents classifier(i,j)'s coefficients a condensed shape
   * s.t. no zero coefficients need be stored: alphas for SVs of class i
   * will be in sv_coef[j-1][svstart[i]] up to sv_coef[j-1][svstart[i]+svcount[i]],
   * negative alphas for SVs of class j will be in sv_coef[i][svstart[j]] up to
   * sv_coef[i][svstart[j]+svcount[j]] */
  currentmachine = 0;
  for (currentclass = 0; currentclass < nrclasses; ++currentclass)
    for (currentclass2 = currentclass + 1; currentclass2 < nrclasses; ++currentclass2)
    {

      dlp_memset(svcount, 0, sizeof(UINT32)*nrclasses); /* reinit */

      for (currentsv = 0; currentsv < nrsvs; ++currentsv)
        /* save appropriate coefficients at the end of their class's block */
        if (currentclass == svclass[currentsv])
          model->sv_coef[currentclass2 - 1][svstart[currentclass]+svcount[currentclass]++] = CData_Dfetch(_this->m_idModelAlphas, currentsv, currentmachine);
        else if (currentclass2 == svclass[currentsv]){
          model->sv_coef[currentclass][svstart[currentclass2]+svcount[currentclass2]++] = -1.0 * CData_Dfetch(_this->m_idModelAlphas, currentsv, currentmachine);
	}

      /* the others are zero anyway */
      ++currentmachine;
    }

  // TODO: check return status (mistakes in sv_coef, SV ?)
  dlp_free(fullrow);
  dlp_free(svclass);
  dlp_free(svstart);
  dlp_free(svcount);

  return TRUE;
}

/** frees all memory allocated (via dlabpro code) to a struct svm_problem
 * @param prob
 */
void freeProblemSet(struct svm_problem* prob)
{
    INT32 i;
    dlp_free(prob->y);
    if (prob->x)
  for (i=0;i<prob->l;i++)
      if (prob->x[i]) dlp_free(prob->x[i]);
    dlp_free(prob->x);
}

/** frees all memory allocated (via dlabpro code) to a struct svm_param
 * @param param
 */
void freeParameters(struct svm_parameter* param)
{
    dlp_free(param->weight);
    dlp_free(param->weight_label);
}

/** frees all memory allocated (via dlabpro code) to a struct svm_model
 * @param model
 */
void freeModel(struct svm_model* model)
{
    INT32 i;
    freeParameters(&model->param);
    if (model->SV)
  for (i = 0; i < model->l; i++)
      dlp_free(model->SV[i]);
    dlp_free(model->SV);
    if (model->sv_coef)
  for (i = 0; i < model->nr_class - 1; i++)
      dlp_free(model->sv_coef[i]);
    dlp_free(model->sv_coef);
    dlp_free(model->nSV);
    dlp_free(model->rho);
    dlp_free(model->probA);
    dlp_free(model->probB);
    dlp_free(model->label);
}

struct svm_node* double_array2svm_node_array(FLOAT64 *input, UINT32 nr_elems, struct svm_node *output)
{
  UINT32 current_elem, current_node;

  if (output == (struct svm_node*)dlp_memset(output,0,nr_elems*sizeof(struct svm_node)))
  {
    current_node = 0;
    for (current_elem = 0; current_elem < nr_elems; current_elem++)
      if (input[current_elem])
      {
        output[current_node].index = current_elem + 1;
        output[current_node].value = input[current_elem];
        ++current_node;
      }
      output[current_node].index = -1;
  }
  return output;
}

FLOAT64* svm_node_array2double_array(struct svm_node* input, UINT32 nr_elems, FLOAT64 *output)
{
    UINT32 current_node = 0, nr_nodes = 0;

    while (input[current_node++].index != -1)
  ++nr_nodes;

    if (output == (FLOAT64*) dlp_memset(output, 0, nr_elems*sizeof(FLOAT64)))
  for (current_node = 0; current_node < nr_nodes; current_node++)
      output[input[current_node].index - 1] = input[current_node].value;

    return output;
}

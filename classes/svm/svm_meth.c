/* dLabPro class CSvm (svm)
 * - Methods
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

/**
 * for API documentation have a look at svm.def and ../../manual/automatic/svm.html
 */

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

/*
 * Manual page in svm.def
 */
FLOAT64 CGEN_PUBLIC CSvm_Crossvalidate(CSvm *_this, INT16 n_fold, data* trainset)
{
  if (n_fold < 2)
    return IERROR(_this,SVM_NOTALLOWED,"cross-validation must be at least 2-fold\n",0,0);

  if (BASEINST(_this)->m_nCheck) svm_set_vlv(1);

  /***** transform CData and other members into libsvm problem set and parameters ****/
  struct svm_problem prob;
  if (! CSvm_toProblemSet(_this, trainset, &prob))
  {
    freeProblemSet(&prob);
    return IERROR(_this,SVM_PARAMETERS,"cannot convert problem set\n",0,0);
  }

  struct svm_parameter param;
  CSvm_toParameters(_this, &param);

  /***** let libsvm check all parameters ******/
  const char *errmsg = svm_check_parameter(&prob, &param);
  if (errmsg)
    {
    freeParameters(&param);
    freeProblemSet(&prob);
    svm_set_vlv(0);
    return IERROR(_this,SVM_PARAMETERS,errmsg,0,0);
  }

  /***** do the cross-validation *****/
  UINT32 currentsample, nrsamples = CData_GetNRecs(trainset), nrcorrect = 0;
  FLOAT64 *target = (FLOAT64*) dlp_calloc(nrsamples, sizeof(FLOAT64));
  svm_cross_validation(&prob,&param,n_fold,target);

  /***** compute the accuracy ********/
  for (currentsample = 0; currentsample < nrsamples; ++currentsample)
    if (target[currentsample] == prob.y[currentsample])
      ++nrcorrect;

  /***** some output **************************/
  IFCHECK
    printf("finished %d-fold cross-validation from %d samples: accuracy = "
      "%.4f%%\n", n_fold, nrsamples, 100.0 * nrcorrect / nrsamples);

  dlp_free(target);
  freeParameters(&param);
  freeProblemSet(&prob);

  svm_set_vlv(0);
  return (100.0 * nrcorrect / nrsamples);
}

/*
 * Manual page in svm.def
 */
INT16 CGEN_PUBLIC CSvm_Train(CSvm *_this, data* trainset)
{
  /**** incremental or from-scratch? **********/
  if (_this->m_bIncremental && CSvm_IsTrained(_this))
    return CSvm_TrainIncremental(_this, trainset);

  /***** transform CData and other members into libsvm problem set and parameters ****/
  struct svm_problem prob;
  if (!CSvm_toProblemSet(_this, trainset, &prob))
  {
    freeProblemSet(&prob);
    return IERROR(_this,SVM_PARAMETERS,"cannot convert problem set\n",0,0);
  }

  struct svm_parameter param;
  CSvm_toParameters(_this, &param);
  if (BASEINST(_this)->m_nCheck) svm_set_vlv(1);

  /***** let libsvm check all parameters ******/
  const char *errmsg = svm_check_parameter(&prob, &param);
  if (errmsg)
  {
    freeParameters(&param);
    freeProblemSet(&prob);
    svm_set_vlv(0);
    return IERROR(_this,SVM_PARAMETERS,errmsg,0,0);
  }

  /***** train the libsvm model ***************/
  struct svm_model* newmodel = svm_train(&prob, &param);

  /***** save the model internally ************/
  CSvm_fromModel(_this, newmodel);

  /***** some output **************************/
  _this->m_nModelNrsamples = prob.l;
  IFCHECK
    printf("\nfinished training %d support vectors distributed among %d "
      "classes\n", newmodel->l, newmodel->nr_class);

  svm_destroy_model(newmodel); /* memory was allocated by libsvm code */
  freeParameters(&param);
  freeProblemSet(&prob);

  svm_set_vlv(0);
  return O_K;
}

/*
 * Manual page in svm.def
 */
INT16 CGEN_PROTECTED CSvm_TrainIncremental(CSvm *_this, CData *trainset)
{
  UINT32 nrcomps = CData_GetNComps(trainset),
    currentsample, nrsamples = CData_GetNRecs(trainset),
    currentclass, currentclass2, targetclass, nrclasses = CData_GetNRecs(_this->m_idModelClasses),
    currentmachine, nrmachines = nrclasses*(nrclasses-1)/2;

  /* verify correct dimensions */
  if (_this->m_nModelNrfeatures + 1 != nrcomps)
    return IERROR(_this,SVM_PARAMETERS, "number of features in training set does not match the previous one", 0, 0);

  /* get old model */
  struct svm_model model;
  if (! CSvm_toModel(_this, &model))
  {
    freeModel(&model);
    return NOT_EXEC;
  }

  if (BASEINST(_this)->m_nCheck) svm_set_vlv(1);

  /* prepare data instance for new training set (add support vectors) */
  CData* inctrainset = NULL;
  ICREATE(CData, inctrainset,NULL);
  CData_Copy(inctrainset, _this->m_idModelSVs);

  /* classify new data with old model (add margin-exceeding or mispredicted points to new training set) */
  struct svm_node *input = (struct svm_node*) dlp_malloc((_this->m_nModelNrfeatures+1) * sizeof(struct svm_node));
  FLOAT64 *fullrow = (FLOAT64*) dlp_malloc(sizeof(FLOAT64) * nrcomps);
  FLOAT64 *decision_values = (FLOAT64*) dlp_malloc(sizeof(FLOAT64) * nrmachines);
  BOOL addcurrent;
  IFCHECK printf("finding incremental training set with old model:\n");
  for (currentsample = 0; currentsample < nrsamples; currentsample++)
  {
    CData_DrecFetch(trainset, fullrow, currentsample, nrcomps, -1);
    if (! double_array2svm_node_array(fullrow, _this->m_nModelNrfeatures, input))
      continue;

    targetclass = CSvm_getClassIndex(_this, (INT32) fullrow[_this->m_nModelNrfeatures]);

    if ((UINT32) -1 == targetclass) // class has not occurred, yet?
    {
      addcurrent = TRUE;
      IFCHECK printf("*");
    }
    else
    {
      addcurrent = FALSE;
      svm_predict_values(&model, input, decision_values);
    }

    currentmachine = 0;
    for (currentclass = 0; currentclass < nrclasses && !addcurrent; currentclass++)
      for (currentclass2 = currentclass + 1; currentclass2 < nrclasses && !addcurrent; currentclass2++)
    if ((currentclass == targetclass && decision_values[currentmachine] < 1.0) \
      || (currentclass2 == targetclass && decision_values[currentmachine] > -1.0))
        addcurrent = TRUE;
    else
        currentmachine++;

    if (addcurrent)
    {
      /* add point to new training set */
      CData_DrecStore(inctrainset, fullrow, CData_AddRecs(inctrainset, 1, 100),
        nrcomps, -1);
      IFCHECK printf(".");
    }

  }
  IFCHECK printf("\n");
  freeModel(&model);
  dlp_free(fullrow);
  dlp_free(input);
  dlp_free(decision_values);

  /***** transform CData and other members into libsvm problem set and parameters ****/
  struct svm_problem prob;
  if (! CSvm_toProblemSet(_this, inctrainset, &prob))
  {
    freeProblemSet(&prob);
    svm_set_vlv(0);
    return IERROR(_this,SVM_PARAMETERS,"cannot convert problem set\n",0,0);
  }

  struct svm_parameter param;
  CSvm_toParameters(_this, &param);

  /***** let libsvm check all parameters ******/
  const char *errmsg = svm_check_parameter(&prob, &param);
  if (errmsg)
  {
    freeParameters(&param);
    freeProblemSet(&prob);
    svm_set_vlv(0);
    return IERROR(_this,SVM_PARAMETERS,errmsg,0,0);
  }

  /***** retrain the libsvm model ***************/
  struct svm_model* newmodel = svm_train(&prob, &param);

  /***** save the model internally ************/
  CSvm_fromModel(_this, newmodel);

  /***** some output **************************/
  _this->m_nModelNrsamples += nrsamples;
  IFCHECK
    printf("finished training %d support vectors distributed among %d "
      "classes\n", newmodel->l, newmodel->nr_class);

  svm_destroy_model(newmodel); /* memory was allocated by libsvm code */
  freeParameters(&param);
  freeProblemSet(&prob);
  IDESTROY(inctrainset);

  return O_K;
}

/*
 * Manual page in svm.def
 */
BOOL CGEN_PUBLIC CSvm_IsTrained(CSvm *_this)
{
  if (_this->m_idModelSVs == NULL) return FALSE;
  if (_this->m_idModelSVs->IsEmpty()) return FALSE;
  if (_this->m_idModelAlphas == NULL) return FALSE;
  if (_this->m_idModelAlphas->IsEmpty()) return FALSE;
  if (_this->m_idModelB == NULL) return FALSE;
  if (_this->m_idModelB->IsEmpty()) return FALSE;
  if (_this->m_idModelClasses == NULL) return FALSE;
  if (_this->m_idModelClasses->IsEmpty()) return FALSE;
  if ((unsigned) CData_GetNComps(_this->m_idModelSVs) - 1 != _this->m_nModelNrfeatures)
  {
    IERROR(_this,SVM_INCONSISTENT, "dimensions of model_SVs have changed", 0, 0);
    return FALSE;
  }
  if (CData_GetNRecs(_this->m_idModelSVs) != CData_GetNRecs(_this->m_idModelAlphas))
  {
    IERROR(_this,SVM_INCONSISTENT, "dimensions of model_SVs or model_alphas have changed", 0, 0);
    return FALSE;
  }
  if (CData_GetNComps(_this->m_idModelAlphas) != CData_GetNRecs(_this->m_idModelB))
  {
    IERROR(_this,SVM_INCONSISTENT, "dimensions of model_alphas or model_b have changed", 0, 0);
    return FALSE;
  }
  if (CData_GetNComps(_this->m_idModelAlphas) != CData_GetNRecs(_this->m_idModelClasses)*(CData_GetNRecs(_this->m_idModelClasses)-1)/2)
  {
    IERROR(_this,SVM_INCONSISTENT, "dimensions of model_alphas or model_classes have changed", 0, 0);
    return FALSE;
  }

  return TRUE;
}

/*
 * Manual page in svm.def
 */
INT16 CGEN_PUBLIC CSvm_Classify(CSvm *_this, data* testset, data* results)
{
  if (! CSvm_IsTrained(_this)) return IERROR(_this,SVM_NOMODEL, 0, 0, 0);

  struct svm_model model;
  if (!CSvm_toModel(_this, &model))
  {
    freeModel(&model);
    return NOT_EXEC;
  }

  if (BASEINST(_this)->m_nCheck) svm_set_vlv(1);

  bool supervised;
  bool allowrejects = _this->m_nParamThreshold > 0;
  INT32 target=0, output;
  char cname[5];
  INT32 nrcomps = CData_GetNComps(testset),
  currentsample, nrsamples = CData_GetNRecs(testset),
  currentclass, targetclass, outputclass, currentclass2,
  extraclass = allowrejects ? 1 : 0,   // rejected is a special class (both locally and globally)
  nrclasses = CData_GetNRecs(_this->m_idModelClasses),
  nrcorrect = 0,
  actualnrclasses = nrclasses,  // initial value, we might find more classes to be considered
  currentmachine,
  nrmachines = nrclasses*(nrclasses-1)/2;

  /* verify correct dimensions */
  if (_this->m_nModelNrfeatures == (unsigned) nrcomps)
    supervised = FALSE;
  else if (_this->m_nModelNrfeatures + 1 == (unsigned) nrcomps)
    supervised = TRUE;
  else
  {
    freeModel(&model);
    return IERROR(_this,SVM_PARAMETERS,
      "number of features in test set does not match the one in training set",
      0,0);
  }

  struct svm_node *input = (struct svm_node*) dlp_malloc((_this->m_nModelNrfeatures+1) * sizeof(struct svm_node));
  FLOAT64 *fullrow = (FLOAT64*) dlp_malloc(sizeof(FLOAT64) * nrcomps);
  FLOAT64 *prob_estimates = (FLOAT64*) dlp_malloc(sizeof(FLOAT64) * nrclasses);
#ifdef __NEW_LIBSVM_INTERFACE
  FLOAT64 *decision_values = (FLOAT64*) dlp_calloc(nrmachines, sizeof(FLOAT64));
#endif

  /* prepare the results data instance */
  CData_Reset(results, TRUE);
  if (_this->m_bProbabilities)
  {
    CData_AddComp(results, "PRED", T_INT);
    for (currentclass = 0; currentclass < nrclasses; ++currentclass)
    {
      sprintf(cname, "%2.0f", (double)CData_Dfetch(_this->m_idModelClasses, currentclass, 0));
      CData_AddComp(results, cname, T_DOUBLE);
    }
    CData_Allocate(results, nrsamples);
  }
  else CData_Array(results, T_INT, 1, nrsamples);

  if(supervised)
  {
    /* prepare the rates statistics data instance */
    if (_this->m_bNewstats) IDESTROY(_this->m_idStatRates);
    IFIELD_RESET(CData,"stat_rates");
    if
    (
      CData_GetNRecs(_this->m_idStatRates) != 8 ||
      CData_GetNComps(_this->m_idStatRates) < nrclasses+2+extraclass
    )
    {
      CData_Array(_this->m_idStatRates, T_DOUBLE, nrclasses+2+extraclass, 8);
      CData_SetCname(_this->m_idStatRates, 0, "MAVG");
      CData_SetCname(_this->m_idStatRates, 1, "mAVG");
      if (allowrejects) CData_SetCname(_this->m_idStatRates, 2, "rjct");
      for (currentclass = 0; currentclass < nrclasses; currentclass++)
        if (sprintf(cname, "%2.0f", (double)CData_Dfetch(_this->m_idModelClasses, currentclass, 0)))
          CData_SetCname(_this->m_idStatRates, currentclass+2+extraclass, cname);
    }

    /* prepare the error statistics data instance */
#ifdef __NEW_LIBSVM_INTERFACE
    if (_this->m_bNewstats) IDESTROY(_this->m_idStatErrs);
    IFIELD_RESET(CData,"stat_errs");
    if
    (
      CData_GetNRecs(_this->m_idStatErrs) < nrclasses+extraclass ||
      CData_GetNComps(_this->m_idStatErrs) < nrclasses+1+extraclass
    )
    {
      CData_Array(_this->m_idStatErrs, T_LONG, nrclasses+1+extraclass, nrclasses+extraclass);
      CData_SetCname(_this->m_idStatErrs, 0, "#pts");
      if (allowrejects) CData_SetCname(_this->m_idStatErrs, 1, "rjct");
      for (currentclass = 0; currentclass < nrclasses; currentclass++)
        if (sprintf(cname, "%2.0f", (double)CData_Dfetch(_this->m_idModelClasses, currentclass, 0)))
          CData_SetCname(_this->m_idStatErrs, currentclass+1+extraclass, cname);
    }
#endif
  }

  /* do the job */
  for (currentsample = 0; currentsample < nrsamples; currentsample++)
  {
    CData_DrecFetch(testset, fullrow, currentsample, nrcomps, -1);
    if (! double_array2svm_node_array(fullrow, _this->m_nModelNrfeatures, input))
      continue;

    if (supervised) target = (INT32) fullrow[_this->m_nModelNrfeatures];

#ifdef __NEW_LIBSVM_INTERFACE
    if (_this->m_bProbabilities)
    {
      output = (INT32) svm_predict_probability_with_values(&model, input, prob_estimates, decision_values);
      CData_DrecStore(results, prob_estimates, currentsample, nrclasses, 0);
    }
    else
      output = (INT32) svm_predict_with_values(&model, input, decision_values);
#else
    if (_this->m_bProbabilities)
    {
      output = (INT32) svm_predict_probability(&model, input, prob_estimates);
      CData_DrecStore(results, prob_estimates, currentsample, nrclasses, 0);
    }
    else
      output = (INT32) svm_predict(&model, input);
#endif

    /* save result */
    CData_Dstore(results, (FLOAT64) output, currentsample, 0);

    /* update multi-class-contingency tables */
    if (supervised)
    {
      targetclass = CSvm_getClassIndex(_this, target);
      outputclass = CSvm_getClassIndex(_this, output);

      /* was the sample rejected (global decision 0)? */
      if (0 == output && allowrejects)
      {
        outputclass = -1; // compensates to zero with "extraclass"
      }
      /* is class label unknown (misses in training set)? */
      else if (-1 == targetclass)
      {
        IERROR(_this,SVM_LABNOTFOUND,target,0,0);

        /* add to class list temporarily */
        CData_AddRecs(_this->m_idModelClasses, 1, 1);
        CData_Dstore(_this->m_idModelClasses, target, actualnrclasses, 0);
        targetclass = actualnrclasses++; // not quite exact: micro-average is becoming dirty here
        sprintf(cname, "!%ld", (long)target);

#ifdef __NEW_LIBSVM_INTERFACE
        CData_AddComp(_this->m_idStatErrs, cname, T_LONG);
        CData_AddRecs(_this->m_idStatErrs, 1, 1);
#endif
        CData_AddComp(_this->m_idStatRates, cname, T_DOUBLE);

        /* catch up with |correctly not assigned| (= nr of samples so far) */
        *(FLOAT64*)CData_XAddr(_this->m_idStatRates, 2, 2+targetclass+extraclass) = currentsample;
      }

      if (targetclass == outputclass)
      {
        /* correctly assigned targetclass */
        *(FLOAT64*)CData_XAddr(_this->m_idStatRates, 0, 2+extraclass+targetclass) += 1;
        *(FLOAT64*)CData_XAddr(_this->m_idStatRates, 0, 1) += 1.0/actualnrclasses; // micro-avg

        /* correctly not assigned others */
        for (currentclass = -extraclass; currentclass < actualnrclasses; currentclass++)
          if (currentclass != targetclass)
            *(FLOAT64*)CData_XAddr(_this->m_idStatRates, 2, 2+extraclass+currentclass) += 1;
        *(FLOAT64*)CData_XAddr(_this->m_idStatRates, 2, 1) += 1.0*(actualnrclasses-1)/actualnrclasses; // micro-avg

        /* simplistic accuracy for display (==micro-avg if actualnrclasses==nrclasses) */
        ++nrcorrect;
      }
      else
      {
        /* incorrectly assigned outputclass */
        *(FLOAT64*)CData_XAddr(_this->m_idStatRates, 1, 2+extraclass+outputclass) += 1;
        if (outputclass >= 0) // exclude rejection (not "assigned")
          *(FLOAT64*)CData_XAddr(_this->m_idStatRates, 1, 1) += 1.0/actualnrclasses; // micro-avg

        /* incorrectly not assigned targetclass */
        *(FLOAT64*)CData_XAddr(_this->m_idStatRates, 3, 2+extraclass+targetclass) += 1;
        *(FLOAT64*)CData_XAddr(_this->m_idStatRates, 3, 1) += 1.0/actualnrclasses; // micro-avg

        /* correctly not assigned others */
        for (currentclass = 0; currentclass < actualnrclasses; currentclass++)
          if (currentclass != targetclass && currentclass != outputclass)
            *(FLOAT64*)CData_XAddr(_this->m_idStatRates, 2, 2+extraclass+currentclass) += 1;
        if (outputclass >= 0) // exclude rejection (not "assigned")
          *(FLOAT64*)CData_XAddr(_this->m_idStatRates, 2, 1) += 1.0*(actualnrclasses-2)/actualnrclasses; // micro-avg
        else
          *(FLOAT64*)CData_XAddr(_this->m_idStatRates, 2, 1) += 1.0*(actualnrclasses-1)/actualnrclasses; // micro-avg

#ifdef __NEW_LIBSVM_INTERFACE
        /* increment number of mispredictions for the target class (= diagonal element on row targetclass) */
        *(INT32*)CData_XAddr(_this->m_idStatErrs, extraclass+targetclass, 1+extraclass+targetclass) += 1;

        /* increment number of false decision values at target vs. decision winner (other elements on row targetclass) */
        currentmachine = 0;
        for (currentclass = 0; currentclass < nrclasses; currentclass++)
          for (currentclass2 = currentclass + 1; currentclass2 < nrclasses; currentclass2++)
          {
            if (decision_values[currentmachine] >= (_this->m_bProbabilities ? 0 : _this->m_nParamThreshold))
            {
              /* currentclass won against currentclass2 */
              if (currentclass2 == targetclass)
              *(INT32*)CData_XAddr(_this->m_idStatErrs, targetclass+extraclass, 1+extraclass+currentclass) += 1;
            }
            else if (decision_values[currentmachine] < - (_this->m_bProbabilities ? 0 : _this->m_nParamThreshold))
            {
              /* currentclass2 won against currentclass */
              if (currentclass == targetclass)
              *(INT32*)CData_XAddr(_this->m_idStatErrs, targetclass+extraclass, 1+extraclass+currentclass2) += 1;
            }
            else
            {
              /* both classes were rejected locally */
              if (currentclass == targetclass || currentclass2 == targetclass)
              *(INT32*)CData_XAddr(_this->m_idStatErrs, targetclass+extraclass, 1) += 1;
            }
            ++currentmachine;
          }

        if (targetclass >= nrclasses)
          /* the above could not have found any pairwise winners in this case */
          *(INT32*)CData_XAddr(_this->m_idStatErrs, targetclass+extraclass, 1+extraclass+outputclass) += 1;

        if (outputclass == -1)
          /* the above could not have found the global losser due to reject */
          *(INT32*)CData_XAddr(_this->m_idStatErrs, targetclass+extraclass, 1) += 1;
#endif
      }

#ifdef __NEW_LIBSVM_INTERFACE
      /* increment number of test samples of that class */
      *(INT32*)CData_XAddr(_this->m_idStatErrs, targetclass+extraclass, 0) += 1;
#endif
    } /* endif supervised */
  } /* endfor each sample */

  /* postprocess multi-class-contingency tables */
  if (supervised)
  {
    IFCHECK
    {
      printf("number of test samples: %d\tnumber of correct classifications: "
        "%d\taccuracy: %.4f%%\n", nrsamples, nrcorrect,
        100.0*nrcorrect/nrsamples);
      if (actualnrclasses-nrclasses)
        printf("number of classes unknown to classifier: %ld\n",
        (long)(actualnrclasses-nrclasses));
      if (allowrejects)
        printf("number of samples rejected completely: %ld\n",
        (long)CData_Dfetch(_this->m_idStatRates, 1, 2));
    }

    /* update precision/recall/F-measure/error rates here */
    // for each class and micro-avg (currentclass==0)
    for (currentclass = -extraclass; currentclass <= actualnrclasses; currentclass++)
    {
      /* precision = |correctly assigned| / ( |correctly assigned| + |incorrectly assigned| ) */
      CData_Dstore(_this->m_idStatRates, CData_Dfetch(_this->m_idStatRates, 0, 1+extraclass+currentclass)/(CData_Dfetch(_this->m_idStatRates, 0, 1+extraclass+currentclass) + CData_Dfetch(_this->m_idStatRates, 1, 1+extraclass+currentclass) + 1e-7), 4, 1+extraclass+currentclass);
      /* recall = |correctly assigned| / ( |correctly assigned| + |incorrectly not assigned| ) */
      CData_Dstore(_this->m_idStatRates, CData_Dfetch(_this->m_idStatRates, 0, 1+extraclass+currentclass)/(CData_Dfetch(_this->m_idStatRates, 0, 1+extraclass+currentclass) + CData_Dfetch(_this->m_idStatRates, 3, 1+extraclass+currentclass) + 1e-7), 5, 1+extraclass+currentclass);
      /* F-measure = 2*|correctly assigned| / ( 2*|correctly assigned| + |incorrectly assigned| + |incorrectly not assigned| )
                   = 2* precision || recall = harmonic average (precision, recall) */
      CData_Dstore(_this->m_idStatRates, 2*CData_Dfetch(_this->m_idStatRates, 0, 1+extraclass+currentclass)/(2*CData_Dfetch(_this->m_idStatRates, 0, 1+extraclass+currentclass) + CData_Dfetch(_this->m_idStatRates, 1, 1+extraclass+currentclass) + CData_Dfetch(_this->m_idStatRates, 3, 1+extraclass+currentclass) + 1e-7), 6, 1+extraclass+currentclass);
      /* error = (|incorrectly assigned| + |incorrectly not assigned|)
              /( |correctly assigned| + |correctly not assigned| + |incorrectly assigned| + |incorrectly not assigned| )
       = |incorrect|/nrsamples */
      CData_Dstore(_this->m_idStatRates, (CData_Dfetch(_this->m_idStatRates, 1, 1+extraclass+currentclass) + CData_Dfetch(_this->m_idStatRates, 3, 1+extraclass+currentclass))/(nrsamples + 1e-7), 7, 1+extraclass+currentclass);
    }
    // for macro-avg
    (*(FLOAT64*)CData_XAddr(_this->m_idStatRates, 4, 0)) = 0;
    (*(FLOAT64*)CData_XAddr(_this->m_idStatRates, 5, 0)) = 0;
    (*(FLOAT64*)CData_XAddr(_this->m_idStatRates, 6, 0)) = 0;
    (*(FLOAT64*)CData_XAddr(_this->m_idStatRates, 7, 0)) = 0;
    for (currentclass = 0; currentclass < actualnrclasses; currentclass++)
    {
      // precision
      (*(FLOAT64*)CData_XAddr(_this->m_idStatRates, 4, 0)) += (CData_Dfetch(_this->m_idStatRates, 4, 2+extraclass+currentclass) / actualnrclasses);
      // recall
      (*(FLOAT64*)CData_XAddr(_this->m_idStatRates, 5, 0)) += (CData_Dfetch(_this->m_idStatRates, 5, 2+extraclass+currentclass) / actualnrclasses);
      // F-measure
      (*(FLOAT64*)CData_XAddr(_this->m_idStatRates, 6, 0)) += (CData_Dfetch(_this->m_idStatRates, 6, 2+extraclass+currentclass) / actualnrclasses);
      // error
      (*(FLOAT64*)CData_XAddr(_this->m_idStatRates, 7, 0)) += (CData_Dfetch(_this->m_idStatRates, 7, 2+extraclass+currentclass) / actualnrclasses);
    }

    IFCHECK
    {
      printf("micro-averaged rates for precision: %lf%%, recall: %lf%%, "
        "F-measure: %lf%%, error: %lf%%\n",
        100*CData_Dfetch(_this->m_idStatRates, 4, 1),
        100*CData_Dfetch(_this->m_idStatRates, 5, 1),
        100*CData_Dfetch(_this->m_idStatRates, 6, 1),
        100*CData_Dfetch(_this->m_idStatRates, 7, 1));
      printf("macro-averaged rates for precision: %lf%%, recall: %lf%%, "
        "F-measure: %lf%%, error: %lf%%\n",
        100*CData_Dfetch(_this->m_idStatRates, 4, 0),
        100*CData_Dfetch(_this->m_idStatRates, 5, 0),
        100*CData_Dfetch(_this->m_idStatRates, 6, 0),
        100*CData_Dfetch(_this->m_idStatRates, 7, 0));
      printf("(see -status for details)\n");
    }
  }

  if (actualnrclasses-nrclasses)
    CData_DeleteRecs(_this->m_idModelClasses, nrclasses, actualnrclasses-nrclasses);

  dlp_free(input);
  dlp_free(fullrow);
  dlp_free(prob_estimates);
#ifdef __NEW_LIBSVM_INTERFACE
  dlp_free(decision_values);
#endif
  freeModel(&model);

  svm_set_vlv(0);
  return O_K;
}

/*
 * Manual page in svm.def
 */
INT16 CGEN_PUBLIC CSvm_Status(CSvm *_this)
{
  UINT32 n, m, k, nrclasses, nrsvs, i;

  printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());
  printf("\n   Status of instance\n   svm %s",_this->m_lpInstanceName);
  printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());
  printf("\n\n   Settings");
  if (_this->m_idParamCWeights)
  {
    for (n = 0; n < (unsigned) CData_GetNRecs(_this->m_idParamCWeights); n++)
      printf("\n   bound/penalty param. C for class %3d    : %lf",
        (INT32) CData_Dfetch(_this->m_idParamCWeights, n, 0),
        CData_Dfetch(_this->m_idParamCWeights, n, 1) * _this->m_nParamC);
    printf("\n   bound/penalty param. C for other classes: %lg",
      _this->m_nParamC);
  }
  else
    printf("\n   bound/penalty param. C for all classes  : %lg",
      _this->m_nParamC);

  printf("\n   termination threshold parameter epsilon : %lg",
    _this->m_nParamEpsilon);

  char *gradientexplanation = (char*) dlp_malloc(80);
  if (_this->m_nParamKnlGradient)
    sprintf(gradientexplanation, "%.5f", (float)_this->m_nParamKnlGradient);
  else if (_this->m_nModelNrfeatures)
    sprintf(gradientexplanation, "%.5f", 1.0/_this->m_nModelNrfeatures);
  else
    sprintf(gradientexplanation, "1/nr_features");

  switch (_this->m_nParamKnlType)
  {
  case SVM_KT_LINEAR:
    printf("\n   kernel K(x,y) %25s : 0 (x'*y)"," ");
    break;
  case SVM_KT_POLY:
    printf("\n   kernel K(x,y) %25s : 1 (%s*x'*y + %.2f)^%d"," ",
      gradientexplanation,_this->m_nParamKnlOffset, _this->m_nParamKnlDegree);
    break;
  case SVM_KT_RBF:
    printf("\n   kernel K(x,y) %25s : 2 exp(-%s*(x-y)'*(x-y))"," ",
      gradientexplanation);
    break;
  case SVM_KT_SIGMOID:
    printf("\n   kernel K(x,y) %25s : 3 tanh(%s*x'*y + %.2f)"," ",
      gradientexplanation,_this->m_nParamKnlOffset);
    break;
  default:
    IERROR(_this,SVM_NOTALLOWED,"param_knl_type %d",_this->m_nParamKnlType,0);
    _this->m_nParamKnlType = SVM_KT_LINEAR;
  }
  dlp_free(gradientexplanation);
  gradientexplanation = NULL;

  printf("\n\n   SVM model");
  if (CSvm_IsTrained(_this))
  {
    nrclasses = CData_GetNRecs(_this->m_idModelClasses);
    nrsvs = CData_GetNRecs(_this->m_idModelSVs);

    printf("\n   Input space dim     %21s %ld",":",(long)_this->m_nModelNrfeatures);
    printf("\n   List of class labels%21s ",":");
    for (n = 0; n < nrclasses; n++)
      printf("%d ", (int) CData_Dfetch(_this->m_idModelClasses, n, 0));
    printf("\n   No. support vectors %21s %ld",":",(long)nrsvs);
    if (_this->m_nModelNrsamples)
      printf("\n   Ratio support vectors / training vectors: %4.1f %%  1)",
      (long)(100.0*nrsvs/_this->m_nModelNrsamples));

    if (!_this->m_bHardMargin && !_this->m_b2norm)
    {
      printf("\n   Bounded support vectors                 : ");
      k = 0;
      for (n = 0; n < nrsvs; n++)
      {
        /* get SV's class membership (determine class weight) */
        i = (INT32)CData_Dfetch(_this->m_idModelSVs,n,_this->m_nModelNrfeatures);

        /* count those classifier situations in which SV's alpha reaches the upper bound */
        for (m = 0; m < nrclasses*(nrclasses-1)/2; m++)
          if ((_this->m_nParamC * CSvm_getClassWeightByLabel(_this, i)) <=
            CData_Dfetch(_this->m_idModelAlphas, n, m))
          {
            k++;
          }
      }
      printf("%.2f %%  2)", 100.0*k/nrsvs/(nrclasses-1));
      /* not "k/nrsvs/(nrclasses*(nrclasses-1)/2)", because each multiclass SV
       * can only support in (nrclasses-1) binary problems
       * (its own against all other classes)
       */
    }

    printf("\n   trained for probability estimates       : %s",
      _this->m_idModelProbA ? "yes" : "no");

    printf("\n\n   1) an upper bound on the generalisation error rate");
    printf(  "\n   2) support vectors on the edge of, within, or beyond the "
             "margin region");

    // Print error statistics
    if (_this->m_idStatErrs && !CData_IsEmpty(_this->m_idStatErrs))
    {
      printf("\n\n   Error statistics   :"                                    );
      printf(  "\n   - Records          : target classes"                     );
      printf(  "\n   - Component 0      : number of test samples"             );
      printf(  "\n   - Other components : incorrectly preferred classes"      );
      printf(  "\n   - Diagonal elements: the total number of mispredicted"   );
      printf(  "\n                        samples of that target class"       );
      ISETOPTION(_this->m_idStatErrs,"/nz");
      CData_Print(_this->m_idStatErrs);
      IRESETOPTIONS(_this->m_idStatErrs);
    }

    // Print accuracy statistics
    if (_this->m_idStatRates && !CData_IsEmpty(_this->m_idStatRates))
    {
      printf(  "\n   Accuracy statistics:"                                    );
      printf(  "\n   - Records 0,1      : true/false positives"               );
      printf(  "\n   - Records 2,3      : true/false negatives"               );
      printf(  "\n   - Records 4-7      : precision, recall, F-measure, error");
      printf(  "\n   - Component 0      : macro averages"                     );
      printf(  "\n   - Component 1      : micro averages"                     );
      printf(  "\n   - other components : classes"                            );
      ISETOPTION(_this->m_idStatRates,"/nz");
      CData_Print(_this->m_idStatRates);
      IRESETOPTIONS(_this->m_idStatRates);
    }
  }
  else printf("\n   - no model trained");

  printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols()); printf("\n");

  return O_K;
}

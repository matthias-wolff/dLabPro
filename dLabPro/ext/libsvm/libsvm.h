/*
 * Copyright (c) 2000-2013 Chih-Chung Chang and Chih-Jen Lin All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither name of copyright holders nor the names of its contributors may be
 * used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * NOTE: Some changes were made to this file for integration into dLabPro.
 */

#ifndef _LIBSVM_H
#define _LIBSVM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "dlp_base.h"

struct svm_node
{
  INT32 index;
  FLOAT64 value;
};

struct svm_problem
{
  INT32 l;
  FLOAT64 *y;
  struct svm_node **x;
};

struct decision_function
{
  FLOAT64 *alpha;
  FLOAT64 rho;
};

enum { C_SVC, NU_SVC, ONE_CLASS, EPSILON_SVR, NU_SVR };  /* svm_type */
enum { LINEAR, POLY, RBF, SIGMOID };  /* kernel_type */

struct svm_parameter
{
  INT32 svm_type;
/* Robert Schubert, Oct 2005: allow application to use L2-SVM alternatively */
        INT32 softsvm_type;
  INT32 kernel_type;
  FLOAT64 degree;  /* for poly */
  FLOAT64 gamma;  /* for poly/rbf/sigmoid */
  FLOAT64 coef0;  /* for poly/sigmoid */

  /* these are for training only */
  FLOAT64 cache_size; /* in MB */
  FLOAT64 eps;  /* stopping criteria */
  FLOAT64 C;  /* for C_SVC, EPSILON_SVR and NU_SVR */
  INT32 nr_weight;    /* for C_SVC */
  INT32 *weight_label;  /* for C_SVC */
  FLOAT64* weight;    /* for C_SVC */
  FLOAT64 nu;  /* for NU_SVC, ONE_CLASS, and NU_SVR */
  FLOAT64 p;  /* for EPSILON_SVR */
  INT32 shrinking;  /* use the shrinking heuristics */
  INT32 probability; /* do probability estimates */

/* Robert Schubert, Feb 2006: allow rejection of samples */
  /* these are for prediction only */
  FLOAT64 threshold;
};

struct svm_model
{
  svm_parameter param;  // parameter
  INT32 nr_class;    // number of classes, = 2 in regression/one class svm
  INT32 l;      // total #SV
  svm_node **SV;    // SVs (SV[l])
  FLOAT64 **sv_coef;  // coefficients for SVs in decision functions (sv_coef[n-1][l])
  FLOAT64 *rho;    // constants in decision functions (rho[n*(n-1)/2])
  FLOAT64 *probA;          // pariwise probability information
  FLOAT64 *probB;

  // for classification only

  INT32 *label;    // label of each class (label[n])
  INT32 *nSV;    // number of SVs for each class (nSV[n])
        // nSV[0] + nSV[1] + ... + nSV[n-1] = l
  // XXX
  INT32 free_sv;    // 1 if svm_model is created by svm_load_model
        // 0 if svm_model is created by svm_train

};

void svm_set_vlv(INT32 nVlv);

struct svm_model *svm_train(const struct svm_problem *prob, const struct svm_parameter *param);
void svm_cross_validation(const struct svm_problem *prob, const struct svm_parameter *param, INT32 nr_fold, FLOAT64 *target);

INT32 svm_save_model(const char *model_file_name, const struct svm_model *model);
struct svm_model *svm_load_model(const char *model_file_name);

INT32 svm_get_svm_type(const struct svm_model *model);
INT32 svm_get_nr_class(const struct svm_model *model);
void svm_get_labels(const struct svm_model *model, INT32 *label);
FLOAT64 svm_get_svr_probability(const struct svm_model *model);

void svm_predict_values(const struct svm_model *model, const struct svm_node *x, FLOAT64* dec_values);
FLOAT64 svm_predict(const struct svm_model *model, const struct svm_node *x);
FLOAT64 svm_predict_probability(const struct svm_model *model, const struct svm_node *x, FLOAT64* prob_estimates);
/* Robert Schubert, Oct 2005: allow application to access decision values after prediction */
FLOAT64 svm_predict_with_values(const struct svm_model *model, const struct svm_node *x, FLOAT64 *dec_values);
FLOAT64 svm_predict_probability_with_values(const struct svm_model *model, const struct svm_node *x, FLOAT64* prob_estimates, FLOAT64 *dec_values);

void svm_destroy_model(struct svm_model *model);
void svm_destroy_param(struct svm_parameter *param);

const char *svm_check_parameter(const struct svm_problem *prob, const struct svm_parameter *param);
INT32 svm_check_probability_model(const struct svm_model *model);

#ifdef __cplusplus
}
#endif

#endif /* _LIBSVM_H */

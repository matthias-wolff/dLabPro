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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <float.h>
#include <string.h>
#include <stdarg.h>
#include "libsvm.h"
typedef FLOAT32 Qfloat;
typedef signed char schar;
#ifndef min
template <class T> inline T min(T x,T y) { return (x<y)?x:y; }
#endif
#ifndef max
template <class T> inline T max(T x,T y) { return (x>y)?x:y; }
#endif
template <class T> inline void swap(T& x, T& y) { T t=x; x=y; y=t; }
template <class S, class T> inline void clone(T*& dst, S* src, INT32 n)
{
  dst = new T[n];
  memcpy((void *)dst,(void *)src,sizeof(T)*n);
}
#define INF HUGE_VAL
#define TAU 1e-12
#define Malloc(type,n) (type *)malloc((n)*sizeof(type))

//
// MWX: dLabPro compatible info messages
//
static INT32 __nVlv = 0;

void svm_set_vlv(INT32 nVlv)
{
  __nVlv = nVlv;
}

void info(const char *fmt,...)
{
  if (__nVlv<1) return;
  va_list ap;
  va_start(ap,fmt);
  vprintf(fmt,ap);
  va_end(ap);
}

void info_flush()
{
  if (__nVlv<1) return;
  fflush(stdout);
}

//
// Kernel Cache
//
// l is the number of total data items
// size is the cache size limit in bytes
//
class Cache
{
public:
  Cache(INT32 l,INT32 size);
  ~Cache();

  // request data [0,len)
  // return some position p where [p,len) need to be filled
  // (p >= len if nothing needs to be filled)
  INT32 get_data(const INT32 index, Qfloat **data, INT32 len);
  void swap_index(INT32 i, INT32 j);  // future_option
private:
  INT32 l;
  INT32 size;
  struct head_t
  {
    head_t *prev, *next;  // a cicular list
    Qfloat *data;
    INT32 len;    // data[0,len) is cached in this entry
  };

  head_t *head;
  head_t lru_head;
  void lru_delete(head_t *h);
  void lru_insert(head_t *h);
};

Cache::Cache(INT32 l_,INT32 size_):l(l_),size(size_)
{
  head = (head_t *)calloc(l,sizeof(head_t));  // initialized to 0
  size /= sizeof(Qfloat);
  size -= l * sizeof(head_t) / sizeof(Qfloat);
  size = max(size, 2*l);  // cache must be large enough for two columns
  lru_head.next = lru_head.prev = &lru_head;
}

Cache::~Cache()
{
  for(head_t *h = lru_head.next; h != &lru_head; h=h->next)
    free(h->data);
  free(head);
}

void Cache::lru_delete(head_t *h)
{
  // delete from current location
  h->prev->next = h->next;
  h->next->prev = h->prev;
}

void Cache::lru_insert(head_t *h)
{
  // insert to last position
  h->next = &lru_head;
  h->prev = lru_head.prev;
  h->prev->next = h;
  h->next->prev = h;
}

INT32 Cache::get_data(const INT32 index, Qfloat **data, INT32 len)
{
  head_t *h = &head[index];
  if(h->len) lru_delete(h);
  INT32 more = len - h->len;

  if(more > 0)
  {
    // free old space
    while(size < more)
    {
      head_t *old = lru_head.next;
      lru_delete(old);
      free(old->data);
      size += old->len;
      old->data = 0;
      old->len = 0;
    }

    // allocate new space
    h->data = (Qfloat *)realloc(h->data,sizeof(Qfloat)*len);
    size -= more;
    swap(h->len,len);
  }

  lru_insert(h);
  *data = h->data;
  return len;
}

void Cache::swap_index(INT32 i, INT32 j)
{
  if(i==j) return;

  if(head[i].len) lru_delete(&head[i]);
  if(head[j].len) lru_delete(&head[j]);
  swap(head[i].data,head[j].data);
  swap(head[i].len,head[j].len);
  if(head[i].len) lru_insert(&head[i]);
  if(head[j].len) lru_insert(&head[j]);

  if(i>j) swap(i,j);
  for(head_t *h = lru_head.next; h!=&lru_head; h=h->next)
  {
    if(h->len > i)
    {
      if(h->len > j)
        swap(h->data[i],h->data[j]);
      else
      {
        // give up
        lru_delete(h);
        free(h->data);
        size += h->len;
        h->data = 0;
        h->len = 0;
      }
    }
  }
}

//
// Kernel evaluation
//
// the static method k_function is for doing single kernel evaluation
// the constructor of Kernel prepares to calculate the l*l kernel matrix
// the member function get_Q is for getting one column from the Q Matrix
//
class QMatrix {
public:
  virtual Qfloat *get_Q(INT32 column, INT32 len) const = 0;
  virtual Qfloat *get_QD() const = 0;
  virtual void swap_index(INT32 i, INT32 j) const = 0;
};

class Kernel: public QMatrix {
public:
  Kernel(INT32 l, svm_node * const * x, const svm_parameter& param);
  virtual ~Kernel();

  static FLOAT64 k_function(const svm_node *x, const svm_node *y,
         const svm_parameter& param);
  virtual Qfloat *get_Q(INT32 column, INT32 len) const = 0;
  virtual Qfloat *get_QD() const = 0;
  virtual void swap_index(INT32 i, INT32 j) const  // no so const...
  {
    swap(x[i],x[j]);
    if(x_square) swap(x_square[i],x_square[j]);
  }
protected:

  FLOAT64 (Kernel::*kernel_function)(INT32 i, INT32 j) const;

private:
  const svm_node **x;
  FLOAT64 *x_square;

  // svm_parameter
  const INT32 kernel_type;
  const FLOAT64 degree;
  const FLOAT64 gamma;
  const FLOAT64 coef0;

  static FLOAT64 dot(const svm_node *px, const svm_node *py);
  FLOAT64 kernel_linear(INT32 i, INT32 j) const
  {
    return dot(x[i],x[j]);
  }
  FLOAT64 kernel_poly(INT32 i, INT32 j) const
  {
    return pow(gamma*dot(x[i],x[j])+coef0,degree);
  }
  FLOAT64 kernel_rbf(INT32 i, INT32 j) const
  {
    return exp(-gamma*(x_square[i]+x_square[j]-2*dot(x[i],x[j])));
  }
  FLOAT64 kernel_sigmoid(INT32 i, INT32 j) const
  {
    return tanh(gamma*dot(x[i],x[j])+coef0);
  }
};

Kernel::Kernel(INT32 l, svm_node * const * x_, const svm_parameter& param)
:kernel_type(param.kernel_type), degree(param.degree),
 gamma(param.gamma), coef0(param.coef0)
{
  switch(kernel_type)
  {
    case LINEAR:
      kernel_function = &Kernel::kernel_linear;
      break;
    case POLY:
      kernel_function = &Kernel::kernel_poly;
      break;
    case RBF:
      kernel_function = &Kernel::kernel_rbf;
      break;
    case SIGMOID:
      kernel_function = &Kernel::kernel_sigmoid;
      break;
  }

  clone(x,x_,l);

  if(kernel_type == RBF)
  {
    x_square = new FLOAT64[l];
    for(INT32 i=0;i<l;i++)
      x_square[i] = dot(x[i],x[i]);
  }
  else
    x_square = 0;
}

Kernel::~Kernel()
{
  delete[] x;
  delete[] x_square;
}

FLOAT64 Kernel::dot(const svm_node *px, const svm_node *py)
{
  FLOAT64 sum = 0;
  while(px->index != -1 && py->index != -1)
  {
    if(px->index == py->index)
    {
      sum += px->value * py->value;
      ++px;
      ++py;
    }
    else
    {
      if(px->index > py->index)
        ++py;
      else
        ++px;
    }      
  }
  return sum;
}

FLOAT64 Kernel::k_function(const svm_node *x, const svm_node *y,
        const svm_parameter& param)
{
  switch(param.kernel_type)
  {
    case LINEAR:
      return dot(x,y);
    case POLY:
      return pow(param.gamma*dot(x,y)+param.coef0,param.degree);
    case RBF:
    {
      FLOAT64 sum = 0;
      while(x->index != -1 && y->index !=-1)
      {
        if(x->index == y->index)
        {
          FLOAT64 d = x->value - y->value;
          sum += d*d;
          ++x;
          ++y;
        }
        else
        {
          if(x->index > y->index)
          {  
            sum += y->value * y->value;
            ++y;
          }
          else
          {
            sum += x->value * x->value;
            ++x;
          }
        }
      }

      while(x->index != -1)
      {
        sum += x->value * x->value;
        ++x;
      }

      while(y->index != -1)
      {
        sum += y->value * y->value;
        ++y;
      }
      
      return exp(-param.gamma*sum);
    }
    case SIGMOID:
      return tanh(param.gamma*dot(x,y)+param.coef0);
    default:
      return 0;  /* Unreachable */
  }
}

// Generalized SMO+SVMlight algorithm
// Solves:
//
//  min 0.5(\alpha^T Q \alpha) + b^T \alpha
//
//    y^T \alpha = \delta
//    y_i = +1 or -1
//    0 <= alpha_i <= Cp for y_i = 1
//    0 <= alpha_i <= Cn for y_i = -1
//
// Given:
//
//  Q, b, y, Cp, Cn, and an initial feasible point \alpha
//  l is the size of vectors and matrices
//  eps is the stopping criterion
//
// solution will be put in \alpha, objective value will be put in obj
//
class Solver {
public:
  Solver() {};
  virtual ~Solver() {};

  struct SolutionInfo {
    FLOAT64 obj;
    FLOAT64 rho;
    FLOAT64 upper_bound_p;
    FLOAT64 upper_bound_n;
          FLOAT64 margin;
    FLOAT64 r;  // for Solver_NU
  };

  void Solve(INT32 l, const QMatrix& Q, const FLOAT64 *b_, const schar *y_,
       FLOAT64 *alpha_, FLOAT64 Cp, FLOAT64 Cn, FLOAT64 eps,
       SolutionInfo* si, INT32 shrinking);
protected:
  INT32 active_size;
  schar *y;
  FLOAT64 *G;    // gradient of objective function
  enum { LOWER_BOUND, UPPER_BOUND, FREE };
  char *alpha_status;  // LOWER_BOUND, UPPER_BOUND, FREE
  FLOAT64 *alpha;
  const QMatrix *Q;
  const Qfloat *QD;
  FLOAT64 eps;
  FLOAT64 Cp,Cn;
  FLOAT64 *b;
  INT32 *active_set;
  FLOAT64 *G_bar;    // gradient, if we treat free variables as 0
  INT32 l;
  bool unshrinked;  // XXX

  FLOAT64 get_C(INT32 i)
  {
    return (y[i] > 0)? Cp : Cn;
  }
  void update_alpha_status(INT32 i)
  {
    if(alpha[i] >= get_C(i))
      alpha_status[i] = UPPER_BOUND;
    else if(alpha[i] <= 0)
      alpha_status[i] = LOWER_BOUND;
    else alpha_status[i] = FREE;
  }
  bool is_upper_bound(INT32 i) { return alpha_status[i] == UPPER_BOUND; }
  bool is_lower_bound(INT32 i) { return alpha_status[i] == LOWER_BOUND; }
  bool is_free(INT32 i) { return alpha_status[i] == FREE; }
  void swap_index(INT32 i, INT32 j);
  void reconstruct_gradient();
  virtual INT32 select_working_set(INT32 &i, INT32 &j);
  virtual INT32 max_violating_pair(INT32 &i, INT32 &j);
  virtual FLOAT64 calculate_rho();
  virtual void do_shrinking();
};

void Solver::swap_index(INT32 i, INT32 j)
{
  Q->swap_index(i,j);
  swap(y[i],y[j]);
  swap(G[i],G[j]);
  swap(alpha_status[i],alpha_status[j]);
  swap(alpha[i],alpha[j]);
  swap(b[i],b[j]);
  swap(active_set[i],active_set[j]);
  swap(G_bar[i],G_bar[j]);
}

void Solver::reconstruct_gradient()
{
  // reconstruct inactive elements of G from G_bar and free variables

  if(active_size == l) return;

  INT32 i;
  for(i=active_size;i<l;i++)
    G[i] = G_bar[i] + b[i];
  
  for(i=0;i<active_size;i++)
    if(is_free(i))
    {
      const Qfloat *Q_i = Q->get_Q(i,l);
      FLOAT64 alpha_i = alpha[i];
      for(INT32 j=active_size;j<l;j++)
        G[j] += alpha_i * Q_i[j];
    }
}

void Solver::Solve(INT32 l, const QMatrix& Q, const FLOAT64 *b_, const schar *y_,
       FLOAT64 *alpha_, FLOAT64 Cp, FLOAT64 Cn, FLOAT64 eps,
       SolutionInfo* si, INT32 shrinking)
{
  this->l = l;
  this->Q = &Q;
  QD=Q.get_QD();
  clone(b, b_,l);
  clone(y, y_,l);
  clone(alpha,alpha_,l);
  this->Cp = Cp;
  this->Cn = Cn;
  this->eps = eps;
  unshrinked = false;

  // initialize alpha_status
  {
    alpha_status = new char[l];
    for(INT32 i=0;i<l;i++)
      update_alpha_status(i);
  }

  // initialize active set (for shrinking)
  {
    active_set = new INT32[l];
    for(INT32 i=0;i<l;i++)
      active_set[i] = i;
    active_size = l;
  }

  // initialize gradient
  {
    G = new FLOAT64[l];
    G_bar = new FLOAT64[l];
    INT32 i;
    for(i=0;i<l;i++)
    {
      G[i] = b[i];
      G_bar[i] = 0;
    }
    for(i=0;i<l;i++)
      if(!is_lower_bound(i))
      {
        const Qfloat *Q_i = Q.get_Q(i,l);
        FLOAT64 alpha_i = alpha[i];
        INT32 j;
        for(j=0;j<l;j++)
          G[j] += alpha_i*Q_i[j];
        if(is_upper_bound(i))
          for(j=0;j<l;j++)
            G_bar[j] += get_C(i) * Q_i[j];
      }
  }

  // optimization step

  INT32 iter = 0;
  INT32 counter = min((int)l,1000)+1;

  while(1)
  {
    // show progress and do shrinking

    if(--counter == 0)
    {
      counter = min((int)l,1000);
      if(shrinking) do_shrinking();
      info("."); info_flush();
    }

    INT32 i,j;
    if(select_working_set(i,j)!=0)
    {
      // reconstruct the whole gradient
      reconstruct_gradient();
      // reset active set size and check
      active_size = l;
      info("*"); info_flush();
      if(select_working_set(i,j)!=0)
        break;
      else
        counter = 1;  // do shrinking next iteration
    }
    
    ++iter;

    // update alpha[i] and alpha[j], handle bounds carefully
    
    const Qfloat *Q_i = Q.get_Q(i,active_size);
    const Qfloat *Q_j = Q.get_Q(j,active_size);

    FLOAT64 C_i = get_C(i);
    FLOAT64 C_j = get_C(j);

    FLOAT64 old_alpha_i = alpha[i];
    FLOAT64 old_alpha_j = alpha[j];

    if(y[i]!=y[j])
    {
      FLOAT64 quad_coef = Q_i[i]+Q_j[j]+2*Q_i[j];
      if (quad_coef <= 0)
        quad_coef = TAU;
      FLOAT64 delta = (-G[i]-G[j])/quad_coef;
      FLOAT64 diff = alpha[i] - alpha[j];
      alpha[i] += delta;
      alpha[j] += delta;
      
      if(diff > 0)
      {
        if(alpha[j] < 0)
        {
          alpha[j] = 0;
          alpha[i] = diff;
        }
      }
      else
      {
        if(alpha[i] < 0)
        {
          alpha[i] = 0;
          alpha[j] = -diff;
        }
      }
      if(diff > C_i - C_j)
      {
        if(alpha[i] > C_i)
        {
          alpha[i] = C_i;
          alpha[j] = C_i - diff;
        }
      }
      else
      {
        if(alpha[j] > C_j)
        {
          alpha[j] = C_j;
          alpha[i] = C_j + diff;
        }
      }
    }
    else
    {
      FLOAT64 quad_coef = Q_i[i]+Q_j[j]-2*Q_i[j];
      if (quad_coef <= 0)
        quad_coef = TAU;
      FLOAT64 delta = (G[i]-G[j])/quad_coef;
      FLOAT64 sum = alpha[i] + alpha[j];
      alpha[i] -= delta;
      alpha[j] += delta;

      if(sum > C_i)
      {
        if(alpha[i] > C_i)
        {
          alpha[i] = C_i;
          alpha[j] = sum - C_i;
        }
      }
      else
      {
        if(alpha[j] < 0)
        {
          alpha[j] = 0;
          alpha[i] = sum;
        }
      }
      if(sum > C_j)
      {
        if(alpha[j] > C_j)
        {
          alpha[j] = C_j;
          alpha[i] = sum - C_j;
        }
      }
      else
      {
        if(alpha[i] < 0)
        {
          alpha[i] = 0;
          alpha[j] = sum;
        }
      }
    }

    // update G

    FLOAT64 delta_alpha_i = alpha[i] - old_alpha_i;
    FLOAT64 delta_alpha_j = alpha[j] - old_alpha_j;
    
    for(INT32 k=0;k<active_size;k++)
    {
      G[k] += Q_i[k]*delta_alpha_i + Q_j[k]*delta_alpha_j;
    }

    // update alpha_status and G_bar

    {
      bool ui = is_upper_bound(i);
      bool uj = is_upper_bound(j);
      update_alpha_status(i);
      update_alpha_status(j);
      INT32 k;
      if(ui != is_upper_bound(i))
      {
        Q_i = Q.get_Q(i,l);
        if(ui)
          for(k=0;k<l;k++)
            G_bar[k] -= C_i * Q_i[k];
        else
          for(k=0;k<l;k++)
            G_bar[k] += C_i * Q_i[k];
      }

      if(uj != is_upper_bound(j))
      {
        Q_j = Q.get_Q(j,l);
        if(uj)
          for(k=0;k<l;k++)
            G_bar[k] -= C_j * Q_j[k];
        else
          for(k=0;k<l;k++)
            G_bar[k] += C_j * Q_j[k];
      }
    }
  }

  // calculate rho

  si->rho = calculate_rho();

  // calculate objective value
  {
    FLOAT64 v = 0;
    FLOAT64 w = 0;
    INT32 i;
    for(i=0;i<l;i++)
    {
      v += alpha[i] * (G[i] + b[i]);
      w += alpha[i] * (G[i] - b[i]);
    }

          si->obj = v/2;
/* V--- Robert Schubert, Oct 2005: calculate resulting margin along with objective value */
    si->margin = 1.0/sqrt(fabs(w)); // gm = 1/|w| = 1/sqrt(w^T w) = 1/sqrt(alpha^T Q alpha) 
/* ^--- */
  }

  // put back the solution
  {
    for(INT32 i=0;i<l;i++)
      alpha_[active_set[i]] = alpha[i];
  }

  // juggle everything back
  /*{
    for(INT32 i=0;i<l;i++)
      while(active_set[i] != i)
        swap_index(i,active_set[i]);
        // or Q.swap_index(i,active_set[i]);
  }*/

  si->upper_bound_p = Cp;
  si->upper_bound_n = Cn;

  info("\noptimization finished, #iter = %d\n",iter);

  delete[] b;
  delete[] y;
  delete[] alpha;
  delete[] alpha_status;
  delete[] active_set;
  delete[] G;
  delete[] G_bar;
}

// return 1 if already optimal, return 0 otherwise
INT32 Solver::select_working_set(INT32 &out_i, INT32 &out_j)
{
  // return i,j such that
  // i: maximizes -y_i * grad(f)_i, i in I_up(\alpha)
  // j: minimizes the decrease of obj value
  //    (if quadratic coefficeint <= 0, replace it with tau)
  //    -y_j*grad(f)_j < -y_i*grad(f)_i, j in I_low(\alpha)
  
  FLOAT64 Gmax = -INF;
  INT32 Gmax_idx = -1;
  INT32 Gmin_idx = -1;
  FLOAT64 obj_diff_min = INF;

  for(INT32 t=0;t<active_size;t++)
    if(y[t]==+1)  
    {
      if(!is_upper_bound(t))
        if(-G[t] >= Gmax)
        {
          Gmax = -G[t];
          Gmax_idx = t;
        }
    }
    else
    {
      if(!is_lower_bound(t))
        if(G[t] >= Gmax)
        {
          Gmax = G[t];
          Gmax_idx = t;
        }
    }

  INT32 i = Gmax_idx;
  const Qfloat *Q_i = NULL;
  if(i != -1) // NULL Q_i not accessed: Gmax=-INF if i=-1
    Q_i = Q->get_Q(i,active_size);

  for(INT32 j=0;j<active_size;j++)
  {
    if(y[j]==+1)
    {
      if (!is_lower_bound(j))
      {
        FLOAT64 grad_diff=Gmax+G[j];
        if (grad_diff >= eps)
        {
          FLOAT64 obj_diff;
          FLOAT64 quad_coef=Q_i[i]+QD[j]-2*y[i]*Q_i[j];
          if (quad_coef > 0)
            obj_diff = -(grad_diff*grad_diff)/quad_coef;
          else
            obj_diff = -(grad_diff*grad_diff)/TAU;

          if (obj_diff <= obj_diff_min)
          {
            Gmin_idx=j;
            obj_diff_min = obj_diff;
          }
        }
      }
    }
    else
    {
      if (!is_upper_bound(j))
      {
        FLOAT64 grad_diff= Gmax-G[j];
        if (grad_diff >= eps)
        {
          FLOAT64 obj_diff;
          FLOAT64 quad_coef=Q_i[i]+QD[j]+2*y[i]*Q_i[j];
          if (quad_coef > 0)
            obj_diff = -(grad_diff*grad_diff)/quad_coef;
          else
            obj_diff = -(grad_diff*grad_diff)/TAU;

          if (obj_diff <= obj_diff_min)
          {
            Gmin_idx=j;
            obj_diff_min = obj_diff;
          }
        }
      }
    }
  }

  if(Gmin_idx == -1)
     return 1;

  out_i = Gmax_idx;
  out_j = Gmin_idx;
  return 0;
}

// return 1 if already optimal, return 0 otherwise
INT32 Solver::max_violating_pair(INT32 &out_i, INT32 &out_j)
{
  // return i,j: maximal violating pair

  FLOAT64 Gmax1 = -INF;    // max { -y_i * grad(f)_i | i in I_up(\alpha) }
  INT32 Gmax1_idx = -1;

  FLOAT64 Gmax2 = -INF;    // max { y_i * grad(f)_i | i in I_low(\alpha) }
  INT32 Gmax2_idx = -1;

  for(INT32 i=0;i<active_size;i++)
  {
    if(y[i]==+1)  // y = +1
    {
      if(!is_upper_bound(i))  // d = +1
      {
        if(-G[i] >= Gmax1)
        {
          Gmax1 = -G[i];
          Gmax1_idx = i;
        }
      }
      if(!is_lower_bound(i))  // d = -1
      {
        if(G[i] >= Gmax2)
        {
          Gmax2 = G[i];
          Gmax2_idx = i;
        }
      }
    }
    else    // y = -1
    {
      if(!is_upper_bound(i))  // d = +1
      {
        if(-G[i] >= Gmax2)
        {
          Gmax2 = -G[i];
          Gmax2_idx = i;
        }
      }
      if(!is_lower_bound(i))  // d = -1
      {
        if(G[i] >= Gmax1)
        {
          Gmax1 = G[i];
          Gmax1_idx = i;
        }
      }
    }
  }

  if(Gmax1+Gmax2 < eps)
     return 1;

  out_i = Gmax1_idx;
  out_j = Gmax2_idx;
  return 0;
}

void Solver::do_shrinking()
{
  INT32 i,j,k;
  if(max_violating_pair(i,j)!=0) return;
  FLOAT64 Gm1 = -y[j]*G[j];
  FLOAT64 Gm2 = y[i]*G[i];

  // shrink
  
  for(k=0;k<active_size;k++)
  {
    if(is_lower_bound(k))
    {
      if(y[k]==+1)
      {
        if(-G[k] >= Gm1) continue;
      }
      else  if(-G[k] >= Gm2) continue;
    }
    else if(is_upper_bound(k))
    {
      if(y[k]==+1)
      {
        if(G[k] >= Gm2) continue;
      }
      else  if(G[k] >= Gm1) continue;
    }
    else continue;

    --active_size;
    swap_index(k,active_size);
    --k;  // look at the newcomer
  }

  // unshrink, check all variables again before final iterations

  if(unshrinked || -(Gm1 + Gm2) > eps*10) return;
  
  unshrinked = true;
  reconstruct_gradient();

  for(k=l-1;k>=active_size;k--)
  {
    if(is_lower_bound(k))
    {
      if(y[k]==+1)
      {
        if(-G[k] < Gm1) continue;
      }
      else  if(-G[k] < Gm2) continue;
    }
    else if(is_upper_bound(k))
    {
      if(y[k]==+1)
      {
        if(G[k] < Gm2) continue;
      }
      else  if(G[k] < Gm1) continue;
    }
    else continue;

    swap_index(k,active_size);
    active_size++;
    ++k;  // look at the newcomer
  }
}

FLOAT64 Solver::calculate_rho()
{
  FLOAT64 r;
  INT32 nr_free = 0;
  FLOAT64 ub = INF, lb = -INF, sum_free = 0;
  for(INT32 i=0;i<active_size;i++)
  {
    FLOAT64 yG = y[i]*G[i];

    if(is_lower_bound(i))
    {
      if(y[i] > 0)
        ub = min(ub,yG);
      else
        lb = max(lb,yG);
    }
    else if(is_upper_bound(i))
    {
      if(y[i] < 0)
        ub = min(ub,yG);
      else
        lb = max(lb,yG);
    }
    else
    {
      ++nr_free;
      sum_free += yG;
    }
  }

  if(nr_free>0)
    r = sum_free/nr_free;
  else
    r = (ub+lb)/2;

  return r;
}

//
// Solver for nu-svm classification and regression
//
// additional constraint: e^T \alpha = constant
//
class Solver_NU : public Solver
{
public:
  Solver_NU() {}
  void Solve(INT32 l, const QMatrix& Q, const FLOAT64 *b, const schar *y,
       FLOAT64 *alpha, FLOAT64 Cp, FLOAT64 Cn, FLOAT64 eps,
       SolutionInfo* si, INT32 shrinking)
  {
    this->si = si;
    Solver::Solve(l,Q,b,y,alpha,Cp,Cn,eps,si,shrinking);
  }
private:
  SolutionInfo *si;
  INT32 select_working_set(INT32 &i, INT32 &j);
  FLOAT64 calculate_rho();
  void do_shrinking();
};

// return 1 if already optimal, return 0 otherwise
INT32 Solver_NU::select_working_set(INT32 &out_i, INT32 &out_j)
{
  // return i,j such that y_i = y_j and
  // i: maximizes -y_i * grad(f)_i, i in I_up(\alpha)
  // j: minimizes the decrease of obj value
  //    (if quadratic coefficeint <= 0, replace it with tau)
  //    -y_j*grad(f)_j < -y_i*grad(f)_i, j in I_low(\alpha)

  FLOAT64 Gmaxp = -INF;
  INT32 Gmaxp_idx = -1;

  FLOAT64 Gmaxn = -INF;
  INT32 Gmaxn_idx = -1;

  INT32 Gmin_idx = -1;
  FLOAT64 obj_diff_min = INF;

  for(INT32 t=0;t<active_size;t++)
    if(y[t]==+1)
    {
      if(!is_upper_bound(t))
        if(-G[t] >= Gmaxp)
        {
          Gmaxp = -G[t];
          Gmaxp_idx = t;
        }
    }
    else
    {
      if(!is_lower_bound(t))
        if(G[t] >= Gmaxn)
        {
          Gmaxn = G[t];
          Gmaxn_idx = t;
        }
    }

  INT32 ip = Gmaxp_idx;
  INT32 in = Gmaxn_idx;
  const Qfloat *Q_ip = NULL;
  const Qfloat *Q_in = NULL;
  if(ip != -1) // NULL Q_ip not accessed: Gmaxp=-INF if ip=-1
    Q_ip = Q->get_Q(ip,active_size);
  if(in != -1)
    Q_in = Q->get_Q(in,active_size);

  for(INT32 j=0;j<active_size;j++)
  {
    if(y[j]==+1)
    {
      if (!is_lower_bound(j))  
      {
        FLOAT64 grad_diff=Gmaxp+G[j];
        if (grad_diff >= eps)
        {
          FLOAT64 obj_diff;
          FLOAT64 quad_coef = Q_ip[ip]+QD[j]-2*Q_ip[j];
          if (quad_coef > 0)
            obj_diff = -(grad_diff*grad_diff)/quad_coef;
          else
            obj_diff = -(grad_diff*grad_diff)/TAU;

          if (obj_diff <= obj_diff_min)
          {
            Gmin_idx=j;
            obj_diff_min = obj_diff;
          }
        }
      }
    }
    else
    {
      if (!is_upper_bound(j))
      {
        FLOAT64 grad_diff=Gmaxn-G[j];
        if (grad_diff >= eps)
        {
          FLOAT64 obj_diff;
          FLOAT64 quad_coef = Q_in[in]+QD[j]-2*Q_in[j];
          if (quad_coef > 0)
            obj_diff = -(grad_diff*grad_diff)/quad_coef;
          else
            obj_diff = -(grad_diff*grad_diff)/TAU;

          if (obj_diff <= obj_diff_min)
          {
            Gmin_idx=j;
            obj_diff_min = obj_diff;
          }
        }
      }
    }
  }

  if(Gmin_idx == -1)
     return 1;

  if (y[Gmin_idx] == +1)
    out_i = Gmaxp_idx;
  else
    out_i = Gmaxn_idx;
  out_j = Gmin_idx;

  return 0;
}

void Solver_NU::do_shrinking()
{
  FLOAT64 Gmax1 = -INF;  // max { -y_i * grad(f)_i | y_i = +1, i in I_up(\alpha) }
  FLOAT64 Gmax2 = -INF;  // max { y_i * grad(f)_i | y_i = +1, i in I_low(\alpha) }
  FLOAT64 Gmax3 = -INF;  // max { -y_i * grad(f)_i | y_i = -1, i in I_up(\alpha) }
  FLOAT64 Gmax4 = -INF;  // max { y_i * grad(f)_i | y_i = -1, i in I_low(\alpha) }

  // find maximal violating pair first
  INT32 k;
  for(k=0;k<active_size;k++)
  {
    if(!is_upper_bound(k))
    {
      if(y[k]==+1)
      {
        if(-G[k] > Gmax1) Gmax1 = -G[k];
      }
      else  if(-G[k] > Gmax3) Gmax3 = -G[k];
    }
    if(!is_lower_bound(k))
    {
      if(y[k]==+1)
      {  
        if(G[k] > Gmax2) Gmax2 = G[k];
      }
      else  if(G[k] > Gmax4) Gmax4 = G[k];
    }
  }

  // shrinking

  FLOAT64 Gm1 = -Gmax2;
  FLOAT64 Gm2 = -Gmax1;
  FLOAT64 Gm3 = -Gmax4;
  FLOAT64 Gm4 = -Gmax3;

  for(k=0;k<active_size;k++)
  {
    if(is_lower_bound(k))
    {
      if(y[k]==+1)
      {
        if(-G[k] >= Gm1) continue;
      }
      else  if(-G[k] >= Gm3) continue;
    }
    else if(is_upper_bound(k))
    {
      if(y[k]==+1)
      {
        if(G[k] >= Gm2) continue;
      }
      else  if(G[k] >= Gm4) continue;
    }
    else continue;

    --active_size;
    swap_index(k,active_size);
    --k;  // look at the newcomer
  }

  // unshrink, check all variables again before final iterations

  if(unshrinked || max(-(Gm1+Gm2),-(Gm3+Gm4)) > eps*10) return;
  
  unshrinked = true;
  reconstruct_gradient();

  for(k=l-1;k>=active_size;k--)
  {
    if(is_lower_bound(k))
    {
      if(y[k]==+1)
      {
        if(-G[k] < Gm1) continue;
      }
      else  if(-G[k] < Gm3) continue;
    }
    else if(is_upper_bound(k))
    {
      if(y[k]==+1)
      {
        if(G[k] < Gm2) continue;
      }
      else  if(G[k] < Gm4) continue;
    }
    else continue;

    swap_index(k,active_size);
    active_size++;
    ++k;  // look at the newcomer
  }
}

FLOAT64 Solver_NU::calculate_rho()
{
  INT32 nr_free1 = 0,nr_free2 = 0;
  FLOAT64 ub1 = INF, ub2 = INF;
  FLOAT64 lb1 = -INF, lb2 = -INF;
  FLOAT64 sum_free1 = 0, sum_free2 = 0;

  for(INT32 i=0;i<active_size;i++)
  {
    if(y[i]==+1)
    {
      if(is_lower_bound(i))
        ub1 = min(ub1,G[i]);
      else if(is_upper_bound(i))
        lb1 = max(lb1,G[i]);
      else
      {
        ++nr_free1;
        sum_free1 += G[i];
      }
    }
    else
    {
      if(is_lower_bound(i))
        ub2 = min(ub2,G[i]);
      else if(is_upper_bound(i))
        lb2 = max(lb2,G[i]);
      else
      {
        ++nr_free2;
        sum_free2 += G[i];
      }
    }
  }

  FLOAT64 r1,r2;
  if(nr_free1 > 0)
    r1 = sum_free1/nr_free1;
  else
    r1 = (ub1+lb1)/2;
  
  if(nr_free2 > 0)
    r2 = sum_free2/nr_free2;
  else
    r2 = (ub2+lb2)/2;
  
  si->r = (r1+r2)/2;
  return (r1-r2)/2;
}

//
// Q matrices for various formulations
//
class SVC_Q: public Kernel
{ 
public:
  SVC_Q(const svm_problem& prob, const svm_parameter& param, const schar *y_)
  :Kernel(prob.l, prob.x, param)
  {
    clone(y,y_,prob.l);
/* V--- Robert Schubert, Oct 2005: allow application to use L2-SVM alternatively */
    this->C = param.C;
    if (param.softsvm_type == 2) this->l2norm = true;
    else this->l2norm = false;
/* ^--- */
    cache = new Cache(prob.l,(INT32)(param.cache_size*(1<<20)));
    QD = new Qfloat[prob.l];
    for(INT32 i=0;i<prob.l;i++)
      QD[i]= (Qfloat)(this->*kernel_function)(i,i);
  }
  
  Qfloat *get_Q(INT32 i, INT32 len) const
  {
    Qfloat *data;
    INT32 start;
    if((start = cache->get_data(i,&data,len)) < len)
    {
      for(INT32 j=start;j<len;j++)
        data[j] = (Qfloat)(y[i]*y[j]*(this->*kernel_function)(i,j));
/* Robert Schubert, Oct 2005: allow application to use L2-SVM alternatively */
      if (l2norm && i >= start && i < len)
          data[i] += 1/C;
/* ^--- */
    }
    return data;
  }

  Qfloat *get_QD() const
  {
    return QD;
  }

  void swap_index(INT32 i, INT32 j) const
  {
    cache->swap_index(i,j);
    Kernel::swap_index(i,j);
    swap(y[i],y[j]);
    swap(QD[i],QD[j]);
  }

  ~SVC_Q()
  {
    delete[] y;
    delete cache;
    delete[] QD;
  }
private:
/* Robert Schubert, Oct 2005: allow application to use L2-SVM alternatively */
        FLOAT64 C;
        bool l2norm;
/* ^--- */
  schar *y;
  Cache *cache;
  Qfloat *QD;
};

class ONE_CLASS_Q: public Kernel
{
public:
  ONE_CLASS_Q(const svm_problem& prob, const svm_parameter& param)
  :Kernel(prob.l, prob.x, param)
  {
/* Robert Schubert, Oct 2005: allow application to use L2-SVM alternatively */
    this->C = param.C;
    if (param.softsvm_type == 2) this->l2norm = true;
    else this->l2norm = false;
/* ^--- */
    cache = new Cache(prob.l,(INT32)(param.cache_size*(1<<20)));
    QD = new Qfloat[prob.l];
    for(INT32 i=0;i<prob.l;i++)
      QD[i]= (Qfloat)(this->*kernel_function)(i,i);
  }
  
  Qfloat *get_Q(INT32 i, INT32 len) const
  {
    Qfloat *data;
    INT32 start;
    if((start = cache->get_data(i,&data,len)) < len)
    {
      for(INT32 j=start;j<len;j++)
        data[j] = (Qfloat)(this->*kernel_function)(i,j);
/* Robert Schubert, Oct 2005: allow application to use L2-SVM alternatively */
      if (l2norm && i >= start && i < len)
          data[i] += 1/C;
/* ^--- */
    }
    return data;
  }

  Qfloat *get_QD() const
  {
    return QD;
  }

  void swap_index(INT32 i, INT32 j) const
  {
    cache->swap_index(i,j);
    Kernel::swap_index(i,j);
    swap(QD[i],QD[j]);
  }

  ~ONE_CLASS_Q()
  {
    delete cache;
    delete[] QD;
  }
private:
/* Robert Schubert, Oct 2005: allow application to use L2-SVM alternatively */
        FLOAT64 C;
        bool l2norm;
/* ^--- */
  Cache *cache;
  Qfloat *QD;
};

class SVR_Q: public Kernel
{ 
public:
  SVR_Q(const svm_problem& prob, const svm_parameter& param)
  :Kernel(prob.l, prob.x, param)
  {
    l = prob.l;
/* Robert Schubert, Oct 2005: allow application to use L2-SVM alternatively */
    this->C = param.C;
    if (param.softsvm_type == 2) this->l2norm = true;
    else this->l2norm = false;
/* ^--- */
    cache = new Cache(l,(INT32)(param.cache_size*(1<<20)));
    QD = new Qfloat[2*l];
    sign = new schar[2*l];
    index = new INT32[2*l];
    for(INT32 k=0;k<l;k++)
    {
      sign[k] = 1;
      sign[k+l] = -1;
      index[k] = k;
      index[k+l] = k;
      QD[k]= (Qfloat)(this->*kernel_function)(k,k);
      QD[k+l]=QD[k];
    }
    buffer[0] = new Qfloat[2*l];
    buffer[1] = new Qfloat[2*l];
    next_buffer = 0;
  }

  void swap_index(INT32 i, INT32 j) const
  {
    swap(sign[i],sign[j]);
    swap(index[i],index[j]);
    swap(QD[i],QD[j]);
  }
  
  Qfloat *get_Q(INT32 i, INT32 len) const
  {
    Qfloat *data;
    INT32 real_i = index[i];
    if(cache->get_data(real_i,&data,l) < l)
    {
      for(INT32 j=0;j<l;j++)
        data[j] = (Qfloat)(this->*kernel_function)(real_i,j);
/* Robert Schubert, Oct 2005: allow application to use L2-SVM alternatively */
      if (l2norm)
          data[real_i] += 1/C;
/* ^--- */
    }

    // reorder and copy
    Qfloat *buf = buffer[next_buffer];
    next_buffer = 1 - next_buffer;
    schar si = sign[i];
    for(INT32 j=0;j<len;j++)
      buf[j] = si * sign[j] * data[index[j]];
    return buf;
  }

  Qfloat *get_QD() const
  {
    return QD;
  }

  ~SVR_Q()
  {
    delete cache;
    delete[] sign;
    delete[] index;
    delete[] buffer[0];
    delete[] buffer[1];
    delete[] QD;
  }
private:
  INT32 l;
/* Robert Schubert, Oct 2005: allow application to use L2-SVM alternatively */
        FLOAT64 C;
        bool l2norm;
/* ^--- */
  Cache *cache;
  schar *sign;
  INT32 *index;
  mutable INT32 next_buffer;
  Qfloat *buffer[2];
  Qfloat *QD;
};

//
// construct and solve various formulations
//
static void solve_c_svc(
  const svm_problem *prob, const svm_parameter* param,
  FLOAT64 *alpha, Solver::SolutionInfo* si, FLOAT64 Cp, FLOAT64 Cn)
{
  INT32 l = prob->l;
  FLOAT64 *minus_ones = new FLOAT64[l];
  schar *y = new schar[l];

  INT32 i;

  for(i=0;i<l;i++)
  {
    alpha[i] = 0;
    minus_ones[i] = -1;
    if(prob->y[i] > 0) y[i] = +1; else y[i]=-1;
  }

  Solver s;
/* Robert Schubert, Oct 2005: allow application to use L2-SVM alternatively */
  if (param->softsvm_type == 2)
      s.Solve(l, SVC_Q(*prob,*param,y), minus_ones, y,
        alpha, INF, INF, param->eps, si, param->shrinking);
  else
/* ^--- */
      s.Solve(l, SVC_Q(*prob,*param,y), minus_ones, y,
        alpha, Cp, Cn, param->eps, si, param->shrinking);

  FLOAT64 sum_alpha=0;
  for(i=0;i<l;i++)
    sum_alpha += alpha[i];

  if (Cp==Cn)
    info("nu = %f\n", sum_alpha/(Cp*prob->l));

  for(i=0;i<l;i++)
    alpha[i] *= y[i];

  delete[] minus_ones;
  delete[] y;
}

static void solve_nu_svc(
  const svm_problem *prob, const svm_parameter *param,
  FLOAT64 *alpha, Solver::SolutionInfo* si)
{
  INT32 i;
  INT32 l = prob->l;
  FLOAT64 nu = param->nu;

  schar *y = new schar[l];

  for(i=0;i<l;i++)
    if(prob->y[i]>0)
      y[i] = +1;
    else
      y[i] = -1;

  FLOAT64 sum_pos = nu*l/2;
  FLOAT64 sum_neg = nu*l/2;

  for(i=0;i<l;i++)
    if(y[i] == +1)
    {
      alpha[i] = min((FLOAT64)1.0,sum_pos);
      sum_pos -= alpha[i];
    }
    else
    {
      alpha[i] = min((FLOAT64)1.0,sum_neg);
      sum_neg -= alpha[i];
    }

  FLOAT64 *zeros = new FLOAT64[l];

  for(i=0;i<l;i++)
    zeros[i] = 0;

  Solver_NU s;
  s.Solve(l, SVC_Q(*prob,*param,y), zeros, y,
      alpha, 1.0, 1.0, param->eps, si,  param->shrinking);
  FLOAT64 r = si->r;

  info("C = %f\n",1/r);

  for(i=0;i<l;i++)
    alpha[i] *= y[i]/r;

  si->rho /= r;
  si->obj /= (r*r);
  si->margin *= r;
  si->upper_bound_p = 1/r;
  si->upper_bound_n = 1/r;

  delete[] y;
  delete[] zeros;
}

static void solve_one_class(
  const svm_problem *prob, const svm_parameter *param,
  FLOAT64 *alpha, Solver::SolutionInfo* si)
{
  INT32 l = prob->l;
  FLOAT64 *zeros = new FLOAT64[l];
  schar *ones = new schar[l];
  INT32 i;

  INT32 n = (INT32)(param->nu*prob->l);  // # of alpha's at upper bound

  for(i=0;i<n;i++)
    alpha[i] = 1;
  if(n<prob->l)
    alpha[n] = param->nu * prob->l - n;
  for(i=n+1;i<l;i++)
    alpha[i] = 0;

  for(i=0;i<l;i++)
  {
    zeros[i] = 0;
    ones[i] = 1;
  }

  Solver s;
/* Robert Schubert, Oct 2005: allow application to use L2-SVM alternatively */
  if (param->softsvm_type == 2)
      s.Solve(l, ONE_CLASS_Q(*prob,*param), zeros, ones,
        alpha, INF, INF, param->eps, si, param->shrinking);
  else
/* ^--- */
      s.Solve(l, ONE_CLASS_Q(*prob,*param), zeros, ones,
        alpha, 1.0, 1.0, param->eps, si, param->shrinking);

  delete[] zeros;
  delete[] ones;
}

static void solve_epsilon_svr(
  const svm_problem *prob, const svm_parameter *param,
  FLOAT64 *alpha, Solver::SolutionInfo* si)
{
  INT32 l = prob->l;
  FLOAT64 *alpha2 = new FLOAT64[2*l];
  FLOAT64 *linear_term = new FLOAT64[2*l];
  schar *y = new schar[2*l];
  INT32 i;

  for(i=0;i<l;i++)
  {
    alpha2[i] = 0;
    linear_term[i] = param->p - prob->y[i];
    y[i] = 1;

    alpha2[i+l] = 0;
    linear_term[i+l] = param->p + prob->y[i];
    y[i+l] = -1;
  }

  Solver s;
/* Robert Schubert, Oct 2005: allow application to use L2-SVM alternatively */
  if (param->softsvm_type == 2)
      s.Solve(2*l, SVR_Q(*prob,*param), linear_term, y,
        alpha2, INF, INF, param->eps, si, param->shrinking);
  else
/* ^--- */
      s.Solve(2*l, SVR_Q(*prob,*param), linear_term, y,
        alpha2, param->C, param->C, param->eps, si, param->shrinking);

  FLOAT64 sum_alpha = 0;
  for(i=0;i<l;i++)
  {
    alpha[i] = alpha2[i] - alpha2[i+l];
    sum_alpha += fabs(alpha[i]);
  }
  info("nu = %f\n",sum_alpha/(param->C*l));

  delete[] alpha2;
  delete[] linear_term;
  delete[] y;
}

static void solve_nu_svr(
  const svm_problem *prob, const svm_parameter *param,
  FLOAT64 *alpha, Solver::SolutionInfo* si)
{
  INT32 l = prob->l;
  FLOAT64 C = param->C;
  FLOAT64 *alpha2 = new FLOAT64[2*l];
  FLOAT64 *linear_term = new FLOAT64[2*l];
  schar *y = new schar[2*l];
  INT32 i;

  FLOAT64 sum = C * param->nu * l / 2;
  for(i=0;i<l;i++)
  {
    alpha2[i] = alpha2[i+l] = min(sum,C);
    sum -= alpha2[i];

    linear_term[i] = - prob->y[i];
    y[i] = 1;

    linear_term[i+l] = prob->y[i];
    y[i+l] = -1;
  }

  Solver_NU s;
  s.Solve(2*l, SVR_Q(*prob,*param), linear_term, y,
    alpha2, C, C, param->eps, si, param->shrinking);

  info("epsilon = %f\n",-si->r);

  for(i=0;i<l;i++)
    alpha[i] = alpha2[i] - alpha2[i+l];

  delete[] alpha2;
  delete[] linear_term;
  delete[] y;
}

//
// decision_function
//

decision_function svm_train_one(
  const svm_problem *prob, const svm_parameter *param,
  FLOAT64 Cp, FLOAT64 Cn)
{
  FLOAT64 *alpha = Malloc(FLOAT64,prob->l);
  Solver::SolutionInfo si;
  switch(param->svm_type)
  {
    case C_SVC:
      solve_c_svc(prob,param,alpha,&si,Cp,Cn);
      break;
    case NU_SVC:
      solve_nu_svc(prob,param,alpha,&si);
      break;
    case ONE_CLASS:
      solve_one_class(prob,param,alpha,&si);
      break;
    case EPSILON_SVR:
      solve_epsilon_svr(prob,param,alpha,&si);
      break;
    case NU_SVR:
      solve_nu_svr(prob,param,alpha,&si);
      break;
  }

  info("obj = %f, b = %f, margin = %f\n",si.obj,si.rho,si.margin);

  // output SVs

  INT32 nSV = 0;
  INT32 nBSV = 0;
  for(INT32 i=0;i<prob->l;i++)
  {
    if(fabs(alpha[i]) > 0)
    {
      ++nSV;
      if(prob->y[i] > 0)
      {
        if(fabs(alpha[i]) >= si.upper_bound_p)
          ++nBSV;
      }
      else
      {
        if(fabs(alpha[i]) >= si.upper_bound_n)
          ++nBSV;
      }
    }
  }

  info("nSV = %d, nBSV = %d\n",nSV,nBSV);

  decision_function f;
  f.alpha = alpha;
  f.rho = si.rho;
  return f;
}

//
// svm_model
//

// Platt's binary SVM Probablistic Output: an improvement from Lin et al.
void sigmoid_train(
  INT32 l, const FLOAT64 *dec_values, const FLOAT64 *labels,
  FLOAT64& A, FLOAT64& B)
{
  FLOAT64 prior1=0, prior0 = 0;
  INT32 i;

  for (i=0;i<l;i++)
    if (labels[i] > 0) prior1+=1;
    else prior0+=1;
  
  INT32 max_iter=100;   // Maximal number of iterations
  FLOAT64 min_step=1e-10;  // Minimal step taken in line search
  FLOAT64 sigma=1e-3;  // For numerically strict PD of Hessian
  FLOAT64 eps=1e-5;
  FLOAT64 hiTarget=(prior1+1.0)/(prior1+2.0);
  FLOAT64 loTarget=1/(prior0+2.0);
  FLOAT64 *t=Malloc(FLOAT64,l);
  FLOAT64 fApB,p,q,h11,h22,h21,g1,g2,det,dA,dB,gd,stepsize;
  FLOAT64 newA,newB,newf,d1,d2;
  INT32 iter;
  
  // Initial Point and Initial Fun Value
  A=0.0; B=log((prior0+1.0)/(prior1+1.0));
  FLOAT64 fval = 0.0;

  for (i=0;i<l;i++)
  {
    if (labels[i]>0) t[i]=hiTarget;
    else t[i]=loTarget;
    fApB = dec_values[i]*A+B;
    if (fApB>=0)
      fval += t[i]*fApB + log(1+exp(-fApB));
    else
      fval += (t[i] - 1)*fApB +log(1+exp(fApB));
  }
  for (iter=0;iter<max_iter;iter++)
  {
    // Update Gradient and Hessian (use H' = H + sigma I)
    h11=sigma; // numerically ensures strict PD
    h22=sigma;
    h21=0.0;g1=0.0;g2=0.0;
    for (i=0;i<l;i++)
    {
      fApB = dec_values[i]*A+B;
      if (fApB >= 0)
      {
        p=exp(-fApB)/(1.0+exp(-fApB));
        q=1.0/(1.0+exp(-fApB));
      }
      else
      {
        p=1.0/(1.0+exp(fApB));
        q=exp(fApB)/(1.0+exp(fApB));
      }
      d2=p*q;
      h11+=dec_values[i]*dec_values[i]*d2;
      h22+=d2;
      h21+=dec_values[i]*d2;
      d1=t[i]-p;
      g1+=dec_values[i]*d1;
      g2+=d1;
    }

    // Stopping Criteria
    if (fabs(g1)<eps && fabs(g2)<eps)
      break;

    // Finding Newton direction: -inv(H') * g
    det=h11*h22-h21*h21;
    dA=-(h22*g1 - h21 * g2) / det;
    dB=-(-h21*g1+ h11 * g2) / det;
    gd=g1*dA+g2*dB;


    stepsize = 1;     // Line Search
    while (stepsize >= min_step)
    {
      newA = A + stepsize * dA;
      newB = B + stepsize * dB;

      // New function value
      newf = 0.0;
      for (i=0;i<l;i++)
      {
        fApB = dec_values[i]*newA+newB;
        if (fApB >= 0)
          newf += t[i]*fApB + log(1+exp(-fApB));
        else
          newf += (t[i] - 1)*fApB +log(1+exp(fApB));
      }
      // Check sufficient decrease
      if (newf<fval+0.0001*stepsize*gd)
      {
        A=newA;B=newB;fval=newf;
        break;
      }
      else
        stepsize = stepsize / 2.0;
    }

    if (stepsize < min_step)
    {
      info("Line search fails in two-class probability estimates\n");
      break;
    }
  }

  if (iter>=max_iter)
    info("Reaching maximal iterations in two-class probability estimates\n");
  free(t);
}

FLOAT64 sigmoid_predict(FLOAT64 decision_value, FLOAT64 A, FLOAT64 B)
{
  FLOAT64 fApB = decision_value*A+B;
  if (fApB >= 0)
    return exp(-fApB)/(1.0+exp(-fApB));
  else
    return 1.0/(1+exp(fApB)) ;
}

// Method 2 from the multiclass_prob paper by Wu, Lin, and Weng
void multiclass_probability(INT32 k, FLOAT64 **r, FLOAT64 *p)
{
  INT32 t,j;
  INT32 iter = 0, max_iter=100;
  FLOAT64 **Q=Malloc(FLOAT64 *,k);
  FLOAT64 *Qp=Malloc(FLOAT64,k);
  FLOAT64 pQp, eps=0.005/k;
  
  for (t=0;t<k;t++)
  {
    p[t]=1.0/k;  // Valid if k = 1
    Q[t]=Malloc(FLOAT64,k);
    Q[t][t]=0;
    for (j=0;j<t;j++)
    {
      Q[t][t]+=r[j][t]*r[j][t];
      Q[t][j]=Q[j][t];
    }
    for (j=t+1;j<k;j++)
    {
      Q[t][t]+=r[j][t]*r[j][t];
      Q[t][j]=-r[j][t]*r[t][j];
    }
  }
  for (iter=0;iter<max_iter;iter++)
  {
    // stopping condition, recalculate QP,pQP for numerical accuracy
    pQp=0;
    for (t=0;t<k;t++)
    {
      Qp[t]=0;
      for (j=0;j<k;j++)
        Qp[t]+=Q[t][j]*p[j];
      pQp+=p[t]*Qp[t];
    }
    FLOAT64 max_error=0;
    for (t=0;t<k;t++)
    {
      FLOAT64 error=fabs(Qp[t]-pQp);
      if (error>max_error)
        max_error=error;
    }
    if (max_error<eps) break;
    
    for (t=0;t<k;t++)
    {
      FLOAT64 diff=(-Qp[t]+pQp)/Q[t][t];
      p[t]+=diff;
      pQp=(pQp+diff*(diff*Q[t][t]+2*Qp[t]))/(1+diff)/(1+diff);
      for (j=0;j<k;j++)
      {
        Qp[j]=(Qp[j]+diff*Q[t][j])/(1+diff);
        p[j]/=(1+diff);
      }
    }
  }
  if (iter>=max_iter)
    info("Exceeds max_iter in multiclass_prob\n");
  for(t=0;t<k;t++) free(Q[t]);
  free(Q);
  free(Qp);
}

// Cross-validation decision values for probability estimates
void svm_binary_svc_probability(
  const svm_problem *prob, const svm_parameter *param,
  FLOAT64 Cp, FLOAT64 Cn, FLOAT64& probA, FLOAT64& probB)
{
  INT32 i;
  INT32 nr_fold = 5;
  INT32 *perm = Malloc(INT32,prob->l);
  FLOAT64 *dec_values = Malloc(FLOAT64,prob->l);

  info("calculating probability model by %d-fold cross-validation over %d points...\n", nr_fold, prob->l);

  // random shuffle
  for(i=0;i<prob->l;i++) perm[i]=i;
  for(i=0;i<prob->l;i++)
  {
    INT32 j = i+rand()%(prob->l-i);
    swap(perm[i],perm[j]);
  }
  for(i=0;i<nr_fold;i++)
  {
    INT32 begin = i*prob->l/nr_fold;
    INT32 end = (i+1)*prob->l/nr_fold;
    INT32 j,k;
    struct svm_problem subprob;

    subprob.l = prob->l-(end-begin);
    subprob.x = Malloc(struct svm_node*,subprob.l);
    subprob.y = Malloc(FLOAT64,subprob.l);
      
    k=0;
    for(j=0;j<begin;j++)
    {
      subprob.x[k] = prob->x[perm[j]];
      subprob.y[k] = prob->y[perm[j]];
      ++k;
    }
    for(j=end;j<prob->l;j++)
    {
      subprob.x[k] = prob->x[perm[j]];
      subprob.y[k] = prob->y[perm[j]];
      ++k;
    }
    INT32 p_count=0,n_count=0;
    for(j=0;j<k;j++)
      if(subprob.y[j]>0)
        p_count++;
      else
        n_count++;

    if(p_count==0 && n_count==0)
      for(j=begin;j<end;j++)
        dec_values[perm[j]] = 0;
    else if(p_count > 0 && n_count == 0)
      for(j=begin;j<end;j++)
        dec_values[perm[j]] = 1;
    else if(p_count == 0 && n_count > 0)
      for(j=begin;j<end;j++)
        dec_values[perm[j]] = -1;
    else
    {
      svm_parameter subparam = *param;
      subparam.probability=0;
      subparam.C=1.0;
      subparam.nr_weight=2;
      subparam.weight_label = Malloc(INT32,2);
      subparam.weight = Malloc(FLOAT64,2);
      subparam.weight_label[0]=+1;
      subparam.weight_label[1]=-1;
      subparam.weight[0]=Cp;
      subparam.weight[1]=Cn;
      struct svm_model *submodel = svm_train(&subprob,&subparam);
      for(j=begin;j<end;j++)
      {
        svm_predict_values(submodel,prob->x[perm[j]],&(dec_values[perm[j]])); 
        // ensure +1 -1 order; reason not using CV subroutine
        dec_values[perm[j]] *= submodel->label[0];
      }    
      svm_destroy_model(submodel);
      svm_destroy_param(&subparam);
      free(subprob.x);
      free(subprob.y);
    }
  }    
  sigmoid_train(prob->l,dec_values,prob->y,probA,probB);
  free(dec_values);
  free(perm);
}

// Return parameter of a Laplace distribution 
FLOAT64 svm_svr_probability(
  const svm_problem *prob, const svm_parameter *param)
{
  INT32 i;
  INT32 nr_fold = 5;
  FLOAT64 *ymv = Malloc(FLOAT64,prob->l);
  FLOAT64 mae = 0;

  svm_parameter newparam = *param;
  newparam.probability = 0;
  svm_cross_validation(prob,&newparam,nr_fold,ymv);
  for(i=0;i<prob->l;i++)
  {
    ymv[i]=prob->y[i]-ymv[i];
    mae += fabs(ymv[i]);
  }    
  mae /= prob->l;
  FLOAT64 std=sqrt(2*mae*mae);
  INT32 count=0;
  mae=0;
  for(i=0;i<prob->l;i++)
          if (fabs(ymv[i]) > 5*std) 
                        count=count+1;
    else 
            mae+=fabs(ymv[i]);
  mae /= (prob->l-count);
  info("Prob. model for test data: target value = predicted value + z,\nz: Laplace distribution e^(-|z|/sigma)/(2sigma),sigma= %g\n",mae);
  free(ymv);
  return mae;
}


// label: label name, start: begin of each class, count: #data of classes, perm: indices to the original data
// perm, length l, must be allocated before calling this subroutine
void svm_group_classes(const svm_problem *prob, INT32 *nr_class_ret, INT32 **label_ret, INT32 **start_ret, INT32 **count_ret, INT32 *perm)
{
  INT32 l = prob->l;
  INT32 max_nr_class = 16;
  INT32 nr_class = 0;
  INT32 *label = Malloc(INT32,max_nr_class);
  INT32 *count = Malloc(INT32,max_nr_class);
  INT32 *data_label = Malloc(INT32,l);
  INT32 i;

  for(i=0;i<l;i++)
  {
    INT32 this_label = (INT32)prob->y[i];
    INT32 j;
    for(j=0;j<nr_class;j++)
    {
      if(this_label == label[j])
      {
        ++count[j];
        break;
      }
    }
    data_label[i] = j;
    if(j == nr_class)
    {
      if(nr_class == max_nr_class)
      {
        max_nr_class *= 2;
        label = (INT32 *)realloc(label,max_nr_class*sizeof(INT32));
        count = (INT32 *)realloc(count,max_nr_class*sizeof(INT32));
      }
      label[nr_class] = this_label;
      count[nr_class] = 1;
      ++nr_class;
    }
  }

  INT32 *start = Malloc(INT32,nr_class);
  start[0] = 0;
  for(i=1;i<nr_class;i++)
    start[i] = start[i-1]+count[i-1];
  for(i=0;i<l;i++)
  {
    perm[start[data_label[i]]] = i;
    ++start[data_label[i]];
  }
  start[0] = 0;
  for(i=1;i<nr_class;i++)
    start[i] = start[i-1]+count[i-1];

  *nr_class_ret = nr_class;
  *label_ret = label;
  *start_ret = start;
  *count_ret = count;
  free(data_label);
}

//
// Interface functions
//
svm_model *svm_train(const svm_problem *prob, const svm_parameter *param)
{
  svm_model *model = Malloc(svm_model,1);
  model->param = *param;
  model->free_sv = 0;  // XXX

  if(param->svm_type == ONE_CLASS ||
     param->svm_type == EPSILON_SVR ||
     param->svm_type == NU_SVR)
  {
    // regression or one-class-svm
    model->nr_class = 2;
    model->label = NULL;
    model->nSV = NULL;
    model->probA = NULL; model->probB = NULL;
    model->sv_coef = Malloc(FLOAT64 *,1);

    if(param->probability && 
       (param->svm_type == EPSILON_SVR ||
        param->svm_type == NU_SVR))
    {
      model->probA = Malloc(FLOAT64,1);
      model->probA[0] = svm_svr_probability(prob,param);
    }

    decision_function f = svm_train_one(prob,param,0,0);
    model->rho = Malloc(FLOAT64,1);
    model->rho[0] = f.rho;

    INT32 nSV = 0;
    INT32 i;
    for(i=0;i<prob->l;i++)
      if(fabs(f.alpha[i]) > 0) ++nSV;
    model->l = nSV;
    model->SV = Malloc(svm_node *,nSV);
    model->sv_coef[0] = Malloc(FLOAT64,nSV);
    INT32 j = 0;
    for(i=0;i<prob->l;i++)
      if(fabs(f.alpha[i]) > 0)
      {
        model->SV[j] = prob->x[i];
        model->sv_coef[0][j] = f.alpha[i];
        ++j;
      }    

    free(f.alpha);
  }
  else
  {
    // classification
    INT32 l = prob->l;
    INT32 nr_class;
    INT32 *label = NULL;
    INT32 *start = NULL;
    INT32 *count = NULL;
    INT32 *perm = Malloc(INT32,l);

    // group training data of the same class
    svm_group_classes(prob,&nr_class,&label,&start,&count,perm);    
    svm_node **x = Malloc(svm_node *,l);
    INT32 i;
    for(i=0;i<l;i++)
      x[i] = prob->x[perm[i]];

    // calculate weighted C

    FLOAT64 *weighted_C = Malloc(FLOAT64, nr_class);
    for(i=0;i<nr_class;i++)
      weighted_C[i] = param->C;
    for(i=0;i<param->nr_weight;i++)
    {  
      INT32 j;
      for(j=0;j<nr_class;j++)
        if(param->weight_label[i] == label[j])
          break;
      if(j == nr_class)
        fprintf(stderr,"warning: class label %d specified in weight is not found\n", (int)param->weight_label[i]);
      else
        weighted_C[j] *= param->weight[i];
    }

    // train k*(k-1)/2 models
    
    bool *nonzero = Malloc(bool,l);
    for(i=0;i<l;i++)
      nonzero[i] = false;
    decision_function *f = Malloc(decision_function,nr_class*(nr_class-1)/2);

    FLOAT64 *probA=NULL,*probB=NULL;
    if (param->probability)
    {
      probA=Malloc(FLOAT64,nr_class*(nr_class-1)/2);
      probB=Malloc(FLOAT64,nr_class*(nr_class-1)/2);
    }

    INT32 p = 0;
    for(i=0;i<nr_class;i++)
      for(INT32 j=i+1;j<nr_class;j++)
      {
        svm_problem sub_prob;
        INT32 si = start[i], sj = start[j];
        INT32 ci = count[i], cj = count[j];
        sub_prob.l = ci+cj;
        sub_prob.x = Malloc(svm_node *,sub_prob.l);
        sub_prob.y = Malloc(FLOAT64,sub_prob.l);
        INT32 k;
        for(k=0;k<ci;k++)
        {
          sub_prob.x[k] = x[si+k];
          sub_prob.y[k] = +1;
        }
        for(k=0;k<cj;k++)
        {
          sub_prob.x[ci+k] = x[sj+k];
          sub_prob.y[ci+k] = -1;
        }
        
        if(nr_class > 2) info("training %d against %d:\n", label[i], label[j]);

        if(param->probability)
          svm_binary_svc_probability(&sub_prob,param,weighted_C[i],weighted_C[j],probA[p],probB[p]);

        f[p] = svm_train_one(&sub_prob,param,weighted_C[i],weighted_C[j]);
        for(k=0;k<ci;k++)
          if(!nonzero[si+k] && fabs(f[p].alpha[k]) > 0)
            nonzero[si+k] = true;
        for(k=0;k<cj;k++)
          if(!nonzero[sj+k] && fabs(f[p].alpha[ci+k]) > 0)
            nonzero[sj+k] = true;
        free(sub_prob.x);
        free(sub_prob.y);
        ++p;
      }

    // build output

    model->nr_class = nr_class;
    
    model->label = Malloc(INT32,nr_class);
    for(i=0;i<nr_class;i++)
      model->label[i] = label[i];
    
    model->rho = Malloc(FLOAT64,nr_class*(nr_class-1)/2);
    for(i=0;i<nr_class*(nr_class-1)/2;i++)
      model->rho[i] = f[i].rho;

    if(param->probability)
    {
      model->probA = Malloc(FLOAT64,nr_class*(nr_class-1)/2);
      model->probB = Malloc(FLOAT64,nr_class*(nr_class-1)/2);
      for(i=0;i<nr_class*(nr_class-1)/2;i++)
      {
        model->probA[i] = probA[i];
        model->probB[i] = probB[i];
      }
    }
    else
    {
      model->probA=NULL;
      model->probB=NULL;
    }

    INT32 total_sv = 0;
    INT32 *nz_count = Malloc(INT32,nr_class);
    model->nSV = Malloc(INT32,nr_class);
    for(i=0;i<nr_class;i++)
    {
      INT32 nSV = 0;
      for(INT32 j=0;j<count[i];j++)
        if(nonzero[start[i]+j])
        {  
          ++nSV;
          ++total_sv;
        }
      model->nSV[i] = nSV;
      nz_count[i] = nSV;
    }
    
    info("Total nSV = %d\n",total_sv);

    model->l = total_sv;
    model->SV = Malloc(svm_node *,total_sv);
    p = 0;
    for(i=0;i<l;i++)
      if(nonzero[i]) model->SV[p++] = x[i];

    INT32 *nz_start = Malloc(INT32,nr_class);
    nz_start[0] = 0;
    for(i=1;i<nr_class;i++)
      nz_start[i] = nz_start[i-1]+nz_count[i-1];

    model->sv_coef = Malloc(FLOAT64 *,nr_class-1);
    for(i=0;i<nr_class-1;i++)
      model->sv_coef[i] = Malloc(FLOAT64,total_sv);

    p = 0;
    for(i=0;i<nr_class;i++)
      for(INT32 j=i+1;j<nr_class;j++)
      {
        // classifier (i,j): coefficients with
        // i are in sv_coef[j-1][nz_start[i]...],
        // j are in sv_coef[i][nz_start[j]...]

        INT32 si = start[i];
        INT32 sj = start[j];
        INT32 ci = count[i];
        INT32 cj = count[j];
        
        INT32 q = nz_start[i];
        INT32 k;
        for(k=0;k<ci;k++)
          if(nonzero[si+k])
            model->sv_coef[j-1][q++] = f[p].alpha[k];
        q = nz_start[j];
        for(k=0;k<cj;k++)
          if(nonzero[sj+k])
            model->sv_coef[i][q++] = f[p].alpha[ci+k];
        ++p;
      }
    
    free(label);
    free(probA);
    free(probB);
    free(count);
    free(perm);
    free(start);
    free(x);
    free(weighted_C);
    free(nonzero);
    for(i=0;i<nr_class*(nr_class-1)/2;i++)
      free(f[i].alpha);
    free(f);
    free(nz_count);
    free(nz_start);
  }
  return model;
}

// Stratified cross validation
void svm_cross_validation(const svm_problem *prob, const svm_parameter *param, INT32 nr_fold, FLOAT64 *target)
{
  INT32 i;
  INT32 *fold_start = Malloc(INT32,nr_fold+1);
  INT32 l = prob->l;
  INT32 *perm = Malloc(INT32,l);
  INT32 nr_class;

  // stratified cv may not give leave-one-out rate
  // Each class to l folds -> some folds may have zero elements
  if((param->svm_type == C_SVC ||
      param->svm_type == NU_SVC) && nr_fold < l)
  {
    INT32 *start = NULL;
    INT32 *label = NULL;
    INT32 *count = NULL;
    svm_group_classes(prob,&nr_class,&label,&start,&count,perm);

    // random shuffle and then data grouped by fold using the array perm
    INT32 *fold_count = Malloc(INT32,nr_fold);
    INT32 c;
    INT32 *index = Malloc(INT32,l);
    for(i=0;i<l;i++)
      index[i]=perm[i];
    for (c=0; c<nr_class; c++) 
      for(i=0;i<count[c];i++)
      {
        INT32 j = i+rand()%(count[c]-i);
        swap(index[start[c]+j],index[start[c]+i]);
      }
    for(i=0;i<nr_fold;i++)
    {
      fold_count[i] = 0;
      for (c=0; c<nr_class;c++)
        fold_count[i]+=(i+1)*count[c]/nr_fold-i*count[c]/nr_fold;
    }
    fold_start[0]=0;
    for (i=1;i<=nr_fold;i++)
      fold_start[i] = fold_start[i-1]+fold_count[i-1];
    for (c=0; c<nr_class;c++)
      for(i=0;i<nr_fold;i++)
      {
        INT32 begin = start[c]+i*count[c]/nr_fold;
        INT32 end = start[c]+(i+1)*count[c]/nr_fold;
        for(INT32 j=begin;j<end;j++)
        {
          perm[fold_start[i]] = index[j];
          fold_start[i]++;
        }
      }
    fold_start[0]=0;
    for (i=1;i<=nr_fold;i++)
      fold_start[i] = fold_start[i-1]+fold_count[i-1];
    free(start);  
    free(label);
    free(count);  
    free(index);
    free(fold_count);
  }
  else
  {
    for(i=0;i<l;i++) perm[i]=i;
    for(i=0;i<l;i++)
    {
      INT32 j = i+rand()%(l-i);
      swap(perm[i],perm[j]);
    }
    for(i=0;i<=nr_fold;i++)
      fold_start[i]=i*l/nr_fold;
  }

  for(i=0;i<nr_fold;i++)
  {
    INT32 begin = fold_start[i];
    INT32 end = fold_start[i+1];
    INT32 j,k;
    struct svm_problem subprob;

    subprob.l = l-(end-begin);
    subprob.x = Malloc(struct svm_node*,subprob.l);
    subprob.y = Malloc(FLOAT64,subprob.l);
      
    k=0;
    for(j=0;j<begin;j++)
    {
      subprob.x[k] = prob->x[perm[j]];
      subprob.y[k] = prob->y[perm[j]];
      ++k;
    }
    for(j=end;j<l;j++)
    {
      subprob.x[k] = prob->x[perm[j]];
      subprob.y[k] = prob->y[perm[j]];
      ++k;
    }
    struct svm_model *submodel = svm_train(&subprob,param);
    if(param->probability && 
       (param->svm_type == C_SVC || param->svm_type == NU_SVC))
    {
      FLOAT64 *prob_estimates=Malloc(FLOAT64,svm_get_nr_class(submodel));
      for(j=begin;j<end;j++)
        target[perm[j]] = svm_predict_probability(submodel,prob->x[perm[j]],prob_estimates);
      free(prob_estimates);      
    }
    else
      for(j=begin;j<end;j++)
        target[perm[j]] = svm_predict(submodel,prob->x[perm[j]]);
    svm_destroy_model(submodel);
    free(subprob.x);
    free(subprob.y);
  }    
  free(fold_start);
  free(perm);  
}


INT32 svm_get_svm_type(const svm_model *model)
{
  return model->param.svm_type;
}

INT32 svm_get_nr_class(const svm_model *model)
{
  return model->nr_class;
}

void svm_get_labels(const svm_model *model, INT32* label)
{
  if (model->label != NULL)
    for(INT32 i=0;i<model->nr_class;i++)
      label[i] = model->label[i];
}

FLOAT64 svm_get_svr_probability(const svm_model *model)
{
  if ((model->param.svm_type == EPSILON_SVR || model->param.svm_type == NU_SVR) &&
      model->probA!=NULL)
    return model->probA[0];
  else
  {
    info("Model doesn't contain information for SVR probability inference\n");
    return 0;
  }
}

void svm_predict_values(const svm_model *model, const svm_node *x, FLOAT64* dec_values)
{
  if(model->param.svm_type == ONE_CLASS ||
     model->param.svm_type == EPSILON_SVR ||
     model->param.svm_type == NU_SVR)
  {
    FLOAT64 *sv_coef = model->sv_coef[0];
    FLOAT64 sum = 0;
    for(INT32 i=0;i<model->l;i++)
      sum += sv_coef[i] * Kernel::k_function(x,model->SV[i],model->param);
    sum -= model->rho[0];
    *dec_values = sum;
  }
  else
  {
    INT32 i;
    INT32 nr_class = model->nr_class;
    INT32 l = model->l;
    
    FLOAT64 *kvalue = Malloc(FLOAT64,l);
    for(i=0;i<l;i++)
      kvalue[i] = Kernel::k_function(x,model->SV[i],model->param);

    INT32 *start = Malloc(INT32,nr_class);
    start[0] = 0;
    for(i=1;i<nr_class;i++)
      start[i] = start[i-1]+model->nSV[i-1];

    INT32 p=0;
    INT32 pos=0;
    for(i=0;i<nr_class;i++)
      for(INT32 j=i+1;j<nr_class;j++)
      {
        FLOAT64 sum = 0;
        INT32 si = start[i];
        INT32 sj = start[j];
        INT32 ci = model->nSV[i];
        INT32 cj = model->nSV[j];
        
        INT32 k;
        FLOAT64 *coef1 = model->sv_coef[j-1];
        FLOAT64 *coef2 = model->sv_coef[i];
        for(k=0;k<ci;k++)
          sum += coef1[si+k] * kvalue[si+k];
        for(k=0;k<cj;k++)
          sum += coef2[sj+k] * kvalue[sj+k];
        sum -= model->rho[p++];
        dec_values[pos++] = sum;
      }

    free(kvalue);
    free(start);
  }
}

/* Robert Schubert, Oct 2005: allow application to access decision values after prediction */
FLOAT64 svm_predict(const svm_model *model, const svm_node *x)
{
    FLOAT64 d;
    FLOAT64 *dec_values = Malloc(FLOAT64, model->nr_class*(model->nr_class-1)/2);
    d = svm_predict_with_values(model, x, dec_values);
    free(dec_values);
    return d;
}

FLOAT64 svm_predict_with_values(const svm_model *model, const svm_node *x, FLOAT64 *dec_values)
/* ^--- */
{
  if(model->param.svm_type == ONE_CLASS ||
     model->param.svm_type == EPSILON_SVR ||
     model->param.svm_type == NU_SVR)
  {
    FLOAT64 res;
    svm_predict_values(model, x, &res);
    
    if(model->param.svm_type == ONE_CLASS)
      return (res>0)?1:-1;
    else
      return res;
  }
  else
  {
    INT32 i;
    INT32 nr_class = model->nr_class;
    /* Robert Schubert, Oct 2005: allow application to access decision values after prediction
    FLOAT64 *dec_values = Malloc(FLOAT64, nr_class*(nr_class-1)/2); */
    svm_predict_values(model, x, dec_values);

    INT32 *vote = Malloc(INT32,nr_class);
    for(i=0;i<nr_class;i++)
      vote[i] = 0;
    INT32 pos=0;
    for(i=0;i<nr_class;i++)
      for(INT32 j=i+1;j<nr_class;j++)
      {
/* Robert Schubert, Feb 2006:  allow rejection of samples 
        if(dec_values[pos++] > 0)
          ++vote[i];
        else
          ++vote[j];
*/
        if(dec_values[pos] >= model->param.threshold)
          ++vote[i];
        else if(dec_values[pos] < - model->param.threshold)
          ++vote[j];
        ++pos;
      }

    INT32 vote_max_idx = 0;
    for(i=1;i<nr_class;i++)
      if(vote[i] > vote[vote_max_idx])
        vote_max_idx = i;
    i = vote[vote_max_idx];
    free(vote);
/* Robert Schubert, Oct 2005: allow application to access decision values after prediction
    free(dec_values); 
*/
/* Robert Schubert, Feb 2006:  allow rejection of samples 
    return model->label[vote_max_idx];
*/
    return i>0 ? model->label[vote_max_idx] : 0;
  }
}

/* Robert Schubert, Oct 2005: allow application to access decision values after prediction */
FLOAT64 svm_predict_probability(
   const svm_model *model, const svm_node *x, FLOAT64 *prob_estimates)
{
    FLOAT64 d;
    FLOAT64 *dec_values = Malloc(FLOAT64, model->nr_class*(model->nr_class-1)/2);
    d = svm_predict_probability_with_values(model, x, prob_estimates, dec_values);
    free(dec_values);
    return d;
}

FLOAT64 svm_predict_probability_with_values(
   const svm_model *model, const svm_node *x, FLOAT64 *prob_estimates, FLOAT64 *dec_values)
/* ^--- */
{
  if ((model->param.svm_type == C_SVC || model->param.svm_type == NU_SVC) &&
      model->probA!=NULL && model->probB!=NULL)
  {
    INT32 i;
    INT32 nr_class = model->nr_class;
/* Robert Schubert, Oct 2005: allow application to access decision values after prediction 
    FLOAT64 *dec_values = Malloc(FLOAT64, nr_class*(nr_class-1)/2);
*/
    svm_predict_values(model, x, dec_values);

    FLOAT64 min_prob=1e-7;
    FLOAT64 **pairwise_prob=Malloc(FLOAT64 *,nr_class);
    for(i=0;i<nr_class;i++)
      pairwise_prob[i]=Malloc(FLOAT64,nr_class);
    INT32 k=0;
    for(i=0;i<nr_class;i++)
      for(INT32 j=i+1;j<nr_class;j++)
      {
        pairwise_prob[i][j]=min(max(sigmoid_predict(dec_values[k],model->probA[k],model->probB[k]),min_prob),1-min_prob);
        pairwise_prob[j][i]=1-pairwise_prob[i][j];
        k++;
      }
    multiclass_probability(nr_class,pairwise_prob,prob_estimates);

    INT32 prob_max_idx = 0;
    for(i=1;i<nr_class;i++)
      if(prob_estimates[i] > prob_estimates[prob_max_idx])
        prob_max_idx = i;
/* Robert Schubert, Oct 2005: allow application to access decision values after prediction 
    free(dec_values); 
   V--- */
    k=0;
    for(i=0;i<nr_class;i++)
      for(INT32 j=i+1;j<nr_class;j++)
          dec_values[k++] = prob_estimates[i] - prob_estimates[j];
/* ^--- */
    for(i=0;i<nr_class;i++)
      free(pairwise_prob[i]);
                free(pairwise_prob);       
/* Robert Schubert, Feb 2006:  allow rejection of samples 
    return model->label[prob_max_idx];
*/
    return prob_estimates[prob_max_idx] > model->param.threshold ? model->label[prob_max_idx] : 0;
  }
  else 
    return svm_predict(model, x);
}

const char *svm_type_table[] =
{
  "c_svc","nu_svc","one_class","epsilon_svr","nu_svr",NULL
};

const char *kernel_type_table[]=
{
  "linear","polynomial","rbf","sigmoid",NULL
};

INT32 svm_save_model(const char *model_file_name, const svm_model *model)
{
  FILE *fp = fopen(model_file_name,"w");
  if(fp==NULL) return -1;

  const svm_parameter& param = model->param;

  fprintf(fp,"svm_type %s\n", svm_type_table[param.svm_type]);
  fprintf(fp,"kernel_type %s\n", kernel_type_table[param.kernel_type]);

  if(param.kernel_type == POLY)
    fprintf(fp,"degree %g\n", (double)param.degree);

  if(param.kernel_type == POLY || param.kernel_type == RBF || param.kernel_type == SIGMOID)
    fprintf(fp,"gamma %g\n", (double)param.gamma);

  if(param.kernel_type == POLY || param.kernel_type == SIGMOID)
    fprintf(fp,"coef0 %g\n", (double)param.coef0);

  INT32 nr_class = model->nr_class;
  INT32 l = model->l;
  fprintf(fp, "nr_class %d\n", (int)nr_class);
  fprintf(fp, "total_sv %d\n",(int)l);
  
  {
    fprintf(fp, "rho");
    for(INT32 i=0;i<nr_class*(nr_class-1)/2;i++)
      fprintf(fp," %g",(double)model->rho[i]);
    fprintf(fp, "\n");
  }
  
  if(model->label)
  {
    fprintf(fp, "label");
    for(INT32 i=0;i<nr_class;i++)
      fprintf(fp," %d",(int)model->label[i]);
    fprintf(fp, "\n");
  }

  if(model->probA) // regression has probA only
  {
    fprintf(fp, "probA");
    for(INT32 i=0;i<nr_class*(nr_class-1)/2;i++)
      fprintf(fp," %g",(double)model->probA[i]);
    fprintf(fp, "\n");
  }
  if(model->probB)
  {
    fprintf(fp, "probB");
    for(INT32 i=0;i<nr_class*(nr_class-1)/2;i++)
      fprintf(fp," %g",(double)model->probB[i]);
    fprintf(fp, "\n");
  }

  if(model->nSV)
  {
    fprintf(fp, "nr_sv");
    for(INT32 i=0;i<nr_class;i++)
      fprintf(fp," %d",(int)model->nSV[i]);
    fprintf(fp, "\n");
  }

  fprintf(fp, "SV\n");
  const FLOAT64 * const *sv_coef = model->sv_coef;
  const svm_node * const *SV = model->SV;

  for(INT32 i=0;i<l;i++)
  {
    for(INT32 j=0;j<nr_class-1;j++)
      fprintf(fp, "%.16g ",(double)sv_coef[j][i]);

    const svm_node *p = SV[i];
    while(p->index != -1)
    {
      fprintf(fp,"%d:%.8g ",(int)p->index,(double)p->value);
      p++;
    }
    fprintf(fp, "\n");
  }

  fclose(fp);
  return 0;
}

svm_model *svm_load_model(const char *model_file_name)
{
  FILE *fp = fopen(model_file_name,"rb");
  if(fp==NULL) return NULL;
  
  // read parameters

  svm_model *model = Malloc(svm_model,1);
  svm_parameter& param = model->param;
  model->rho = NULL;
  model->probA = NULL;
  model->probB = NULL;
  model->label = NULL;
  model->nSV = NULL;

  char   cmd[81];
  double nTmpDbl;
  int    nTmpInt;
  while (1) {
    if(fscanf(fp, "%80s", cmd) != 1) return NULL;

    if (strcmp(cmd, "svm_type") == 0) {
      if(fscanf(fp, "%80s", cmd) != 1) return NULL;
      INT32 i;
      for (i = 0; svm_type_table[i]; i++) {
        if (strcmp(svm_type_table[i], cmd) == 0) {
          param.svm_type = i;
          break;
        }
      }
      if (svm_type_table[i] == NULL) {
        fprintf(stderr, "unknown svm type.\n");
        free(model->rho);
        free(model->label);
        free(model->nSV);
        free(model);
        return NULL;
      }
    } else if (strcmp(cmd, "kernel_type") == 0) {
      if(fscanf(fp, "%80s", cmd) != 1) return NULL;
      INT32 i;
      for (i = 0; kernel_type_table[i]; i++) {
        if (strcmp(kernel_type_table[i], cmd) == 0) {
          param.kernel_type = i;
          break;
        }
      }
      if (kernel_type_table[i] == NULL) {
        fprintf(stderr, "unknown kernel function.\n");
        free(model->rho);
        free(model->label);
        free(model->nSV);
        free(model);
        return NULL;
      }
    } else if (strcmp(cmd, "degree") == 0) {
      if(fscanf(fp, "%lf", &nTmpDbl) != 1) return NULL;
      param.degree = (FLOAT64) nTmpDbl;
    } else if (strcmp(cmd, "gamma") == 0) {
      if(fscanf(fp, "%lf", &nTmpDbl) != 1) return NULL;
      param.gamma = (FLOAT64) nTmpDbl;
    } else if (strcmp(cmd, "coef0") == 0) {
      if(fscanf(fp, "%lf", &nTmpDbl) != 1) return NULL;
      param.coef0 = (FLOAT64) nTmpDbl;
    } else if (strcmp(cmd, "nr_class") == 0) {
      if(fscanf(fp, "%d", &nTmpInt) != 1) return NULL;
      model->nr_class = (INT32)nTmpInt;
    } else if (strcmp(cmd, "total_sv") == 0) {
      if(fscanf(fp, "%d", &nTmpInt) != 1) return NULL;
      model->l = (INT32)nTmpInt;
    } else if (strcmp(cmd, "rho") == 0) {
      INT32 n = model->nr_class * (model->nr_class - 1) / 2;
      model->rho = Malloc(FLOAT64,n);
      for (INT32 i = 0; i < n; i++) {
        if(fscanf(fp, "%lf", &nTmpDbl) != 1) return NULL;
        model->rho[i] = (FLOAT64) nTmpDbl;
      }
    } else if (strcmp(cmd, "label") == 0) {
      INT32 n = model->nr_class;
      model->label = Malloc(INT32,n);
      for (INT32 i = 0; i < n; i++) {
        if(fscanf(fp, "%d", &nTmpInt) != 1) return NULL;
        model->label[i] = (INT32)nTmpInt;
      }
    } else if (strcmp(cmd, "probA") == 0) {
      INT32 n = model->nr_class * (model->nr_class - 1) / 2;
      model->probA = Malloc(FLOAT64,n);
      for (INT32 i = 0; i < n; i++) {
        if(fscanf(fp, "%lf", &nTmpDbl) != 1) return NULL;
        model->probA[i] = (FLOAT64)nTmpDbl;
      }
    } else if (strcmp(cmd, "probB") == 0) {
      INT32 n = model->nr_class * (model->nr_class - 1) / 2;
      model->probB = Malloc(FLOAT64,n);
      for (INT32 i = 0; i < n; i++) {
        if(fscanf(fp, "%lf", &nTmpDbl) != 1) return NULL;
        model->probB[i] = (FLOAT64)nTmpDbl;
      }
    } else if (strcmp(cmd, "nr_sv") == 0) {
      INT32 n = model->nr_class;
      model->nSV = Malloc(INT32,n);
      for (INT32 i = 0; i < n; i++) {
        if(fscanf(fp, "%d", &nTmpInt) != 1) return NULL;
        model->nSV[i] = (INT32)nTmpInt;
      }
    } else if (strcmp(cmd, "SV") == 0) {
      while (1) {
        INT32 c = getc(fp);
        if (c == EOF || c == '\n')
          break;
      }
      break;
    } else {
      fprintf(stderr, "unknown text in model file\n");
      free(model->rho);
      free(model->label);
      free(model->nSV);
      free(model);
      return NULL;
    }
  }

  // read sv_coef and SV

  INT32 elements = 0;
  INT64 pos = ftell(fp);

  while(1)
  {
    INT32 c = fgetc(fp);
    switch(c)
    {
      case '\n':
        // count the '-1' element
      case ':':
        ++elements;
        break;
      case EOF:
        goto out;
      default:
        ;
    }
  }
out:
  fseek(fp,pos,SEEK_SET);

  INT32 m = model->nr_class - 1;
  INT32 l = model->l;
  model->sv_coef = Malloc(FLOAT64 *,m);
  INT32 i;
  for(i=0;i<m;i++)
    model->sv_coef[i] = Malloc(FLOAT64,l);
  model->SV = Malloc(svm_node*,l);
  svm_node *x_space=NULL;
  if(l>0) x_space = Malloc(svm_node,elements);

  INT32 j=0;
  for(i=0;i<l;i++)
  {
    model->SV[i] = &x_space[j];
    for(INT32 k=0;k<m;k++) {
      if(fscanf(fp,"%lf",&nTmpDbl) != 1) return NULL;
      model->sv_coef[k][i] = (FLOAT64)nTmpDbl;
    }
    while(1)
    {
      INT32 c;
      do {
        c = getc(fp);
        if(c=='\n') goto out2;
      } while(isspace(c));
      ungetc(c,fp);
      if(fscanf(fp,"%d:%lf",&nTmpInt, &nTmpDbl) != 1) return NULL;
      x_space[j].index = (INT32)nTmpInt;
      x_space[j].value = (FLOAT64)nTmpDbl;
      ++j;
    }  
out2:
    x_space[j++].index = -1;
  }

  fclose(fp);

  model->free_sv = 1;  // XXX
  return model;
}

void svm_destroy_model(svm_model* model)
{
  if(model->free_sv && model->l > 0)
    free((void *)(model->SV[0]));
  for(INT32 i=0;i<model->nr_class-1;i++)
    free(model->sv_coef[i]);
  free(model->SV);
  free(model->sv_coef);
  free(model->rho);
  free(model->label);
  free(model->probA);
  free(model->probB);
  free(model->nSV);
  free(model); 
}

void svm_destroy_param(svm_parameter* param)
{
  free(param->weight_label);
  free(param->weight);
}

const char *svm_check_parameter(const svm_problem *prob, const svm_parameter *param)
{
  // svm_type

  INT32 svm_type = param->svm_type;
  if(svm_type != C_SVC &&
     svm_type != NU_SVC &&
     svm_type != ONE_CLASS &&
     svm_type != EPSILON_SVR &&
     svm_type != NU_SVR)
    return "unknown svm type";
  
  // kernel_type
  
  INT32 kernel_type = param->kernel_type;
  if(kernel_type != LINEAR &&
     kernel_type != POLY &&
     kernel_type != RBF &&
     kernel_type != SIGMOID)
    return "unknown kernel type";

  // cache_size,eps,C,nu,p,shrinking

  if(param->cache_size <= 0)
    return "cache_size <= 0";

  if(param->eps <= 0)
    return "eps <= 0";

  if(svm_type == C_SVC ||
     svm_type == EPSILON_SVR ||
     svm_type == NU_SVR)
    if(param->C <= 0)
      return "C <= 0";

  if(svm_type == NU_SVC ||
     svm_type == ONE_CLASS ||
     svm_type == NU_SVR)
    if(param->nu < 0 || param->nu > 1)
      return "nu < 0 or nu > 1";

  if(svm_type == EPSILON_SVR)
    if(param->p < 0)
      return "p < 0";

  if(param->shrinking != 0 &&
     param->shrinking != 1)
    return "shrinking != 0 and shrinking != 1";

  if(param->probability != 0 &&
     param->probability != 1)
    return "probability != 0 and probability != 1";

  if(param->probability == 1 &&
     svm_type == ONE_CLASS)
    return "one-class SVM probability output not supported yet";


  // check whether nu-svc is feasible
  
  if(svm_type == NU_SVC)
  {
    INT32 l = prob->l;
    INT32 max_nr_class = 16;
    INT32 nr_class = 0;
    INT32 *label = Malloc(INT32,max_nr_class);
    INT32 *count = Malloc(INT32,max_nr_class);

    INT32 i;
    for(i=0;i<l;i++)
    {
      INT32 this_label = (INT32)prob->y[i];
      INT32 j;
      for(j=0;j<nr_class;j++)
        if(this_label == label[j])
        {
          ++count[j];
          break;
        }
      if(j == nr_class)
      {
        if(nr_class == max_nr_class)
        {
          max_nr_class *= 2;
          label = (INT32 *)realloc(label,max_nr_class*sizeof(INT32));
          count = (INT32 *)realloc(count,max_nr_class*sizeof(INT32));
        }
        label[nr_class] = this_label;
        count[nr_class] = 1;
        ++nr_class;
      }
    }
  
    for(i=0;i<nr_class;i++)
    {
      INT32 n1 = count[i];
      for(INT32 j=i+1;j<nr_class;j++)
      {
        INT32 n2 = count[j];
        if(param->nu*(n1+n2)/2 > min(n1,n2))
        {
          free(label);
          free(count);
          return "specified nu is infeasible";
        }
      }
    }
    free(label);
    free(count);
  }

  return NULL;
}

INT32 svm_check_probability_model(const svm_model *model)
{
  return ((model->param.svm_type == C_SVC || model->param.svm_type == NU_SVC) &&
    model->probA!=NULL && model->probB!=NULL) ||
    ((model->param.svm_type == EPSILON_SVR || model->param.svm_type == NU_SVR) &&
     model->probA!=NULL);
}

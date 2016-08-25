#include<iostream>

#include "JuliaNonlinearFunction.h"
#include "MinotaurConfig.h"

using namespace minotaur_julia;

JuliaNonlinearFunction::JuliaNonlinearFunction()
 : nVars_(0),
   neg_(false),
   isInObj_(false)
{
}

JuliaNonlinearFunction::JuliaNonlinearFunction( Minotaur::UInt i, 
						Minotaur::UInt nvars, 
            					eval_f_cb eval_f, eval_g_cb eval_g, 
            					eval_grad_f_cb eval_grad_f,
            					UserDataPtr userdata,
						bool is_in_obj)
     
     :nVars_(nvars),
      cons_index(i),
      eval_f_(eval_f),
      eval_g_(eval_g),
      eval_grad_f_(eval_grad_f),
      userdata_(userdata),
      neg_(false),
      isInObj_(is_in_obj)
{

}	

double JuliaNonlinearFunction::eval(double *x, int *error) 
{
  //double *xx = const_cast<double *>(x);
  // fint ferror = 0;

  double *ret;     // return value 

  if (true == isInObj_) { // objective function evaluation 
    (*eval_f_)(x, ret, userdata_);
  } else {
    // call the corresponding eval function from Julia -constraint 
    (*eval_g_)(x, ret, userdata_);
  }
  if (neg_) {
    *ret *= -1.;
  }
  // *error = (int) ferror;
  return *ret;
}

void JuliaNonlinearFunction::evalGradient (double *x, double *grad_f,
                                          int *error) 
{
  //double *xx = const_cast<double *>(x);
  //fint ferror = 0;

  std::fill(grad_f, grad_f+nVars_, 0);
  if (true == isInObj_) {
    (*eval_grad_f_)(x, grad_f, userdata_);
  } else {
    // ! if it is constraint, and if we don't specify the i-th constraint, sizes would crash.. 
  }
  if (neg_) {
    for (Minotaur::UInt i=0; i<nVars_; ++i) {
      grad_f[i] *= -1.;
    }
  }
  //*error = (int) ferror;
}

void JuliaNonlinearFunction::evalHessian(const double, const double *, 
                                        const Minotaur::LTHessStor *,
                                        double *, int *)
{
  assert(!"can't fill hessian in a AMPL nonlinear function.");
}


void JuliaNonlinearFunction::fillHessStor(Minotaur::LTHessStor *)
{
  assert(!"can't fill hessian in a AMPL nonlinear function.");
}


void JuliaNonlinearFunction::finalHessStor(const Minotaur::LTHessStor *) 
{
  assert(!"can't fill hessian in a AMPL nonlinear function.");
}


void JuliaNonlinearFunction::fillJac(const double *, double *, int *) 
{
  assert(!"can't fill jacobian in a ampl nonlinear function.");
}


void JuliaNonlinearFunction::getVars(Minotaur::VariableSet *vars)
{
  vars->insert(vars_.begin(), vars_.end());
}


void JuliaNonlinearFunction::multiply(double c)
{
  // assert(fabs(c+1.0)<1e-9); // only allowed to multiply by -1.0
  neg_ = !neg_;
}


void JuliaNonlinearFunction::prepJac(Minotaur::VarSetConstIter, 
                                    Minotaur::VarSetConstIter)
{
  assert(!"can't fill jacobian in a ampl nonlinear function.");
}


void JuliaNonlinearFunction::setVars(Minotaur::VarSetConstIterator vb, 
                                    Minotaur::VarSetConstIterator ve)
{
  vars_.insert(vb, ve);
}


void JuliaNonlinearFunction::write(std::ostream &out) const
{
  out << "nonlinear function";
}

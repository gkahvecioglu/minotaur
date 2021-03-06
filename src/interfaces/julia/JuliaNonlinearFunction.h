#ifndef JULIANONLINEARFUNCTION_H
#define JULIANONLINEARFUNCTION_H

#include "NonlinearFunction.h"
#include "JuliaProblemWrapper.h"

namespace minotaur_julia {

class JuliaInterface;

class JuliaNonlinearFunction: public Minotaur::NonlinearFunction {
public:

 // Default constructor 
 JuliaNonlinearFunction();

 JuliaNonlinearFunction(Minotaur::UInt i, Minotaur::UInt nvars, 
 			eval_f_cb eval_f, eval_g_cb eval_g, 
 			eval_grad_f_cb eval_grad_f, 
 			UserDataPtr userdata,
			bool is_in_obj);

 // Evaluate at a point. objective or constraint
  double eval(double *x, int *error);

  // Evaluate gradient at a point. Base class function.
  void evalGradient(double *x, double *grad_f,
                    int *error);

  void evalHessian(const double mult, const double *x, 
                   const Minotaur::LTHessStor *stor, double *values, 
                   int *error);

  // Not available.
  void  fillHessStor(Minotaur::LTHessStor *);

  // Not available.
  void  finalHessStor(const Minotaur::LTHessStor *);

  // Not available.
  void fillJac(const double *, double *, int *);

  // Get variables used in this function. Base class method.
  void getVars(Minotaur::VariableSet *);

  // Multiply by a constant. Base class method.
  void multiply(double c);

  // Not available.
  void prepJac(Minotaur::VarSetConstIter, Minotaur::VarSetConstIter);

  /**
   * \brief Tell what variables are in this function.
   *
   * These variabvles are then
   * stored in this class for future use by Minotaur routines.
   *
   * \param [in] vb Iterator pointing to first element of set.
   * \param [in] ve Iterator pointing to end of set.
   */
  void setVars(Minotaur::VarSetConstIterator vb, 
               Minotaur::VarSetConstIterator ve);

  // Display the function. Base class method.
  void write(std::ostream &out) const;

private:
  
  // Number of variables in the problem 
  Minotaur::UInt nVars_;

  //Index of the corresponding constraint/obj 
  Minotaur::UInt cons_index;
 
  bool isInObj_;
  
  bool neg_;

  /** Pointer to callback function evaluating value of objective function */
  eval_f_cb eval_f_;
  
  /**  Pointer to callback function evaluating value of constraints */
  eval_g_cb eval_g_;
  
  /** Pointer to callback function evaluating gradient of objective
     *  function */
  eval_grad_f_cb eval_grad_f_;
  


  /** Pointer to user data */
  UserDataPtr userdata_;

}; //JuliaNonlinearFunction
typedef boost::shared_ptr<JuliaNonlinearFunction> JuliaNlfPtr; 
} //namespace 

#endif

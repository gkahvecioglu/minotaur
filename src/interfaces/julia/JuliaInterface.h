#ifndef JULIAINTERFACE_H_
#define JULIAINTERFACE_H_

#include "Types.h"
//#include "nlp.h"
#include "JuliaProblemWrapper.h"


namespace Minotaur {
	class   Environment;
	class   LinearFunction;
	class   PolynomialFunction;
	class   Problem;
	class   QuadraticFunction;
	class   Solution;
	typedef boost::shared_ptr<Environment> EnvPtr;
	typedef boost::shared_ptr<LinearFunction> LinearFunctionPtr;
	typedef boost::shared_ptr<PolynomialFunction> PolyFunPtr;
	typedef boost::shared_ptr<Problem> ProblemPtr;
	typedef boost::shared_ptr<QuadraticFunction> QuadraticFunctionPtr;
	typedef boost::shared_ptr<const Solution> ConstSolutionPtr;
}



namespace minotaur_julia {

/* problem-specific information is held in ProblemInfo */
typedef struct ProblemInfo
{
	int n;			// number of variables 
	int m;			// number of constraints 
	double* varLB;		// lower bounds on variables
	double* varUB;		// upper bounds on variables 
	double* consLB; 	// lower bounds on constraints
	double* consUB;		// upper bounds on constraints	
	int nele_jac;		// number of nonzeros in jacobian 
	int nele_hess;		// number of nonzeros in hessian
	int obj_sense;		// the sense of objective function: Maximize or Minimum 	
	bool isobj_nl;		// true if objective function is nonlinear	
	double obj_const; 	// the constant in the objective function 
	int nb_obj;		// number of objective functions
	char* var_type;		// array keeps types of variables, binary, cont, integer 
	int nlc;		// number of linear constraints
	int nqc; 		// number of quadratic constraints 
	int nnlc;		// number of nonlinear constraints 

	// callback functions 
  	eval_f_cb eval_f_;
  	eval_g_cb eval_g_;
  	eval_grad_f_cb eval_grad_f_;
  	eval_jac_g_cb eval_jac_g_;
  	eval_h_cb eval_h_;
  	UserDataPtr userdata_;	

} JuliaProblemInfo;

typedef struct ograd ograd; // store the objective function coefficients and variables 
struct ograd {
  double coef;  // objective function coefficient 
  ograd *next; // pointer to the next element 
  int varno;  // variable number 
};

class JuliaNonlinearFunction;
typedef boost::shared_ptr<JuliaNonlinearFunction> JuliaNlfPtr;

class JuliaInterface;
typedef JuliaInterface* JuliaInterfacePtr;

class JuliaInterface{
public:
 
  // Constructors
  JuliaInterface(Minotaur::Environment * env, std::string solver= "minotaurjulia");

  JuliaInterface(Minotaur::Environment *env, int n, int m,
                   double *varLB, double *varUB, double *consLB, double *consUB,
                   int nele_jac, int nele_hess, int objSense,
                   bool isObj_nl, int n_obj);

  // Destroy
  ~JuliaInterface();

  // Get pointer to JuliaProblemInfo struct
  JuliaProblemInfo* getJulia();
  
  // set callback functions 
  void setCallbacks(eval_f_cb eval_f, eval_g_cb eval_g, eval_grad_f_cb eval_grad_f, eval_jac_g_cb eval_jac_g, eval_h_cb eval_h);
	
  // Free the JuliaProblemInfo struct 
  void freeJulia(); 

  // wrapper function for ProblemInstance()
  Minotaur::Problem *readProblemInstance();

private:

   /**
   * The number of defined variables. AMPL stores the number of 5 kinds of
   * defined variables: 
   * ncomo = number of defined variables in more than one objectives only
   * ncomc = number of defined variables in more than one constraints only
   * ncomb = number of defined variables in both objectives and constraints
   * ncomo1 = number of defined variables in only one objective
   * ncomc1 = number of defined variables in only one constraint.
   */
  int nDefVars_;

  /// ncomo+ncomc+ncomb
  int nDefVarsBco_;

  /// ncomo1+ncomc1
  int nDefVarsCo1_;

  std::vector<Minotaur::VariablePtr> vars_;

  // Tolerance to check integrality (of powers for example).
  const double intTol_;
  
  // Tolerance to check if zero.
  const double zTol_;

  /// Log manager
  Minotaur::LoggerPtr logger_;

  // Add variables 
  void addVariablesFromJulia_(Minotaur::ProblemPtr instance);

  // Add the i-th linear constraint from Julia
  void addLinearConstraint_(int i, Minotaur::ProblemPtr problemPtr);

  // Add the i-th linear objective from Julia?? what's i-th objective? 
  void addLinearObjective_(int i, Minotaur::ProblemPtr problemPtr);

   /**
   * Get linear terms in the i-th constraint and put them into lf. If lf
   * is NULL, new space is allocated.
   */
   void addLinearTermsFromConstr_(Minotaur::LinearFunctionPtr &lf, int i);

   /**
   * Get linear terms in the i-th objective and put them into lf. If lf
   * is NULL, new space is allocated.
   */
  void addLinearTermsFromObj_(Minotaur::LinearFunctionPtr &lf, int i);

  JuliaProblemInfo* myJulia_;

  /* load Minotaur problem with instance parameters*/
  Minotaur::Problem *readInstance();

  Minotaur::ProblemPtr getInstance(std::vector<std::set<int> > &vids);

  void saveNlVars_(std::vector<std::set<int> > &vars);
	
  std::string varName(int i);  // generates a name for i-th variable 

  std::string constName(int i);	// generates a name for i-th constraint 	
  
 
};

} // namespace MINOTAUR_JULIA

#endif

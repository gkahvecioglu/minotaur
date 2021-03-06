#ifndef _JULIAPROBLEMWRAPPER_H__
#define _JULIAPROBLEMWRAPPER_H__

#include <iostream>

#include "MinotaurApiEnv.h"

//#include "MinotaurConfig.h"
#include "BndProcessor.h"
#include "BranchAndBound.h"
#include "EngineFactory.h"
#include "Environment.h"
#include "IntVarHandler.h"
#include "LexicoBrancher.h"
#include "LinearHandler.h"
#include "LinFeasPump.h"
#include "Logger.h"
#include "LPEngine.h"
#include "MaxFreqBrancher.h"
#include "MaxVioBrancher.h"
#include "MINLPDiving.h"
#include "NLPEngine.h"
#include "NlPresHandler.h"
#include "NodeIncRelaxer.h"
#include "Objective.h"
#include "Option.h"
//#include "PCBProcessor.h"
#include "Presolver.h"
#include "ProblemSize.h"
#include "QPEngine.h"
#include "Problem.h"
#include "RandomBrancher.h"
#include "Relaxation.h"
#include "ReliabilityBrancher.h"
#include "Solution.h"
#include "SOS1Handler.h"
#include "SOS2Handler.h"
#include "Timer.h"
#include "TreeManager.h"


namespace minotaur_julia{

	class JuliaInterface;
	typedef JuliaInterface * JuliaInterfacePtr;

	typedef void * UserDataPtr;

	/*
	 * prob points to the userdata field of the JuliaProblemStruct.
	 * row_node_id and col_node_id are the node ids in the stochastic tree
	 *
	 * The row and col node ids are used to identify the block in problem.
	 * When vectors are requested, the row and col ids should be equal. 
	 * eval_f_grad is an exception. See below for details.
	 * When the requested data are sub-matrices, the row and col node ids are 
	 * used to identify the block.
	 */

	typedef struct CallBackData
	{
	  UserDataPtr prob;
	  int row_node_id;
	  int col_node_id;
	  int typeflag; // typeflag 0 is for traditional bound and jacobian information and type flag 1 is reserved for linking constraints
	} CallBackData;

	typedef CallBackData* CallBackDataPtr;

	extern "C" typedef int (*eval_f_cb)(double *x, double *obj, UserDataPtr userdata);

	extern "C" typedef int (*eval_g_cb)(double *x, double *g, UserDataPtr userdata);

	extern "C" typedef int (*eval_grad_f_cb)(double *x, double* grad_f, UserDataPtr userdata);

	extern "C" typedef int (*eval_jac_g_cb)(double *x, double *values, int *iRow, int *jCol, 
			UserDataPtr userdata); 

	extern "C" typedef int (*eval_h_cb)(double* x, double* lamdba,
			double* values, int* iRow, int *jCol,
			UserDataPtr userdata);


	extern "C"
	{  
	  	void loadJuliaInterface(
				     Minotaur::MinotaurApiEnv *ApiEnv,
				     int nvars, 			// number of variables 
				     int ncons,				// number of constraints 
				     double* varLB,			// upper bounds on decision variables 
				     double* varUB,			// lower bounds on decision variables 
				     double* consLB, 			// upper bounds on constraints 
				     double* consUB, 			// lower bounds on constraints 	
				     int nele_jac, 			// number of nonzeros in jacobian 
				     int nele_hess,			// number of nonzeros in hessian 
				     int obj_sense,			// Objective sense: maximization ot minimization	
				     bool isobj_nl,			// Is objective nonlinear: true if it is nlp, otherwise false
				     int nb_obj);			// number of objective functions defined 	
		
		void setCallbacks(
				     Minotaur::MinotaurApiEnv *ApiEnv, 	        // pointer to the julia interface 
 				     eval_f_cb eval_f,                  	// Callback function of objective function 
                                     eval_g_cb eval_g,                  	// Callback function of constraint body 
                                     eval_grad_f_cb eval_grad_f,        	// Callback function of objective gradient 
                                     eval_jac_g_cb eval_jac_g,          	// Callback function of constraint Jacobian 
                                     eval_h_cb eval_h);                 	// Callback function of Lagrangian Hessian  
                
		/* load Minotaur problem */                    	
		void loadProblem(Minotaur::MinotaurApiEnv *ApiEnv);
		
		int solveProblem(JuliaInterfacePtr iface, 
				double* obj_val, double* x, UserDataPtr prob);
				 
		/* creates Minotaur API Environment*/
 		Minotaur::MinotaurApiEnv *createEnv(void);
		
		/* free Minotaur API Environment  */
		void freeEnv(Minotaur::MinotaurApiEnv *env);
	
	}

} //namespace 				

#endif // JuliaProblemWrapper endif 

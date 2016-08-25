#include <sstream>
#include <stdint.h>
#include <iostream>
//#include "opcode.hd"

#include "MinotaurConfig.h"
//#include "CGraph.h"
#include "CNode.h"
#include "Constraint.h"
#include "Environment.h"
#include "Function.h"
#include "Logger.h"
#include "LinearFunction.h"
#include "Option.h"
#include "PolynomialFunction.h"
#include "Problem.h"
#include "QuadraticFunction.h"
#include "Solution.h"
#include "SOS.h"
#include "Variable.h"
#include "JuliaInterface.h"

using namespace minotaur_julia;

JuliaInterface::JuliaInterface(Minotaur::Environment * env, std::string solver)
 : intTol_(1e-8), zTol_(1e-8)
{
  std::string str = "minotaurjulia";
 // logger_ = (Minotaur::LoggerPtr) new Minotaur::Logger((Minotaur::LogLevel)
   //   env_->getOptions()->findInt("julia_log_level")->getValue());
  //getOptionsFromEnv_(solver);
  //addOptions_();
}

JuliaInterface::JuliaInterface(Minotaur::Environment * env, int n, int m,
                               double *varLB, double *varUB, double *consLB, double *consUB,
                               int nele_jac, int nele_hess, int objSense,
                               bool isObj_nl, int n_obj)
 : intTol_(1e-8), zTol_(1e-8)
{
    myJulia_ = new JuliaProblemInfo;

	myJulia_->n = n;
	myJulia_->m = m;

	myJulia_->varLB = varLB;
	myJulia_->varUB = varUB;
	myJulia_->consLB = consLB;
	myJulia_->consUB = consUB;
	myJulia_->nele_jac = nele_jac;
	myJulia_->nele_hess = nele_hess;
	myJulia_->obj_sense = objSense;
	myJulia_->isobj_nl = isObj_nl;
	myJulia_->nb_obj = n_obj;

	// if the obj is linear 
	if (myJulia_->isobj_nl == false){
		ograd *og;
	}
}
JuliaInterface::~JuliaInterface()
{
  vars_.clear();
}

void JuliaInterface::setCallbacks(eval_f_cb eval_f, eval_g_cb eval_g, eval_grad_f_cb eval_grad_f,
				  eval_jac_g_cb eval_jac_g, eval_h_cb eval_h)
{
	// set evaluate and derivative function pointers
	myJulia_->eval_f_ = eval_f;		// evaluate objective function
	myJulia_->eval_g_ = eval_g;		// evaluate constraints 
	myJulia_->eval_grad_f_ = eval_grad_f;	// evaluate gradient of objective 
	myJulia_->eval_jac_g_ = eval_jac_g;	// evaluate jacobian of constraints
	myJulia_->eval_h_ = eval_h;		// evaluate hessian
}

void JuliaInterface::addVariablesFromJulia_(Minotaur::ProblemPtr instance)
{
	Minotaur::UInt stop_index, start_index;
	std::string vName;
	Minotaur::VariablePtr vPtr;
	Minotaur::VariableType vtype;


  	start_index = 0;
	stop_index = myJulia_->n;
	for (Minotaur::UInt i=start_index; i<stop_index; ++i) {
		// check how JuMP names a variable 
		vName = varName(i);
 		// determine the type of variable 
		switch (myJulia_->var_type[i]) {
		 case 'C':
			vtype = Minotaur::Continuous;
			break;
		 case 'B':
			vtype = Minotaur::Binary;
			break;
		 case 'I':
			vtype = Minotaur::Integer;
			break;
		}
		vPtr = instance->newVariable(myJulia_->varLB[i],
					     myJulia_->varUB[i], vtype, vName);
		vars_.push_back(vPtr);
	}
	
}


void JuliaInterface::addLinearConstraint_(int i, Minotaur::ProblemPtr instance)
{
	std::string cName;
	Minotaur::LinearFunctionPtr lfPtr = Minotaur::LinearFunctionPtr(); 	// NULL
	Minotaur::FunctionPtr fPtr;

	assert (i <= myJulia_->m); 
	cName = constName(i);

	addLinearTermsFromConstr_(lfPtr, i);
	fPtr = (Minotaur::FunctionPtr) new Minotaur::Function(lfPtr);

	instance->newConstraint(fPtr, myJulia_->consLB[i], myJulia_->consUB[i], cName);
}

std::string JuliaInterface::constName(int i)
{
	return std::string("i");
}

std::string JuliaInterface::varName(int i)
{
	return std::string("i");
}
void JuliaInterface::addLinearObjective_(int i, Minotaur::ProblemPtr instance)
{
	Minotaur::FunctionPtr fPtr;
	Minotaur::LinearFunctionPtr lfPtr = Minotaur::LinearFunctionPtr(); // NULL
	Minotaur::ObjectiveType obj_sense;
	std::string oName; 
	
	// add objective 
	assert (myJulia_->nb_obj < 2);	// assert the number of objective is less than 2 
	if (myJulia_->nb_obj > 0) {
		if (myJulia_->obj_sense == 1){
			obj_sense = Minotaur::Maximize;
		} else {
			obj_sense = Minotaur::Minimize;
		}
	
		addLinearTermsFromObj_(lfPtr, i);
		// different than before: don't create objective name while adding 
		instance->newObjective(fPtr, myJulia_->obj_const, obj_sense);
	}
	// add SOS type missing 
}

// if lfPtr is null, allocate memory and simply add terms from ASL. ASL can be
// trusted for no repititions. Otherwise, there are some existing terms in lf
// and we need to use lf->incTerm().
void JuliaInterface::addLinearTermsFromConstr_(Minotaur::LinearFunctionPtr & lfPtr,
												int i)
{
	// cgrad: constraint gradient (linear part) info 
}

void JuliaInterface::addLinearTermsFromObj_(Minotaur::LinearFunctionPtr & lf, 
                                           int i)
{
	// ograd: objective gradient (for linear parts)
	ograd *og; // for ampl
    
    /*if (lf) {
      for (og = myAsl_->i.Ograd_[i]; og; og = og->next) {
        lf->incTerm(vars_[og->varno], og->coef);
      }
    } else {
      lf= (Minotaur::LinearFunctionPtr) new Minotaur::LinearFunction();
      for (og = myAsl_->i.Ograd_[i]; og; og = og->next) {
        lf->addTerm(vars_[og->varno], og->coef);
      }
    }
    if (lf->getNumTerms()==0) {
      lf.reset();
    }*/
}

JuliaProblemInfo* JuliaInterface::getJulia()
{
	return myJulia_;
}

void JuliaInterface::freeJulia()
{
	// fill later 
}

Minotaur::ProblemPtr JuliaInterface::getInstance(std::vector<std::set<int> > &vids)
{	
	Minotaur::LinearFunctionPtr lfPtr;
	Minotaur::FunctionPtr fPtr;
	Minotaur::QuadraticFunctionPtr qfPtr = Minotaur::QuadraticFunctionPtr();	
	std::vector<Minotaur::VariableSet> vars;
	Minotaur::VariableSet vset;
	
	Minotaur::ObjectiveType obj_sense = Minotaur::Minimize;
	std::string cName, oName;

	Minotaur::UInt n = myJulia_->n; 	// set number of variables 
	
	Minotaur::UInt stop_index;
	Minotaur::UInt start_index;

	// new instance
	Minotaur::ProblemPtr instance = (Minotaur::ProblemPtr) new Minotaur::Problem();
	
	addVariablesFromJulia_(instance);
	nDefVars_ = 	0;
	nDefVarsBco_ =  0;
	nDefVarsCo1_ =  0;

	// convert vids to vars 
	Minotaur::VariablePtr v;
	for (std::vector<std::set<int> >::iterator it=vids.begin();
	     it!=vids.end(); ++it) {
		for (std::set<int>::iterator it2=it->begin(); it2!=it->end(); ++it2) {
			v = instance->getVariable(*it2);
			vset.insert(v);
		}	
		vars.push_back(vset);
		vset.clear();
	}

	// add constraints now
  
 	//
  	// Ordering of constraints by JuMP:
	// https://github.com/JuliaOpt/JuMP.jl/blob/master/doc/nlp.rst
  
  	//
  	// category           count
  	// -------------------------------------------------------------------------
  	// linear constraints  ... 
  	// quadratic constraints  ...
  	// nonlinear constraints ...
  	//
	
	// add linear constraints 
	start_index = 0;
	stop_index = myJulia_->nlc; // m: total number of constraints 
	for (Minotaur::UInt i=start_index; i<stop_index; ++i) {
		addLinearConstraint_(i, instance);
	}

	// add quadratic constraints 
	start_index = stop_index;
	stop_index = myJulia_->nqc + myJulia_->nnlc;
	for (Minotaur::UInt i=start_index; i<stop_index; ++i) {
		// fill in later 
		std::string("fill it");
	}	

	// add non-linear constraints  
	lfPtr = Minotaur::LinearFunctionPtr();	// NULL 
	start_index = stop_index;	
	stop_index = myJulia_->m; // m:total number of constraints 	
	for (Minotaur::UInt i=start_index; i<stop_index; ++i) {
		std::string("fill it");
		// should have a method to generate a constraint name 
	}	// FILL OUT THIS SECTION.. once nonlinear data structure is figured out
	

	// add objective 
	if (myJulia_->obj_sense == 1) {
		obj_sense = Minotaur::Maximize;
	} else {
		obj_sense = Minotaur::Minimize;
	}

	if (myJulia_->isobj_nl){
		oName = std::string("MyJulia nlobj");
	} else {
		// linear objective 
		addLinearObjective_(0, instance);
	}

	return instance;
}

Minotaur::Problem *JuliaInterface::readInstance()
{
	Minotaur::Problem *instance;
	std::vector<std::set<int> > vars;

	if (nDefVars_<1) {
	// identify variables in each nonlinear constraint and objective
		saveNlVars_(vars);
	}
	Minotaur::ProblemPtr shared_instance;
	shared_instance = getInstance(vars);
	instance = shared_instance.get();
	return instance; 
}

Minotaur::Problem *JuliaInterface::readProblemInstance()
{
	Minotaur::Problem *instance;
	instance = readInstance();
	return instance;
}

void JuliaInterface::saveNlVars_(std::vector<std::set<int> > &vars)
{

}

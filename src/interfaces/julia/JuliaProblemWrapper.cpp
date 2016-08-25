/**
 * Gokce Kahvecioglu
 */

#define MINOTAUR_DEBUG

#include "JuliaProblemWrapper.h"
#include "JuliaInterface.h"

#include "MinotaurConfig.h"
//using namespace minotaur_julia;

#ifdef __cplusplus
extern "C"{
#endif
using namespace minotaur_julia;

/** create API environment */
Minotaur::MinotaurApiEnv *createEnv(void)
{
	return new Minotaur::MinotaurApiEnv;
}

/** free API environment */
void freeEnv(Minotaur::MinotaurApiEnv *env)
{
    if (env) {
        delete env;
        env = NULL;
    }
}

void setCallbacks(
        Minotaur::MinotaurApiEnv *ApiEnv, // pointer to julia interface
        eval_f_cb eval_f,                  // Callback function of objective function
        eval_g_cb eval_g,                  // Callback function of constraint body 
        eval_grad_f_cb eval_grad_f,                // Callback function of objective gradient 
        eval_jac_g_cb eval_jac_g,          // Callback function of constraint Jacobian 
        eval_h_cb eval_h)                  // Callback function of Lagrangian Hessian 
{
    if (ApiEnv->iface_ != NULL)
        ApiEnv->iface_->setCallbacks(eval_f, eval_g, eval_grad_f, eval_jac_g, eval_h);
    else
        std::cout << "JuliaInterface is NULL.\n";
}

void loadJuliaInterface(
        Minotaur::MinotaurApiEnv *ApiEnv,
        int nvars,
        int ncons,
        double *varLB,
        double *varUB,
        double *consLB,
        double *consUB,
        int nele_jac,
        int nele_hess,
        int obj_sense,  // Objective sense: maximization ot minimization
        bool isobj_nl,  // Is objective nonlinear
        int nb_obj) {
    if (ApiEnv->env_ == NULL)
        std::cerr << "Environment is null.\n";
    if (ApiEnv->iface_ != NULL) {
        /** TODO release interface */
    }
#ifdef MINOTAUR_DEBUG
    std::cout << nvars << std::endl;
    std::cout << ncons << std::endl;
    for (int i = 0; i < nvars; ++i)
        printf("varLB[%d] %+e, varUB[%d] %+e\n", i, varLB[i], i, varUB[i]);
    for (int i = 0; i < ncons; ++i)
        printf("consLB[%d] %+e, consUB[%d] %+e\n", i, consLB[i], i, consUB[i]);
    std::cout << nele_jac << std::endl;
    std::cout << nele_hess << std::endl;
    std::cout << obj_sense << std::endl;
    std::cout << nb_obj << std::endl;
#endif
    ApiEnv->iface_ = new JuliaInterface(ApiEnv->env_, nvars, ncons,
                                        varLB, varUB, consLB, consUB, nele_jac, nele_hess,
                                        obj_sense, isobj_nl, nb_obj);
#ifdef MINOTAUR_DEBUG
    std::cout << "ApiEnv->iface_ " << ApiEnv->iface_ << std::endl;
#endif
}

int solveProblem(JuliaInterfacePtr iface,
		 double* obj_val, double* x, UserDataPtr prob)
{
	return 0;
}

void loadProblem(Minotaur::MinotaurApiEnv *ApiEnv) 
{
	ApiEnv->problem_ = ApiEnv->iface_->readProblemInstance(); 

}
#ifdef __cplusplus
}
#endif

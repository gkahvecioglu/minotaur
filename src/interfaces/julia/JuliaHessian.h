#ifndef JULIAHESSIAN_H
#define JULIAHESSIAN_H 

#include "HessianOfLag.h"
#include "JuliaProblemWrapper.h"

namespace minotaur_julia {

typedef struct ProblemInfo JuliaProblemInfo;

class JuliaInterface;
typedef JuliaInterface* JuliaInterfacePtr;

class JuliaHessian: public Minotaur::HessianOfLag {

public:
    JuliaHessian(JuliaInterfacePtr iface);
    
    ~JuliaHessian(); 

    Minotaur::UInt getNumNz() const;

    void fillRowColIndices(Minotaur::UInt *iRow, Minotaur::UInt *jCol);

    void fillRowColValues(double *x, double obj_mult, double *con_mult, double *values, int *error);

    void negateObj();
private:
    
    // pointer to julia problem info 
    JuliaProblemInfo *myJulia_;
   
    /// No. of nonzeros in Hessian of Lagrangian. Initialized in the constructor.
    Minotaur::UInt nNz_;

    bool negObj_;
    
    /** Pointer to callback function evaluating Hessian of Lagrangian */
    eval_h_cb eval_h_;

    /* Pointer to user data*/
    UserDataPtr userdata_;
};
typedef boost::shared_ptr<JuliaHessian> JuliaHessianPtr;
} //namespace

#endif  

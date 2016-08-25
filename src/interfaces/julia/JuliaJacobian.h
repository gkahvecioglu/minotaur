#ifndef JULIAJACOBIAN_H
#define JULIAJACOBIAN_H

#include "Jacobian.h"
#include "JuliaProblemWrapper.h"

namespace minotaur_julia {

typedef struct ProblemInfo JuliaProblemInfo;
class JuliaInterface;
typedef JuliaInterface* JuliaInterfacePtr;

class JuliaJacobian: public Minotaur::Jacobian {
public:
 JuliaJacobian(JuliaInterfacePtr iface);

 //Destroy 
 ~JuliaJacobian();

 void fillColRowIndices(Minotaur::UInt *jcol, Minotaur::UInt *irow);

 void fillRowColIndices(Minotaur::UInt *irow, Minotaur::UInt *jcol);

 void fillColRowValues(const double *x, double *values, int *error);

 void fillRowColValues(const double *x, double *values, int *error);

// Return the number of non-zeros in the Jacobian 
 Minotaur::UInt getNumNz();

private:
 // pointer to problem info in Julia 
 JuliaProblemInfo* myJulia_;

 // number of nonzeros in Jacobian. Initialized in the constructor 
 Minotaur::UInt nNz_;

 // temporary array used to save Jacobian in fillRowColValues() function
 double *tmp_;

 // size of tmp_ array 
 Minotaur::UInt tmpSize_;
};
typedef boost::shared_ptr<JuliaJacobian> JuliaJacobianPtr;

} //namespace

#endif

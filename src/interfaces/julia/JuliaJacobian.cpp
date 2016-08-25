#include "MinotaurConfig.h"
#include "JuliaInterface.h"
#include "JuliaJacobian.h"

using namespace minotaur_julia;

JuliaJacobian::JuliaJacobian(JuliaInterfacePtr iface)
   : myJulia_(iface->getJulia()),
     tmp_(0),
     tmpSize_(0)
{
 nNz_ = myJulia_->nele_jac; // number of nonzeros in Jacobian 
}

JuliaJacobian::~JuliaJacobian()
{
	if (tmp_){
	   delete [] tmp_;
	}
}

Minotaur::UInt JuliaJacobian::getNumNz()
{
    return nNz_;
}

void JuliaJacobian::fillColRowIndices(Minotaur::UInt *jcol, Minotaur::UInt *irow)
{
}

void JuliaJacobian::fillColRowValues(const double *x, 
                                    double *values, int *error)
{
}

void JuliaJacobian::fillRowColIndices(Minotaur::UInt *irow, Minotaur::UInt *jcol)
{
}

void JuliaJacobian::fillRowColValues(const double *x, 
                                    double *values, int *error)
{
}

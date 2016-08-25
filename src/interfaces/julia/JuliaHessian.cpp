#include "MinotaurConfig.h"
#include "JuliaHessian.h"
#include "JuliaInterface.h"

using namespace minotaur_julia; 

JuliaHessian::JuliaHessian(JuliaInterfacePtr iface)
    :myJulia_(iface->getJulia()),
     negObj_(false),
     nNz_(myJulia_->nele_hess),
     eval_h_(myJulia_->eval_h_), 
     userdata_(myJulia_->userdata_)	
{
}

JuliaHessian::~JuliaHessian()
{
}

void JuliaHessian::fillRowColIndices(Minotaur::UInt *iRow, Minotaur::UInt *jCol)
{
	// Ihess and Jhess, hesslag_structure are required !! lines 80-81-82
	//iRow = 1;
	// jCol = 1;
}

void JuliaHessian::fillRowColValues(double *x, double obj_mult,
                                    double *con_mult, double *values,
                                    int *error)
{
	// ccallback function accepts integer, did not accept MInotaur::UInt 
	int *iRow;
	int *jCol;
	// fillRowColIndices(iRow, jCol);
	(*eval_h_)(x, con_mult, values, iRow, jCol, userdata_);
}

Minotaur::UInt JuliaHessian::getNumNz() const
{
  return nNz_;
}

void JuliaHessian::negateObj()
{
  negObj_ = !negObj_;
}


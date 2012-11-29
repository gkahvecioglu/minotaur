// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2010 - 2012 The MINOTAUR Team.
// 

/**
 * \file BqpdEngine.cpp
 * \brief Define the class BqpdEngine.
 * \author Sven Leyffer, Argonne National Laboratory
 * 
 * Implement the interface to bqpd engine and provide warm-starting
 * capability.
 * 
 */

#include <cmath>
#include <iomanip>
#include <iostream>
#include <string.h> // for memset

#include "MinotaurConfig.h"
#include "BqpdEngine.h"
#include "BqpdEngineTypes.h"
#include "Constraint.h"
#include "Environment.h"
#include "Function.h"
#include "HessianOfLag.h"
#include "LinearFunction.h"
#include "Logger.h"
#include "Objective.h"
#include "Option.h"
#include "QuadraticFunction.h"
#include "Problem.h"
#include "ProblemSize.h"
#include "Solution.h"
#include "Timer.h"
#include "Variable.h"

using namespace Minotaur;

BqpdEngine::BqpdEngine()
  : bndRelaxed_(false),
    bndChanges_(0),
    bTol_(1e-9),
    chkPt_(0),
    consModed_(false),
    dualCons_(0),
    dualX_(0),
    env_(EnvPtr()),
    fStart_(0),
    infty_(1e20),
    maxIterLimit_(1000),
    prevStrBr_(false),
    resolveError_(true),
    sol_(SolutionPtr()),    // NULL
    stats_(0),
    strBr_(false),
    timer_(0),
    wsMode_(6)
{
#ifndef USE_BQPD 
#error Need to set USE_BQPD
#endif
  status_ = EngineError;
  logger_ = (LoggerPtr) new Logger(LogInfo);
  iterLimit_ = maxIterLimit_;
  problem_ = ProblemPtr(); // NULL
}


BqpdEngine::BqpdEngine(EnvPtr env)
  : bndRelaxed_(false),
    bndChanges_(0),
    bTol_(1e-9),
    chkPt_(0),
    consModed_(false),
    dualCons_(0),
    dualX_(0),
    env_(env),
    fStart_(0),
    infty_(1e20),
    maxIterLimit_(1000),
    prevStrBr_(false),
    resolveError_(true),
    sol_(SolutionPtr()),
    strBr_(false),
    timer_(0)
{
  wsMode_ = env->getOptions()->findInt("bqpd_ws_mode")->getValue();
  status_ = EngineError;
  logger_ = (LoggerPtr) new Logger((LogLevel) (env->getOptions()
        ->findInt("bqpd_log_level")->getValue()));
  iterLimit_ = maxIterLimit_;
  timer_ = env->getNewTimer();
  stats_ = new BqpdStats();
  stats_->calls    = 0;
  stats_->strCalls = 0;
  stats_->time     = 0;
  stats_->strTime  = 0;
  stats_->cTime    = 0;
  stats_->iters    = 0;
  stats_->strIters = 0;
  problem_ = ProblemPtr(); // NULL

  assert(wsMode_<7 && wsMode_>0);
}


BqpdEngine::~BqpdEngine()
{
  //delete ;
  if (sol_) {
    sol_.reset();
  }
  if (problem_) {
	 problem_->unsetEngine();
    problem_.reset();
  }
  if (fStart_) {
    delete fStart_;
  }
  if (timer_) {
    delete timer_;
  }
  if (stats_) {
    delete stats_;
  }
  if (dualX_) {
    delete [] dualX_;
  }
  if (dualCons_) {
    delete [] dualCons_;
  }
}


void BqpdEngine::addConstraint(ConstraintPtr)
{
  consModed_ = true;
}


EnginePtr BqpdEngine::emptyCopy()
{
  if (env_) {
    return (BqpdEnginePtr) new BqpdEngine(env_);
  }
  return (BqpdEnginePtr) new BqpdEngine();
}


void BqpdEngine::load(ProblemPtr problem)
{
  problem_ = problem;
  consModed_ = true;
  bndRelaxed_ = true;
  bndChanges_ = 0;
  problem->setEngine(this);
}

void BqpdEngine::clear() {

  if (problem_) {
    problem_->unsetEngine();
    problem_.reset();
  }
  if (fStart_) {
    delete fStart_;
    fStart_=0;
  }
}


void BqpdEngine::load_()
{
  Int n       = problem_->getNumVars();
  Int m       = problem_->getNumCons();
  Int mlp     = 1000;      // Max level of degeneracy. 
  problem_->prepareForSolve(); // important before calling getNumJacNnzs.
  Int mxwk0   = 50000+10*(problem_->getNumJacNnzs()+n+m);
  Int mxiwk0  = 500000;    // Initial workspace
  Int kmax    = 500;       // Maximum value of k. set kmax= 0 iff LP.
  Int lh1     = problem_->getNumHessNnzs() + 8 + 2*n + m;
  Int mxwk    = 21*n + 8*m + mlp + lh1 + kmax*(kmax+9)/2 + mxwk0;
  Int mxiwk   = 13*n + 4*m + mlp + lh1 + kmax + 113 + mxiwk0;
  Int kk0, ll0;           // For setwsc routine.
  UInt maxa   = n + problem_->getSize()->linTerms;
  logger_->MsgStream(LogDebug1) << "Bqpd: Loaded problem." << std::endl;
  //std::cout << "mxwk3 = " << mxwk << std::endl;

  // set storage map for bqpd
  kk0 = problem_->getHessian()->getNumNz() + 2;
  ll0 = problem_->getHessian()->getNumNz() + 3 + n;
  setwsc(&mxwk, &mxiwk, &kk0, &ll0);

  sol_ = (SolutionPtr) new Solution(INFINITY, 0, problem_);
  if (dualX_) {
    delete [] dualX_;
    delete [] dualCons_;
  }
  dualX_ = new Double[n];
  dualCons_ = new Double[m];
  fStart_ = new BqpdData(n, m, maxa, lh1, problem_->getNumJacNnzs());

  logger_->MsgStream(LogDebug2) << "Bqpd: " << "maxa = " << maxa 
    << std::endl;
}


void BqpdEngine::removeCons(std::vector<ConstraintPtr> &)
{
  consModed_ = true;
  if (true==strBr_ && chkPt_) {
    delete chkPt_;
    chkPt_ = 0;
  }
}


void BqpdEngine::setGradient_()
{
  UInt n = problem_->getNumVars();
  UInt maxa = n + problem_->getSize()->linTerms;
  UInt cnt = 1, ptr_pos = maxa+1;
  UInt m = problem_->getNumCons();
  Double *values;       // gradient values.
  Double *x;
  Int *la = fStart_->la;
  Double *a  = fStart_->a;
  Int error = 0;
  //a[0] = 0;
  
  ObjectivePtr oPtr = problem_->getObjective();
  assert(oPtr);
  LinearFunctionPtr lf;
  memset(la, 0, (maxa+m+3)*sizeof(fint));
  la[0] = maxa+1;
  la[ptr_pos] = 1;
  ++ptr_pos;

  // first do objective
  values = new Double[n];
  x = new Double[n];
  memset(values, 0, n*sizeof(Double));
  memset(x, 0, n*sizeof(Double));

  objOff_ = oPtr->eval(x, &error);
  if (oPtr) {
    oPtr->evalGradient(x, values, &error); 
    assert(0==error);
  }
  for (UInt i=0; i<n; ++i) {
    a[cnt-1] = values[i];
    la[cnt] = i+1; // offset by 1 since x[0] is really x[1] in
    ++cnt;
  }
  la[ptr_pos] = cnt;
  ++ptr_pos;

  // now do the constraints
  for (ConstraintConstIterator it2=problem_->consBegin(); 
      it2!=problem_->consEnd(); ++it2) {
    lf = (*it2)->getLinearFunction();
    if (lf) {
      for (VariableGroupConstIterator it=lf->termsBegin(); it!=lf->termsEnd(); 
          ++it) {
        a[cnt-1] = it->second;
        la[cnt] = it->first->getIndex()+1; // offset by 1 
        ++cnt;
      }
    } 
    la[ptr_pos] = cnt;
    ++ptr_pos;
  }
  delete [] values;
  delete [] x;
  assert(cnt==maxa+1);
  assert(ptr_pos==maxa+m+3);
}


void BqpdEngine::setHessian_()
{
  UInt n        = 0;    // Number of variables in problem.
  UInt m        = 0;    // Number of constraints in problem.
  UInt *iRow, *jCol;    // Indices of nonzeros in Hessian of objective.
  Double *values;       // Hessian values.
  UInt hess_nnzs = 0;   // Number of nonzeros in Hessian of objective.
  Double *con_mult, *x;
  Double obj_mult = 1.;
  Int *lws = fStart_->lws;
  Double *ws = fStart_->ws;
  Int error = 0;
  
  n = problem_->getNumVars();
  m = problem_->getNumCons();

  hess_nnzs = problem_->getHessian()->getNumNz();
  lws[0]    = 1+hess_nnzs;
  ws[0]     = 0;
  iRow      = new UInt[hess_nnzs];
  jCol      = new UInt[hess_nnzs];

  // we get a lower triangular matrix which is ordered by row. This is same as
  // upper triangular matrix which is ordered by column. This is what
  // BqpdAux.f wants! yay! Hence we switch iRow and jCol, but only in the next
  // statement and nowhere else.
  problem_->getHessian()->fillRowColIndices(jCol, iRow) ;
  con_mult =  new Double[m];
  x =  new Double[n];
  memset(x, 0, n*sizeof(Double));
  memset(con_mult, 0, m*sizeof(Double));
  values = new Double[hess_nnzs];
  problem_->getHessian()->fillRowColValues(x, obj_mult, con_mult, values, 
      &error);
  assert(0==error);

  for (UInt i=0; i<hess_nnzs; ++i) {
    ++iRow[i];
    ++jCol[i];
    //std::cout << iRow[i] << ", " << jCol[i] << std::endl;
  }

#if DEBUG
  if (hess_nnzs>0) {
    for (UInt i=0; i<hess_nnzs-1; ++i) {
      assert(jCol[i] <= jCol[i+1]);
    }
  }
#endif

  //std::copy(iRow, iRow + hess_nnzs, lws+1);
  //std::copy(jCol, jCol + hess_nnzs, lws+1+hess_nnzs);

  UInt curr_col = 1;
  UInt fill_pos = 1;
  lws[hess_nnzs+curr_col] = fill_pos;
  for (UInt i=0; i<hess_nnzs; ++i) {
    while (jCol[i] > curr_col) {
      ++curr_col;
      lws[hess_nnzs+curr_col] = fill_pos;
    }
    lws[fill_pos] = iRow[i];
     ws[fill_pos] = values[i]; // ws and lws are not off by one, 
                               // unlike la_ and a. such is life.
    ++fill_pos;
  }
  while (curr_col < n) {
    ++curr_col;
    lws[hess_nnzs+curr_col] = fill_pos;
  }
  ++curr_col;
  lws[hess_nnzs+curr_col] = fill_pos;

  //for (UInt i=0; i<2+hess_nnzs+n; ++i) {
  //  std::cout << "lws[" << i << "] = " << lws[i] << std::endl;
  //}
  //for (UInt i=0; i<hess_nnzs+1; ++i) {
  //  std::cout << "ws[" << i << "] = " << ws[i] << std::endl;
  //}

  delete [] iRow;
  delete [] jCol;
  delete [] con_mult;
  delete [] x;
  delete [] values;
}


EngineStatus BqpdEngine::solve()
{
  real f        = 1.E20;   // Solution value.
  Int mode      = 0;

  if (consModed_ == true) {
    // redo from scratch
    if (fStart_) {
      delete fStart_;
    }
    timer_->start();
    load_();
    stats_->cTime += timer_->query();
    timer_->stop();
    mode = 1;
    // set gradient
    setGradient_();
    // set hessian
    setHessian_();
    // set initial point
    setInitialPoint_();

    // set bounds on constraints.
    setConsBounds_();
  } else if (chkPt_) {
    mode = wsMode_;
  } else if (false == bndRelaxed_ && bndChanges_ < 3) {
    mode = wsMode_;
  } else if (wsMode_ > 2) {
    mode = 2;
  } else {
    mode = wsMode_;
  }
  consModed_ = false;
  bndRelaxed_ = false;
  bndChanges_ = 0;

  // set bounds on variables
  setVarBounds_();

  solve_(mode, f);
  if (EngineError == status_ && resolveError_) {
    logger_->MsgStream(LogInfo) 
      << "Bqpd: failure in solving in mode " << mode << std::endl;
    switch(mode) {
     case 0:
       mode = 1;
       break;
     case 1:
       mode = 0;
       break;
     default:
      mode = 1;
    }
    logger_->MsgStream(LogInfo) 
      << "Bqpd: solving in mode " << mode << std::endl;
    solve_(mode,f);
    if (EngineError == status_) {
      logger_->MsgStream(LogInfo) 
        << "Bqpd: failure in mode " << mode << " as well." << std::endl;
      //for (UInt i=0; i<problem_->getNumVars(); ++i) {
      //  problem_->changeBound(i,fStart_->bl[i],fStart_->bu[i]);
      //}
      //problem_->write(std::cout,20);
      //exit(0);
    }
  }

  // store the solution
  storeSol_(f);

  if (chkPt_) {
    timer_->start();
    restorecommon();
    if (fStart_) {
      fStart_->copyFrom(chkPt_);
    } else {
      fStart_ = chkPt_->clone();
    }
    stats_->cTime += timer_->query();
    timer_->stop();
    prevStrBr_ = true;
  } else {
    prevStrBr_ = false;
  }

  return status_;
}


void BqpdEngine::solve_(Int mode, Double &f)
{
  fint n        = problem_->getNumVars();  // numbver of variables
  fint m        = problem_->getNumCons();  // number of constraints
  fint kmax     = 500;     // Maximum value of k. set kmax= 0 iff LP.
  real fmin     = -1.E20;  // Lower bnd on objective, for unboundedness.
  fint mlp      = 1000;    // Max level of degeneracy. 
  fint ifail    = 0;       // Outcome status message.
  fint iprint   = 0;       // Level of output
  fint nout     = 6;       // 6 for output to stdout, 7 for no output

  // set the status of the engine to unknown.
  status_ = EngineUnknownStatus;
  f       = 1.E20;   

#if DEBUG
  logger_->MsgStream(LogDebug2) 
    << "Bqpd:    n = " << n << std::endl
    << "Bqpd:    m = " << m << std::endl
    << "Bqpd: mode = " << mode << std::endl
    << "Bqpd: kmax = " << kmax << std::endl
    << "Bqpd: solve no. = " << stats_->calls+1 << std::endl;
#endif

  // solve QP by calling bqpd. x contains the final solution. f contains
  // the objective value.
  timer_->start();
  bqpd_(&n, &m, &(fStart_->k), &kmax, fStart_->a, fStart_->la, fStart_->x,
      fStart_->bl, fStart_->bu, &f, &fmin, fStart_->g, fStart_->r,
      fStart_->w, fStart_->e, fStart_->ls, fStart_->alp, fStart_->lp,
      &mlp, &(fStart_->peq), fStart_->ws, fStart_->lws, &mode, &ifail,
      fStart_->info, &iprint, &nout);
#if DEBUG
  logger_->MsgStream(LogDebug2) 
    << "Bqpd:    iters = " << fStart_->info[0] << std::endl;
#endif
  
  if (true == strBr_) {
    stats_->strCalls += 1;
    stats_->strTime  += timer_->query();
    stats_->strIters += fStart_->info[0];
  } 
  stats_->calls += 1;
  stats_->time  += timer_->query();
  stats_->iters += fStart_->info[0];

  timer_->stop();
  //writewsc();

#if DEBUG
  logger_->MsgStream(LogDebug) << "Bqpd: ifail = " << ifail << std::endl;
#endif

  // set return status from Bqpd
  switch (ifail) {
   case(0):
    status_ = ProvenLocalOptimal;
    break;
   case(1):
    status_ = ProvenUnbounded;
    break;
   case(2):
    status_ = ProvenInfeasible;
    break;
   case(3):
    status_ = ProvenLocalInfeasible;
    break;
   case(4):
    status_ = ProvenFailedCQFeas;
    assert(!"check for status_ ProvenFailedCQFeas");
    break;
   case(5):
    status_ = EngineError;
    break;
   case(6):
    status_ = EngineIterationLimit;
    break;
   default:
    status_ = EngineError;
  }
#if DEBUG
  logger_->MsgStream(LogDebug) << "Bqpd: status = " << getStatusString() 
    << std::endl;
  logger_->MsgStream(LogDebug) << "Bqpd: value = " << f << std::endl;
#endif
}


void BqpdEngine::storeSol_(Double f)
{
  UInt index;
  UInt n = fStart_->n;
  UInt m = fStart_->m;
  Int  k = fStart_->k;
  Int sign = 0;
  const Double *r = fStart_->r;

  sol_->setPrimal(fStart_->x);
  sol_->setObjValue(f+objOff_);
  // dual:
  // n = #vars
  // k = output from bqpd = dimension of null-space
  // ls(1:n-k), integer, are the indices for dual values.
  // i.e. |ls[i]| is the index. If it is greater than n, then it is the index
  // of constraint. Otherwise, it is index of variable.
  // r[|ls[i]|]*sign(ls[i]) is the dual value.
  //
  // from bqpd.f
  //  ls(n+m) stores indices of the active constraints in locations 1:n-k and of
  //  the inactive constraints in locations n-k+1:n+m. The simple bounds
  //  on the variables are indexed by 1:n and the general constraints by
  //  n+1:n+m. The sign of ls(j) indicates whether the lower bound (+) or
  //  the upper bound (-) of constraint ls(j) is currently significant.
  //  Within the set of active constraints, locations 1:peq store the
  //  indices of active equality constraints, and indices peq+1:lp(1)-1
  //  are indices of any pseudo-bounds (active constraints which are not
  //  at their bound). If mode>=2, the first n-k elements of ls must be set
  //  on entry
  memset(dualX_, 0, n * sizeof(Double));
  memset(dualCons_, 0, m * sizeof(Double));
  if (status_ != ProvenInfeasible && status_ != EngineError) {
    for (UInt i=0; i<n-k; ++i) {
      index = (UInt) fabs(fStart_->ls[i]);
      assert(index != 0);
      --index;
      if (fStart_->ls[i] > 0) {
        sign = 1;
      } else {
        sign = -1;
      }
      if (index<n) {
        dualX_[index] = sign*r[index]; 
      } else {
        dualCons_[index-n] = sign*r[index]; 
      }
    }
  }
  sol_->setDualOfVars(dualX_);
  sol_->setDualOfCons(dualCons_);
}


void BqpdEngine::setInitialPoint_()
{
  const Double *initial_point = problem_->getInitialPoint();
  if (initial_point) {
    std::copy(initial_point, initial_point + problem_->getNumVars(), fStart_->x);
  } else {
    // start with zero.
    memset(fStart_->x, 0, problem_->getNumVars() * sizeof(Double));
  }
}


void BqpdEngine::setVarBounds_()
{
  VariablePtr vPtr;
  VariableConstIterator vIter;
  Double l, u;
  Double *bl = fStart_->bl;
  Double *bu = fStart_->bu;

  for (vIter=problem_->varsBegin(); vIter!=problem_->varsEnd(); 
      ++vIter, ++bl, ++bu) {
    vPtr = *vIter;
    l = std::max(-infty_, vPtr->getLb());
    u = std::min( infty_, vPtr->getUb());
    if (l>u && l<u+bTol_) {
      u = l+bTol_;
    }
    *bl = l;
    *bu = u;
  }
}


void BqpdEngine::setConsBounds_()
{
  ConstraintPtr cPtr;
  ConstraintConstIterator cIter;
  Int i= problem_->getNumVars();
  Double *bl = fStart_->bl+i;
  Double *bu = fStart_->bu+i;
  Double l, u;

  for (cIter=problem_->consBegin(); cIter!=problem_->consEnd(); 
      ++cIter, ++bl, ++bu) {
    cPtr = *cIter;
    l = std::max(-infty_, cPtr->getLb());
    u = std::min( infty_, cPtr->getUb());
    if (l>u && l<u+bTol_) {
      u = l+bTol_;
    }
    *bl = l;
    *bu = u;
  }
}


Double BqpdEngine::getSolutionValue() 
{
  if (sol_) {
    return sol_->getObjValue();
  } else {
    return INFINITY;  // throw exception instead
  }
}


ConstSolutionPtr BqpdEngine::getSolution() 
{
  if (sol_) {
    return sol_;
  } 
  return SolutionPtr(); // NULL
}


EngineStatus BqpdEngine::getStatus() 
{
  return status_;
}
  

void BqpdEngine::changeBound(ConstraintPtr, BoundType, Double)
{
  consModed_ = true;
}


void BqpdEngine::changeBound(VariablePtr var, BoundType lu, Double new_val)
{
  // no need to do anything because the 'solve' function reloads bounds from
  // problem.
  // assert(!"implement me!");
  // bug here: if fStart_ is not available, this function does nothing.
  if (fStart_) {
    if ((lu == Lower && new_val<fStart_->bl[var->getIndex()]) || 
        (lu == Upper && new_val>fStart_->bu[var->getIndex()])) {
      if (!chkPt_) {
        bndRelaxed_ = true;
      }
    }
  }
  ++bndChanges_;
}


void BqpdEngine::changeBound(VariablePtr var, Double new_lb, 
    Double new_ub)
{
  // no need to do anything because the 'solve' function reloads bounds from
  // problem.
  // assert(!"implement me!");
  // bug here: if fStart_ is not available, this function does nothing.
  if (fStart_) {
    if (new_lb<fStart_->bl[var->getIndex()] ||
        new_ub>fStart_->bu[var->getIndex()]) {
      if (!chkPt_) {
        bndRelaxed_ = true;
      }
    }
  }
  bndChanges_ += 2;
}


void BqpdEngine::changeObj(FunctionPtr, Double)
{
  consModed_ = true;
  if (true==strBr_ && chkPt_) {
    delete chkPt_;
    chkPt_ = 0;
  }
}


void BqpdEngine::negateObj()
{
  consModed_ = true;
  if (true==strBr_ && chkPt_) {
    delete chkPt_;
    chkPt_ = 0;
  }
}


void BqpdEngine::changeConstraint(ConstraintPtr, LinearFunctionPtr, 
                                  Double , Double)
{
  consModed_ = true;
  if (true==strBr_ && chkPt_) {
    delete chkPt_;
    chkPt_ = 0;
  }
}


void BqpdEngine::enableStrBrSetup()
{
  strBr_ = true;
  if (wsMode_>2) {
    timer_->start();
    chkPt_ = fStart_->clone();
    savecommon();
    stats_->cTime += timer_->query();
    timer_->stop();
  }
  prevStrBr_ = false;
}


void BqpdEngine::disableStrBrSetup()
{
  if (chkPt_) {
    delete chkPt_;
    chkPt_ = 0;
  }
  if (prevStrBr_) {
    bndRelaxed_ = true; // otherwise gives wrong answers for st_test5.nl in bnb
  }
  strBr_ = false;
}


void BqpdEngine::setIterationLimit(Int limit)
{
  // TODO: this limit is never passed to bqpd. Fix it.
  if (limit<1) {
    limit = maxIterLimit_;
  }
  iterLimit_ = limit;
}


void BqpdEngine::resetIterationLimit()
{
  iterLimit_ = maxIterLimit_;
}


void BqpdEngine::writeStats()
{
  if (stats_) {
    std::string me = "Bqpd:  ";
    logger_->MsgStream(LogInfo) 
      << me << "total calls            = " << stats_->calls << std::endl
      << me << "strong branching calls = " << stats_->strCalls << std::endl
      << me << "total time in solving  = " << stats_->time  << std::endl
      << me << "time in str branching  = " << stats_->strTime << std::endl
      << me << "time in copying data   = " << stats_->cTime << std::endl
      << me << "total iterations       = " << stats_->iters << std::endl
      << me << "strong br iterations   = " << stats_->strIters << std::endl;
  }
}


std::string BqpdEngine::getName() const
{
  return "Bqpd";
}


// ----------------------------------------------------------------------- //
// End of BqpdEngine Class.
// ----------------------------------------------------------------------- //
BqpdData::BqpdData(UInt n_t, UInt m_t, UInt maxa_t, UInt lh1_t, 
                   UInt nJac_t, Bool zero)
 : n(n_t),
   m(m_t),
   lh1(lh1_t),
   nJac(nJac_t),
   maxa(maxa_t),
   peq(0),
   k(0)
{
  UInt nm     = n+m;
  UInt kmax   = 500;       // Maximum value of k. set kmax= 0 iff LP.
  UInt mlp    = 1000;      // Max level of degeneracy. 
  UInt mxwk0  = 50000+10*(nJac+n+m);
  UInt mxiwk0 = 500000;    // Initial workspace
  UInt mxwk   = 21*n + 8*m + mlp + lh1 + kmax*(kmax+9)/2 + mxwk0;
  UInt mxiwk  = 13*n + 4*m + mlp + lh1 + kmax + 113 + mxiwk0;
  //std::cout << "mxwk2 = " << mxwk << std::endl;


  x     = new real[n];   // primal solution.
  r     = new real[nm];  // residuals/multipliers
  e     = new real[nm];  // steepest-edge normalization coefficients 
  w     = new real[nm];  // denominators for ratio tests
  g     = new real[n];   // gradient vector of f(x)
  ls    = new fint[nm];  // indices of the active constraints 
  alp   = new real[mlp]; // workspace associated with recursion
  lp    = new fint[mlp]; // workspace associated with recursion
  info  = new fint[10];  // information on return from bqpd
  bl    = new real[nm];  // lower bounds for variables and constraints
  bu    = new real[nm];  // upper bounds for variables and constraints

  // allocate storage for Hessian etc, and zero them out
  ws    = new real[mxwk];  // workspace
  lws   = new fint[mxiwk]; // workspace

  // storage for gradient and jacobian
  a     = new real[maxa+1];    // linear part of objective & Jacobian
  la    = new fint[maxa+m+3];

  if (true==zero) {
    memset(info, 0, 10 * sizeof(Int));
    memset(ws, 0, mxwk*sizeof(real));
    memset(lws, 0, mxiwk*sizeof(fint));
  }
}


BqpdData* BqpdData::clone()
{
  BqpdData* lhs = new BqpdData(n, m, maxa, lh1, nJac, false);
  UInt nm = n+m;
  UInt kmax   = 500;       // Maximum value of k. set kmax= 0 iff LP.
  UInt mlp    = 1000;      // Max level of degeneracy. 
  UInt mxwk0  = 50000+10*(nJac+n+m);
  UInt mxiwk0 = 500000;    // Initial workspace
  UInt mxwk   = 21*n + 8*m + mlp + lh1 + kmax*(kmax+9)/2 + mxwk0;
  UInt mxiwk  = 13*n + 4*m + mlp + lh1 + kmax + 113 + mxiwk0;
  //std::cout << "mxwk1 = " << mxwk << std::endl;

  lhs->peq = peq;
  lhs->k   = k;

  std::copy(x,    x    +  n,        lhs->x);
  std::copy(r,    r    +  nm,       lhs->r);
  std::copy(e,    e    +  nm,       lhs->e);
  std::copy(w,    w    +  nm,       lhs->w);
  std::copy(g,    g    +  n,        lhs->g);
  std::copy(ls,   ls   +  nm,       lhs->ls);
  std::copy(alp,  alp  +  mlp,      lhs->alp);
  std::copy(lp,   lp   +  mlp,      lhs->lp);
  std::copy(info, info +  10,       lhs->info);
  memset(info, 0, 10*sizeof(fint));
  std::copy(bl,   bl   +  nm,       lhs->bl);
  std::copy(bu,   bu   +  nm,       lhs->bu);
  //std::copy(ws,   ws   +  mxwk,     lhs->ws);
  //std::copy(lws,  lws  +  mxiwk,    lhs->lws);
  memcpy(lhs->ws, ws, mxwk*sizeof(real));
  memcpy(lhs->lws, lws, mxiwk*sizeof(fint));
  std::copy(a,    a    +  maxa+1,   lhs->a);
  std::copy(la,   la   +  maxa+m+3, lhs->la);

  return lhs;
}


void BqpdData::copyFrom(const BqpdData* rhs)
{
  UInt nm = n+m;
  UInt kmax   = 500;       // Maximum value of k. set kmax= 0 iff LP.
  UInt mlp    = 1000;      // Max level of degeneracy. 
  UInt mxwk0  = 50000+10*(nJac+n+m);
  UInt mxiwk0 = 500000;    // Initial workspace
  UInt mxwk   = 21*n + 8*m + mlp + lh1 + kmax*(kmax+9)/2 + mxwk0;
  UInt mxiwk  = 13*n + 4*m + mlp + lh1 + kmax + 113 + mxiwk0;
  //std::cout << "mxwk4 = " << mxwk << std::endl;

  peq = rhs->peq;
  k   = rhs->k;

  std::copy(rhs->x,    rhs->x    +  n,        x);
  std::copy(rhs->r,    rhs->r    +  nm,       r);
  std::copy(rhs->e,    rhs->e    +  nm,       e);
  std::copy(rhs->w,    rhs->w    +  nm,       w);
  std::copy(rhs->g,    rhs->g    +  n,        g);
  std::copy(rhs->ls,   rhs->ls   +  nm,       ls);
  std::copy(rhs->alp,  rhs->alp  +  mlp,      alp);
  std::copy(rhs->lp,   rhs->lp   +  mlp,      lp);
  memset(rhs->info, 0, 10*sizeof(fint));
  std::copy(rhs->bl,   rhs->bl   +  nm,       bl);
  std::copy(rhs->bu,   rhs->bu   +  nm,       bu);
  std::copy(rhs->ws,   rhs->ws   +  mxwk,     ws);
  std::copy(rhs->lws,  rhs->lws  +  mxiwk,    lws);
  std::copy(rhs->a,    rhs->a    +  maxa+1,   a);
  std::copy(rhs->la,   rhs->la   +  maxa+m+3, la);
}


BqpdData::~BqpdData()
{
  if (x) {
    delete [] x;
    x = 0;
  }
  if (r) {
    delete [] r;
    r = 0;
  }
  if (e) {
    delete [] e;
    e = 0;
  }
  if (w) {
    delete [] w;
    w = 0;
  }
  if (g) {
    delete [] g;
    g = 0;
  }
  if (ls) {
    delete [] ls;
    ls = 0;
  }
  if (alp) {
    delete [] alp;
    alp = 0;
  }
  if (lp) {
    delete [] lp;
    lp = 0;
  }
  if (info) {
    delete [] info;
    info = 0;
  }
  if (bl) {
    delete [] bl;
    bl = 0;
  }
  if (bu) {
    delete [] bu;
    bu = 0;
  }
  if (ws) {
    delete [] ws;
    ws = 0;
  }
  if (lws) {
    delete [] lws;
    lws = 0;
  }
  if (a) {
    delete [] a;
    a = 0;
  }
  if (la) {
    delete [] la;
    la = 0;
  }
}


void BqpdData::write(std::ostream &out) const
{
  UInt mlp    = 1000;      // Max level of degeneracy. 
  //UInt mxwk0  = 50000+10*(nJac+n+m);
  //UInt mxiwk0 = 500000;    // Initial workspace
  //UInt kmax   = 500;       // Maximum value of k. set kmax= 0 iff LP.
  //UInt mxwk   = 21*n + 8*m + mlp + lh1 + kmax*(kmax+9)/2 + mxwk0;
  //UInt mxiwk  = 13*n + 4*m + mlp + lh1 + kmax + 113 + mxiwk0;
  //std::cout << "mxwk5 = " << mxwk << std::endl;

  out << "x = ";
  if (x) {
    for (UInt i=0; i<n; ++i) {
      out << std::fixed
          << std::setprecision(6) 
          << x[i] << " ";
    }
  }
  out << std::endl;

  out << "r = ";
  if (r) {
    for (UInt i=0; i<n+m; ++i) {
      out << std::fixed
          << std::setprecision(6) 
          << r[i] << " ";
    }
  }
  out << std::endl;

  out << "e = ";
  if (e) {
    for (UInt i=0; i<n+m; ++i) {
      out << std::fixed
          << std::setprecision(6) 
          << e[i] << " ";
    }
  }
  out << std::endl;

  out << "w = ";
  if (w) {
    for (UInt i=0; i<n+m; ++i) {
      out << std::fixed
          << std::setprecision(6) 
          << w[i] << " ";
    }
  }
  out << std::endl;

  out << "g = ";
  if (g) {
    for (UInt i=0; i<n; ++i) {
      out << std::fixed
          << std::setprecision(6) 
          << g[i] << " ";
    }
  }
  out << std::endl;

  out << "ls = ";
  if (ls) {
    for (UInt i=0; i<n+m; ++i) {
      out << std::fixed
          << std::setprecision(6) 
          << ls[i] << " ";
    }
  }
  out << std::endl;

  out << "alp = ";
  if (alp) {
    for (UInt i=0; i<mlp; ++i) {
      out << std::fixed
          << std::setprecision(6) 
          << alp[i] << " ";
    }
  }
  out << std::endl;

  out << "lp = ";
  if (lp) {
    for (UInt i=0; i<mlp; ++i) {
      out << std::fixed
          << std::setprecision(6) 
          << lp[i] << " ";
    }
  }
  out << std::endl;

  out << "info = ";
  if (info) {
    for (UInt i=0; i<10; ++i) {
      out << std::fixed
          << std::setprecision(6) 
          << info[i] << " ";
    }
  }
  out << std::endl;

  out << "bl = ";
  if (bl) {
    for (UInt i=0; i<n+m; ++i) {
      out << std::fixed
          << std::setprecision(6) 
          << bl[i] << " ";
    }
  }
  out << std::endl;

  out << "bu = ";
  if (bu) {
    for (UInt i=0; i<n+m; ++i) {
      out << std::fixed
          << std::setprecision(6) 
          << bu[i] << " ";
    }
  }
  out << std::endl;

  out << "ws = ";
  if (ws) {
    //for (UInt i=0; i<mxwk; ++i) {
    //  out << std::fixed
    //      << std::setprecision(6) 
    //      << ws[i] << " ";
    //}
  }
  out << std::endl;

  out << "lws = ";
  if (lws) {
    //for (UInt i=0; i<mxiwk; ++i) {
    //  out << std::fixed
    //      << std::setprecision(6) 
    //      << lws[i] << " ";
    //}
  }
  out << std::endl;

  out << "a = ";
  if (a) {
    for (UInt i=0; i<maxa+1; ++i) {
      out << std::fixed
          << std::setprecision(6) 
          << a[i] << " ";
    }
  }
  out << std::endl;

  out << "la = ";
  if (la) {
    //for (UInt i=0; i<maxa+m+3; ++i) {
    //  out << std::fixed
    //      << std::setprecision(6) 
    //      << la[i] << " ";
    //}
  }
  out << std::endl;
}


// Local Variables: 
// mode: c++ 
// eval: (c-set-style "k&r") 
// eval: (c-set-offset 'innamespace 0) 
// eval: (setq c-basic-offset 2) 
// eval: (setq fill-column 78) 
// eval: (auto-fill-mode 1) 
// eval: (setq column-number-mode 1) 
// eval: (setq indent-tabs-mode nil) 
// End:
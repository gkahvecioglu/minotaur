//
//    MINOTAUR -- It's only 1/2 bull
//
//    (C)opyright 2009 -- 2010 The MINOTAUR Team.
//

/**
 * \file Bnb.cpp
 * \brief The main function for solving instances in ampl format (.nl) by
 * using Branch-and-Bound alone.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#include <iomanip>
#include <iostream>

#include "MinotaurConfig.h"
#include "BndProcessor.h"
#include "BranchAndBound.h"
#include "EngineFactory.h"
#include "Environment.h"
#include "IntVarHandler.h"
#include "LexicoBrancher.h"
#include "LinearHandler.h"
#include "LinFeasPump.h"
#include "LPEngine.h"
#include "LPProcessor.h"
#include "MaxVioBrancher.h"
#include "MINLPDiving.h"
#include "NLPEngine.h"
#include "NlPresHandler.h"
#include "NodeIncRelaxer.h"
#include "Objective.h"
#include "Option.h"
#include "Presolver.h"
#include "ProblemSize.h"
#include "QPEngine.h"
#include "Problem.h"
#include "Relaxation.h"
#include "ReliabilityBrancher.h"
#include "Solution.h"
#include "Timer.h"
#include "TreeManager.h"

#include "AMPLHessian.h"
#include "AMPLInterface.h"
#include "AMPLJacobian.h"

using namespace Minotaur;
BrancherPtr createBrancher(EnvPtr env, ProblemPtr p, HandlerVector handlers,
                           EnginePtr e);

BranchAndBound* createBab(EnvPtr env, ProblemPtr p, EnginePtr e, 
                          HandlerVector &handlers)
{
  BranchAndBound *bab = new BranchAndBound(env, p);
  NodeProcessorPtr nproc = NodeProcessorPtr(); // NULL
  IntVarHandlerPtr v_hand = (IntVarHandlerPtr) new IntVarHandler(env, p);
  LinHandlerPtr l_hand = (LinHandlerPtr) new LinearHandler(env, p);
  NlPresHandlerPtr nlhand;
  NodeIncRelaxerPtr nr;
  RelaxationPtr rel;
  BrancherPtr br;
  const std::string me("bnb main: ");
  OptionDBPtr options = env->getOptions();

  handlers.push_back(v_hand);
  if (true==options->findBool("presolve")->getValue()) {
    handlers.push_back(l_hand);
  }
  if (!p->isLinear() && 
       true==options->findBool("presolve")->getValue() &&
       true==options->findBool("use_native_cgraph")->getValue() &&
       true==options->findBool("nl_presolve")->getValue()) {
    nlhand = (NlPresHandlerPtr) new NlPresHandler(env, p);
    handlers.push_back(nlhand);
  }
  if (handlers.size()>1) {
    nproc = (LPProcessorPtr) new LPProcessor(env, e, handlers);
  } else {
    nproc = (BndProcessorPtr) new BndProcessor(env, e, handlers);
  }
  br = createBrancher(env, p, handlers, e);
  nproc->setBrancher(br);
  bab->setNodeProcessor(nproc);

  nr = (NodeIncRelaxerPtr) new NodeIncRelaxer(env, handlers);
  rel = (RelaxationPtr) new Relaxation(p);
  rel->calculateSize();
  if (options->findBool("use_native_cgraph")->getValue() ||
      rel->isQP() || rel->isQuadratic()) {
    rel->setNativeDer();
  } else {
    rel->setJacobian(p->getJacobian());
    rel->setHessian(p->getHessian());
  }
  rel->setInitialPoint(p->getInitialPoint());
  nr->setRelaxation(rel);
  nr->setEngine(e);
  bab->setNodeRelaxer(nr);
  bab->shouldCreateRoot(false);

  if (0 <= options->findInt("Divheur")->getValue()) {
    MINLPDivingPtr div_heur;
    EnginePtr e2 = e->emptyCopy();
    if (true==options->findBool("use_native_cgraph")->getValue() ||
        rel->isQP() || rel->isQuadratic()) {
      p->setNativeDer();
    }
    div_heur = (MINLPDivingPtr) new MINLPDiving(env, p, e2);
    bab->addPreRootHeur(div_heur);
  }
  if (0 <= options->findInt("LinFPump")->getValue()) {
    EngineFactory efac(env);
    EnginePtr lpe = efac.getLPEngine();
    EnginePtr nlpe = e->emptyCopy();
    LinFeasPumpPtr lin_feas_pump = (LinFeasPumpPtr) 
      new LinFeasPump(env, p, nlpe, lpe);
    bab->addPreRootHeur(lin_feas_pump);
  }
  return bab;
}


BrancherPtr createBrancher(EnvPtr env, ProblemPtr p, HandlerVector handlers,
                           EnginePtr e)
{
  BrancherPtr br;
  UInt t;
  const std::string me("bnb main: ");

  if (env->getOptions()->findString("brancher")->getValue() == "rel") {
    ReliabilityBrancherPtr rel_br;
    rel_br = (ReliabilityBrancherPtr) new ReliabilityBrancher(env, handlers);
    rel_br->setEngine(e);
    t = (p->getSize()->ints + p->getSize()->bins)/10;
    t = std::max(t, (UInt) 2);
    t = std::min(t, (UInt) 4);
    rel_br->setThresh(t);
    std::cout << me << "setting reliability threshhold to " << t << std::endl;
    t = (UInt) p->getSize()->ints + p->getSize()->bins/20+2;
    t = std::min(t, (UInt) 10);
    rel_br->setMaxDepth(t);
    std::cout << me << "setting reliability maxdepth to " << t << std::endl;
    if (e->getName()=="Filter-SQP") {
      rel_br->setIterLim(5);
    }
    std::cout << me << "reliability branching iteration limit = " 
              << rel_br->getIterLim() << std::endl;
    br = rel_br;
  } else if (env->getOptions()->findString("brancher")->getValue() ==
             "maxvio") {
    br = (MaxVioBrancherPtr) new MaxVioBrancher(env, handlers);
  } else if (env->getOptions()->findString("brancher")->getValue() == "lex") {
    br = (LexicoBrancherPtr) new LexicoBrancher(env, handlers);
  }
  std::cout << me << "brancher used = " << br->getName() << std::endl;
  return br;
}


EnginePtr getEngine(EnvPtr env, ProblemPtr p, Int &err)
{
  EngineFactory *efac = new EngineFactory(env);
  EnginePtr e = EnginePtr(); // NULL
  Bool cont=false;
  const std::string me("bnb main: ");

  err = 0;
  p->calculateSize();
  if (p->isLinear()) {
    e = efac->getLPEngine();
    if (!e) {
      cont = true;
    }
  }

  if (true==cont || p->isQP()) {
    e = efac->getQPEngine();
    if (!e) {
      cont = true;
    }
  }

  if (!e) {
    e = efac->getNLPEngine();
  }

  if (!e) {
    std::cout << me << "No engine available for this problem.";
    err = 1;
  } else {
    std::cout << me << "engine used = " << e->getName() << std::endl;
  }
  delete efac;
  return e;
}


void loadProblem(EnvPtr env, MINOTAUR_AMPL::AMPLInterface* iface,
                 ProblemPtr &oinst)
{
  Timer *timer     = env->getNewTimer();
  OptionDBPtr options = env->getOptions();
  JacobianPtr jac;
  HessianOfLagPtr hess;
  const std::string me("bnb main: ");

  timer->start();
  oinst = iface->readInstance(options->findString("problem_file")->getValue());
  std::cout << me << "time used in reading instance = " << std::fixed 
    << std::setprecision(2) << timer->query() << std::endl;

  // display the problem
  oinst->calculateSize();
  if (options->findBool("display_problem")->getValue()==true) {
    oinst->write(std::cout, 12);
  }
  if (options->findBool("display_size")->getValue()==true) {
    oinst->writeSize(std::cout);
  }
  // create the jacobian
  if (false==options->findBool("use_native_cgraph")->getValue()) {
    jac = (MINOTAUR_AMPL::AMPLJacobianPtr) 
      new MINOTAUR_AMPL::AMPLJacobian(iface);
    oinst->setJacobian(jac);

    // create the hessian
    hess = (MINOTAUR_AMPL::AMPLHessianPtr)
      new MINOTAUR_AMPL::AMPLHessian(iface);
    oinst->setHessian(hess);
  }

  // set initial point
  oinst->setInitialPoint(iface->getInitialPoint(), 
      oinst->getNumVars()-iface->getNumDefs());
}


void overrideOptions(EnvPtr env)
{
  env->getOptions()->findString("interface_type")->setValue("AMPL");
  env->getOptions()->findBool("modify_rel_only")->setValue(true);
}


PresolverPtr presolve(EnvPtr env, ProblemPtr p, Size_t ndefs, 
                        HandlerVector &handlers)
{
  PresolverPtr pres = PresolverPtr(); // NULL
  const std::string me("bnb main: ");

  p->calculateSize();
  if (env->getOptions()->findBool("presolve")->getValue() == true) {
    LinHandlerPtr lhandler = (LinHandlerPtr) new LinearHandler(env, p);
    handlers.push_back(lhandler);
    if (p->isQP() || p->isQuadratic() || p->isLinear() ||
        true==env->getOptions()->findBool("use_native_cgraph")->getValue()) {
      lhandler->setPreOptPurgeVars(true);
      lhandler->setPreOptPurgeCons(true);
      lhandler->setPreOptCoeffImp(true);
    } else {
      lhandler->setPreOptPurgeVars(false);
      lhandler->setPreOptPurgeCons(false);
      lhandler->setPreOptCoeffImp(false);
    }
    if (ndefs>0) {
      lhandler->setPreOptDualFix(false);
    } else {
      lhandler->setPreOptDualFix(true);
    }

    if (!p->isLinear() && 
         true==env->getOptions()->findBool("use_native_cgraph")->getValue() && 
         true==env->getOptions()->findBool("nl_presolve")->getValue() 
         ) {
      NlPresHandlerPtr nlhand = (NlPresHandlerPtr) new NlPresHandler(env, p);
      handlers.push_back(nlhand);
    }

    // write the names.
    std::cout << me << "handlers used in presolve:" << std::endl;
    for (HandlerIterator h = handlers.begin(); h != handlers.end(); 
        ++h) {
      std::cout<<(*h)->getName()<<std::endl;
    }
  }

  pres = (PresolverPtr) new Presolver(p, env, handlers);
  pres->standardize(); 
  if (env->getOptions()->findBool("presolve")->getValue() == true) {
    pres->solve();
  }
  return pres;
}


void setInitialOptions(EnvPtr env)
{
  env->getOptions()->findBool("presolve")->setValue(true);
  env->getOptions()->findBool("use_native_cgraph")->setValue(true);
  env->getOptions()->findBool("nl_presolve")->setValue(true);
}


void showHelp()
{
  std::cout << "Usage:" << std::endl
            << "To show version: bnb -v (or --show_version yes) " << std::endl
            << "To show all options: bnb -= (or --show_options yes)" 
            << std::endl
            << "To solve an instance: bnb --option1 [value] "
            << "--option2 [value] ... " << " .nl-file" << std::endl;
}


Int showInfo(EnvPtr env)
{
  OptionDBPtr options = env->getOptions();
  const std::string me("bnb main: ");

  if (options->findBool("show_options")->getValue() ||
      options->findFlag("=")->getValue()) {
    options->write(std::cout);
    return 1;
  }

  if (options->findBool("show_help")->getValue() ||
      options->findFlag("?")->getValue()) {
    showHelp();
    return 1;
  }

  if (options->findBool("show_version")->getValue() ||
      options->findFlag("v")->getValue()) {
    std::cout << me << "Minotaur version " << env->getVersion() << std::endl;
#if DEBUG
    std::cout << me; 
    env->writeFullVersion(std::cout);
    std::cout << std::endl;
#endif
    return 1;
  }

  if (options->findString("problem_file")->getValue()=="") {
    showHelp();
    return 1;
  }

  std::cout << me << "Minotaur version " << env->getVersion() << std::endl;
  return 0;
}


void writeSol(EnvPtr env, VarVector *orig_v, Double obj_sense,
              BranchAndBound* bab, PresolverPtr pres,
              MINOTAUR_AMPL::AMPLInterface* iface)
{
  const std::string me("bnb main: ");
  SolutionPtr sol = bab->getSolution(); 
  Int err = 0;

  if (sol) {
    sol = pres->getPostSol(sol);
  }
  if (env->getOptions()->findFlag("AMPL")->getValue()) {
    iface->writeSolution(sol, bab->getStatus());
  } else if (sol) {
    sol->writePrimal(std::cout, orig_v);
  }
  std::cout << me << std::fixed << std::setprecision(4) << 
    "best solution value = " << obj_sense*bab->getUb() << std::endl;
  std::cout << me << std::fixed << std::setprecision(4) << 
    "best bound estimate from remaining nodes = "
    <<  obj_sense*bab->getLb() << std::endl;
  std::cout << me << "time used = " << std::fixed << std::setprecision(2) 
    << env->getTime(err) << std::endl; assert(0==err);
  env->stopTimer(err); assert(0==err);
  std::cout << me << "status of branch-and-bound: " 
            << getSolveStatusString(bab->getStatus()) << std::endl;
}


int main(int argc, char** argv)
{
  EnvPtr env      = (EnvPtr) new Environment();
  OptionDBPtr options;
  MINOTAUR_AMPL::AMPLInterface* iface = 0;
  ProblemPtr oinst;    // instance that needs to be solved.
  EnginePtr engine;    // engine for solving relaxations. 
  SolutionPtr sol, sol2;
  JacobianPtr jPtr;
  HessianOfLagPtr hPtr;
  BranchAndBound * bab = 0;
  PresolverPtr pres;
  const std::string me("bnb main: ");
  VarVector *orig_v=0;
  HandlerVector handlers;
  Int err = 0;
  Double obj_sense = 1.0;

  env->startTimer(err);
  if (err) {
    goto CLEANUP;
  }

  setInitialOptions(env);

  // Important to setup AMPL Interface first as it adds several options.
  iface = new MINOTAUR_AMPL::AMPLInterface(env, "bnb");

  // Parse command line for options set by the user.
  env->readOptions(argc, argv);
  
  overrideOptions(env);
  if (0!=showInfo(env)) {
    goto CLEANUP;
  }

  loadProblem(env, iface, oinst);
  if (oinst->getObjective() &&
      oinst->getObjective()->getObjectiveType()==Maximize) {
    obj_sense = -1.0;
  }

  orig_v = new VarVector(oinst->varsBegin(), oinst->varsEnd());
  pres = presolve(env, oinst, iface->getNumDefs(), handlers);
  handlers.clear();

  if (false==env->getOptions()->findBool("solve")->getValue()) {
    goto CLEANUP;
  }

  engine = getEngine(env, oinst, err);
  if (err) {
    goto CLEANUP;
  }

  bab = createBab(env, oinst, engine, handlers);
  bab->solve();
  bab->writeStats();
  engine->writeStats();
  for (HandlerVector::iterator it=handlers.begin(); it!=handlers.end(); ++it) {
    (*it)->writeStats(std::cout);
  }
  
  writeSol(env, orig_v, obj_sense, bab, pres, iface);

CLEANUP:
  if (iface) {
    delete iface;
  }
  if (bab) {
    delete bab;
  }
  if (orig_v) {
    delete orig_v;
  }

  return 0;
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
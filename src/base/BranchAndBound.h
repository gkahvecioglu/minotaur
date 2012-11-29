// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2008 - 2012 The MINOTAUR Team.
// 

/**
 * \file BranchAndBound.h
 * \brief Declare the default branch-and-bound-algorithm.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#ifndef MINOTAURBRANCHANDBOUND_H
#define MINOTAURBRANCHANDBOUND_H

#include "Types.h"

namespace Minotaur {

  struct  BabOptions;
  struct  BabStats;
  class   NodeProcessor;
  class   NodeRelaxer;
  class   Problem;
  class   Solution;
  class   SolutionPool;
  class   Timer;
  class   TreeManager;
  typedef boost::shared_ptr <BabOptions> BabOptionsPtr;
  typedef boost::shared_ptr <NodeProcessor> NodeProcessorPtr;
  typedef boost::shared_ptr <NodeRelaxer> NodeRelaxerPtr;
  typedef boost::shared_ptr <Problem> ProblemPtr;
  typedef boost::shared_ptr <Solution> SolutionPtr;
  typedef boost::shared_ptr <SolutionPool> SolutionPoolPtr;
  typedef boost::shared_ptr <TreeManager> TreeManagerPtr;


  /**
   * BranchAndBound class is meant to implement the branch-and-bound
   * algorithm. 
   */
  class BranchAndBound {
  public:
    /// Default constructor.
    BranchAndBound();

    /// Constructor for a given Problem and Environment.
    BranchAndBound(EnvPtr env, ProblemPtr problem);

    /// Destroy.
    virtual ~BranchAndBound();

    /// Add a heuristic that will be called before root node.
    void addPreRootHeur(HeurPtr h);

    /**
     * Percentage gap between the lower and upper bounds. It is calculated as:
     * 
     * (ub - lb)/(fabs(ub)+tol)
     */
    Double getGap();

    /**
     * Get the lowerbound from the search tree. This bound is defined as the 
     * minimum of the bounds from all active nodes. It may not be a bound on
     * the optimal solution value.
     */
    Double getLb();

    /// Get a pointer to node processor.
    NodeProcessorPtr getNodeProcessor();

    /// Get a pointer to NodeRelaxer.
    NodeRelaxerPtr getNodeRelaxer(); 

    /*
     * Get solution from the last solve. If no solution was found, return
     * NULL.
     */
    SolutionPtr getSolution();

    /// Get the final status.
    SolveStatus getStatus();

    /**
     * Get a pointer to the tree manager. The client can then directly query
     * the TreeManager for its size and other attributes.
     */
    TreeManagerPtr getTreeManager(); 

    /**
     * Get the upperbound for the solution value from the search tree. 
     * This bound may or may not correspond to a feasible solution of the
     * problem. It may be obtained from a feasible solution of a relaxation of 
     * the problem.
     */
    Double getUb();

    /// return number of processed nodes
    UInt numProcNodes();

    /// Set log level.
    void setLogLevel(LogLevel level);

    /// Set a limit on the number of nodes in the branch-and-bound tree.
    void setNodeLimit(UInt limit);

    /// The processor that processes each node.
    void setNodeProcessor(NodeProcessorPtr p);

    /// Is the node relaxation created incrementally or from scratch?
    void setNodeRelaxer(NodeRelaxerPtr nr);

    /// Should the root node be solved in branch-and-bound.
    void shouldCreateRoot(Bool b);

    /// Start solving the Problem using branch-and-bound
    void solve();

    /// return total time taken
    Double totalTime();

    /// Write statistics to the ostream out
    void writeStats(std::ostream & out);

    /// Write statistics to the logger
    void writeStats();

  private:
    /// Pointer to the enviroment.
    EnvPtr env_;

    /** 
     * Log manager for displaying messages for Branch-and-Bound. This logger
     * is different from those of Engines, TreeManager, Node, Branch, 
     * Modification etc. 
     */
    LoggerPtr logger_;

    /// String name.
    static const std::string me_;

    /// The processor to process each node.
    NodeProcessorPtr nodePrcssr_;

    /// The relaxer to create a relaxation at each node.
    NodeRelaxerPtr nodeRlxr_;

    /// Options.
    BabOptionsPtr options_;

    /**
     * Heuristics that need to be called before creating and solving the root
     * node.
     */
    HeurVector preHeurs_;

    /// The Problem that is solved using branch-and-bound.
    ProblemPtr problem_;

    /// The TreeManager used to manage the search tree.
    SolutionPoolPtr solPool_;

    /**
     * Statistics about the branch-and-bound (including time, number of
     * iterations etc.)
     */
    BabStats *stats_;

    /// The status of the branch-and-bound algorithm.
    SolveStatus status_;

    /**
     * Timer for keeping track of time. The user or the environment from which
     * branch-and-bound is called can set up the timer and even start it
     * before sending it to branch-and-bound.
     */
    Timer *timer_;
   
    /// The TreeManager used to manage the search tree.
    TreeManagerPtr tm_;

    /**
     * Process the root node. NodePtr is the next candidate that will be
     * processed after the root. It can be NULL. dive is true if we dive after
     * processing the root.
     */
    NodePtr processRoot_(Bool *should_prune, Bool *should_dive);

    /// Check if a node can be pruned.
    Bool shouldPrune_(NodePtr node);

    /**
     * Should the branch-and-bound stop because of time limit, or node limit
     * or if solved?
     */
    Bool shouldStop_();

    /// Display status.
    void showStatus_(Bool current_uncounted);
  };

  /// Statistics about the branch-and-bound.
  struct BabStats 
  {
    /// Constructor. All data should be initialized to zero.
    BabStats();

    /// Number of nodes processed.
    UInt nodesProc;

    /// Total time used in branch-and-bound.
    Double timeUsed;

    /// Time of the last log display.
    Double updateTime;
  };


  /// Different options and parameters that control branch-and-bound
  struct BabOptions
  {
    /// Default constructor. 
    BabOptions();

    /// Constructor created from options in environment. 
    BabOptions(EnvPtr env);

    /**
     * Should the root be created in branch-and-bound (yes), or the user calls
     * branch-and-bound after creating the root (no)?
     */
    Bool createRoot;

    /// Time in seconds between status updates of the progress.
    Double logInterval;

    /// Verbosity of log.
    LogLevel logLevel;

    /// Limit on number of nodes created.
    Double nodeLimit;

    /// Time limit in seconds for the branch-and-bound.
    Double timeLimit;
  };

  typedef boost::shared_ptr<BranchAndBound> BranchAndBoundPtr;
}
#endif

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
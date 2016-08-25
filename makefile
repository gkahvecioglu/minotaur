#     MINOTAUR -- It's only 1/2 bull
#
#     (C)opyright 2009 -- The MINOTAUR Team

# Simple working Makefile

##############################################################################
### IMPORTANT IMPORTANT IMPORTANT
##############################################################################
### This makefile can be used to build MINOTAUR libraries and executables. 
### It creates the required files in the current directory. So copy this file
### to the directory where you want to build and then make. Object files are
### created in the MINOTAUR src directory and not in the current directory. So
### clean frequently.
### Also, copy MinotaurConfig.h.manual as MinotaurConfig.h to that directory.
##############################################################################

##############################################################################
### compiler options. Must be filled.
##############################################################################
CC = gcc
CFLAGS = -ggdb
CXX = g++
# CXXFLAGS = 
CXXFLAGS = -ggdb -Wall -pedantic -Wmissing-include-dirs -Wunused -Wextra \
	   -Wundef -Wshadow -Wredundant-decls -Woverloaded-virtual
F77 = gfortran

##############################################################################
### Leave these blank. 
##############################################################################
FILTERSQP_LIB = 
IPOPT_INST =
IPOPT_INCS =
OSI_INCS =
OSI_INST = 
AMPL_INST =
AMPL_INCS =

##############################################################################
### Directory containing all minotaur externals. Optional
##############################################################################
EXT_DIR = 

##############################################################################
### Fill in the location of boost headers. The directory specified here 
### must contain boost/. Leave blank if they are installed in your system 
### include path.
##############################################################################
BOOST_INC = $(EXT_DIR)
//BOOST_INC = homes/gkahvecioglu/minotaur-externals

#############################################################################
### Select engines. Can leave blank if no engines are required.
### Examples:
#ENGINES = OsiLP Ipopt FilterSQP           #(e.g.)
#ENGINES = Ipopt                           #(e.g.)
#ENGINES = FilterSQP                       #(e.g.)
#ENGINES =                                 #(e.g.)
##############################################################################
ENGINES = 

##############################################################################
### Select interfaces. Can leave blank if no interfaces are required.
#INTERFACES = AMPL                          #(e.g.)
#INTERFACES =                               #(e.g.)
##############################################################################
INTERFACES =JULIA

##############################################################################
### If you wanted FilterSQP engine, specify path to the install-directory of 
### FilterSQP. FilterSQP_LIB should point to the library. Nothing required if
### IPOPT is not needed. 
##############################################################################
ifeq ($(findstring FilterSQP,$(ENGINES)),FilterSQP)
  FILTERSQP_LIB = $(EXT_DIR)/filter-87/filterlib.a
endif
ifeq ($(findstring Bqpd,$(ENGINES)),Bqpd)
  FILTERSQP_LIB = $(EXT_DIR)/filter-87/filterlib.a
endif

##############################################################################
### If you wanted IPOPT engine, specify path to the install-directory of 
### IPOPT. IPOPT_INST path must contain a lib/ directory. Nothing required if
### IPOPT is not needed. IPOPT_INCS must contain the header files of Ipopt.
##############################################################################
ifeq ($(findstring Ipopt,$(ENGINES)),Ipopt)
  IPOPT_INST = $(EXT_DIR)/ipopt-3.10.2/
  IPOPT_INCS = -I$(IPOPT_INST)/include #(e.g.)
endif

##############################################################################
### If you wanted OSI engine, specify path to the install-directory of OSI. 
### OSI_INST path must contain a lib/ directory. Nothing required if OSI is 
### not needed. OSI_INCS must contain the header files of OSI.
##############################################################################
ifeq ($(findstring OsiLP,$(ENGINES)),OsiLP)
  OSI_INST = $(EXT_DIR)/clp-1.14.6
  OSI_INCS = -I$(OSI_INST)/include #(e.g.)
endif

##############################################################################
### If you wanted AMPL interface, specify path to the install-directory of 
### ASL. ASL path must contain a amplsolver.a libraray. Nothing required if
### AMPL is not needed. AMPL_INCS must contain the header files of ASL.
##############################################################################
ifeq ($(findstring AMPL,$(INTERFACES)),AMPL)
  AMPL_INST = $(EXT_DIR)/asl-20120126
  AMPL_INCS = -I$(AMPL_INST) # location of asl.h
endif


##############################################################################
### The location of MINOTAUR source code. We can build the code anywhere 
### irrespective of where MINOTAUR source code is located. The location must 
### contain the MINOTAUR src/ directory.
##############################################################################
//MINOTAUR = /Users/kibaekkim/Documents/REPOS/SMINLP/JuliaMinotaur/minotaur-0.2.0-src
MINOTAUR = /homes/gkahvecioglu/SMINLP/JuliaMinotaur/minotaur-0.2.0-src/

##############################################################################
### Optional flags. Don't change them if you don't know about them.
##############################################################################
DEFINES = -DSPEW=0 -DDEBUG=0 -DCOIN_BIG_INDEX=0

EXTRA_LIBS = -lpthread
##############################################################################
### End of user options.
##############################################################################

# location of base with respect to ./
BASE_DIR = $(MINOTAUR)/src/base

BASE_SRCS = \
	$(BASE_DIR)/BndProcessor.cpp  \
	$(BASE_DIR)/Branch.cpp  \
	$(BASE_DIR)/BranchAndBound.cpp  \
	$(BASE_DIR)/Brancher.cpp  \
	$(BASE_DIR)/BrCand.cpp  \
	$(BASE_DIR)/BrVarCand.cpp  \
	$(BASE_DIR)/Chol.cpp \
	$(BASE_DIR)/CGraph.cpp \
	$(BASE_DIR)/CNode.cpp \
	$(BASE_DIR)/Constraint.cpp  \
	$(BASE_DIR)/CutInfo.cpp \
	$(BASE_DIR)/CutMan1.cpp \
	$(BASE_DIR)/CxQuadHandler.cpp  \
	$(BASE_DIR)/CxUnivarHandler.cpp \
	$(BASE_DIR)/Eigen.cpp  \
	$(BASE_DIR)/Engine.cpp \
	$(BASE_DIR)/Environment.cpp \
	$(BASE_DIR)/FeasibilityPump.cpp \
	$(BASE_DIR)/Function.cpp \
	$(BASE_DIR)/HessianOfLag.cpp \
	$(BASE_DIR)/Interrupt.cpp \
	$(BASE_DIR)/IntVarHandler.cpp \
	$(BASE_DIR)/Jacobian.cpp \
	$(BASE_DIR)/LPProcessor.cpp \
	$(BASE_DIR)/LPRelaxation.cpp \
	$(BASE_DIR)/LexicoBrancher.cpp \
	$(BASE_DIR)/LinBil.cpp \
	$(BASE_DIR)/LinConMod.cpp \
	$(BASE_DIR)/LinearCut.cpp \
	$(BASE_DIR)/LinearFunction.cpp \
	$(BASE_DIR)/LinearHandler.cpp\
	$(BASE_DIR)/LinFeasPump.cpp \
	$(BASE_DIR)/LinMods.cpp \
	$(BASE_DIR)/Logger.cpp \
	$(BASE_DIR)/MaxFreqBrancher.cpp\
	$(BASE_DIR)/MaxVioBrancher.cpp \
	$(BASE_DIR)/MINLPDiving.cpp \
	$(BASE_DIR)/MsProcessor.cpp \
	$(BASE_DIR)/NLPRelaxation.cpp \
	$(BASE_DIR)/NlPresHandler.cpp \
	$(BASE_DIR)/NLPMultiStart.cpp \
	$(BASE_DIR)/Node.cpp \
	$(BASE_DIR)/NodeFullRelaxer.cpp \
	$(BASE_DIR)/NodeHeap.cpp \
	$(BASE_DIR)/NodeIncRelaxer.cpp \
	$(BASE_DIR)/NodeProcessor.cpp \
	$(BASE_DIR)/NodeStack.cpp \
	$(BASE_DIR)/NonlinearFunction.cpp \
	$(BASE_DIR)/Objective.cpp \
	$(BASE_DIR)/Operations.cpp \
	$(BASE_DIR)/Option.cpp \
	$(BASE_DIR)/PolynomialFunction.cpp \
	$(BASE_DIR)/PreAuxVars.cpp \
	$(BASE_DIR)/PreDelVars.cpp \
	$(BASE_DIR)/PreSubstVars.cpp \
	$(BASE_DIR)/Presolver.cpp \
	$(BASE_DIR)/Problem.cpp \
	$(BASE_DIR)/QGHandler.cpp \
	$(BASE_DIR)/QPDRelaxer.cpp \
	$(BASE_DIR)/QPDProcessor.cpp \
	$(BASE_DIR)/QuadHandler.cpp \
	$(BASE_DIR)/QuadraticFunction.cpp \
	$(BASE_DIR)/RandomBrancher.cpp\
	$(BASE_DIR)/Relaxation.cpp \
	$(BASE_DIR)/ReliabilityBrancher.cpp \
	$(BASE_DIR)/SecantMod.cpp \
	$(BASE_DIR)/SimpleCutMan.cpp \
	$(BASE_DIR)/SimpleTransformer.cpp \
	$(BASE_DIR)/Solution.cpp \
	$(BASE_DIR)/SolutionPool.cpp \
	$(BASE_DIR)/SOS.cpp \
	$(BASE_DIR)/SOS1Handler.cpp \
	$(BASE_DIR)/SOS2Handler.cpp \
	$(BASE_DIR)/SOSBrCand.cpp \
	$(BASE_DIR)/Transformer.cpp \
	$(BASE_DIR)/TransPoly.cpp \
	$(BASE_DIR)/TreeManager.cpp \
	$(BASE_DIR)/Types.cpp \
	$(BASE_DIR)/VarBoundMod.cpp \
	$(BASE_DIR)/Variable.cpp \
	$(BASE_DIR)/YEqCGs.cpp \
	$(BASE_DIR)/YEqLFs.cpp \
	$(BASE_DIR)/YEqMonomial.cpp \
	$(BASE_DIR)/YEqUCGs.cpp \
	$(BASE_DIR)/YEqVars.cpp \

ALL_HEADERS = \
	$(BASE_DIR)/MinotaurDeconfig.h \
	$(BASE_DIR)/ActiveNodeStore.h \
	$(BASE_DIR)/BndProcessor.h \
	$(BASE_DIR)/Branch.h \
	$(BASE_DIR)/Brancher.h \
	$(BASE_DIR)/BranchAndBound.h \
	$(BASE_DIR)/BrCand.h \
	$(BASE_DIR)/BrVarCand.h \
	$(BASE_DIR)/CGraph.h \
	$(BASE_DIR)/CNode.h \
	$(BASE_DIR)/Constraint.h \
	$(BASE_DIR)/CutInfo.h \
	$(BASE_DIR)/CutMan1.h \
	$(BASE_DIR)/CutManager.h \
	$(BASE_DIR)/CxQuadHandler.h \
	$(BASE_DIR)/CxUnivarHandler.h \
	$(BASE_DIR)/Eigen.h \
	$(BASE_DIR)/Engine.h \
	$(BASE_DIR)/Environment.h \
	$(BASE_DIR)/FeasibilityPump.h  \
	$(BASE_DIR)/Exception.h \
	$(BASE_DIR)/Function.h \
	$(BASE_DIR)/Handler.h \
	$(BASE_DIR)/HessianOfLag.h \
	$(BASE_DIR)/Heuristic.h \
	$(BASE_DIR)/Iterate.h \
	$(BASE_DIR)/Interrupt.h \
	$(BASE_DIR)/IntVarHandler.h \
	$(BASE_DIR)/Jacobian.h \
	$(BASE_DIR)/LexicoBrancher.h \
	$(BASE_DIR)/LinBil.cpp \
	$(BASE_DIR)/LinConMod.h \
	$(BASE_DIR)/LinearCut.h \
	$(BASE_DIR)/LinearFunction.h \
	$(BASE_DIR)/LinearHandler.h \
	$(BASE_DIR)/LinFeasPump.h  \
	$(BASE_DIR)/LinMods.h \
	$(BASE_DIR)/Logger.h \
	$(BASE_DIR)/LPEngine.h \
	$(BASE_DIR)/LPProcessor.h \
	$(BASE_DIR)/LPRelaxation.h \
	$(BASE_DIR)/MaxFreqBrancher.h\
	$(BASE_DIR)/MaxVioBrancher.h \
	$(BASE_DIR)/MINLPDiving.h \
	$(BASE_DIR)/Modification.h \
	$(BASE_DIR)/MsProcessor.h \
	$(BASE_DIR)/NLPEngine.h \
	$(BASE_DIR)/NLPRelaxation.h \
	$(BASE_DIR)/NlPresHandler.h \
	$(BASE_DIR)/NLPMultiStart.h \
	$(BASE_DIR)/Node.h \
	$(BASE_DIR)/NodeHeap.h \
	$(BASE_DIR)/NodeRelaxer.h \
	$(BASE_DIR)/NodeIncRelaxer.h \
	$(BASE_DIR)/NodeProcessor.h \
	$(BASE_DIR)/NodeStack.h \
	$(BASE_DIR)/NonlinearFunction.h \
	$(BASE_DIR)/Operations.h \
	$(BASE_DIR)/Objective.h \
	$(BASE_DIR)/Option.h \
	$(BASE_DIR)/OpCode.h \
	$(BASE_DIR)/PolynomialFunction.h \
	$(BASE_DIR)/PreAuxVars.h \
	$(BASE_DIR)/PreDelVars.h \
	$(BASE_DIR)/PreMod.h \
	$(BASE_DIR)/Presolver.h \
	$(BASE_DIR)/PreSubstVars.h \
	$(BASE_DIR)/Problem.h \
	$(BASE_DIR)/ProblemSize.h \
	$(BASE_DIR)/QPEngine.h \
	$(BASE_DIR)/QGHandler.h \
	$(BASE_DIR)/QPDProcessor.h \
	$(BASE_DIR)/QuadHandler.h \
	$(BASE_DIR)/QPDRelaxer.h \
	$(BASE_DIR)/QuadraticFunction.h \
	$(BASE_DIR)/RandomBrancher.h\
	$(BASE_DIR)/Relaxation.h \
	$(BASE_DIR)/ReliabilityBrancher.h \
	$(BASE_DIR)/SecantMod.h \
	$(BASE_DIR)/SimpleCutMan.h \
	$(BASE_DIR)/SimpleTransformer.h \
	$(BASE_DIR)/Solution.h \
	$(BASE_DIR)/SolutionPool.h \
	$(BASE_DIR)/SOS.h \
	$(BASE_DIR)/SOS1Handler.h \
	$(BASE_DIR)/SOS2Handler.h \
	$(BASE_DIR)/SOSBrCand.h \
	$(BASE_DIR)/Timer.h \
	$(BASE_DIR)/Transformer.h  \
	$(BASE_DIR)/TransPoly.h  \
	$(BASE_DIR)/TreeManager.h \
	$(BASE_DIR)/Types.h \
	$(BASE_DIR)/WarmStart.h \
	$(BASE_DIR)/Variable.h \
	$(BASE_DIR)/YEqCGs.h \
	$(BASE_DIR)/YEqLFs.h \
	$(BASE_DIR)/YEqMonomial.h \
	$(BASE_DIR)/YEqUCGs.h \
	$(BASE_DIR)/YEqVars.h \

BASE_OBJS = $(BASE_SRCS:.cpp=.o)

ALG_DIR       = $(MINOTAUR)/src/algorithms
BQPD_DIR      = $(MINOTAUR)/src/engines/Bqpd
FILTERSQP_DIR = $(MINOTAUR)/src/engines/FilterSQP
IPOPT_DIR     = $(MINOTAUR)/src/engines/Ipopt
OSI_DIR       = $(MINOTAUR)/src/engines/OsiLP

AMPL_DIR = $(MINOTAUR)/src/interfaces/ampl
JULIA_DIR =$(MINOTAUR)/src/interfaces/julia

INCLUDES = -I$(BOOST_INC)
INCLUDES += ./include/minotaur
LIBS = -L./lib  -lmntrengfac 
##############################################################################
### Bqpd engine
##############################################################################
BQPD_SRCS   =  
BQPD_F_SRCS =  
BQPD_OBJS   =  
ifeq ($(findstring Bqpd,$(ENGINES)),Bqpd)
  BQPD_SRCS   +=  $(BQPD_DIR)/BqpdEngine.cpp 
  BQPD_F_SRCS  =  $(BQPD_DIR)/BqpdAux.f
  DEFINES     += -DUSE_BQPD
  LIBS        += -lmntrbqpd ${BQPD_LIB} 
  ALL_HEADERS += $(BQPD_DIR)/BqpdEngineTypes.h \
  		 $(BQPD_DIR)/BqpdEngine.h
endif
BQPD_OBJS =  $(BQPD_SRCS:.cpp=.o)
BQPD_OBJS+=  $(BQPD_F_SRCS:.f=.o)


##############################################################################
### FilterSQP engine
##############################################################################
FILTERSQP_SRCS =  
FILTERSQP_OBJS =  
ifeq ($(findstring FilterSQP,$(ENGINES)),FilterSQP)
  FILTERSQP_SRCS     +=  $(FILTERSQP_DIR)/FilterSQPEngine.cpp
  DEFINES            += -DUSE_FILTERSQP
  LIBS               += -lmntrfiltersqp ${FILTERSQP_LIB} 
  ALL_HEADERS        += $(FILTERSQP_DIR)/FilterSQPEngineTypes.h \
			$(FILTERSQP_DIR)/FilterSQPEngine.h
endif
FILTERSQP_OBJS =  $(FILTERSQP_SRCS:.cpp=.o)

##############################################################################
### OSI engine
##############################################################################
OSI_OBJS =
OSI_SRCS =

ifeq ($(findstring OsiLP,$(ENGINES)),OsiLP)
  OSI_SRCS    +=  $(OSI_DIR)/OsiLPEngine.cpp
  DEFINES     += -DUSE_OSILP -DMNTROSICLP=1 -DMNTROSICPX=0 -DMNTROSIGRB=0
  LIBS        += -lmntrosilp 
  LIBS        += -L$(OSI_INST)/lib -lOsi -lOsiClp -lClp -lCoinUtils \
                 -Wl,-rpath $(OSI_INST)/lib
  ALL_HEADERS += $(OSI_DIR)/OsiLPEngine.h
endif
OSI_OBJS   =  $(OSI_SRCS:.cpp=.o)

##############################################################################
### Ipopt engine
##############################################################################
ifeq ($(findstring Ipopt,$(ENGINES)),Ipopt)
  IPOPT_SRCS +=  $(IPOPT_DIR)/IpoptEngine.cpp
  DEFINES    += -DUSE_IPOPT
  LIBS       += -lmntripopt -L$(IPOPT_INST)/lib -lipopt -Wl,-rpath $(IPOPT_INST)/lib
  ALL_HEADERS += $(IPOPT_DIR)/IpoptEngine.h $(IPOPT_DIR)/IpoptEngineTnlp.h
endif
IPOPT_OBJS    =  $(IPOPT_SRCS:.cpp=.o)

##############################################################################
### Engine factory
##############################################################################
ENGFAC_OBJS =
ENGFAC_SRCS = $(MINOTAUR)/src/engines/EngineFactory.cpp
ENGFAC_OBJS = $(ENGFAC_SRCS:.cpp=.o)
#WRAPPER_SRCS = $(MINOTAUR)/src/algorithms/Bnb_wrapper.cpp
WRAPPER_SRCS = $(MINOTAUR)/src/interfaces/julia/JuliaProblemWrapper.cpp
WRAPPER_OBJS = $(WRAPPER_SRCS:.cpp=.o)
ALL_HEADERS += $(MINOTAUR)/src/engines/EngineFactory.h 
#ALL_HEADERS += $(MINOTAUR)/src/algorithms/Bnb_wrapper.h
ALL_HEADERS += $(MINOTAUR)/src/interfaces/julia/JuliaProblemWrapper.h
##############################################################################
### AMPL
##############################################################################
ifeq ($(findstring AMPL,$(INTERFACES)),AMPL)
  AMPL_SRCS  += \
		$(AMPL_DIR)/AMPLHessian.cpp \
		$(AMPL_DIR)/AMPLInterface.cpp \
		$(AMPL_DIR)/AMPLJacobian.cpp \
		$(AMPL_DIR)/AMPLNonlinearFunction.cpp

  DEFINES    += -DUSE_MINOTAUR_AMPL_INTERFACE
  LIBS       += -lmntrampl -lgfortran $(AMPL_INST)/amplsolver.a -ldl -llapack -lblas
  ALL_HEADERS += $(AMPL_DIR)/AMPLHessian.h $(AMPL_DIR)/AMPLInterface.h \
		 $(AMPL_DIR)/AMPLJacobian.h $(AMPL_DIR)/AMPLNonlinearFunction.h
		 
endif
AMPL_OBJS =  $(AMPL_SRCS:.cpp=.o)
##############################################################################
### JULIA
##############################################################################
ifeq ($(findstring JULIA,$(INTERFACES)),JULIA)
  JULIA_SRCS   += \
  		$(JULIA_DIR)/JuliaInterface.cpp \
  		$(JULIA_DIR)/JuliaHessian.cpp \
	    $(JULIA_DIR)/JuliaJacobian.cpp \
	    $(JULIA_DIR)/JuliaNonlinearFunction.cpp \
		$(JULIA_DIR)/MinotaurApiEnv.cpp

  DEFINES     += -DUSE_MINOTAUR_JULIA_INTERFACE
  LIBS        += -lmntrjulia -lgfortran -ldl -llapack -lblas
  ALL_HEADERS += \
  		$(JULIA_DIR)/JuliaInterface.h \
		$(JULIA_DIR)/JuliaHessian.h \
		$(JULIA_DIR)/JuliaJacobian.h \
		$(JULIA_DIR)/JuliaNonlinearFunction.h \
		$(JULIA_DIR)/MinotaurApiEnv.h
endif
JULIA_OBJS =  $(JULIA_SRCS:.cpp=.o)
##############################################################################
### Start building
##############################################################################

LIBS += -L. -lminotaur $(EXTRA_LIBS)

INCLUDES += $(IPOPT_INCS) $(OSI_INCS) $(AMPL_INCS)

.PHONY: depend autodoc install all clean lib libengfac libbqpd libfiltersqp libosilp libipopt libampl bin 

CXXFLAGS += -fPIC

.cpp.o:
	$(CXX) $(CXXFLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

.f.o: 
	$(F77) -c $< -o $@

default: all

all: lib bin

config: 
	@mkdir -p lib bin include include/minotaur ; \
	cp -fv $(MINOTAUR)/MinotaurConfig.h.manual include/minotaur/MinotaurConfig.h ; \
	cp -fv $(MINOTAUR)/Version.h.manual include/minotaur/Version.h ; \
	for i in $(ALL_HEADERS); do \
		cp -fv $$i include/minotaur/ ; \
	done


lib: config libbase libengfac libbqpd libfiltersqp libosilp libipopt libampl libjulia libminotaur_shared

bin: lib
       	#$(CXX) $(CXXFLAGS) $(DEFINES) $(INCLUDES) $(JULIA_DIR)/JuliaInterface.cpp  \
	#$(LDFLAGS) $(LIBS) -o bin/julia;
	
	#$(CXX) $(CXXFLAGS) $(DEFINES) $(INCLUDES) $(ALG_DIR)/Bnb_wrapper.cpp  \
	#$(LDFLAGS) $(LIBS) -o bin/wrapper;\
	
	#$(CXX) $(CXXFLAGS) $(DEFINES) $(INCLUDES) $(ALG_DIR)/Glob.cpp \
	#$(LDFLAGS) $(LIBS) -o bin/glob ;\
	#$(CXX) $(CXXFLAGS) $(DEFINES) $(INCLUDES) $(ALG_DIR)/qg.cpp \
	#$(LDFLAGS) $(LIBS) -o bin/qg ;\
	#$(CXX) $(CXXFLAGS) $(DEFINES) $(INCLUDES) $(ALG_DIR)/QPDive.cpp \
	#$(LDFLAGS) $(LIBS) -o bin/qpd ;\
	

libbase: $(BASE_OBJS)
	ar rcv lib/libminotaur.a $(BASE_OBJS); ranlib lib/libminotaur.a

libminotaur_shared: $(WRAPPER_OBJS)
	$(CXX) $(CXXFLAGS) $(DEFINES) $(INCLUDES) $(JULIA_DIR)/JuliaProblemWrapper.cpp  \
	$(LDFLAGS) $(LIBS) -shared -o lib/libminotaur_shared.so;

ifeq ($(findstring Bqpd,$(ENGINES)),Bqpd)
libbqpd: $(BQPD_OBJS)
	ar rcv lib/libmntrbqpd.a $(BQPD_OBJS); ranlib lib/libmntrbqpd.a
else
libbqpd: 

endif

ifeq ($(findstring FilterSQP,$(ENGINES)),FilterSQP)
libfiltersqp: $(FILTERSQP_OBJS)
	ar rcv lib/libmntrfiltersqp.a $(FILTERSQP_OBJS); ranlib lib/libmntrfiltersqp.a
else
libfiltersqp: 

endif

ifeq ($(findstring OsiLP,$ENGFAC_OBJS(ENGINES)),OsiLP)
libosilp: $(OSI_OBJS)
	ar rcv lib/libmntrosilp.a $(OSI_OBJS); ranlib lib/libmntrosilp.a
else
libosilp: 

endif

libengfac: $(ENGFAC_OBJS)
	ar rcv lib/libmntrengfac.a $(ENGFAC_OBJS); ranlib lib/libmntrengfac.a



ifeq ($(findstring Ipopt,$(ENGINES)),Ipopt)
libipopt: $(IPOPT_OBJS)
	ar rcv lib/libmntripopt.a $(IPOPT_OBJS); ranlib lib/libmntripopt.a
else
libipopt: 

endif



ifeq ($(findstring AMPL,$(INTERFACES)),AMPL)
libampl: $(AMPL_OBJS)
	ar rcv lib/libmntrampl.a $(AMPL_OBJS); ranlib lib/libmntrampl.a

else
libmntrampl:

endif

ifeq ($(findstring JULIA,$(INTERFACES)),JULIA)
libjulia: $(JULIA_OBJS)
	ar rcv lib/libmntrjulia.a $(JULIA_OBJS); ranlib lib/libmntrjulia.a

else
libmntrjulia:

endif

clean:
	rm -fv $(BASE_OBJS) $(ENGFAC_OBJS) $(OSI_OBJS) $(IPOPT_OBJS) \
		$(AMPL_OBJS) $(FILTERSQP_OBJS) $(BQPD_OBJS) $(JULIA_OBJS) ;\
	rm -fv include/minotaur/*.h  ;\
	rm -fv bin/bnb bin/glob bin/qg bin/qpd ;\
	rm -fv lib/libminotaur.a  lib/libmntrampl.a  lib/libmntrbqpd.a \
	       lib/libmntrengfac.a  lib/libmntrfiltersqp.a  lib/libmntripopt.a \
	       lib/libmntrosilp.a


clean_all: clean 
	rm -fv libminotaur.a libmntrosilp.a libmntripopt.a libmntrampl.a libmntrjulia.a 

minotaur.d: config depend

depend: $(BASE_SRCS)
	$(CXX) $(CXXFLAGS) $(DEFINES) $(INCLUDES) -MM $(BASE_SRCS) > minotaur.d

include minotaur.d


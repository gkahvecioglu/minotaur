//#include <iostream>
#include "MinotaurApiEnv.h"
#include "Environment.h"
#include "Problem.h"
#include "JuliaInterface.h"

using namespace Minotaur;

MinotaurApiEnv::MinotaurApiEnv() {
    env_ = new Environment();
    problem_ = new Problem();
    iface_ = NULL; // new minotaur_julia::JuliaInterface(env_, solver);
}


MinotaurApiEnv::~MinotaurApiEnv() {
    if (env_) {
        delete env_;
        env_ = NULL;
    }
    if (problem_) {
        delete problem_;
        problem_ = NULL;
    }
    if (iface_) {
        delete iface_;
        iface_ = NULL;
    }
}
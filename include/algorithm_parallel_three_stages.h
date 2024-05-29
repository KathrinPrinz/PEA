#pragma once
#include "cplex_interface.h"
#include <chrono>
#include <vector>
#include <array>
#include <string>


void calculate_successors(std::vector<IloEnv>& environments,
    std::vector<IloModel>& models,
    std::vector<IloCplex>& cplexes,
    std::vector<IloNumArray>& startVals,
    std::vector<IloNumVarArray>& xs,
    std::vector<std::vector<IloExpr>>& objectivess, std::vector< std::vector<std::pair<std::vector<int>,
    std::vector<int>>>>& threadY_N,
    std::vector<int> y, std::vector<int> z,bool& found_feasible,std::vector<std::pair<std::vector<int>, std::vector<int>>>& feasible,
    std::vector<std::array<int,5>>& counter_scalarizations);

bool feasible_generation(IloEnv& env, IloModel& model, IloCplex& cplex, IloNumVarArray& x, IloNumArray& startVal,
	std::vector<IloExpr>& objectives,std::vector<std::pair<std::vector<int>, std::vector<int>>>& feasible,
	std::vector<std::array<int,5>>& counter_scalarizations);

void initialize_parallel_tasks(const std::string &instance, const std::string & result_location, int THREADS);
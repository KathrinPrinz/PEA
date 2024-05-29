#pragma once
#include <vector>
#include <ilcplex/ilocplex.h>
std::tuple<IloEnv, IloCplex, IloModel, IloNumVarArray, IloNumArray, std::vector<IloExpr>> createCplexInterface(const std::string& instance, const int& threads = 1, const int& multiobjective = 0);

std::pair<std::vector<int>, std::vector<int>> epsilon_first_stage(std::vector<int>& u,
    IloEnv& env, IloModel& model, IloCplex& cplex, IloNumVarArray& x, IloNumArray& startVal,
    std::vector<IloExpr>& objectives, const std::vector<int>& x_start = std::vector<int>(), const std::vector<int>& order = { 0,1,2 });

std::pair<std::vector<int>, std::vector<int>> epsilon_second_stage(std::vector<int>& u, std::vector<int>& y_start,
    IloEnv& env, IloModel& model, IloCplex& cplex, IloNumVarArray& x, IloNumArray& startVal,
    std::vector<IloExpr>& objectives, std::vector<int>& x_start, const std::vector<int>& order = { 1,0,2 });

std::pair<std::vector<int>, std::vector<int>> epsilon_third_stage(std::vector<int>& u, std::vector<int>& y_start,
    IloEnv& env, IloModel model, IloCplex& cplex, IloNumVarArray& x, IloNumArray& startVal,
    std::vector<IloExpr>& objectives, std::vector<int>& x_start, const std::vector<int>& order = { 1,0,2 });


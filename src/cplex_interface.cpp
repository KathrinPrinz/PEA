#include <ilcplex/ilocplex.h>
#include <vector>
#include <string>
#include <tuple>
#include <math.h>
std::tuple<IloEnv, IloCplex, IloModel, IloNumVarArray, IloNumArray, std::vector<IloExpr>> createCplexInterface(const std::string& instance, const int& threads = 1, const int& multiobjective = 0) {
    IloEnv env;
    IloCplex cplex(env);
    IloModel model(env);
    IloObjective objective(env);
    IloNumVarArray x(env);
    IloRangeArray ranges(env);
    cplex.importModel(model, instance.c_str(), objective, x, ranges);
    if (multiobjective) {
        cplex.extract(model);
    }
    IloNumArray startVal(env, x.getSize());
    std::vector<IloExpr> objectives;
    for (int i = 0; i < 3; i++) {
        IloExpr objectiveExpr;
        objectiveExpr = objective.getCriterion(i);

        objectives.push_back(objectiveExpr);
    }
    model.remove(objective);
    objective.end();
    if (!multiobjective) {
        cplex.extract(model);
    }
    cplex.setParam(IloCplex::Param::Threads, threads); // Set the number of threads
    cplex.setOut(env.getNullStream());
    cplex.setParam(IloCplex::Param::MIP::Tolerances::MIPGap, 1e-06);
    return std::tie(env, cplex, model, x, startVal, objectives);
}

std::pair<std::vector<int>, std::vector<int>> epsilon_first_stage(std::vector<int>& u,
    IloEnv& env, IloModel& model, IloCplex& cplex, IloNumVarArray& x, IloNumArray& startVal,
    std::vector<IloExpr>& objectives, const std::vector<int>& x_start = std::vector<int>(), const std::vector<int>& order = { 0,1,2 })
{
    //clear warm starts from previous iterations
    if (cplex.getNMIPStarts())
    {
        cplex.deleteMIPStarts(0, cplex.getNMIPStarts());
    }

    if (!x_start.empty()) {
        for (int i = 0; i < x.getSize(); i++) {
            startVal[i] = x_start[i];
        }
    }
    // Create the objective expression
    IloObjective objective = IloObjective(env, objectives[order[2]], IloObjective::Minimize);
    model.add(objective);

    // Add constraints
    std::vector<IloConstraint> constraints;
    for (int i = 0; i < 2; i++) {
        if (u[order[i]] < INT_MAX) {
            IloConstraint constr = IloConstraint(objectives[order[i]] <= u[order[i]] - 1);
            model.add(constr);
            constraints.push_back(constr);
        }

    }

    if (!x_start.empty()) {
        cplex.addMIPStart(x, startVal);
    }

    try {
        cplex.solve(); // Solve the optimization problem

        if (cplex.getStatus() == IloAlgorithm::Optimal) {
            // Extract and return the optimal solution
            std::vector<int> xopt(x.getSize());
            for (int i = 0; i < x.getSize(); i++) {
                xopt[i] = std::round(cplex.getValue(x[i]));
            }
            std::vector<int> yopt;
            for (int i = 0; i < 3; i++) {
                yopt.push_back(round(cplex.getValue(objectives[i])));
            }

            // Clean up the CPLEX environment

            model.remove(objective);
            for (auto constr : constraints) {
                model.remove(constr);
            }
            return std::make_pair(xopt, yopt);
        }
    }
    catch (IloException e) {
        std::cerr << "Error: " << e.getMessage() << std::endl;
    }


    model.remove(objective);

    for (auto constr : constraints) {
        model.remove(constr);
    }


    std::vector<int> xopt;
    std::vector<int> yopt;
    return std::make_pair(xopt, yopt);
}

std::pair<std::vector<int>, std::vector<int>> epsilon_second_stage(std::vector<int>& u, std::vector<int>& y_start,
    IloEnv& env, IloModel& model, IloCplex& cplex, IloNumVarArray& x, IloNumArray& startVal,
    std::vector<IloExpr>& objectives, std::vector<int>& x_start, const std::vector<int>& order = { 1,0,2 })
{
    //warm start
    if (!x_start.empty()) {
        for (int i = 0; i < x.getSize(); i++) {
            startVal[i] = x_start[i];
        }
    }


    IloObjective objective = IloObjective(env, objectives[order[0]], IloObjective::Minimize);
    model.add(objective);


    // Add constraints
    std::vector<IloConstraint> constraints;
    for (int i = 0; i < 3; i++) {
        if (i == 2) {
            IloConstraint constr = IloConstraint(objectives[order[i]] <= y_start[order[i]]);
            constraints.push_back(constr);
            model.add(constr);
        }
        else if (u[order[i]] < INT_MAX) {
            IloConstraint constr = IloConstraint(objectives[order[i]] <= u[order[i]] - 1);
            constraints.push_back(constr);
            model.add(constr);
        }
    }

    cplex.addMIPStart(x, startVal); //warm start the model
    try {
        cplex.solve(); // Solve the optimization problem

        if (cplex.getStatus() == IloAlgorithm::Optimal) {
            // Extract and return the optimal solution
            std::vector<int> xopt(x.getSize());
            for (int i = 0; i < x.getSize(); i++) {
                xopt[i] = std::round(cplex.getValue(x[i]));
            }
            std::vector<int> yopt;
            for (int i = 0; i < 3; i++) {
                yopt.push_back(round(cplex.getValue(objectives[i])));
            }
            // Clean up the CPLEX environment
            model.remove(objective);
            for (auto constr : constraints) {
                model.remove(constr);
            }

            return std::make_pair(xopt, yopt);
        }
        else {
            std::cerr << "Optimal solution not found." << std::endl;
        }
    }
    catch (IloException e) {
        std::cerr << "Error: " << e.getMessage() << std::endl;
    }

    // Clean up the CPLEX environment
    model.remove(objective);
    for (auto constr : constraints) {
        model.remove(constr);
    }
    std::vector<int> xopt;
    std::vector<int> yopt;
    return std::make_pair(xopt, yopt);
}

std::pair<std::vector<int>, std::vector<int>> epsilon_third_stage(std::vector<int>& u, std::vector<int>& y_start,
    IloEnv& env, IloModel model, IloCplex& cplex, IloNumVarArray& x, IloNumArray& startVal,
    std::vector<IloExpr>& objectives, std::vector<int>& x_start, const std::vector<int>& order = { 1,0,2 })
{
    //warm start
    if (!x_start.empty()) {
        for (int i = 0; i < x.getSize(); i++) {
            startVal[i] = x_start[i];
        }
    }


    IloObjective objective = IloObjective(env, objectives[order[1]], IloObjective::Minimize);
    model.add(objective);
    cplex.extract(model);

    // Add constraints
    std::vector<IloConstraint> constraints;
    for (int i = 0; i < 3; i++) {
        if (i != 1) {
            IloConstraint constr = IloConstraint(objectives[order[i]] <= y_start[order[i]]);
            constraints.push_back(constr);
            model.add(constr);
        }
        else if (u[order[i]] < INT_MAX) {
            IloConstraint constr = IloConstraint(objectives[order[i]] <= u[order[i]] - 1);
            constraints.push_back(constr);
            model.add(constr);
        }
    }

    cplex.addMIPStart(x, startVal); //warm start the model
    try {
        cplex.solve(); // Solve the optimization problem

        if (cplex.getStatus() == IloAlgorithm::Optimal) {
            // Extract and return the optimal solution
            std::vector<int> xopt(x.getSize());
            for (int i = 0; i < x.getSize(); i++) {
                xopt[i] = std::round(cplex.getValue(x[i]));               
            }
            std::vector<int> yopt;
            for (int i = 0; i < 3; i++) {
                yopt.push_back(round(cplex.getValue(objectives[i])));
            }
            // Clean up the CPLEX environment
            model.remove(objective);
            for (auto constr : constraints) {
                model.remove(constr);
            }
            return std::make_pair(xopt, yopt);
        }
        else {
            std::cerr << "Optimal solution not found." << std::endl;
        }
    }
    catch (IloException e) {
        std::cerr << "Error: " << e.getMessage() << std::endl;
    }

    // Clean up the CPLEX environment
    model.remove(objective);
    for (auto constr : constraints) {
        model.remove(constr);
    }
    std::vector<int> xopt;
    std::vector<int> yopt;
    return std::make_pair(xopt, yopt);
}



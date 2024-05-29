#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>
#include <tuple>
#include <chrono>
#include "cplex_interface.h"
#include <omp.h>
#include <unordered_map>
#include <array>
#include <thread>
#include <json.hpp>



void calculate_successors(std::vector<IloEnv>& environments,
    std::vector<IloModel>& models,
    std::vector<IloCplex>& cplexes,
    std::vector<IloNumArray>& startVals,
    std::vector<IloNumVarArray>& xs,
    std::vector<std::vector<IloExpr>>& objectivess, std::vector< std::vector<std::pair<std::vector<int>,
    std::vector<int>>>>& threadY_N,
    std::vector<int> y, std::vector<int> z,bool& found_feasible,std::vector<std::pair<std::vector<int>, std::vector<int>>>& feasible,
    std::vector<std::array<int,5>>& counter_scalarizations) {


        int i;
        int ID = omp_get_thread_num();
        bool found_start,feasible_generation_done;
            std::vector<int> y_new, y_feasible, x_new, x_feasible, u;
            while (!z.empty() && z[1] >= y[1]) {

                u = { y[0],z[1],0 };
                i=0;
                feasible_generation_done=found_feasible;
                found_start = false;
                while (!found_start && i < feasible.size())
                {
                    if (feasible[i].first[0] < u[0] && feasible[i].first[1] < u[1]) {
                        found_start = true;
                        x_feasible = feasible[i].second;
                        y_feasible = feasible[i].first;

                    }
                i++;
                }
                if (found_start){
                    std::tie(x_new, y_new) = epsilon_first_stage(u, environments[ID], models[ID], cplexes[ID], xs[ID], startVals[ID], objectivess[ID], x_feasible);
                    counter_scalarizations[ID][0]++;
                }
                else if (!feasible_generation_done){
                    std::tie(x_new, y_new) = epsilon_first_stage(u, environments[ID], models[ID], cplexes[ID], xs[ID], startVals[ID], objectivess[ID]);
                    counter_scalarizations[ID][0]++;
                    counter_scalarizations[ID][4]++;
                    if (x_new.empty()){
                        counter_scalarizations[ID][3]++;
                    }
                }
                else {
                    x_new.clear();
                }

                if (!x_new.empty()) {
                    //call second_stage and reassign x_new and y_new
                    std::tie(x_new,y_new) = epsilon_second_stage(u, y_new, environments[ID], models[ID], cplexes[ID], xs[ID], startVals[ID], objectivess[ID], x_new);
                    counter_scalarizations[ID][1]++;
                    //call third_stage if necessary and reassign x_new and y_new
                    if (found_start && y_new[2]==y_feasible[2] && y_new[1]==y_feasible[1])
                    {
                        y_new = y_feasible;
                        x_new = x_feasible;
                    }
                    else {
                        std::tie(x_new,y_new) = epsilon_third_stage(u, y_new, environments[ID], models[ID], cplexes[ID], xs[ID], startVals[ID], objectivess[ID], x_new);
                        counter_scalarizations[ID][2]++;
                    }
                    if (y_new[0] >= z[0]) {

                        threadY_N[ID].emplace_back(y_new, x_new);   
                        #pragma omp task shared(environments,models,cplexes,startVals,xs,objectivess,threadY_N,feasible,found_feasible,counter_scalarizations)         
                        { calculate_successors(environments, models, cplexes, startVals, xs, objectivess, threadY_N, y_new, z,found_feasible,feasible,counter_scalarizations);}

                    }
                    z = y_new;
                }
                else {
                    z.clear();
                }
            }
}

bool feasible_generation(IloEnv& env, IloModel& model, IloCplex& cplex, IloNumVarArray& x, IloNumArray& startVal,
    std::vector<IloExpr>& objectives,std::vector<std::pair<std::vector<int>, std::vector<int>>>& feasible,
    std::vector<std::array<int,5>>& counter_scalarizations)
{   
    //generate single objective minima 
    int ID = omp_get_thread_num();
    std::vector<int> order = {2,1,0};
    std::vector<int> u = { INT_MAX, INT_MAX, INT_MAX };
    std::vector<int> initial_y, initial_x;
    std::tie(initial_x, initial_y) = epsilon_first_stage(u, env, model, cplex, x, startVal, objectives, std::vector<int>(), order);
    counter_scalarizations[ID][0]++;
    counter_scalarizations[ID][4]++;
    std::tie(initial_x, initial_y) = epsilon_second_stage(u, initial_y, env, model, cplex, x, startVal, objectives, initial_x, order);
    counter_scalarizations[ID][1]++;
    std::tie(initial_x, initial_y) = epsilon_third_stage(u, initial_y, env, model, cplex, x, startVal, objectives, initial_x, order);
    counter_scalarizations[ID][2]++;
    std::vector<int> feasible_y, feasible_x;
    
    order = {2,0,1};
    std::tie(feasible_x,feasible_y) = epsilon_first_stage(u, env, model, cplex, x, startVal, objectives, initial_x, order);
    counter_scalarizations[ID][0]++;
    std::tie(feasible_x,feasible_y) = epsilon_second_stage(u, feasible_y, env, model, cplex, x, startVal, objectives, feasible_x, order);
    counter_scalarizations[ID][1]++;
    std::tie(feasible_x,feasible_y) = epsilon_third_stage(u, feasible_y, env, model, cplex, x, startVal, objectives, feasible_x, order);
    counter_scalarizations[ID][2]++;
    //feasible generation
    order = {2,1,0};
    std::vector<int> y, z, y_new, x_new;
    feasible.emplace_back(initial_y,initial_x);
    z = { INT_MIN, INT_MIN, INT_MAX };
    y = initial_y;
    while (y[1] > feasible_y[1]) {
            u = { 0, y[1], z[2] };
            std::tie(x_new, y_new) = epsilon_first_stage(u, env, model, cplex, x, startVal, objectives, feasible_x,order);
            counter_scalarizations[ID][0]++;
            std::tie(x_new, y_new) = epsilon_second_stage(u, y_new, env, model, cplex, x, startVal, objectives, x_new, order);
            counter_scalarizations[ID][1]++;
            if (y_new[2]==feasible_y[2] && y_new[0]==feasible_y[0]){
                y_new=feasible_y;
                x_new=feasible_x;
            }
            else {
                std::tie(x_new,y_new)=epsilon_third_stage(u,y_new,env,model,cplex,x,startVal,objectives,x_new,order);
                counter_scalarizations[ID][2]++;
            }           
            
            feasible.emplace_back(y_new, x_new);
            y = y_new;
        }
    return true;
}
void initialize_parallel_tasks(const std::string & instance, const std::string & result_location, int THREADS) {
    std::chrono::steady_clock::time_point  start = std::chrono::steady_clock::now();
    //define stuff for each thread
    std::vector<IloEnv> environments;
    std::vector<IloModel> models;
    std::vector<IloCplex> cplexes;
    std::vector<IloNumArray> startVals;
    std::vector<IloNumVarArray> xs;
    std::vector<std::vector<IloExpr>> objectivess;
    std::vector<std::vector<std::pair<std::vector<int>, std::vector<int>>>> threadY_N(THREADS);
    std::vector<std::array<int,5>> counter_scalarizations(THREADS,{0,0,0,0,0});
    for (int i = 0; i < THREADS; i++) {
        IloEnv env;
        IloModel model;
        IloNumVarArray x;
        IloNumArray startVal;
        IloCplex cplex;
        std::vector<IloExpr> objectives;
        std::tie(env, cplex, model, x, startVal, objectives) = createCplexInterface(instance);
        environments.push_back(env);
        models.push_back(model);
        xs.push_back(x);
        startVals.push_back(startVal);
        cplexes.push_back(cplex);
        objectivess.push_back(objectives);
    }
    omp_set_num_threads (THREADS);
    std::vector<std::pair<std::vector<int>, std::vector<int>>> feasible;
    std::chrono::steady_clock::time_point end_initialization= std::chrono::steady_clock::now();
    bool found_feasible = false;
    #pragma omp parallel
    {
    #pragma omp single
    {
        #pragma omp task shared(environments,models,cplexes,startVals,xs,objectivess,threadY_N,feasible,found_feasible,counter_scalarizations)   
        {
        int ID=omp_get_thread_num();
        found_feasible=feasible_generation(environments[ID],models[ID],cplexes[ID],xs[ID],startVals[ID],objectivess[ID],feasible,counter_scalarizations);  
        }

        #pragma omp task shared(environments,models,cplexes,startVals,xs,objectivess,threadY_N,feasible,found_feasible,counter_scalarizations)
        {
        calculate_successors(environments, models, cplexes, startVals, xs, objectivess, threadY_N, { INT_MAX,INT_MIN,INT_MIN }, { INT_MIN,INT_MAX,INT_MIN },found_feasible,feasible,counter_scalarizations);
        }
    }
    }
    std::chrono::steady_clock::time_point  end = std::chrono::steady_clock::now();
    //output

    nlohmann::json output;
    nlohmann::json scalarizations;
    nlohmann::json threads;
    nlohmann::json nondominated_set=nlohmann::json::array();
    std::array<int,6> number_scalarizations{};
    
    int number_nondominated = 0;
    for (int i=0; i< THREADS; i++){
        nlohmann::json thread;
        nlohmann::json thread_scalarizations;
        thread_scalarizations["first_stage"]=counter_scalarizations[i][0];
        thread_scalarizations["second_stage"]=counter_scalarizations[i][1];
        thread_scalarizations["third_stage"]=counter_scalarizations[i][2];
        thread_scalarizations["infeasible"]=counter_scalarizations[i][3];
        thread_scalarizations["no_warm_start"]=counter_scalarizations[i][4];
        thread_scalarizations["total"]=0;
        number_scalarizations[0]+=counter_scalarizations[i][0];
        number_scalarizations[1]+=counter_scalarizations[i][1];;
        number_scalarizations[2]+=counter_scalarizations[i][2];
        number_scalarizations[3]+=counter_scalarizations[i][3];
        number_scalarizations[4]+=counter_scalarizations[i][4];
        int total_scalarizations=0;
        for (int j=0;j<3;j++ ){
            number_scalarizations[5]+=counter_scalarizations[i][j];
            total_scalarizations+=counter_scalarizations[i][j];
        }
        thread_scalarizations["total"]=total_scalarizations;
        thread["#scalarizations"]=thread_scalarizations;
        thread["#nondominated"]=threadY_N[i].size();
        number_nondominated+=threadY_N[i].size();
        output[std::to_string(i)]=thread;
        for (int k=0; k < threadY_N[i].size();k++){
            nondominated_set.push_back(threadY_N[i][k].first);
        }
    }
    scalarizations["first_stage"]=number_scalarizations[0];
    scalarizations["second_stage"]=number_scalarizations[1];
    scalarizations["third_stage"]=number_scalarizations[2];
    scalarizations["infeasible"]=number_scalarizations[3];
    scalarizations["no_warm_start"]=number_scalarizations[4];
    scalarizations["total"]=number_scalarizations[5];
    output["Y_N"]=nondominated_set;
    output["threads"] = THREADS;
    output["|Y_N3|"]=feasible.size();
    output["#scalarizations"]=scalarizations;
    output["cpu(micro_s)"]=std::chrono::duration_cast<std::chrono::microseconds>(end-start).count();
    output["|Y_N|"] = number_nondominated;
    output["instanz"] = instance;
    output["cpu_init(micro_s)"]=std::chrono::duration_cast<std::chrono::microseconds>(end_initialization-start).count();
    std::ofstream outfile(result_location.c_str());
    outfile << output.dump();
    outfile.close();
}

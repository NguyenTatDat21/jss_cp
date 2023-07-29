#define IL_STD
#include <iostream>
#include <memory>
#include <fstream>
#include "data_manager.h"

#include "ortools/base/logging.h"
#include "ortools/sat/cp_model.h"
#include "ortools/sat/cp_model.pb.h"
#include "ortools/sat/cp_model_solver.h"
#include "ortools/util/sorted_interval_list.h"
#include "solution_sequence.h"

#include <ilcp/cp.h>
#include <chrono>

void CplexModel(const std::shared_ptr<DataManager> &dm, const std::shared_ptr<SolutionSequence> &ss = nullptr) {
    IloEnv env;
    IloModel model(env);
    IloIntervalVarArray2 x(env, dm->n_job);
    IloIntVar c_max(env, 0, 1000000);
    IloIntExprArray ends(env);
    for (int i = 0; i < dm->n_job; ++i) {
        x[i] = IloIntervalVarArray(env, dm->n_machine);
        for (int j = 0; j < dm->n_machine; ++j) {
            x[i][j] = IloIntervalVar(env, dm->tasks[i][j]->interval);
            x[i][j].setStartMin(0);
            x[i][j].setEndMin(0);
            x[i][j].setStartMax(1000000);
            x[i][j].setEndMax(1000000);
            ends.add(IloEndOf(x[i][j]));
        }
    }
    for (int j = 0; j < dm->n_machine; ++j) {
        IloIntervalVarArray seq(env);
        for (int i = 0; i < dm->n_job; ++i) {
            seq.add(x[i][j]);
        }
        model.add(IloNoOverlap(env, seq));
    }

    model.add(c_max == IloMax(ends));
    for (int i = 0; i < dm->n_job; ++i) {
        for (int j = 0; j < dm->n_machine; ++j) {
            for (int k = 0; k < j; ++k) {
                int p_m = dm->jobs[i]->tasks[k]->machine->id;
                int m = dm->jobs[i]->tasks[j]->machine->id;
                model.add(IloEndBeforeStart(env, x[i][p_m], x[i][m]));
            }
        }
    }
    model.add(IloMinimize(env, c_max));
    IloCP cplex(model);
    cplex.setParameter(IloCP::NumParam::TimeLimit, 3600.0);
    cplex.setOut(env.getNullStream());
    auto start = std::chrono::steady_clock::now();
    cplex.solve();
    std::cout << cplex.getObjBound() << " " << cplex.getObjValue() << " " << std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start).count() << "\n";

}

namespace operations_research::sat{
    void Jscp1(const std::shared_ptr<DataManager> &dm, const std::shared_ptr<SolutionSequence> &ss = nullptr) {
        CpModelBuilder cp_model;
        Domain time_domain(0, 1000000);
        std::vector<std::vector<IntervalVar>> x;
        x.resize(dm->n_job, std::vector<IntervalVar>(dm->n_machine));
        IntVar c_max = cp_model.NewIntVar(time_domain);
        std::vector<LinearExpr> ends;
        std::vector<IntervalVar> itvs;
        for (int i = 0; i < dm->n_job; ++i) {
            for (int j = 0; j < dm->n_machine; ++j) {
                IntVar start = cp_model.NewIntVar(time_domain);
                x[i][j] = cp_model.NewFixedSizeIntervalVar(start, dm->tasks[i][j]->interval);
                ends.push_back(x[i][j].EndExpr());
                itvs.push_back(x[i][j]);
                cp_model.AddGreaterOrEqual(c_max, x[i][j].EndExpr());
            }
        }
        cp_model.Minimize(c_max);
        cp_model.AddMaxEquality(c_max, ends);

        for (int i = 0; i < dm->n_job; ++i) {
            for (int j = 1; j < dm->n_machine; ++j) {
                int p_m = dm->jobs[i]->tasks[j-1]->machine->id;
                int m = dm->jobs[i]->tasks[j]->machine->id;
                cp_model.AddGreaterOrEqual(x[i][m].StartExpr(), x[i][p_m].EndExpr());
            }
        }
        for (int i = 0; i < dm->n_job; ++i) {
            for (int j = 0; j < dm->n_machine; ++j) {
                for (int k = 0; k < j; ++k) {
                    int p_m = dm->jobs[i]->tasks[k]->machine->id;
                    int m = dm->jobs[i]->tasks[j]->machine->id;
                    cp_model.AddGreaterOrEqual(x[i][m].StartExpr(), x[i][p_m].EndExpr());
                }
            }
        }
        for (int j = 0; j < dm->n_machine; ++j) {
            std::vector<IntervalVar> seq;
            for (int i = 0; i < dm->n_job; ++i) {
                seq.push_back(x[i][j]);
            }
            cp_model.AddNoOverlap(seq);
        }
//        for (int i = 0; i < dm->n_job; ++i) {
//            std::vector<IntervalVar> seq;
//            for (int j = 0; j < dm->n_machine; ++j) {
//                seq.push_back(x[i][j]);
//            }
//            cp_model.AddNoOverlap(seq);
//        }
        // Extra constraints
        LinearExpr sum;
        for (int i = 0; i < dm->n_job; ++i) {
            int m = dm->jobs[i]->tasks.back()->machine->id;
            sum += x[i][m].EndExpr();
        }
//        cp_model.AddGreaterOrEqual(c_max * dm->n_job, sum);
        if (ss != nullptr) {
            for (int j = 0; j < dm->n_machine; ++j) {
                for (int i = 1; i < dm->n_job; ++i) {
                    cp_model.AddGreaterOrEqual(x[ss->seq[j][i]][j].StartExpr(), x[ss->seq[j][i-1]][j].EndExpr());
                }
            }
        }

        SatParameters parameters;
//        parameters.set_log_search_progress(true);
        parameters.set_max_time_in_seconds(3600);
//        parameters.set_use_combined_no_overlap(true);
        Model model;

        model.Add(NewSatParameters(parameters));

        const CpSolverResponse response = SolveCpModel(cp_model.Build(), &model);

        if (response.status() == CpSolverStatus::OPTIMAL ||
            response.status() == CpSolverStatus::FEASIBLE) {
//            std::vector<std::vector<int>> start(dm->n_machine, std::vector<int>(dm->n_job));
//            for (int j = 0; j < dm->n_machine; ++j) {
//                std::vector<int> idx(dm->n_job);
//                for (int i = 0; i < dm->n_job; ++i) {
//                    idx[i] = i;
//                    start[j][i] = SolutionIntegerValue(response, x[i][j].EndExpr());
//                }
//                std::sort(idx.begin(), idx.end(), [&](int a, int b) {
//                    return start[j][a] < start[j][b];
//                });
//                for (int i : idx) {
//                    std::cout << i << " (" << start[j][i] << ") ";
//                }
//                std::cout << "\n";
//            }
            std::cout << response.best_objective_bound() << " " << response.objective_value() << " " << response.user_time() << "\n";
//            LOG(INFO) << response.best_objective_bound() << " " << response.objective_value() << " " << response.user_time();
        } else {
            LOG(INFO) << "No solution found.";
        }
    }

    void Jscp2(const std::shared_ptr<DataManager> &dm, const std::shared_ptr<SolutionSequence> &ss = nullptr) {
        CpModelBuilder cp_model;
        Domain time_domain(0, 1000000);
        std::vector<std::vector<IntervalVar>> x;
        x.resize(dm->n_job, std::vector<IntervalVar>(dm->n_machine));
        IntVar c_max = cp_model.NewIntVar(time_domain);
        std::vector<LinearExpr> ends;
        std::vector<IntervalVar> itvs;
        for (int i = 0; i < dm->n_job; ++i) {
            for (int j = 0; j < dm->n_machine; ++j) {
                IntVar start = cp_model.NewIntVar(time_domain);
                x[i][j] = cp_model.NewFixedSizeIntervalVar(start, dm->tasks[i][j]->interval);
                std::cout << dm->tasks[i][j]->interval << " ";
                ends.push_back(x[i][j].EndExpr());
                itvs.push_back(x[i][j]);
                cp_model.AddGreaterOrEqual(c_max, x[i][j].EndExpr());
            }
        }
        cp_model.Minimize(c_max);
        cp_model.AddMaxEquality(c_max, ends);

        for (int i = 0; i < dm->n_job; ++i) {
            for (int j = 1; j < dm->n_machine; ++j) {
                int p_m = dm->jobs[i]->tasks[j-1]->machine->id;
                int m = dm->jobs[i]->tasks[j]->machine->id;
                cp_model.AddGreaterOrEqual(x[i][m].StartExpr(), x[i][p_m].EndExpr());
            }
        }
//        for (int i = 0; i < dm->n_job; ++i) {
//            for (int j = 0; j < dm->n_machine; ++j) {
//                for (int k = 0; k < j; ++k) {
//                    int p_m = dm->jobs[i]->tasks[k]->machine->id;
//                    int m = dm->jobs[i]->tasks[j]->machine->id;
//                    cp_model.AddGreaterOrEqual(x[i][m].StartExpr(), x[i][p_m].EndExpr());
//                }
//            }
//        }
        auto cc = cp_model.AddCumulative(dm->n_machine);
        for (int j = 0; j < dm->n_machine; ++j) {
            std::vector<IntervalVar> seq;
            auto c = cp_model.AddCumulative(1);
            for (int i = 0; i < dm->n_job; ++i) {
                seq.push_back(x[i][j]);
                c.AddDemand(x[i][j], 1);
                cc.AddDemand(x[i][j], 1);
            }
//            cp_model.AddNoOverlap(seq);

        }
        for (int i = 0; i < dm->n_job; ++i) {
            std::vector<IntervalVar> seq;
            auto c = cp_model.AddCumulative(1);
            for (int j = 0; j < dm->n_machine; ++j) {
                c.AddDemand(x[i][j], 1);
            }
        }
        // Extra constraints
        LinearExpr sum;
        for (int i = 0; i < dm->n_job; ++i) {
            int m = dm->jobs[i]->tasks.back()->machine->id;
            sum += x[i][m].EndExpr();
        }
//        cp_model.AddGreaterOrEqual(c_max * dm->n_job, sum);
        if (ss != nullptr) {
            for (int j = 0; j < dm->n_machine; ++j) {
                for (int i = 1; i < dm->n_job; ++i) {
                    cp_model.AddGreaterOrEqual(x[ss->seq[j][i]][j].StartExpr(), x[ss->seq[j][i-1]][j].EndExpr());
                }
            }
        }

        SatParameters parameters;
        parameters.set_log_search_progress(true);
        parameters.set_max_time_in_seconds(3600);
//        parameters.set_use_combined_no_overlap(true);
        Model model;

        model.Add(NewSatParameters(parameters));

        const CpSolverResponse response = SolveCpModel(cp_model.Build(), &model);

        if (response.status() == CpSolverStatus::OPTIMAL ||
            response.status() == CpSolverStatus::FEASIBLE) {
            std::vector<std::vector<int>> start(dm->n_machine, std::vector<int>(dm->n_job));
            for (int j = 0; j < dm->n_machine; ++j) {
                std::vector<int> idx(dm->n_job);
                for (int i = 0; i < dm->n_job; ++i) {
                    idx[i] = i;
                    start[j][i] = SolutionIntegerValue(response, x[i][j].EndExpr());
                }
                std::sort(idx.begin(), idx.end(), [&](int a, int b) {
                    return start[j][a] < start[j][b];
                });
                for (int i : idx) {
                    std::cout << i << " (" << start[j][i] << ") ";
                }
                std::cout << "\n";
            }
            LOG(INFO) << response.best_objective_bound() << " " << response.objective_value() << " " << response.user_time();
        } else {
            LOG(INFO) << "No solution found.";
        }
    }
}

void run_all_instances() {
    std::ifstream ifs("/home/tatdatnguyen/CLionProjects/jss-cp/data/filename.txt");
    std::vector<std::string> instances_name;
    std::string ss;
    while (ifs >> ss) {
        instances_name.push_back(ss);
    }
    ifs.close();
    for (auto &s : instances_name) {
        std::string filename = "/home/tatdatnguyen/CLionProjects/jss-cp/data/rc/" + s;
        std::shared_ptr<DataManager> dm = std::make_shared<DataManager>(filename, "de");
//        operations_research::sat::Jscp1(dm);
        CplexModel(dm);
    }
};

int main(int argc, char** argv) {
//    auto dm = std::make_shared<DataManager>(argv[1], "de");
//    std::shared_ptr<SolutionSequence> ss;
//    if (argc > 2) {
//        ss = std::make_shared<SolutionSequence>(dm, argv[2]);
//    }
//    CplexModel(dm, ss);
    run_all_instances();
    return 0;
}

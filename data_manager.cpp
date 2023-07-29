//
// Created by tatdatnguyen on 15/07/23.
//

#include <iostream>
#include <fstream>
#include <sstream>
#include "data_manager.h"

DataManager::DataManager(const std::string &filename, const std::string &data_type) {
    std::ifstream ifs(filename);
    if (data_type == "ta") {
        std::string s;
        getline(ifs, s);
        char c;
        ifs >> n_job >> c >> n_machine;
        getline(ifs, s);
        getline(ifs, s);
        machines.resize(n_machine);
        jobs.resize(n_job);
        tasks.resize(n_job, std::vector<std::shared_ptr<Task>>(n_machine));
        for (int i = 0; i < n_machine; ++i) {
            machines[i] = std::make_shared<Machine>();
            machines[i]->id = i;
        }
        for (int i = 0; i < n_job; ++i) {
            jobs[i] = std::make_shared<Job>();
        }
        for (int i = 0; i < n_job; ++i) {
            for (int j = 0; j < n_machine; ++j) {
                tasks[i][j] = std::make_shared<Task>();
                ifs >> tasks[i][j]->interval;
                if (j < n_machine - 1) {
                    ifs >> c;
                }
                tasks[i][j]->job = jobs[i];
                tasks[i][j]->machine = machines[j];
            }
        }
        getline(ifs, s);
        getline(ifs, s);
        for (int i = 0; i < n_job; ++i) {
            for (int j = 0; j < n_machine; ++j) {
                int id;
                ifs >> id;
                --id;
                if (j < n_machine - 1) {
                    ifs >> c;
                }
                jobs[i]->tasks.push_back(tasks[i][id]);
            }
        }
    } else {
        ifs >> n_job >> n_machine;
        machines.resize(n_machine);
        jobs.resize(n_job);
        tasks.resize(n_job, std::vector<std::shared_ptr<Task>>(n_machine));
        for (int i = 0; i < n_machine; ++i) {
            machines[i] = std::make_shared<Machine>();
            machines[i]->id = i;
        }
        for (int i = 0; i < n_job; ++i) {
            jobs[i] = std::make_shared<Job>();
        }
        for (int i = 0; i < n_job; ++i) {
            for (int j = 0; j < n_machine; ++j) {
                tasks[i][j] = std::make_shared<Task>();
            }
            for (int j = 0; j < n_machine; ++j) {
                int itv, id;
                ifs >> id >> itv;
                tasks[i][id]->interval = itv;
                tasks[i][id]->job = jobs[i];
                tasks[i][id]->machine = machines[id];
                jobs[i]->tasks.push_back(tasks[i][id]);
            }
        }
    }
    ifs.close();
}

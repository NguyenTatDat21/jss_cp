//
// Created by tatdatnguyen on 15/07/23.
//

#ifndef JSS_CP_DATA_MANAGER_H
#define JSS_CP_DATA_MANAGER_H


#include <string>
#include <vector>
#include <memory>

struct Task;

struct Machine {
    int id;
    std::vector<std::shared_ptr<Task>> tasks;
};

struct Job {
    std::vector<std::shared_ptr<Task>> tasks;
};

struct Task {
    std::shared_ptr<Job> job;
    std::shared_ptr<Machine> machine;
    int interval;
};

class DataManager {
public:
    int n_job;
    int n_machine;
    std::vector<std::shared_ptr<Job>> jobs;
    std::vector<std::shared_ptr<Machine>> machines;
    std::vector<std::vector<std::shared_ptr<Task>>> tasks;
    explicit DataManager(const std::string &filename, const std::string &data_type = "ta");

};


#endif //JSS_CP_DATA_MANAGER_H

//
// Created by tatdatnguyen on 20/07/23.
//

#ifndef JSS_CP_SOLUTION_SEQUENCE_H
#define JSS_CP_SOLUTION_SEQUENCE_H


#include <memory>
#include "data_manager.h"

class SolutionSequence {
public:
    std::shared_ptr<DataManager> dm;
    std::vector<std::vector<int>> seq;
    explicit SolutionSequence(const std::shared_ptr<DataManager> &dm, const std::string &filename);
};


#endif //JSS_CP_SOLUTION_SEQUENCE_H

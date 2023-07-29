//
// Created by tatdatnguyen on 20/07/23.
//

#include <fstream>
#include "solution_sequence.h"

SolutionSequence::SolutionSequence(const std::shared_ptr<DataManager> &dm, const std::string &filename) {
    this->dm = dm;
    std::ifstream ifs(filename);
    seq.resize(dm->n_machine, std::vector<int>(dm->n_job));
    for (int j = 0; j < dm->n_machine; ++j) {
        for (int i = 0; i < dm->n_job; ++i) {
            ifs >> seq[j][i];
        }
    }
}

//
// Created by beni on 11/8/2026.
//

#ifndef MINI_H
#define MINI_H

#pragma once
#include "../problem.hpp"
#include "../cxxopts.hpp"
#include <string>
#include <vector>
#include <set>
#include <map>

using namespace std;

class Minimalizalas : public Problem {
public:
    void initialize_parser(cxxopts::Options& options) override;
    bool is_chosen_problem(const cxxopts::ParseResult& args) override;
    int run(const cxxopts::ParseResult& args) override;
};

#endif //MINI_H

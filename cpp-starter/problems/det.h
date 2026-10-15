//
// Created by beni on 10/15/2026.
//

#ifndef DET_H
#define DET_H
#pragma once
#include "../problem.hpp"
#include "../cxxopts.hpp"
#include <string>
#include <vector>
#include <set>
#include <map>

using namespace std;

class Determinizalas : public Problem {
public:
    void initialize_parser(cxxopts::Options& options) override;
    bool is_chosen_problem(const cxxopts::ParseResult& args) override;
    int run(const cxxopts::ParseResult& args) override;
};
#endif //DET_H

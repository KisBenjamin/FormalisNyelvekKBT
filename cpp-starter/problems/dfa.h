//
// Created by beni on 10/4/2026.
//

#ifndef DFA_H
#define DFA_H

#pragma once
#include "../problem.hpp"
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include "../cxxopts.hpp"

using namespace std;

class DFA : public Problem {
private:
    string start_state;
    unordered_set<string> final_states;
    // Map: Current State -> (Character -> Next State)
    unordered_map<string, unordered_map<char, string>> transitions;

    void parse_input(const string& filename);
    bool simulate(const string& word);

public:
    void initialize_parser(cxxopts::Options& options) override;
    bool is_chosen_problem(const cxxopts::ParseResult& args) override;
    int run(const cxxopts::ParseResult& args) override;
};

#endif //DFA_H

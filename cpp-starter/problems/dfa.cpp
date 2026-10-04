//
// Created by beni on 10/4/2026.
//

#include "dfa.h"
#include <iostream>
#include <fstream>
#include <sstream>

void DFA::initialize_parser(cxxopts::Options& options) {
    options.add_options()
        ("c,check", "Words to check", cxxopts::value<string>());
}

bool DFA::is_chosen_problem(const cxxopts::ParseResult& args) {
    // --check flag == RUN dfa problems
    return args.count("check") > 0;
}

void DFA::parse_input(const string& filename) {
    ifstream infile(filename);
    if (!infile.is_open()) {
        cerr << "Error: Could not open input file " << filename << "\n";
        return;
    }

    string line;
    // L1: States
    getline(infile, line);

    // L2: Alphabet
    getline(infile, line);

    // L3: Start state
    getline(infile, line);
    stringstream ss_start(line);
    ss_start >> start_state;

    // L4: Final states
    getline(infile, line);
    stringstream ss_final(line);
    string f_state;
    while (ss_final >> f_state) {
        final_states.insert(f_state);
    }

    // Remaining lines ... transitions (state1 symbol state2)
    string state1, state2;
    char symbol;
    while (infile >> state1 >> symbol >> state2) {
        transitions[state1][symbol] = state2;
    }
}

bool DFA::simulate(const string& word) {
    string current_state = start_state;
    for (char c : word) {
        if (transitions[current_state].find(c) != transitions[current_state].end()) {
            current_state = transitions[current_state][c];
        } else {
            return false; // no valid transition == reject
        }
    }
    // Final state == accept
    return final_states.find(current_state) != final_states.end();
}

int DFA::run(const cxxopts::ParseResult& args) {
    // Extract args using cxxopts
    string input_file = args["input"].as<string>();
    string output_file = args["output"].as<string>();
    string check_words_str = args["check"].as<string>();

    parse_input(input_file);

    // Seperate words from the check argument
    vector<string> words_to_check;
    stringstream ss_words(check_words_str);
    string word;
    while (getline(ss_words, word, ',')) {
        words_to_check.push_back(word);
    }

    // Simulate and write
    ofstream outfile(output_file);
    if (!outfile.is_open()) {
        cerr << "Error: Could not open output file " << output_file << "\n";
        return 1;
    }

    for (const auto& w : words_to_check) {
        if (simulate(w)) {
            outfile << "IGEN\n";
        } else {
            outfile << "NEM\n";
        }
    }
    return 0;
}
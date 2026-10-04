#include <iostream>
#include <string>
#include <vector>

#include "cxxopts.hpp"

#include "problems/dfa.h"
#include "problems/sum.hpp"

using namespace std;

int runProblem(int argc, char* argv[]) {
    // Add your own problems here
    vector<Problem *> problems;
    problems.push_back(new SumProblem());
    problems.push_back(new DFA());

    cxxopts::Options options("project", "Run the specific problem");

    options.add_options()
        ("i,input", "Input file name", cxxopts::value<string>())
        ("o,output", "Output file name", cxxopts::value<string>())
        ("h,help", "Print usage");
    
    for (Problem *p : problems) {
        p->initialize_parser(options);
    }

    cxxopts::ParseResult args = options.parse(argc, argv);

    for (Problem *p : problems) {
        if (p->is_chosen_problem(args)) {
            p->run(args);
            break;
        }
    }

    cout << options.help() << endl;

    for (Problem *p : problems) {
        delete p;
    }
    return 0;
}

int main(int argc, char* argv[]) {
    try {
        return runProblem(argc, argv);
    } catch (const cxxopts::exceptions::exception &e) {
        cerr << "Error parsing options: " << e.what() << endl;
        return 1;
    }
}
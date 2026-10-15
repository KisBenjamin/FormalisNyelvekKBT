#include "det.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <queue>
#include <algorithm>

void Determinizalas::initialize_parser(cxxopts::Options& options) {
    // hozzaadjuk a custom flaget ehhez a hazihoz
    options.add_options()
        ("d,det", "Determinize the automaton");
}

bool Determinizalas::is_chosen_problem(const cxxopts::ParseResult& args) {
    // checkoljuk hogy a --det meg lett e adva
    return args.count("det") > 0;
}

int Determinizalas::run(const cxxopts::ParseResult& args) {
    if (args.count("input") == 0 || args.count("output") == 0) {
        cerr << "Hiba: Hianyzo bemeneti vagy kimeneti fajl argumentum!\n";
        return 1;
    }

    string input_file = args["input"].as<string>();
    string output_file = args["output"].as<string>();

    ifstream infile(input_file);
    if (!infile.is_open()) {
        cerr << "Hiba: Nem lehet megnyitni a bemeneti fajlt: " << input_file << "\n";
        return 1; // exit if file not found
    }

    string line;

    // 1. states beolvasasa
    getline(infile, line);
    stringstream ss_states(line);
    set<string> valid_states;
    string state;
    while (ss_states >> state) valid_states.insert(state);

    // 2. alphabet beolvasasa
    getline(infile, line);
    stringstream ss_abc(line);
    vector<string> alphabet;
    string symbol;
    while (ss_abc >> symbol) alphabet.push_back(symbol);

    // 3. start state check
    getline(infile, line);
    string start_state;
    stringstream ss_start(line);
    ss_start >> start_state;
    if (valid_states.find(start_state) == valid_states.end()) {
        cerr << "Hiba: A kezdoallapot ervenytelen (nincs a listaban)!\n";
        return 1;
    }

    // 4. final states check
    getline(infile, line);
    stringstream ss_final(line);
    set<string> final_states;
    string f_state;
    while (ss_final >> f_state) {
        if (valid_states.find(f_state) == valid_states.end()) {
            cerr << "Hiba: Ervenytelen vegallapot!\n";
            return 1;
        }
        final_states.insert(f_state);
    }

    // 5. transitions beolvasasa
    // map of map of sets, mert egy beture tobb fele is mehetunk
    map<string, map<string, set<string>>> transitions;
    string src, sym, dst;
    while (infile >> src >> sym >> dst) {
        // error handling: benne vannak e a valid listaban?
        if (valid_states.find(src) == valid_states.end() ||
            valid_states.find(dst) == valid_states.end() ||
            find(alphabet.begin(), alphabet.end(), sym) == alphabet.end()) {
            cerr << "Hiba: Ervenytelen allapot vagy szimbolum az atmeneteknel (" << src << " " << sym << " " << dst << ")!\n";
            return 1;
        }
        transitions[src][sym].insert(dst);
    }
    infile.close();

    // --- DETERMINIZALAS (subset construction) ---
    map<set<string>, string> dfa_state_names; // mapping set -> uj nev (s0, s1)
    vector<set<string>> dfa_states_ordered;   // kell a kiiro reszhez hogy jo legyen a sorrend
    int state_counter = 0;

    queue<set<string>> worklist; // BFS queue
    set<string> initial_subset = {start_state};

    // initial setup
    dfa_state_names[initial_subset] = "s" + to_string(state_counter++);
    dfa_states_ordered.push_back(initial_subset);
    worklist.push(initial_subset);

    // uj atmenetek tarolasa (src, symbol, dst tuple)
    vector<tuple<string, string, string>> dfa_transitions;

    while (!worklist.empty()) {
        set<string> current_subset = worklist.front();
        worklist.pop();

        string current_name = dfa_state_names[current_subset];

        // loop through the alphabet
        for (const string& a : alphabet) {
            set<string> next_subset;
            // csinalunk egy uniont az osszes elerheto allapotbol
            for (const string& q : current_subset) {
                if (transitions[q].find(a) != transitions[q].end()) {
                    for (const string& target : transitions[q][a]) {
                        next_subset.insert(target);
                    }
                }
            }

            // ha a halmaz nem ures
            if (!next_subset.empty()) {
                // ha ez egy uj state amit meg nem lattunk
                if (dfa_state_names.find(next_subset) == dfa_state_names.end()) {
                    dfa_state_names[next_subset] = "s" + to_string(state_counter++);
                    dfa_states_ordered.push_back(next_subset);
                    worklist.push(next_subset); // push to queue
                }
                // mentes a transition listaba
                dfa_transitions.push_back({current_name, a, dfa_state_names[next_subset]});
            }
        }
    }

    // final states kivalasztasa a DFA-hoz
    vector<string> dfa_finals;
    for (const auto& subset : dfa_states_ordered) {
        bool is_final = false;
        for (const string& q : subset) {
            // ha tartalmaz eredeti final state-t, akkor ez is az lesz
            if (final_states.find(q) != final_states.end()) {
                is_final = true;
                break;
            }
        }
        if (is_final) {
            dfa_finals.push_back(dfa_state_names[subset]);
        }
    }

    // --- OUTPUT KIIRAS ---
    ofstream outfile(output_file);
    if (!outfile.is_open()) {
        cerr << "Hiba: Nem lehet megnyitni a kimeneti fajlt: " << output_file << "\n";
        return 1;
    }

    // 1. sor: allapotok kiirasa space-el elvalasztva
    for (size_t i = 0; i < dfa_states_ordered.size(); ++i) {
        outfile << dfa_state_names[dfa_states_ordered[i]];
        if (i + 1 < dfa_states_ordered.size()) outfile << " ";
    }
    outfile << "\n";

    // 2. sor: abc
    for (size_t i = 0; i < alphabet.size(); ++i) {
        outfile << alphabet[i];
        if (i + 1 < alphabet.size()) outfile << " ";
    }
    outfile << "\n";

    // 3. sor: start state
    outfile << dfa_state_names[initial_subset] << "\n";

    // 4. sor: vegallapotok
    for (size_t i = 0; i < dfa_finals.size(); ++i) {
        outfile << dfa_finals[i];
        if (i + 1 < dfa_finals.size()) outfile << " ";
    }
    outfile << "\n";

    // atmenetek: abc sorrendben keri a feladat.
    // a std::sort pont lexikografikusan rendez tuple-oknel szoval ez igy tokeletes
    sort(dfa_transitions.begin(), dfa_transitions.end());

    for (const auto& t : dfa_transitions) {
        outfile << get<0>(t) << " " << get<1>(t) << " " << get<2>(t) << "\n";
    }

    outfile.close();
    return 0; // done
}
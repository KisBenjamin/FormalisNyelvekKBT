
#include "mini.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

void Minimalizalas::initialize_parser(cxxopts::Options& options) {
    // flag hozzaadasa a minimalizalashoz
    options.add_options()
        ("m,mini", "Minimize the automaton");
}

bool Minimalizalas::is_chosen_problem(const cxxopts::ParseResult& args) {
    // ha ott a --mini flag, akkor ezt futtatjuk
    return args.count("mini") > 0;
}

int Minimalizalas::run(const cxxopts::ParseResult& args) {
    if (args.count("input") == 0 || args.count("output") == 0) {
        cerr << "Hiba: Hianyzo bemeneti vagy kimeneti fajl!\n";
        return 1;
    }

    string input_file = args["input"].as<string>();
    string output_file = args["output"].as<string>();

    ifstream infile(input_file);
    if (!infile.is_open()) {
        cerr << "Hiba: Nem lehet megnyitni a bemeneti fajlt: " << input_file << "\n";
        return 1;
    }

    string line;

    // 1. states
    getline(infile, line);
    stringstream ss_states(line);
    set<string> valid_states;
    string state;
    while (ss_states >> state) valid_states.insert(state);

    // 2. alphabet
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
        cerr << "Hiba: Kezdoallapot nincs a valid listaban!\n";
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

    // 5. transitions beolvasas
    map<string, map<string, string>> transitions;
    string src, sym, dst;
    while (infile >> src >> sym >> dst) {
        if (valid_states.find(src) == valid_states.end() ||
            valid_states.find(dst) == valid_states.end() ||
            find(alphabet.begin(), alphabet.end(), sym) == alphabet.end()) {
            cerr << "Hiba: Ervenytelen adat az atmeneteknel!\n";
            return 1;
        }
        transitions[src][sym] = dst;
    }
    infile.close();

    // MINIMALIZALAS (Partition Refinement)
    // Kezdo felosztas: vegallapotok vs nem vegallapotok
    vector<set<string>> partitions;
    set<string> non_finals;
    for (const string& s : valid_states) {
        if (final_states.find(s) == final_states.end()) {
            non_finals.insert(s);
        }
    }
    if (!final_states.empty()) partitions.push_back(final_states);
    if (!non_finals.empty()) partitions.push_back(non_finals);

    bool changed = true;
    while (changed) {
        changed = false;
        vector<set<string>> new_partitions;

        // map the state to its current partition index (gyors kereseshez)
        map<string, int> state_to_part;
        for (size_t i = 0; i < partitions.size(); ++i) {
            for (const string& st : partitions[i]) {
                state_to_part[st] = i;
            }
        }

        // finomitjuk a csoportokat
        for (const auto& group : partitions) {
            map<vector<int>, set<string>> subgroups; // signature -> states
            for (const string& q : group) {
                vector<int> sig;
                for (const string& a : alphabet) {
                    if (transitions[q].find(a) != transitions[q].end()) {
                        string target = transitions[q][a];
                        sig.push_back(state_to_part[target]); // hova mutat az atmenet
                    } else {
                        sig.push_back(-1); // ha nincs atmenet
                    }
                }
                subgroups[sig].insert(q);
            }

            // ha egy group tobb darabra szakadt, akkor valtozott a rendszer
            for (const auto& kv : subgroups) {
                new_partitions.push_back(kv.second);
            }
            if (subgroups.size() > 1) changed = true;
        }
        partitions = new_partitions;
    }

    // UJ NEVEK ES ATMENETEK GENERALASA
    map<string, string> state_to_new_name;
    set<string> new_states; // abc sorrend miatt set

    for (const auto& group : partitions) {
        // keressuk a legkisebb nevet a csoportban (lexikografikusan)
        string min_state = *group.begin();
        for (const string& st : group) {
            if (st < min_state) min_state = st;
        }

        // nev atirasa: q1 -> s1
        string new_name = min_state;
        if (new_name.length() > 0 && new_name[0] == 'q') {
            new_name[0] = 's';
        } else {
            new_name = "s" + new_name;
        }

        new_states.insert(new_name);
        for (const string& st : group) {
            state_to_new_name[st] = new_name;
        }
    }

    // uj atmenetek letrehozasa es duplikaciok kiszurese
    // a set automatikusan sorbarendezi az elemeket (src, sym, dst) alapjan
    set<tuple<string, string, string>> new_transitions;
    for (const string& s : valid_states) {
        for (const string& a : alphabet) {
            if (transitions[s].find(a) != transitions[s].end()) {
                string target = transitions[s][a];
                new_transitions.insert({state_to_new_name[s], a, state_to_new_name[target]});
            }
        }
    }

    // KIIRAS
    ofstream outfile(output_file);
    if (!outfile.is_open()) {
        cerr << "Hiba: Nem tudom letrehozni az output fajlt!\n";
        return 1;
    }

    // 1. sor: allapotok
    bool first = true;
    for (const string& ns : new_states) {
        if (!first) outfile << " ";
        outfile << ns;
        first = false;
    }
    outfile << "\n";

    // 2. sor: abc
    first = true;
    for (const string& a : alphabet) {
        if (!first) outfile << " ";
        outfile << a;
        first = false;
    }
    outfile << "\n";

    // 3. sor: start state
    outfile << state_to_new_name[start_state] << "\n";

    // 4. sor: final states (kiszurve a duplikat)
    set<string> new_finals;
    for (const string& fs : final_states) {
        new_finals.insert(state_to_new_name[fs]);
    }
    first = true;
    for (const string& nf : new_finals) {
        if (!first) outfile << " ";
        outfile << nf;
        first = false;
    }
    outfile << "\n";

    // Atmenetek kiirasa
    for (const auto& t : new_transitions) {
        outfile << get<0>(t) << " " << get<1>(t) << " " << get<2>(t) << "\n";
    }

    outfile.close();
    return 0; // done
}
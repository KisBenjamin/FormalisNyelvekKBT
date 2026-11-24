#include "rgx.h"
#include <iostream>
#include <fstream>
#include <regex>
#include <vector>
#include <unordered_set>
#include <unordered_map>

void RegexProblem::initialize_parser(cxxopts::Options& options) {
    // hozzaadjuk a --rgx flaget ami egy intet var (1-5)
    options.add_options()
        ("r,rgx", "Regex task number", cxxopts::value<int>());
}

bool RegexProblem::is_chosen_problem(const cxxopts::ParseResult& args) {
    return args.count("rgx") > 0;
}

int RegexProblem::run(const cxxopts::ParseResult& args) {
    if (args.count("input") == 0 || args.count("output") == 0) {
        cerr << "Hiba: Hianyzo input vagy output fajl!\n";
        return 1;
    }

    string input_file = args["input"].as<string>();
    string output_file = args["output"].as<string>();
    int task = args["rgx"].as<int>();

    ifstream infile(input_file);
    if (!infile.is_open()) {
        cerr << "Hiba: Nem lehet megnyitni a bemeneti fajlt: " << input_file << "\n";
        return 1;
    }

    ofstream outfile(output_file);
    if (!outfile.is_open()) {
        cerr << "Hiba: Nem tudom letrehozni az output fajlt: " << output_file << "\n";
        return 1;
    }

    string line;

    if (task == 1) {
        // 10 darab counter az 1. feladat alpontjaihoz
        int c[10] = {0};

        // regexek leforditasa a loop elott (kulonben nagyon lassu)
        regex r0(R"(^10-15\s+)"); // 10. ho 15
        regex r1(R"(^10-15\s+10:18:51\b)"); // 10-15 10:18:51
        regex r2(R"(^\d{2}-\d{2}\s+\d{2}:\d{2}:51\b)"); // barmelyik perc 51. masodperce
        regex r3(R"(^\d{2}-\d{2}\s+\d{2}:\d{2}:\d{2}\.\d+\s+\d+\s+\d+\s+[IDVEFAW]\s+lowmemorykiller\s*:)"); // lowmemorykiller tag
        regex r4(R"(^\d{2}-\d{2}\s+10:18:51\.\d+\s+221\s+)"); // 10:18:51 es 221-es PID
        regex r5(R"(^\d{2}-\d{2}\s+\d{2}:\d{2}:\d{2}\.\d+\s+\d+\s+\d+\s+E\s+)"); // osszes error (E)
        regex r6(R"(^\d{2}-\d{2}\s+\d{2}:\d{2}:\d{2}\.\d+\s+\d+\s+\d+\s+W\s+PackageManager\s*:)"); // PackageManager warning
        regex r7(R"(^\d{2}-\d{2}\s+\d{2}:\d{2}:\d{2}\.\d+\s+\d+\s+\d+\s+D\s+.*ExoPlayer)"); // ExoPlayer debug
        regex r8(R"(^\d{2}-\d{2}\s+\d{2}:\d{2}:\d{2}\.\d+\s+\d+\s+\d+\s+[EW]\s+.*\.java:\d+)"); // java fajl pozicio E/W sorban
        regex r9(R"(^\d{2}-\d{2}\s+\d{2}:\d{2}:\d{2}\.\d+\s+\d+\s+\d+\s+[IDVEFAW]\s+[^:]+:.*Thread)"); // Thread szo a message reszben

        while (getline(infile, line)) {
            if (regex_search(line, r0)) c[0]++;
            if (regex_search(line, r1)) c[1]++;
            if (regex_search(line, r2)) c[2]++;
            if (regex_search(line, r3)) c[3]++;
            if (regex_search(line, r4)) c[4]++;
            if (regex_search(line, r5)) c[5]++;
            if (regex_search(line, r6)) c[6]++;
            if (regex_search(line, r7)) c[7]++;
            if (regex_search(line, r8)) c[8]++;
            if (regex_search(line, r9)) c[9]++;
        }

        for (int i = 0; i < 10; ++i) {
            outfile << c[i] << "\n";
        }
    } 
    else if (task == 2) {
        // E sorokbol kiszedjuk a *.java fajlnevet (stack trace)
        regex r_java(R"(^\d{2}-\d{2}\s+\d{2}:\d{2}:\d{2}\.\d+\s+\d+\s+\d+\s+E\s+.*\b([A-Za-z0-9_$]+\.java):\d+)");
        smatch m;

        while (getline(infile, line)) {
            if (regex_search(line, m, r_java)) {
                outfile << m[1].str() << "\n";
            }
        }
    } 
    else if (task == 3) {
        // kulonbozo PID-ek elofordulasi sorrendben
        regex r_pid(R"(^\d{2}-\d{2}\s+\d{2}:\d{2}:\d{2}\.\d+\s+(\d+)\s+\d+\s+[IDVEFAW]\s+)");
        smatch m;
        unordered_set<string> seen;

        while (getline(infile, line)) {
            if (regex_search(line, m, r_pid)) {
                string pid = m[1].str();
                if (seen.find(pid) == seen.end()) {
                    seen.insert(pid);
                    outfile << pid << "\n";
                }
            }
        }
    } 
    else if (task == 4) {
        // legtobb errort tartalmazo process
        regex r_err_pid(R"(^\d{2}-\d{2}\s+\d{2}:\d{2}:\d{2}\.\d+\s+(\d+)\s+\d+\s+E\s+)");
        smatch m;
        unordered_map<string, int> err_counts;
        vector<string> pid_order;

        while (getline(infile, line)) {
            if (regex_search(line, m, r_err_pid)) {
                string pid = m[1].str();
                if (err_counts.find(pid) == err_counts.end()) {
                    pid_order.push_back(pid);
                }
                err_counts[pid]++;
            }
        }

        string max_pid = "";
        int max_err = -1;
        for (const string& pid : pid_order) {
            if (err_counts[pid] > max_err) {
                max_err = err_counts[pid];
                max_pid = pid;
            }
        }

        if (!max_pid.empty()) {
            outfile << "PID=" << max_pid << " (" << max_err << " errors)\n";
        }
    } 
    else if (task == 5) {
        // fo szalon tortent hibak (PID == TID), backreference-el (\1)
        regex r_main_err(R"(^\d{2}-\d{2}\s+\d{2}:\d{2}:\d{2}\.\d+\s+(\d+)\s+\1\s+E\s+)");
        int count = 0;

        while (getline(infile, line)) {
            if (regex_search(line, r_main_err)) {
                count++;
            }
        }

        outfile << count << "\n";
    } 
    else {
        cerr << "Hiba: Ismeretlen --rgx feladat szam: " << task << "\n";
        return 1;
    }

    infile.close();
    outfile.close();
    return 0; // done
}
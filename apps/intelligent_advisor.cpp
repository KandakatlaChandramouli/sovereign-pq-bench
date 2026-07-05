#include "sovereign/crypto/signature_scheme.hpp"
#include "sovereign/crypto/ecdsa_engine.hpp"
#include "sovereign/crypto/ed25519_engine.hpp"
#include "sovereign/crypto/ml_dsa_engine.hpp"
#include "sovereign/crypto/pq_engine.hpp"
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <map>
#include <cmath>
#include <chrono>
#include <fstream>
#include <sstream>

struct CaseStudy { int id; std::string domain,name,entity,tech,risk,rec; };

static std::vector<CaseStudy> load_cases() {
    std::vector<CaseStudy> cases;
    std::ifstream f("../profiles/gulf_100_cases.csv");
    std::string line;
    std::getline(f, line); // skip header
    while (std::getline(f, line)) {
        std::stringstream ss(line);
        std::string id,domain,name,entity,tech,risk,rec;
        std::getline(ss,id,','); std::getline(ss,domain,','); std::getline(ss,name,',');
        std::getline(ss,entity,','); std::getline(ss,tech,','); std::getline(ss,risk,',');
        std::getline(ss,rec,',');
        cases.push_back({std::stoi(id),domain,name,entity,tech,risk,rec});
    }
    return cases;
}

void run_bench(const std::string& algo) {
    auto bench = [](auto& engine) {
        auto kp = engine.generate_keypair();
        std::vector<std::byte> msg(256);
        double sn=0, vn=0;
        for (int i=0; i<20; i++) {
            auto t1=std::chrono::high_resolution_clock::now();
            auto sig=engine.sign(msg, kp.first->private_key);
            auto t2=std::chrono::high_resolution_clock::now();
            (void)engine.verify(msg, sig.first->data, kp.first->public_key);
            auto t3=std::chrono::high_resolution_clock::now();
            sn+=std::chrono::duration<double,std::nano>(t2-t1).count();
            vn+=std::chrono::duration<double,std::nano>(t3-t2).count();
        }
        std::cout << "  Sign: " << std::fixed << std::setprecision(1) << sn/20/1000 << "us | Verify: " << vn/20/1000 << "us | Key: " << kp.first->public_key.size() << "B\n";
    };
    if (algo.find("ML-DSA-44")!=std::string::npos) { sovereign::MlDsa44Engine e; bench(e); }
    else if (algo.find("Falcon-512")!=std::string::npos) { sovereign::Falcon512Engine e; bench(e); }
    else if (algo.find("SPHINCS")!=std::string::npos) { sovereign::Sphincs128sEngine e; bench(e); }
    else if (algo.find("ML-DSA-65")!=std::string::npos) { sovereign::MlDsa65Engine e; bench(e); }
    else if (algo.find("ML-KEM")!=std::string::npos) { std::cout << "  (KEM benchmark: use ./gulf_bench)\n"; }
    else { std::cout << "  (Use ./gulf_bench for full metrics)\n"; }
}

int main() {
    auto cases = load_cases();
    std::cout << "Loaded " << cases.size() << " Gulf case studies.\n\n";
    
    // Show domains
    std::map<std::string,int> domains;
    for (auto& c : cases) { if (domains.find(c.domain)==domains.end()) domains[c.domain]=domains.size()+1; }
    std::cout << "Select Domain:\n";
    for (auto& [name,id] : domains) std::cout << "  " << id << ". " << name << "\n";
    std::cout << "  Enter (1-" << domains.size() << "): ";
    int dchoice; std::cin >> dchoice;
    std::string sel_domain;
    for (auto& [n,i] : domains) if (i==dchoice) { sel_domain=n; break; }
    if (sel_domain.empty()) { std::cout << "Invalid\n"; return 1; }
    
    // Show cases
    std::vector<int> idxs;
    std::cout << "\nCases in " << sel_domain << ":\n";
    for (size_t i=0; i<cases.size(); i++) {
        if (cases[i].domain==sel_domain) {
            idxs.push_back(i);
            std::cout << "  " << idxs.size() << ". " << cases[i].name << " (" << cases[i].entity << ")\n";
        }
    }
    std::cout << "  Select (1-" << idxs.size() << "): ";
    int cchoice; std::cin >> cchoice;
    if (cchoice<1||cchoice>(int)idxs.size()) { std::cout << "Invalid\n"; return 1; }
    auto& c = cases[idxs[cchoice-1]];
    
    std::cout << "\n=== " << c.name << " ===\n";
    std::cout << "Entity: " << c.entity << " | Risk: " << c.risk << "\n";
    std::cout << "Tech: " << c.tech << "\n";
    std::cout << "RECOMMENDATION: " << c.rec << "\n";
    run_bench(c.rec);
    return 0;
}
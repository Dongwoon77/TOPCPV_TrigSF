#include <TFile.h>
#include <TTree.h>
#include <TChain.h>
#include <TH2D.h>
#include <TString.h>
#include <TSystem.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <cmath>
#include <filesystem>
#include <chrono>
#include <iomanip>

std::vector<double> ptBins = {20, 30, 50, 70, 100, 140, 200, 300, 600, 1000};
std::vector<double> etaBins;

// Function to display progress with ETA
void displayProgress(Long64_t current, Long64_t total, 
                    const std::chrono::steady_clock::time_point& start_time) {
    if (total == 0) return;
    
    std::cout << "\r[Progress] Event " << current << " / " << total;
    
    // ETA calculation
    if (current > 0) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start_time);
        double avg_time_per_event = elapsed.count() / static_cast<double>(current);
        double eta_seconds = avg_time_per_event * (total - current);
        
        int eta_hours = static_cast<int>(eta_seconds / 3600);
        int eta_minutes = static_cast<int>((eta_seconds - eta_hours * 3600) / 60);
        int eta_secs = static_cast<int>(eta_seconds - eta_hours * 3600 - eta_minutes * 60);
        
        std::cout << " ETA: ";
        if (eta_hours > 0) {
            std::cout << eta_hours << "h " << eta_minutes << "m " << eta_secs << "s";
        } else if (eta_minutes > 0) {
            std::cout << eta_minutes << "m " << eta_secs << "s";
        } else {
            std::cout << eta_secs << "s";
        }
    }
    
    std::cout.flush();
}

void initializeEtaBins() {
    for (double eta = 0.0; eta <= 2.41; eta += 0.2) {
        etaBins.push_back(eta);
    }
}

struct WPSet {
    float Loose;
    float Medium;
    float Tight;
};

std::map<TString, std::map<TString, WPSet>> WP_year_taggers = {
    {"2016preVFP", {
        {"DeepCSV", {0.2027, 0.6001, 0.8819}},
        {"DeepJet", {0.0508, 0.2598, 0.6502}}
    }},
    {"2016postVFP", {
        {"DeepCSV", {0.1918, 0.5847, 0.8767}},
        {"DeepJet", {0.0480, 0.2489, 0.6377}}
    }},
    {"2017", {
        {"DeepCSV", {0.1355, 0.4506, 0.7738}},
        {"DeepJet", {0.0532, 0.3040, 0.7476}}
    }},
    {"2018", {
        {"DeepCSV", {0.1208, 0.4168, 0.7665}},
        {"DeepJet", {0.0490, 0.2783, 0.7100}}
    }},
    {"2022", {
        {"UParTAK4", {0.0246, 0.1272, 0.4648}}
    }},
    {"2022PostEE", {
        {"UParTAK4", {0.0246, 0.1272, 0.4648}}
    }},
    {"2023", {
        {"UParTAK4", {0.0246, 0.1272, 0.4648}}
    }},
    {"2023BPix", {
        {"UParTAK4", {0.0246, 0.1272, 0.4648}}
    }},
    {"2024", {
        {"UParTAK4", {0.0246, 0.1272, 0.4648}}
    }}
};

std::string extractYearFromPath(const std::string& path) {
    if (path.find("UL2016PreVFP") != std::string::npos) return "2016preVFP";
    if (path.find("UL2016PostVFP") != std::string::npos) return "2016postVFP";
    if (path.find("UL2017") != std::string::npos) return "2017";
    if (path.find("UL2018") != std::string::npos) return "2018";
    // Run3: check longer strings first to avoid partial match
    if (path.find("2022PostEE") != std::string::npos) return "2022PostEE";
    if (path.find("2023BPix") != std::string::npos) return "2023BPix";
    if (path.find("2022") != std::string::npos) return "2022";
    if (path.find("2023") != std::string::npos) return "2023";
    if (path.find("2024") != std::string::npos) return "2024";
    return "2016preVFP";
}

std::string extractNumberFromOutputPrefix(const std::string& outputPrefix) {
    // outputPrefix에서 마지막 언더스코어 이후의 숫자 추출
    size_t pos = outputPrefix.find_last_of('_');
    if (pos != std::string::npos) {
        return outputPrefix.substr(pos + 1);
    }
    return "0"; // 기본값
}

void processFiles(const std::vector<std::string>& fileList, const std::string& outputPrefix, const std::string& year, Long64_t maxEvents = -1) {
    std::cout << "[INFO] Using b-tagging working points for year: " << year << std::endl;
    std::map<TString, WPSet> WP = WP_year_taggers[year];
    for (auto it = WP.begin(); it != WP.end(); ++it) {
        std::cout << "[INFO]  Tagger: " << it->first
                  << " | Loose: " << it->second.Loose
                  << ", Medium: " << it->second.Medium
                  << ", Tight: " << it->second.Tight << std::endl;
    }

    TChain chain("Events");
    for (const auto& file : fileList) {
        std::cout << "[INFO] Adding file to TChain: " << file << std::endl;
        chain.Add(file.c_str());
    }
    std::cout << "[INFO] Total entries in TChain: " << chain.GetEntries() << std::endl;

    Float_t Jet_pt[100], Jet_eta[100], Jet_btagDeepB[100], Jet_btagDeepFlavB[100], Jet_btagUParTAK4B[100];
    UChar_t Jet_hadronFlavour[100];
    Int_t nJet;
    chain.SetBranchAddress("Jet_pt", Jet_pt);
    chain.SetBranchAddress("Jet_eta", Jet_eta);
    chain.SetBranchAddress("Jet_hadronFlavour", Jet_hadronFlavour);
    chain.SetBranchAddress("nJet", &nJet);
    chain.SetBranchAddress("Jet_btagDeepFlavB", Jet_btagDeepFlavB);
    chain.SetBranchAddress("Jet_btagUParTAK4B", Jet_btagUParTAK4B);
    if (chain.FindBranch("Jet_btagDeepB")) {
        chain.SetBranchAddress("Jet_btagDeepB", Jet_btagDeepB);  // Run2 DeepCSV
    }

    initializeEtaBins();

    std::map<TString, std::map<TString, TH2D*>> h_num_b, h_den_b, h_num_c, h_den_c, h_num_l, h_den_l;
    for (auto it = WP.begin(); it != WP.end(); ++it) {
        TString algo = it->first;
        for (TString wp : {"Loose", "Medium", "Tight"}) {
            h_num_b[algo][wp] = new TH2D("h_num_b_" + algo + "_" + wp, "", ptBins.size()-1, &ptBins[0], etaBins.size()-1, &etaBins[0]);
            h_den_b[algo][wp] = new TH2D("h_den_b_" + algo + "_" + wp, "", ptBins.size()-1, &ptBins[0], etaBins.size()-1, &etaBins[0]);
            h_num_c[algo][wp] = new TH2D("h_num_c_" + algo + "_" + wp, "", ptBins.size()-1, &ptBins[0], etaBins.size()-1, &etaBins[0]);
            h_den_c[algo][wp] = new TH2D("h_den_c_" + algo + "_" + wp, "", ptBins.size()-1, &ptBins[0], etaBins.size()-1, &etaBins[0]);
            h_num_l[algo][wp] = new TH2D("h_num_l_" + algo + "_" + wp, "", ptBins.size()-1, &ptBins[0], etaBins.size()-1, &etaBins[0]);
            h_den_l[algo][wp] = new TH2D("h_den_l_" + algo + "_" + wp, "", ptBins.size()-1, &ptBins[0], etaBins.size()-1, &etaBins[0]);
        }
    }

    Long64_t nentries = chain.GetEntries();
    if(maxEvents > 0 && maxEvents < nentries) nentries = maxEvents;

    auto start_time = std::chrono::steady_clock::now();

    for (Long64_t i = 0; i < nentries; ++i) {
        if (i % 10000 == 0)
            displayProgress(i, nentries, start_time);

        chain.GetEntry(i);

        for (Int_t j = 0; j < nJet; ++j) {
            float pt = Jet_pt[j];
            float eta = Jet_eta[j];
            int flav = Jet_hadronFlavour[j];
            if (pt < 20 || fabs(eta) > 2.4) continue;

            for (auto it = WP.begin(); it != WP.end(); ++it) {
                TString algo = it->first;
                WPSet cuts = it->second;
                //float score = (algo == "UParTAK4") ? Jet_btagUParTAK4B[j] : (algo == "DeepJet") ? Jet_btagDeepFlavB[j] : Jet_btagDeepB[j];
                float score = 0.0f;
                if (algo == "DeepJet") {
                    score = Jet_btagDeepFlavB[j];
                } else if (algo == "UParTAK4") {
                    score = Jet_btagUParTAK4B[j];
                } else if (algo == "DeepCSV") {
                    score = Jet_btagDeepB[j];
                } else {
                    continue; // 알 수 없는 tagger는 건너뜀
                }
                std::map<TString, float> thresholds = {
                    {"Loose", cuts.Loose},
                    {"Medium", cuts.Medium},
                    {"Tight", cuts.Tight}
                };
                float etaAbs = fabs(eta);
                for (auto wp_it = thresholds.begin(); wp_it != thresholds.end(); ++wp_it) {
                    TString wp = wp_it->first;
                    float cut = wp_it->second;
                    if (flav == 5) {
                        h_den_b[algo][wp]->Fill(pt, etaAbs);
                        if (score > cut) h_num_b[algo][wp]->Fill(pt, etaAbs);
                    } else if (flav == 4) {
                        h_den_c[algo][wp]->Fill(pt, etaAbs);
                        if (score > cut) h_num_c[algo][wp]->Fill(pt, etaAbs);
                    } else {
                        h_den_l[algo][wp]->Fill(pt, etaAbs);
                        if (score > cut) h_num_l[algo][wp]->Fill(pt, etaAbs);
                    }
                }
            }
        }
    }
    displayProgress(nentries, nentries, start_time);
    std::cout << std::endl;

    // 출력 디렉토리 추출 및 생성
    std::filesystem::path outputPath(outputPrefix);
    std::string outputDir = outputPath.parent_path().string();
    if (!outputDir.empty()) {
        std::filesystem::create_directories(outputDir);
    }

    for (auto it = WP.begin(); it != WP.end(); ++it) {
        TString algo = it->first;
        std::string number = extractNumberFromOutputPrefix(outputPrefix);
        
        // outputPrefix의 디렉토리 부분 추출
        std::filesystem::path outputPath(outputPrefix);
        std::string outputDir = outputPath.parent_path().string();
        
        TString filename = outputDir + "/btagEff_" + algo + "_" + number + ".root";
        TFile fout(filename.Data(), "RECREATE");
        for (TString wp : {"Loose", "Medium", "Tight"}) {
            auto write = [&](TH2D* num, TH2D* den, const TString& flav) {
                TH2D* eff = (TH2D*)num->Clone("eff_" + algo + "_" + flav + "_" + wp);
                eff->Divide(den);
                eff->Write(); num->Write(); den->Write();
            };
            write(h_num_b[algo][wp], h_den_b[algo][wp], "b");
            write(h_num_c[algo][wp], h_den_c[algo][wp], "c");
            write(h_num_l[algo][wp], h_den_l[algo][wp], "l");
        }
        fout.Close();
    }
}

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " file_list.txt outputPrefix [maxEvents]" << std::endl;
        return 1;
    }

    std::ifstream infile(argv[1]);
    std::string line;
    std::vector<std::string> fileList;
    while (std::getline(infile, line)) {
        if (!line.empty()) fileList.push_back(line);
    }

    std::string year = extractYearFromPath(argv[1]);
    std::string outputPrefix = argv[2];

    Long64_t maxEvents = -1;
    if (argc > 3) maxEvents = std::stoll(argv[3]);

    processFiles(fileList, outputPrefix, year, maxEvents);
    return 0;
}

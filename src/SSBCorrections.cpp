#include "../interface/SSBCorrections.h"
#include "../TextReader/TextReader.hpp"
#include <fstream>
#include <cstdlib>
#include "correction.h"
#include "TRandom3.h"
#include <cmath>
#include <iostream>
#include <filesystem>
#include <vector>
#include "TLorentzVector.h"



SSBCorrections::SSBCorrections(TextReader* reader, const std::string inputfileName) {
    std::cout << "TextReader in SSBCorrections ! " << std::endl;
    std::cout << "Current directory: " << std::filesystem::current_path() << std::endl;
    reader->PrintoutVariables();


    std::string jsonDir;// = std::filesystem::current_path().string() + "/jsonpog-integration/POG/";
    const std::string cvmfsPath = "/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/";
    if (std::filesystem::exists(cvmfsPath)) {
        std::cout << "[INFO] Using CVMFS path for JSONs: " << cvmfsPath << std::endl;
        jsonDir = cvmfsPath;
    } else {
        std::string localPath = std::filesystem::current_path().string() + "/jsonpog-integration/POG/";
        std::cout << "[WARNING] CVMFS path not found. Falling back to local path: " << localPath << std::endl;
        jsonDir = localPath;
    }

//    std::string jsonDir = std::filesystem::current_path().string() + "/jsonpog-integration/POG/";

    std::string puw_path     = reader->GetText("PUWeightPath");
    std::string jec_path     = reader->GetText("JECPath");
    std::string jer_path     = reader->GetText("JERPath");
    std::string jmar_path     = reader->GetText("JMARPath");
    std::string jetid_path    = reader->Check("JetIDPath") ? reader->GetText("JetIDPath", false) : "";
    std::string jetid_tight_name = reader->Check("JetIDTightName") ? reader->GetText("JetIDTightName", false) : "AK4PUPPI_Tight";
    std::string jetid_tightlepveto_name = reader->Check("JetIDTightLepVetoName") ? reader->GetText("JetIDTightLepVetoName", false) : "AK4PUPPI_TightLeptonVeto";
    std::string jveto_path    = reader->GetText("JetVetoPath");
    //std::string jveto_key    = reader->GetText("JetVetoName");
    //std::string jveto_map_key = reader->GetText("JetVetoKey");
    std::string jveto_name    = reader->GetText("JetVetoName");
    std::string jveto_map_key = reader->GetText("JetVetoKey");
    std::string jveto_type     = reader->GetText("JetVetoType");
    std::string jer_sf_path  = reader->GetText("JERSFPath");
    std::string jec_name     = reader->GetText("JECName");
    std::string jer_name     = reader->GetText("JERName");
    std::string jer_res_name = reader->GetText("JERResName");
    std::string jet_btag_conf = reader->GetText("Jet_btag");  // e.g., "deepCSVL", "deepJetM"
    std::string muon_path    = reader->GetText("MuonSFPath");
    std::string elec_path    = reader->GetText("ElecSFPath");
    std::string RunPeriod    = reader->GetText("RunRange");
    std::string puJson  =  "";
    btag_sf_type_            = reader->GetText("BTagSFType");

    // Muon Infor. ID  ISO // 
    std::string muon_id_corName  = reader->GetText( "MuonIDSFName"  );
    std::string muon_iso_corName = reader->GetText( "MuonIsoSFName" );
    
    // Electron ID ISO // 
    std::string ele_sf_name_      = reader->GetText( "ElecIDSFName"  );

    // Trigger SF // 
    std::string Trig_sf_name_      = reader->GetText( "TrigSFFile"  );  
    std::string Trig_sf_histname_  = reader->GetText( "TrigSFHist"  );  



    //if inputfileName    

    if (RunPeriod.find("2016PreVFP") != std::string::npos) {
        year_ = "2016preVFP";
        puJson = "Collisions16_UltraLegacy_goldenJSON";
        
    } else if (RunPeriod.find("2016PostVFP") != std::string::npos) {
        year_ = "2016postVFP";
        puJson = "Collisions16_UltraLegacy_goldenJSON";
    } else if (RunPeriod.find("2017") != std::string::npos) {
        year_ = "2017";
        puJson = "Collisions17_UltraLegacy_goldenJSON";
    } else if (RunPeriod.find("2018") != std::string::npos) {
        year_ = "2018";
        puJson = "Collisions18_UltraLegacy_goldenJSON";
    } else if (RunPeriod.find("2022PostEE") != std::string::npos) {
        year_ = "2022PostEE";
        puJson = "Collisions2022_359022_362760_eraEFG_GoldenJson";
    } else if (RunPeriod.find("2022") != std::string::npos) {
        year_ = "2022";
        puJson = "Collisions2022_355100_357900_eraBCD_GoldenJson";
    } else if (RunPeriod.find("2023BPix") != std::string::npos) {
        year_ = "2023BPix";
        puJson = "Collisions2023_369803_370790_eraD_GoldenJson";
    } else if (RunPeriod.find("2023") != std::string::npos) {
        year_ = "2023";
        puJson = "Collisions2023_366403_369802_eraBC_GoldenJson";
    } else if (RunPeriod.find("2024") != std::string::npos) {
        year_ = "2024";
        puJson = "Collisions24_BCDEFGHI_goldenJSON";
    } else {
        std::cerr << "[SSBCorrections] Unknown RunPeriod: " << RunPeriod << std::endl;
        year_ = "2018"; // fallback or throw error
    }


    bool is_data = inputfileName.find("Data") != std::string::npos;

    std::string era = "Unknown";

    if (is_data) {
   	 // For data: extract year and era from the input file name, e.g. "Run2016F-HIPM"
    	size_t run_pos = inputfileName.find("Run");
    	if (run_pos != std::string::npos && run_pos + 7 <= inputfileName.size()) {
        	// Extract year: 4 digits following "Run"
        	std::string year_digits = inputfileName.substr(run_pos + 3, 4); // e.g., "2016", "2017"

        	// Extract era: characters immediately after "RunYYYY"
        	era = inputfileName.substr(run_pos + 7); // e.g., "B", "F-HIPM"
        	size_t delim = era.find_first_of("._/");
        	if (delim != std::string::npos) {
            		era = era.substr(0, delim); // Trim suffix after era
        	}
    	}
    } else {
        // For MC: use the RunPeriod directly (e.g., "2016PreVFP", "2016PostVFP", "2017", "2018")
       era = "MC";  // Dummy placeholder; era is not needed for MC in most JEC logic
    }

    std::cout << "era : " << era <<  std::endl;
    std::cout << "jec_name: " << jec_name << std::endl; 
    jec_name = ExpandJECName(jec_name, RunPeriod, era, is_data); 
    std::cout << "after ExpandJECName jec_name: " << jec_name << std::endl; 
    


    // For 2024, use puw_path directly; for other years, use jsonDir + puw_path
    pu_weight_ = nullptr;
    if (!puw_path.empty()) {
        std::string pu_full_path;
        if (RunPeriod.find("2024") != std::string::npos) {
            pu_full_path = puw_path;
            std::cout << "puw_path (2024): " << pu_full_path << std::endl;
        } else {
            pu_full_path = jsonDir + puw_path;
            std::cout << "jsonDir + puw_path: " << pu_full_path << std::endl;
        }
        
        if (std::filesystem::exists(pu_full_path)) {
            try {
                auto puset = correction::CorrectionSet::from_file(pu_full_path);
                pu_weight_ = puset->at(puJson);
            } catch (const std::exception& e) {
                std::cerr << "[WARNING] Failed to load PU weights ('" << puJson
                          << "') from " << pu_full_path << ": " << e.what() << std::endl;
                pu_weight_ = nullptr;
            }
        } else {
            std::cout << "[INFO] PU weights file not found at " << pu_full_path << " — skipping PU weights." << std::endl;
        }
    } else {
        std::cout << "[INFO] PUWeightPath empty — skipping PU weights." << std::endl;
    }
    
    /*pu_weight_ = nullptr;
    if (!puw_path.empty()) {
        std::string pu_full = jsonDir + puw_path;
        std::cout << "jsonDir + puw_path : " << pu_full << std::endl;
        if (std::filesystem::exists(pu_full)) {
            try {
                auto puset = correction::CorrectionSet::from_file(pu_full);
                pu_weight_ = puset->at(puJson);
            } catch (const std::exception& e) {
                std::cerr << "[WARNING] Failed to load PU weights ('" << puJson
                          << "') from " << pu_full << ": " << e.what() << std::endl;
                pu_weight_ = nullptr;
            }
        } else {
            std::cout << "[INFO] PU weights file not found at " << pu_full << " — skipping PU weights." << std::endl;
        }
    } else {
        std::cout << "[INFO] PUWeightPath empty — skipping PU weights." << std::endl;
    }*/


    try {
        auto jec_set = correction::CorrectionSet::from_file(jsonDir + jec_path);
        jec_ = jec_set->compound().at(jec_name);
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Failed to load JEC ('" << jec_name
                  << "') from " << jsonDir + jec_path << ": " << e.what() << std::endl;
        throw;
    }

    try {
        auto jer_set = correction::CorrectionSet::from_file(jsonDir + jer_path);
        jer_ = jer_set->at(jer_res_name);
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Failed to load JER resolution ('" << jer_res_name
                  << "') from " << jsonDir + jer_path << ": " << e.what() << std::endl;
        throw;
    }

    try {
        auto jer_sf_set = correction::CorrectionSet::from_file(jsonDir + jer_sf_path);
        jer_sf_ = jer_sf_set->at(jer_name);
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Failed to load JER scale factor ('" << jer_name
                  << "') from " << jsonDir + jer_sf_path << ": " << e.what() << std::endl;
        throw;
    }

    // Load JetID JSON corrections (NanoAODv13+ recommendation).
    // Keep this as "load-only" for now; selection logic can consume these objects later.
    jetid_tight_ = nullptr;
    jetid_tightlepveto_ = nullptr;
    try {
        std::string jetid_full_path = jetid_path;
        if (jetid_full_path.empty()) {
            if (year_ == "2022") jetid_full_path = "JME/2022_Summer22/jetid.json.gz";
            else if (year_ == "2022PostEE") jetid_full_path = "JME/2022_Summer22EE/jetid.json.gz";
            else if (year_ == "2023") jetid_full_path = "JME/2023_Summer23/jetid.json.gz";
            else if (year_ == "2023BPix") jetid_full_path = "JME/2023_Summer23BPix/jetid.json.gz";
            else if (year_ == "2024") jetid_full_path = "JME/2024_Summer24/jetid.json.gz";
        } else {
            // Config allows either absolute path or POG-relative path.
            if (!jetid_full_path.empty() && jetid_full_path.front() != '/') {
                jetid_full_path = jsonDir + jetid_full_path;
            }
        }

        if (std::filesystem::exists(jetid_full_path)) {
            auto jetid_set = correction::CorrectionSet::from_file(jetid_full_path);
            jetid_tight_ = jetid_set->at(jetid_tight_name);
            jetid_tightlepveto_ = jetid_set->at(jetid_tightlepveto_name);
            std::cout << "[INFO] Loaded JetID corrections from: " << jetid_full_path
                      << " (" << jetid_tight_name << ", " << jetid_tightlepveto_name << ")" << std::endl;
        } else {
            std::cout << "[INFO] JetID JSON not found at " << jetid_full_path
                      << " — skipping JetID correction loading." << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "[WARNING] Failed to load JetID JSON corrections: " << e.what() << std::endl;
        jetid_tight_ = nullptr;
        jetid_tightlepveto_ = nullptr;
    }

    //auto jmar_sf_set = correction::CorrectionSet::from_file(jsonDir + jmar_path);
    //pujetid_sf_ = jmar_sf_set->at("PUJetID_eff");

    if (!jmar_path.empty()) {
        try {
            auto jmar_sf_set = correction::CorrectionSet::from_file(jsonDir + jmar_path);
            pujetid_sf_ = jmar_sf_set->at("PUJetID_eff");
        } catch (const std::exception& e) {
            std::cerr << "[WARNING] Failed to load JMAR (PUJetID) from '" << (jsonDir + jmar_path)
                      << "': " << e.what() << std::endl;
            pujetid_sf_ = nullptr;
        }
    } else {
        std::cout << "[INFO] JMARPath empty — skipping PUJetID SF." << std::endl;
        pujetid_sf_ = nullptr;
    }

    jveto_name_ = jveto_name;
    jveto_key_ = jveto_map_key;
    jveto_type_ = jveto_type;	

    if (!jveto_path.empty() && !jveto_name.empty()) {
       try {
           auto jetveto_set = correction::CorrectionSet::from_file(jsonDir + jveto_path);
           jetvetomap_ = jetveto_set->at(jveto_name);  // jveto_name 사용
           std::cout << "[INFO] Loaded jet veto map: " << jveto_name
                     << " with key: " << jveto_map_key
                     << " type: " << jveto_type << std::endl;
       } catch (const std::exception& e) {
           std::cerr << "[WARNING] Failed to load jet veto map: " << e.what() << std::endl;
           jetvetomap_ = nullptr;
       }
    } else {
       std::cout << "[INFO] Jet veto map not configured, skipping..." << std::endl;
       jetvetomap_ = nullptr;
    }

    std::string btag_algo = "";
    std::string btag_wp = "";

    if (jet_btag_conf.find("deepCSV") != std::string::npos) {
        btag_algo = "DeepCSV";
    } else if (jet_btag_conf.find("deepJet") != std::string::npos) {
        btag_algo = "DeepJet";
    } else if (jet_btag_conf.find("pfCSVV2") != std::string::npos) {
        btag_algo = "CSVv2";
    } else if (jet_btag_conf.find("UParTAK4") != std::string::npos) {
        btag_algo = "UParTAK4";
    } else {
        std::cerr << "[WARNING] Unknown b-tag algorithm in Jet_btag: " << jet_btag_conf << std::endl;
    }

    char last = jet_btag_conf.back();
    if (last == 'L' || last == 'l') btag_wp = "Loose";
    else if (last == 'M' || last == 'm') btag_wp = "Medium";
    else if (last == 'T' || last == 't') btag_wp = "Tight";
    else {
        std::cerr << "[WARNING] Unknown WP in Jet_btag: " << jet_btag_conf << std::endl;
    }

    // Validate SF type after b-tag algorithm is known.
    // Run2/DeepCSV/DeepJet: comb or mujets
    // Run3/UParTAK4: kinfit, ptrel, negtagDY
    if (btag_algo == "UParTAK4") {
        if (btag_sf_type_ != "kinfit" &&
            btag_sf_type_ != "ptrel" &&
            btag_sf_type_ != "negtagDY") {
            std::cerr << "[WARNING] Invalid BTagSFType for UParTAK4: " << btag_sf_type_
                      << ". Using default 'kinfit'." << std::endl;
            btag_sf_type_ = "kinfit";
        }
        std::cout << "[INFO] Using '" << btag_sf_type_
                  << "' SF for UParTAK4." << std::endl;
    } else {
        if (btag_sf_type_ != "comb" && btag_sf_type_ != "mujets") {
            std::cerr << "[WARNING] Invalid BTagSFType: " << btag_sf_type_
                      << ". Using default 'comb'." << std::endl;
            btag_sf_type_ = "comb";
        }
        if (btag_sf_type_ == "comb") {
            std::cout << "[INFO] Using 'comb' SF (QCD + ttbar enriched regions)" << std::endl;
        } else {
            std::cout << "[INFO] Using 'mujets' SF (QCD enriched regions, bias avoidance)" << std::endl;
        }
    }

    // Determine the correct Summer version and filename based on year_
    // 2024 uses btagging_preliminary.json.gz, other years use btagging.json.gz
    std::string btag_year_prefix;
    std::string summer_version;
    std::string btag_filename;
    if (year_ == "2022" || year_ == "2022PostEE") {
        btag_year_prefix = "2022";
        summer_version = (year_ == "2022PostEE") ? "Summer22EE" : "Summer22";
        btag_filename = "btagging.json.gz";
    } else if (year_ == "2023" || year_ == "2023BPix") {
        btag_year_prefix = "2023";
        summer_version = (year_ == "2023BPix") ? "Summer23BPix" : "Summer23";
        btag_filename = "btagging.json.gz";
    } else if (year_ == "2024") {
        btag_year_prefix = "2024";
        summer_version = "Summer24";
        btag_filename = "btagging_preliminary.json.gz";  // 2024 uses preliminary version
    } else if (year_ == "2018") {
        btag_year_prefix = "2018";
        summer_version = "UL";
        btag_filename = "btagging.json.gz";
    } else {
        // Fallback for older years (2016/2017 UL paths should be added similarly if needed)
        btag_year_prefix = year_;
        summer_version = "Summer24";
        btag_filename = "btagging.json.gz";
    }
    std::string btag_sf_json = "BTV/" + btag_year_prefix + "_" + summer_version + "/" + btag_filename;
    std::string btag_tagger = (btag_algo == "UParTAK4")
                                  ? "UParTAK4_" + btag_sf_type_
                                  : (btag_algo == "DeepJet") ? "deepJet_comb" : "deepCSV_comb";
    // Run2: UL prefix | Run3: no UL prefix (2022, 2022PostEE, 2023, 2023BPix, 2024)
    // Run3 efficiencies are stored by sample under:
    //   CorrectionFiles/BTag/<year>/<sample>/btagEff_<algo>.root
    // with optional recreated file preferred when available.
    std::string btag_eff_path;
    if (RunPeriod.find("2022") != std::string::npos || RunPeriod.find("2023") != std::string::npos || RunPeriod.find("2024") != std::string::npos) {
        const std::string sample_name = inputfileName;
        const std::string base_dir = "CorrectionFiles/BTag/" + RunPeriod + "/" + sample_name;
        const std::string recreated = base_dir + "/btagEff_" + btag_algo + "_recreated.root";
        const std::string nominal   = base_dir + "/btagEff_" + btag_algo + ".root";
        const std::string legacy    = "CorrectionFiles/BTag/" + RunPeriod + "/btagEff_" + btag_algo + ".root";

        if (std::filesystem::exists(recreated)) {
            btag_eff_path = recreated;
        } else if (std::filesystem::exists(nominal)) {
            btag_eff_path = nominal;
        } else {
            // Backward-compatible fallback for old layout.
            btag_eff_path = legacy;
        }
    } else {
        btag_eff_path = "CorrectionFiles/BTag/UL" + RunPeriod + "/btagEff_" + btag_algo + ".root";
    }
    std::cout << "btag_eff_path : " << btag_eff_path << std::endl;

    InitBtagSFCorrection(jsonDir + btag_sf_json, btag_tagger);
    if (!is_data) {
        LoadMCBtagEfficiencies(btag_eff_path, btag_algo);
    }

    // Load muon SF
    //auto muon_set = CorrectionSet::from_file(jsonDir+muon_path);
    std::cout << "jsonDir+muon_path " << jsonDir+muon_path << std::endl; 
    auto muon_set = correction::CorrectionSet::from_file(jsonDir + muon_path);

    std::cout << "muon_id_corName : " << muon_id_corName << std::endl;
    std::cout << "muon_iso_corName : " << muon_iso_corName << std::endl;

    muon_id_   = muon_set->at(muon_id_corName);
    muon_iso_  = muon_set->at(muon_iso_corName);

    auto cset = correction::CorrectionSet::from_file(jsonDir + elec_path);
    ele_sf_ = cset->at(ele_sf_name_);

    ele_id_sf_year_.clear();
    if (reader->Check("ElecIDSFYear")) {
        ele_id_sf_year_ = reader->GetText("ElecIDSFYear", false);
        std::cout << "[INFO] Electron ID/Reco SF year key (ElecIDSFYear): " << ele_id_sf_year_ << std::endl;
    }

    // Optional: Reco SF in separate file (e.g. 2024 electron_v1.json with Reco20to75/RecoAbove75)
    ele_reco_sf_ = nullptr;
    ele_reco_sf_year_.clear();
    if (reader->Check("ElecRecoSFPath")) {
        std::string ele_reco_path = reader->GetText("ElecRecoSFPath");
        std::string ele_reco_name = reader->Check("ElecRecoSFName") ? reader->GetText("ElecRecoSFName", false) : "Electron-ID-SF";
        ele_reco_sf_year_ = reader->Check("ElecRecoSFYear") ? reader->GetText("ElecRecoSFYear", false) : "";
        std::string ele_reco_full = (ele_reco_path.front() == '/') ? ele_reco_path : (jsonDir + ele_reco_path);
        if (std::filesystem::exists(ele_reco_full)) {
            try {
                auto reco_cset = correction::CorrectionSet::from_file(ele_reco_full);
                ele_reco_sf_ = reco_cset->at(ele_reco_name);
                std::cout << "[INFO] Loaded electron Reco SF from " << ele_reco_path
                          << " (year=" << ele_reco_sf_year_ << ")" << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "[WARNING] Failed to load electron Reco SF: " << e.what() << std::endl;
            }
        } else {
            std::cerr << "[WARNING] ElecRecoSFPath not found: " << ele_reco_full << std::endl;
        }
    }
    std::string TrigSFPath = std::filesystem::current_path().string() + "/CorrectionFiles/Trig/";
    TFile *f_trg     = new TFile((TrigSFPath+Trig_sf_name_).c_str());
    if (!f_trg || f_trg->IsZombie()) {
        std::cerr << "[SSBCorrections] ERROR: Cannot open Trigger SF file: " << (TrigSFPath+Trig_sf_name_) << std::endl;
        H_trig = nullptr;
    } else {
        H_trig = (TH2D*) f_trg->Get(Trig_sf_histname_.c_str());
        if (!H_trig) {
            std::cerr << "[SSBCorrections] ERROR: Histogram '" << Trig_sf_histname_ << "' not found in " << Trig_sf_name_ << std::endl;
        }
    }

    /// Muon Rochester correction ///
    std::string rochesterCorrFile;
    std::cout << "RunPeriod : " << RunPeriod << std::endl;
    if (RunPeriod.find("2016Pre")  != std::string::npos) {    rochesterCorrFile =  "./CorrectionFiles/Rochester/RoccoR2016aUL.txt";}
    else if (RunPeriod.find("2016Post") != std::string::npos) { rochesterCorrFile =  "./CorrectionFiles/Rochester/RoccoR2016bUL.txt";}
    else if (RunPeriod.find("2017")     != std::string::npos) { rochesterCorrFile =  "./CorrectionFiles/Rochester/RoccoR2017UL.txt";}
    else if (RunPeriod.find("2018")     != std::string::npos) { rochesterCorrFile =  "./CorrectionFiles/Rochester/RoccoR2018UL.txt";}
    else if (RunPeriod.find("2022PostEE")     != std::string::npos) { rochesterCorrFile =  "./CorrectionFiles/Rochester/RoccoR2018UL.txt";}
    else if (RunPeriod.find("2022")     != std::string::npos) { rochesterCorrFile =  "./CorrectionFiles/Rochester/RoccoR2018UL.txt";}
    else if (RunPeriod.find("2023BPix")     != std::string::npos) { rochesterCorrFile =  "./CorrectionFiles/Rochester/RoccoR2018UL.txt";}
    else if (RunPeriod.find("2023")     != std::string::npos) { rochesterCorrFile =  "./CorrectionFiles/Rochester/RoccoR2018UL.txt";}
    else if (RunPeriod.find("2024")     != std::string::npos) { rochesterCorrFile =  "./CorrectionFiles/Rochester/RoccoR2018UL.txt";}
    else {
        std::cerr << "[SSBCorrections] Unknown RunPeriod in rochesterCorrFile : " << RunPeriod << std::endl;
    }

    std::cout << "[Debug!!] in SSBCorrections !! rochesterCorrFile  : " << rochesterCorrFile<< std::endl;
    std::ifstream file(rochesterCorrFile);
    if(!file.good()){
            std::cerr << "ERROR:  Rochester file is not found:" << rochesterCorrFile << std::endl;
            std::exit(1);
    }

    rc.init(rochesterCorrFile);
}

// Destructor ///
//
SSBCorrections::~SSBCorrections() {
    std::cout << "[SSBCorrections] Destructor called - cleaning up memory..." << std::endl;
    
    // 1. Safely delete H_trig histogram
    if (H_trig) {
        std::cout << "[SSBCorrections] Deleting H_trig histogram..." << std::endl;
        delete H_trig;
        H_trig = nullptr;
    }
    
    // 2. Safely delete all TH2D pointers in eff_histograms_ map
    std::cout << "[SSBCorrections] Cleaning up " << eff_histograms_.size() 
              << " efficiency histograms..." << std::endl;
              
    for (auto& pair : eff_histograms_) {
        if (pair.second) {
            std::cout << "[SSBCorrections] Deleting histogram: " << pair.first << std::endl;
            delete pair.second;
            pair.second = nullptr;
        }
    }
    
    // Clear the map itself
    eff_histograms_.clear();
    
    std::cout << "[SSBCorrections] Destructor completed successfully." << std::endl;
}
/*
SSBCorrections::~SSBCorrections() {
    std::cout << "[SSBCorrections] Destructor called - cleaning up memory..." << std::endl;

    if (H_trig) {
        // Check if ROOT object is still valid
        if (gROOT && gROOT->FindObject(H_trig)) {
            std::cout << "[SSBCorrections] Deleting H_trig histogram..." << std::endl;
            delete H_trig;
        }
        H_trig = nullptr;
    }

    std::cout << "[SSBCorrections] Cleaning up " << eff_histograms_.size()
              << " efficiency histograms..." << std::endl;

    for (auto& pair : eff_histograms_) {
        if (pair.second) {
            // Check if ROOT object is still valid
            if (gROOT && gROOT->FindObject(pair.second)) {
                std::cout << "[SSBCorrections] Deleting histogram: " << pair.first << std::endl;
                delete pair.second;
            }
            pair.second = nullptr;
        }
    }

    // Clear the map itself
    eff_histograms_.clear();

    std::cout << "[SSBCorrections] Destructor completed successfully." << std::endl;
}
*/

double SSBCorrections::GetCorrectedJetPt(double raw_pt, double eta, double area, double rho, double phi, unsigned int runnb, bool isData) const {
    //std::cout << "in GetCorrectedJetPt raw_pt : " << raw_pt
    //          << " eta " << eta << " area : " << area << " rho : " << rho << std::endl;

    double sf;
    try {
        // JEC compound correction schema varies by year:
        // 2022, 2022PostEE: MC {area, eta, pt, rho}, DATA {area, eta, pt, rho}
        // 2023: MC {area, eta, pt, rho}, DATA {area, eta, pt, rho, run}
        // 2023BPix, 2024: MC {area, eta, pt, rho, phi}, DATA {area, eta, pt, rho, phi, run}
        
        bool needsPhi = (year_.find("2023BPix") != std::string::npos || 
                         year_.find("2024") != std::string::npos);
        bool needsRunForData = (year_.find("2023") != std::string::npos || 
                                year_.find("2024") != std::string::npos);
        
        if (isData) {
            if (needsPhi) {
                // 2023BPix, 2024 DATA: {area, eta, pt, rho, phi, run}
                sf = jec_->evaluate({area, eta, raw_pt, rho, phi, static_cast<double>(runnb)});
            } else if (needsRunForData && year_.find("2022") == std::string::npos) {
                // 2023 DATA: {area, eta, pt, rho, run}
                sf = jec_->evaluate({area, eta, raw_pt, rho, static_cast<double>(runnb)});
            } else {
                // 2022, 2022PostEE DATA: {area, eta, pt, rho}
                sf = jec_->evaluate({area, eta, raw_pt, rho});
            }
        } else {
            if (needsPhi) {
                // 2023BPix, 2024 MC: {area, eta, pt, rho, phi}
                sf = jec_->evaluate({area, eta, raw_pt, rho, phi});
            } else {
                // 2022, 2022PostEE, 2023 MC: {area, eta, pt, rho}
                sf = jec_->evaluate({area, eta, raw_pt, rho});
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] GetCorrectedJetPt JEC failed: " << e.what()
                  << " (year=" << year_ << ", area=" << area << ", eta=" << eta 
                  << ", pt=" << raw_pt << ", rho=" << rho << ", phi=" << phi 
                  << ", run=" << runnb << ", isData=" << isData << ")" << std::endl;
        throw;
    }
    //std::cout << "sf in GetCorrectedJetPt : " << sf << std::endl;

    return raw_pt * sf;
}

double SSBCorrections::GetCorrectedJetMass(double raw_mass, double raw_pt, double eta, double area, double rho, double phi, unsigned int runnb, bool isData) const {
    double sf;
    try {
        // JEC compound correction schema varies by year:
        // 2022, 2022PostEE: MC {area, eta, pt, rho}, DATA {area, eta, pt, rho}
        // 2023: MC {area, eta, pt, rho}, DATA {area, eta, pt, rho, run}
        // 2023BPix, 2024: MC {area, eta, pt, rho, phi}, DATA {area, eta, pt, rho, phi, run}
        
        bool needsPhi = (year_.find("2023BPix") != std::string::npos || 
                         year_.find("2024") != std::string::npos);
        bool needsRunForData = (year_.find("2023") != std::string::npos || 
                                year_.find("2024") != std::string::npos);
        
        if (isData) {
            if (needsPhi) {
                // 2023BPix, 2024 DATA: {area, eta, pt, rho, phi, run}
                sf = jec_->evaluate({area, eta, raw_pt, rho, phi, static_cast<double>(runnb)});
            } else if (needsRunForData && year_.find("2022") == std::string::npos) {
                // 2023 DATA: {area, eta, pt, rho, run}
                sf = jec_->evaluate({area, eta, raw_pt, rho, static_cast<double>(runnb)});
            } else {
                // 2022, 2022PostEE DATA: {area, eta, pt, rho}
                sf = jec_->evaluate({area, eta, raw_pt, rho});
            }
        } else {
            if (needsPhi) {
                // 2023BPix, 2024 MC: {area, eta, pt, rho, phi}
                sf = jec_->evaluate({area, eta, raw_pt, rho, phi});
            } else {
                // 2022, 2022PostEE, 2023 MC: {area, eta, pt, rho}
                sf = jec_->evaluate({area, eta, raw_pt, rho});
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] GetCorrectedJetMass JEC failed: " << e.what()
                  << " (year=" << year_ << ", area=" << area << ", eta=" << eta 
                  << ", pt=" << raw_pt << ", rho=" << rho << ", phi=" << phi 
                  << ", run=" << runnb << ", isData=" << isData << ")" << std::endl;
        throw;
    }
    return raw_mass * sf;
}

double SSBCorrections::GetJER(double eta, double pt, double rho) const {
    try {
        // JER Resolution schema: {JetEta, JetPt, Rho}
        return jer_->evaluate({eta, pt, rho});
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] GetJER failed: " << e.what()
                  << " (eta=" << eta << ", pt=" << pt << ", rho=" << rho << ")" << std::endl;
        throw;
    }
}

double SSBCorrections::SmearJER(double reco_pt, double gen_pt, double eta, double rho, const std::string& jer_tag) const {
    double sf, resolution;
    try {
        sf = jer_sf_->evaluate({eta, reco_pt, jer_tag});
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] SmearJER JER SF failed: " << e.what()
                  << " (eta=" << eta << ", pt=" << reco_pt << ", tag=" << jer_tag << ")" << std::endl;
        throw;
    }
    try {
        resolution = jer_->evaluate({eta, reco_pt, rho});
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] SmearJER JER resolution failed: " << e.what()
                  << " (eta=" << eta << ", pt=" << reco_pt << ", rho=" << rho << ")" << std::endl;
        throw;
    }

    if (gen_pt > 0.0) {
        double delta_pt = reco_pt - gen_pt;
        double smeared_pt = gen_pt + sf * delta_pt;
        return std::max(0.0, smeared_pt);
    }

    double smear_factor = 1.0 + std::sqrt(sf * sf - 1.0) * gRandom->Gaus(0, 1) * resolution;
    return std::max(0.0, reco_pt * smear_factor);
}

TLorentzVector SSBCorrections::RecomputeMET(double raw_met_pt, double raw_met_phi,
                                            const std::vector<TLorentzVector>& rawJets,
                                            const std::vector<TLorentzVector>& corrJets) const {
    // Convert raw MET to px, py components
    double met_px = raw_met_pt * std::cos(raw_met_phi);
    double met_py = raw_met_pt * std::sin(raw_met_phi);

    // Calculate correction in px, py only
    for (size_t i = 0; i < rawJets.size(); ++i) {
        // Add back the difference in transverse momentum
        // (raw jet was subtracted from original MET, now we subtract corrected jet)
        met_px += (rawJets[i].Px() - corrJets[i].Px());
        met_py += (rawJets[i].Py() - corrJets[i].Py());
    }

    // Create corrected MET with mass = 0
    double corrected_met_pt = std::sqrt(met_px * met_px + met_py * met_py);
    double corrected_met_phi = std::atan2(met_py, met_px);

    TLorentzVector correctedMET;
    correctedMET.SetPtEtaPhiM(corrected_met_pt, 0, corrected_met_phi, 0);

    return correctedMET;
}

float SSBCorrections::GetPUJetIDSFAndEff(float pt, float eta, bool passPU, bool genMatched, const std::string& wp, const std::string& syst, bool getEff) const {
    if (!pujetid_sf_) {
        std::cerr << "[SSBCorrections::GetPUJetIDSFAndEff] PUJetID correction not loaded." << std::endl;
        return 1.0;
    }
    
    if (pt >= 50.0) return 1.0;
    
    try {
        std::string eval_type = getEff ? "MCEff" : syst;  // Efficiency or Scale Factor
        std::variant<double, std::vector<double>> val = pujetid_sf_->evaluate({eta, pt, eval_type, wp});
        return std::get<double>(val);
    } catch (const std::exception& e) {
        std::cerr << "[SSBCorrections::GetPUJetIDSFAndEff] Evaluation failed: " << e.what() << std::endl;
        return 1.0;
    }
}


double SSBCorrections::GetMuonRecoSF(double pt, double eta) const {
    if (!muon_reco_) return 1.0;
    try {
        std::variant<double, std::vector<double>> val = muon_reco_->evaluate({pt, eta});
        return std::get<double>(val);
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] GetMuonRecoSF failed: " << e.what()
                  << " (pt=" << pt << ", eta=" << eta << ")" << std::endl;
        throw;  // re-throw to see where it crashes
    }
}

double SSBCorrections::GetMuonIDSF(double pt, double eta, const std::string& syst_tag) const {
    if (!muon_id_) return 1.0;
    try {
        std::variant<double, std::vector<double>> val = muon_id_->evaluate({eta, pt, syst_tag});
        return std::get<double>(val);
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] GetMuonIDSF failed: " << e.what()
                  << " (eta=" << eta << ", pt=" << pt << ", syst=" << syst_tag << ")" << std::endl;
        throw;  // re-throw to see where it crashes
    }
}

double SSBCorrections::GetMuonIsoSF(double pt, double eta, const std::string& syst_tag) const {
    if (!muon_iso_) return 1.0;
    try {
        std::variant<double, std::vector<double>> val = muon_iso_->evaluate({eta, pt, syst_tag});
        return std::get<double>(val);
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] GetMuonIsoSF failed: " << e.what()
                  << " (eta=" << eta << ", pt=" << pt << ", syst=" << syst_tag << ")" << std::endl;
        throw;  // re-throw to see where it crashes
    }
}


double SSBCorrections::DoubleMuon_IDIsoEff(TLorentzVector lep1, TLorentzVector lep2,
                                           TString muidsys, TString muisosys, TString tracksys) const {
    float pt1 = std::min(lep1.Pt(), 119.999);
    float pt2 = std::min(lep2.Pt(), 119.999);
    float abseta1 = std::abs(lep1.Eta());
    float abseta2 = std::abs(lep2.Eta());

    //std::cout << "muidsys : " << muidsys << std::endl;
    //std::cout << "muisosys : " << muisosys << std::endl;

    std::string IDSyst = "nominal", IsoSyst = "nominal";

    if (muidsys.Contains("up", TString::kIgnoreCase)) IDSyst = "up";
    else if (muidsys.Contains("down", TString::kIgnoreCase)) IDSyst = "down";

    if (muisosys.Contains("up", TString::kIgnoreCase)) IsoSyst = "up";
    else if (muisosys.Contains("down", TString::kIgnoreCase)) IsoSyst = "down";
    //std::cout << "IDSyst : " << IDSyst << std::endl;
    double mu1id  = GetMuonIDSF(pt1, abseta1, IDSyst);
    //std::cout << "After mu1id  " << mu1id << std::endl;
    double mu2id  = GetMuonIDSF(pt2, abseta2, IDSyst);
    double mu1iso = GetMuonIsoSF(pt1, abseta1, IsoSyst);
    double mu2iso = GetMuonIsoSF(pt2, abseta2, IsoSyst);

    // track SF is 1.0, so you don't need to apply them... 
    double mu1trk =1.0;// TrackSF(lep1->Eta());
    double mu2trk =1.0;// TrackSF(lep2->Eta());
/*
    if (tracksys.Contains("up", TString::kIgnoreCase)) {
        mu1trk += TrackSFErr(lep1->Eta(), tracksys);
        mu2trk += TrackSFErr(lep2->Eta(), tracksys);
    } else if (tracksys.Contains("down", TString::kIgnoreCase)) {
        mu1trk -= TrackSFErr(lep1->Eta(), tracksys);
        mu2trk -= TrackSFErr(lep2->Eta(), tracksys);
    }
*/
    return mu1id * mu2id * mu1iso * mu2iso * mu1trk * mu2trk;
}

// Add this function to SSBCorrections.cpp


// Add this function to SSBCorrections.cpp

double SSBCorrections::DoubleElec_Eff(
    const TLorentzVector& lep1, const TLorentzVector& lep2,
    double ele1sueta, double ele2sueta, 
    const std::string& id_wp,        // "Tight", "Medium", "Loose"
    const std::string& id_syst,      // "nominal", "up", "down"
    const std::string& reco_syst     // "nominal", "up", "down"
) const {
    
    // Step 1: Apply pt/eta limits based on JSON ranges
    // pT range in JSON: 10.0 to Infinity, but clamp very high values
    float lep1pt = std::clamp(static_cast<float>(lep1.Pt()), 10.0f, 999.0f);
    float lep2pt = std::clamp(static_cast<float>(lep2.Pt()), 10.0f, 999.0f);
    
    // Eta range in JSON: -Infinity to +Infinity, but avoid extreme values  
    // Keep within reasonable detector range
    float lep1sueta_clamped = std::clamp(static_cast<float>(ele1sueta), -3.0f, 3.0f);
    float lep2sueta_clamped = std::clamp(static_cast<float>(ele2sueta), -3.0f, 3.0f);
    
    // Get phi from TLorentzVector (needed for 2023, 2023BPix)
    float lep1phi = static_cast<float>(lep1.Phi());
    float lep2phi = static_cast<float>(lep2.Phi());
    
    // Step 2: Calculate ID SF for each electron using GetElectronSF
    // Check if id_wp is empty and set default
    std::string actual_id_wp = id_wp;
    if (actual_id_wp.empty()) {
        actual_id_wp = "Tight";  // Set default working point
        std::cout << "[WARNING] Empty ID working point, using default: " << actual_id_wp << std::endl;
    }
    
    // Use working point directly (not with "ID" prefix)
    float ele1id = GetElectronSF(actual_id_wp, lep1sueta_clamped, lep1pt, id_syst, lep1phi);
    float ele2id = GetElectronSF(actual_id_wp, lep2sueta_clamped, lep2pt, id_syst, lep2phi);
    
    // Step 3: Calculate Reco SF for each electron using GetElectronSF
    // "Reco" type will automatically choose RecoAbove20/RecoBelow20 based on pt
    float ele1reco = GetElectronSF("Reco", lep1sueta_clamped, lep1pt, reco_syst, lep1phi);
    float ele2reco = GetElectronSF("Reco", lep2sueta_clamped, lep2pt, reco_syst, lep2phi);
    
    // Step 4: Isolation SF is 1.0 (typically included in ID for electrons)
    float ele1iso = 1.0f;
    float ele2iso = 1.0f;
    
    // Step 5: Compute total double electron efficiency
    double doubleEleff = static_cast<double>(ele1id) * static_cast<double>(ele2id) * 
                        static_cast<double>(ele1iso) * static_cast<double>(ele2iso) * 
                        static_cast<double>(ele1reco) * static_cast<double>(ele2reco);
    
    // Debug output (can be removed in production)
    /*
    std::cout << "[DoubleElec_Eff] Debug info:" << std::endl;
    std::cout << "  Ele1: pt=" << lep1pt << ", eta=" << lep1sueta_clamped 
              << ", ID_SF=" << ele1id << ", Reco_SF=" << ele1reco << std::endl;
    std::cout << "  Ele2: pt=" << lep2pt << ", eta=" << lep2sueta_clamped 
              << ", ID_SF=" << ele2id << ", Reco_SF=" << ele2reco << std::endl;
    std::cout << "  Total efficiency: " << doubleEleff << std::endl;
    */ 
    
    return doubleEleff;
}

double SSBCorrections::MuonElec_Eff(const TLorentzVector& muon, const TLorentzVector& electron,
                                    double muon_eta, double electron_sueta,
                                    const std::string& mu_id_syst, 
                                    const std::string& mu_iso_syst,
                                    const std::string& ele_id_wp,
                                    const std::string& ele_id_syst, 
                                    const std::string& ele_reco_syst) const {
    
    // Step 1: Apply pt/eta limits based on JSON ranges
    // Muon: pt range typically up to ~120 GeV in measurements
    float muon_pt = std::clamp(static_cast<float>(muon.Pt()), 15.0f, 119.999f);
    float muon_abseta = std::clamp(std::abs(static_cast<float>(muon_eta)), 0.0f, 2.4f);
    
    // Electron: pt range 10.0 to Infinity in JSON, eta range within detector acceptance
    float electron_pt = std::clamp(static_cast<float>(electron.Pt()), 10.0f, 999.0f);
    float electron_sueta_clamped = std::clamp(static_cast<float>(electron_sueta), -3.0f, 3.0f);
    
    // Get phi from TLorentzVector (needed for 2023, 2023BPix)
    float electron_phi = static_cast<float>(electron.Phi());

    // Step 2: Calculate Muon ID SF
    float mu_id = GetMuonIDSF(muon_pt, muon_abseta, mu_id_syst);
    
    // Step 3: Calculate Muon Iso SF  
    float mu_iso = GetMuonIsoSF(muon_pt, muon_abseta, mu_iso_syst);
    
    // Step 4: Muon tracking SF (typically 1.0 for current analyses)
    float mu_trk = 1.0f;

    // Step 5: Calculate Electron ID SF
    std::string actual_ele_id_wp = ele_id_wp;
    if (actual_ele_id_wp.empty()) {
        actual_ele_id_wp = "Tight";  // Set default working point
        std::cout << "[WARNING] Empty electron ID working point, using default: " << actual_ele_id_wp << std::endl;
    }
    
    float ele_id = GetElectronSF(actual_ele_id_wp, electron_sueta_clamped, electron_pt, ele_id_syst, electron_phi);
    
    // Step 6: Calculate Electron Reco SF
    // "Reco" type will automatically choose RecoAbove20/RecoBelow20 based on pt
    float ele_reco = GetElectronSF("Reco", electron_sueta_clamped, electron_pt, ele_reco_syst, electron_phi);
    
    // Step 7: Electron isolation SF is 1.0 (typically included in ID for electrons)
    float ele_iso = 1.0f;

    // Step 8: Compute total muon-electron efficiency
    double muonelec_eff = static_cast<double>(mu_id) * static_cast<double>(mu_iso) * 
                         static_cast<double>(mu_trk) * static_cast<double>(ele_id) * 
                         static_cast<double>(ele_iso) * static_cast<double>(ele_reco);

    // Debug output (can be removed in production)
    /*
    std::cout << "[MuonElec_Eff] Debug info:" << std::endl;
    std::cout << "  Muon: pt=" << muon_pt << ", abseta=" << muon_abseta 
              << ", ID_SF=" << mu_id << ", Iso_SF=" << mu_iso << ", Trk_SF=" << mu_trk << std::endl;
    std::cout << "  Electron: pt=" << electron_pt << ", sueta=" << electron_sueta_clamped 
              << ", ID_SF=" << ele_id << ", Reco_SF=" << ele_reco << ", Iso_SF=" << ele_iso << std::endl;
    std::cout << "  Total efficiency: " << muonelec_eff << std::endl;
    */

    return muonelec_eff;
}




float SSBCorrections::GetElectronSF(const std::string& sf_type, float eta, float pt, const std::string& syst, float phi) const {

    // Map systematic names: "nominal" -> "sf", "up" -> "sfup", "down" -> "sfdown"
    std::string valtype = "sf";  // default

    if (syst == "up") {
        valtype = "sfup";
    } else if (syst == "down") {
        valtype = "sfdown";
    } else if (syst == "nominal" || syst.empty()) {
        valtype = "sf";
    } else {
        // Handle unknown systematic values
        std::cerr << "[WARNING] Unknown systematic: '" << syst << "', using nominal (sf)" << std::endl;
        valtype = "sf";
    }

    // Map working point names from config to JSON format
    std::string working_point = sf_type;
    if (sf_type == "Reco") {
        // Reco SF binning depends on era:
        // Run2 (2016-2018): RecoBelow20 / RecoAbove20
        // Run3 (2022+): RecoBelow20 / Reco20to75 / RecoAbove75
        if (ele_reco_sf_) {
            // Separate Reco file (e.g. 2024 electron_v1.json): no RecoBelow20
            if (pt < 20.0f) return 1.0f;
            working_point = (pt >= 75.0f) ? "RecoAbove75" : "Reco20to75";
        } else if (pt < 20.0f) {
            working_point = "RecoBelow20";
        } else {
            bool run3_reco_bins = (year_.find("2022") != std::string::npos ||
                                   year_.find("2023") != std::string::npos ||
                                   year_.find("2024") != std::string::npos ||
                                   !ele_id_sf_year_.empty());
            if (run3_reco_bins) {
                working_point = (pt >= 75.0f) ? "RecoAbove75" : "Reco20to75";
            } else {
                working_point = "RecoAbove20";
            }
        }
    } else if (sf_type.find("SCB") == 0) {
        // Convert SCB (Scale factor Cut-Based) format to JSON format
        if (sf_type == "SCBVeto") working_point = "Veto";
        else if (sf_type == "SCBLoose") working_point = "Loose";
        else if (sf_type == "SCBMedium") working_point = "Medium";
        else if (sf_type == "SCBTight") working_point = "Tight";
        else {
            std::cerr << "[WARNING] Unknown SCB working point: " << sf_type << ", using Tight" << std::endl;
            working_point = "Tight";
        }
    } else if (sf_type.find("MVA") == 0) {
        // Convert MVA format to JSON format if needed
        if (sf_type == "MVALoose") working_point = "wp90noiso";
        else if (sf_type == "MVAMedium") working_point = "wp80noiso";
        else if (sf_type == "MVATight") working_point = "wp90iso";
        else {
            std::cerr << "[WARNING] Unknown MVA working point: " << sf_type << ", using wp90iso" << std::endl;
            working_point = "wp90iso";
        }
    }
    // For other cases (direct JSON names like "Tight", "Medium"), use as-is

    try {
        if (!ele_sf_) {
            std::cerr << "[ERROR] ele_sf_ pointer is null!" << std::endl;
            return 1.0;
        }

        // Validate parameters before evaluation
        if (year_.empty()) {
            std::cerr << "[ERROR] Year is empty!" << std::endl;
            return 1.0;
        }

        if (working_point.empty()) {
            std::cerr << "[ERROR] Working point is empty!" << std::endl;
            return 1.0;
        }

        // Electron SF schema varies by year:
        // 2024 Reco: use ele_reco_sf_ from electron_v1.json, year="2024Prompt"
        // 2022, 2022PostEE, 2024 ID: {year, ValType, WorkingPoint, eta, pt} - 5 inputs
        // 2023, 2023BPix: {year, ValType, WorkingPoint, eta, pt, phi} - 6 inputs
        const std::string& ele_sf_year = ele_id_sf_year_.empty() ? year_ : ele_id_sf_year_;
        float result;
        if (sf_type == "Reco" && ele_reco_sf_ && !ele_reco_sf_year_.empty()) {
            result = ele_reco_sf_->evaluate({ele_reco_sf_year_, valtype, working_point, eta, pt});
        } else {
            bool needsPhi = (ele_sf_year.find("2023") != std::string::npos &&
                             ele_sf_year.find("2024") == std::string::npos);
            if (needsPhi) {
                result = ele_sf_->evaluate({ele_sf_year, valtype, working_point, eta, pt, phi});
            } else {
                result = ele_sf_->evaluate({ele_sf_year, valtype, working_point, eta, pt});
            }
        }

        // Sanity check on result
        if (result <= 0.0 || result > 10.0) {
            std::cerr << "[WARNING] Unusual SF value: " << result
                      << " for parameters: " << working_point << ", eta=" << eta << ", pt=" << pt << std::endl;
        }

        return result;

    } catch (const std::exception& e) {
        std::cerr << "[GetElectronSF] Evaluation failed: " << e.what() << std::endl;
        const std::string& ele_sf_year_err = ele_id_sf_year_.empty() ? year_ : ele_id_sf_year_;
        std::cerr << "  Parameters: year='" << ele_sf_year_err << "', valtype='" << valtype
                  << "', working_point='" << working_point << "', eta=" << eta << ", pt=" << pt 
                  << ", phi=" << phi << std::endl;
        return 1.0;
    }
}

float SSBCorrections::GetPUWeight(float nTrueInt, const std::string& systTag) const {
    std::string variation = systTag;
    
    if (!pu_weight_) {
        std::cerr << "[SSBCorrections::GetPUWeight] PU correction not loaded.\n";
        return 1.0;
    }
    
    // If systTag is "Central" or empty, treat as "nominal"
    if (systTag == "Central" || systTag == "") {
        variation = "nominal";
    }
    
    //std::cout << "variation in PU " << variation << std::endl;

    // If the variation is not valid, fallback to "nominal"
    if (variation != "nominal" && variation != "up" && variation != "down") {
        std::cerr << "PileUp sys Error ... Defaulting to Weight_PileUp ... Original value: " << variation << std::endl;
        variation = "nominal";
    }
    
    //std::cout << "nTrueInt : " << nTrueInt << std::endl;
    
    try {
        std::variant<double, std::vector<double>> val = pu_weight_->evaluate({ nTrueInt, variation });
        double weight = std::get<double>(val);
        //std::cout << "PileUp weight: " << weight << std::endl;  
        return weight;
    } catch (const std::exception& e) {
        std::cerr << "[SSBCorrections::GetPUWeight] Evaluation failed: " << e.what() << std::endl;
        return 1.0;
    }
}


float SSBCorrections::MatchGenPt(const TLorentzVector& reco_jet,
                                  const std::vector<TLorentzVector>& gen_jets,
                                  float maxDR) const {
    float minDR = maxDR;
    float matched_genpt = -1.0;

    for (const auto& gen_jet : gen_jets) {
        float dR = reco_jet.DeltaR(gen_jet);
        if (dR < minDR) {
            minDR = dR;
            matched_genpt = gen_jet.Pt();
        }
    }
    return matched_genpt;
}

JetCorrectionOutput SSBCorrections::ApplyJetCorrectionsWithMET(
    const std::vector<TLorentzVector>& rawJets,
    const std::vector<float>& rawFactors,
    const std::vector<float>& areas,
    float rho,
    bool isData,
    bool applyJES,
    bool applyJER,
    double raw_met_pt,
    double raw_met_phi,
    const std::vector<TLorentzVector>& genJets,
    const std::vector<int>& genJetIndices,
    unsigned int runnb
) const {
    std::vector<TLorentzVector> correctedJets;
    std::vector<TLorentzVector> rawJetsRebuilt;

    correctedJets.reserve(rawJets.size());
    rawJetsRebuilt.reserve(rawJets.size());

    for (size_t i = 0; i < rawJets.size(); ++i) {
        double eta  = rawJets[i].Eta();
        double phi  = rawJets[i].Phi();

        double raw_pt   = rawJets[i].Pt() * (1.0 - rawFactors[i]);
        double raw_mass = rawJets[i].M()  * (1.0 - rawFactors[i]);

        TLorentzVector raw_jet;
        raw_jet.SetPtEtaPhiM(raw_pt, eta, phi, raw_mass);
        rawJetsRebuilt.push_back(raw_jet);

        double corrected_pt = raw_pt;
        double corrected_mass = raw_mass;

        if (applyJES) {
            corrected_pt   = GetCorrectedJetPt(raw_pt, eta, areas[i], rho, phi, runnb, isData);
            corrected_mass = GetCorrectedJetMass(raw_mass, raw_pt, eta, areas[i], rho, phi, runnb, isData);
        }

        if (!isData && applyJER && genJetIndices.size() > i && genJetIndices[i] >= 0 && genJetIndices[i] < genJets.size()) {
            float matched_genpt = genJets[genJetIndices[i]].Pt();
            corrected_pt = SmearJER(corrected_pt, matched_genpt, eta, rho, "nominal");
        }

        if (raw_pt > 0) {
            corrected_mass *= (corrected_pt / raw_pt);
        }

        TLorentzVector corr_jet;
        corr_jet.SetPtEtaPhiM(corrected_pt, eta, phi, corrected_mass);
        correctedJets.push_back(corr_jet);
    }

    TLorentzVector correctedMET = RecomputeMET(raw_met_pt, raw_met_phi, rawJetsRebuilt, correctedJets);
    JetCorrectionOutput result;

    result.corrected_jets = correctedJets;
    result.corrected_met  = correctedMET;

    return result;
}


bool SSBCorrections::ShouldVetoJet(const TLorentzVector& jet) const {
    // Only apply for 2018 data/MC
    if (year_ != "2018" && year_ != "2024" && year_ != "2023" && year_ != "2022" && year_ != "2022PostEE" && year_ != "2023BPix") {
        return false;
    }
    
    // Check if jetvetomap is loaded
    if (!jetvetomap_) {
        std::cerr << "[WARNING] Jet veto map not loaded, skipping HEM veto" << std::endl;
        return false;
    }
    
    float eta = jet.Eta();
    float phi = jet.Phi();
    
    try {
        // Use configured key
        std::variant<double, std::vector<double>> val = jetvetomap_->evaluate({
            jveto_key_, eta, phi
        });
        
        double veto_flag = std::get<double>(val);
        
        // Debug output to see what values we're getting
//        std::cout << "[DEBUG] Jet eta=" << eta << ", phi=" << phi 
//                  << " -> veto_flag=" << veto_flag << std::endl;
        
        // Return true if jet should be vetoed (non-zero value)
        bool should_veto = (veto_flag > 0.0);
        
//        if (should_veto) {
//            std::cout << "[DEBUG] -> VETOED (flag=" << veto_flag << ")" << std::endl;
//        }
        
        return should_veto;
        
    } catch (const std::exception& e) {
        std::cerr << "[WARNING] Jet veto map evaluation failed for jet (eta=" 
                  << eta << ", phi=" << phi << ") with key=" << jveto_key_ 
                  << ": " << e.what() << std::endl;
        return false;
    }
}

bool SSBCorrections::HasJetIDCorrection(bool use_tightlepveto) const {
    return use_tightlepveto ? (jetid_tightlepveto_ != nullptr) : (jetid_tight_ != nullptr);
}

bool SSBCorrections::PassJetIDFromJSON(float eta,
                                       float chHEF, float neHEF, float chEmEF, float neEmEF, float muEF,
                                       int chMultiplicity, int neMultiplicity, int multiplicity,
                                       bool use_tightlepveto) const {
    const auto& corr = use_tightlepveto ? jetid_tightlepveto_ : jetid_tight_;
    if (!corr) {
        return true; // fallback-open if not loaded
    }

    auto evalJetID = [&](double eta_value) -> double {
        std::vector<correction::Variable::Type> args;
        const auto& inputs = corr->inputs();
        args.reserve(inputs.size());

        for (const auto& in : inputs) {
            const auto& name = in.name();
            const auto type = in.type();

            if (type == correction::Variable::VarType::real) {
                if (name == "eta" || name == "abseta") args.emplace_back(static_cast<double>(eta_value));
                else if (name == "chHEF") args.emplace_back(static_cast<double>(chHEF));
                else if (name == "neHEF") args.emplace_back(static_cast<double>(neHEF));
                else if (name == "chEmEF") args.emplace_back(static_cast<double>(chEmEF));
                else if (name == "neEmEF") args.emplace_back(static_cast<double>(neEmEF));
                else if (name == "muEF") args.emplace_back(static_cast<double>(muEF));
                else args.emplace_back(0.0);
            } else if (type == correction::Variable::VarType::integer) {
                if (name == "chMultiplicity") args.emplace_back(static_cast<int>(chMultiplicity));
                else if (name == "neMultiplicity") args.emplace_back(static_cast<int>(neMultiplicity));
                else if (name == "multiplicity" || name == "nConstituents") args.emplace_back(static_cast<int>(multiplicity));
                else args.emplace_back(0);
            } else { // string
                args.emplace_back(std::string(""));
            }
        }
        return static_cast<double>(corr->evaluate(args));
    };

    try {
        return evalJetID(static_cast<double>(eta)) > 0.5;
    } catch (const std::exception&) {
        // Retry once with |eta| for schemas that are defined in absolute eta.
        try {
            return evalJetID(static_cast<double>(std::fabs(eta))) > 0.5;
        } catch (const std::exception& e2) {
            std::cerr << "[WARNING] JetID JSON evaluation failed: " << e2.what() << std::endl;
            return true; // fallback-open to avoid hard event loss on transient issues
        }
    }
}


void SSBCorrections::InitBtagSFCorrection(const std::string& json_path, 
                                          const std::string& tagger_name) {
    std::cout << "[SSBCorrections] Loading b-tagging SF from JSON: " << json_path << std::endl;
    
    auto cset = correction::CorrectionSet::from_file(json_path);

    // Run3 UParTAK4 corrections are provided as a single correction object
    // with flavor as an input variable (no comb/incl split names).
    if (tagger_name.find("UParTAK4_") == 0) {
        try {
            btag_corrections_["heavy"] = cset->at(tagger_name);
            btag_corrections_["light"] = cset->at(tagger_name);
            std::cout << "[SSBCorrections] Loaded UParTAK4 correction: "
                      << tagger_name << std::endl;
            btag_sf_available_ = true;
        } catch (const std::out_of_range& e) {
            std::cerr << "[WARNING] UParTAK4 correction '" << tagger_name 
                      << "' not found in JSON. B-tagging SF will be disabled." << std::endl;
            btag_sf_available_ = false;
        }
        return;
    }

    // Run2/DeepCSV/DeepJet corrections use comb/incl naming convention.
    std::string heavy_flavor_name = tagger_name;  // e.g., "deepJet_comb"
    size_t pos = heavy_flavor_name.find("comb");
    if (pos != std::string::npos) {
        heavy_flavor_name.replace(pos, 4, btag_sf_type_);
    } else {
        std::cerr << "[ERROR] Expected 'comb' in tagger_name: " << tagger_name << std::endl;
        btag_sf_available_ = false;
        return;
    }

    std::string light_flavor_name = tagger_name;
    pos = light_flavor_name.find("comb");
    if (pos != std::string::npos) {
        light_flavor_name.replace(pos, 4, "incl");
    }

    try {
        btag_corrections_["heavy"] = cset->at(heavy_flavor_name);
        btag_corrections_["light"] = cset->at(light_flavor_name);
        std::cout << "[SSBCorrections] Loaded corrections: "
                  << heavy_flavor_name << " and " << light_flavor_name << std::endl;
        btag_sf_available_ = true;
    } catch (const std::out_of_range& e) {
        std::cerr << "[WARNING] B-tagging correction not found in JSON. B-tagging SF will be disabled." << std::endl;
        btag_sf_available_ = false;
    }
}

std::string SSBCorrections::getBtagCorrectionName(int flavor) const {
    // 0 = light flavor (u,d,s,g), others = heavy flavor (b,c)
    return (flavor == 0) ? "light" : "heavy";
}

float SSBCorrections::GetBtagSF(float pt, float eta, int flav, 
                                const std::string& wp, const std::string& syst) const {
    // If b-tagging SF is not available for this year, return 1.0
    if (!btag_sf_available_) {
        return 1.0f;
    }
    
    float pt_clamped = std::clamp(pt, 20.0f, 1000.0f);
    float eta_abs = std::fabs(eta);

    std::string corr_name = getBtagCorrectionName(flav);  // "heavy" or "light"
    auto it = btag_corrections_.find(corr_name);
    
    if (it == btag_corrections_.end()) {
        std::cerr << "[ERROR] B-tag correction not found: " << corr_name << std::endl;
        return 1.0;
    }

    try {
        return it->second->evaluate({syst, wp, flav, eta_abs, pt_clamped});
    } catch (const std::exception& e) {
        std::cerr << "[WARNING] GetBtagSF failed: " << e.what() << std::endl;
        return 1.0;
    }
}

float SSBCorrections::ComputeBTagEventWeight(const std::vector<float>& pts,
                                             const std::vector<float>& etas,
                                             const std::vector<int>& flavs,
                                             const std::vector<bool>& isTagged,
                                             const std::string& algo,
                                             const std::string& wp,
                                             const std::string& syst) const {
    
    // Check input vector sizes
    if (pts.size() != etas.size() || 
        pts.size() != flavs.size() || 
        pts.size() != isTagged.size()) {
        std::cout << "Input vectors have different sizes" << std::endl;
        return 1.0;
    }
    //std::cout << "start ! " << std::endl; 
    // Initialize probabilities
    float p_mc = 1.0;
    float p_data = 1.0;
    
    // Loop over all jets
    for (size_t i = 0; i < pts.size(); ++i) {
        float pt   = pts[i];
        float eta  = etas[i];
        int flav   = flavs[i];
        bool tagged = isTagged[i];
         
        // Get MC efficiency
        float eff = GetMCBtagEfficiency(pt, eta, flav, algo, wp);
        
        // UParTAK4_kinfit in current Run3 JSON provides SF only for b-jets (flavor=5).
        // For c/light jets, keep SF=1.0 to avoid invalid category lookups.
        float sf = 1.0f;
        if (!(algo == "UParTAK4" && flav != 5)) {
            sf = GetBtagSF(pt, eta, flav, wp, syst);
        }
        
        // Avoid division by zero
        if (eff < 1e-5) eff = 1e-5;
        if (eff > 1.0 - 1e-5) eff = 1.0 - 1e-5;
        
        // Calculate probabilities
        if (tagged) {
            // Tagged jet contribution
            p_mc *= eff;
            p_data *= (eff * sf);
        } else {
            // Not tagged jet contribution
            p_mc *= (1.0f - eff);
            p_data *= (1.0f - eff * sf);
        }
        
        /*std::cout << "  Jet " << i << ": pt=" << pt << ", eta=" << eta 
                  << ", flav=" << flav << ", tagged=" << tagged 
                  << ", SF=" << sf << ", Eff=" << eff 
                  << ", p_mc=" << p_mc << ", p_data=" << p_data << std::endl;*/
    }
    
    // Calculate final event weight
    float btag_evt_weight = (p_mc > 1e-10) ? p_data / p_mc : 1.0;
    
    return btag_evt_weight;
}


void SSBCorrections::LoadMCBtagEfficiencies(const std::string& filepath, const std::string& algo) {
    TFile* f = TFile::Open(filepath.c_str(), "READ");
    if (!f || f->IsZombie()) {
        std::cerr << "[ERROR] Failed to open efficiency file: " << filepath << std::endl;
        return;
    }

    for (const std::string& flav : {"b", "c", "l"}) {
        for (const std::string& wp : {"Loose", "Medium", "Tight"}) {
            std::string name = "eff_" + algo + "_" + flav + "_" + wp;
            TH2D* hist = (TH2D*)f->Get(name.c_str());
            if (hist) {
                // Create a safe copy of the histogram
                TH2D* hist_copy = (TH2D*)hist->Clone((name + "_copy").c_str());
                hist_copy->SetDirectory(0); // Remove ROOT ownership
                
                // Delete existing histogram if present
                auto it = eff_histograms_.find(algo + "_" + flav + "_" + wp);
                if (it != eff_histograms_.end() && it->second) {
                    delete it->second;
                }
                
                eff_histograms_[algo + "_" + flav + "_" + wp] = hist_copy;
                std::cout << "[INFO] Loaded and copied hist: " << name << std::endl;
            } else {
                std::cerr << "[WARNING] Histogram not found: " << name << std::endl;
            }
        }
    }
    f->Close();
    delete f; // Explicitly delete file object
}


/*
void SSBCorrections::LoadMCBtagEfficiencies(const std::string& filepath, const std::string& algo) {
    TFile* f = TFile::Open(filepath.c_str(), "READ");
    if (!f || f->IsZombie()) {
        std::cerr << "[ERROR] Failed to open efficiency file: " << filepath << std::endl;
        return;
    }

    for (const std::string& flav : {"b", "c", "l"}) {
        for (const std::string& wp : {"Loose", "Medium", "Tight"}) {
            std::string name = "eff_" + algo + "_" + flav + "_" + wp;
	    //std::cout << "name in LoadMCBtagEfficiencies : " << name << std::endl;
            TH2D* hist = (TH2D*)f->Get(name.c_str());
            if (hist) {
                hist->SetDirectory(0); // Detach histogram from file
                eff_histograms_[algo + "_" + flav + "_" + wp] = hist;
                //std::cout << "[INFO] Loaded hist: " << name << std::endl;
            } else {
                std::cerr << "[WARNING] Histogram not found: " << name << std::endl;
            }
        }
    }
    f->Close();
}
*/
float SSBCorrections::GetMCBtagEfficiency(float pt, float eta, int flav, const std::string& algo, const std::string& wp) const {
    std::string flav_str = "l";
    if (flav == 5) flav_str = "b";
    else if (flav == 4) flav_str = "c";

    // Convert single character WP to full name for efficiency lookup
    std::string wp_full = wp;
    if (wp == "L") wp_full = "Loose";
    else if (wp == "M") wp_full = "Medium";
    else if (wp == "T") wp_full = "Tight";

    std::string key = algo + "_" + flav_str + "_" + wp_full;
    //std::cout << "[Info] key in GetMCBtagEfficiency " << key << std::endl;
    auto it = eff_histograms_.find(key);
    if (it == eff_histograms_.end()) {
        std::cerr << "[WARNING] Efficiency hist not found: " << key << std::endl;
        return 1.0;
    }

    TH2D* hist = it->second;
    int bin_x = hist->GetXaxis()->FindBin(pt);
    int bin_y = hist->GetYaxis()->FindBin(std::fabs(eta));
    float eff = hist->GetBinContent(bin_x, bin_y);

    return std::clamp(eff, 0.0f, 1.0f);
}

double SSBCorrections::RochesterCorrectionData(TString year, int Q, double pt, double eta, double phi, int s,int m) const{
        double correction;
        correction = rc.kScaleDT(Q,pt,eta,phi,s,m); return correction;
}

double SSBCorrections::RochesterCorrectionMC(TString year, int Q, double pt, double eta,double phi,int genID,double genPt,int nl, int s,int m) const{

    double correction = 1.0; double u =1.0;

    bool genMatch = genID != -1;

    if(genMatch) correction = rc.kSpreadMC(Q,pt,eta,phi,genPt,s,m);
    else if(!genMatch){u = gRandom->Rndm(); correction = rc.kSmearMC(Q,pt,eta,phi,nl,u,s,m);} //Random number is needed when gen-mathcing is failed
    return correction;
}


TLorentzVector SSBCorrections::METXYCorrection(const TLorentzVector& type1_met,
                                               int runnb, TString year, bool isMC, int npv, bool isUL, bool ispuppi
                                               ) const {


    std::pair<double, double> correctedMET = METXYCorr_Met_MetPhi(type1_met.Pt(),type1_met.Phi(),runnb,year,isMC,npv,isUL,ispuppi);

    double met_pt  = correctedMET.first;
    double met_phi = correctedMET.second;

    TLorentzVector corrected_met;
    corrected_met.SetPtEtaPhiM(met_pt,0,met_phi,0);
    return corrected_met;
}


TLorentzVector SSBCorrections::METXYCorrection_corrlib(const TLorentzVector& type1_met,
                                               const std::string& era,
                                               bool isData,
                                               int npv) const {
    if (!metphi_corr_) {
        std::cerr << "[SSBCorrections::METXYCorrection] MET correction object not loaded.\n";
        return type1_met;
    }

    std::string data_tag = isData ? "data" : "mc";

    double corr_x = 0.0;
    double corr_y = 0.0;

    try {
        std::variant<double, std::vector<double>> val_x = metphi_corr_->evaluate({era, data_tag, npv, "x"});
        std::variant<double, std::vector<double>> val_y = metphi_corr_->evaluate({era, data_tag, npv, "y"});

        if (std::holds_alternative<double>(val_x)) {
            corr_x = std::get<double>(val_x);
        } else {
            std::cerr << "[METXYCorrection] Warning: unexpected type for x correction\n";
        }

        if (std::holds_alternative<double>(val_y)) {
            corr_y = std::get<double>(val_y);
        } else {
            std::cerr << "[METXYCorrection] Warning: unexpected type for y correction\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "[SSBCorrections::METXYCorrection] Evaluation failed: " << e.what() << std::endl;
        return type1_met;
    }

    double met_x = type1_met.Px() - corr_x;
    double met_y = type1_met.Py() - corr_y;

    TLorentzVector corrected_met;
    corrected_met.SetPxPyPzE(met_x, met_y, 0, std::sqrt(met_x * met_x + met_y * met_y));
    return corrected_met;
}
/// Trigger SF 
double SSBCorrections::TrigDiMuon_Eff(TLorentzVector lep1, TLorentzVector lep2, TString Sys_) {
    double pt1 = lep1.Pt();
    double pt2 = lep2.Pt();

    double leading_pt = std::max(pt1, pt2);
    double subleading_pt = std::min(pt1, pt2);

    return GetTrgEff(leading_pt, subleading_pt, Sys_);
}

double SSBCorrections::TrigDiElec_Eff(TLorentzVector lep1, TLorentzVector lep2, TString Sys_) {
    double pt1 = lep1.Pt();
    double pt2 = lep2.Pt();

    double leading_pt = std::max(pt1, pt2);
    double subleading_pt = std::min(pt1, pt2);

    return GetTrgEff(leading_pt, subleading_pt, Sys_);
}

double SSBCorrections::TrigMuElec_Eff(TLorentzVector muon, TLorentzVector elec, TString Sys_) {
    double pt_mu = muon.Pt();   // Assuming lep1 is muon
    double pt_ele = elec.Pt();  // Assuming lep2 is electron

    return GetTrgEff(pt_ele, pt_mu, Sys_);
}

double SSBCorrections::GetTrgEff(double pt1, double pt2, TString Sys_) {
    if (!H_trig) return 1.0;  // No correction if histogram failed to load

    // Clamp values below the histogram max range (assumed 500)
    double max_pt = 499.999;

    pt1 = std::min(pt1, max_pt);
    pt2 = std::min(pt2, max_pt);

    int xbin = H_trig->GetXaxis()->FindBin(pt1);
    int ybin = H_trig->GetYaxis()->FindBin(pt2);

    double trgsf = H_trig->GetBinContent(xbin, ybin);
    double trgsferr = 0.0;

    if (Sys_ == "nominal" || Sys_ == "Central") {trgsferr = 0.0;}
    else if (Sys_ == "Up" || Sys_ == "up")
        trgsferr = H_trig->GetBinError(xbin, ybin);
    else if (Sys_ == "Down" || Sys_ == "down")
        trgsferr = -H_trig->GetBinError(xbin, ybin);

    return trgsf + trgsferr;
}

std::string SSBCorrections::ExpandJECName(const std::string& base_jec_name, const std::string runPeriod, const std::string& era, bool is_data) {
    //  "Summer19UL16_v7" -> prefix = "Summer19UL16", version = "7"
    size_t pos = base_jec_name.find("_V");
    if (pos == std::string::npos) {
        std::cerr << "[ERROR] Invalid jec_name format: " << base_jec_name << std::endl;
        return base_jec_name; // fallback
    }

    std::string prefix = base_jec_name.substr(0, pos);             // "Summer19UL16"
    std::string version = base_jec_name.substr(pos + 2);           // "7"
    // Run2 UL NanoAOD AK4 jets are PF CHS; Run3 uses Puppi.
    std::string suffix = "_L1L2L3Res_AK4PFPuppi";
    if (runPeriod.find("2018") != std::string::npos) {
        suffix = "_L1L2L3Res_AK4PFchs";
    }

    std::string expanded = prefix;  // prefix

    // Year-specific logic
    if (runPeriod.find("16Pre") != std::string::npos) {
        if (is_data) {
            if (era == "B" || era == "C" || era == "D") {
                expanded += "_RunBCD";
            } else if (era == "E") {
                expanded += "_RunEF";
            } else if (era.find("F") == 0) {
                expanded += "_RunEF";
            } else if (era == "G" || era == "H") {
                expanded += "_RunGH";
            } else {
                expanded += "_RunFGH";  // fallback
            }
        } else {
            expanded += (era == "B" || era == "C" || era == "D" || era == "E") ? "APV" : "";
        }
    } else if (runPeriod.find("16Post") != std::string::npos){
        if (is_data) {
	    if (era.find("F") == 0) {
                expanded += "_RunFGH";
            } else if (era == "G" || era == "H") {
                expanded += "_RunFGH";
            } else {
                expanded += "_RunFGH";  // fallback
            }
        } else {
            expanded += (era == "F" || era == "G" || era == "H") ? "APV" : "";
        }

    } else if (runPeriod.find("2017") != std::string::npos) {
        if (is_data) {
            if (era == "B" ) {
                expanded += "_RunB";
            } else if (era == "C") {
                expanded += "_RunC";
            } else if (era == "D" ) {
                expanded += "_RunD";
            } else if (era == "E") {
                expanded += "_RunE";
            } else if (era == "F") {
                expanded += "_RunF"; 
            } else {
                expanded += "_RunB"; 
            }
        
        }
    } else if (runPeriod.find("2018") != std::string::npos) {
        if (is_data) {
            if (era == "A" ) {
                expanded += "_RunA";
            } else if (era == "B") {
                expanded += "_RunB";
            } else if (era == "C" ) {
                expanded += "_RunC";
            } else if (era == "D") {
                expanded += "_RunD";
            } else {
                expanded += "_RunB"; 
            }
        }
    } else if (runPeriod.find("2024") != std::string::npos) {
        if (is_data) {
            if (era == "C" || era == "D" || era == "E" || era == "F" || era == "G" || era == "H" || era == "I" || era == "I_v2" ) {
                expanded += "";
            }
        }
    } else if (runPeriod.find("2023BPix") != std::string::npos) {
        if (is_data) {
            if (era == "D") {
                expanded += "";
            }
        }
    } else if (runPeriod.find("2023") != std::string::npos) {
        if (is_data) {
            if (era == "C") {
                expanded += "";
            }
        }
    } else if (runPeriod.find("2022PostEE") != std::string::npos) {
        if (is_data) {
            if (era == "E") {
                expanded += "_RunE";
            } else if (era == "F") {
                expanded += "_RunF";
            } else if (era == "G") {
                expanded += "_RunG";
            } else {
                expanded += "_RunE";  // fallback
            }
        }
    } else if (runPeriod.find("2022") != std::string::npos) {
        if (is_data) {
            if (era == "C" || era == "D") {
                expanded += "_RunCD";
            } else {
                expanded += "_RunCD";  // fallback
            }
        }
    }

    // Add version and data/MC tag
    expanded += "_V" + version + "_";
    expanded += is_data ? "DATA" : "MC";
    expanded += suffix;

    return expanded;
}
std::string SSBCorrections::GetJetVetoType() const {
	return jveto_type_; 
}

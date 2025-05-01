#include "../interface/SSBCorrections.h"
#include "../TextReader/TextReader.hpp"

#include "correction.h"
#include "TRandom3.h"
#include <cmath>
#include <iostream>
#include <filesystem>


using correction::CorrectionSet;

SSBCorrections::SSBCorrections(TextReader* reader) {
    std::cout << "TextReader in SSBCorrections ! " << std::endl;
    std::cout << "Current directory: " << std::filesystem::current_path() << std::endl;
    reader->PrintoutVariables();
    //std::string jsonDir = std::filesystem::current_path()+"./jsonpog-integration/POG/";
    std::string jsonDir = std::filesystem::current_path().string() + "/jsonpog-integration/POG/";

    // Load correction file paths from config
    std::string jec_path     = reader->GetText("JECPath");
    std::string jer_path     = reader->GetText("JERPath");
    std::string jer_sf_path  = reader->GetText("JERSFPath");
    std::string muon_path    = reader->GetText("MuonSFPath");
    std::cout << "jsonDir+jec_path : " << (jsonDir+jec_path) << std::endl;
    // Load JEC
    std::cout << "before jec " << std::endl;
    auto jec_set = CorrectionSet::from_file(jsonDir+jec_path);
    //jec_ = jec_set->at("Summer19UL16APV_V7_MC_L1L2L3Res_AK4PFchs");
    jec_ = jec_set->compound().at("Summer19UL16APV_V7_MC_L1L2L3Res_AK4PFchs");
    
    std::cout << "done jec" << std::endl;
    // Load JER resolution
    auto jer_set = CorrectionSet::from_file(jsonDir+jer_path);
    jer_ = jer_set->at("Summer20UL16APV_JRV3_MC_PtResolution_AK4PFchs");
    std::cout << "done jer_set" << std::endl;

    //  Load JER scale factor (JERSF)
    auto jer_sf_set = CorrectionSet::from_file(jsonDir+jer_sf_path);
    jer_sf_ = jer_sf_set->at("Summer20UL16APV_JRV3_MC_ScaleFactor_AK4PFchs");
    std::cout << "done jer_sf_set" << std::endl;

    // Load muon SF
    //auto muon_set = CorrectionSet::from_file(jsonDir+muon_path);
    //muon_id_sf_  = muon_set->at("NUM_TightID_DEN_TrackerMuons_abseta_pt");
    //muon_iso_sf_ = muon_set->at("NUM_TightRelIso_DEN_TightIDandIPCut_abseta_pt");
}

double SSBCorrections::GetJEC(double eta, double pt, double rho) const {
    return jec_->evaluate({"2017", "AK4PFchs", "MC", eta, pt, rho});// example//
}

double SSBCorrections::GetJER(double eta, double pt) const {
    return jer_->evaluate({eta, pt});
}

double SSBCorrections::SmearJER(double reco_pt, double gen_pt, double eta, double rho, const std::string& jer_tag) const {
    // Get JER scale factor for this eta and variation (nominal/up/down)
    double sf = jer_sf_->evaluate({jer_tag, eta});

    // Get JER resolution
    double resolution = jer_->evaluate({eta, reco_pt, rho});

    // Case 1: valid genJet match → deterministic smearing
    if (gen_pt > 0.0) {
        double delta_pt = reco_pt - gen_pt;
        double smeared_pt = gen_pt + sf * delta_pt;
        return std::max(0.0, smeared_pt);
    }

    // Case 2: no genJet match → stochastic smearing (hybrid)
    double sigma = resolution;
    double smear_factor = 1.0 + std::sqrt(std::max(sf * sf - 1.0, 0.0)) * gRandom->Gaus(0.0, 1.0);
    return std::max(0.0, reco_pt * smear_factor);
}

double SSBCorrections::GetMuonIDSF(double eta, double pt) const {
    return 1;//muon_id_sf_->evaluate({"nominal", std::abs(eta), pt});
}

double SSBCorrections::GetMuonIsoSF(double eta, double pt) const {
    return 1;//muon_iso_sf_->evaluate({"nominal", std::abs(eta), pt});
}

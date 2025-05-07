#include "../interface/SSBCorrections.h"
#include "../TextReader/TextReader.hpp"

#include "correction.h"
#include "TRandom3.h"
#include <cmath>
#include <iostream>
#include <filesystem>
#include <vector>
#include "TLorentzVector.h"

SSBCorrections::SSBCorrections(TextReader* reader) {
    std::cout << "TextReader in SSBCorrections ! " << std::endl;
    std::cout << "Current directory: " << std::filesystem::current_path() << std::endl;
    reader->PrintoutVariables();
    std::string jsonDir = std::filesystem::current_path().string() + "/jsonpog-integration/POG/";

    std::string jec_path     = reader->GetText("JECPath");
    std::string jer_path     = reader->GetText("JERPath");
    std::string jer_sf_path  = reader->GetText("JERSFPath");
    std::string muon_path    = reader->GetText("MuonSFPath");
    std::string elec_path    = reader->GetText("ElecSFPath");
    std::string RunPeriod    = reader->GetText("RunRange");

    if (RunPeriod.find("2016PreVFP") != std::string::npos) {
        year_ = "2016preVFP";
    } else if (RunPeriod.find("2016PostVFP") != std::string::npos) {
        year_ = "2016postVFP";
    } else if (RunPeriod.find("2017") != std::string::npos) {
        year_ = "2017";
    } else if (RunPeriod.find("2018") != std::string::npos) {
        year_ = "2018";
    } 
    else {
        std::cerr << "[SSBCorrections] Unknown RunPeriod: " << RunPeriod << std::endl;
        year_ = "2018"; // fallback or throw error
    }


    auto jec_set = correction::CorrectionSet::from_file(jsonDir + jec_path);
    jec_ = jec_set->compound().at("Summer19UL16APV_V7_MC_L1L2L3Res_AK4PFchs");

    auto jer_set = correction::CorrectionSet::from_file(jsonDir + jer_path);
    jer_ = jer_set->at("Summer20UL16APV_JRV3_MC_PtResolution_AK4PFchs");

    auto jer_sf_set = correction::CorrectionSet::from_file(jsonDir + jer_sf_path);
    jer_sf_ = jer_sf_set->at("Summer20UL16APV_JRV3_MC_ScaleFactor_AK4PFchs");

    // Load muon SF
    //auto muon_set = CorrectionSet::from_file(jsonDir+muon_path);
    std::cout << "jsonDir+muon_path " << jsonDir+muon_path << std::endl; 
    //muon_id_sf_  = muon_set->at("NUM_TightID_DEN_TrackerMuons_abseta_pt");
    //muon_iso_sf_ = muon_set->at("NUM_TightRelIso_DEN_TightIDandIPCut_abseta_pt");
    //muon_reco_ = muon_set->at("NUM_TrackerMuons_DEN_genTracks");
    //muon_id_   = muon_set->at("NUM_TightID_DEN_TrackerMuons");
    //muon_iso_  = muon_set->at("NUM_TightRelIso_DEN_TightID");

    auto muon_set = correction::CorrectionSet::from_file(jsonDir + muon_path);

    //muon_reco_ = muon_set->at("NUM_TightID_DEN_TrackerMuons");
    muon_id_   = muon_set->at("NUM_TightID_DEN_TrackerMuons");
    muon_iso_  = muon_set->at("NUM_TightRelIso_DEN_TightIDandIPCut");

    //std::string path = "jsonpog-integration/POG/EGM/" + year + "_UL/electron.json.gz";
    std::cout << "electron SF path : " << jsonDir + elec_path << std::endl;
    year_ = "2016preVFP";
    //auto cset = correction::CorrectionSet::from_file(jsonDir + elec_path);
    //ele_id_sf_ = cset->at("Electron-ID-SF");
    //ele_reco_sf_ = cset->at("Electron-Reco-SF");
    //year_ = "2016";
}

double SSBCorrections::GetCorrectedJetPt(double raw_pt, double eta, double area) const {
    double sf = jec_->evaluate({eta, raw_pt, area});
    return raw_pt * sf;
}

double SSBCorrections::GetCorrectedJetMass(double raw_mass, double raw_pt, double eta, double area) const {
    double sf = jec_->evaluate({eta, raw_pt, area});
    return raw_mass * sf;
}

double SSBCorrections::GetJER(double eta, double pt) const {
    return jer_->evaluate({eta, pt});
}

double SSBCorrections::SmearJER(double reco_pt, double gen_pt, double eta, double rho, const std::string& jer_tag) const {
    double sf = jer_sf_->evaluate({eta, jer_tag});
    double resolution = jer_->evaluate({eta, reco_pt, rho});

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
    TLorentzVector rawMET;
    rawMET.SetPtEtaPhiM(raw_met_pt, 0, raw_met_phi, 0);

    TLorentzVector correctionSum;
    for (size_t i = 0; i < rawJets.size(); ++i) {
        correctionSum += (rawJets[i] - corrJets[i]);
    }
    TLorentzVector correctedMET = rawMET + correctionSum;
    return correctedMET;
}

TLorentzVector SSBCorrections::BuildCorrectedJetWithJER(double raw_pt, double raw_mass, double eta, double phi, double area, double rho, double gen_pt, const std::string& jer_tag) const {
    double pt_corr = GetCorrectedJetPt(raw_pt, eta, area);
    double mass_corr = GetCorrectedJetMass(raw_mass, raw_pt, eta, area);
    double pt_corr_smear = SmearJER(pt_corr, gen_pt, eta, rho, jer_tag);

    TLorentzVector jet;
    jet.SetPtEtaPhiM(pt_corr_smear, eta, phi, mass_corr * (pt_corr_smear / pt_corr));
    return jet;
}

double SSBCorrections::GetMuonRecoSF(double pt, double eta) const {
    //auto val = muon_reco_->evaluate({pt, eta});
    std::variant<double, std::vector<double>> val = muon_reco_->evaluate({pt, eta});
    //return std::get<double>(val);
    return std::get<double>(val);
}

double SSBCorrections::GetMuonIDSF(double pt, double eta, const std::string& syst_tag) const {
    //auto val = muon_id_->evaluate({eta, pt, syst_tag});
    std::variant<double, std::vector<double>> val = muon_id_->evaluate({eta, pt, syst_tag});
    return std::get<double>(val);
}

double SSBCorrections::GetMuonIsoSF(double pt, double eta, const std::string& syst_tag) const {
    //auto val = muon_iso_->evaluate({eta, pt, syst_tag});
    std::variant<double, std::vector<double>> val = muon_iso_->evaluate({eta, pt, syst_tag});
    return std::get<double>(val);
}


double SSBCorrections::DoubleMuon_IDIsoEff(TLorentzVector lep1, TLorentzVector lep2,
                                           TString muidsys, TString muisosys, TString tracksys) const {
    float pt1 = std::min(lep1.Pt(), 119.999);
    float pt2 = std::min(lep2.Pt(), 119.999);
    float abseta1 = std::abs(lep1.Eta());
    float abseta2 = std::abs(lep2.Eta());

    std::cout << "muidsys : " << muidsys << std::endl;
    std::cout << "muisosys : " << muisosys << std::endl;

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

float SSBCorrections::GetElectronIDSF(float pt, float eta, const std::string& wp) const {
    if (!ele_id_sf_) {
        std::cerr << "[GetElectronIDSF] SF not loaded! Call LoadElectronSF() first.\n";
        return 1.0;
    }

    //float id_sf = ele_id_sf_->evaluate({ year_, wp, "nominal", eta, pt });
    //float reco_sf = ele_reco_sf_->evaluate({ year_, "nominal", eta, pt });
    return ele_id_sf_->evaluate({ year_, wp, "nominal", eta, pt });;
}

float SSBCorrections::GetElectronRecoSF(float pt, float eta) const {
    if (!ele_reco_sf_) {
        std::cerr << "[GetElectronRecoSF] SF not loaded! Call LoadElectronSF() first.\n";
        return 1.0;
    }

    //float id_sf = ele_id_sf_->evaluate({ year_, wp, "nominal", eta, pt });
    //float reco_sf = ele_reco_sf_->evaluate({ year_, "nominal", eta, pt });
    return ele_reco_sf_->evaluate({ year_, "nominal", eta, pt });
}

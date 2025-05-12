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

    std::string puw_path     = reader->GetText("PUWeightPath");
    std::string jec_path     = reader->GetText("JECPath");
    std::string jer_path     = reader->GetText("JERPath");
    std::string jer_sf_path  = reader->GetText("JERSFPath");
    std::string muon_path    = reader->GetText("MuonSFPath");
    std::string elec_path    = reader->GetText("ElecSFPath");
    std::string RunPeriod    = reader->GetText("RunRange");
    std::string puJson  =  "";

    // Muon Infor. ID  ISO // 
    std::string muon_id_corName  = reader->GetText( "MuonIDSFName"  );
    std::string muon_iso_corName = reader->GetText( "MuonIsoSFName" );
    
    // Electron ID ISO // 
    std::string ele_sf_name_      = reader->GetText( "ElecIDSFName"  );  
    std::string ele_reco_sf_name_    = reader->GetText( "ElecRecoSFName" );

    // Trigger SF // 
    std::string Trig_sf_name_      = reader->GetText( "TrigSFFile"  );  
    std::string Trig_sf_histname_  = reader->GetText( "TrigSFHist"  );  
    

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
    } 
    else {
        std::cerr << "[SSBCorrections] Unknown RunPeriod: " << RunPeriod << std::endl;
        year_ = "2018"; // fallback or throw error
    }

    std::cout << "jsonDir + puw_path : " << jsonDir + puw_path << std::endl;
    auto puset = correction::CorrectionSet::from_file(jsonDir + puw_path);
    
    pu_weight_ = puset->at(puJson);


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
    //std::string muon_id_corName = "";
    //std::string muon_iso_corName = "";

    std::cout << "muon_id_corName : " << muon_id_corName << std::endl;
    std::cout << "muon_iso_corName : " << muon_iso_corName << std::endl;
    //muon_id_   = muon_set->at("NUM_TightID_DEN_TrackerMuons");
    //muon_iso_  = muon_set->at("NUM_TightRelIso_DEN_TightIDandIPCut");

    muon_id_   = muon_set->at(muon_id_corName);
    muon_iso_  = muon_set->at(muon_iso_corName);
    //std::string path = "jsonpog-integration/POG/EGM/" + year + "_UL/electron.json.gz";
    //std::cout << "electron SF path : " << jsonDir + elec_path << std::endl;
    //year_ = "2016preVFP";
    auto cset = correction::CorrectionSet::from_file(jsonDir + elec_path);
    //std::cout << "sk ele 1 ele_sf_name_ : " << ele_sf_name_ << std::endl;
    ele_sf_ = cset->at(ele_sf_name_);
    //std::cout << "sk ele 2 ele_reco_sf_name_: " << ele_reco_sf_name_ << std::endl;
    TFile *f_trg     = new TFile(Trig_sf_name_.c_str());
    H_trig = (TH2D*) f_trg->Get(Trig_sf_histname_.c_str()); 

}
/*
double SSBCorrections::GetCorrectedJetPt(double raw_pt, double eta, double area) const {
    std::cout << "in GetCorrectedJetPt raw_pt : " << raw_pt << " eta " << eta << " area : " << area << std::endl;
    std::cout << "Inputs for JEC correction:" << std::endl;
for (const auto& input : jec_->inputs()) {
    std::cout << " - " << input.name() << std::endl;
}
    //double sf = jec_->evaluate({eta, raw_pt, area});
    double sf = jec_->evaluate({eta, raw_pt});
    std::cout << "sf in GetCorrectedJetPt : "<< sf << std::endl; 
    //double sf = jec_->evaluate({raw_pt,eta});
    return raw_pt * sf;
}
*/
double SSBCorrections::GetCorrectedJetPt(double raw_pt, double eta, double area, double rho) const {
    //std::cout << "in GetCorrectedJetPt raw_pt : " << raw_pt
    //          << " eta " << eta << " area : " << area << " rho : " << rho << std::endl;

    double sf = jec_->evaluate({area, eta, raw_pt, rho});  
    //std::cout << "sf in GetCorrectedJetPt : " << sf << std::endl;

    return raw_pt * sf;
}

double SSBCorrections::GetCorrectedJetMass(double raw_mass, double raw_pt, double eta, double area, double rho) const {
    //double sf = jec_->evaluate({eta, raw_pt, area});
    double sf = jec_->evaluate({area, eta, raw_pt, rho});
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

/*TLorentzVector SSBCorrections::BuildCorrectedJetWithJER(double raw_pt, double raw_mass, double eta, double phi, double area, double rho, double gen_pt, const std::string& jer_tag) const {
    double pt_corr = GetCorrectedJetPt(raw_pt, eta, area);
    double mass_corr = GetCorrectedJetMass(raw_mass, raw_pt, eta, area);
    double pt_corr_smear = SmearJER(pt_corr, gen_pt, eta, rho, jer_tag);

    TLorentzVector jet;
    jet.SetPtEtaPhiM(pt_corr_smear, eta, phi, mass_corr * (pt_corr_smear / pt_corr));
    return jet;
}*/

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
    if (!ele_sf_) {
        std::cerr << "[GetElectronIDSF] SF not loaded! Call LoadElectronSF() first.\n";
        return 1.0;
    }

    //float id_sf = ele_sf_->evaluate({ year_, wp, "nominal", eta, pt });
    //float reco_sf = ele_reco_sf_->evaluate({ year_, "nominal", eta, pt });
    return ele_sf_->evaluate({ year_, wp, "nominal", eta, pt });;
}

float SSBCorrections::GetElectronRecoSF(float pt, float eta) const {
    if (!ele_reco_sf_) {
        std::cerr << "[GetElectronRecoSF] SF not loaded! Call LoadElectronSF() first.\n";
        return 1.0;
    }

    //float id_sf = ele_sf_->evaluate({ year_, wp, "nominal", eta, pt });
    //float reco_sf = ele_reco_sf_->evaluate({ year_, "nominal", eta, pt });
    return ele_reco_sf_->evaluate({ year_, "nominal", eta, pt });
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

float SSBCorrections::GetElectronSF(const std::string& sf_type, float eta, float pt, const std::string& syst) const {
    std::string recoPtThreshold = pt >= 20 ? "RecoAbove20" : "RecoBelow20";
    std::string type = sf_type;

    if (sf_type == "Reco") type = recoPtThreshold;

    try {
        return ele_sf_->evaluate({year_, syst, type, eta, pt});
    } catch (const std::exception& e) {
        std::cerr << "[GetElectronSF] Failed to evaluate: " << e.what() << std::endl;
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

// Apply JES and JER corrections to jets, then recompute MET accordingly
/*JetCorrectionOutput SSBCorrections::ApplyJetCorrectionsWithMET(
    const std::vector<TLorentzVector>& rawJets,
    const std::vector<float>& rawFactors,
    const std::vector<float>& areas,
    float rho,
    bool isData,
    bool applyJES,
    bool applyJER,
    double raw_met_pt,
    double raw_met_phi,
    const std::vector<TLorentzVector>& genJets  // Gen-level jets for hybrid smearing
) const {
    std::vector<TLorentzVector> correctedJets;
    std::vector<TLorentzVector> rawJetsWithRawPt;
    correctedJets.reserve(rawJets.size());
    rawJetsWithRawPt.reserve(rawJets.size());

    for (size_t i = 0; i < rawJets.size(); ++i) {
        double eta  = rawJets[i].Eta();
        double phi  = rawJets[i].Phi();
        double mass = rawJets[i].M() * (1.0 - rawFactors[i]);
        double raw_pt = rawJets[i].Pt() * (1.0 - rawFactors[i]);

        // Apply Jet Energy Scale correction
        if (applyJES) {
            raw_pt = GetCorrectedJetPt(raw_pt, eta, areas[i]);
            mass   = GetCorrectedJetMass(mass, raw_pt, eta, areas[i]);
        }

        // Apply Jet Energy Resolution smearing (only for MC)
        if (!isData && applyJER) {
            float matched_genpt = MatchGenPt(rawJets[i], genJets, 0.2);  // ΔR < 0.2
            raw_pt = SmearJER(raw_pt, matched_genpt, eta, rho, "nominal");
        }

        // Build the corrected jet
        TLorentzVector corrected;
        corrected.SetPtEtaPhiM(raw_pt, eta, phi, mass);
        correctedJets.push_back(corrected);

        // Rebuild raw jet using raw pt for MET recomputation
        TLorentzVector raw_rebuilt;
        raw_rebuilt.SetPtEtaPhiM(rawJets[i].Pt() * (1.0 - rawFactors[i]), eta, phi, mass);
        rawJetsWithRawPt.push_back(raw_rebuilt);
    }

    // Propagate the jet corrections to MET
    TLorentzVector correctedMET = RecomputeMET(raw_met_pt, raw_met_phi, rawJetsWithRawPt, correctedJets);

    JetCorrectionOutput result;
    result.corrected_jets = correctedJets;
    result.corrected_met = correctedMET;
    return result;
}
*/

// Apply JES and JER corrections to jets, then recompute MET accordingly
/*JetCorrectionOutput SSBCorrections::ApplyJetCorrectionsWithMET(
    const std::vector<TLorentzVector>& rawJets,
    const std::vector<float>& rawFactors,
    const std::vector<float>& areas,
    float rho,
    bool isData,
    bool applyJES,
    bool applyJER,
    double raw_met_pt,
    double raw_met_phi,
    const std::vector<TLorentzVector>& genJets
) const {
    std::vector<TLorentzVector> correctedJets;
    std::vector<TLorentzVector> rawJetsRebuilt;

    correctedJets.reserve(rawJets.size());
    rawJetsRebuilt.reserve(rawJets.size());

    std::cout << "ApplyJetCorrectionsWithMET step1 " << std::endl;
    for (size_t i = 0; i < rawJets.size(); ++i) {
        double eta  = rawJets[i].Eta();
        double phi  = rawJets[i].Phi();
        std::cout << "ApplyJetCorrectionsWithMET step1-1 " << std::endl;

        // Reconstruct raw pt and mass from rawFactor
        double raw_pt   = rawJets[i].Pt()   * (1.0 - rawFactors[i]);
        double raw_mass = rawJets[i].M()    * (1.0 - rawFactors[i]);
        std::cout << "ApplyJetCorrectionsWithMET step1-2 " << std::endl;

        // Rebuild the raw jet (used for MET correction)
        TLorentzVector raw_jet;
        raw_jet.SetPtEtaPhiM(raw_pt, eta, phi, raw_mass);
        rawJetsRebuilt.push_back(raw_jet);
        std::cout << "ApplyJetCorrectionsWithMET step1-3 " << std::endl;

        double corrected_pt   = raw_pt;
        double corrected_mass = raw_mass;

        std::cout << "corrected_pt : " << corrected_pt << std::endl;
        std::cout << "corrected_mass : " << corrected_mass << std::endl;
        std::cout << "applyJES : " << applyJES << std::endl;
        // Apply Jet Energy Scale (JES) correction
        if (applyJES) {
        std::cout << "start ! ApplyJetCorrectionsWithMET step1-4-applyJES " << std::endl;
        std::cout << "raw_pt " << raw_pt << " eta " << eta << " areas " << areas[i]  << std::endl;
          
            corrected_pt   = GetCorrectedJetPt(raw_pt, eta, areas[i], rho);
        std::cout << "raw_mass " << raw_mass << std::endl;
            corrected_mass = GetCorrectedJetMass(raw_mass, raw_pt, eta, areas[i], rho);
        std::cout << "after raw_mass " << raw_mass << std::endl;
        }
        std::cout << "2-corrected_pt : " << corrected_pt << std::endl;
        std::cout << "2-corrected_mass : " << corrected_mass << std::endl;

        // Apply Jet Energy Resolution (JER) smearing — only for MC
        if (!isData && applyJER) {
            float matched_genpt = MatchGenPt(rawJets[i], genJets, 0.2);  // DeltaR matching
            corrected_pt = SmearJER(corrected_pt, matched_genpt, eta, rho, "nominal");
            std::cout << "ApplyJetCorrectionsWithMET step1-4-applyJER " << std::endl;
        }

        // Optionally scale mass to match new pt/pT ratio
        if (raw_pt > 0) {
            corrected_mass *= (corrected_pt / raw_pt);
        std::cout << "ApplyJetCorrectionsWithMET step1-4-applyJetMass " << std::endl;
        }

        // Construct final corrected jet
        TLorentzVector corr_jet;
        corr_jet.SetPtEtaPhiM(corrected_pt, eta, phi, corrected_mass);
        std::cout << "ApplyJetCorrectionsWithMET step1-5" << std::endl;
        correctedJets.push_back(corr_jet);
    }

        std::cout << "ApplyJetCorrectionsWithMET step2" << std::endl;
    // Type-1 MET correction: subtract (rawJets - correctedJets) from raw MET
    TLorentzVector correctedMET = RecomputeMET(raw_met_pt, raw_met_phi, rawJetsRebuilt, correctedJets);
        std::cout << "ApplyJetCorrectionsWithMET step3" << std::endl;

    JetCorrectionOutput result;
    result.corrected_jets = correctedJets;
    result.corrected_met = correctedMET;
    return result;
}*/

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
    const std::vector<int>& genJetIndices
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
            corrected_pt   = GetCorrectedJetPt(raw_pt, eta, areas[i], rho);
            corrected_mass = GetCorrectedJetMass(raw_mass, raw_pt, eta, areas[i], rho);
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
    result.corrected_met = correctedMET;
    return result;
}


TLorentzVector SSBCorrections::METXYCorrection(const TLorentzVector& type1_met,
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

double SSBCorrections::TrigMuElec_Eff(TLorentzVector lep1, TLorentzVector lep2, TString Sys_) {
    double pt_mu = lep1.Pt();   // Assuming lep1 is muon
    double pt_ele = lep2.Pt();  // Assuming lep2 is electron

    double leading_pt = std::max(pt_mu, pt_ele);
    double subleading_pt = std::min(pt_mu, pt_ele);

    return GetTrgEff(leading_pt, subleading_pt, Sys_);
}

double SSBCorrections::GetTrgEff(double pt1, double pt2, TString Sys_) {
    // Clamp values below the histogram max range (assumed 500)
    double max_pt = 499.999;
    pt1 = std::min(pt1, max_pt);
    pt2 = std::min(pt2, max_pt);

    int xbin = H_trig->GetXaxis()->FindBin(pt1);
    int ybin = H_trig->GetYaxis()->FindBin(pt2);

    double trgsf = H_trig->GetBinContent(xbin, ybin);
    double trgsferr = 0.0;

    if (Sys_ == "Up" || Sys_ == "up")
        trgsferr = H_trig->GetBinError(xbin, ybin);
    else if (Sys_ == "Down" || Sys_ == "down")
        trgsferr = -H_trig->GetBinError(xbin, ybin);

    return trgsf + trgsferr;
}

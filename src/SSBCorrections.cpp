#include "../interface/SSBCorrections.h"
#include "../TextReader/TextReader.hpp"

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
    std::string jer_sf_path  = reader->GetText("JERSFPath");
    std::string jec_name     = reader->GetText("JECName");
    std::string jer_name     = reader->GetText("JERName");
    std::string jer_res_name = reader->GetText("JERResName");
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
    } 
    else {
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
    


    std::cout << "jsonDir + puw_path : " << jsonDir + puw_path << std::endl;
    auto puset = correction::CorrectionSet::from_file(jsonDir + puw_path);
    
    pu_weight_ = puset->at(puJson);


    auto jec_set = correction::CorrectionSet::from_file(jsonDir + jec_path);
    jec_ = jec_set->compound().at(jec_name);

    auto jer_set = correction::CorrectionSet::from_file(jsonDir + jer_path);
    jer_ = jer_set->at(jer_res_name);

    auto jer_sf_set = correction::CorrectionSet::from_file(jsonDir + jer_sf_path);
    jer_sf_ = jer_sf_set->at(jer_name);

    auto jmar_sf_set = correction::CorrectionSet::from_file(jsonDir + jer_sf_path);
    pujetid_sf_ = jmar_sf_set->at(jer_name);

    // Load muon SF
    //auto muon_set = CorrectionSet::from_file(jsonDir+muon_path);
    std::cout << "jsonDir+muon_path " << jsonDir+muon_path << std::endl; 
    auto muon_set = correction::CorrectionSet::from_file(jsonDir + muon_path);

    std::cout << "muon_id_corName : " << muon_id_corName << std::endl;
    std::cout << "muon_iso_corName : " << muon_iso_corName << std::endl;

    muon_id_   = muon_set->at(muon_id_corName);
    muon_iso_  = muon_set->at(muon_iso_corName);

    auto cset = correction::CorrectionSet::from_file(jsonDir + elec_path);
    //std::cout << "sk ele 1 ele_sf_name_ : " << ele_sf_name_ << std::endl;
    ele_sf_ = cset->at(ele_sf_name_);
    //std::cout << "sk ele 2 ele_reco_sf_name_: " << ele_reco_sf_name_ << std::endl;
    std::string TrigSFPath = std::filesystem::current_path().string() + "/CorrectionFiles/Trig/";
    TFile *f_trg     = new TFile((TrigSFPath+Trig_sf_name_).c_str());
    H_trig = (TH2D*) f_trg->Get(Trig_sf_histname_.c_str()); 
}

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
/*
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
*/
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

float SSBCorrections::GetPUJetIDSF(float pt, float eta, bool passPU, bool genMatched, const std::string& wp, const std::string& syst) const {
    if (!pujetid_sf_) {
        std::cerr << "[SSBCorrections::GetPUJetIDSF] PUJetID correction not loaded." << std::endl;
        return 1.0;
    }
    if (wp.empty()) {
        std::cerr << "[SSBCorrections::GetPUJetIDSF] PUJetID working point not loaded. check out wp in PU jet id!!" << std::endl;// wp L, M, T (Loose, Medium, Tight)
        return 1.0;
    }
    if (pt >= 50.0 || !passPU || !genMatched) return 1.0;

    try {
        std::variant<double, std::vector<double>> val = pujetid_sf_->evaluate({eta, pt, syst, "L"});
        return std::get<double>(val);
    } catch (const std::exception& e) {
        std::cerr << "[SSBCorrections::GetPUJetIDSF] Evaluation failed: " << e.what() << std::endl;
        return 1.0;
    }
}

float SSBCorrections::GetEventPUJetIDWeight(
    const std::vector<TLorentzVector>& jets,
    const std::vector<bool>& passPUJetID,
    const std::vector<bool>& genMatched,
    const std::vector<float>& jetPt,
    const std::vector<float>& jetEta,
    const std::string& wp,
    const std::string& syst
) const {
    if (jets.size() != passPUJetID.size() ||
        jets.size() != genMatched.size() ||
        jets.size() != jetPt.size() ||
        jets.size() != jetEta.size()) {
        std::cerr << "[SSBCorrections::GetEventPUJetIDWeight] Vector size mismatch!" << std::endl;
        return 1.0;
    }

    float weight = 1.0;

    for (size_t i = 0; i < jets.size(); ++i) {
        float pt  = jetPt[i];
        float eta = jetEta[i];

        // Requirement : pt < 50 && passPUJetID && genMatched
        float sf = GetPUJetIDSF(pt, eta, passPUJetID[i], genMatched[i], wp, syst);
        weight *= sf;
    }

    return weight;
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
    std::string suffix = "_L1L2L3Res_AK4PFchs";

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
    }

    // Add version and data/MC tag
    expanded += "_V" + version + "_";
    expanded += is_data ? "DATA" : "MC";
    expanded += suffix;

    return expanded;
}

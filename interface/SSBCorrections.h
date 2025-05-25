#ifndef SSBCORRECTIONS_H
#define SSBCORRECTIONS_H

#include <memory>
#include <string>
#include <correction.h>
#include "TLorentzVector.h"
#include <variant> 
#include <TFile.h>
#include <TH2D.h>
#include <TString.h>


using correction::CorrectionSet;
using correction::CompoundCorrection;
// Forward declarations
class TextReader;

struct JetCorrectionOutput {
    std::vector<TLorentzVector> corrected_jets;
    TLorentzVector corrected_met;
};

// A utility class for loading and applying correctionlib-based
// JEC, JER, and muon scale factors using configuration
class SSBCorrections {
public:
    // Constructor using a configuration reader (TextReader)
    explicit SSBCorrections(TextReader* reader, const std::string inputfileName);
    //std::string ExpandJECName(const std::string& base_jec_name, const std::string& era, bool is_data);
    std::string ExpandJECName(const std::string& base_jec_name, const std::string runPeriod, const std::string& era, bool is_data);
    // Get PU weight for a given nTrueInt and variation
    float GetPUWeight(float nTrueInt, const std::string& variation = "nominal") const;


    // Jet Energy Correction (JEC) factor
    double GetJEC(double eta, double pt, double rho) const;

    // Jet Energy Resolution (JER) sigma
    double GetJER(double eta, double pt) const;

    // Smear JER for a MC jet using hybrid method (with optional JER systematic variation)
    double SmearJER(double reco_pt, double gen_pt, double eta, double rho, const std::string& jer_tag = "nominal") const;
/*    JetCorrectionOutput ApplyJetCorrections(
    const std::vector<TLorentzVector>& raw_jets,
    const TLorentzVector& original_met,
    const std::vector<float>& rawFactors,
    const std::vector<float>& etas,
    const std::vector<float>& areas,
    float rho,
    bool isData,
    bool applyJES,
    bool applyJER) const;*/

    // Reco/gen matching for JER hybrid smearing
    float MatchGenPt(const TLorentzVector& reco_jet,
                     const std::vector<TLorentzVector>& gen_jets,
                     float maxDR = 0.2) const;

    // Apply JES/JER corrections to jets and propagate to MET

    JetCorrectionOutput ApplyJetCorrectionsWithMET(
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
    ) const;

    /*// Muon ID scale factor
    double GetMuonIDSF(double eta, double pt) const;

    // Muon isolation scale factor
    double GetMuonIsoSF(double eta, double pt) const;*/
    double GetMuonRecoSF(double pt, double eta) const;
    double GetMuonIDSF(double pt, double eta, const std::string& tag = "nominal") const;
    double GetMuonIsoSF(double pt, double eta, const std::string& tag = "nominal") const;

    double DoubleMuon_IDIsoEff(TLorentzVector lep1, TLorentzVector lep2, TString muidsys, TString muisosys, TString tracksys) const;

    float GetElectronIDSF(float pt, float eta, const std::string& wp = "wp80") const;
    float GetElectronRecoSF(float pt, float eta) const;
    // Electron scale factor accessor
    float GetElectronSF(const std::string& sf_type, float eta, float pt, const std::string& syst = "sf") const;

    double TrigDiMuon_Eff(TLorentzVector lep1, TLorentzVector lep2, TString Sys_ = "nominal");
    double TrigDiElec_Eff(TLorentzVector lep1, TLorentzVector lep2, TString Sys_ = "nominal");
    double TrigMuElec_Eff(TLorentzVector lep1, TLorentzVector lep2, TString Sys_ = "nominal");

    TLorentzVector RecomputeMET(double raw_met_pt, double raw_met_phi,
                                const std::vector<TLorentzVector>& rawJets,
                                const std::vector<TLorentzVector>& corrJets) const;

/*    TLorentzVector BuildCorrectedJetWithJER(double raw_pt, double raw_mass, double eta, double phi,
                                            double area, double rho, double gen_pt, const std::string& jer_tag) const;*/

//    double GetCorrectedJetPt(double raw_pt, double eta, double area) const;
    double GetCorrectedJetPt(double raw_pt, double eta, double area, double rho) const;
    double GetCorrectedJetMass(double raw_mass, double raw_pt, double eta, double area, double rho) const;
    TLorentzVector GetCorrectedJet(const TLorentzVector& raw_jet,
                                float rawFactor, float eta, float area, float rho,
                                int jet_index, int seed, bool isData,
                                bool applyJES, bool applyJER) const;



    TLorentzVector METXYCorrection(const TLorentzVector& type1_met,
                                const std::string& era,
                                bool isData,
                                int npv) const;


private:
    //std::shared_ptr<const correction::Correction> jec_;
    //std::shared_ptr<correction::CompoundCorrection::Ref> jec_;
    //correction::CompoundCorrection::Ref jec_;
    //std::unique_ptr<correction::CorrectionSet> jec_;
    //std::shared_ptr<const correction::Correction> jec_;
    //std::shared_ptr<const correction::CorrectionSet> c_jec_;
    std::string year_;
    double GetTrgEff(double pt1, double pt2, TString Sys_);
    TH2D* H_trig;

    std::shared_ptr<const correction::Correction> pu_weight_;

    std::shared_ptr<const correction::Correction> muon_id_sf_;
    std::shared_ptr<const correction::Correction> muon_iso_sf_;
    std::shared_ptr<const correction::Correction> muon_reco_;
    std::shared_ptr<const correction::Correction> muon_id_;
    std::shared_ptr<const correction::Correction> muon_iso_;

    std::shared_ptr<const correction::Correction> ele_sf_;
    std::shared_ptr<const correction::Correction> ele_reco_sf_;

    std::shared_ptr<const correction::CompoundCorrection> jec_;
    std::shared_ptr<const correction::Correction> jer_;
    std::shared_ptr<const correction::Correction> jer_sf_;  // JER scale factor

    std::shared_ptr<const correction::Correction> metphi_corr_;
};

#endif  // SSBCORRECTIONS_H


#ifndef SSBCORRECTIONS_H
#define SSBCORRECTIONS_H

#include <memory>
#include <string>
#include <correction.h>
#include "TLorentzVector.h"
#include <variant> 


using correction::CorrectionSet;
using correction::CompoundCorrection;
// Forward declarations
class TextReader;
/*
namespace correction {
    class Correction;
}
*/
// A utility class for loading and applying correctionlib-based
// JEC, JER, and muon scale factors using configuration
class SSBCorrections {
public:
    // Constructor using a configuration reader (TextReader)
    explicit SSBCorrections(TextReader* reader);

    // Get PU weight for a given nTrueInt and variation
    float GetPUWeight(float nTrueInt, const std::string& variation = "nominal") const;


    // Jet Energy Correction (JEC) factor
    double GetJEC(double eta, double pt, double rho) const;

    // Jet Energy Resolution (JER) sigma
    double GetJER(double eta, double pt) const;

    // Smear JER for a MC jet using hybrid method (with optional JER systematic variation)
    double SmearJER(double reco_pt, double gen_pt, double eta, double rho, const std::string& jer_tag = "nominal") const;

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



    TLorentzVector RecomputeMET(double raw_met_pt, double raw_met_phi,
                                const std::vector<TLorentzVector>& rawJets,
                                const std::vector<TLorentzVector>& corrJets) const;

    TLorentzVector BuildCorrectedJetWithJER(double raw_pt, double raw_mass, double eta, double phi,
                                            double area, double rho, double gen_pt, const std::string& jer_tag) const;

    double GetCorrectedJetPt(double raw_pt, double eta, double area) const;
    double GetCorrectedJetMass(double raw_mass, double raw_pt, double eta, double area) const;

private:
    //std::shared_ptr<const correction::Correction> jec_;
    //std::shared_ptr<correction::CompoundCorrection::Ref> jec_;
    //correction::CompoundCorrection::Ref jec_;
    //std::unique_ptr<correction::CorrectionSet> jec_;
    //std::shared_ptr<const correction::Correction> jec_;
    //std::shared_ptr<const correction::CorrectionSet> c_jec_;
    std::string year_;

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
};

#endif  // SSBCORRECTIONS_H


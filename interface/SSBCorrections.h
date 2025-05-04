#ifndef SSBCORRECTIONS_H
#define SSBCORRECTIONS_H

#include <memory>
#include <string>
#include <correction.h>


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

    // Jet Energy Correction (JEC) factor
    double GetJEC(double eta, double pt, double rho) const;

    // Jet Energy Resolution (JER) sigma
    double GetJER(double eta, double pt) const;

    // Smear JER for a MC jet using hybrid method (with optional JER systematic variation)
    double SmearJER(double reco_pt, double gen_pt, double eta, double rho, const std::string& jer_tag = "nominal") const;

    // Muon ID scale factor
    double GetMuonIDSF(double eta, double pt) const;

    // Muon isolation scale factor
    double GetMuonIsoSF(double eta, double pt) const;

private:
    //std::shared_ptr<const correction::Correction> jec_;
    correction::CompoundCorrection::Ref jec_;
    //std::unique_ptr<correction::CorrectionSet> jec_;
    //std::shared_ptr<const correction::Correction> jec_;
    std::shared_ptr<const correction::CorrectionSet> c_jec_;
    std::shared_ptr<const correction::Correction> jer_;
    std::shared_ptr<const correction::Correction> jer_sf_;  // JER scale factor
    std::shared_ptr<const correction::Correction> muon_id_sf_;
    std::shared_ptr<const correction::Correction> muon_iso_sf_;
};

#endif  // SSBCORRECTIONS_H

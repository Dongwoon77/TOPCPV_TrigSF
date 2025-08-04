#ifndef XYMETCORRECTION_H
#define XYMETCORRECTION_H

#include "TString.h"
#include "TMath.h"
#include <utility>

std::pair<double,double> METXYCorr_Met_MetPhi(double uncormet, double uncormet_phi,
    int runnb, TString year, bool isMC, int npv, bool isUL = false, bool ispuppi = false);

#endif

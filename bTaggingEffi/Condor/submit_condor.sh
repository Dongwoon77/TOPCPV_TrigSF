#!/bin/bash

if [ "$1" == "16pre" ]; then
    samplelist=( 
        ## 2016PreVFP MC ##
        "TTbar_Signal"
        "DYJetsToLL_M_10To50"
        "DYJetsToLL_M_50"
        "ST_s-channel_4f_leptonDecays"
        "ST_t-channel_antitop_4f_InclusiveDecays"
        "ST_t-channel_top_4f_InclusiveDecays"
        "ST_tW_antitop_5f_NoFullyHadronicDecays"
        "ST_tW_top_5f_NoFullyHadronicDecays"
        "TTbar_AllHadronic"
        "TTbar_SemiLeptonic"
        "TTWJetsToLNu"
        "TTWJetsToQQ"
        "TTZToLLNuNu"
        "TTZToQQ"
        "WJetsToLNu"
        "WW"
        "WZ"
        "ZZ"
    )
    runPeriod="UL2016PreVFP/MC"
elif [ "$1" == "16post" ]; then
    samplelist=( 
        ## 2016 PostVFP MC ##
        #"TTbar_Signal"
        #"DYJetsToLL_M_10To50"
        #"DYJetsToLL_M_50"
        "ST_s-channel_4f_leptonDecays"
        #"ST_t-channel_antitop_4f_InclusiveDecays"
        #"ST_t-channel_top_4f_InclusiveDecays"
        #"ST_tW_antitop_5f_NoFullyHadronicDecays"
        "ST_tW_top_5f_NoFullyHadronicDecays"
        #"TTbar_AllHadronic"
        #"TTbar_SemiLeptonic"
        "TTWJetsToLNu"
        "TTWJetsToQQ"
        "TTZToLLNuNu"
        #"TTZToQQ"
        #"WJetsToLNu"
        #"WW"
        #"WZ"
        #"ZZ"
    )
    runPeriod="UL2016PostVFP/MC"
elif [ "$1" == "2017" ]; then
    samplelist=( 
        ## 2017 MC ##
        #"TTbar_Signal" 
        "DYJetsToLL_M_10to50" 
        #"DYJetsToLL_M_50" 
        #"ST_s-channel_4f_leptonDecays" 
        #"ST_t-channel_antitop_4f_InclusiveDecays" 
        #"ST_t-channel_top_4f_InclusiveDecays" 
        #"ST_tW_antitop_5f_NoFullyHadronicDecays" 
        #"ST_tW_top_5f_NoFullyHadronicDecays" 
        #"TTbar_AllHadronic"  
        #"TTbar_SemiLeptonic" 
        #"TTWJetsToLNu" 
        #"TTWJetsToQQ" 
        #"TTZToLLNuNu_M-10" 
        #"TTZToQQ_13TeV_amcatlno" 
        #"TTZToQQ_amcatnlo" 
        #"WJetsToLNu" 
        #"WW" 
        #"WZ" 
        #"ZZ" 
    )
    runPeriod="UL2017/MC"
elif [ "$1" == "2018" ]; then
    samplelist=( 
        ## 2018 MC ##
        "TTbar_Signal" 
        "DYJetsToLL_M_10To50" 
        "DYJetsToLL_M_50" 
        "ST_s-channel_4f_leptonDecays" 
        "ST_t-channel_antitop_4f_InclusiveDecays" 
        "ST_t-channel_top_4f_InclusiveDecays" 
        "ST_tW_antitop_5f_NoFullyHadronicDecays" 
        "ST_tW_top_5f_NoFullyHadronicDecays" 
        "TTbar_AllHadronic" 
        "TTbar_SemiLeptonic" 
        "TTWJetsToLNu" 
        "TTWJetsToQQ" 
        "TTZToLLNuNu" 
        "TTZToQQ" 
        "WJetsToLNu" 
        "WW" 
        "WZ" 
        "ZZ" 
    )
    runPeriod="UL2018/MC"
elif [ "$1" == "2024" ]; then
    samplelist=( 
        ## 2024 MC (Run3) ##
        "DYto2E-2Jets_Bin-MLL-10to50"
        "DYto2E-2Jets_Bin-MLL-50"
        "DYto2E-4Jets_Bin-MLL-10to50"
        "DYto2E-4Jets_Bin-MLL-50"
        "DYto2Mu-2Jets_Bin-MLL-10to50"
        "DYto2Mu-2Jets_Bin-MLL-50"
        "DYto2Mu-4Jets_Bin-MLL-10to50_TuneCP5_13p6TeV_madgraphMLM-pythia8"
        "DYto2Mu-4Jets_Bin-MLL-50_TuneCP5_13p6TeV_madgraphMLM-pythia8"
        "TTbar_Hadronic"
        "TTbar_SemiLeptonic"
        "TTbar_Signal"
        "TWminusto2L2Nu"
        "TWminusto4Q"
        "TWminustoLNu2Q"
        "TbarWplusto2L2Nu"
        "TbarWplusto4Q"
        "TbarWplustoLNu2Q"
        "WW"
        "WZ"
        "ZZ"
    )
    runPeriod="2024"
    version="1_bTagEffforRun3"
else
    echo "Invalid argument: $1"
    echo "Usage: ./submit_condor.sh [16pre|16post|2017|2018|2024] [re]"
    exit 1
fi

#######################################
# ? < script usage >                  #
# * ./submit_condor.sh [type] [re]    #
# ! type: 16pre, 16post, 2017, 2018, 2024 #
# ! re: re-submit jobs if you need    #
#######################################
### version information ###
Channels="MuMu"
if [ -z "${version}" ]; then
    version="1_bTagEffforRun2"
fi

### submit jobs for each sample ###
for sample in "${samplelist[@]}";
do
    sample_path="./Job_Version/${version}/${Channels}/${runPeriod}"
    cd ${sample_path}/${sample}
    # select the script to run depending on whether it is a re-submission
    if [ "$2" == "re" ]; then
        condor_submit "recondor_sub.sub"
    else
        condor_submit "condor_sub.sub"
    fi
    echo "submitted ${sample} jobs!!"
    cd -
done

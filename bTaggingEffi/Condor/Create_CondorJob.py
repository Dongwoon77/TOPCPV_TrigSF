import os
import sys

# 첫 번째 인자를 Type으로 설정
year = sys.argv[1]

samplelist = [
    [   ## 2016 PreVFP MC ##
        #["TTbar_Signal", "33"],
        #["DYJetsToLL_M_10To50", "23"],
        #["DYJetsToLL_M_50", "62"],
        #["ST_s-channel_4f_leptonDecays", "12"],
        #["ST_t-channel_antitop_4f_InclusiveDecays", "46"],
        #["ST_t-channel_top_4f_InclusiveDecays", "49"],
        #["ST_tW_antitop_5f_NoFullyHadronicDecays", "12"],
        #["ST_tW_top_5f_NoFullyHadronicDecays", "4"],
        #["TTbar_AllHadronic", "79"],
        #["TTbar_SemiLeptonic", "117"],
        #["TTWJetsToLNu", "26"],
        #["TTWJetsToQQ", "7"],
        #["TTZToLLNuNu", "15"],
        #["TTZToQQ", "293"],
        #["WJetsToLNu", "59"],
        #["WW", "22"],
        #["WZ", "13"],
        #["ZZ", "8"],
    ],
    [   ## 2016 PostVFP MC ##
        #["TTbar_Signal", "49"],
        #["DYJetsToLL_M_10To50", "25"],
        #["DYJetsToLL_M_50", "41"],
        #["ST_s-channel_4f_leptonDecays", "19"],
        #["ST_t-channel_antitop_4f_InclusiveDecays", "35"],
        #["ST_t-channel_top_4f_InclusiveDecays", "86"],
        #["ST_tW_antitop_5f_NoFullyHadronicDecays", "16"],
        #["ST_tW_top_5f_NoFullyHadronicDecays", "11"],
        #["TTbar_AllHadronic", "146"],
        #["TTbar_SemiLeptonic", "138"],
        #["TTWJetsToLNu", "12"],
        #["TTWJetsToQQ", "8"],
        #["TTZToLLNuNu", "42"],
        #["TTZToQQ", "224"],
        #["WJetsToLNu", "28"],
        #["WW", "41"],
        #["WZ", "16"],
        #["ZZ", "17"],
    ],
    [   ## 2017 MC ##
        #["TTbar_Signal", "99"],
        #["DYJetsToLL_M_10to50", "41"],
        #["DYJetsToLL_M_50", "153"],
        #["ST_s-channel_4f_leptonDecays", "16"],
        #["ST_t-channel_antitop_4f_InclusiveDecays", "60"],
        #["ST_t-channel_top_4f_InclusiveDecays", "197"],
        #["ST_tW_antitop_5f_NoFullyHadronicDecays", "9"],
        #["ST_tW_top_5f_NoFullyHadronicDecays", "20"],
        #["TTbar_AllHadronic", "199"],
        #["TTbar_SemiLeptonic", "297"],
        #["TTWJetsToLNu", "9"],
        #["TTWJetsToQQ", "10"],
        #["TTZToLLNuNu_M-10", "19"],
        #["TTZToQQ_13TeV_amcatlno", "359"],
        #["TTZToQQ_amcatnlo", "35"],
        #["WJetsToLNu", "45"],
        #["WW", "16"],
        #["WZ", "20"],
        #["ZZ", "2"],
    ],
    [   ## 2018 MC ##
        #["TTbar_Signal", "155"],
        #["DYJetsToLL_M_10To50", "49"],
        #["DYJetsToLL_M_50", "204"],
        #["ST_s-channel_4f_leptonDecays", "19"],
        #["ST_t-channel_antitop_4f_InclusiveDecays", "130"],
        #["ST_t-channel_top_4f_InclusiveDecays", "149"],
        #["ST_tW_antitop_5f_NoFullyHadronicDecays", "16"],
        #["ST_tW_top_5f_NoFullyHadronicDecays", "15"],
        #["TTbar_AllHadronic", "339"],
        #["TTbar_SemiLeptonic", "391"],
        #["TTWJetsToLNu", "13"],
        #["TTWJetsToQQ", "3"],
        #["TTZToLLNuNu", "21"],
        #["TTZToQQ", "492"],
        #["WJetsToLNu", "54"],
        #["WW", "31"],
        #["WZ", "16"],
        #["ZZ", "6"],
    ],
    [   ## 2022 MC (Run3) ##
        ["DYto2L-2Jets_MLL-50_TuneCP5_13p6TeV_amcatnloFXFX-pythia8", "1"],
        ["TTbar_Hadronic", "1"],
        ["TTbar_SemiLeptonic", "1"],
        ["TTbar_Signal", "1"],
    ],
    [   ## 2022PostEE MC (Run3) ##
        ["DYto2L-2Jets_MLL-50_TuneCP5_13p6TeV_amcatnloFXFX-pythia8_postEE", "3"],
        ["TTbar_Hadronic_postEE", "3"],
        ["TTbar_SemiLeptonic_postEE", "4"],
        ["TTbar_Signal_postEE", "2"],
    ],
    [   ## 2023 MC (Run3) ##
        ["DYto2L-2Jets_MLL-50_TuneCP5_13p6TeV_amcatnloFXFX-pythia8", "2"],
        ["TTbar_Hadronic", "2"],
        ["TTbar_SemiLeptonic", "2"],
        ["TTbar_Signal", "1"],
    ],
    [   ## 2023BPix MC (Run3) ##
        ["DYto2L-2Jets_MLL-50_TuneCP5_13p6TeV_amcatnloFXFX-pythia8_postBPix", "1"],
        ["TTbar_Hadronic_postBPix", "1"],
        ["TTbar_SemiLeptonic_postBPix", "1"],
        ["TTbar_Signal_postBPix", "1"],
    ],
    [   ## 2024 MC (Run3) ##
        ["DYto2E-2Jets_Bin-MLL-10to50", "7"],
        ["DYto2E-2Jets_Bin-MLL-50", "30"],
        ["DYto2E-4Jets_Bin-MLL-10to50", "25"],
        ["DYto2E-4Jets_Bin-MLL-50", "29"],
        ["DYto2Mu-2Jets_Bin-MLL-10to50", "11"],
        ["DYto2Mu-2Jets_Bin-MLL-50", "30"],
        ["DYto2Mu-4Jets_Bin-MLL-10to50_TuneCP5_13p6TeV_madgraphMLM-pythia8", "31"],
        ["DYto2Mu-4Jets_Bin-MLL-50_TuneCP5_13p6TeV_madgraphMLM-pythia8", "32"],
        ["TTbar_Hadronic", "8"],
        ["TTbar_SemiLeptonic", "8"],
        ["TTbar_Signal", "78"],
        ["TWminusto2L2Nu", "2"],
        ["TWminusto4Q", "2"],
        ["TWminustoLNu2Q", "3"],
        ["TbarWplusto2L2Nu", "1"],
        ["TbarWplusto4Q", "2"],
        ["TbarWplustoLNu2Q", "3"],
        ["WW", "6"],
        ["WZ", "4"],
        ["ZZ", "1"],
    ]
]
if year == "16pre":
    runPeriod = f"UL2016PreVFP/MC"
    targetList = samplelist[0]
elif year == "16post":
    runPeriod = f"UL2016PostVFP/MC"
    targetList = samplelist[1]
elif year == "2017":
    runPeriod = f"UL2017/MC"
    targetList = samplelist[2]
elif year == "2018":
    runPeriod = f"UL2018/MC"
    targetList = samplelist[3]
elif year == "2022":
    runPeriod = f"2022"
    targetList = samplelist[4]
elif year == "2022PostEE":
    runPeriod = f"2022PostEE"
    targetList = samplelist[5]
elif year == "2023":
    runPeriod = f"2023"
    targetList = samplelist[6]
elif year == "2023BPix":
    runPeriod = f"2023BPix"
    targetList = samplelist[7]
elif year == "2024":
    runPeriod = f"2024"
    targetList = samplelist[8]
else:
    print("Invalid year")
    print("Usage: python Create_CondorJob.py [16pre|16post|2017|2018|2022|2022PostEE|2023|2023BPix|2024]")
    sys.exit(1)

Channels = "MuMu"
NumEvt = -1
version = "1_bTagEffforRun3"

for sample in range(len(targetList)):
    ### set up jobversion ###
    jobversion = f"Job_Version/{version}"
    ### set up directory ###
    sample_dir = f"{jobversion}/{Channels}/{runPeriod}/{targetList[sample][0]}"
    os.makedirs(sample_dir, exist_ok=True)
    os.makedirs(f"{sample_dir}/log_condor", exist_ok=True)
    ### create log_condor directory ###
    os.makedirs(f"{sample_dir}/log_condor/err", exist_ok=True)
    os.makedirs(f"{sample_dir}/log_condor/log", exist_ok=True)
    os.makedirs(f"{sample_dir}/log_condor/out", exist_ok=True)
    ### delete log_condor directory ###
    for f in os.listdir(f"{sample_dir}/log_condor/err"):
        os.remove(os.path.join(f"{sample_dir}/log_condor/err", f))
    for f in os.listdir(f"{sample_dir}/log_condor/log"):
        os.remove(os.path.join(f"{sample_dir}/log_condor/log", f))
    for f in os.listdir(f"{sample_dir}/log_condor/out"):
        os.remove(os.path.join(f"{sample_dir}/log_condor/out", f))
    ### created executive condor job: condor_run.sh ###
    with open(f"{sample_dir}/condor_run.sh", "w") as file:
        file.write("#!/bin/bash \n\n")
        ### CMSSW environment setting ###
        file.write("export SCRAM_ARCH=el9_amd64_gcc12\n")
        file.write("source /cvmfs/cms.cern.ch/cmsset_default.sh \n\n")
        ### ROOT environment setting ###
        file.write("ROOTSYS=/cvmfs/cms.cern.ch/el9_amd64_gcc12/lcg/root/6.26.11-09a813662aa0f8eece54cb6d94bfed85\n")
        file.write("export PATH=$ROOTSYS/bin:$PATH\n")
        file.write("export ROOT_INCLUDE_PATH=$ROOTSYS/include\n")
        file.write("export LD_LIBRARY_PATH=$ROOTSYS/lib:$LD_LIBRARY_PATH\n\n")
        ### ROOT related special environment variables ###
        file.write("export ROOT_CLING_SYMLINK_BOOL=1\n")
        file.write("export CLING_STANDARD_PCH=none\n")
        file.write("export ROOT_HIST=0\n\n")
        file.write("fileListNum=$((${1}+1)) \n")
        ### CMSSW basic directory and working directory variables ###
        file.write("cd /u/user/dwkim/TOP/YU_class_TOP/NANOAOD/NANOAOD_RUN3_test_FV/CMSSW_13_3_0/src/SSBNanoAODANCode/ \n")
        file.write("cmsenv \n\n")
        ### output directory creation ###
        file.write("mkdir -p ./output/%s/\n" %(sample_dir))
        
        # bTaggingEffi 디렉토리로 이동하여 makeBtagEff_All 실행
        file.write("cd bTaggingEffi \n")
        
        # 파일 리스트 경로 설정 (run_selected_btag_eff.sh와 동일하게)
        year = runPeriod.split('/')[0]  # UL2018, UL2016PreVFP 등
        file.write("./makeBtagEff_All ./input/%s/%s/%s_${fileListNum}.list ./output/%s/%s_${fileListNum} -1\n" % (year, targetList[sample][0], targetList[sample][0], sample_dir, targetList[sample][0]))
    ### created submit condor job: condor_sub.sub ###
    with open(f"{sample_dir}/condor_sub.sub", "w") as file:
        file.write("universe = vanilla \n")
        file.write("executable = condor_run.sh \n\n")
        file.write("arguments = $(Process) \n")
        file.write("request_memory = 1024 MB \n\n")
        file.write("getenv = True \n")
        file.write("should_transfer_files = YES \n")
        file.write("when_to_transfer_output = ON_EXIT \n\n")
        file.write('Requirements = (machine =!= "dm01.knu.ac.kr") && (machine =!= "dm02.knu.ac.kr") && (TARGET.Arch == "X86_64") && (TARGET.OpSys == "LINUX") && (TARGET.HasFileTransfer)\n')
        file.write("output = /u/user/dwkim/TOP/YU_class_TOP/NANOAOD/NANOAOD_RUN3_test_FV/CMSSW_13_3_0/src/SSBNanoAODANCode/bTaggingEffi/Condor/%s/log_condor/out/out_$(Process).out \n" %(sample_dir) )
        file.write("error  = /u/user/dwkim/TOP/YU_class_TOP/NANOAOD/NANOAOD_RUN3_test_FV/CMSSW_13_3_0/src/SSBNanoAODANCode/bTaggingEffi/Condor/%s/log_condor/err/err_$(Process).err \n" %(sample_dir) )
        file.write("log    = /u/user/dwkim/TOP/YU_class_TOP/NANOAOD/NANOAOD_RUN3_test_FV/CMSSW_13_3_0/src/SSBNanoAODANCode/bTaggingEffi/Condor/%s/log_condor/log/log_$(Process).log \n\n" %(sample_dir) )
        file.write("JobBatchName = bTagEff_%s_%s \n" %(year,targetList[sample][0]) )
        file.write("queue %s" %(targetList[sample][1]) )
    print("%s condor submit file created!!! " %(targetList[sample][0]))

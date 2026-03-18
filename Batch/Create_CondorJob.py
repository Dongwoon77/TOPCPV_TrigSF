import os
import sys

# 명령줄 인자 확인
if len(sys.argv) < 2:
    print("Usage: python Create_CondorJob.py [data|MC|16]")
    sys.exit(1)

# 첫 번째 인자를 Type으로 설정
Type = sys.argv[1].lower()  # 소문자로 변환하여 대소문자 구분 없애기

# 두 번째 인자가 있으면 Channels 설정에 사용 (선택적)
Channels = "MuMu"  # 기본값
if len(sys.argv) > 2:
    Channels = sys.argv[2]

if Type == "data":
    samplelist = [
        ## 2018 Data ##
        ["DoubleMuon_Run2018A", "277"],
        ["DoubleMuon_Run2018B", "131"],
        ["DoubleMuon_Run2018C", "131"],
        ["DoubleMuon_Run2018D", "315"],
        ["EGamma_Run2018A", "278"],
        ["EGamma_Run2018B", "132"],
        ["EGamma_Run2018C", "131"],
        ["EGamma_Run2018D", "315"],
        ["SingleMuon_Run2018A", "277"],
        ["SingleMuon_Run2018B", "131"],
        ["SingleMuon_Run2018C", "131"],
        ["SingleMuon_Run2018D", "315"],
    ]
    runPeriod = "UL2018/Data"
elif Type == "mc":
    samplelist = [
        ## 2018 MC ##
        ["TTTo2L2Nu", "16"],
        #["DYJetsToLL_M-10to50_amcatnloFXFX", "5"],
        #["DYJetsToLL_M-10to50_madgraphMLM", "9"],
        #["DYJetsToLL_M-50_amcatnloFXFX", "21"],
        #["DYJetsToLL_M-50_madgraphMLM", "12"],
        #["ST_s-channel_4f_leptonDecays", "2"],
        #["ST_t-channel_antitop_4f_InclusiveDecays", "13"],
        #["ST_t-channel_top_4f_InclusiveDecays", "15"],
        #["ST_tW_antitop_5f_NoFullyHadronicDecays", "2"],
        #["ST_tW_top_5f_NoFullyHadronicDecays", "2"],
        #["TTJets_amcatnloFXFX", "36"],
        #["TTJets_madgraphMLM", "4"],
        #["TTToHadronic", "34"],
        #["TTToSemiLeptonic", "40"],
        #["TTWJetsToLNu_amcatnloFXFX", "2"],
        #["TTWJetsToQQ", "1"],
        #["TTZToLLNuNu_M-10", "3"],
        #["TTZToQQ", "50"],
        #["WJetsToLNu_amcatnloFXFX", "6"],
        #["WJetsToLNu_madgraphMLM", "47"],
        #["WW", "4"],
        #["WZ", "2"],
        #["ZZ", "1"],
    ]
    runPeriod = "UL2018/MC"
elif Type == "16":
    samplelist = [
        ## 2016PreVFP MC ##
        ["TTbar_Signal", "3"],
    ]
    runPeriod = "UL2016PreVFP/MC"
else:
    print(f"Invalid Type: {Type}")
    print("Usage: python Create_CondorJob.py [data|MC|16]")
    sys.exit(1)

Channels = "MuMu"
NumEvt = -1
version = "testCondor"

if Channels == "MuMu":
    confch = "ULSummer20/" + runPeriod.split('/')[0] + "/dimuon.config"
elif Channels == "ElEl":
    confch = "ULSummer20/" + runPeriod.split('/')[0] + "/dielec.config"
elif Channels == "MuEl":
    confch = "ULSummer20/" + runPeriod.split('/')[0] + "/muelec.config"

for sample in range(len(samplelist)):
    ### set up jobversion ###
    if "dtG" in samplelist[sample][0]:
        jobversion = f"Job_Version/{version}/CPV_Sample"
    else:
        jobversion = f"Job_Version/{version}"
    ### set up directory ###
    sample_dir = f"{jobversion}/{Channels}/{runPeriod}/{samplelist[sample][0]}"
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
        file.write("cd /u/user/gcho/TopPhysics/CPV/NanoAOD/CMSSW_13_3_0/src/SSBNanoAODANCode/ \n")
        file.write("cmsenv \n\n")
        ### output directory creation ###
        file.write("mkdir -p ./output/%s/\n" %(sample_dir))
        if "dtG" in samplelist[sample][0]:
            file.write("./ssb_analysis CEDM_Sample/%s/%s/%s_${fileListNum}.list /%s/%s_${fileListNum}.root 0 %s \n" % (runPeriod,samplelist[sample][0],samplelist[sample][0],sample_dir,samplelist[sample][0],samplelist[sample][0]))
        else:
            file.write("./ssb_analysis %s/%s/%s_${fileListNum}.list %s/%s_${fileListNum}.root %s None %s %s \n" % (runPeriod,samplelist[sample][0],samplelist[sample][0],sample_dir,samplelist[sample][0],confch,runPeriod.split('/')[0],NumEvt))
    ### created submit condor job: condor_sub.sub ###
    with open(f"{sample_dir}/condor_sub.sub", "w") as file:
        file.write("universe = vanilla \n")
        file.write("executable = condor_run.sh \n\n")
        file.write("arguments = $(Process) \n")
        ## set up memory request: data: 6144, MC: 3072, CPV: 2048
        if "Data" in samplelist[sample][0]:
            file.write("request_memory = 1024 MB \n\n") ## data samples
        elif "dtG" in samplelist[sample][0]:
            file.write("request_memory = 1024 MB \n\n") ## CPV samples
        else:
            file.write("request_memory = 1024 MB \n\n") ## MC samples
        file.write("getenv = True \n")
        file.write("should_transfer_files = YES \n")
        file.write("when_to_transfer_output = ON_EXIT \n\n")
        file.write('Requirements = (machine =!= "dm01.knu.ac.kr") && (TARGET.Arch == "X86_64") && (TARGET.OpSys == "LINUX") && (TARGET.HasFileTransfer)\n')
        file.write("output = /u/user/gcho/TopPhysics/CPV/NanoAOD/CMSSW_13_3_0/src/SSBNanoAODANCode/Condor/%s/log_condor/out/out_$(Process).out \n" %(sample_dir) )
        file.write("error  = /u/user/gcho/TopPhysics/CPV/NanoAOD/CMSSW_13_3_0/src/SSBNanoAODANCode/Condor/%s/log_condor/err/err_$(Process).err \n" %(sample_dir) )
        file.write("log    = /u/user/gcho/TopPhysics/CPV/NanoAOD/CMSSW_13_3_0/src/SSBNanoAODANCode/Condor/%s/log_condor/log/log_$(Process).log \n\n" %(sample_dir) )
        file.write("JobBatchName = %s \n" %(samplelist[sample][0]) )
        file.write("queue %s" %(samplelist[sample][1]) )
    print("%s condor submit file created!!! " %(samplelist[sample][0]))

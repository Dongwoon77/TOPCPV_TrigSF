# -*- coding: utf-8 -*-
import os
import sys
import subprocess


def Make_CondorScr(sampleName, outputName, runPeriod, Channels, NumFileList, NumJob):
    path = os.getcwd()
    output_dir = "%s/%s/%s/%s" % (outputName, runPeriod, Channels, sampleName)
    os.system("mkdir -p %s" % output_dir)
    
    path = path.replace("/Batch", "/")
    os.getcwd()
    os.system('pwd')
    os.system('mkdir -p Log')
    os.system('ls')
    print(path)
    numjobs = NumJob
    numfiles = NumFileList
    SampleFile = sampleName
    SubFile = SampleFile + "_"
    
    Lists = MakeSeparateList(numfiles, numjobs)
    for i in range(0, numjobs):
        idx_ = i + 1
        runing = output_dir + "/%s_%s.sh" % (SampleFile, idx_)
        f = open(runing, 'w')
        f.write("#!/bin/bash \n\n")
        f.write("export SCRAM_ARCH=el9_amd64_gcc12\n")
        f.write("source /cvmfs/cms.cern.ch/cmsset_default.sh \n\n")
        f.write("ROOTSYS=/cvmfs/cms.cern.ch/el9_amd64_gcc12/lcg/root/6.26.11-09a813662aa0f8eece54cb6d94bfed85\n")
        f.write("export PATH=$ROOTSYS/bin:$PATH\n")
        f.write("export ROOT_INCLUDE_PATH=$ROOTSYS/include\n")
        f.write("export LD_LIBRARY_PATH=$ROOTSYS/lib:$LD_LIBRARY_PATH\n\n")
        f.write("export ROOT_CLING_SYMLINK_BOOL=1\n")
        f.write("export CLING_STANDARD_PCH=none\n")
        f.write("export ROOT_HIST=0\n\n")
        f.write("fileListNum=$((${1}+1)) \n")
        f.write("cd /u/user/dwkim/TOP/YU_class_TOP/NANOAOD/NANOAOD_RUN2_v6p2/CMSSW_13_3_0/src/SSBNanoAODANCode/ \n")
        f.write("cmsenv \n\n")

        #f.write('#!/bin/tcsh \n')
        #f.write('setenv SCRAM_ARCH slc7_amd64_gcc530 \n')
        #f.write('source /cvmfs/cms.cern.ch/cmsset_default.csh \n')
        #f.write('setenv LD_PRELOAD "/usr/lib64/libpdcap.so" \n')
        f.write('cd '+ path + " \n")
        f.write('mkdir -p ./output/%s/%s/%s/%s \n' % (outputName, runPeriod, Channels, sampleName))
        #f.write('cmsenv \n')
        SampList = MakeSampleIdxList(SampleFile, Lists[i])
        print("SampList %s" % (SampList))
        f.write('inputlists=(%s)\n' % (SampList))
        f.write('for i in "${inputlists[@]}"\n')
        f.write('do\n')
        f.write('   mkdir -p output\n')
        f.write('   ./ssb_analysis InputLists/%s/%s/${i}.list %s/%s/%s/%s/${i}.root ULSummer20/UL2016PreVFP/dimuon.config None %s -1 %s/branch_list.txt \n' % (runPeriod, sampleName, outputName, runPeriod, Channels, sampleName, runPeriod, runPeriod))
        f.write('done\n')
        f.close()
        runchMod = "chmod 755 " + runing
        os.system(runchMod)
        
        condor_file = "./%s/condor_%s_%s.submit" % (output_dir, SampleFile, idx_)
        f1 = open(condor_file, 'w')
        f1.write('universe = vanilla\n')
        f1.write('executable = %s\n' % runing)
        f1.write('output = %s/%s_%s.out\n' % (output_dir, SampleFile, idx_))
        f1.write('error = %s/errors_%s_%s.err\n' % (output_dir, SampleFile, idx_))
        f1.write('log = %s/test_%s_%s.log\n' % (output_dir, SampleFile, idx_))
        f1.write('request_memory = 1000\n')
        f1.write('+JobFlavour = "tomorrow"\n')
        f1.write('queue\n')
        f1.close()
        
        subchMod = "condor_submit " + condor_file
        os.system(subchMod)
    pass

def MakeSampleIdxList(Sample, List):
    SampleList = ""
    for index_ in List:
        print("index_ %s in MakeSampleIdxList " % (index_))
        SampleList += '"%s_%s"' % (Sample, index_)
        SampleList += " "
        pass
    print(SampleList)
    return SampleList
    pass

def MakeSeparateList(NumFiles, NumJob):
    quo = NumFiles / NumJob
    seplists = []
    for inx_ in range(NumJob):
        emptyarray = []
        seplists.append(emptyarray)
    print("size of seplists : %s " % len(seplists))
    for inumfile in range(1, NumFiles + 1):
        seplists[inumfile % NumJob].append(inumfile)
    for idx_ in seplists:
        print("content of %s " % (idx_))
    return seplists
    pass


if __name__ == '__main__':
    # Output name suffix
    output_suffix = "_NanoAOD_v6p2"
    run_period = "UL2016PreVFP"
    channels = "MuMu"
    num_jobs = 5
    
    # Data samples
    # DoubleEG
    #Make_CondorScr("Data_DoubleEG_Run2016B", "Data_DoubleEG_Run2016B" + output_suffix, run_period, channels, 528, num_jobs)
    #Make_CondorScr("Data_DoubleEG_Run2016C", "Data_DoubleEG_Run2016C" + output_suffix, run_period, channels, 175, num_jobs)
    #Make_CondorScr("Data_DoubleEG_Run2016D", "Data_DoubleEG_Run2016D" + output_suffix, run_period, channels, 293, num_jobs)
    #Make_CondorScr("Data_DoubleEG_Run2016E", "Data_DoubleEG_Run2016E" + output_suffix, run_period, channels, 248, num_jobs)
    #Make_CondorScr("Data_DoubleEG_Run2016F-HIPM", "Data_DoubleEG_Run2016F-HIPM" + output_suffix, run_period, channels, 157, num_jobs)
    
    # DoubleMuon
    Make_CondorScr("Data_DoubleMuon_Run2016B", "Data_DoubleMuon_Run2016B" + output_suffix, run_period, channels, 527, num_jobs)
    Make_CondorScr("Data_DoubleMuon_Run2016C", "Data_DoubleMuon_Run2016C" + output_suffix, run_period, channels, 175, num_jobs)
    Make_CondorScr("Data_DoubleMuon_Run2016D", "Data_DoubleMuon_Run2016D" + output_suffix, run_period, channels, 292, num_jobs)
    Make_CondorScr("Data_DoubleMuon_Run2016E", "Data_DoubleMuon_Run2016E" + output_suffix, run_period, channels, 248, num_jobs)
    Make_CondorScr("Data_DoubleMuon_Run2016F-HIPM", "Data_DoubleMuon_Run2016F-HIPM" + output_suffix, run_period, channels, 157, num_jobs)
    
    # MuonEG
    #Make_CondorScr("Data_MuonEG_Run2016B", "Data_MuonEG_Run2016B" + output_suffix, run_period, channels, 527, num_jobs)
    #Make_CondorScr("Data_MuonEG_Run2016C", "Data_MuonEG_Run2016C" + output_suffix, run_period, channels, 175, num_jobs)
    #Make_CondorScr("Data_MuonEG_Run2016D", "Data_MuonEG_Run2016D" + output_suffix, run_period, channels, 292, num_jobs)
    #Make_CondorScr("Data_MuonEG_Run2016E", "Data_MuonEG_Run2016E" + output_suffix, run_period, channels, 248, num_jobs)
    #Make_CondorScr("Data_MuonEG_Run2016F-HIPM", "Data_MuonEG_Run2016F-HIPM" + output_suffix, run_period, channels, 157, num_jobs)
    
    # SingleElectron
    #Make_CondorScr("Data_SingleElectron_Run2016B", "Data_SingleElectron_Run2016B" + output_suffix, run_period, channels, 528, num_jobs)
    #Make_CondorScr("Data_SingleElectron_Run2016D", "Data_SingleElectron_Run2016D" + output_suffix, run_period, channels, 292, num_jobs)
    #Make_CondorScr("Data_SingleElectron_Run2016E", "Data_SingleElectron_Run2016E" + output_suffix, run_period, channels, 248, num_jobs)
    #Make_CondorScr("Data_SingleElectron_Run2016F-HIPM", "Data_SingleElectron_Run2016F-HIPM" + output_suffix, run_period, channels, 157, num_jobs)
    
    # SingleMuon
    Make_CondorScr("Data_SingleMuon_Run2016B", "Data_SingleMuon_Run2016B" + output_suffix, run_period, channels, 527, num_jobs)
    Make_CondorScr("Data_SingleMuon_Run2016C", "Data_SingleMuon_Run2016C" + output_suffix, run_period, channels, 175, num_jobs)
    Make_CondorScr("Data_SingleMuon_Run2016D", "Data_SingleMuon_Run2016D" + output_suffix, run_period, channels, 292, num_jobs)
    Make_CondorScr("Data_SingleMuon_Run2016E", "Data_SingleMuon_Run2016E" + output_suffix, run_period, channels, 249, num_jobs)
    Make_CondorScr("Data_SingleMuon_Run2016F-HIPM", "Data_SingleMuon_Run2016F-HIPM" + output_suffix, run_period, channels, 157, num_jobs)
    
    # TTbar samples
    Make_CondorScr("TTbar_Signal", "TTbar_Signal" + output_suffix, run_period, channels, 33, num_jobs)
    Make_CondorScr("TTbar_AllHadronic", "TTbar_AllHadronic" + output_suffix, run_period, channels, 79, num_jobs)
    Make_CondorScr("TTbar_SemiLeptonic", "TTbar_SemiLeptonic" + output_suffix, run_period, channels, 117, num_jobs)
    
    # TTJets samples
    #Make_CondorScr("TTJets_TuneCP5_amcatnloFXFX", "TTJets_TuneCP5_amcatnloFXFX" + output_suffix, run_period, channels, 95, num_jobs)
    #Make_CondorScr("TTJets_TuneCP5_madgraphMLM", "TTJets_TuneCP5_madgraphMLM" + output_suffix, run_period, channels, 27, num_jobs)
    
    # DYJets samples
    Make_CondorScr("DYJetsToLL_M_10To50", "DYJetsToLL_M_10To50" + output_suffix, run_period, channels, 23, num_jobs)
    #Make_CondorScr("DYJetsToLL_M_10To50_madgraphMLM", "DYJetsToLL_M_10To50_madgraphMLM" + output_suffix, run_period, channels, 19, num_jobs)
    Make_CondorScr("DYJetsToLL_M_50", "DYJetsToLL_M_50" + output_suffix, run_period, channels, 62, num_jobs)
    #Make_CondorScr("DYJetsToLL_M_50_madgraphMLM", "DYJetsToLL_M_50_madgraphMLM" + output_suffix, run_period, channels, 66, num_jobs)
    
    # WJets samples
    Make_CondorScr("WJetsToLNu", "WJetsToLNu" + output_suffix, run_period, channels, 59, num_jobs)
    #Make_CondorScr("WJetsToLNu_madgraphMLM", "WJetsToLNu_madgraphMLM" + output_suffix, run_period, channels, 38, num_jobs)
    
    # TTW, TTZ samples
    Make_CondorScr("TTWJetsToLNu", "TTWJetsToLNu" + output_suffix, run_period, channels, 26, num_jobs)
    Make_CondorScr("TTWJetsToQQ", "TTWJetsToQQ" + output_suffix, run_period, channels, 7, num_jobs)
    Make_CondorScr("TTZToLLNuNu", "TTZToLLNuNu" + output_suffix, run_period, channels, 15, num_jobs)
    Make_CondorScr("TTZToQQ", "TTZToQQ" + output_suffix, run_period, channels, 278, num_jobs)
    
    # ST samples
    Make_CondorScr("ST_s-channel_4f_leptonDecays", "ST_s-channel_4f_leptonDecays" + output_suffix, run_period, channels, 12, num_jobs)
    Make_CondorScr("ST_t-channel_antitop_4f_InclusiveDecays", "ST_t-channel_antitop_4f_InclusiveDecays" + output_suffix, run_period, channels, 46, num_jobs)
    Make_CondorScr("ST_t-channel_top_4f_InclusiveDecays", "ST_t-channel_top_4f_InclusiveDecays" + output_suffix, run_period, channels, 49, num_jobs)
    Make_CondorScr("ST_tW_antitop_5f_NoFullyHadronicDecays", "ST_tW_antitop_5f_NoFullyHadronicDecays" + output_suffix, run_period, channels, 12, num_jobs)
    Make_CondorScr("ST_tW_top_5f_NoFullyHadronicDecays", "ST_tW_top_5f_NoFullyHadronicDecays" + output_suffix, run_period, channels, 4, num_jobs)
    
    # Diboson samples
    Make_CondorScr("WW", "WW" + output_suffix, run_period, channels, 22, num_jobs)
    Make_CondorScr("WZ", "WZ" + output_suffix, run_period, channels, 13, num_jobs)
    Make_CondorScr("ZZ", "ZZ" + output_suffix, run_period, channels, 8, num_jobs)
    
    # QCD HT samples
    #Make_CondorScr("QCD_HT100to200_TuneCP5_PSWeights_madgraph", "QCD_HT100to200_TuneCP5_PSWeights_madgraph" + output_suffix, run_period, channels, 87, num_jobs)
    #Make_CondorScr("QCD_HT2000toInf_TuneCP5_PSWeights_madgraph", "QCD_HT2000toInf_TuneCP5_PSWeights_madgraph" + output_suffix, run_period, channels, 37, num_jobs)
    #Make_CondorScr("QCD_HT200to300_TuneCP5_PSWeights_madgraph", "QCD_HT200to300_TuneCP5_PSWeights_madgraph" + output_suffix, run_period, channels, 47, num_jobs)
    #Make_CondorScr("QCD_HT300to500_TuneCP5_PSWeights_madgraph", "QCD_HT300to500_TuneCP5_PSWeights_madgraph" + output_suffix, run_period, channels, 89, num_jobs)
    #Make_CondorScr("QCD_HT500to700_TuneCP5_PSWeights_madgraph", "QCD_HT500to700_TuneCP5_PSWeights_madgraph" + output_suffix, run_period, channels, 34, num_jobs)
    #Make_CondorScr("QCD_HT700to1000_TuneCP5_PSWeights_madgraph", "QCD_HT700to1000_TuneCP5_PSWeights_madgraph" + output_suffix, run_period, channels, 20, num_jobs)
    
    # QCD MuEnriched samples (partial)
    #Make_CondorScr("QCD_Pt-1000_MuEnrichedPt5_TuneCP5_pythia8", "QCD_Pt-1000_MuEnrichedPt5_TuneCP5_pythia8" + output_suffix, run_period, channels, 42, num_jobs)
    #Make_CondorScr("QCD_Pt-120To170_MuEnrichedPt5_TuneCP5_pythia8", "QCD_Pt-120To170_MuEnrichedPt5_TuneCP5_pythia8" + output_suffix, run_period, channels, 32, num_jobs)
    #Make_CondorScr("QCD_Pt-15To20_MuEnrichedPt5_TuneCP5_pythia8", "QCD_Pt-15To20_MuEnrichedPt5_TuneCP5_pythia8" + output_suffix, run_period, channels, 28, num_jobs)
    #Make_CondorScr("QCD_Pt-170To300_MuEnrichedPt5_TuneCP5_pythia8", "QCD_Pt-170To300_MuEnrichedPt5_TuneCP5_pythia8" + output_suffix, run_period, channels, 63, num_jobs)
    #Make_CondorScr("QCD_Pt-20To30_MuEnrichedPt5_TuneCP5_pythia8", "QCD_Pt-20To30_MuEnrichedPt5_TuneCP5_pythia8" + output_suffix, run_period, channels, 50, num_jobs)
    #Make_CondorScr("QCD_Pt-20_MuEnrichedPt15_TuneCP5_pythia8", "QCD_Pt-20_MuEnrichedPt15_TuneCP5_pythia8" + output_suffix, run_period, channels, 11, num_jobs)
    #Make_CondorScr("QCD_Pt-300To470_MuEnrichedPt5_TuneCP5_pythia8", "QCD_Pt-300To470_MuEnrichedPt5_TuneCP5_pythia8" + output_suffix, run_period, channels, 46, num_jobs)
    #Make_CondorScr("QCD_Pt-30_MuEnrichedPt4_TuneCP5_13TeV_pythia8", "QCD_Pt-30_MuEnrichedPt4_TuneCP5_13TeV_pythia8" + output_suffix, run_period, channels, 69, num_jobs)
    
    # QCD EMEnriched samples (partial)
    #Make_CondorScr("QCD_Pt-120to170_EMEnriched_TuneCP5_pythia8", "QCD_Pt-120to170_EMEnriched_TuneCP5_pythia8" + output_suffix, run_period, channels, 13, num_jobs)
    #Make_CondorScr("QCD_Pt-15to20_EMEnriched_TuneCP5_pythia8", "QCD_Pt-15to20_EMEnriched_TuneCP5_pythia8" + output_suffix, run_period, channels, 42, num_jobs)
    #Make_CondorScr("QCD_Pt-170to300_EMEnriched_TuneCP5_pythia8", "QCD_Pt-170to300_EMEnriched_TuneCP5_pythia8" + output_suffix, run_period, channels, 7, num_jobs)
    #Make_CondorScr("QCD_Pt-20to30_EMEnriched_TuneCP5_pythia8", "QCD_Pt-20to30_EMEnriched_TuneCP5_pythia8" + output_suffix, run_period, channels, 9, num_jobs)
    #Make_CondorScr("QCD_Pt-300toInf_EMEnriched_TuneCP5_pythia8", "QCD_Pt-300toInf_EMEnriched_TuneCP5_pythia8" + output_suffix, run_period, channels, 14, num_jobs)
    #Make_CondorScr("QCD_Pt-30to40_DoubleEMEnriched_MGG-80toInf_TuneCP5_pythia8", "QCD_Pt-30to40_DoubleEMEnriched_MGG-80toInf_TuneCP5_pythia8" + output_suffix, run_period, channels, 22, num_jobs)
    
    # Other QCD samples
    #Make_CondorScr("QCD_Pt_1000to1400_TuneCP5_13TeV_pythia8", "QCD_Pt_1000to1400_TuneCP5_13TeV_pythia8" + output_suffix, run_period, channels, 19, num_jobs)
    #Make_CondorScr("QCD_Pt_120to170_TuneCP5_13TeV_pythia8", "QCD_Pt_120to170_TuneCP5_13TeV_pythia8" + output_suffix, run_period, channels, 26, num_jobs)
    #Make_CondorScr("QCD_Pt_1400to1800_TuneCP5_13TeV_pythia8", "QCD_Pt_1400to1800_TuneCP5_13TeV_pythia8" + output_suffix, run_period, channels, 20, num_jobs)
    #Make_CondorScr("QCD_Pt_15to30_TuneCP5_13TeV_pythia8", "QCD_Pt_15to30_TuneCP5_13TeV_pythia8" + output_suffix, run_period, channels, 15, num_jobs)
    #Make_CondorScr("QCD_Pt_170to300_TuneCP5_13TeV_pythia8", "QCD_Pt_170to300_TuneCP5_13TeV_pythia8" + output_suffix, run_period, channels, 25, num_jobs)
    #Make_CondorScr("QCD_Pt_1800to2400_TuneCP5_13TeV_pythia8", "QCD_Pt_1800to2400_TuneCP5_13TeV_pythia8" + output_suffix, run_period, channels, 10, num_jobs)
    
    
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
    
    # Determine which branch list file and config file to use based on sample name
    if "Run2017B" in sampleName:
        branch_list_file = "%s/branch_list_2017B.txt" % runPeriod
        config_file = "ULSummer20/UL2017/dimuon_Data_RunB.config"
    else:
        branch_list_file = "%s/branch_list.txt" % runPeriod
        config_file = "Run3/2024/dielec.config"
    
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
        f.write("cd /u/user/dwkim/TOP/YU_class_TOP/NANOAOD/NANOAOD_RUN3_test_FV/CMSSW_13_3_0/src/SSBNanoAODANCode/\n")
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
        f.write('   ./ssb_analysis InputLists/%s/%s/${i}.list %s/%s/%s/%s/${i}.root %s None %s -1 %s \n' % (runPeriod, sampleName, outputName, runPeriod, Channels, sampleName, config_file, runPeriod, branch_list_file))
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
        f1.write('+JobType = "long"\n')
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
    output_suffix = "_NanoAOD_v6p2_RUN3"
    run_period = "2024"
    channels = "ElEl"
    num_jobs = 50
    
    # Data samples
    # EGamma0
    Make_CondorScr("Data_EGamma0_Run2024C", "Data_EGamma0_Run2024C" + output_suffix, run_period, channels, 243, num_jobs)
    Make_CondorScr("Data_EGamma0_Run2024D", "Data_EGamma0_Run2024D" + output_suffix, run_period, channels, 232, num_jobs)
    Make_CondorScr("Data_EGamma0_Run2024E", "Data_EGamma0_Run2024E" + output_suffix, run_period, channels, 315, num_jobs)
    Make_CondorScr("Data_EGamma0_Run2024F", "Data_EGamma0_Run2024F" + output_suffix, run_period, channels, 698, num_jobs)
    Make_CondorScr("Data_EGamma0_Run2024G", "Data_EGamma0_Run2024G" + output_suffix, run_period, channels, 923, num_jobs)
    Make_CondorScr("Data_EGamma0_Run2024H", "Data_EGamma0_Run2024H" + output_suffix, run_period, channels, 131, num_jobs)
    Make_CondorScr("Data_EGamma0_Run2024I_v1", "Data_EGamma0_Run2024I_v1" + output_suffix, run_period, channels, 138, num_jobs)
    Make_CondorScr("Data_EGamma0_Run2024I_v2", "Data_EGamma0_Run2024I_v2" + output_suffix, run_period, channels, 130, num_jobs)

    # EGamma1
    Make_CondorScr("Data_EGamma1_Run2024C", "Data_EGamma1_Run2024C" + output_suffix, run_period, channels, 244, num_jobs)
    Make_CondorScr("Data_EGamma1_Run2024D", "Data_EGamma1_Run2024D" + output_suffix, run_period, channels, 232, num_jobs)
    Make_CondorScr("Data_EGamma1_Run2024E", "Data_EGamma1_Run2024E" + output_suffix, run_period, channels, 315, num_jobs)
    Make_CondorScr("Data_EGamma1_Run2024F", "Data_EGamma1_Run2024F" + output_suffix, run_period, channels, 697, num_jobs)
    Make_CondorScr("Data_EGamma1_Run2024G", "Data_EGamma1_Run2024G" + output_suffix, run_period, channels, 923, num_jobs)
    Make_CondorScr("Data_EGamma1_Run2024H", "Data_EGamma1_Run2024H" + output_suffix, run_period, channels, 131, num_jobs)
    Make_CondorScr("Data_EGamma1_Run2024I_v1", "Data_EGamma1_Run2024I_v1" + output_suffix, run_period, channels, 138, num_jobs)
    Make_CondorScr("Data_EGamma1_Run2024I_v2", "Data_EGamma1_Run2024I_v2" + output_suffix, run_period, channels, 130, num_jobs)
    
    # MuonEG
    #Make_CondorScr("Data_MuonEG_Run2016F", "Data_MuonEG_Run2016F" + output_suffix, run_period, channels, 25, num_jobs)
    #Make_CondorScr("Data_MuonEG_Run2016G", "Data_MuonEG_Run2016G" + output_suffix, run_period, channels, 427, num_jobs)
    #Make_CondorScr("Data_MuonEG_Run2016H", "Data_MuonEG_Run2016H" + output_suffix, run_period, channels, 475, num_jobs)
    
    # SingleElectron
    #Make_CondorScr("Data_SingleElectron_Run2016F", "Data_SingleElectron_Run2016F" + output_suffix, run_period, channels, 25, num_jobs)
    #Make_CondorScr("Data_SingleElectron_Run2016G", "Data_SingleElectron_Run2016G" + output_suffix, run_period, channels, 427, num_jobs)
    #Make_CondorScr("Data_SingleElectron_Run2016H", "Data_SingleElectron_Run2016H" + output_suffix, run_period, channels, 475, num_jobs)
    
    # SingleMuon
    #Make_CondorScr("Data_SingleMuon_Run2017B", "Data_SingleMuon_Run2017B" + output_suffix, run_period, channels, 243, num_jobs)
    #Make_CondorScr("Data_SingleMuon_Run2017C", "Data_SingleMuon_Run2017C" + output_suffix, run_period, channels, 516, num_jobs)
    #Make_CondorScr("Data_SingleMuon_Run2017D", "Data_SingleMuon_Run2017D" + output_suffix, run_period, channels, 271, num_jobs)
    #Make_CondorScr("Data_SingleMuon_Run2017E", "Data_SingleMuon_Run2017E" + output_suffix, run_period, channels, 429, num_jobs)
    #Make_CondorScr("Data_SingleMuon_Run2017F", "Data_SingleMuon_Run2017F" + output_suffix, run_period, channels, 585, num_jobs)
    
    # TTbar samples
    Make_CondorScr("TTbar_Signal", "TTbar_Signal" + output_suffix, run_period, channels, 78, num_jobs)
    Make_CondorScr("TTbar_Hadronic", "TTbar_Hadronic" + output_suffix, run_period, channels, 8, 8)
    Make_CondorScr("TTbar_SemiLeptonic", "TTbar_SemiLeptonic" + output_suffix, run_period, channels, 8, 8)
    
    # TTJets samples
    #Make_CondorScr("TTJets_TuneCP5_13TeV-amcatnloFXFX", "TTJets_TuneCP5_13TeV-amcatnloFXFX" + output_suffix, run_period, channels, 11, num_jobs)
    #Make_CondorScr("TTJets_TuneCP5_13TeV-madgraphMLM", "TTJets_TuneCP5_13TeV-madgraphMLM" + output_suffix, run_period, channels, 3, 3)
    
    # DYJets samples

    Make_CondorScr("DYto2E-2Jets_Bin-MLL-10to50", "DYto2E-2Jets_Bin-MLL-10to50" + output_suffix, run_period, channels, 7, 7)
    Make_CondorScr("DYto2E-2Jets_Bin-MLL-50", "DYto2E-2Jets_Bin-MLL-50" + output_suffix, run_period, channels, 30, 30)
    Make_CondorScr("DYto2E-4Jets_Bin-MLL-10to50", "DYto2E-4Jets_Bin-MLL-10to50" + output_suffix, run_period, channels, 25, 25)
    Make_CondorScr("DYto2E-4Jets_Bin-MLL-50", "DYto2E-4Jets_Bin-MLL-50" + output_suffix, run_period, channels, 29, 29)


    #Make_CondorScr("DYJetsToLL_M_10to50", "DYJetsToLL_M_10to50" + output_suffix, run_period, channels, 41, num_jobs)
    #Make_CondorScr("DYJetsToLL_M_10To50_madgraphMLM", "DYJetsToLL_M_10To50_madgraphMLM" + output_suffix, run_period, channels, 34, num_jobs)
    #Make_CondorScr("DYJetsToLL_M_50", "DYJetsToLL_M_50" + output_suffix, run_period, channels, 153, num_jobs)
    #Make_CondorScr("DYJetsToLL_M_50_madgraphMLM", "DYJetsToLL_M_50_madgraphMLM" + output_suffix, run_period, channels, 61, num_jobs)


    #Make_CondorScr("WJetsToLNu", "WJetsToLNu" + output_suffix, run_period, channels, 45, num_jobs)
    #Make_CondorScr("WJetsToLNu_madgraphMLM", "WJetsToLNu_madgraphMLM" + output_suffix, run_period, channels, 80, num_jobs)
    
    # TTW, TTZ samples
    #Make_CondorScr("TTWJetsToLNu", "TTWJetsToLNu" + output_suffix, run_period, channels, 9, 9)
    #Make_CondorScr("TTWJetsToQQ", "TTWJetsToQQ" + output_suffix, run_period, channels, 10, num_jobs)
    #Make_CondorScr("TTZToLLNuNu_M-10", "TTZToLLNuNu" + output_suffix, run_period, channels, 19, num_jobs)
    #Make_CondorScr("TTZToQQ_13TeV_amcatlno", "TTZToQQ_13TeV_amcatlno" + output_suffix, run_period, channels, 359, num_jobs)
    #Make_CondorScr("TTZToQQ_amcatnlo", "TTZToQQ_amcatnlo" + output_suffix, run_period, channels, 35, num_jobs)

    
    # ST samples
    Make_CondorScr("TbarWplusto2L2Nu", "TbarWplusto2L2Nu" + output_suffix, run_period, channels, 1, 1)
    Make_CondorScr("TbarWplustoLNu2Q", "TbarWplustoLNu2Q" + output_suffix, run_period, channels, 3, 3)
    Make_CondorScr("TbarWplusto4Q", "TbarWplusto4Q" + output_suffix, run_period, channels, 2, 2)
    Make_CondorScr("TWminusto2L2Nu", "TWminusto2L2Nu" + output_suffix, run_period, channels, 2, 2)
    Make_CondorScr("TWminustoLNu2Q", "TWminustoLNu2Q" + output_suffix, run_period, channels, 3, 3)
    Make_CondorScr("TWminusto4Q", "TWminusto4Q" + output_suffix, run_period, channels, 2, 2)
    
    # Diboson samples
    Make_CondorScr("WW", "WW" + output_suffix, run_period, channels, 6,  6)
    Make_CondorScr("WZ", "WZ" + output_suffix, run_period, channels, 4,  4)
    Make_CondorScr("ZZ", "ZZ" + output_suffix, run_period, channels, 1,  1)
    
    
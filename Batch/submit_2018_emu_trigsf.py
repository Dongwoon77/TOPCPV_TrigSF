# -*- coding: utf-8 -*-
import os
import sys
import subprocess

# SE output directory for ROOT output files
SE_OUTPUT_DIR = "/u/user/dwkim/SE_UserHome/TOP_CPV_output/2018_trigSF"

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
        config_file = "ULSummer20/UL2018/muelec.config"
    
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
        f.write("cd /u/user/dwkim/TOP/YU_class_TOP/NANOAOD/TriggerSF/CMSSW_13_3_0/src/SSBNanoAODANCode/\n")
        f.write("cmsenv \n\n")

        #f.write('#!/bin/tcsh \n')
        #f.write('setenv SCRAM_ARCH slc7_amd64_gcc530 \n')
        #f.write('source /cvmfs/cms.cern.ch/cmsset_default.csh \n')
        #f.write('setenv LD_PRELOAD "/usr/lib64/libpdcap.so" \n')
        f.write('cd '+ path + " \n")
        f.write('mkdir -p %s/%s/%s/%s/%s \n' % (SE_OUTPUT_DIR, outputName, runPeriod, Channels, sampleName))
        #f.write('cmsenv \n')
        SampList = MakeSampleIdxList(SampleFile, Lists[i])
        print("SampList %s" % (SampList))
        f.write('inputlists=(%s)\n' % (SampList))
        f.write('for i in "${inputlists[@]}"\n')
        f.write('do\n')
        f.write('   ./ssb_analysis InputLists/%s/%s/${i}.list %s/%s/%s/%s/${i}.root %s %s %s -1 %s \n' % (runPeriod, sampleName, outputName, runPeriod, Channels, sampleName, config_file, SE_OUTPUT_DIR, runPeriod, branch_list_file))
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
    output_suffix = "_RUN3"
    run_period = "UL2018"
    channels = "MuEl"
    num_jobs = 50
    
    # Data samples
    #MC
    Make_CondorScr("TTTo2L2Nu_TuneCP5_13TeV-powheg-pythia8_2018_trigsf", "TTTo2L2Nu_TuneCP5_13TeV-powheg-pythia8_2018_trigsf" + output_suffix, run_period, channels, 2, 2)


    
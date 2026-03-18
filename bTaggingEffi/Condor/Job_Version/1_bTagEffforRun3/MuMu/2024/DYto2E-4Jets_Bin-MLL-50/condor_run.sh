#!/bin/bash 

export SCRAM_ARCH=el9_amd64_gcc12
source /cvmfs/cms.cern.ch/cmsset_default.sh 

ROOTSYS=/cvmfs/cms.cern.ch/el9_amd64_gcc12/lcg/root/6.26.11-09a813662aa0f8eece54cb6d94bfed85
export PATH=$ROOTSYS/bin:$PATH
export ROOT_INCLUDE_PATH=$ROOTSYS/include
export LD_LIBRARY_PATH=$ROOTSYS/lib:$LD_LIBRARY_PATH

export ROOT_CLING_SYMLINK_BOOL=1
export CLING_STANDARD_PCH=none
export ROOT_HIST=0

fileListNum=$((${1}+1)) 
cd /u/user/dwkim/TOP/YU_class_TOP/NANOAOD/NANOAOD_RUN3_test_FV/CMSSW_13_3_0/src/SSBNanoAODANCode/ 
cmsenv 

mkdir -p ./output/Job_Version/1_bTagEffforRun3/MuMu/2024/DYto2E-4Jets_Bin-MLL-50/
cd bTaggingEffi 
./makeBtagEff_All ./input/2024/DYto2E-4Jets_Bin-MLL-50/DYto2E-4Jets_Bin-MLL-50_${fileListNum}.list ./output/Job_Version/1_bTagEffforRun3/MuMu/2024/DYto2E-4Jets_Bin-MLL-50/DYto2E-4Jets_Bin-MLL-50_${fileListNum} -1

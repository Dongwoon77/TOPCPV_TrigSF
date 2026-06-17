#!/bin/bash

#runPeriod="UL2016PreVFP"
runPeriod="2024"
#runPeriod="2022PostEE"
StudyName="Test_Run3"
Channels="ElEl"
echo "$runPeriod"

#inputlists=("TTto2L2Nu_TuneCP5_13p6TeV_powheg-pythia8_2022_trigsf_1")
inputlists=("TTto2L2Nu_TuneCP5_13p6TeV_powheg-pythia8_2024_trigsf_1")
#inputlists=("TTto2L2Nu_TuneCP5_13p6TeV_powheg-pythia8_2022PostEE_trigsf_1")
#inputlists=("Data_JetMET_Run2022C_trigsf_1")


configdir=""
configch=""
echo "Good"

if [ "$Channels" = "MuMu" ]; then
    confch="dimuon.cfg"
elif [ "$Channels" = "ElEl" ]; then
    confch="dielec.cfg"
elif [ "$Channels" = "MuEl" ]; then
    confch="muelec.cfg"
else
    echo "Unknown Channels: $Channels"
    exit 1
fi

config="Run3/${runPeriod}/${confch}"

for i in "${inputlists[@]}"
do
   #mkdir -p output/${StudyName}/${runPeriod}/${Channels}/TTto2L2Nu_TuneCP5_13p6TeV_powheg-pythia8_2022_trigsf
   #mkdir -p output/${StudyName}/${runPeriod}/${Channels}/Data_SingleMuon_Run2017B
   #./ssb_analysis "${runPeriod}/TTbar_Signal/${i}.list" "${StudyName}/${runPeriod}/${Channels}/TTbar_Signal/${i}.root" "ULSummer20/${runPeriod}/dimuon.config" "None" ${runPeriod} -1
   #echo ./ssb_analysis "InputLists/${runPeriod}/${i}.list" "${StudyName}/${runPeriod}/${Channels}/${i}.root" "Run3/${runPeriod}/dimuon.config" "None" ${runPeriod} -1 ${runPeriod}/brach_list.txt 
   #./ssb_analysis "InputLists/${runPeriod}/${i}.list" "${StudyName}/${runPeriod}/${Channels}/${i}.root" "Run3/${runPeriod}/dimuon.config" "None" ${runPeriod} 100000 ${runPeriod}/branch_list.txt 
   ./ssb_analysis "InputLists/${runPeriod}/${i}.list" "${StudyName}/${runPeriod}/${Channels}/${i}.root" "Run3/${runPeriod}/dielec.config" "None" ${runPeriod} 100000 ${runPeriod}/branch_list.txt 
   #./ssb_analysis "${runPeriod}/${i}.list" "${StudyName}/${runPeriod}/${Channels}/${i}.root" "ULSummer20/${runPeriod}/dimuon_Data_RunB.config" "None" ${runPeriod} -1 branch_list.txt 
done


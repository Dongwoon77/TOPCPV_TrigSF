#!/bin/bash

#runPeriod="UL2016PreVFP"
runPeriod="UL2017"
StudyName="Testv1"
Channels="MuMu"
echo "$runPeriod"

inputlists=("TTbar_Signal")
inputlists=("TTbar_Signal_1")
inputlists=("Data_SingleMuon_Run2017B/Data_SingleMuon_Run2017B_1")

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

config="ULSummer20/${runPeriod}/${confch}"

for i in "${inputlists[@]}"
do
   #mkdir -p output/${StudyName}/${runPeriod}/${Channels}/TTbar_Signal
   #mkdir -p output/${StudyName}/${runPeriod}/${Channels}/Data_SingleMuon_Run2017B
   #./ssb_analysis "${runPeriod}/TTbar_Signal/${i}.list" "${StudyName}/${runPeriod}/${Channels}/TTbar_Signal/${i}.root" "ULSummer20/${runPeriod}/dimuon.config" "None" ${runPeriod} -1
   echo ./ssb_analysis "${runPeriod}/${i}.list" "${StudyName}/${runPeriod}/${Channels}/${i}.root" "ULSummer20/${runPeriod}/dimuon.config" "None" ${runPeriod} -1 ${runPeriod}/brach_list_2017.txt 
   ./ssb_analysis "${runPeriod}/${i}.list" "${StudyName}/${runPeriod}/${Channels}/${i}.root" "ULSummer20/${runPeriod}/dimuon_Data_RunB.config" "None" ${runPeriod} -1 ${runPeriod}/branch_list_2017B.txt 
   #./ssb_analysis "${runPeriod}/${i}.list" "${StudyName}/${runPeriod}/${Channels}/${i}.root" "ULSummer20/${runPeriod}/dimuon_Data_RunB.config" "None" ${runPeriod} -1 branch_list.txt 
done


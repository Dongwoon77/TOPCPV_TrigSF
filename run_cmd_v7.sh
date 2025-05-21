#!/bin/bash

# === [USAGE CHECK] ===
if [ "$#" -lt 9 ]; then
    echo "Usage: $0 <MainPath> <runPeriod> <StudyName> <Channels> <configFile> <branchList> <MaxEvents> <SEDir> <input1> [<input2> ...]"
    exit 1
fi

# === [STEP 0: Move to directory & Setup CMSSW environment] ===
MainPath=$1
shift

if [ "$MainPath" = "./" ] || [ "$MainPath" = "current" ]; then
    MainPath=$(pwd)
fi

cd "$MainPath" || { echo "[ERROR] Cannot change to directory: $MainPath"; exit 1; }

# === [CMSSW ENVIRONMENT SETUP] ===
current_path=$(pwd)
cmssw_dir=$(echo "$current_path" | grep -o ".*/CMSSW_[^/]*")

if [ -d "$cmssw_dir/src" ]; then
    echo "[INFO] Setting up CMSSW from: $cmssw_dir"
    cd "$cmssw_dir/src" || exit 1
    export SCRAM_ARCH=el9_amd64_gcc12  # 필요 시 사용자 환경에 맞게 변경
    source /cvmfs/cms.cern.ch/cmsset_default.sh
    eval "$(scramv1 runtime -sh)"
    cd "$current_path" || exit 1
else
    echo "[ERROR] Could not detect CMSSW base directory."
    exit 1
fi
# === [CORRECT INPUT MAPPING] ===
runPeriod="$1"
StudyName="$2"
Channels="$3"
configFile="$4"
branchList="$5"
maxEvents="$6"
sedirname="$7"
shift 7

# === [DEBUGGING INFO] ===
echo "Run Period       = $runPeriod"
echo "Study Name       = $StudyName"
echo "Channels         = $Channels"
echo "Config File      = $configFile"
echo "Branch List File = $branchList"
echo "Max Events       = $maxEvents"
echo "SE Dir Name      = $sedirname"
echo "Input Files      = $@"

# === [CHECK EXECUTABLE] ===
if [ ! -f "./ssb_analysis" ]; then
    echo "Error: ssb_analysis program not found in directory $dir"
    exit 1
fi


# === [MAIN LOOP OVER INPUT LISTS] ===
for entry in "$@"; do
    dir=$(dirname "$entry")     # e.g., Data_SingleMuon_Run2018A
    base=$(basename "$entry")   # e.g., Data_SingleMuon_Run2018A_1

    # Remove .list extension if present
    #base_name="${list_file%.list}"
    # Paths
    inputlist="${runPeriod}/${dir}/${base}.list"
    relpath="${StudyName}/${runPeriod}/${Channels}/${dir}"
    # Handle conditional output directory path
    if [ "$sedirname" != "None" ]; then
        outdir="${sedirname}/${relpath}"
    else
        outdir="output/${relpath}"
    fi
    outputfile="${relpath}/${base}.root"
    echo $branchList
    mkdir -p "$outdir"
    #mkdir -p "output/${relpath}"




    #echo "[RUN] ./ssb_analysis ${inputlist} ${outputfile} ULSummer20/${runPeriod}/${config} ${sedirname} ${runPeriod} ${maxEvents} ${branch_list}"
    echo "[RUN] ./ssb_analysis ${inputlist} ${outputfile} ULSummer20/${runPeriod}/${configFile} ${sedirname} ${runPeriod} ${maxEvents} ${branchList}"
    ./ssb_analysis "${inputlist}" \
                   "${outputfile}" \
                   "ULSummer20/${runPeriod}/${configFile}" \
                   "${sedirname}" \
                   "${runPeriod}" \
                   "${maxEvents}" \
                   "${branchList}"
done
#./ssb_analysis "$runPeriod" "$StudyName" "$Channels" "$configFile" "$branchList" "$maxEvents" "$sedirname" "$@"

#!/bin/bash

# === [CONFIGURATION] ===
runPeriod="UL2017"
StudyName="Testv1"
Channels="MuMu"

echo "[INFO] runPeriod = $runPeriod"
echo "[INFO] Channels  = $Channels"

# List of input samples in the form: "Directory/FileBase"
inputlists=(
#    "TTbar_Signal/TTbar_Signal_1"
    #"Data_SingleMuon_Run2017C/Data_SingleMuon_Run2017C_1"
    "Data_SingleMuon_Run2017D/Data_SingleMuon_Run2017D_1"
)

# === [DETERMINE CHANNEL CONFIG BASE] ===
case "$Channels" in
    "MuMu") confch="dimuon" ;;
    "ElEl") confch="dielec" ;;
    "MuEl") confch="muelec" ;;
    *)
        echo "[ERROR] Unknown Channels: $Channels"
        exit 1
        ;;
esac

# === [MAIN LOOP OVER INPUT LISTS] ===
for entry in "${inputlists[@]}"; do
    dir=$(dirname "$entry")     # e.g. Data_SingleMuon_Run2017C
    base=$(basename "$entry")   # e.g. Data_SingleMuon_Run2017C_1

    # Build relative path and full output directory path
    relpath="${StudyName}/${runPeriod}/${Channels}/${dir}"
    outdir="output/${relpath}"
    mkdir -p "$outdir"

    echo "[INFO] Output directory will be: $outdir"

    # Determine config and branch list file
    if [[ "$dir" == Data_* ]]; then
        runTag=$(echo "$dir" | grep -o 'Run2017.' | sed 's/Run2017//')
        config="ULSummer20/${runPeriod}/${confch}_Data_Run${runTag}.config"
        branch_list="${runPeriod}/branch_list_2017${runTag}.txt"
    else
        config="ULSummer20/${runPeriod}/${confch}.config"
        branch_list="${runPeriod}/branch_list_2017.txt"
    fi

    # Build input list and output file path
    inputlist="${runPeriod}/${dir}/${base}.list"
    outputfile="${relpath}/${base}.root"

    echo "[RUN] ./ssb_analysis $inputlist $outputfile $config None $runPeriod -1 $branch_list"
    ./ssb_analysis "$inputlist" "$outputfile" "$config" "None" "$runPeriod" -1 "$branch_list"
done

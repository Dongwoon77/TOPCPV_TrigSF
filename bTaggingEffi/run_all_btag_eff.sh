#!/bin/bash

# B-tagging efficiency calculation script for all MC samples
# Usage: ./run_all_btag_eff.sh [year] [max_events]
# Year: 16pre|16post|2017|2018 (Run2) | 2022|2022PostEE|2023|2023BPix|2024 (Run3 UParTAK4)
### use condor instead of this script###

# Check if executable exists
EXEC="./makeBtagEff_All"
if [ ! -x "$EXEC" ]; then
    echo "[ERROR] Executable not found or not executable: $EXEC"
    echo "Try 'make' to compile it first."
    exit 1
fi

# Set default values
YEAR=${1:-"2024"}
MAX_EVENTS=${2:-"-1"}  # -1 means all events

echo "=========================================="
echo "[INFO] Starting b-tagging efficiency calculation"
echo "[INFO] Year: $YEAR"
echo "[INFO] Max events per sample: $MAX_EVENTS"
echo "=========================================="

# Sample list - same as in Create_CondorJob.py
SAMPLES=(
    "TTbar_Signal"
    #"DYJetsToLL_M_10to50"
    #"DYJetsToLL_M_50"
    #"ST_s-channel_4f_leptonDecays"
    #"ST_t-channel_antitop_4f_InclusiveDecays"
    #"ST_t-channel_top_4f_InclusiveDecays"
    #"ST_tW_antitop_5f_NoFullyHadronicDecays"
    #"ST_tW_top_5f_NoFullyHadronicDecays"
    #"TTbar_AllHadronic"
    #"TTbar_SemiLeptonic"
    #"TTWJetsToLNu"
    #"TTWJetsToQQ"
    #"TTZToLLNuNu"
    #"TTZToQQ"
    #"WJetsToLNu"
    #"WW"
    #"WZ"
    #"ZZ"
)

# Initialize counters
TOTAL_SAMPLES=${#SAMPLES[@]}
PROCESSED=0
FAILED=0

echo "[INFO] Total samples to process: $TOTAL_SAMPLES"
echo ""

# Process each sample
for SAMPLE in "${SAMPLES[@]}"; do
    echo "------------------------------------------"
    echo "[INFO] Processing sample: $SAMPLE"
    echo "[INFO] Progress: $((PROCESSED + 1))/$TOTAL_SAMPLES"
    
    # Construct input file path
    #INPUT_FILE="../InputLists/$YEAR/MC/$SAMPLE/${SAMPLE}_1.list"
    INPUT_FILE="./input/$YEAR/$SAMPLE/${SAMPLE}_1.list"

    
    # Check if input file exists
    if [ ! -f "$INPUT_FILE" ]; then
        echo "[WARNING] Input file not found: $INPUT_FILE"
        echo "[WARNING] Skipping $SAMPLE"
        ((FAILED++))
        continue
    fi
    
    # Create output directory
    OUTPUT_DIR="output/$YEAR/$SAMPLE"
    mkdir -p "$OUTPUT_DIR"
    echo "[INFO] Output directory: $OUTPUT_DIR"
    OUTPUT_PREFIX="${OUTPUT_DIR}/btagEff_1"

    # Run makeBtagEff_All
    echo "[INFO] Running: $EXEC $INPUT_FILE $MAX_EVENTS"
    START_TIME=$(date +%s)
    
    if [ "$MAX_EVENTS" == "-1" ]; then
        $EXEC "$INPUT_FILE" "$OUTPUT_PREFIX"
    else
        $EXEC "$INPUT_FILE" "$OUTPUT_PREFIX" "$MAX_EVENTS"
    fi
    
    # Check if execution was successful
    if [ $? -eq 0 ]; then
        END_TIME=$(date +%s)
        DURATION=$((END_TIME - START_TIME))
        echo "[INFO] Successfully processed $SAMPLE in ${DURATION}s"
        ((PROCESSED++))
    else
        echo "[ERROR] Failed to process $SAMPLE"
        ((FAILED++))
    fi
    
    echo ""
done

echo "=========================================="
echo "[INFO] B-tagging efficiency calculation completed"
echo "[INFO] Successfully processed: $PROCESSED/$TOTAL_SAMPLES samples"
echo "[INFO] Failed: $FAILED samples"
echo "=========================================="

# List output files
echo "[INFO] Output files generated:"
find output/$YEAR -name "*.root" -type f | sort
echo ""

if [ $FAILED -gt 0 ]; then
    echo "[WARNING] Some samples failed to process. Check the log messages above."
    exit 1
else
    echo "[INFO] All samples processed successfully!"
    exit 0
fi 
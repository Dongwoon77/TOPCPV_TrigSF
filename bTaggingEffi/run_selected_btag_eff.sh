#!/bin/bash

# Run makeBtagEff_All on selected file lists
# Usage: $0 <relative_path_to_list> [relative_path_to_list2] ... [maxEvents]
# Example: $0 2024/TTbar_Signal/TTbar_Signal_1.list
# Example: $0 2024/TTbar_Signal/TTbar_Signal_1.list 10000  (limit to 10000 events)

EXEC="./makeBtagEff_All"
INPUT_DIR="./input"

if [ ! -x "$EXEC" ]; then
    echo "[ERROR] Executable not found or not executable: $EXEC"
    echo "Try 'make' to compile it first."
    exit 1
fi

if [ $# -lt 1 ]; then
    echo "Usage: $0 <relative_path_to_list1> [path2 ...] [maxEvents]"
    echo "  Path is relative to $INPUT_DIR"
    echo "  maxEvents: optional, -1 for all (default)"
    echo "Example: $0 2024/TTbar_Signal/TTbar_Signal_1.list"
    echo "Example: $0 2024/TTbar_Signal/TTbar_Signal_1.list 2024/TTbar_Signal/TTbar_Signal_2.list"
    echo "Example: $0 2024/TTbar_Signal/TTbar_Signal_1.list 10000"
    exit 2
fi

# Last arg might be maxEvents (numeric)
MAX_EVENTS="-1"
if [[ "${@: -1}" =~ ^[0-9-]+$ ]]; then
    MAX_EVENTS="${@: -1}"
    ARGS=("${@:1:$#-1}")
else
    ARGS=("$@")
fi

for REL_PATH in "${ARGS[@]}"; do
    LIST_FILE="${INPUT_DIR}/${REL_PATH}"
    if [ -f "$LIST_FILE" ]; then
        # Output: output/<year>/<sample>/btagEff_<num>
        # e.g. 2024/TTbar_Signal/TTbar_Signal_1.list -> output/2024/TTbar_Signal/btagEff_1
        SAMPLE_NAME=$(basename "$REL_PATH" .list)
        NUM="${SAMPLE_NAME##*_}"
        SAMPLE_DIR=$(dirname "$REL_PATH")
        YEAR=$(echo "$SAMPLE_DIR" | cut -d'/' -f1)
        SAMPLE=$(echo "$SAMPLE_DIR" | cut -d'/' -f2)
        OUTPUT_PREFIX="output/${SAMPLE_DIR}/btagEff_${NUM}"
        mkdir -p "output/${SAMPLE_DIR}"
        echo "=========================================="
        echo "[INFO] Processing: $LIST_FILE"
        echo "[INFO] Output prefix: $OUTPUT_PREFIX"
        echo "------------------------------------------"
        if [ "$MAX_EVENTS" == "-1" ]; then
            $EXEC "$LIST_FILE" "$OUTPUT_PREFIX"
        else
            $EXEC "$LIST_FILE" "$OUTPUT_PREFIX" "$MAX_EVENTS"
        fi
        echo "[INFO] Finished: $REL_PATH"
        echo ""
    else
        echo "[WARNING] File not found: $LIST_FILE"
    fi
done

echo "[INFO] Selected sample processing complete."

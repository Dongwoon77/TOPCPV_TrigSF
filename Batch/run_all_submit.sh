#!/bin/bash

# Script to run all submit scripts for Run3 years
# Usage: ./run_all_submit.sh

# Get the directory where this script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

echo "=========================================="
echo "Running all Run3 submit scripts"
echo "=========================================="
echo ""

# Array of submit scripts to run
SUBMIT_SCRIPTS=(
    "submit_2022.py"
    "submit_2022PostEE.py"
    "submit_2023.py"
    "submit_2023BPix.py"
    "submit_2024.py"
)

# Track success/failure
SUCCESS_COUNT=0
FAILURE_COUNT=0
FAILED_SCRIPTS=()

# Run each submit script
for script in "${SUBMIT_SCRIPTS[@]}"; do
    echo "----------------------------------------"
    echo "Running: $script"
    echo "----------------------------------------"
    
    if [ ! -f "$script" ]; then
        echo "ERROR: File $script not found!"
        FAILURE_COUNT=$((FAILURE_COUNT + 1))
        FAILED_SCRIPTS+=("$script (not found)")
        continue
    fi
    
    # Run the Python script
    python3 "$script"
    EXIT_CODE=$?
    
    if [ $EXIT_CODE -eq 0 ]; then
        echo "✓ Successfully completed: $script"
        SUCCESS_COUNT=$((SUCCESS_COUNT + 1))
    else
        echo "✗ Failed: $script (exit code: $EXIT_CODE)"
        FAILURE_COUNT=$((FAILURE_COUNT + 1))
        FAILED_SCRIPTS+=("$script")
    fi
    
    echo ""
done

# Print summary
echo "=========================================="
echo "Summary"
echo "=========================================="
echo "Total scripts: ${#SUBMIT_SCRIPTS[@]}"
echo "Successful: $SUCCESS_COUNT"
echo "Failed: $FAILURE_COUNT"
echo ""

if [ $FAILURE_COUNT -gt 0 ]; then
    echo "Failed scripts:"
    for failed in "${FAILED_SCRIPTS[@]}"; do
        echo "  - $failed"
    done
    echo ""
    exit 1
else
    echo "All scripts completed successfully!"
    exit 0
fi


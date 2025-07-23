#!/usr/bin/env python3

import argparse
import os
import subprocess
import re

def get_sample_list(base_dir):
    if not os.path.exists(base_dir):
        print(f"[ERROR] Base directory not found: {base_dir}")
        return []
    return sorted([
        name for name in os.listdir(base_dir)
        if os.path.isdir(os.path.join(base_dir, name))
    ])

def get_listfiles(sample, run_period, listfile_root):
    print("in get_listfiles sample runperiod listfile_root : %s %s %s "%(sample, run_period, listfile_root) )
    listdir = os.path.join(listfile_root, run_period, sample)
    print("in get_listfiles : %s"%(listdir) )
    if not os.path.exists(listdir):
        return []

    listfiles = []
    for f in os.listdir(listdir):
        if re.match(f"{re.escape(sample)}_\\d+\\.list", f):
            listfiles.append(os.path.join(listdir, f))
    return sorted(listfiles)

def extract_root_from_listfile(filepath):
    with open(filepath, "r") as f:
        return [
            line.strip()
            for line in f if line.strip() and not line.startswith("#")
        ]

def main():
    parser = argparse.ArgumentParser(description="Run hadd with listfile checking against base directory.")
    parser.add_argument("--StudyName", required=True)
    parser.add_argument("--RunPeriod", required=True)
    parser.add_argument("--Channel", required=True)
    parser.add_argument("--ListfileDir", required=True, help="Base input dir (e.g., ../input/)")
    parser.add_argument("--OutputLocation", default=".", help="Merged root output dir")
    parser.add_argument("--RecreateMerge", action='store_true')
    
    # Add new option: BaseDir for flexible base directory specification
    parser.add_argument("--BaseDir", default=None, 
                       help="Base directory path where ROOT files are stored. If not provided, uses ../RunPeriod/Channel/")
    
    args = parser.parse_args()

    os.makedirs(args.OutputLocation, exist_ok=True)

    # BaseDir configuration logic
    if args.BaseDir:
        # User specified BaseDir directly
        base_dir = args.BaseDir
    else:
        # Default: go up from current directory and use RunPeriod/Channel structure
        base_dir = os.path.join("..", args.RunPeriod, args.Channel)
    
    # Convert to absolute path
    base_dir = os.path.abspath(base_dir)
    
    print(f"[INFO] Using base directory: {base_dir}")
    
    sampleList = get_sample_list(base_dir)
    print("Sample : %s"%sampleList) 
    if not sampleList:
        print(f"[ERROR] No samples found in base directory: {base_dir}")
        return

    for sample in sampleList:
        print(f"[INFO] Processing sample: {sample}")
        sample_dir = os.path.join(base_dir, sample)
        listfiles = get_listfiles(sample, args.RunPeriod, args.ListfileDir)

        if not listfiles:
            print(f"[WARN] No listfiles found for sample {sample}. Skipping.")
            continue

        valid_rootfiles = []
        missing_entries = []

        for listfile in listfiles:
            # Convert listfile name to expected ROOT file name
            # e.g., DYJetsToLL_M_10To50_9.list → DYJetsToLL_M_10To50_9.root
            list_basename = os.path.splitext(os.path.basename(listfile))[0]
            expected_rootfile = os.path.join(sample_dir, f"{list_basename}.root")

            if os.path.exists(expected_rootfile):
                valid_rootfiles.append(expected_rootfile)
            else:
                missing_entries.append((listfile, expected_rootfile))

        if missing_entries:
            # Create full directory structure: OutputLocation/RunPeriod/Channel/Sample/
            sample_log_dir = os.path.join(args.OutputLocation, args.RunPeriod, args.Channel, sample)
            os.makedirs(sample_log_dir, exist_ok=True)
            log_path = os.path.join(sample_log_dir, f"{sample}_missing_check.log")
            
            with open(log_path, "w") as log:
                log.write(f"[WARNING] Mismatch or missing entries for {sample}:\n")
                for listf, expectf in missing_entries:
                    log.write(f"List: {listf}\nExpected: {expectf}\n\n")
            
            print(f"[WARN] Some entries missing or mismatched. See log: {log_path}")
            continue
            
        if not valid_rootfiles:
            print(f"[SKIP] No valid ROOT inputs for {sample}.")
            continue

        # Perform hadd operation
        # Create output directory per sample
        output_dir = os.path.join(args.OutputLocation, args.RunPeriod, args.Channel, sample)
        os.makedirs(output_dir, exist_ok=True)

        outputfile = os.path.join(output_dir, f"{sample}.root")
        hadd_logfile = os.path.join(output_dir, f"{sample}.log")

        if not args.RecreateMerge and os.path.exists(outputfile):
            print(f"[SKIP] Output already exists for {sample}: {outputfile}")
            continue

        cmd = ["hadd"]
        if args.RecreateMerge:
            cmd.append("-f")
        cmd.extend([outputfile] + valid_rootfiles)

        with open(hadd_logfile, "w") as log:
            try:
                subprocess.run(cmd, stdout=log, stderr=log, check=True)
                print(f"[OK] Successfully merged {sample}.")
                print(f"     Output: {outputfile}")
                print(f"     Log: {hadd_logfile}")
            except subprocess.CalledProcessError:
                print(f"[ERROR] hadd failed for {sample}. Check: {hadd_logfile}")

if __name__ == "__main__":
    main()

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
    listdir = os.path.join(listfile_root, run_period, sample)
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
    args = parser.parse_args()

    os.makedirs(args.OutputLocation, exist_ok=True)

    base_dir = f"/pnfs/knu.ac.kr/data/cms/store/user/sha/CPV_Run2/ULSummer20/{args.StudyName}/{args.RunPeriod}/{args.Channel}"
    sampleList = get_sample_list(base_dir)

    for sample in sampleList:
        print(f"[INFO] Processing sample: {sample}")
        sample_dir = os.path.join(base_dir, sample)
        listfiles = get_listfiles(sample, args.RunPeriod, args.ListfileDir)

        if not listfiles:
            print(f"[WARN] No listfiles found for sample {sample}. Skipping.")
            continue
        """
        valid_rootfiles = []
        missing_entries = []
        for listfile in listfiles:
            expected_basename = os.path.splitext(os.path.basename(listfile))[0] + ".root"
            expected_fullpath = os.path.join(sample_dir, expected_basename)

            entries = extract_root_from_listfile(listfile)
            if len(entries) != 1 or not os.path.exists(expected_fullpath) or entries[0] != expected_fullpath:
                missing_entries.append((listfile, expected_fullpath))
            else:
                valid_rootfiles.append(expected_fullpath)
        """
        valid_rootfiles = []
        missing_entries = []

        for listfile in listfiles:
            # DYJetsToLL_M_10To50_9.list → DYJetsToLL_M_10To50_9.root
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

        # Perform hadd
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
        """
        outputfile = os.path.join(args.OutputLocation, f"{sample}.root")
        hadd_logfile = os.path.join(args.OutputLocation, f"{sample}.log")

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
                print(f"[OK] Successfully merged {sample}. Output: {outputfile}")
            except subprocess.CalledProcessError:
                print(f"[ERROR] hadd failed for {sample}. Check: {hadd_logfile}")
        """
if __name__ == "__main__":
    main()

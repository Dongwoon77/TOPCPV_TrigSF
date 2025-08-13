import os
import sys
import subprocess
import re
import time
from collections import defaultdict

def submit_condor_job(submit_file_path):
    """Submit a job to HTCondor with appropriate settings"""
    # Determine if running on KISTI
    user_home = os.path.expanduser("~")
    is_kisti = user_home.startswith("/cms/ldap_home")

    # KISTI 전용: Check proxy
    if is_kisti:
        uid = os.getuid()
        proxy_path = f"/tmp/x509up_u{uid}"
        if not os.path.isfile(proxy_path):
            print(f"[ERROR] Proxy file not found: {proxy_path}")
            print("Run: voms-proxy-init -voms cms")
            sys.exit(1)

    # Build condor_submit command
    cmd = ["condor_submit"]
    if is_kisti:
        cmd += ["-append", "accounting_group = group_cms"]
    cmd.append(submit_file_path)

    print(f"[INFO] Submitting job: {' '.join(cmd)}")
    try:
        subprocess.run(cmd, check=True)
    except subprocess.CalledProcessError as e:
        print(f"[ERROR] condor_submit failed: {e}")
        sys.exit(1)

def isDirectoryInDataList(directory, channel, runPeriod, debug=False):
    """Check if directory matches the required data list for the channel"""
    dimuonList = ["SingleMuon", "DoubleMuon"]
    dielecList = ["SingleElectron", "DoubleEG"]
    muelecList = ["SingleMuon", "SingleElectron", "MuonEG"]
    dataList = []

    if runPeriod == "UL2018":
        dielecList = ["EGamma"]
        muelecList = ["SingleMuon", "EGamma", "MuonEG"]

    if channel == "MuMu":
        dataList = dimuonList
    elif channel == "ElEl":
        dataList = dielecList
    elif channel == "MuEl":
        dataList = muelecList

    if debug:
        print(f"Data list for {channel}: {dataList}")

    return any(item in directory for item in dataList)

def contains_str_prefix(disc, string):
    """Check if string contains the discriminator prefix"""
    return disc in string

def has_number_suffix(filename):
    """Check if filename has a number suffix"""
    return bool(re.match(r".+_\d+$", filename))

def check_job_completion(logDir, studyName, runPeriod, channel, sampleDir, listFileName, debug=False):
    """
    Check if a job needs resubmission based on .err and .out files
    Returns True if resubmission is needed
    Checks both original and resubmitted job logs
    """
    jobLogDir = os.path.join(logDir, studyName, runPeriod, channel, sampleDir)
    
    # Check both original and resubmitted logs (prioritize most recent)
    log_prefixes = ["resubmit_", ""]  # Check resubmit logs first
    
    for prefix in log_prefixes:
        # Check .err file
        err_file = os.path.join(jobLogDir, f"{prefix}{sampleDir}_{listFileName}.err")
        out_file = os.path.join(jobLogDir, f"{prefix}{sampleDir}_{listFileName}.out")
        
        if not os.path.exists(err_file):
            if debug and prefix == "":  # Only print for original if no resubmit exists
                print(f"[DEBUG] Error file not found: {err_file}")
            continue  # Try next prefix
        
        # Check if .err file has content (should be empty)
        if os.path.getsize(err_file) > 0:
            if debug:
                print(f"[DEBUG] Error file has content: {err_file}")
            return True
        
        # Check .out file
        if not os.path.exists(out_file):
            if debug:
                print(f"[DEBUG] Output file not found: {out_file}")
            return True
        
        try:
            with open(out_file, 'r') as f:
                content = f.read()
                
            # Check for "Total number of events after merging root files: 0"
            if "Total number of events after merging root files: 0" in content:
                if debug:
                    print(f"[DEBUG] Job produced 0 events: {out_file}")
                return True
            
            # Check for completion indicators at the end
            required_endings = [
                "fout successfully deleted.",
                "SSBConfReader successfully deleted.",
                "SSBCorr successfully deleted.",
                "SSBCPVCal successfully deleted.",
                "Analysis destructor completed."
            ]
            
            # Check if all required endings are present
            for ending in required_endings:
                if ending not in content:
                    if debug:
                        print(f"[DEBUG] Missing completion indicator '{ending}' in: {out_file}")
                    return True
            
            if debug:
                print(f"[DEBUG] Job completed successfully: {out_file}")
            return False  # Job completed successfully
            
        except Exception as e:
            if debug:
                print(f"[DEBUG] Error reading output file {out_file}: {e}")
            return True
    
    # If no log files found at all
    if debug:
        print(f"[DEBUG] No log files found for {sampleDir}_{listFileName}")
    return True

def check_job_status(logDir, studyName, runPeriod, channel, sampleDir, listFileName, debug=False):
    """
    Check the status of a specific job
    Returns: 'completed', 'failed', 'not_started', 'no_output', 'zero_events', 'not_submitted'
    """
    jobLogDir = os.path.join(logDir, studyName, runPeriod, channel, sampleDir)
    
    # Check both original and resubmitted logs (prioritize most recent)
    log_prefixes = ["resubmit_", ""]
    
    for prefix in log_prefixes:
        err_file = os.path.join(jobLogDir, f"{prefix}{sampleDir}_{listFileName}.err")
        out_file = os.path.join(jobLogDir, f"{prefix}{sampleDir}_{listFileName}.out")
        log_file = os.path.join(jobLogDir, f"{prefix}{sampleDir}_{listFileName}.log")
        
        # Check if any log files exist
        files_exist = {
            'err': os.path.exists(err_file),
            'out': os.path.exists(out_file),
            'log': os.path.exists(log_file)
        }
        
        if debug:
            print(f"[DEBUG] Checking {prefix}{sampleDir}_{listFileName}: {files_exist}")
        
        # If no files exist at all, continue to next prefix
        if not any(files_exist.values()):
            continue
        
        # If log file exists but err/out don't, job was submitted but didn't start properly
        if files_exist['log'] and not (files_exist['err'] and files_exist['out']):
            return 'not_started'
        
        # If err/out exist but no log, unusual case
        if (files_exist['err'] or files_exist['out']) and not files_exist['log']:
            if debug:
                print(f"[DEBUG] Unusual case: err/out exist but no log file")
        
        # If only err or only out exists, incomplete execution
        if files_exist['err'] != files_exist['out']:
            return 'no_output'
        
        # Both err and out exist, check their content
        if files_exist['err'] and files_exist['out']:
            # Check if .err file has content (should be empty)
            if os.path.getsize(err_file) > 0:
                if debug:
                    print(f"[DEBUG] Error file has content: {err_file}")
                return 'failed'
            
            try:
                with open(out_file, 'r') as f:
                    content = f.read()
                
                # Check for "Total number of events after merging root files: 0"
                if "Total number of events after merging root files: 0" in content:
                    return 'zero_events'
                
                # Check for completion indicators
                required_endings = [
                    "fout successfully deleted.",
                    "SSBConfReader successfully deleted.",
                    "SSBCorr successfully deleted.",
                    "SSBCPVCal successfully deleted.",
                    "Analysis destructor completed."
                ]
                
                # Check if all required endings are present
                for ending in required_endings:
                    if ending not in content:
                        if debug:
                            print(f"[DEBUG] Missing completion indicator '{ending}' in: {out_file}")
                        return 'failed'
                
                return 'completed'
                
            except Exception as e:
                if debug:
                    print(f"[DEBUG] Error reading output file {out_file}: {e}")
                return 'failed'
    
    # No log files found for any prefix
    return 'not_submitted'

def check_all_jobs_status(inputListPath, logDir, studyName, runPeriod, channel, samples, debug=False):
    """
    Check status of all jobs and provide a comprehensive report
    """
    print("="*80)
    print(f"JOB STATUS REPORT")
    print(f"Study: {studyName}, Period: {runPeriod}, Channel: {channel}")
    print("="*80)
    
    overall_stats = defaultdict(int)
    sample_details = {}
    
    if "all" in samples:
        samples = os.listdir(inputListPath)
        if debug:
            print(f"Using all samples in {inputListPath}")
    
    for sampleDir in samples:
        sampleInputPath = os.path.join(inputListPath, sampleDir)
        if not os.path.isdir(sampleInputPath):
            continue
            
        print(f"\n📁 Sample: {sampleDir}")
        print("-" * 60)
        
        sample_stats = defaultdict(int)
        job_details = []
        
        # Get all list files
        list_files = []
        for listFile in os.listdir(sampleInputPath):
            if listFile.endswith(".list"):
                listFileName = os.path.splitext(listFile)[0]
                if listFileName == sampleDir or not listFileName.startswith(f"{sampleDir}_"):
                    continue
                number_part = listFileName[len(sampleDir)+1:]
                if not number_part.isdigit():
                    continue
                list_files.append(number_part)
        
        list_files.sort(key=int)  # Sort numerically
        
        # Check status of each job
        for job_num in list_files:
            status = check_job_status(logDir, studyName, runPeriod, channel, sampleDir, job_num, debug)
            sample_stats[status] += 1
            overall_stats[status] += 1
            
            # Store details for failed/problematic jobs
            if status != 'completed':
                job_details.append((job_num, status))
        
        # Print sample summary
        total_jobs = len(list_files)
        if total_jobs > 0:
            completed = sample_stats['completed']
            failed = sample_stats['failed']
            zero_events = sample_stats['zero_events']
            not_submitted = sample_stats.get('not_submitted', 0)
            not_started = sample_stats.get('not_started', 0) 
            no_output = sample_stats.get('no_output', 0)
            
            completion_rate = (completed / total_jobs) * 100
            
            print(f"  Total Jobs: {total_jobs}")
            print(f"  ✅ Completed: {completed} ({completion_rate:.1f}%)")
            if failed > 0:
                print(f"  ❌ Failed: {failed}")
            if zero_events > 0:
                print(f"  🔢 Zero Events: {zero_events}")
            if not_submitted > 0:
                print(f"  🚫 Not Submitted: {not_submitted}")
            if not_started > 0:
                print(f"  ⏸️  Not Started: {not_started}")
            if no_output > 0:
                print(f"  📄 Incomplete Output: {no_output}")
            
            # Show problematic jobs
            if job_details:
                print(f"  🔍 Problematic Jobs:")
                for job_num, status in job_details[:10]:  # Show first 10
                    status_icon = {
                        "failed": "❌", 
                        "zero_events": "🔢",
                        "not_submitted": "🚫",
                        "not_started": "⏸️",
                        "no_output": "📄"
                    }
                    print(f"    {status_icon.get(status, '?')} Job {job_num}: {status}")
                if len(job_details) > 10:
                    print(f"    ... and {len(job_details) - 10} more")
        else:
            print(f"  ⚠️  No jobs found")
        
        sample_details[sampleDir] = {
            'total': total_jobs,
            'stats': dict(sample_stats),
            'problematic': job_details
        }
    
    # Print overall summary
    print("\n" + "="*80)
    print("OVERALL SUMMARY")
    print("="*80)
    
    total_jobs = sum(overall_stats.values())
    if total_jobs > 0:
        completed = overall_stats['completed']
        failed = overall_stats['failed']
        zero_events = overall_stats['zero_events']
        not_submitted = overall_stats.get('not_submitted', 0)
        not_started = overall_stats.get('not_started', 0)
        no_output = overall_stats.get('no_output', 0)
        
        overall_completion = (completed / total_jobs) * 100
        
        print(f"📊 Total Jobs: {total_jobs}")
        print(f"✅ Completed: {completed} ({overall_completion:.1f}%)")
        if failed > 0:
            print(f"❌ Failed: {failed} ({(failed/total_jobs)*100:.1f}%)")
        if zero_events > 0:
            print(f"🔢 Zero Events: {zero_events} ({(zero_events/total_jobs)*100:.1f}%)")
        if not_submitted > 0:
            print(f"🚫 Not Submitted: {not_submitted} ({(not_submitted/total_jobs)*100:.1f}%)")
        if not_started > 0:
            print(f"⏸️  Not Started: {not_started} ({(not_started/total_jobs)*100:.1f}%)")
        if no_output > 0:
            print(f"📄 Incomplete Output: {no_output} ({(no_output/total_jobs)*100:.1f}%)")
        
        # Recommendations
        print(f"\n💡 RECOMMENDATIONS:")
        problematic_count = failed + zero_events + not_submitted + not_started + no_output
        if problematic_count > 0:
            print(f"   • Run with resubmit=True to resubmit failed jobs")
            if not_submitted > 0:
                print(f"   • {not_submitted} jobs were never submitted - check input lists")
            if not_started > 0:
                print(f"   • {not_started} jobs were submitted but didn't start - check HTCondor queue")
            problematic_samples = [s for s, d in sample_details.items() 
                                 if sum(d['stats'].values()) - d['stats'].get('completed', 0) > 0]
            if len(problematic_samples) <= 5:
                print(f"   • Problematic samples: {', '.join(problematic_samples)}")
            else:
                print(f"   • {len(problematic_samples)} samples need attention")
        else:
            print(f"   🎉 All jobs completed successfully!")
    else:
        print("⚠️  No jobs found!")
    
    print("="*80)
    
    return sample_details

def find_resubmission_jobs(logDir, studyName, runPeriod, channel, sampleDir, sampleInputPath, debug=False):
    """
    Find jobs that need resubmission based on log analysis
    Resubmits all jobs that are not completed
    """
    resubmit_list = []
    
    for listFile in os.listdir(sampleInputPath):
        if listFile.endswith(".list"):
            listFileName = os.path.splitext(listFile)[0]
            if listFileName == sampleDir or not listFileName.startswith(f"{sampleDir}_"):
                if debug:
                    print(f"Skipping file that doesn't match pattern: {listFileName}")
                continue
            
            number_part = listFileName[len(sampleDir)+1:]
            if not number_part.isdigit():
                if debug:
                    print(f"Skipping file without numeric suffix: {listFileName}")
                continue
            
            # Use check_job_status for consistency with status reporting
            status = check_job_status(logDir, studyName, runPeriod, channel, sampleDir, number_part, debug)
            
            # Resubmit everything that's not completed
            if status != 'completed':
                resubmit_list.append(number_part)
                if debug:
                    print(f"[DEBUG] Job {number_part} needs resubmission: {status}")
            elif debug:
                print(f"[DEBUG] Job {number_part} already completed")
    
    return resubmit_list

def submit_jobs(inputListPath, runScriptPath, logDir, studyName, runPeriod, channel, outputPath, 
               submitDir, MainPath, samples, configFile, branchList, maxEvents="-1", 
               debug=False, resubmit=False, check_status=False):
    """
    Main function to submit, check, or resubmit HTCondor jobs
    """
    
    # If only checking status, run the status check and return
    if check_status:
        return check_all_jobs_status(inputListPath, logDir, studyName, runPeriod, channel, samples, debug)
    
    submitDir = os.path.join(submitDir, studyName, runPeriod, channel)
    if debug:
        print(f"Creating submission directory: {submitDir}")
    os.makedirs(submitDir, exist_ok=True)

    if "all" in samples:
        samples = os.listdir(inputListPath)
        if debug:
            print(f"Detected 'all' in samples. Using all directories in {inputListPath}: {samples}")

    total_resubmitted = 0

    for sampleDir in samples:
        if debug:
            print(f"Processing sample directory: {sampleDir}")

        if contains_str_prefix("Data_", sampleDir):
            if debug:
                print("Detected data sample")
            if not isDirectoryInDataList(sampleDir, channel, runPeriod, debug):
                if debug:
                    print(f"Skipping data sample directory (not in data list): {sampleDir}")
                continue
        else:
            if debug:
                print("Detected MC sample")

        sampleInputPath = os.path.join(inputListPath, sampleDir)
        if not os.path.isdir(sampleInputPath):
            continue
            
        sampleOutputPath = os.path.join(outputPath, sampleDir)
        jobLogDir = os.path.join(logDir, studyName, runPeriod, channel, sampleDir)
        os.makedirs(jobLogDir, exist_ok=True)

        file_prefix = "resubmit_" if resubmit else ""

        submit_file_content = f"""Universe   = vanilla
Executable = {runScriptPath}
Log        = {jobLogDir}/{file_prefix}{sampleDir}_$(InputListName).log
Output     = {jobLogDir}/{file_prefix}{sampleDir}_$(InputListName).out
Error      = {jobLogDir}/{file_prefix}{sampleDir}_$(InputListName).err
RequestMemory = 2 GB
RequestCpus = 1
should_transfer_files = YES
when_to_transfer_output = ON_EXIT
transfer_input_files = {runScriptPath}
accounting_group = group_cms
Arguments  = {MainPath} {runPeriod} {studyName} {channel} {configFile} {branchList} {maxEvents} {outputPath} {sampleDir}/{sampleDir}_$(InputListName)
Queue InputListName from (
"""

        if resubmit:
            # Use find_resubmission_jobs function (resubmits all non-completed jobs)
            resubmit_list = find_resubmission_jobs(logDir, studyName, runPeriod, channel, sampleDir, sampleInputPath, debug)
            
            if resubmit_list:
                if debug:
                    print(f"Found {len(resubmit_list)} jobs for resubmission in {sampleDir}")
                    print(f"Jobs to resubmit: {sorted(resubmit_list, key=int)}")
                
                for file_name in resubmit_list:
                    submit_file_content += f"{file_name}\n"
                submit_file_content += ")"
                
                # Generate unique filename with timestamp to avoid conflicts
                timestamp = int(time.time())
                submit_file_path = os.path.join(submitDir, f"resubmit_{sampleDir}_{timestamp}.sub")
                with open(submit_file_path, "w") as submit_file:
                    submit_file.write(submit_file_content)
                
                print(f"[INFO] Resubmitting {len(resubmit_list)} jobs for {sampleDir}")
                if debug:
                    print(f"[DEBUG] Submit file created: {submit_file_path}")
                submit_condor_job(submit_file_path)
                total_resubmitted += len(resubmit_list)
            else:
                print(f"[INFO] No jobs need resubmission for {sampleDir}")
        else:
            # Normal submission - submit all jobs
            job_count = 0
            for listFile in os.listdir(sampleInputPath):
                if listFile.endswith(".list"):
                    listFileName = os.path.splitext(listFile)[0]
                    if listFileName == sampleDir or not listFileName.startswith(f"{sampleDir}_"):
                        if debug:
                            print(f"Skipping file that doesn't match pattern: {listFileName}")
                        continue
                    number_part = listFileName[len(sampleDir)+1:]
                    if not number_part.isdigit():
                        if debug:
                            print(f"Skipping file without numeric suffix: {listFileName}")
                        continue
                    
                    submit_file_content += f"{number_part}\n"
                    job_count += 1
            
            if job_count > 0:
                submit_file_content += ")"
                submit_file_path = os.path.join(submitDir, f"submit_{sampleDir}.sub")
                with open(submit_file_path, "w") as submit_file:
                    submit_file.write(submit_file_content)
                
                print(f"[INFO] Submitting {job_count} jobs for {sampleDir}")
                if debug:
                    print(f"Submitting Condor job for {sampleDir}")
                submit_condor_job(submit_file_path)
            else:
                print(f"[WARNING] No valid job files found for {sampleDir}")

    if resubmit:
        print(f"\n[SUMMARY] Total jobs resubmitted: {total_resubmitted}")

def main():
    """Main configuration and execution"""
    # ===== Main Configuration =====
    this_script_dir = os.path.dirname(os.path.abspath(__file__))
    MainPath = os.path.abspath(os.path.join(this_script_dir, ".."))

    inputList = os.path.join(MainPath, "input/")
    logDir = os.path.join(os.getcwd(), "condorLog")
    submitDir = os.path.join(os.getcwd(), "condorSubmit")
    runScriptPath = os.path.join(MainPath, "run_cmd_v7.sh")
    outputPath = "/pnfs/knu.ac.kr/data/cms/store/user/sha/CPV_Run2/ULSummer20"
    
    # Study configuration
    studyName = "RochesterApplied_v3"
    runPeriod = "UL2018"
    channel = "MuMu"
    maxEvents = "-1"

    # Sample list
    samples = [
        "DYJetsToLL_M_10To50",
        "DYJetsToLL_M_50",
        "Data_DoubleMuon_Run2018A",
        "Data_DoubleMuon_Run2018B",
        "Data_DoubleMuon_Run2018C",
        "Data_DoubleMuon_Run2018D",
        "Data_SingleMuon_Run2018A",
        "Data_SingleMuon_Run2018B",
        "Data_SingleMuon_Run2018C",
        "Data_SingleMuon_Run2018D",
        "ST_s-channel_4f_leptonDecays",
        "ST_t-channel_antitop_4f_InclusiveDecays",
        "ST_t-channel_top_4f_InclusiveDecays",
        "ST_tW_antitop_5f_NoFullyHadronicDecays",
        "ST_tW_top_5f_NoFullyHadronicDecays",
        "TTJets_TuneCP5_amcatnloFXFX",
        "TTJets_TuneCP5_madgraphMLM",
        "TTWJetsToLNu",
        "TTWJetsToQQ",
        "TTZToLLNuNu",
        "TTZToQQ",
        "TTbar_Signal",
        "TTbar_AllHadronic",
        "TTbar_SemiLeptonic",
        "WJetsToLNu",
        "WJetsToLNu_madgraphMLM",
        "WW",
        "WZ",
        "ZZ"
    ]

    branchList = "UL2018/branch_list.txt"
    configFile = "dimuon.config"
    debug = False

    # Execution settings
    submit_jobs(
        f"{inputList}/{runPeriod}",
        runScriptPath,
        logDir,
        studyName,
        runPeriod,
        channel,
        outputPath,
        submitDir,
        MainPath,
        samples,
        configFile,
        branchList,
        maxEvents=maxEvents,
        debug=debug,
        resubmit=True,      # Change to True for resubmission
        #resubmit=False,      # Change to True for resubmission
        check_status=False  # Change to True to check job status only
        #check_status=True  # Change to True to check job status only
    )

if __name__ == "__main__":
    main()

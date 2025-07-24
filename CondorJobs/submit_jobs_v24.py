import subprocess
import os
import sys

def submit_condor_job(submit_file_path):
    """Submit HTCondor job with KISTI-specific settings"""
    # Determine if running on KISTI
    user_home = os.path.expanduser("~")
    is_kisti = user_home.startswith("/cms/ldap_home")

    # KISTI specific: Check proxy
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


# ========== ORIGINAL SCRIPT STARTS HERE ==========
import os
import re
import subprocess

def isDirectoryInDataList(directory, channel, runPeriod, debug=False):
    """Check if directory matches the data list for given channel"""
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

    for item in dataList:
        if item in directory:
            return True
    return False

def contains_str_prefix(disc, string):
    """Check if string contains the discriminator"""
    return disc in string

def has_number_suffix(filename):
    """Check if filename has numeric suffix pattern"""
    return bool(re.match(r".+_\d+$", filename))

def needs_resubmission(outputPath, sampleDir, listFileName):
    """Check if job needs resubmission based on output file"""
    if not has_number_suffix(listFileName):
        return False
    expected_root_file = os.path.join(outputPath, sampleDir, f"{listFileName}.root")
    if not os.path.isfile(expected_root_file):
        return True
    elif os.path.getsize(expected_root_file) < 5120:  # 5KB threshold
        return True
    return False

def submit_jobs(sampleType, inputListPath, runScriptPath, logDir, studyName, runPeriod, 
               channel, outputPath, submitDir, MainPath, samples, configFile, branchList, 
               maxEvents="-1", debug=False, resubmit=False):
    """Main function to submit HTCondor jobs"""
    
    # Create submission directory
    submitDir = os.path.join(submitDir, studyName, runPeriod, channel)
    if debug:
        print(f"Creating submission directory: {submitDir}")
    os.makedirs(submitDir, exist_ok=True)

    # Handle 'all' samples
    if "all" in samples:
        samples = os.listdir(inputListPath)
        if debug:
            print(f"Detected 'all' in samples. Using all directories in {inputListPath}: {samples}")

    # Process each sample
    for sampleDir in samples:
        if debug:
            print(f"Processing sample directory: {sampleDir}")
        
        # Check if it's data sample and filter by channel
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

        # Set up paths
        sampleInputPath = os.path.join(inputListPath, sampleDir)
        sampleOutputPath = os.path.join(outputPath, sampleDir)
        jobLogDir = os.path.join(logDir, studyName, runPeriod, channel, sampleDir)
        os.makedirs(jobLogDir, exist_ok=True)

        file_prefix = "resubmit_" if resubmit else ""

        # Create temporary input directory for this sample only
        temp_input_dir = os.path.join(submitDir, f"input_{sampleDir}")
        os.makedirs(temp_input_dir, exist_ok=True)
        
        # Copy only the required input files for this sample
        sample_input_src = os.path.join(inputListPath, sampleDir)
        sample_input_dst = os.path.join(temp_input_dir, runPeriod, sampleDir)
        os.makedirs(os.path.dirname(sample_input_dst), exist_ok=True)
        
        if os.path.exists(sample_input_src):
            import shutil
            shutil.copytree(sample_input_src, sample_input_dst)
            if debug:
                print(f"Copied input files for {sampleDir} to {sample_input_dst}")
        
        # Create HTCondor submit file content
        # NOTE: Transfer only necessary files for this specific sample
        submit_file_content = f"""Universe   = vanilla
Executable = run_cmd_v8.sh
Log        = {jobLogDir}/{file_prefix}{sampleDir}_$(InputListName).log
Output     = {jobLogDir}/{file_prefix}{sampleDir}_$(InputListName).out
Error      = {jobLogDir}/{file_prefix}{sampleDir}_$(InputListName).err
RequestMemory = 4 GB
RequestCpus = 1
should_transfer_files = YES
when_to_transfer_output = ON_EXIT
transfer_input_files = run_cmd_v8.sh,ssb_analysis,ULSummer20,branchlist,{temp_input_dir}
Requirements = (OpSys == "LINUX") && (Arch == "X86_64") && (HasSingularity =?= True)
+SingularityImage = "/cvmfs/unpacked.cern.ch/registry.hub.docker.com/cmssw/slc7:latest"
accounting_group = group_cms
Arguments  = ./ {runPeriod} {studyName} {channel} {configFile} {branchList} {maxEvents} {outputPath} {sampleDir}/{sampleDir}_$(InputListName)
Queue InputListName from (
"""

        # Process list files for resubmission or normal submission
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

                if resubmit and needs_resubmission(outputPath, sampleDir, listFileName):
                    resubmit_list.append(number_part)
                elif not resubmit:
                    submit_file_content += f"{number_part}\n"

        # Submit jobs
        if resubmit and resubmit_list:
            for file_name in resubmit_list:
                submit_file_content += f"{file_name}\n"
            submit_file_content += ")"
            submit_file_path = os.path.join(submitDir, f"submit_{sampleDir}.sub")
            with open(submit_file_path, "w") as submit_file:
                submit_file.write(submit_file_content)
            if debug:
                print(f"Submitting Condor job for {sampleDir} with resubmission list: {resubmit_list}")
            submit_condor_job(submit_file_path)
        elif not resubmit:
            submit_file_content += ")"
            submit_file_path = os.path.join(submitDir, f"submit_{sampleDir}.sub")
            with open(submit_file_path, "w") as submit_file:
                submit_file.write(submit_file_content)
            if debug:
                print(f"Submitting Condor job for {sampleDir}")
            submit_condor_job(submit_file_path)

def main():
    """Main configuration and execution"""
    # ===== Main Configuration =====
    # NOTE: Use current directory as MainPath for file transfer compatibility
    MainPath = os.getcwd()
    inputList = os.path.join(MainPath, "input/")
    logDir = os.path.join(os.getcwd(), "condorLog")
    submitDir = os.path.join(os.getcwd(), "condorSubmit")
    runScriptPath = os.path.join(os.getcwd(), "run_cmd_v8.sh")
    outputPath = "./"
    runPeriod = "UL2018"
    studyName = "NanoAOD_v3p8"
    channel = "MuMu"
    maxEvents = "-1"

    # Sample list for processing
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

    branchList = "branchlist/UL2018/branch_list.txt"
    configFile = "dimuon.config"
    debug = True

    # Submit jobs
    submit_jobs(
        "None", 
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
        resubmit=False
    )

if __name__ == "__main__":
    main()

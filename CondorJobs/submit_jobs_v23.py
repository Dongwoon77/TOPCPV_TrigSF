import subprocess
import os
import sys

def submit_condor_job(submit_file_path):
    # Determine if running on KISTI
    user_home = os.path.expanduser("~")
    is_kisti = user_home.startswith("/cms/ldap_home")  # 수정된 부분

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


# ========== ORIGINAL SCRIPT STARTS HERE ==========
import os
import re
import subprocess

def isDirectoryInDataList(directory, channel, runPeriod, debug=False):
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
    return disc in string

def has_number_suffix(filename):
    return bool(re.match(r".+_\d+$", filename))

def needs_resubmission(outputPath, sampleDir, listFileName):
    if not has_number_suffix(listFileName):
        return False
    expected_root_file = os.path.join(outputPath, sampleDir, f"{listFileName}.root")
    if not os.path.isfile(expected_root_file):
        return True
    elif os.path.getsize(expected_root_file) < 5120:
        return True
    return False

def submit_jobs(sampleType, inputListPath, runScriptPath, logDir, studyName, runPeriod, channel, outputPath, submitDir, MainPath, samples, configFile, branchList, maxEvents="-1", debug=False, resubmit=False):
    submitDir = os.path.join(submitDir, studyName, runPeriod, channel)
    if debug:
        print(f"Creating submission directory: {submitDir}")
    os.makedirs(submitDir, exist_ok=True)

    if "all" in samples:
        samples = os.listdir(inputListPath)
        if debug:
            print(f"Detected 'all' in samples. Using all directories in {inputListPath}: {samples}")

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
        sampleOutputPath = os.path.join(outputPath, sampleDir)
        jobLogDir = os.path.join(logDir, studyName, runPeriod, channel, sampleDir)
        os.makedirs(jobLogDir, exist_ok=True)

        file_prefix = "resubmit_" if resubmit else ""

        # 수정된 부분: accounting_group 추가
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

        if resubmit and resubmit_list:
            for file_name in resubmit_list:
                submit_file_content += f"{file_name}\n"
            submit_file_content += ")"
            submit_file_path = os.path.join(submitDir, f"submit_{sampleDir}.sub")
            with open(submit_file_path, "w") as submit_file:
                submit_file.write(submit_file_content)
            if debug:
                print(f"Submitting Condor job for {sampleDir} with resubmission list: {resubmit_list}")
            submit_condor_job(submit_file_path)  # 수정된 함수 호출
        elif not resubmit:
            submit_file_content += ")"
            submit_file_path = os.path.join(submitDir, f"submit_{sampleDir}.sub")
            with open(submit_file_path, "w") as submit_file:
                submit_file.write(submit_file_content)
            if debug:
                print(f"Submitting Condor job for {sampleDir}")
            submit_condor_job(submit_file_path)  # 수정된 함수 호출

def main():
    # ===== Main Configuration =====
    MainPath = "/cms/ldap_home/sha/Develop/CPViolation/AnlaysisCode/v6/CMSSW_13_3_0/src/ANCode/v6p2/SSBNanoAODANCode/"
    inputList = os.path.join(MainPath, "input/")
    logDir = os.path.join(os.getcwd(), "condorLog")
    submitDir = os.path.join(os.getcwd(), "condorSubmit")
    runScriptPath = os.path.join(MainPath, "run_cmd_v7.sh")
    outputPath = "./"
    runPeriod = "UL2018"
    studyName = "NanoAOD_v3p4"
    channel = "MuMu"
    maxEvents = "-1"

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
    debug = True

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

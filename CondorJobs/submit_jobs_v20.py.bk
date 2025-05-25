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

# 숫자 패턴이 있는 파일명인지 확인하는 함수
def has_number_suffix(filename):
    # 파일명이 "_숫자"로 끝나는지 확인 (예: sample_1)
    return bool(re.match(r".+_\d+$", filename))

# Updated helper function to check if a root file is missing, too small, and matches the sample_Number format
def needs_resubmission(outputPath, sampleDir, listFileName):
    # 숫자 패턴이 없으면 재제출하지 않음
    if not has_number_suffix(listFileName):
        return False
    
    expected_root_file = os.path.join(outputPath, sampleDir, f"{listFileName}.root")
    # Check if file does not exist or if it exists but is less than 5KB
    if not os.path.isfile(expected_root_file):
        return True
    elif os.path.getsize(expected_root_file) < 5120:  # 5KB = 5120 bytes
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
        
        # Skip the directory if it doesn't match the data list criteria
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
        
        # Add "resubmit_" prefix to log, output, and error files if resubmitting
        file_prefix = "resubmit_" if resubmit else ""
        
       
        # 로그 파일 경로 수정 - 샘플 디렉토리 이름 추가
        submit_file_content = f"""Universe   = vanilla
Executable = {runScriptPath}
Log        = {jobLogDir}/{file_prefix}{sampleDir}_$(InputListName).log
Output     = {jobLogDir}/{file_prefix}{sampleDir}_$(InputListName).out
Error      = {jobLogDir}/{file_prefix}{sampleDir}_$(InputListName).err
RequestMemory = 5 GB
RequestCpus = 1
# 필요한 환경 파일 전송
should_transfer_files = YES
when_to_transfer_output = ON_EXIT
transfer_input_files = {runScriptPath}
# CMSSW 환경 설정 스크립트를 통해 실행
Arguments  = {MainPath} {runPeriod} {studyName} {channel} {configFile} {branchList} {maxEvents} {outputPath} {sampleDir}/{sampleDir}_$(InputListName)
Queue InputListName from (
"""

        # List to track specific files for resubmission
        resubmit_list = []
        for listFile in os.listdir(sampleInputPath):
            if listFile.endswith(".list"):
                listFileName = os.path.splitext(listFile)[0]
                
                # Check if the filename ends with a number (e.g. sample_1, sample_2, etc)
                # 본 파일 이름이 이미 sampleDir_숫자 형식인 경우 처리
                if listFileName == sampleDir or not listFileName.startswith(f"{sampleDir}_"):
                    if debug:
                        print(f"Skipping file that doesn't match pattern: {listFileName}")
                    continue
                
                # Extract just the number part from listFileName (remove the sampleDir_ prefix)
                number_part = listFileName[len(sampleDir)+1:]  # +1 for the underscore
                
                # 숫자 패턴이 있는 파일만 처리
                if not number_part.isdigit():
                    if debug:
                        print(f"Skipping file without numeric suffix: {listFileName}")
                    continue
                
                # For resubmission, only include missing, small, and correctly formatted root files
                if resubmit and needs_resubmission(outputPath, sampleDir, listFileName):
                    resubmit_list.append(number_part)
                elif not resubmit:
                    submit_file_content += f"{number_part}\n"

        # Only create submission content if there are files for resubmission
        if resubmit and resubmit_list:
            for file_name in resubmit_list:
                submit_file_content += f"{file_name}\n"
                
            submit_file_content += ")"
            
            submit_file_path = os.path.join(submitDir, f"submit_{sampleDir}.sub")
            with open(submit_file_path, "w") as submit_file:
                submit_file.write(submit_file_content)
            
            if debug:
                print(f"Submitting Condor job for {sampleDir} with resubmission list: {resubmit_list}")
            subprocess.run(["condor_submit", submit_file_path])
            if debug:
                print(f"Condor job submitted for {sampleDir}")
        elif not resubmit:
            submit_file_content += ")"
            
            submit_file_path = os.path.join(submitDir, f"submit_{sampleDir}.sub")
            with open(submit_file_path, "w") as submit_file:
                submit_file.write(submit_file_content)
            
            if debug:
                print(f"Submitting Condor job for {sampleDir}")
            subprocess.run(["condor_submit", submit_file_path])
            if debug:
                print(f"Condor job submitted for {sampleDir}")

# 수정된 메인 코드
MainPath = "/u/user/sha/Develop/CPviolation/SSB/AnalysisCode/ForRun2UL/NanoAOD/2025_0515_v1/CMSSW_13_3_0/src/v2/SSBNanoAODANCode"
inputList = os.path.join(MainPath, "input/")
logDir = os.path.join(os.getcwd(), "condorLog")
submitDir = os.path.join(os.getcwd(), "condorSubmit")
runScriptPath = os.path.join(MainPath, "run_cmd_v7.sh")
outputPath = "/pnfs/knu.ac.kr/data/cms/store/user/sha/CPV_Run2/ULSummer20"
runPeriod = "UL2018"
studyName = "NanoAOD_v1"
channel = "MuMu"
maxEvents = "-1"
samples = ["TTbar_Signal"]
samples = ["Data_SingleMuon_Run2018A", "Data_SingleMuon_Run2018B", "Data_SingleMuon_Run2018C", "Data_SingleMuon_Run2018D", "Data_DoubleMuon_Run2018A", "Data_DoubleMuon_Run2018B", "Data_DoubleMuon_Run2018C", "Data_DoubleMuon_Run2018D"
        ]
samples = [
    "DYJetsToLL_M_10To50",
#    "DYJetsToLL_M_10To50_madgraphMLM",
    "DYJetsToLL_M_50",
#    "DYJetsToLL_M_50_madgraphMLM",
    "Data_DoubleMuon_Run2018A",
    "Data_DoubleMuon_Run2018B",
    "Data_DoubleMuon_Run2018C",
    "Data_DoubleMuon_Run2018D",
#    "Data_SingleElectron_Run2018A",
#    "Data_SingleElectron_Run2018B",
#    "Data_SingleElectron_Run2018C",
#    "Data_SingleElectron_Run2018D",
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
#    "TTbar_AllHadronic",
#    "TTbar_SemiLeptonic",
    "TTbar_Signal",
    "WJetsToLNu",
    "WJetsToLNu_madgraphMLM",
    "WW",
    "WZ",
    "ZZ"
]
branchList = "UL2018/branch_list.txt"
configFile = "dimuon.config"
debug = True  # Set this to False to turn off debugging output

# 함수 호출 수정
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

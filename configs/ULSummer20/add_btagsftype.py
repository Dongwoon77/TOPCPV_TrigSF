import os

def add_btagsftype_to_config(directory):
    """
    지정된 디렉토리와 그 하위 디렉토리의 모든 .config 파일에
    'BTagSFType' 설정이 없는 경우 추가하고, 결과를 출력합니다.
    """
    for root, _, files in os.walk(directory):
        for filename in files:
            if filename.endswith(".config"):
                filepath = os.path.join(root, filename)
                print(f"Processing {filepath}...")
                
                with open(filepath, 'r') as f:
                    lines = f.readlines()

                modified_lines = []
                btagsftype_action_taken = "none" # "added", "existed"

                i = 0
                while i < len(lines):
                    line = lines[i]
                    modified_lines.append(line)
                    
                    # 'Jet_btag'으로 시작하는 라인을 찾습니다.
                    # .strip()으로 앞뒤 공백을 제거하고 .startswith()로 정확히 시작하는지 확인합니다.
                    if line.strip().startswith("Jet_btag :") and btagsftype_action_taken == "none":
                        # 다음 라인에 'BTagSFType'이 있는지 확인합니다.
                        # 없으면 추가합니다.
                        if i + 1 < len(lines) and lines[i+1].strip().startswith("BTagSFType :"):
                            btagsftype_action_taken = "existed"
                        else:
                            # 현재 라인 다음에 BTagSFType이 없으면 추가
                            modified_lines.append('BTagSFType : "mujets" # mujets, comb (defalt is comb but BTV recommend mujets for ttbar dilepton analysis)\n')
                            btagsftype_action_taken = "added"
                    i += 1
                
                if btagsftype_action_taken == "added":
                    print(f"  -> '{filepath}' 파일에 'BTagSFType'을 추가했습니다.")
                    # 수정된 내용을 파일에 다시 쓰기
                    with open(filepath, 'w') as f:
                        f.writelines(modified_lines)
                elif btagsftype_action_taken == "existed":
                    print(f"  -> '{filepath}' 파일에 'BTagSFType'이 이미 있었습니다.")
                else:
                    print(f"  -> '{filepath}' 파일에서 'Jet_btag' 라인을 찾지 못했습니다. 'BTagSFType'은 변경되지 않았습니다.")

# 스크립트를 실행할 디렉토리 (현재 디렉토리 또는 'ULSummer20' 등으로 지정)
# 중요한 설정 파일들은 반드시 백업해두시는 것을 권장합니다!
target_directory = './' # 현재 스크립트가 실행되는 디렉토리
# target_directory = 'ULSummer20' # 특정 디렉토리를 지정할 경우

add_btagsftype_to_config(target_directory)
print("\n모든 Config 파일 업데이트가 완료되었습니다.")

# 2024 Dimuon & Dielec 채널 문제점 보고서

## 1. Config 불일치 (dimuon vs dielec)

### 1.1 CenOfEn 불일치
| config | CenOfEn |
|--------|---------|
| dimuon.config | `13.6TeV` |
| dielec.config | `13TeV` |

**문제:** 2024는 13.6 TeV 충돌 에너지. dielec이 13TeV로 잘못 설정됨.

**수정:** `dielec.config` → `CenOfEn : "13.6TeV"`

---

### 1.2 Rochester Correction
| config | applyRochester |
|--------|----------------|
| dimuon | `false` |
| dielec | `true` |

**설명:** dimuon은 muon 신호 레pton, dielec은 electron 신호. Rochester는 muon momentum 보정. dielec에서 veto muon에 적용되는지 확인 필요. 2024에서 Rochester 권장 여부 확인.

---

### 1.3 VetoMuonId / ApplyPUID
| config | VetoMuonId | ApplyPUID |
|--------|------------|-----------|
| dimuon | Loose | false |
| dielec | Tight | true |

**설명:** 채널별로 의도된 차이일 수 있음. dielec은 electron 채널이라 veto muon을 더 엄격히 할 수 있음.

---

## 2. JERName / JERResName - Summer23 vs Summer24

**파일:** `dimuon.config` (123-124행), `dielec.config` (126-127행)

**현재:**
```
JERName : "Summer23BPixPrompt23_RunD_JRV1_MC_ScaleFactor_AK4PFPuppi"
JERResName : "Summer23BPixPrompt23_RunD_JRV1_MC_PtResolution_AK4PFPuppi"
```

**문제:** JERPath/JERSFPath는 `JME/2024_Summer24/jet_jerc.json.gz`를 사용하는데, JER key는 **Summer23BPix**. 2024 Summer24 JSON에 Summer23BPix key가 있는지 확인 필요. 없으면 런타임 에러.

**조치:** `JME/2024_Summer24/jet_jerc.json.gz` 내부 correction 이름 확인 후, 2024용 key로 수정 (예: Summer24Prompt24 관련).

---

## 3. PileUp - UL2018 사용

**파일:** 둘 다 (24-26행)
```
PileUpMCFileName : "UL2018/MC_Pileup.root"
PileUpDATAFileName : "UL2018/Offcial_Data.root"
```

**문제:** 2024 config가 2018 PU 파일 사용. PUWeightPath는 `CorrectionFiles/puWeights_BCDEFGHI.json`으로 별도 설정되어 있어 correctionlib PU는 2024용일 수 있음.

**확인:** ROOT 기반 PU (PileUpMCFileName 등)가 실제로 사용되는지, 아니면 correctionlib PU만 쓰는지 코드 확인.

**Typo:** `Offcial` → `Official` (실제 파일명이 Offcial이면 유지)

---

## 4. Trigger SF - 2018 파일 사용

**파일:** 둘 다 (96-97행)
```
TrigSFFile : "TriggerSF_2018_ULv2.root"
```

**문제:** 2024 트리거가 2018과 다름. 2024 전용 트리거 SF가 있으면 교체 필요.

**트리거:**
- dimuon: `HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass3p8_v`, `HLT_IsoMu24_v`
- dielec: `HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL_v`, `HLT_DoubleEle33_CaloIdL_MW_v`, `HLT_Ele30_WPTight_Gsf_v`

---

## 5. Dielec 전용 - Electron Reco SF

### 5.1 ElecRecoSFName
**파일:** `dielec.config` (136-139행)
```
ElecRecoSFPath : "EGM/2024_Summer24/electron_v1.json.gz"
ElecRecoSFName : "Electron-ID-SF"
ElecRecoSFYear : "2024Prompt"
```

**확인:** `electron_v1.json`은 Reco SF용. correction 이름이 `Electron-ID-SF`가 맞는지 검증 필요. Reco SF용이면 `Electron-Reco-SF` 등 다른 이름일 수 있음.

---

## 6. Trigger 로직 (Analysis.cpp) - OK

**2024 dimuon:** `FileName_.Contains("Muon") && !Contains("MuonEG")` → Muon0, Muon1 ✓  
**2024 dielec:** `FileName_.Contains("EGamma")` → EGamma0, EGamma1 ✓

---

## 7. Submit 스크립트 - OK

- `submit_2024_mumu.py`: Muon0, Muon1, dimuon.config ✓
- `submit_2024_ee.py`: EGamma0, EGamma1, dielec.config ✓

---

## 8. xsecAndsample/2024.txt - OK

TTbar, DY, ST, Diboson 샘플 포함. FileName_과 키 일치.

---

## 요약 - 수정 우선순위

| # | 심각도 | 항목 | 파일 | 조치 |
|---|--------|------|------|------|
| 1 | **HIGH** | CenOfEn 13TeV | dielec.config | 13.6TeV로 수정 |
| 2 | **HIGH** | JERName/JERResName | dimuon, dielec | 2024 Summer24 JSON key 확인 후 수정 |
| 3 | MEDIUM | TrigSFFile 2018 | 둘 다 | 2024 SF 확인 |
| 4 | MEDIUM | PileUp UL2018 | 둘 다 | 2024 PU 확인 |
| 5 | LOW | ElecRecoSFName | dielec | electron_v1.json correction 이름 검증 |
| 6 | LOW | Offcial typo | 둘 다 | Official (파일명 확인 후) |

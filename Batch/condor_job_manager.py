#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Condor Job Manager
Comprehensive tool for monitoring, detecting stuck jobs, and automatic resubmission
"""

import os
import sys
import subprocess
import time
import datetime
import glob
import re
import json
from pathlib import Path

class CondorJobManager:
    def __init__(self, log_dir=None):
        self.log_dir = log_dir or os.getcwd()
        self.stuck_threshold_minutes = 30  # 30분 이상 진행이 없으면 stuck으로 판단
        self.stuck_jobs_history = []  # stuck된 job들의 히스토리
        
    def _detect_batch_root(self, submit_file: str = None):
        """Batch 루트 디렉토리(즉, 하위에 Data_* 등의 샘플 폴더가 있는 위치)를 추정"""
        # 1) log_dir 기준으로 Batch 존재 시: 그곳을 사용
        cand = os.path.join(self.log_dir, 'Batch')
        if os.path.isdir(cand):
            return cand
        
        # 2) submit_file이 주어졌다면, 상위로 올라가며 'Batch' 폴더를 찾음
        if submit_file:
            cur = os.path.abspath(os.path.dirname(submit_file))
            for _ in range(6):  # 안전하게 몇 단계만 상향 탐색
                if os.path.basename(cur) == 'Batch':
                    return cur
                parent = os.path.dirname(cur)
                if parent == cur:
                    break
                cur = parent
        
        # 3) 폴백: log_dir 자체를 사용
        return self.log_dir
        
    def get_running_jobs(self):
        """현재 running 중인 condor job들을 가져옴"""
        try:
            result = subprocess.run(['condor_q', '-format', '%s ', 'ClusterId', 
                                   '-format', '%s ', 'ProcId',
                                   '-format', '%s ', 'JobStatus',
                                   '-format', '%s ', 'Owner',
                                   '-format', '%s\n', 'Cmd'],
                                  capture_output=True, text=True, check=True)
            
            running_jobs = []
            for line in result.stdout.strip().split('\n'):
                if line.strip():
                    parts = line.strip().split()
                    if len(parts) >= 5 and parts[2] == '2':  # JobStatus 2 = Running
                        job_info = {
                            'cluster_id': parts[0],
                            'proc_id': parts[1],
                            'status': parts[2],
                            'owner': parts[3],
                            'cmd': ' '.join(parts[4:])
                        }
                        running_jobs.append(job_info)
            
            return running_jobs
        except subprocess.CalledProcessError as e:
            print(f"Error getting running jobs: {e}")
            return []
    
    def find_log_files(self, job_id):
        """특정 job의 log 파일들을 찾음"""
        # job_id는 "682688.0" 형태
        cluster_id = job_id.split('.')[0]
        
        # 모든 로그 파일을 검색
        log_files = []
        for root, dirs, files in os.walk(self.log_dir):
            for file in files:
                if file.endswith('.log'):
                    log_path = os.path.join(root, file)
                    try:
                        # 로그 파일의 첫 몇 줄을 읽어서 job ID 확인
                        with open(log_path, 'r') as f:
                            first_line = f.readline().strip()
                            # condor log format: 000 (682688.000.000) 2025-07-24 01:47:11
                            match = re.search(r'\((\d+)\.\d+\.\d+\)', first_line)
                            if match and match.group(1) == cluster_id:
                                log_files.append(log_path)
                    except Exception:
                        continue
        
        return log_files
    
    def analyze_log_file(self, log_file):
        """log 파일을 분석하여 작업 상태를 판단"""
        try:
            if not os.path.exists(log_file):
                return {'status': 'log_not_found', 'last_update': None, 'error': 'Log file not found'}
            
            # 파일의 마지막 수정 시간
            mtime = os.path.getmtime(log_file)
            last_modified = datetime.datetime.fromtimestamp(mtime)
            
            with open(log_file, 'r') as f:
                lines = f.readlines()
            
            if not lines:
                return {'status': 'empty_log', 'last_update': last_modified, 'error': 'Empty log file'}
            
            # 마지막 로그 라인 분석
            last_line = lines[-1].strip()
            
            # 작업 완료 확인
            if 'Job terminated' in last_line and 'Normal termination' in last_line:
                return {'status': 'completed', 'last_update': last_modified, 'exit_code': '0'}
            
            # 작업 실패 확인
            if 'Job terminated' in last_line and 'Normal termination' not in last_line:
                return {'status': 'failed', 'last_update': last_modified, 'error': last_line}
            
            # 마지막 업데이트 시간 찾기
            last_update = None
            for line in reversed(lines):
                if re.match(r'^\d{3}\s+\(\d+\.\d+\.\d+\)\s+\d{4}-\d{2}-\d{2}\s+\d{2}:\d{2}:\d{2}', line):
                    # condor log timestamp format: 000 (682688.000.000) 2025-07-24 01:47:11
                    match = re.search(r'(\d{4}-\d{2}-\d{2}\s+\d{2}:\d{2}:\d{2})', line)
                    if match:
                        last_update = datetime.datetime.strptime(match.group(1), '%Y-%m-%d %H:%M:%S')
                        break
            
            if last_update is None:
                last_update = last_modified
            
            # stuck 여부 판단
            time_diff = datetime.datetime.now() - last_update
            if time_diff.total_seconds() > self.stuck_threshold_minutes * 60:
                return {
                    'status': 'stuck',
                    'last_update': last_update,
                    'stuck_duration_minutes': int(time_diff.total_seconds() / 60),
                    'last_line': last_line
                }
            else:
                return {
                    'status': 'running',
                    'last_update': last_update,
                    'last_line': last_line
                }
                
        except Exception as e:
            return {'status': 'error', 'last_update': None, 'error': str(e)}
    
    def check_job_progress(self, job_info):
        """개별 job의 진행 상황을 체크"""
        job_id = f"{job_info['cluster_id']}.{job_info['proc_id']}"
        log_files = self.find_log_files(job_id)
        
        if not log_files:
            return {
                'job_id': job_id,
                'status': 'no_log_found',
                'error': 'No log files found for this job'
            }
        
        # 가장 최근 log 파일 사용
        latest_log = max(log_files, key=os.path.getmtime)
        log_analysis = self.analyze_log_file(latest_log)
        
        return {
            'job_id': job_id,
            'log_file': latest_log,
            'cmd': job_info['cmd'],
            **log_analysis
        }
    
    def extract_job_info_from_log(self, log_file):
        """로그 파일에서 job 정보 추출 (재제출용)"""
        try:
            with open(log_file, 'r') as f:
                first_line = f.readline().strip()
                # condor log format: 000 (682688.000.000) 2025-07-24 01:47:11
                match = re.search(r'\((\d+)\.\d+\.\d+\)', first_line)
                cluster_id = match.group(1) if match else None

            # 1) 파일명에서 sample과 job 번호를 직접 파싱 (보편적, 경로 패턴에 의존 X)
            file_name = os.path.basename(log_file)
            name_match = re.match(r'^test_(.+)_(\d+)\.log$', file_name)
            if name_match:
                sample_name = name_match.group(1)
                job_num = int(name_match.group(2))
                submit_dir = os.path.dirname(log_file)
                return {
                    'cluster_id': cluster_id,
                    'sample_name': sample_name,
                    'job_num': job_num,
                    'l1prefire_type': 'unknown',
                    'log_file': log_file,
                    'submit_dir': submit_dir,
                }

            # 2) 실패 시 기존 폴백 로직 사용 (특정 패턴)
            log_path = Path(log_file)
            parts = log_path.parts
            for i, part in enumerate(parts):
                if '_NanoAOD_v6_L1PreFire' in part:
                    sample_name = part.replace('_NanoAOD_v6_L1PreFire', '')
                    l1prefire_type = 'L1PreFire'
                elif 'non_L1PreFire' in part:
                    sample_name = part.replace('non_L1PreFire', '')
                    l1prefire_type = 'non_L1PreFire'
                else:
                    continue

                for j in range(i+1, len(parts)):
                    if parts[j].startswith('test_') and parts[j].endswith('.log'):
                        job_num_match = re.search(r'_(\d+)\.log$', parts[j])
                        if job_num_match:
                            job_num = int(job_num_match.group(1))
                            return {
                                'cluster_id': cluster_id,
                                'sample_name': sample_name,
                                'job_num': job_num,
                                'l1prefire_type': l1prefire_type,
                                'log_file': log_file
                            }
        except Exception as e:
            print(f"Error extracting job info from {log_file}: {e}")
        
        return None
    
    def find_submit_file(self, sample_name, job_num, l1prefire_type='L1PreFire', submit_dir=None):
        """submit 파일 찾기 (가능하면 로그와 같은 디렉토리 우선)"""
        # 1) 로그와 같은 디렉토리에서 직접 찾기
        if submit_dir:
            candidate = os.path.join(submit_dir, f"condor_{sample_name}_{job_num}.submit")
            if os.path.exists(candidate):
                return candidate

        # 2) 알려진 디렉토리 패턴들에서 탐색 (Batch 유무, MuMu/ElEl 채널 지원)
        candidate_patterns = [
            f"./Batch/{sample_name}_NanoAOD_v6p2_RUN3/*/MuMu/{sample_name}/condor_{sample_name}_{job_num}.submit",
            f"./{sample_name}_NanoAOD_v6p2_RUN3/*/MuMu/{sample_name}/condor_{sample_name}_{job_num}.submit",
            f"./Batch/{sample_name}_NanoAOD_v6p2_RUN3/*/ElEl/{sample_name}/condor_{sample_name}_{job_num}.submit",
            f"./{sample_name}_NanoAOD_v6p2_RUN3/*/ElEl/{sample_name}/condor_{sample_name}_{job_num}.submit",
            f"./Batch/{sample_name}_NanoAOD_v6_L1PreFire/*/MuMu/{sample_name}/condor_{sample_name}_{job_num}.submit",
            f"./{sample_name}_NanoAOD_v6_L1PreFire/*/MuMu/{sample_name}/condor_{sample_name}_{job_num}.submit",
            f"./Batch/{sample_name}non_L1PreFire/*/MuMu/{sample_name}/condor_{sample_name}_{job_num}.submit",
            f"./{sample_name}non_L1PreFire/*/MuMu/{sample_name}/condor_{sample_name}_{job_num}.submit",
            # 가장 일반적인 폴백: MuMu 또는 ElEl
            f"./**/MuMu/{sample_name}/condor_{sample_name}_{job_num}.submit",
            f"./**/ElEl/{sample_name}/condor_{sample_name}_{job_num}.submit",
        ]

        for pattern in candidate_patterns:
            submit_files = glob.glob(pattern, recursive=True)
            if submit_files:
                return submit_files[0]

        return None
    
    def resubmit_job(self, job_info):
        """개별 job 재제출"""
        sample_name = job_info['sample_name']
        job_num = job_info['job_num']
        l1prefire_type = job_info.get('l1prefire_type', 'unknown')
        submit_dir = job_info.get('submit_dir')

        submit_file = self.find_submit_file(sample_name, job_num, l1prefire_type, submit_dir)

        if not submit_file:
            print(f"  ❌ Submit file not found for {sample_name}_{job_num} ({l1prefire_type})")
            return False

        try:
            batch_root = self._detect_batch_root(submit_file)
            submit_relpath = os.path.relpath(submit_file, start=batch_root)
            result = subprocess.run(['condor_submit', submit_relpath],
                                  cwd=batch_root,
                                  capture_output=True, text=True, check=True)
            print(f"  ✅ Successfully resubmitted {sample_name}_{job_num}")
            return True
        except subprocess.CalledProcessError as e:
            err_msg = e.stderr.strip() if hasattr(e, 'stderr') and e.stderr else str(e)
            print(f"  ❌ Failed to resubmit {sample_name}_{job_num}: {err_msg}")
            return False
    
    def monitor_and_manage_jobs(self, auto_resubmit=False):
        """모든 running job들을 모니터링하고 관리"""
        print("=== Condor Job Manager ===")
        print(f"Monitoring jobs at: {datetime.datetime.now()}")
        print(f"Stuck threshold: {self.stuck_threshold_minutes} minutes")
        print(f"Auto resubmit: {auto_resubmit}")
        print()
        
        running_jobs = self.get_running_jobs()
        
        if not running_jobs:
            print("No running jobs found.")
            return
        
        print(f"Found {len(running_jobs)} running jobs:")
        print("-" * 80)
        
        stuck_jobs = []
        problematic_jobs = []
        jobs_to_resubmit = []
        
        for job_info in running_jobs:
            job_status = self.check_job_progress(job_info)
            
            print(f"Job ID: {job_status['job_id']}")
            print(f"Command: {job_status.get('cmd', 'N/A')}")
            print(f"Status: {job_status['status']}")
            
            if job_status['status'] == 'stuck':
                print(f"⚠️  STUCK: No progress for {job_status['stuck_duration_minutes']} minutes")
                print(f"   Last update: {job_status['last_update']}")
                print(f"   Log file: {job_status['log_file']}")
                stuck_jobs.append(job_status)
                
                # 재제출을 위한 정보 추출
                job_info = self.extract_job_info_from_log(job_status['log_file'])
                if job_info:
                    jobs_to_resubmit.append(job_info)
                    
            elif job_status['status'] in ['failed', 'error', 'no_log_found']:
                print(f"❌ PROBLEM: {job_status.get('error', 'Unknown error')}")
                problematic_jobs.append(job_status)
            elif job_status['status'] == 'running':
                print(f"✅ Running normally")
                print(f"   Last update: {job_status['last_update']}")
            elif job_status['status'] == 'completed':
                print(f"✅ Completed successfully")
            
            print("-" * 80)
        
        # 요약
        print("\n=== SUMMARY ===")
        print(f"Total running jobs: {len(running_jobs)}")
        print(f"Stuck jobs: {len(stuck_jobs)}")
        print(f"Problematic jobs: {len(problematic_jobs)}")
        print(f"Normal jobs: {len(running_jobs) - len(stuck_jobs) - len(problematic_jobs)}")
        
        if stuck_jobs:
            print("\n=== STUCK JOBS ===")
            for job in stuck_jobs:
                print(f"Job {job['job_id']}: Stuck for {job['stuck_duration_minutes']} minutes")
                print(f"  Log: {job['log_file']}")
        
        if problematic_jobs:
            print("\n=== PROBLEMATIC JOBS ===")
            for job in problematic_jobs:
                print(f"Job {job['job_id']}: {job.get('error', 'Unknown error')}")
        
        # 재제출 처리
        if jobs_to_resubmit:
            print(f"\n=== JOBS TO RESUBMIT ({len(jobs_to_resubmit)}) ===")
            for job_info in jobs_to_resubmit:
                print(f"  {job_info['sample_name']}_{job_info['job_num']} (Cluster: {job_info['cluster_id']})")
            
            if auto_resubmit:
                print(f"\n🔄 Auto-resubmitting {len(jobs_to_resubmit)} stuck jobs...")
                
                # 먼저 stuck된 job들을 kill
                print("Killing stuck jobs...")
                for job in stuck_jobs:
                    job_id = job['job_id']
                    try:
                        result = subprocess.run(['condor_rm', job_id], capture_output=True, text=True)
                        if result.returncode == 0:
                            print(f"  ✅ Killed job {job_id}")
                        else:
                            print(f"  ❌ Failed to kill job {job_id}")
                    except Exception as e:
                        print(f"  ❌ Error killing job {job_id}: {e}")
                
                # 잠시 기다린 후 재제출
                print("Waiting 10 seconds before resubmitting...")
                time.sleep(10)
                
                # 재제출
                print("Resubmitting jobs...")
                success_count = 0
                for job_info in jobs_to_resubmit:
                    if self.resubmit_job(job_info):
                        success_count += 1
                
                print(f"\n✅ Successfully resubmitted {success_count}/{len(jobs_to_resubmit)} jobs")
                
                # 히스토리에 추가
                self.stuck_jobs_history.append({
                    'timestamp': datetime.datetime.now().isoformat(),
                    'killed_jobs': len(stuck_jobs),
                    'resubmitted_jobs': success_count,
                    'jobs': [job_info['sample_name'] + '_' + str(job_info['job_num']) for job_info in jobs_to_resubmit]
                })
                
            else:
                print("\n💡 Use --auto-resubmit to automatically kill and resubmit stuck jobs")
        
        return {
            'total': len(running_jobs),
            'stuck': stuck_jobs,
            'problematic': problematic_jobs,
            'normal': len(running_jobs) - len(stuck_jobs) - len(problematic_jobs),
            'to_resubmit': jobs_to_resubmit
        }
    
    def show_history(self):
        """재제출 히스토리 표시"""
        if not self.stuck_jobs_history:
            print("No resubmission history found.")
            return
        
        print("=== Resubmission History ===")
        for entry in self.stuck_jobs_history:
            print(f"Time: {entry['timestamp']}")
            print(f"Killed: {entry['killed_jobs']} jobs")
            print(f"Resubmitted: {entry['resubmitted_jobs']} jobs")
            print(f"Jobs: {', '.join(entry['jobs'])}")
            print("-" * 40)

def main():
    import argparse
    
    parser = argparse.ArgumentParser(description='Comprehensive condor job manager')
    parser.add_argument('--log-dir', default=None, help='Directory containing log files')
    parser.add_argument('--threshold', type=int, default=30, help='Stuck threshold in minutes (default: 30)')
    parser.add_argument('--auto-resubmit', action='store_true', help='Automatically kill and resubmit stuck jobs')
    parser.add_argument('--continuous', action='store_true', help='Monitor continuously every 5 minutes')
    parser.add_argument('--history', action='store_true', help='Show resubmission history')
    
    args = parser.parse_args()
    
    manager = CondorJobManager(args.log_dir)
    manager.stuck_threshold_minutes = args.threshold
    
    if args.history:
        manager.show_history()
        return
    
    if args.continuous:
        print("Starting continuous monitoring... Press Ctrl+C to stop")
        try:
            while True:
                manager.monitor_and_manage_jobs(args.auto_resubmit)
                print(f"\nNext check in 5 minutes... ({datetime.datetime.now()})")
                time.sleep(300)  # 5 minutes
        except KeyboardInterrupt:
            print("\nMonitoring stopped by user.")
    else:
        manager.monitor_and_manage_jobs(args.auto_resubmit)

if __name__ == "__main__":
    main() 
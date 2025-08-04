#!/usr/bin/env python3
import os
import glob

def modify_config_file(filepath):
    """
    Modify a single config file by replacing MET_cut line with extended configuration
    """
    try:
        with open(filepath, 'r') as f:
            lines = f.readlines()
        
        modified_lines = []
        i = 0
        while i < len(lines):
            line = lines[i]
            
            # Check if this line contains MET_cut
            if 'MET_cut' in line and ':' in line:
                # Find the MET requirement comment line above (if exists)
                met_comment_idx = -1
                for j in range(i-1, max(-1, i-5), -1):  # Look back up to 5 lines
                    if '####### MET requirement #######' in lines[j]:
                        met_comment_idx = j
                        break
                
                # If we found the comment, replace from comment to MET_cut line
                if met_comment_idx >= 0:
                    # Add lines before the MET comment
                    modified_lines.extend(lines[:met_comment_idx])
                    
                    # Add the new configuration block
                    modified_lines.append('####### MET requirement #######\n')
                    modified_lines.append('MET_cut : 40.0\n')
                    modified_lines.append('\n')
                    modified_lines.append('####### MET Algoritm #######\n')
                    modified_lines.append('METtype : "Puppi" ## PF or Puppi\n')
                    modified_lines.append('applyMETXY : "False" ## "False" or "True"\n')
                    modified_lines.append('\n')
                    modified_lines.append('####### Rochester Correction #######\n')
                    modified_lines.append('applyRochester : "True"  ## "False" or "True"\n')
                    
                    # Skip to the line after MET_cut
                    i = i + 1
                else:
                    # If no comment found, just replace the MET_cut line
                    modified_lines.extend(lines[:i])
                    
                    # Add the new configuration block
                    modified_lines.append('####### MET requirement #######\n')
                    modified_lines.append('MET_cut : 40.0\n')
                    modified_lines.append('\n')
                    modified_lines.append('####### MET Algoritm #######\n')
                    modified_lines.append('METtype : "Puppi" ## PF or Puppi\n')
                    modified_lines.append('applyMETXY : "False" ## "False" or "True"\n')
                    modified_lines.append('\n')
                    modified_lines.append('####### Rochester Correction #######\n')
                    modified_lines.append('applyRochester : "True"  ## "False" or "True"\n')
                    
                    # Skip the original MET_cut line
                    i = i + 1
            else:
                modified_lines.append(line)
                i += 1
        
        # Write the modified content back to file
        with open(filepath, 'w') as f:
            f.writelines(modified_lines)
        
        print(f"✓ Modified: {filepath}")
        return True
        
    except Exception as e:
        print(f"✗ Error modifying {filepath}: {e}")
        return False

def main():
    """
    Main function to find and modify all config files
    """
    base_dir = "."  # Current directory
    
    # Define directories to search
    directories = [
        "UL2016PostVFP",
        "UL2016PreVFP", 
        "UL2017",
        "UL2018"
    ]
    
    # Also check for analysis_config.config in current directory
    config_files = []
    
    # Add main config file if it exists
    if os.path.exists("analysis_config.config"):
        config_files.append("analysis_config.config")
    
    # Find all .config files in subdirectories
    for directory in directories:
        if os.path.exists(directory):
            pattern = os.path.join(directory, "*.config")
            config_files.extend(glob.glob(pattern))
    
    if not config_files:
        print("No .config files found!")
        return
    
    print(f"Found {len(config_files)} config files:")
    for f in config_files:
        print(f"  - {f}")
    
    print("\nModifying files...")
    
    success_count = 0
    for config_file in config_files:
        if modify_config_file(config_file):
            success_count += 1
    
    print(f"\nCompleted: {success_count}/{len(config_files)} files modified successfully")

if __name__ == "__main__":
    main()

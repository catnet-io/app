import json
import sys

def parse_check_runs(file_path):
    """
    Parse a JSON file containing GitHub API 'check-runs' response
    and print the status, name, and output of each check run.
    """
    try:
        # Use utf-16 as PowerShell's ConvertTo-Json often outputs UTF-16 LE
        with open(file_path, encoding='utf-16') as f:
            d = json.load(f)
        for c in d.get('check_runs', []):
            print(f"[{c.get('conclusion', 'in_progress')}] {c.get('name')} - ID: {c.get('id')}")
            if c.get('conclusion') == 'failure':
                output = c.get('output', {})
                print(f"  Annotations URL: {output.get('annotations_url')}")
    except Exception as e:
        print(f"Error parsing check runs: {e}")

def parse_annotations(file_path):
    """
    Parse a JSON file containing GitHub API annotations response
    to pinpoint the exact failing line and error message.
    """
    try:
        with open(file_path, encoding='utf-16') as f:
            data = json.load(f)
            
        # Sometimes ConvertTo-Json encapsulates the list in a 'value' dict depending on Depth
        annotations = data.get('value', data) if isinstance(data, dict) else data
        
        for ann in annotations:
            print(f"[{ann.get('annotation_level')}] {ann.get('path')}:{ann.get('start_line')} - {ann.get('message')}")
    except Exception as e:
        print(f"Error parsing annotations: {e}")

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print("Usage: python parse_github_api.py [check_runs|annotations] <file.json>")
        sys.exit(1)
        
    mode = sys.argv[1]
    file_path = sys.argv[2]
    
    if mode == 'check_runs':
        parse_check_runs(file_path)
    elif mode == 'annotations':
        parse_annotations(file_path)

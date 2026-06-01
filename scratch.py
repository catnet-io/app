import json

with open(r'C:\Users\fabio\.gemini\antigravity-ide\brain\b277298b-955f-4cf3-a510-bc3d2153775b\.system_generated\steps\151\content.md', encoding='utf-8') as f:
    text = f.read().split('---')[1].strip()
    
d = json.loads(text)
for c in d['check_runs']:
    print(f"{c['name']}: {c['conclusion']}")
    if c['conclusion'] == 'failure':
        print(c['output'])

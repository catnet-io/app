import urllib.request
import json

url = "https://api.github.com/repos/mendsec/catnet_scanner/pulls?state=all&per_page=100"
req = urllib.request.Request(url)
req.add_header('User-Agent', 'Python-urllib/3.x')

try:
    with urllib.request.urlopen(req) as response: # nosemgrep: python.lang.security.audit.dynamic-urllib-use-detected.dynamic-urllib-use-detected
        data = json.loads(response.read().decode())
        with open('docs/ci_troubleshooting/PR_HISTORY.md', 'w', encoding='utf-8') as f:
            f.write("# Histórico de Pull Requests\n\n")
            f.write("Abaixo consta o histórico de todas as PRs analisadas para referência de troubleshooting:\n\n")
            for pr in data:
                f.write(f"### PR #{pr['number']}: {pr['title']}\n")
                f.write(f"- **Status:** {pr['state']}\n")
                if pr.get('body'):
                    # Truncate body to keep it clean
                    body_text = pr['body'].split('\n')[0][:100]
                    f.write(f"- **Descrição:** {body_text}...\n")
                f.write("\n")
        print("PR_HISTORY.md created successfully.")
except Exception as e:
    print(f"Error: {e}")

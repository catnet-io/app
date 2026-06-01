import json
try:
    with open('annotations.json', encoding='utf-16') as f:
        data = json.load(f)
    print(data)
except Exception as e:
    print(e)
except Exception as e:
    print(e)

import os
from collections import defaultdict

guards = defaultdict(list)
for root, _, files in os.walk("../lib"):
    for f in files:
        if f.endswith((".h", ".hpp")):
            path = os.path.join(root, f)
            with open(path, "r", encoding="utf8", errors="ignore") as file:
                for line in file:
                    if line.strip().startswith("#define"):
                        parts = line.strip().split()
                        if len(parts) == 2 and parts[1].endswith(("_H", "_HPP")):
                            guards[parts[1]].append(path)
                        break

duplicates = {k:v for k,v in guards.items() if len(v)>1}

if not duplicates:
    print("✅ No hay include guards duplicados.")
else:
    print("⚠️ Include guards duplicados encontrados:\n")
    for guard, files in duplicates.items():
        print(f"{guard}:")
        for f in files:
            print(f"  - {f}")

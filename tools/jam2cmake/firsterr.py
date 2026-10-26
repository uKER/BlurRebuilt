#!/usr/bin/env python3
"""firsterr.py <msbuild.log> [max-per-target] — first error per (target, file).

Groups by the file the error is *in* (header or source), which is what you
need to fix; errsum.py groups by error kind instead.
"""
import collections
import re
import sys

log = open(sys.argv[1], encoding='utf-8', errors='replace').read().splitlines()
limit = int(sys.argv[2]) if len(sys.argv) > 2 else 40
rx = re.compile(r'^(.*?)\((\d+),\d+\): error (C\d+): (.*?) \[.*?[\\/](\w+)\.vcxproj\]')
seen = collections.OrderedDict()
for l in log:
    m = rx.search(l)
    if not m:
        continue
    f = re.sub(r'^C:.AIProjects.Blur2PC.', '', m.group(1))
    key = (m.group(5), f)
    if key not in seen:
        seen[key] = (m.group(2), m.group(3), m.group(4)[:120])
by = collections.defaultdict(list)
for (t, f), v in seen.items():
    by[t].append((f,) + v)
print('targets with errors:', len(by), '| files with errors:', len(seen))
for t in sorted(by, key=lambda t: -len(by[t])):
    print(f'\n### {t}: {len(by[t])} files')
    for f, line, code, msg in by[t][:limit]:
        print(f'  {f}({line}): {code} {msg}')

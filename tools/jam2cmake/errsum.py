#!/usr/bin/env python3
"""errsum.py <msbuild.log> [N] — group MSVC/link errors in a build log by kind."""
import collections
import re
import sys

log = open(sys.argv[1], encoding='utf-8', errors='replace').read().splitlines()
errs = [l for l in log if re.search(r'error C\d+|error LNK|fatal error', l)]
# MSBuild prints its summary in the system locale, so judge by error lines only.
print('total error lines:', len(errs), '| no errors' if not errs else '| BUILD FAILED')
c = collections.Counter()
samples = {}
files = collections.Counter()
for l in errs:
    m = re.search(r'(error C\d+|error LNK\d+|fatal error [A-Z]+\d+): (.*?)(\[|$)', l)
    if m:
        key = m.group(1) + ': ' + re.sub(r"'[^']*'", "'X'", m.group(2))[:100]
        c[key] += 1
        samples.setdefault(key, l)
    fm = re.match(r'\s*(.*?)\(\d+', l)
    if fm:
        files[re.sub(r'^.*Blur2PC.', '', fm.group(1))] += 1
n = int(sys.argv[2]) if len(sys.argv) > 2 else 25
for k, cnt in c.most_common(n):
    print(f'{cnt:4d}  {k}')
    print('        e.g.', re.sub(r'C:.AIProjects.Blur2PC.', '', samples[k])[:230])
print('\nfiles with most errors:')
for f, cnt in files.most_common(12):
    print(f'{cnt:4d}  {f}')

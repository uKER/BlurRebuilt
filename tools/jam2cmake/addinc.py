#!/usr/bin/env python3
"""addinc.py <file> <include-line> [<after-substring>]

Insert an include line (with the [Blur2PC] marker appended if absent) into a
source file: after the first line containing <after-substring>, or after the
first existing #include line. Idempotent: does nothing if the line is present.
Keeps the file's original line endings and encoding (latin-1 passthrough).
"""
import re
import sys


def main():
    path, line = sys.argv[1], sys.argv[2]
    anchor = sys.argv[3] if len(sys.argv) > 3 else None
    if 'Blur2PC' not in line:
        line = line + '\t// [Blur2PC]'
    s = open(path, encoding='latin-1', newline='').read()
    if line.split('//')[0].strip() in s:
        print('present', path)
        return
    nl = '\r\n' if '\r\n' in s else '\n'
    lines = s.split(nl)
    idx = None
    for i, l in enumerate(lines):
        if anchor and anchor in l:
            idx = i
            break
        if not anchor and re.match(r'\s*#\s*include', l):
            idx = i
            break
    if idx is None:
        print('NO ANCHOR', path)
        sys.exit(1)
    lines.insert(idx + 1, line)
    open(path, 'w', encoding='latin-1', newline='').write(nl.join(lines))
    print('patched', path)


if __name__ == '__main__':
    main()

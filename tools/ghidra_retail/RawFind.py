# Searches all memory for each text in BLUR2PC_TEXTS (|-separated) as raw ASCII and UTF-16LE (exact case,
# no terminator required) and prints each hit with the surrounding printable text and any code references.
import os, jpype
mem = currentProgram.getMemory(); fm = currentProgram.getFunctionManager(); rm = currentProgram.getReferenceManager()
out = open(os.environ["BLUR2PC_OUT"], "w")
def find_all(bs):
    res = []; start = mem.getMinAddress()
    jb = jpype.JArray(jpype.JByte)([b if b < 128 else b - 256 for b in bs])
    while True:
        a = mem.findBytes(start, jb, None, True, monitor)
        if a is None: break
        res.append(a); start = a.add(1)
    return res
def around(a, step):
    s = a
    for i in range(64):
        p = s.subtract(step)
        c = mem.getByte(p) & 0xff
        if c < 0x20 or c > 0x7e: break
        s = p
    b = bytearray(); p = s
    for i in range(160):
        c = mem.getByte(p) & 0xff
        if c < 0x20 or c > 0x7e: break
        b.append(c); p = p.add(step)
    return s, b.decode("ascii")
for t in os.environ["BLUR2PC_TEXTS"].split("|"):
    for enc, step in (("ascii", 1), ("utf-16-le", 2)):
        for a in find_all(t.encode(enc)):
            s, txt = around(a, step)
            refs = ["%s(%s)" % (r.getFromAddress(), fm.getFunctionContaining(r.getFromAddress())) for r in rm.getReferencesTo(s)]
            out.write("%s %s at %s (string %s): '%s' refs %s\n" % (t, enc, a, s, txt, refs))
out.close()

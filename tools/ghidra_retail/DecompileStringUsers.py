# For each string in BLUR2PC_STRINGS (| separated), finds it in memory, then decompiles every
# function that references it (directly or via a pointer to it). Output to BLUR2PC_OUT.
import os, jpype
from ghidra.app.decompiler import DecompInterface
from ghidra.util.task import ConsoleTaskMonitor
mem = currentProgram.getMemory(); fm = currentProgram.getFunctionManager(); rm = currentProgram.getReferenceManager()
di = DecompInterface(); di.openProgram(currentProgram)
out = open(os.environ["BLUR2PC_OUT"], "w")
def find_all(bs):
    res = []; start = mem.getMinAddress()
    jb = jpype.JArray(jpype.JByte)([b if b < 128 else b - 256 for b in bs])
    while True:
        a = mem.findBytes(start, jb, None, True, monitor)
        if a is None: break
        res.append(a); start = a.add(1)
    return res
done = set()
for s in os.environ["BLUR2PC_STRINGS"].split("|"):
    for a in find_all(s.encode("ascii") + b"\x00"):
        # walk back to the string start (the search text may be a suffix)
        refs = list(rm.getReferencesTo(a))
        # also pointers to the string (Lua registration tables store pointers)
        le = bytes([(a.getOffset() >> k) & 0xff for k in (0, 8, 16, 24)])
        ptrs = find_all(le)
        out.write("== '%s' at %s: %d refs, %d pointers\n" % (s, a, len(refs), len(ptrs)))
        froms = [r.getFromAddress() for r in refs]
        for p in ptrs:
            out.write("   pointer at %s\n" % p)
            for r in rm.getReferencesTo(p): froms.append(r.getFromAddress())
            # Lua reg tables: {name, func}; record the neighbouring function pointer
            try:
                nxt = toAddr("%x" % (mem.getInt(p.add(4)) & 0xffffffff))
            except Exception:
                nxt = None
            if nxt is not None and fm.getFunctionAt(nxt):
                out.write("   paired function %s\n" % nxt); froms.append(nxt)
        for fa in froms:
            f = fm.getFunctionContaining(fa)
            if f is None or f.getEntryPoint() in done: continue
            done.add(f.getEntryPoint())
            r = di.decompileFunction(f, 300, ConsoleTaskMonitor())
            out.write("// ===== %s %s\n" % (f.getEntryPoint(), f.getName()))
            out.write(r.getDecompiledFunction().getC() if r.decompileCompleted() else "// failed\n")
out.close()

# Locates c_dfDIKeyboard (DIDATAFORMAT 0x18,0x10,2,0x100,0x100), the functions referencing it,
# and decompiles every function whose entry lies within +-0x1800 bytes of them.
import os, jpype
from ghidra.app.decompiler import DecompInterface
from ghidra.util.task import ConsoleTaskMonitor
mem = currentProgram.getMemory(); fm = currentProgram.getFunctionManager(); rm = currentProgram.getReferenceManager()
pat = bytes([0x18,0,0,0, 0x10,0,0,0, 2,0,0,0, 0,1,0,0, 0,1,0,0])
jb = jpype.JArray(jpype.JByte)([b if b < 128 else b - 256 for b in pat])
out = open(os.environ["BLUR2PC_OUT"], "w")
a = mem.findBytes(mem.getMinAddress(), jb, None, True, monitor)
out.write("c_dfDIKeyboard at %s\n" % a)
users = set()
for r in rm.getReferencesTo(a):
    f = fm.getFunctionContaining(r.getFromAddress())
    if f: users.add(f)
if not users:
    # no analysed reference: scan for the address as an immediate
    le = bytes([(a.getOffset() >> k) & 0xff for k in (0, 8, 16, 24)])
    jb2 = jpype.JArray(jpype.JByte)([b if b < 128 else b - 256 for b in le])
    s = mem.getMinAddress()
    while True:
        h = mem.findBytes(s, jb2, None, True, monitor)
        if h is None: break
        f = fm.getFunctionContaining(h)
        if f: users.add(f)
        out.write("  immediate at %s in %s\n" % (h, f.getEntryPoint() if f else None))
        s = h.add(1)
di = DecompInterface(); di.openProgram(currentProgram)
cands = set()
for u in users:
    lo = u.getEntryPoint().subtract(0x1800); hi = u.getEntryPoint().add(0x1800)
    for f in fm.getFunctions(lo, True):
        if f.getEntryPoint().compareTo(hi) > 0: break
        cands.add(f)
for f in sorted(cands, key=lambda f: f.getEntryPoint().getOffset()):
    r = di.decompileFunction(f, 120, ConsoleTaskMonitor())
    c = r.getDecompiledFunction().getC() if r.decompileCompleted() else ""
    if "0x100" in c or f in users:
        out.write("// ===== %s%s\n" % (f.getEntryPoint(), " (uses c_dfDIKeyboard)" if f in users else ""))
        out.write(c)
out.close()

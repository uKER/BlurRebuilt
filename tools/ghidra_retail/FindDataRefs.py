# Lists every instruction that references an address in [BLUR2PC_LO, BLUR2PC_HI) (as a Ghidra
# reference or as a scalar operand), grouped by function; decompiles those functions when
# BLUR2PC_DECOMPILE=1. Used to find who fills a global table.
import os
from ghidra.app.decompiler import DecompInterface
from ghidra.util.task import ConsoleTaskMonitor
from ghidra.program.model.scalar import Scalar
lo = int(os.environ["BLUR2PC_LO"], 16); hi = int(os.environ["BLUR2PC_HI"], 16)
fm = currentProgram.getFunctionManager(); listing = currentProgram.getListing()
out = open(os.environ["BLUR2PC_OUT"], "w")
hits = {}
for ins in listing.getInstructions(True):
    found = False
    for r in ins.getReferencesFrom():
        v = r.getToAddress().getOffset()
        if lo <= v < hi: found = True
    if not found:
        for i in range(ins.getNumOperands()):
            for o in ins.getOpObjects(i):
                if isinstance(o, Scalar) and lo <= (o.getValue() & 0xffffffff) < hi: found = True
    if found:
        f = fm.getFunctionContaining(ins.getAddress())
        key = f.getEntryPoint() if f else None
        hits.setdefault(key, []).append(ins)
for k, lst in hits.items():
    out.write("== %s\n" % k)
    for ins in lst: out.write("   %s  %s\n" % (ins.getAddress(), ins))
if os.environ.get("BLUR2PC_DECOMPILE") == "1":
    di = DecompInterface(); di.openProgram(currentProgram)
    for k in hits:
        if k is None: continue
        f = fm.getFunctionAt(k)
        r = di.decompileFunction(f, 300, ConsoleTaskMonitor())
        out.write("// ===== %s\n" % k)
        out.write(r.getDecompiledFunction().getC() if r.decompileCompleted() else "// failed\n")
out.close()

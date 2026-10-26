# Decompiles every function with an entry point in [BLUR2PC_LO, BLUR2PC_HI).
import os
from ghidra.app.decompiler import DecompInterface
from ghidra.util.task import ConsoleTaskMonitor
fm = currentProgram.getFunctionManager()
lo = toAddr(os.environ["BLUR2PC_LO"]); hi = toAddr(os.environ["BLUR2PC_HI"])
di = DecompInterface(); di.openProgram(currentProgram)
out = open(os.environ["BLUR2PC_OUT"], "w")
for f in fm.getFunctions(lo, True):
    if f.getEntryPoint().compareTo(hi) >= 0: break
    r = di.decompileFunction(f, 120, ConsoleTaskMonitor())
    out.write("// ===== %s\n" % f.getEntryPoint())
    out.write(r.getDecompiledFunction().getC() if r.decompileCompleted() else "// failed\n")
out.close()

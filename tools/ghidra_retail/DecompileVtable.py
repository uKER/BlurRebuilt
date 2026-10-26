# Decompiles the first BLUR2PC_N entries of the vtable at BLUR2PC_VT.
import os
from ghidra.app.decompiler import DecompInterface
from ghidra.util.task import ConsoleTaskMonitor
from ghidra.app.cmd.function import CreateFunctionCmd
mem = currentProgram.getMemory(); fm = currentProgram.getFunctionManager()
vt = toAddr(os.environ["BLUR2PC_VT"]); n = int(os.environ["BLUR2PC_N"])
di = DecompInterface(); di.openProgram(currentProgram)
out = open(os.environ["BLUR2PC_OUT"], "w")
for i in range(n):
    t = toAddr(mem.getInt(vt.add(i * 4)) & 0xffffffff)
    f = fm.getFunctionAt(t)
    if f is None:
        CreateFunctionCmd(t).applyTo(currentProgram); f = fm.getFunctionAt(t)
    out.write("// ===== slot %d -> %s\n" % (i, t))
    if f is None: continue
    r = di.decompileFunction(f, 120, ConsoleTaskMonitor())
    out.write(r.getDecompiledFunction().getC() if r.decompileCompleted() else "// failed\n")
out.close()

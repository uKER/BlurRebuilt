# Decompiles functions at BLUR2PC_ADDRS plus the targets of the 4-byte pointers stored at BLUR2PC_PTRS.
import os
from ghidra.app.decompiler import DecompInterface
from ghidra.util.task import ConsoleTaskMonitor
from ghidra.app.cmd.function import CreateFunctionCmd
mem = currentProgram.getMemory(); fm = currentProgram.getFunctionManager()
di = DecompInterface(); di.openProgram(currentProgram)
out = open(os.environ["BLUR2PC_OUT"], "w")
targets = [toAddr(a) for a in os.environ.get("BLUR2PC_ADDRS", "").split(",") if a]
for p in [a for a in os.environ.get("BLUR2PC_PTRS", "").split(",") if a]:
    t = toAddr(mem.getInt(toAddr(p)) & 0xffffffff); out.write("// pointer %s -> %s\n" % (p, t)); targets.append(t)
for t in targets:
    f = fm.getFunctionAt(t) or fm.getFunctionContaining(t)
    if f is None:
        CreateFunctionCmd(t).applyTo(currentProgram); f = fm.getFunctionAt(t)
    if f is None:
        out.write("// no function at %s\n" % t); continue
    r = di.decompileFunction(f, 300, ConsoleTaskMonitor())
    out.write("// ===== %s %s\n" % (f.getEntryPoint(), f.getName()))
    out.write(r.getDecompiledFunction().getC() if r.decompileCompleted() else "// failed\n")
out.close()

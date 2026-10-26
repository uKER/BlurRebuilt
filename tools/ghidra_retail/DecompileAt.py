# Decompiles the functions at the addresses in BLUR2PC_ADDRS (comma-separated hex) to BLUR2PC_OUT.
import os
from ghidra.app.decompiler import DecompInterface
from ghidra.util.task import ConsoleTaskMonitor
addrs = [a.strip() for a in os.environ["BLUR2PC_ADDRS"].split(",") if a.strip()]
out = open(os.environ["BLUR2PC_OUT"], "w")
di = DecompInterface(); di.openProgram(currentProgram)
fm = currentProgram.getFunctionManager()
for a in addrs:
    f = fm.getFunctionContaining(toAddr(a))
    if f is None:
        out.write("// no function at %s\n" % a); continue
    r = di.decompileFunction(f, 300, ConsoleTaskMonitor())
    out.write("// ===== %s %s\n" % (f.getEntryPoint(), f.getName()))
    out.write(r.getDecompiledFunction().getC() if r.decompileCompleted() else "// decompile failed: %s\n" % r.getErrorMessage())
out.close()

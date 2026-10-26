# Forces disassembly + a function definition at each address in BLUR2PC_ADDRS (comma-separated
# hex), for code auto-analysis never reached (e.g. only entered via a computed jump/return),
# then decompiles. In-memory only (-readOnly project keeps this from being saved back).
import os
from ghidra.app.decompiler import DecompInterface
from ghidra.util.task import ConsoleTaskMonitor
from ghidra.app.cmd.function import CreateFunctionCmd
out = open(os.environ["BLUR2PC_OUT"], "w")
addrs = [a.strip() for a in os.environ["BLUR2PC_ADDRS"].split(",") if a.strip()]
fm = currentProgram.getFunctionManager()
for a in addrs:
    t = toAddr(a)
    ok = disassemble(t)
    out.write("// disassemble(%s) -> %s\n" % (a, ok))
    f = fm.getFunctionAt(t) or fm.getFunctionContaining(t)
    if f is None:
        cmd = CreateFunctionCmd(t)
        cmd.applyTo(currentProgram)
        f = fm.getFunctionAt(t)
    if f is None:
        out.write("// still no function at %s\n" % a)
        # just list raw instructions forward
        listing = currentProgram.getListing()
        ins = listing.getInstructionAt(t)
        for i in range(40):
            if ins is None: break
            out.write("   %s  %s\n" % (ins.getAddress(), ins))
            ins = ins.getNext()
        continue
    di = DecompInterface(); di.openProgram(currentProgram)
    r = di.decompileFunction(f, 300, ConsoleTaskMonitor())
    out.write("// ===== %s %s\n" % (f.getEntryPoint(), f.getName()))
    out.write(r.getDecompiledFunction().getC() if r.decompileCompleted() else "// decompile failed: %s\n" % r.getErrorMessage())
out.close()

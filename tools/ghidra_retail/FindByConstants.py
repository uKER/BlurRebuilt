# Finds functions whose instructions use all the scalar constants in BLUR2PC_CONSTS (comma-separated),
# ranks by how many distinct target constants appear, and decompiles the top BLUR2PC_TOP matches.
import os
from ghidra.app.decompiler import DecompInterface
from ghidra.util.task import ConsoleTaskMonitor
from ghidra.program.model.scalar import Scalar
want = set(int(x, 0) for x in os.environ["BLUR2PC_CONSTS"].split(","))
top = int(os.environ.get("BLUR2PC_TOP", "2"))
fm = currentProgram.getFunctionManager(); listing = currentProgram.getListing()
rows = []
for f in fm.getFunctions(True):
    seen = set()
    for ins in listing.getInstructions(f.getBody(), True):
        for i in range(ins.getNumOperands()):
            for o in ins.getOpObjects(i):
                if isinstance(o, Scalar) and o.getValue() in want:
                    seen.add(o.getValue())
    if len(seen) >= len(want) - 1:
        rows.append((len(seen), f))
rows.sort(key=lambda r: -r[0])
out = open(os.environ["BLUR2PC_OUT"], "w")
for n, f in rows[:20]:
    out.write("%d/%d %s %s size %d\n" % (n, len(want), f.getEntryPoint(), f.getName(), f.getBody().getNumAddresses()))
di = DecompInterface(); di.openProgram(currentProgram)
for n, f in rows[:top]:
    r = di.decompileFunction(f, 300, ConsoleTaskMonitor())
    out.write("// ===== %s\n" % f.getEntryPoint())
    out.write(r.getDecompiledFunction().getC() if r.decompileCompleted() else "// failed\n")
out.close()

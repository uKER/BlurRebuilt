# Lists every instruction whose scalar operands (immediates and displacements) fall in
# [BLUR2PC_LO, BLUR2PC_HI), skipping the values in BLUR2PC_EXCLUDE (comma-separated), grouped by function.
import os
from ghidra.program.model.scalar import Scalar
lo = int(os.environ["BLUR2PC_LO"], 0); hi = int(os.environ["BLUR2PC_HI"], 0)
ex = set(int(x, 0) for x in os.environ.get("BLUR2PC_EXCLUDE", "").split(",") if x)
fm = currentProgram.getFunctionManager(); listing = currentProgram.getListing()
out = open(os.environ["BLUR2PC_OUT"], "w")
for f in fm.getFunctions(True):
    hits = []
    for ins in listing.getInstructions(f.getBody(), True):
        for i in range(ins.getNumOperands()):
            for o in ins.getOpObjects(i):
                if isinstance(o, Scalar) and lo <= o.getValue() < hi and o.getValue() not in ex:
                    hits.append("   %s  %s" % (ins.getAddress(), ins))
    if hits:
        out.write("== %s %s\n" % (f.getEntryPoint(), f.getName()))
        out.write("\n".join(sorted(set(hits))) + "\n")
out.close()

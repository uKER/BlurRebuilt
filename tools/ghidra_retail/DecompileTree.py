# Decompiles the functions at BLUR2PC_ADDRS and, breadth-first, every function they call, down to
# BLUR2PC_DEPTH levels (default 2). Functions listed in BLUR2PC_SKIP (comma-separated) and functions
# called from more than BLUR2PC_MAXREFS places (default 200; serialiser read/write helpers) are named but not expanded.
import os
from ghidra.app.decompiler import DecompInterface
from ghidra.util.task import ConsoleTaskMonitor
fm = currentProgram.getFunctionManager(); listing = currentProgram.getListing()
refs = currentProgram.getReferenceManager()
depth = int(os.environ.get("BLUR2PC_DEPTH", "2"))
maxrefs = int(os.environ.get("BLUR2PC_MAXREFS", "200"))
skip = set(int(x, 16) for x in os.environ.get("BLUR2PC_SKIP", "").split(",") if x)
di = DecompInterface(); di.openProgram(currentProgram)
out = open(os.environ["BLUR2PC_OUT"], "w")
seen = set()
level = [fm.getFunctionContaining(toAddr(a)) for a in os.environ["BLUR2PC_ADDRS"].split(",")]
for d in range(depth + 1):
    nxt = []
    for f in level:
        if f is None or f.getEntryPoint().getOffset() in seen:
            continue
        seen.add(f.getEntryPoint().getOffset())
        r = di.decompileFunction(f, 300, ConsoleTaskMonitor())
        out.write("// ===== %s %s (depth %d)\n" % (f.getEntryPoint(), f.getName(), d))
        out.write(r.getDecompiledFunction().getC() if r.decompileCompleted() else "// failed\n")
        for ins in listing.getInstructions(f.getBody(), True):
            if not ins.getFlowType().isCall():
                continue
            for t in ins.getFlows():
                g = fm.getFunctionAt(t)
                if g is None or g.getEntryPoint().getOffset() in seen or g.getEntryPoint().getOffset() in skip:
                    continue
                if refs.getReferenceCountTo(g.getEntryPoint()) > maxrefs:
                    continue
                nxt.append(g)
    level = nxt
out.close()

# For each call to BLUR2PC_TARGET, prints the caller and the 14 instructions before the call.
import os
fm = currentProgram.getFunctionManager(); listing = currentProgram.getListing(); rm = currentProgram.getReferenceManager()
t = toAddr(os.environ["BLUR2PC_TARGET"])
out = open(os.environ["BLUR2PC_OUT"], "w")
for r in rm.getReferencesTo(t):
    a = r.getFromAddress(); f = fm.getFunctionContaining(a)
    out.write("== call at %s in %s\n" % (a, f.getEntryPoint() if f else None))
    ins = listing.getInstructionAt(a); prev = []
    p = ins
    for i in range(int(os.environ.get("BLUR2PC_BACK","14"))):
        p = p.getPrevious()
        if p is None: break
        prev.append(p)
    for p in reversed(prev): out.write("   %s  %s\n" % (p.getAddress(), p))
    out.write("   %s  %s\n" % (ins.getAddress(), ins))
out.close()

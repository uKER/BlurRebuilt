# Lists every call instruction (address, target, pushed immediates just before) in the functions at BLUR2PC_ADDRS.
import os
fm = currentProgram.getFunctionManager(); listing = currentProgram.getListing()
out = open(os.environ["BLUR2PC_OUT"], "w")
for a in os.environ["BLUR2PC_ADDRS"].split(","):
    f = fm.getFunctionContaining(toAddr(a))
    out.write("== %s %s\n" % (f.getEntryPoint(), f.getName()))
    for ins in listing.getInstructions(f.getBody(), True):
        if ins.getFlowType().isCall():
            tg = [str(r.getToAddress()) for r in ins.getReferencesFrom() if r.getReferenceType().isCall()]
            out.write("  %s  %s  -> %s\n" % (ins.getAddress(), ins, ",".join(tg)))
out.close()

# Prints the instructions of the function containing each address in BLUR2PC_ADDRS to BLUR2PC_OUT.
import os
fm = currentProgram.getFunctionManager(); listing = currentProgram.getListing()
out = open(os.environ["BLUR2PC_OUT"], "w")
for a in os.environ["BLUR2PC_ADDRS"].split(","):
    f = fm.getFunctionContaining(toAddr(a))
    if f is None:
        out.write("// no function at %s\n" % a); continue
    out.write("== %s %s\n" % (f.getEntryPoint(), f.getName()))
    for ins in listing.getInstructions(f.getBody(), True):
        out.write("  %s  %s\n" % (ins.getAddress(), ins))
out.close()

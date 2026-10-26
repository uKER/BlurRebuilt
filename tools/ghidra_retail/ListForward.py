# Lists BLUR2PC_COUNT instructions in straight program order starting at BLUR2PC_ADDR,
# ignoring function/jump-table boundaries (walks the instruction listing via getNext()).
import os
listing = currentProgram.getListing()
out = open(os.environ["BLUR2PC_OUT"], "w")
ins = listing.getInstructionAt(toAddr(os.environ["BLUR2PC_ADDR"]))
n = int(os.environ.get("BLUR2PC_COUNT", "60"))
for i in range(n):
    if ins is None:
        out.write("<no instruction>\n"); break
    out.write("%s  %s\n" % (ins.getAddress(), ins))
    ins = ins.getNext()
out.close()

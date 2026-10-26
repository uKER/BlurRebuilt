# Lists symbols whose name matches (case-insensitive substring) any of BLUR2PC_NAMES (comma-separated).
import os
sym = currentProgram.getSymbolTable()
names = [n.strip().lower() for n in os.environ["BLUR2PC_NAMES"].split(",") if n.strip()]
out = open(os.environ["BLUR2PC_OUT"], "w")
it = sym.getAllSymbols(False)
for s in it:
    n = s.getName()
    ln = n.lower()
    if any(want in ln for want in names):
        out.write("%s  %s  %s\n" % (s.getAddress(), n, s.getSymbolType()))
out.close()

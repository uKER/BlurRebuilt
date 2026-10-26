# Case-insensitive substring search over defined Ghidra strings for each needle in BLUR2PC_NEEDLES
# (|-separated). Prints address, string, and referencing functions.
import os
listing = currentProgram.getListing(); fm = currentProgram.getFunctionManager(); rm = currentProgram.getReferenceManager()
needles = [n.lower() for n in os.environ["BLUR2PC_NEEDLES"].split("|") if n]
out = open(os.environ["BLUR2PC_OUT"], "w")
data = listing.getDefinedData(True)
for d in data:
    try:
        val = d.getValue()
    except Exception:
        continue
    if not isinstance(val, (str,)):
        continue
    lv = val.lower()
    if any(n in lv for n in needles):
        a = d.getAddress()
        refs = list(rm.getReferencesTo(a))
        out.write("%s: %r  (%d refs)\n" % (a, val, len(refs)))
        for r in refs:
            f = fm.getFunctionContaining(r.getFromAddress())
            out.write("   ref at %s in %s\n" % (r.getFromAddress(), f.getEntryPoint() if f else None))
out.close()

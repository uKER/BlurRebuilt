# Finds the MSVC RTTI type-descriptor string for BLUR2PC_CLASS (undecorated substring, e.g.
# "C_ReplayDirector") and lists functions referencing it (typically the vtable's type_info,
# reachable from the class's vftable and its destructor).
import os
listing = currentProgram.getListing(); fm = currentProgram.getFunctionManager(); rm = currentProgram.getReferenceManager()
mem = currentProgram.getMemory()
name = os.environ["BLUR2PC_CLASS"]
out = open(os.environ["BLUR2PC_OUT"], "w")
data = listing.getDefinedData(True)
for d in data:
    try:
        val = d.getValue()
    except Exception:
        continue
    if not isinstance(val, str):
        continue
    if name in val and val.startswith('.?A'):
        a = d.getAddress()
        out.write("RTTI type descriptor %s at %s\n" % (val, a))
        for r in rm.getReferencesTo(a):
            fr = r.getFromAddress()
            f = fm.getFunctionContaining(fr)
            out.write("  ref at %s in %s\n" % (fr, f.getEntryPoint() if f else None))
out.close()

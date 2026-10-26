# Finds the import symbol(s) for each name in BLUR2PC_IMPORTS (comma-separated, e.g.
# CreateMutexA,CreateMutexW), then for every call site prints the caller function and the
# BLUR2PC_BACK instructions before the call (default 14). Output to BLUR2PC_OUT.
import os
fm = currentProgram.getFunctionManager(); listing = currentProgram.getListing(); rm = currentProgram.getReferenceManager()
sym = currentProgram.getSymbolTable()
names = [n.strip() for n in os.environ["BLUR2PC_IMPORTS"].split(",") if n.strip()]
back = int(os.environ.get("BLUR2PC_BACK", "14"))
out = open(os.environ["BLUR2PC_OUT"], "w")
for name in names:
    syms = list(sym.getSymbols(name))
    if not syms:
        out.write("== %s: no symbol\n" % name); continue
    for s in syms:
        t = s.getAddress()
        refs = list(rm.getReferencesTo(t))
        out.write("== %s at %s (%d refs)\n" % (name, t, len(refs)))
        for r in refs:
            a = r.getFromAddress(); f = fm.getFunctionContaining(a)
            ins = listing.getInstructionAt(a)
            if ins is None:
                out.write("  data ref at %s in %s\n" % (a, f.getEntryPoint() if f else None))
                continue
            out.write("  call at %s in %s\n" % (a, f.getEntryPoint() if f else None))
            prev = []
            p = ins
            for i in range(back):
                p = p.getPrevious()
                if p is None: break
                prev.append(p)
            for p in reversed(prev): out.write("     %s  %s\n" % (p.getAddress(), p))
            out.write("     %s  %s\n" % (ins.getAddress(), ins))
out.close()

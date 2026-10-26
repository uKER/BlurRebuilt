# Lists functions by the most frequent single callee (call-site count), to locate
# long straight-line setup functions such as the input binding table.
from ghidra.program.model.symbol import RefType
fm = currentProgram.getFunctionManager()
listing = currentProgram.getListing()
rows = []
for f in fm.getFunctions(True):
    counts = {}
    for ins in listing.getInstructions(f.getBody(), True):
        if not ins.getFlowType().isCall():
            continue
        for ref in ins.getReferencesFrom():
            if ref.getReferenceType().isCall():
                t = ref.getToAddress()
                counts[t] = counts.get(t, 0) + 1
    if counts:
        t, n = max(counts.items(), key=lambda kv: kv[1])
        if n >= 40:
            rows.append((n, f.getEntryPoint(), f.getName(), t, len(counts)))
rows.sort(reverse=True)
out = open(r"C:\AIProjects\Blur2PC\_work\ghidra\heavy_callers.txt", "w")
for n, ep, name, t, k in rows[:60]:
    callee = fm.getFunctionAt(t)
    out.write("%5d  %s %-30s top callee %s %s (%d distinct)\n" % (n, ep, name, t, callee.getName() if callee else "?", k))
out.close()

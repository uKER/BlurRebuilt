# Finds the MSVC RTTI type descriptor for a class name, follows it to the complete object
# locator(s) and vtable(s), then lists the functions that reference each vtable (constructors,
# including inlined ones) with reference counts.
import os
from ghidra.program.model.address import AddressSet
mem = currentProgram.getMemory(); fm = currentProgram.getFunctionManager(); rm = currentProgram.getReferenceManager()
names = [n.strip() for n in os.environ["BLUR2PC_CLASSES"].split(",")]
out = open(os.environ["BLUR2PC_OUT"], "w")
def find_all(bytestr):
    res = []; start = mem.getMinAddress()
    pat = bytearray(bytestr)
    import jpype
    jb = jpype.JArray(jpype.JByte)([b if b < 128 else b - 256 for b in pat])
    while True:
        a = mem.findBytes(start, jb, None, True, monitor)
        if a is None: break
        res.append(a); start = a.add(1)
    return res
def le32(v):
    return bytes([(v >> s) & 0xff for s in (0, 8, 16, 24)])
for n in names:
    hits = find_all(n.encode("ascii") + b"\x00")
    out.write("== %s: %d string hits\n" % (n, len(hits)))
    for h in hits:
        td = h.subtract(8)  # TypeDescriptor: vftable ptr, spare, name
        out.write("  type descriptor %s\n" % td)
        for col in find_all(le32(td.getOffset())):
            colbase = col.subtract(12)  # COL: sig, offset, cdOffset, pTypeDescriptor
            for vtref in find_all(le32(colbase.getOffset())):
                vt = vtref.add(4)
                out.write("    COL %s vtable %s\n" % (colbase, vt))
                users = {}
                for r in rm.getReferencesTo(vt):
                    f = fm.getFunctionContaining(r.getFromAddress())
                    k = (f.getEntryPoint(), f.getName()) if f else (r.getFromAddress(), "?")
                    users[k] = users.get(k, 0) + 1
                for (ep, fn), c in sorted(users.items(), key=lambda kv: -kv[1])[:15]:
                    out.write("      %4d refs in %s %s\n" % (c, ep, fn))
out.close()

# Dumps int tables: BLUR2PC_TABLES = "addr:rows:cols;..." ; ints that point at ASCII strings are shown as strings.
import os
mem = currentProgram.getMemory()
out = open(os.environ["BLUR2PC_OUT"], "w")
def cstr(a):
    try:
        bs = []
        for i in range(64):
            b = mem.getByte(a.add(i)) & 0xff
            if b == 0: break
            if b < 32 or b > 126: return None
            bs.append(chr(b))
        return "".join(bs) if len(bs) >= 2 else None
    except Exception:
        return None
for spec in os.environ["BLUR2PC_TABLES"].split(";"):
    a, rows, cols = spec.split(":"); base = toAddr(a); rows = int(rows); cols = int(cols)
    out.write("== %s rows %d cols %d\n" % (base, rows, cols))
    for r in range(rows):
        vals = []
        for c in range(cols):
            v = mem.getInt(base.add((r * cols + c) * 4))
            s = None
            if 0x00400000 <= (v & 0xffffffff) < 0x02400000:
                s = cstr(toAddr(v & 0xffffffff))
            vals.append(('"%s"' % s) if s else ("%d" % v))
        out.write("  %2d: %s\n" % (r, ", ".join(vals)))
out.close()

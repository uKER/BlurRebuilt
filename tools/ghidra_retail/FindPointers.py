# Finds 32-bit little-endian copies of each address in BLUR2PC_ADDRS (comma-separated) anywhere in
# initialised memory and prints the hit, the dwords around it and any string the neighbours point to
# (e.g. the name beside a function in a Lua registration table).
import os
mem = currentProgram.getMemory()
out = open(os.environ["BLUR2PC_OUT"], "w")
def cstr(a):
    try:
        b = bytearray()
        p = toAddr(a)
        while len(b) < 80:
            c = mem.getByte(p) & 0xff
            if c == 0: break
            if c < 0x20 or c > 0x7e: return None
            b.append(c); p = p.add(1)
        return b.decode("ascii") if len(b) > 1 else None
    except: return None
for a in os.environ["BLUR2PC_ADDRS"].split(","):
    v = int(a, 16)
    pat = bytearray([v & 0xff, (v >> 8) & 0xff, (v >> 16) & 0xff, (v >> 24) & 0xff])
    out.write("== %s\n" % a)
    p = mem.getMinAddress()
    while p is not None:
        p = mem.findBytes(p, bytes(pat), None, True, monitor)
        if p is None: break
        row = []
        for k in range(-2, 3):
            q = p.add(4 * k)
            try: d = mem.getInt(q) & 0xffffffff
            except: continue
            s = cstr(d)
            row.append("%08x%s" % (d, (" '%s'" % s) if s else ""))
        out.write("   %s: %s\n" % (p, " | ".join(row)))
        p = p.add(1)
out.close()

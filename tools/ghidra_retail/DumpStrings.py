# Dumps the null-terminated ASCII string at each address in BLUR2PC_ADDRS (comma-separated hex).
import os
mem = currentProgram.getMemory()
def cstr(a):
    bs = []
    for i in range(128):
        b = mem.getByte(a.add(i)) & 0xff
        if b == 0: break
        bs.append(chr(b) if 32 <= b < 127 else '.')
    return "".join(bs)
out = open(os.environ["BLUR2PC_OUT"], "w")
for a in os.environ["BLUR2PC_ADDRS"].split(","):
    addr = toAddr(a.strip())
    out.write("%s: %r\n" % (a, cstr(addr)))
out.close()

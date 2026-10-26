# Scans every executable block byte by byte for E8 (CALL rel32) and E9 (JMP rel32) encodings whose target is
# one of BLUR2PC_ADDRS, including code Ghidra never disassembled (the DRM-relocated .8thSin section calls
# through "PUSH return; JMP target"). Prints each hit with the instructions just before it.
import os
mem = currentProgram.getMemory(); listing = currentProgram.getListing()
targets = set(int(a, 16) for a in os.environ["BLUR2PC_ADDRS"].split(","))
out = open(os.environ["BLUR2PC_OUT"], "w")
for blk in mem.getBlocks():
    if not blk.isExecute() or not blk.isInitialized():
        continue
    base = blk.getStart().getOffset(); n = blk.getSize()
    data = bytearray(b & 0xff for b in getBytes(blk.getStart(), int(n)))
    for i in range(n - 5):
        if data[i] not in (0xe8, 0xe9):
            continue
        rel = data[i+1] | (data[i+2] << 8) | (data[i+3] << 16) | (data[i+4] << 24)
        if rel >= 0x80000000: rel -= 0x100000000
        t = (base + i + 5 + rel) & 0xffffffff
        if t in targets:
            out.write("%08x %s -> %08x (block %s)\n" % (base + i, "CALL" if data[i] == 0xe8 else "JMP", t, blk.getName()))
out.close()

# Disassembles [BLUR2PC_LO, BLUR2PC_HI) (in memory; -readOnly keeps the project unchanged) and lists
# each instruction, with string operands resolved, to BLUR2PC_OUT. For code auto-analysis left
# undefined, where there is no function to decompile.
import os
mem = currentProgram.getMemory(); listing = currentProgram.getListing()
lo = toAddr(os.environ["BLUR2PC_LO"]); hi = toAddr(os.environ["BLUR2PC_HI"])
def cstr(v):
    try:
        a = toAddr("%x" % v); bs = []
        for i in range(64):
            b = mem.getByte(a.add(i)) & 0xff
            if b == 0: break
            if b < 32 or b > 126: return None
            bs.append(chr(b))
        return "".join(bs) if len(bs) >= 3 else None
    except Exception:
        return None
disassemble(lo)
out = open(os.environ["BLUR2PC_OUT"], "w")
a = lo
while a.compareTo(hi) < 0:
    ins = listing.getInstructionAt(a)
    if ins is None:
        disassemble(a); ins = listing.getInstructionAt(a)
        if ins is None:
            a = a.add(1); continue
    note = ""
    for i in range(ins.getNumOperands()):
        for o in ins.getOpObjects(i):
            try:
                s = cstr(o.getValue() & 0xffffffff)
                if s: note += '  "%s"' % s
            except Exception:
                pass
    out.write("%s  %-40s%s\n" % (ins.getAddress(), ins, note))
    a = ins.getMaxAddress().add(1)
out.close()

# Disassembles [BLUR2PC_LO, BLUR2PC_HI) linearly (in memory) and lists every indirect call through
# [reg + BLUR2PC_OFF] (a COM/vtable slot), with the BLUR2PC_BACK instructions before it. Used to
# find D3D calls (e.g. IDirect3D9::CreateDevice = +0x40) in code auto-analysis never reached.
import os
listing = currentProgram.getListing()
lo = toAddr(os.environ["BLUR2PC_LO"]); hi = toAddr(os.environ["BLUR2PC_HI"])
off = os.environ["BLUR2PC_OFF"].lower()
back = int(os.environ.get("BLUR2PC_BACK", "24"))
out = open(os.environ["BLUR2PC_OUT"], "w")
a = lo
while a.compareTo(hi) < 0:
    ins = listing.getInstructionAt(a)
    if ins is None:
        disassemble(a); ins = listing.getInstructionAt(a)
        if ins is None:
            a = a.add(1); continue
    t = str(ins).lower()
    anyuse = os.environ.get("BLUR2PC_ANY") == "1"	# also loads of the slot (MOV reg,[reg+off] then CALL reg)
    if (t.startswith("call dword ptr [e") or (anyuse and t.startswith("mov e"))) and ("+ %s]" % off) in t:
        out.write("== %s %s\n" % (ins.getAddress(), ins))
        p = ins; prev = []
        for i in range(back):
            p = p.getPrevious()
            if p is None: break
            prev.append(p)
        for p in reversed(prev): out.write("   %s  %s\n" % (p.getAddress(), p))
    a = ins.getMaxAddress().add(1)
out.close()

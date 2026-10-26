# For each function start in BLUR2PC_ADDRS (comma-separated hex; disassembled in memory if needed),
# lists every call with the `this` global loaded into ECX just before it, the immediates/float
# constants pushed for it, and the debug-variable name registered for that global.
# Names come from a whole-program scan: static initialisers do `MOV ECX,<global>` next to
# `PUSH "<Dotted.Name>"`; that pair gives the name of each Debug:: variable.
import os, struct
mem = currentProgram.getMemory(); listing = currentProgram.getListing()
from ghidra.program.model.scalar import Scalar
def cstr(v):
    try:
        a = toAddr("%x" % v); bs = []
        for i in range(96):
            b = mem.getByte(a.add(i)) & 0xff
            if b == 0: break
            if b < 32 or b > 126: return None
            bs.append(chr(b))
        return "".join(bs) if len(bs) >= 3 else None
    except Exception:
        return None
def fval(v):
    try:
        return struct.unpack("<f", struct.pack("<I", mem.getInt(toAddr("%x" % v)) & 0xffffffff))[0]
    except Exception:
        return None
names = {}
for ins in listing.getInstructions(True):
    m = ins.getMnemonicString()
    if m not in ("MOV", "PUSH"): continue
    s = None; g = None
    if m == "PUSH":
        for o in ins.getOpObjects(0):
            if isinstance(o, Scalar):
                t = cstr(o.getValue() & 0xffffffff)
                if t and "." in t and " " not in t: s = t
        if s is None: continue
        # look around for MOV ECX,imm
        p = ins
        for k in range(6):
            p = p.getPrevious()
            if p is None: break
            if p.getMnemonicString() == "MOV" and str(p.getRegister(0)) == "ECX":
                for o in p.getOpObjects(1):
                    if isinstance(o, Scalar): g = o.getValue() & 0xffffffff
                break
        if g is None:
            p = ins
            for k in range(6):
                p = p.getNext()
                if p is None: break
                if p.getMnemonicString() == "MOV" and str(p.getRegister(0)) == "ECX":
                    for o in p.getOpObjects(1):
                        if isinstance(o, Scalar): g = o.getValue() & 0xffffffff
                    break
        if g is not None and g not in names: names[g] = s
out = open(os.environ["BLUR2PC_OUT"], "w")
for a in os.environ["BLUR2PC_ADDRS"].split(","):
    start = toAddr(a.strip()); disassemble(start)
    out.write("== %s\n" % a)
    ins = listing.getInstructionAt(start); ecx = None; pushes = []
    n = 0
    while ins is not None and n < 400:
        n += 1
        m = ins.getMnemonicString()
        txt = str(ins)
        if m == "MOV" and str(ins.getRegister(0)) == "ECX":
            ecx = None
            for o in ins.getOpObjects(1):
                if isinstance(o, Scalar): ecx = o.getValue() & 0xffffffff
            if ecx is None: ecx = txt
        if m == "PUSH":
            for o in ins.getOpObjects(0):
                if isinstance(o, Scalar):
                    v = o.getValue() & 0xffffffff
                    f = struct.unpack("<f", struct.pack("<I", v))[0]
                    pushes.append("0x%x(%g)" % (v, f))
            if not ins.getOpObjects(0) or not isinstance(ins.getOpObjects(0)[0], Scalar):
                pushes.append(txt)
        if m in ("FLD", "MOVSS") :
            for o in ins.getOpObjects(ins.getNumOperands()-1):
                if isinstance(o, Scalar) or hasattr(o, "getOffset"):
                    try:
                        v = o.getValue() & 0xffffffff if isinstance(o, Scalar) else o.getOffset()
                        pushes.append("[%x]=%s" % (v, fval(v)))
                    except Exception: pass
            if m == "FLD" and txt.startswith("FLD1"): pushes.append("1.0")
        if m.startswith("FLD1") or txt == "FLD1": pushes.append("1.0")
        if m == "FLDZ" or txt == "FLDZ": pushes.append("0.0")
        if ins.getFlowType().isCall() or (m == "JMP" and pushes and "0x" in pushes[-1]):
            tg = [str(r.getToAddress()) for r in ins.getReferencesFrom()]
            nm = names.get(ecx) if isinstance(ecx, int) else None
            out.write("  %s %s ecx=%s %s args=%s\n" % (ins.getAddress(), txt, ("%x" % ecx) if isinstance(ecx, int) else ecx, ('"%s"' % nm) if nm else "", pushes))
            pushes = []
        if m == "MOV" and "ptr [" in txt and txt.split(",")[0].endswith("]") and "0x0" in txt.split(",")[0]:
            out.write("  %s %s\n" % (ins.getAddress(), txt))
        if m == "RET":
            out.write("  %s RET\n" % ins.getAddress())
            nxt = ins.getNext()
            if nxt is None or listing.getInstructionAt(nxt.getAddress()) is None: break
            # stop at padding
            if str(nxt).startswith("INT3"): break
        ins = ins.getNext()
out.close()

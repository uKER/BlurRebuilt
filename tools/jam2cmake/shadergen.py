#!/usr/bin/env python3
"""
shadergen.py — compile the engine's built-in .psh/.vsh shaders with fxc and
emit them as C arrays in the blob layout the PC engine expects.

The engine references each shader as `_binary_<symbol>_bin_start` (see
mercury/library/graphics/display/include/display/shader/ShaderUtils.h). On PC
the blob is read by C_D3D9Device::CreatePixelShader/CreateVertexShader as:

    U32 header[E_ShaderProfile_Count * 2] = { off_2_0, size_2_0, off_3_0, size_3_0 };
    U8  bytecode...                        // offsets are bytes from blob start

Usage:
  python shadergen.py --json _work/build/targets_pc_speed.json --out _work/build/gen/shaders [--fxc path]
Writes one <target>_shaders.cpp per target plus shaders_report.json.
"""
import argparse
import concurrent.futures
import glob
import json
import os
import struct
import subprocess
import sys
import tempfile

PROFILES = ['2_0', '3_0']   # order = Display::E_ShaderProfile enum


def find_fxc():
    cands = glob.glob(r'C:\Program Files (x86)\Windows Kits\10\bin\*\x86\fxc.exe')
    cands += glob.glob(r'C:\Program Files (x86)\Windows Kits\10\bin\*\x64\fxc.exe')
    cands.sort(reverse=True)
    return cands[0] if cands else None


def compile_one(fxc, src, stage, profile, defines, incdirs, entry='main'):
    with tempfile.NamedTemporaryFile(suffix='.bin', delete=False) as tf:
        outp = tf.name
    # /Zpr: row-major matrix packing. The engine uploads matrices row by row (MatrixRow44), and
    # Bizarre's PC shader scripts pass /Zpr (mercury/applications/Graphics/TestDisplay/src/mk_common_pc_mesh.bat).
    # Without it every built-in shader that transforms by a uploaded matrix gets it transposed.
    cmd = [fxc, '/nologo', '/T', f'{stage}_{profile}', '/E', entry, '/O3', '/Zpr', '/Fo', outp, src]
    # BLUR2PC marks the PC port's own shader builds (the jamfiles pass no platform define; shader
    # sources test PLATFORM_PS3 / XB2 and fall through to a shared PS3/PC path where PC needs its own).
    cmd += ['/D', 'BLUR2PC=1']
    for d in defines:
        cmd += ['/D', d]
    for i in incdirs:
        cmd += ['/I', i]
    try:
        r = subprocess.run(cmd, capture_output=True, text=True, timeout=120)
        if r.returncode == 0 and os.path.exists(outp):
            data = open(outp, 'rb').read()
            return data, ''
        return None, (r.stderr or r.stdout).strip()
    except Exception as e:  # noqa
        return None, str(e)
    finally:
        try:
            os.unlink(outp)
        except OSError:
            pass


def build_blob(datas):
    """datas: list of bytes-or-None per profile, in PROFILES order."""
    hdr_size = 4 * 2 * len(PROFILES)
    header = []
    body = b''
    off = hdr_size
    for d in datas:
        if d:
            pad = (-len(d)) % 4
            header += [off, len(d)]
            body += d + b'\0' * pad
            off += len(d) + pad
        else:
            header += [0, 0]
    return struct.pack('<%dI' % len(header), *header) + body


def words(blob):
    pad = (-len(blob)) % 4
    blob += b'\0' * pad
    return struct.unpack('<%dI' % (len(blob) // 4), blob)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--json', required=True)
    ap.add_argument('--out', required=True)
    ap.add_argument('--fxc', default=None)
    ap.add_argument('--jobs', type=int, default=os.cpu_count() or 4)
    ap.add_argument('--only', default=None, help='only this target')
    ap.add_argument('--overlay', action='append', default=[], help='[Blur2PC] overlay dir: a shader or include dir found there replaces the base one (repeatable, lowest first; DIR=VAR accepted as for gen_cmake.py)')
    a = ap.parse_args()

    fxc = a.fxc or find_fxc()
    if not fxc or not os.path.exists(fxc):
        print('fxc.exe not found', file=sys.stderr)
        return 2
    d = json.load(open(a.json, encoding='utf-8'))
    top = d['config']['top']
    overlays = [os.path.abspath(o.partition('=')[0]) for o in reversed(a.overlay)]   # highest first

    def ov(rel):
        for o in overlays:
            if os.path.isfile(os.path.join(o, rel)):
                return os.path.join(o, rel)
        return os.path.join(top, rel)

    os.makedirs(a.out, exist_ok=True)

    jobs = []
    for t in d['targets']:
        if not t['shaders'] or (a.only and t['name'] != a.only):
            continue
        incs = []
        for i in t['includes']:
            if not i['exists']:
                continue
            for o in overlays:
                if os.path.isdir(os.path.join(o, i['path'])):
                    incs.append(os.path.join(o, i['path']))
            incs.append(os.path.join(top, i['path']))
        for sh in t['shaders']:
            src = ov(sh['file'])
            stage = 'ps' if os.path.basename(src).lower().startswith('ps') else 'vs'
            sdir = os.path.dirname(src)
            bdir = os.path.dirname(os.path.join(top, sh['file']))
            incdirs = [sdir, os.path.dirname(sdir)]
            if bdir != sdir:
                incdirs += [bdir, os.path.dirname(bdir)]
            incdirs += incs
            jobs.append((t['name'], sh, src, stage, incdirs))

    results = {}
    report = {'ok': 0, 'failed': [], 'profile_2_0_missing': []}

    def work(job):
        tname, sh, src, stage, incdirs = job
        datas, errs = [], []
        for p in PROFILES:
            data, err = compile_one(fxc, src, stage, p, sh['defines'], incdirs)
            datas.append(data)
            errs.append(err)
        return tname, sh, datas, errs

    with concurrent.futures.ThreadPoolExecutor(max_workers=a.jobs) as ex:
        for tname, sh, datas, errs in ex.map(work, jobs):
            results.setdefault(tname, []).append((sh, datas, errs))

    for tname, items in results.items():
        lines = ['// Generated by tools/jam2cmake/shadergen.py - do not edit',
                 '// Built-in shader blobs for target ' + tname,
                 'extern "C" {']
        for sh, datas, errs in sorted(items, key=lambda x: x[0]['symbol']):
            sym = sh['symbol']
            if datas[-1] is None:
                report['failed'].append({'target': tname, 'symbol': sym, 'file': sh['file'],
                                         'defines': sh['defines'], 'error': errs[-1][:800]})
            elif datas[0] is None:
                report['profile_2_0_missing'].append({'target': tname, 'symbol': sym})
                report['ok'] += 1
            else:
                report['ok'] += 1
            blob = build_blob(datas)
            w = words(blob)
            lines.append(f'// {sh["file"]}  defines={" ".join(sh["defines"])}  sizes={[len(x) if x else 0 for x in datas]}')
            # 'extern' is required: a const object inside an extern "C" block still has internal linkage
            lines.append(f'extern const unsigned int _binary_{sym}_bin_start[{len(w)}] = {{')
            for i in range(0, len(w), 12):
                lines.append('    ' + ', '.join(f'0x{x:08x}' for x in w[i:i + 12]) + ',')
            lines.append('};')
            lines.append(f'extern const unsigned int _binary_{sym}_bin_end[1] = {{ 0 }};')
        lines.append('}')
        with open(os.path.join(a.out, f'{tname}_shaders.cpp'), 'w', encoding='ascii', newline='\n') as fh:
            fh.write('\n'.join(lines) + '\n')

    with open(os.path.join(a.out, 'shaders_report.json'), 'w', encoding='utf-8') as fh:
        json.dump(report, fh, indent=1)
    print(f'shaders: {report["ok"]} ok, {len(report["failed"])} failed (3_0), '
          f'{len(report["profile_2_0_missing"])} without a 2_0 build; targets: {len(results)}')
    for f in report['failed'][:15]:
        print('  FAIL', f['target'], f['symbol'], '--', f['error'].splitlines()[0] if f['error'] else '')
    return 0


if __name__ == '__main__':
    sys.exit(main())

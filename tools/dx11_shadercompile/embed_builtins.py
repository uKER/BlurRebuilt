"""Emit a native SM5 built-in shader registry keyed by original shader blob identity."""
from __future__ import annotations
import argparse
import json
from pathlib import Path


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument('--manifest', type=Path, required=True)
    parser.add_argument('--compiled', type=Path, required=True)
    parser.add_argument('--out', type=Path, required=True)
    args = parser.parse_args()
    targets = json.loads(args.manifest.read_text(encoding='utf-8'))['targets']
    entries = [(target['name'], shader['symbol']) for target in targets
               for shader in target['shaders']]
    lines = [
        '// [Blur1PCDX11] Generated native SM5 built-in shaders. Do not edit.',
        '#include <cstddef>', '#include <cstdint>',
        'extern "C" {',
    ]
    for _, symbol in entries:
        lines.append(f'extern const unsigned int _binary_{symbol}_bin_start[];')
    lines.append('}')
    lines.append('namespace Display { namespace D3D11 {')
    for target, symbol in entries:
        data = (args.compiled / target / f'{symbol}.cso').read_bytes()
        lines.append(f'static const uint8_t shader_{symbol}[] = {{')
        for offset in range(0, len(data), 24):
            lines.append('  ' + ','.join(f'0x{value:02x}' for value in data[offset:offset+24]) + ',')
        lines.append('};')
    lines.append('struct Builtin { const void* legacy; const uint8_t* native; size_t bytes; };')
    lines.append('static const Builtin builtins[] = {')
    for _, symbol in entries:
        lines.append(f'  {{_binary_{symbol}_bin_start,shader_{symbol},sizeof(shader_{symbol})}},')
    lines.append('};')
    lines.append('const uint8_t* FindBuiltinShader(const void* legacy, size_t* bytes)')
    lines.append('{')
    lines.append('  for (const auto& shader : builtins) if (shader.legacy == legacy)')
    lines.append('  { if (bytes) *bytes = shader.bytes; return shader.native; }')
    lines.append('  if (bytes) *bytes = 0; return nullptr;')
    lines.append('}')
    lines.append('} }')
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text('\n'.join(lines) + '\n', encoding='ascii')
    print(f'embedded {len(entries)} native built-in shaders')


if __name__ == '__main__':
    main()

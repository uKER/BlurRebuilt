#!/usr/bin/env python3
"""
jamextract.py — evaluate Bizarre's Jam build files for one target/build and
emit the build description (targets, sources, include paths, defines, PCH,
link lists) as JSON.

The shipped tree has the Jam interpreter sources but not Bizarre's Jambase
(the custom rules SubDir/IncludePath/Define/Library/Application/...). This
script implements the subset of the Jam language the jamfiles use, plus
those rules with the semantics the jamfiles rely on, and records what a
real build would have compiled and linked.

Usage:
  python jamextract.py --top C:\\AIProjects\\Blur2PC --root code/amax/game/jamfile.jam
                       --target WIN32 --platform PC --build Speed
                       --out _work/build/targets_pc_speed.json

Nothing here writes into the source tree.
"""
import argparse
import glob as globmod
import json
import os
import re
import sys

# ----------------------------------------------------------------------------
# Lexer
# ----------------------------------------------------------------------------

KEYWORDS = {
    ';', '{', '}', '[', ']', ':', '=', '+=', '?=', '!=', '<', '>', '<=', '>=',
    '(', ')', '!', '||', '&&', '|', '&', 'in', 'if', 'else', 'for', 'local', 'rule',
    'return', 'include', 'on', 'actions', 'while', 'switch', 'case', 'default',
    'break', 'continue',
}


class Tok:
    __slots__ = ('text', 'quoted', 'line')

    def __init__(self, text, quoted, line):
        self.text, self.quoted, self.line = text, quoted, line

    def is_kw(self, kw):
        return not self.quoted and self.text == kw

    def __repr__(self):
        return f'Tok({self.text!r},{self.line})'


def lex(src):
    toks = []
    i, n, line = 0, len(src), 1
    while i < n:
        c = src[i]
        if c == '\n':
            line += 1
            i += 1
            continue
        if c.isspace():
            i += 1
            continue
        if c == '#':
            while i < n and src[i] != '\n':
                i += 1
            continue
        # token: run until whitespace; quotes group; backslash escapes
        buf = []
        quoted = False
        start_line = line
        while i < n and not src[i].isspace():
            c = src[i]
            if c == '\\' and i + 1 < n:
                buf.append(src[i + 1])
                i += 2
                continue
            if c == '"':
                quoted = True
                i += 1
                while i < n and src[i] != '"':
                    if src[i] == '\\' and i + 1 < n:
                        buf.append(src[i + 1])
                        i += 2
                        continue
                    if src[i] == '\n':
                        line += 1
                    buf.append(src[i])
                    i += 1
                i += 1  # closing quote
                continue
            buf.append(c)
            i += 1
        toks.append(Tok(''.join(buf), quoted, start_line))
    return toks


# ----------------------------------------------------------------------------
# Parser -> AST (lists of tuples)
# ----------------------------------------------------------------------------

class ParseError(Exception):
    pass


class Parser:
    def __init__(self, toks, fname):
        self.t = toks
        self.i = 0
        self.fname = fname

    def peek(self, k=0):
        j = self.i + k
        return self.t[j] if j < len(self.t) else None

    def at(self, kw):
        p = self.peek()
        return p is not None and p.is_kw(kw)

    def expect(self, kw):
        if not self.at(kw):
            p = self.peek()
            raise ParseError(f'{self.fname}:{p.line if p else "EOF"}: expected {kw!r}, got {p.text if p else "EOF"!r}')
        self.i += 1

    def parse_block(self):
        stmts = []
        while self.peek() is not None and not self.at('}'):
            stmts.append(self.parse_stmt())
        return stmts

    def parse_stmt(self):
        p = self.peek()
        if p.is_kw('{'):
            self.i += 1
            b = self.parse_block()
            self.expect('}')
            return ('block', b)
        if p.is_kw('include'):
            self.i += 1
            lst = self.parse_list()
            self.expect(';')
            return ('include', lst)
        if p.is_kw('local'):
            self.i += 1
            names = self.parse_list()
            val = None
            if self.at('='):
                self.i += 1
                val = self.parse_list()
            self.expect(';')
            return ('local', names, val)
        if p.is_kw('return'):
            self.i += 1
            lst = self.parse_list()
            self.expect(';')
            return ('return', lst)
        if p.is_kw('break') or p.is_kw('continue'):
            self.i += 1
            self.expect(';')
            return (p.text,)
        if p.is_kw('for'):
            self.i += 1
            var = self.peek().text
            self.i += 1
            self.expect('in')
            lst = self.parse_list()
            self.expect('{')
            b = self.parse_block()
            self.expect('}')
            return ('for', var, lst, b)
        if p.is_kw('if'):
            self.i += 1
            cond = self.parse_cond()
            self.expect('{')
            b = self.parse_block()
            self.expect('}')
            els = None
            if self.at('else'):
                self.i += 1
                els = self.parse_stmt()
            return ('if', cond, b, els)
        if p.is_kw('while'):
            self.i += 1
            cond = self.parse_cond()
            self.expect('{')
            b = self.parse_block()
            self.expect('}')
            return ('while', cond, b)
        if p.is_kw('rule'):
            self.i += 1
            name = self.peek().text
            self.i += 1
            params = []
            if self.at('('):
                self.i += 1
                while not self.at(')'):
                    params.append(self.peek().text)
                    self.i += 1
                self.i += 1
            elif not self.at('{'):
                # rule name a : b { }
                while not self.at('{'):
                    if not self.at(':'):
                        params.append(self.peek().text)
                    self.i += 1
            self.expect('{')
            b = self.parse_block()
            self.expect('}')
            return ('rule', name, params, b)
        if p.is_kw('actions'):
            # actions [flags] name [bind vars] { raw text }
            self.i += 1
            while not self.at('{'):
                self.i += 1
            self.expect('{')
            depth = 1
            while depth:
                q = self.peek()
                if q is None:
                    break
                if q.is_kw('{'):
                    depth += 1
                elif q.is_kw('}'):
                    depth -= 1
                self.i += 1
            return ('actions',)
        if p.is_kw('on'):
            # on target statement
            self.i += 1
            tgt = self.parse_arg()
            st = self.parse_stmt()
            return ('on', tgt, st)
        if p.is_kw('switch'):
            self.i += 1
            lst = self.parse_list()
            self.expect('{')
            cases = []
            while self.at('case'):
                self.i += 1
                pat = self.peek().text
                self.i += 1
                self.expect(':')
                b = []
                while not self.at('case') and not self.at('}'):
                    b.append(self.parse_stmt())
                cases.append((pat, b))
            self.expect('}')
            return ('switch', lst, cases)
        # assignment or rule invocation. First arg then look at next token.
        first = self.parse_arg()
        q = self.peek()
        if q is not None and not q.quoted and q.text in ('=', '+=', '?=', 'default'):
            op = q.text
            self.i += 1
            if op == 'default':
                self.expect('=')
                op = '?='
            val = self.parse_list()
            self.expect(';')
            return ('assign', first, op, val)
        if q is not None and q.is_kw('on'):
            # var on targets = list ;
            self.i += 1
            tgts = self.parse_list()
            op = self.peek().text
            self.i += 1
            val = self.parse_list()
            self.expect(';')
            return ('assign_on', first, tgts, op, val)
        # rule invocation: name args ;
        args = self.parse_lol()
        self.expect(';')
        return ('call', first, args)

    def parse_lol(self):
        lol = [self.parse_list()]
        while self.at(':'):
            self.i += 1
            lol.append(self.parse_list())
        return lol

    def parse_list(self):
        items = []
        while True:
            p = self.peek()
            if p is None:
                break
            if p.is_kw('['):
                self.i += 1
                items.append(self.parse_bracket())
                continue
            if not p.quoted and p.text in (';', ':', ']', '{', '}', '=', '+=', '?=', '!=', '<', '>', '<=', '>=', 'in', '||', '&&', '|', '&', ')', 'on', 'default'):
                break
            items.append(('arg', p.text))
            self.i += 1
        return items

    def parse_arg(self):
        p = self.peek()
        if p.is_kw('['):
            self.i += 1
            return self.parse_bracket()
        self.i += 1
        return ('arg', p.text)

    def parse_bracket(self):
        # after '['
        if self.at('on'):
            self.i += 1
            tgt = self.parse_arg()
            if self.at('return'):
                self.i += 1
                lst = self.parse_list()
                self.expect(']')
                return ('on_return', tgt, lst)
            name = self.parse_arg()
            args = self.parse_lol()
            self.expect(']')
            return ('on_call', tgt, name, args)
        name = self.parse_arg()
        args = self.parse_lol()
        self.expect(']')
        return ('bcall', name, args)

    # conditions -------------------------------------------------------
    def parse_cond(self):
        return self.parse_or()

    def parse_or(self):
        left = self.parse_and()
        while self.at('||') or self.at('|'):
            self.i += 1
            right = self.parse_and()
            left = ('or', left, right)
        return left

    def parse_and(self):
        left = self.parse_not()
        while self.at('&&') or self.at('&'):
            self.i += 1
            right = self.parse_not()
            left = ('and', left, right)
        return left

    def parse_not(self):
        if self.at('!'):
            self.i += 1
            return ('not', self.parse_not())
        if self.at('('):
            self.i += 1
            c = self.parse_cond()
            self.expect(')')
            return c
        left = self.parse_list()
        p = self.peek()
        if p is not None and not p.quoted and p.text in ('=', '!=', '<', '>', '<=', '>=', 'in'):
            op = p.text
            self.i += 1
            right = self.parse_list()
            return ('cmp', op, left, right)
        return ('truth', left)


# ----------------------------------------------------------------------------
# Interpreter
# ----------------------------------------------------------------------------

class ReturnSignal(Exception):
    def __init__(self, value):
        self.value = value


VAR_RE = re.compile(r'\$\(')


class Interp:
    def __init__(self, top, target, platform, build, username='blur2pc'):
        self.top = os.path.abspath(top).replace('\\', '/').rstrip('/')
        self.vars = {
            'TOP': [self.top],
            'TARGET': [target],
            'PLATFORM': [platform],
            '_PLATFORM': [''],
            'BUILD': [build],
            'USERNAME': [username],
            'OS': ['NT'],
            'JAMOPTS': [],
            'DXSDK_PATH': ['${DXSDK_PATH}'],
            'XEDK': ['${XEDK}'],
            'XENONSDK': ['${XEDK}'],
            'SCE_PS3_ROOT': ['${SCE_PS3_ROOT}'],
            'INTDIR': ['obj'],
        }
        self.rules = {}
        self.local_stack = []
        # per-subdir state
        self.subdir = self.top
        self.subdir_tokens = []
        self.dir_includes = []
        self.dir_defines = []
        self.supported = True
        self.file_stack = []
        # outputs
        self.targets = []
        self.link_libraries = {}   # target name (lower) -> [libs]
        self.object_defines = []   # (file, [defs])
        self.object_ccflags = []
        self.pch_header = {}       # abs file -> pch header
        self.batch_seq = 0
        self.batch_of = {}         # abs file (lower) -> batch id
        self.batches = {}          # batch id -> {pch, files, dir}
        self.cpfiles = []
        self.importcp = []
        self.lua_bundles = []
        self.effects = []
        self.copies = []
        self.unknown_rules = {}
        self.warnings = []
        self.visited_jamfiles = []
        self._visited_keys = set()
        self.repeat_includes = {}
        self.missing_sources = []
        self.include_dirs_missing = set()

    # --- path helpers ------------------------------------------------------
    def resolve_ci(self, path):
        """Resolve a path case-insensitively against the file system. Returns
        (resolved_path, exists)."""
        path = path.replace('\\', '/')
        if os.path.exists(path):
            return os.path.normpath(path).replace('\\', '/'), True
        parts = path.split('/')
        # find longest existing prefix
        cur = parts[0] + '/' if parts[0].endswith(':') else (parts[0] or '/')
        if not os.path.isdir(cur):
            return os.path.normpath(path).replace('\\', '/'), False
        for p in parts[1:]:
            if p in ('', '.'):
                continue
            if p == '..':
                cur = os.path.dirname(cur.rstrip('/'))
                continue
            cand = os.path.join(cur, p)
            if os.path.exists(cand):
                cur = cand
                continue
            try:
                names = os.listdir(cur)
            except OSError:
                return os.path.normpath(path).replace('\\', '/'), False
            hit = [n for n in names if n.lower() == p.lower()]
            if not hit:
                return os.path.normpath(path).replace('\\', '/'), False
            cur = os.path.join(cur, hit[0])
        return os.path.normpath(cur).replace('\\', '/'), True

    def strip_grist(self, s):
        if s.startswith('<'):
            j = s.find('>')
            if j >= 0:
                return s[j + 1:]
        return s

    def resolve_src(self, tok):
        s = self.strip_grist(tok).strip('"')
        s = s.replace('\\', '/')
        if (len(s) > 1 and s[1] == ':') or s.startswith('/'):
            return self.resolve_ci(s)
        # Relative: SUBDIR first, then its parents up to TOP. Some jamfiles
        # (amax/game/GameAudio) list sources relative to the parent directory.
        d = self.subdir
        first = None
        while True:
            r, ok = self.resolve_ci(d + '/' + s)
            if first is None:
                first = r
            if ok:
                return r, True
            if os.path.normcase(d) == os.path.normcase(self.top) or len(d) <= len(self.top):
                break
            d = os.path.dirname(d)
        return first, False

    def resolve_inc(self, tok):
        s = tok.strip('"').replace('\\', '/')
        if '${' in s:
            return s, True
        if not (len(s) > 1 and s[1] == ':') and not s.startswith('/'):
            s = self.subdir + '/' + s
        r, ok = self.resolve_ci(s)
        if not ok:
            self.include_dirs_missing.add(r)
        return r, ok

    def rel(self, path):
        p = path.replace('\\', '/')
        if p.lower().startswith(self.top.lower() + '/'):
            return p[len(self.top) + 1:]
        return p

    # --- variables ----------------------------------------------------
    def lookup(self, name):
        for scope in reversed(self.local_stack):
            if name in scope:
                return scope[name]
        return self.vars.get(name, [])

    def setvar(self, name, val, op='='):
        scope = None
        for sc in reversed(self.local_stack):
            if name in sc:
                scope = sc
                break
        if scope is None:
            scope = self.vars
        if op == '=':
            scope[name] = list(val)
        elif op == '+=':
            scope[name] = list(scope.get(name, [])) + list(val)
        elif op == '?=':
            if not scope.get(name):
                scope[name] = list(val)

    def expand(self, s):
        """Expand $(VAR[:mods]) with product semantics. Returns a list."""
        if '$(' not in s:
            return [s]
        # find first $( ... ) with nesting
        i = s.find('$(')
        depth = 0
        j = i
        while j < len(s):
            if s.startswith('$(', j):
                depth += 1
                j += 2
                continue
            if s[j] == ')':
                depth -= 1
                if depth == 0:
                    break
            j += 1
        inner = s[i + 2:j]
        rest = s[j + 1:]
        prefix = s[:i]
        # modifiers
        mods = ''
        k = inner.find(':')
        if k >= 0 and not inner.startswith('$('):  # simple name with mods
            name_part, mods = inner[:k], inner[k + 1:]
        else:
            # nested: expand inner fully, then split mods on last ':' if outside parens
            name_part = inner
            # handle $($(X)LIBS:D) rarely; ignore mods for nested
        names = self.expand(name_part) if '$(' in name_part else [name_part]
        values = []
        for nm in names:
            values.extend(self.apply_mods(self.lookup(nm), mods))
        out = []
        rest_exp = self.expand(rest)
        for v in values:
            for r in rest_exp:
                out.append(prefix + v + r)
        return out

    def apply_mods(self, vals, mods):
        if not mods:
            return list(vals)
        out = []
        # parse mods like D, B, S, E=default, G, L, U, J=x
        mlist = []
        m = 0
        while m < len(mods):
            c = mods[m]
            m += 1
            if m < len(mods) and mods[m] == '=':
                # value until next modifier? Jam: rest of string for E=/J=/D= etc.
                val = mods[m + 1:]
                mlist.append((c, val))
                break
            mlist.append((c, None))
        if not vals:
            for c, val in mlist:
                if c == 'E':
                    return [val]
            return []
        for v in vals:
            grist = ''
            body = v
            if body.startswith('<'):
                gj = body.find('>')
                if gj >= 0:
                    grist, body = body[:gj + 1], body[gj + 1:]
            d, f = os.path.split(body)
            b, sfx = os.path.splitext(f)
            parts_keep = None
            res = None
            for c, val in mlist:
                if c == 'D':
                    res = (res or '') + (val if val is not None else d)
                elif c == 'B':
                    res = (res or '') + b
                elif c == 'S':
                    res = (res or '') + (val if val is not None else sfx)
                elif c == 'G':
                    res = (res or '') + grist
                elif c == 'L':
                    v = v.lower()
                elif c == 'U':
                    v = v.upper()
                elif c == 'E':
                    pass
                elif c == 'P':
                    res = (res or '') + d
                elif c == 'J':
                    pass
            out.append(res if res is not None else v)
        return out

    def eval_list(self, items):
        out = []
        for it in items:
            out.extend(self.eval_item(it))
        return out

    def eval_item(self, it):
        kind = it[0]
        if kind == 'arg':
            return self.expand(it[1])
        if kind == 'bcall':
            name = self.eval_item(it[1])
            name = name[0] if name else ''
            args = [self.eval_list(a) for a in it[2]]
            return self.call_rule(name, args)
        if kind == 'on_return':
            return self.eval_list(it[2])
        if kind == 'on_call':
            name = self.eval_item(it[2])
            name = name[0] if name else ''
            args = [self.eval_list(a) for a in it[3]]
            return self.call_rule(name, args)
        raise ParseError(f'bad item {it}')

    # --- conditions ---------------------------------------------------
    def eval_cond(self, c):
        k = c[0]
        if k == 'or':
            return self.eval_cond(c[1]) or self.eval_cond(c[2])
        if k == 'and':
            return self.eval_cond(c[1]) and self.eval_cond(c[2])
        if k == 'not':
            return not self.eval_cond(c[1])
        if k == 'truth':
            return any(x != '' for x in self.eval_list(c[1]))
        if k == 'cmp':
            op, l, r = c[1], self.eval_list(c[2]), self.eval_list(c[3])
            if op == 'in':
                return all(x in r for x in l)
            n = max(len(l), len(r))
            l2 = l + [''] * (n - len(l))
            r2 = r + [''] * (n - len(r))
            if op == '=':
                return l2 == r2
            if op == '!=':
                return l2 != r2
            ls, rs = ' '.join(l2), ' '.join(r2)
            return {'<': ls < rs, '>': ls > rs, '<=': ls <= rs, '>=': ls >= rs}[op]
        raise ParseError(f'bad cond {c}')

    # --- statements ---------------------------------------------------
    def exec_block(self, stmts):
        for st in stmts:
            self.exec_stmt(st)

    def exec_stmt(self, st):
        k = st[0]
        if k == 'block':
            self.exec_block(st[1])
        elif k == 'include':
            for f in self.eval_list(st[1]):
                self.include_file(f)
        elif k == 'local':
            names = self.eval_list(st[1])
            val = self.eval_list(st[2]) if st[2] is not None else []
            scope = self.local_stack[-1] if self.local_stack else self.vars
            for nm in names:
                scope[nm] = list(val)
        elif k == 'return':
            raise ReturnSignal(self.eval_list(st[1]))
        elif k in ('break', 'continue', 'actions'):
            pass
        elif k == 'for':
            for v in self.eval_list(st[2]):
                self.setvar(st[1], [v])
                self.exec_block(st[3])
        elif k == 'if':
            if self.eval_cond(st[1]):
                self.exec_block(st[2])
            elif st[3] is not None:
                self.exec_stmt(st[3])
        elif k == 'while':
            guard = 0
            while self.eval_cond(st[1]) and guard < 10000:
                self.exec_block(st[2])
                guard += 1
        elif k == 'rule':
            self.rules[st[1]] = (st[2], st[3])
        elif k == 'on':
            self.exec_stmt(st[2])
        elif k == 'switch':
            vals = self.eval_list(st[1])
            v = vals[0] if vals else ''
            for pat, b in st[2]:
                if globmod.fnmatch.fnmatch(v, pat) if hasattr(globmod, 'fnmatch') else v == pat:
                    self.exec_block(b)
                    break
        elif k == 'assign':
            names = self.eval_item(st[1])
            val = self.eval_list(st[3])
            for nm in names:
                self.setvar(nm, val, st[2])
        elif k == 'assign_on':
            pass  # target-specific variables (FXCFLAGS on ..., etc.) are not needed
        elif k == 'call':
            name = self.eval_item(st[1])
            name = name[0] if name else ''
            args = [self.eval_list(a) for a in st[2]]
            self.call_rule(name, args)
        else:
            raise ParseError(f'bad stmt {st}')

    def include_file(self, f):
        f = f.replace('\\', '/')
        cands = []
        if (len(f) > 1 and f[1] == ':') or f.startswith('/'):
            cands.append(f)
        else:
            cands.append(self.subdir + '/' + f)
            if self.file_stack:
                cands.append(os.path.dirname(self.file_stack[-1]) + '/' + f)
        for c in cands:
            r, ok = self.resolve_ci(c)
            if ok:
                self.run_file(r)
                return
        self.warnings.append(f'include not found: {f} (from {self.file_stack[-1] if self.file_stack else "?"})')

    def run_file(self, path):
        key = os.path.normcase(os.path.abspath(path))
        if key in self._visited_keys:
            # Jam would re-run it; for extraction one pass per jamfile is enough
            # and it breaks SubInclude cycles between the game and engine trees.
            self.repeat_includes[self.rel(path)] = self.repeat_includes.get(self.rel(path), 0) + 1
            return
        self._visited_keys.add(key)
        with open(path, 'r', encoding='latin-1') as fh:
            src = fh.read()
        toks = lex(src)
        ast = Parser(toks, path).parse_block()
        self.visited_jamfiles.append(self.rel(path))
        self.file_stack.append(path)
        try:
            self.exec_block(ast)
        except ReturnSignal:
            pass
        finally:
            self.file_stack.pop()

    # --- rules --------------------------------------------------------
    def call_rule(self, name, args):
        if name in self.rules:
            params, body = self.rules[name]
            scope = {}
            for idx, a in enumerate(args):
                scope[str(idx + 1)] = a
                if idx == 0:
                    scope['<'] = a
                if idx == 1:
                    scope['>'] = a
            for idx, p in enumerate(params):
                scope[p] = args[idx] if idx < len(args) else []
            self.local_stack.append(scope)
            try:
                self.exec_block(body)
                return []
            except ReturnSignal as r:
                return r.value
            finally:
                self.local_stack.pop()
        h = getattr(self, 'rule_' + name, None)
        if h is None:
            self.unknown_rules[name] = self.unknown_rules.get(name, 0) + 1
            return []
        return h(args)

    @staticmethod
    def a(args, i):
        return args[i] if i < len(args) else []

    def subdir_from_tokens(self, toks):
        if not toks:
            return self.top, []
        root = self.lookup(toks[0])
        base = root[0] if root else toks[0]
        path = base + ('/' + '/'.join(toks[1:]) if len(toks) > 1 else '')
        r, ok = self.resolve_ci(path)
        if not ok:
            self.warnings.append(f'SubDir not found: {path}')
        return r, toks

    def rule_SubDir(self, args):
        toks = self.a(args, 0)
        self.subdir, self.subdir_tokens = self.subdir_from_tokens(toks)
        self.dir_includes = []
        self.dir_defines = []
        self.supported = True
        self.vars['SUBDIR'] = [self.subdir]
        self.vars['SUBDIRHDRS'] = []
        self.vars['SUBDIRCCFLAGS'] = []
        self.vars['SEARCH_SOURCE'] = [self.subdir]
        # Per-directory source lists. Jamfiles rely on $($(PLATFORM)SRC) etc.
        # being empty unless they set it, so Bizarre's SubDir must have reset
        # them; without this, e.g. movie's PCSRC leaks into every later library.
        for k in [k for k in self.vars if k.endswith('SRC') and k != 'SRC']:
            self.vars[k] = []
        self.vars['SRC'] = []
        return []

    def rule_SubInclude(self, args):
        toks = self.a(args, 0)
        path, _ = self.subdir_from_tokens(toks)
        jam, ok = self.resolve_ci(path + '/jamfile.jam')
        if not ok:
            jam, ok = self.resolve_ci(path + '/Jamfile')
        if not ok:
            self.warnings.append(f'SubInclude: no jamfile in {path}')
            return []
        # save/restore per-dir state around the child
        saved = (self.subdir, self.subdir_tokens, self.dir_includes, self.dir_defines, self.supported,
                 dict((k, list(v)) for k, v in self.vars.items() if k in ('SUBDIR', 'SUBDIRHDRS', 'SUBDIRCCFLAGS', 'SEARCH_SOURCE')))
        self.run_file(jam)
        self.subdir, self.subdir_tokens, self.dir_includes, self.dir_defines, self.supported, dv = saved
        self.vars.update(dv)
        return []

    def rule_SupportedTargets(self, args):
        tg = self.lookup('TARGET')
        self.supported = bool(tg) and tg[0] in self.a(args, 0)
        return []

    def rule_IncludePath(self, args):
        for p in self.a(args, 0):
            self.dir_includes.append(p)
        return []

    def rule_Define(self, args):
        self.dir_defines.extend(self.a(args, 0))
        return []

    def _collect_sources(self, srcs, shaders):
        out = []
        seen = set()
        for s in srcs:
            r, ok = self.resolve_src(s)
            if not ok:
                self.missing_sources.append(r)
            if r.lower().endswith(('.psh', '.vsh')):
                variant, defs = '', ''
                if s.startswith('<'):
                    g = s[1:s.find('>')]
                    if '|' in g:
                        variant, defs = g.split('|', 1)
                base = os.path.splitext(os.path.basename(r))[0]
                sym = base + variant
                if sym.lower() in seen:
                    continue
                seen.add(sym.lower())
                shaders.append({'file': self.rel(r), 'symbol': sym, 'variant': variant,
                                'defines': [d[2:] for d in defs.split() if d.startswith('-D')]})
                continue
            out.append(r)
        return out

    def _current_includes(self):
        incs = []
        for p in list(self.dir_includes) + list(self.lookup('SUBDIRHDRS')):
            r, ok = self.resolve_inc(p)
            incs.append({'path': self.rel(r), 'exists': ok})
        return incs

    def _add_target(self, kind, args):
        name = self.a(args, 0)
        name = name[0] if name else '?'
        shaders = []
        srcs = self._collect_sources(self.a(args, 1), shaders)
        pch = self.a(args, 2)
        pch_src = None
        if pch:
            r, ok = self.resolve_src(pch[0])
            pch_src = self.rel(r)
            if not ok:
                self.missing_sources.append(r)
        # unity groups: consecutive sources that came from the same Batch call
        batches = []
        cur = None
        for s in srcs:
            bid = self.batch_of.get(s.lower())
            if bid is None:
                cur = None
                continue
            if cur is None or cur['id'] != bid:
                cur = {'id': bid, 'pch': self.batches[bid]['pch'], 'files': []}
                batches.append(cur)
            cur['files'].append(self.rel(s))
        t = {
            'kind': kind,
            'name': name,
            'dir': self.rel(self.subdir),
            'jamfile': self.rel(self.file_stack[-1]) if self.file_stack else None,
            'supported': self.supported,
            'sources': [self.rel(s) for s in srcs],
            'batches': batches,
            'shaders': shaders,
            'pch_source': pch_src,
            'pch_headers': sorted({self.pch_header[s] for s in srcs if s in self.pch_header}),
            'includes': self._current_includes(),
            'defines': list(self.dir_defines),
        }
        if kind in ('application', 'dll'):
            t['linklibs'] = list(self.lookup('LINKLIBS'))
            t['linkflags'] = list(self.lookup('LINKFLAGS'))
            t['pclibs'] = list(self.lookup('PCLIBS'))
        self.targets.append(t)
        return []

    def rule_Library(self, args):
        return self._add_target('library', args)

    def rule_Application(self, args):
        return self._add_target('application', args)

    def rule_DLL(self, args):
        return self._add_target('dll', args)

    def rule_SPULibrary(self, args):
        return []

    def rule_LinkLibraries(self, args):
        for t in self.a(args, 0):
            self.link_libraries.setdefault(t, []).extend(self.a(args, 1))
        return []

    def rule_ObjectDefines(self, args):
        # ObjectDefines <SPU_NET>car.cpp : SPU_NET_TASK applies to that gristed
        # (SPU module) object only, never to the plain car.cpp in the library.
        toks = self.a(args, 0)
        files = [self.rel(self.resolve_src(f)[0]) for f in toks]
        grists = [f[1:f.find('>')] if f.startswith('<') else '' for f in toks]
        self.object_defines.append({'files': files, 'grists': grists, 'defines': self.a(args, 1), 'dir': self.rel(self.subdir)})
        return []

    def rule_ObjectCcFlags(self, args):
        files = [self.rel(self.resolve_src(f)[0]) for f in self.a(args, 0)]
        self.object_ccflags.append({'files': files, 'flags': self.a(args, 1), 'dir': self.rel(self.subdir)})
        return []

    def rule_Batch(self, args):
        # [ Batch files : StdAfx.h ] compiled its files as ONE translation
        # unit (a unity build): SP_Rewards.cpp uses a template defined in
        # SP_SaveGame.cpp, which only works that way. Record the grouping so
        # the build generator can reproduce it.
        files = self.a(args, 0)
        hdr = self.a(args, 1)
        self.batch_seq += 1
        bid = self.batch_seq
        members = []
        for f in files:
            r, _ = self.resolve_src(f)
            if hdr:
                self.pch_header[r] = hdr[0]
            self.batch_of[r.lower()] = bid
            members.append(r)
        self.batches[bid] = {'pch': hdr[0] if hdr else None, 'files': members, 'dir': self.rel(self.subdir)}
        return files

    def rule_CPFile(self, args):
        for f in self.a(args, 0):
            self.cpfiles.append({'file': self.rel(self.resolve_src(f)[0]), 'dir': self.rel(self.subdir)})
        return []

    def rule_ImportCP(self, args):
        src = self.a(args, 0)
        toks = self.a(args, 1)
        path, _ = self.subdir_from_tokens(toks)
        self.importcp.append({'header': src[0] if src else None, 'from': self.rel(path), 'into': self.rel(self.subdir)})
        return []

    def rule_LUABundle(self, args):
        name = self.a(args, 0)
        self.lua_bundles.append({
            'target': name[0] if name else None,
            'scripts': [self.rel(self.resolve_src(s)[0]) for s in self.a(args, 1)],
            'root': self.rel(self.resolve_src(self.a(args, 2)[0])[0]) if self.a(args, 2) else None,
        })
        return []

    def rule_Effects(self, args):
        for f in self.a(args, 0):
            self.effects.append({'fx': self.rel(self.resolve_src(f)[0]), 'variants': self.a(args, 1), 'dir': self.rel(self.subdir)})
        return []

    def rule_Copy(self, args):
        self.copies.append({'dst': self.a(args, 0), 'src': self.a(args, 1), 'dir': self.rel(self.subdir)})
        return []

    # identity / no-op rules --------------------------------------------
    def _ident(self, args):
        return self.a(args, 0)

    rule_FGristFiles = _ident
    rule_AutoGrist = _ident
    rule_ConvertDOSPath = _ident
    rule_OBJfromSRC = _ident

    def rule_FSubDirPath(self, args):
        path, _ = self.subdir_from_tokens(self.a(args, 0))
        return [path]

    def rule_FDirName(self, args):
        return ['/'.join(self.a(args, 0))]

    def rule_GLOB(self, args):
        dirs = self.a(args, 0)
        pats = self.a(args, 1)
        out = []
        for d in dirs:
            for p in pats:
                out.extend(sorted(x.replace('\\', '/') for x in globmod.glob(os.path.join(d, p))))
        return out

    def rule_Shader(self, args):
        # [ Shader file : "<Variant>-DA -DB" ... ] -> one gristed entry per
        # variant, "<Variant|-DA -DB>file". The symbol the code references is
        # basename(file) + Variant (EXTERN_SHADER(psDefaultVertexColour)).
        files = self.a(args, 0)
        variants = self.a(args, 1) or ['<>']
        out = []
        for f in files:
            for v in variants:
                name, defs = '', v
                if v.startswith('<'):
                    j = v.find('>')
                    name, defs = v[1:j], v[j + 1:]
                out.append(f'<{name}|{defs.strip()}>{self.strip_grist(f)}')
        return out

    rule_Shaders = rule_Shader

    @staticmethod
    def _split_variant(tok):
        if tok.startswith('<'):
            j = tok.find('>')
            return tok[1:j], tok[j + 1:].strip()
        return '', tok.strip()

    def rule_Expand(self, args):
        # [ Expand "<A>-DA" "<>" : "<B> -DB" "<>" ] -> every combination, one
        # per argument list, with grists and defines concatenated in order:
        # "<AB>-DA -DB", "<A>-DA", "<B>-DB", "<>". Shader symbol names are the
        # base name plus the concatenated grist (psParticleSoftPointLights).
        lists = [a for a in args if a]
        out = [('', '')]
        for lst in lists:
            nxt = []
            for g0, d0 in out:
                for tok in lst:
                    g, d = self._split_variant(tok)
                    nxt.append((g0 + g, (d0 + ' ' + d).strip()))
            out = nxt
        return [f'<{g}>{d}' for g, d in out]

    def rule_Combinations(self, args):
        # every subset of the given variant tokens (used for .fx effect permutations)
        toks = self.a(args, 0)
        out = [('', '')]
        for tok in toks:
            g, d = self._split_variant(tok)
            out = out + [(g0 + g, (d0 + ' ' + d).strip()) for g0, d0 in out]
        return [f'<{g}>{d}' for g, d in out]

    def rule_MD(self, args):
        return []

    def rule_MD5(self, args):
        return ['0']

    def _noop(self, args):
        return []

    for _n in ('SrcOptim', 'Optim', 'Includes', 'Depends', 'DEPENDS', 'MkDir', 'EmbedManifest', 'ExportVcproj',
               'VcprojGroup', 'CheckXDKVersion', 'CheckPS3SDKVersion', 'EnableCallcap', 'AddSPUModule',
               'SetGameFolder', 'Echo', 'echo', 'ECHO', 'NOTFILE', 'NotFile', 'Always', 'ALWAYS', 'Clean',
               'InstallBin', 'InstallLib', 'ObjectHdrs', 'ObjectCFlags', 'Main', 'LinkLibs', 'Exit', 'EXIT',
               'PS3EncryptSelf', 'XexCfg', 'LUAScripts', 'Effect', 'BundleFiles', 'UseSPUModule'):
        locals()['rule_' + _n] = _noop
    del _n


# ----------------------------------------------------------------------------
# main
# ----------------------------------------------------------------------------

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--top', required=True, help='directory that contains code/ (Jam TOP)')
    ap.add_argument('--root', action='append', required=True, help='root jamfile(s), relative to --top')
    ap.add_argument('--target', default='WIN32')
    ap.add_argument('--platform', default='PC')
    ap.add_argument('--build', default='Speed')
    ap.add_argument('--username', default='blur2pc')
    ap.add_argument('--out', required=True)
    a = ap.parse_args()

    it = Interp(a.top, a.target, a.platform, a.build, a.username)
    for root in a.root:
        r, ok = it.resolve_ci(os.path.join(a.top, root))
        if not ok:
            print('root jamfile not found:', root, file=sys.stderr)
            return 2
        it.subdir = os.path.dirname(r)
        it.vars['SUBDIR'] = [it.subdir]
        it.run_file(r)

    # attach link libraries to targets. LINKLIBS/LINKFLAGS are read by Jam
    # at action time, i.e. after every jamfile ran, so record the final values.
    for t in it.targets:
        if t['kind'] in ('application', 'dll'):
            t['link_libraries'] = it.link_libraries.get(t['name'], [])
            t['linklibs_final'] = list(it.vars.get('LINKLIBS', []))
            t['linkflags_final'] = list(it.vars.get('LINKFLAGS', []))
            t['pclibs_final'] = list(it.vars.get('PCLIBS', []))
    supported = [t for t in it.targets if t['supported']]
    result = {
        'config': {'top': it.top, 'target': a.target, 'platform': a.platform, 'build': a.build},
        'targets': it.targets,
        'object_defines': it.object_defines,
        'object_ccflags': it.object_ccflags,
        'cpfiles': it.cpfiles,
        'importcp': it.importcp,
        'lua_bundles': it.lua_bundles,
        'effects_count': len(it.effects),
        'effects': it.effects,
        'copies': it.copies,
        'jamfiles_visited': it.visited_jamfiles,
        'jamfiles_repeat_includes': it.repeat_includes,
        'unknown_rules': it.unknown_rules,
        'missing_sources': sorted(set(it.rel(m) for m in it.missing_sources)),
        'missing_include_dirs': sorted(it.rel(m) for m in it.include_dirs_missing),
        'warnings': it.warnings,
    }
    os.makedirs(os.path.dirname(os.path.abspath(a.out)), exist_ok=True)
    with open(a.out, 'w', encoding='utf-8') as fh:
        json.dump(result, fh, indent=1)

    nsrc = sum(len(t['sources']) for t in supported)
    print(f'{a.target}/{a.platform}/{a.build}: {len(it.targets)} targets ({len(supported)} supported), '
          f'{nsrc} sources in supported targets, {len(it.visited_jamfiles)} jamfiles')
    print(f'missing sources: {len(result["missing_sources"])}, missing include dirs: {len(result["missing_include_dirs"])}, '
          f'unknown rules: {sorted(it.unknown_rules)}, warnings: {len(it.warnings)}')
    return 0


if __name__ == '__main__':
    sys.exit(main())

#!/usr/bin/env python3
"""Enum-style Val interpreter — push the eval-based experiment to its limit.

Idea: define ``Val`` (a Rust-enum-like tagged union with `kind` ∈ {INT,
FLOAT, BOOL, CHAR, STR, ARRAY}) and overload every Python operator on it
to match C semantics:

  +    string concat if either side is STR; int+int→int; mixed→float
  -    numeric
  *    numeric
  /    int/int → C-style truncate-toward-zero;  any float → float
  %    C-style sign-of-dividend
  <<   bit shift (int)         (cout's `<<` is on _Cout, not Val)
  >>   bit shift (int)
  <, <=, >, >=, ==, !=         return Val(BOOL, …)
  unary -, +                    numeric
  bool(self)                    truthy iff numeric≠0 or BOOL True

The translator wraps every literal as ``Val(...)``, every declaration as
``Val(...)``, then emits the source operators verbatim so Python's
operator dispatch lets Val do C semantics.  The only structural helpers
left are ``_ai`` (indexed assignment in expression context — Python
doesn't allow ``arr[i] = v`` as an expression) and the cout/cin classes
(they overload ``<<`` / ``>>`` rather than encode anything).
"""

import io
import sys

# ============== Val: the enum-like tagged value ==============


class Val:
    INT, FLOAT, BOOL, CHAR, STR, ARRAY = range(6)
    _NAMES = ("int", "float", "bool", "char", "str", "array")
    __slots__ = ("kind", "v")

    def __init__(self, kind, v):
        self.kind = kind
        self.v = v

    # ---- helpers ----

    @staticmethod
    def wrap(x):
        """Lift a bare Python value to a Val (Vals pass through)."""
        if isinstance(x, Val):
            return x
        if isinstance(x, bool):
            return Val(Val.BOOL, x)
        if isinstance(x, int):
            return Val(Val.INT, x)
        if isinstance(x, float):
            return Val(Val.FLOAT, x)
        if isinstance(x, str):
            return Val(Val.STR, x)
        if isinstance(x, list):
            return Val(Val.ARRAY, x)
        return x

    @staticmethod
    def show(x) -> str:
        """Format for cout / string concat — C semantics for each kind."""
        if isinstance(x, Val):
            if x.kind == Val.BOOL:
                return "true" if x.v else "false"
            if x.kind == Val.FLOAT:
                return f"{x.v:.3f}"
            return str(x.v)
        if isinstance(x, bool):
            return "true" if x else "false"
        if isinstance(x, float):
            return f"{x:.3f}"
        return str(x)

    def __repr__(self):
        return f"Val({Val._NAMES[self.kind]}, {self.v!r})"

    def __bool__(self):
        if self.kind in (Val.INT, Val.FLOAT):
            return self.v != 0
        if self.kind == Val.BOOL:
            return bool(self.v)
        return bool(self.v)

    # ---- numeric helpers ----

    def _numeric_pair(self, other):
        """Return (lv, rv, result_kind) for arithmetic between Val/int/float."""
        if isinstance(other, Val):
            ok = other.kind
            ov = other.v
        else:
            ov = other
            ok = (Val.FLOAT if isinstance(ov, float)
                  else Val.BOOL if isinstance(ov, bool)
                  else Val.CHAR if isinstance(ov, str)
                  else Val.INT)
        lv = self.v
        rv = ov
        if isinstance(lv, bool): lv = int(lv)
        if isinstance(rv, bool): rv = int(rv)
        if self.kind == Val.CHAR and isinstance(lv, str): lv = ord(lv)
        if ok == Val.CHAR and isinstance(rv, str): rv = ord(rv)
        if self.kind == Val.FLOAT or ok == Val.FLOAT:
            return float(lv), float(rv), Val.FLOAT
        return int(lv), int(rv), Val.INT

    # ---- arithmetic ----

    def __add__(self, other):
        # String concatenation if either side is STR.
        if self.kind == Val.STR or (isinstance(other, Val) and other.kind == Val.STR) or isinstance(other, str):
            return Val(Val.STR, Val.show(self) + Val.show(other))
        lv, rv, k = self._numeric_pair(other)
        return Val(k, lv + rv)

    def __radd__(self, other):
        # other + self  — used when other is a bare int/float/str
        if isinstance(other, str) or self.kind == Val.STR:
            return Val(Val.STR, Val.show(other) + Val.show(self))
        return Val.wrap(other).__add__(self)

    def __sub__(self, other):
        lv, rv, k = self._numeric_pair(other)
        return Val(k, lv - rv)

    def __rsub__(self, other):
        return Val.wrap(other).__sub__(self)

    def __mul__(self, other):
        lv, rv, k = self._numeric_pair(other)
        return Val(k, lv * rv)

    def __rmul__(self, other):
        return Val.wrap(other).__mul__(self)

    def __truediv__(self, other):
        lv, rv, k = self._numeric_pair(other)
        if k == Val.FLOAT:
            return Val(Val.FLOAT, lv / rv)
        # C truncate-toward-zero
        q = lv // rv
        if (lv < 0) != (rv < 0) and q * rv != lv:
            q += 1
        return Val(Val.INT, q)

    def __rtruediv__(self, other):
        return Val.wrap(other).__truediv__(self)

    def __mod__(self, other):
        lv, rv, k = self._numeric_pair(other)
        # C sign-of-dividend
        q = lv // rv
        if (lv < 0) != (rv < 0) and q * rv != lv:
            q += 1
        r = lv - q * rv
        if k == Val.FLOAT:
            return Val(Val.FLOAT, r)
        return Val(Val.INT, int(r))

    def __rmod__(self, other):
        return Val.wrap(other).__mod__(self)

    # ---- shift (bit) ----

    def __lshift__(self, other):
        lv, rv, _k = self._numeric_pair(other)
        return Val(Val.INT, int(lv) << int(rv))

    def __rlshift__(self, other):
        return Val.wrap(other).__lshift__(self)

    def __rshift__(self, other):
        lv, rv, _k = self._numeric_pair(other)
        return Val(Val.INT, int(lv) >> int(rv))

    def __rrshift__(self, other):
        return Val.wrap(other).__rshift__(self)

    # ---- comparisons ----

    def _compare(self, other, op):
        # If both sides are strings, compare as strings
        if (self.kind == Val.STR or isinstance(other, str)
                or (isinstance(other, Val) and other.kind == Val.STR)):
            ls = Val.show(self) if isinstance(self, Val) else str(self)
            rs = Val.show(other)
            a, b = ls, rs
        else:
            lv, rv, _k = self._numeric_pair(other)
            a, b = lv, rv
        if op == "<": r = a < b
        elif op == "<=": r = a <= b
        elif op == ">": r = a > b
        elif op == ">=": r = a >= b
        elif op == "==": r = a == b
        elif op == "!=": r = a != b
        else: raise ValueError(op)
        return Val(Val.BOOL, r)

    def __lt__(self, other): return self._compare(other, "<")
    def __le__(self, other): return self._compare(other, "<=")
    def __gt__(self, other): return self._compare(other, ">")
    def __ge__(self, other): return self._compare(other, ">=")
    def __eq__(self, other):
        if other is None: return False
        return self._compare(other, "==")
    def __ne__(self, other):
        if other is None: return True
        return self._compare(other, "!=")
    def __hash__(self): return hash((self.kind, self.v))

    # ---- unary ----

    def __neg__(self):
        if self.kind == Val.FLOAT: return Val(Val.FLOAT, -self.v)
        return Val(Val.INT, -int(self.v) if not isinstance(self.v, bool) else -int(self.v))

    def __pos__(self):
        return self

    # ---- string + Val (when string literal is on the right of a +) ----
    # __radd__ handles `"hi" + val`; for `val + "hi"` __add__ handles it.

    # ---- indexing (for ARRAY) ----

    def __getitem__(self, key):
        if self.kind != Val.ARRAY:
            raise TypeError("indexing non-array")
        i = key.v if isinstance(key, Val) else int(key)
        return self.v[i]

    def __setitem__(self, key, value):
        # Mutate the existing element Val in place (not replace) so any
        # by-reference alias to that element still sees the new content.
        if self.kind != Val.ARRAY:
            raise TypeError("indexing non-array")
        i = key.v if isinstance(key, Val) else int(key)
        self.v[i].set(value)

    def __len__(self):
        if self.kind == Val.ARRAY:
            return len(self.v)
        return len(self.v)

    # ---- Factory classmethods — used by translated declarations ----
    @classmethod
    def make_int(cls, v=0):    return cls(cls.INT, v)
    @classmethod
    def make_float(cls, v=0.0): return cls(cls.FLOAT, v)
    @classmethod
    def make_bool(cls, v=False): return cls(cls.BOOL, v)
    @classmethod
    def make_char(cls, v="\0"): return cls(cls.CHAR, v)
    @classmethod
    def make_str(cls, v=""):    return cls(cls.STR, v)
    @classmethod
    def make_arr(cls, elem_kind, n):
        defaults = {
            cls.INT:   lambda: cls(cls.INT, 0),
            cls.FLOAT: lambda: cls(cls.FLOAT, 0.0),
            cls.BOOL:  lambda: cls(cls.BOOL, False),
            cls.CHAR:  lambda: cls(cls.CHAR, "\0"),
            cls.STR:   lambda: cls(cls.STR, ""),
        }
        d = defaults.get(elem_kind, lambda: cls(cls.INT, 0))
        return cls(cls.ARRAY, [d() for _ in range(n)])

    # ---- Mutation methods (replace _setv) — every Our-C assignment
    # translates to a call on the existing Val, preserving object identity
    # so by-reference aliases see the change ----

    def set(self, value):
        if isinstance(value, Val):
            self.kind = value.kind
            self.v = value.v
        elif isinstance(value, bool):
            self.kind = Val.BOOL;  self.v = value
        elif isinstance(value, int):
            self.kind = Val.INT;   self.v = value
        elif isinstance(value, float):
            self.kind = Val.FLOAT; self.v = value
        elif isinstance(value, str):
            self.kind = Val.STR;   self.v = value
        else:
            self.v = value
        return self

    def copy(self):
        return Val(self.kind, self.v)

    def inc(self, delta=1):
        self.v = self.v + delta
        return self

    def dec(self, delta=1):
        self.v = self.v - delta
        return self

    def postinc(self, delta=1):
        old = Val(self.kind, self.v)
        self.v = self.v + delta
        return old

    def postdec(self, delta=1):
        old = Val(self.kind, self.v)
        self.v = self.v - delta
        return old

    # ---- Indexed mutation (arrays) ----

    def aset(self, i, value):
        idx = i.v if isinstance(i, Val) else int(i)
        self.v[idx].set(value)
        return self.v[idx]

    def ainc(self, i, delta=1):
        idx = i.v if isinstance(i, Val) else int(i)
        self.v[idx].inc(delta)
        return self.v[idx]

    def apostinc(self, i, delta=1):
        idx = i.v if isinstance(i, Val) else int(i)
        return self.v[idx].postinc(delta)


# ============== Cout / Cin / signals ==============


class _Cout:
    def __init__(self):
        self.buf: list[str] = []

    def __lshift__(self, x):
        self.buf.append(Val.show(x))
        return self

    def drain(self) -> str:
        s = "".join(self.buf)
        self.buf = []
        return s


class _Cin:
    def __rshift__(self, _x):
        return self


class _DoneSignal(Exception):
    pass


class _ReturnSignal(Exception):
    def __init__(self, value=None):
        self.value = value


class _Fn:
    """A callable Our-C function: pre-compiled body string + param info.

    On each call we build a fresh local Scope, copy by-value params,
    share by-ref / array params (so the caller's Val is the callee's),
    and ``exec`` the body. ``return X`` translates to
    ``raise _ReturnSignal(X)`` which we catch here.
    """

    __slots__ = ("name", "params", "byref", "body")

    def __init__(self, name, params, byref, body_src):
        self.name = name
        self.params = params
        self.byref = byref
        self.body = compile(body_src, f"<{name}>", "exec")

    def __call__(self, *args):
        _lns = Scope(_TOP_SCOPE_REF[0])
        for nm, arg, isref in zip(self.params, args, self.byref):
            if isref:
                dict.__setitem__(_lns, nm, arg)
            else:
                dict.__setitem__(_lns, nm, Val.wrap(arg).copy())
        try:
            exec(self.body, _TOP_SCOPE_REF[0], _lns)
        except _ReturnSignal as rs:
            return rs.value
        return None


# Module-level mutable reference to the current top-level Scope.
# Functions read this at call time so they always see the latest top.
_TOP_SCOPE_REF: list = [None]


class Scope(dict):
    """A dict subclass that intercepts ``name = value`` assignments inside
    ``exec(code, globals, scope)`` so they mutate the existing ``Val``
    instead of rebinding the name.

    Python's ``a = 10`` normally creates / rebinds the name. By using a
    custom dict as the *locals* argument to ``exec``, every STORE_NAME at
    that scope routes through ``__setitem__`` — letting us preserve the
    ``Val`` object's identity (which is what by-reference aliasing and
    cross-call mutation rely on).

    Lookup chain: ``Scope`` carries an optional ``_outer`` Scope so that
    writes to a name that's not local but exists as a ``Val`` in the
    outer scope (i.e. a global being assigned from within a function)
    mutate the outer ``Val`` in place too.
    """

    def __init__(self, outer=None):
        super().__init__()
        self._outer = outer

    def __setitem__(self, k, v):
        if k in self:
            existing = dict.__getitem__(self, k)
            if isinstance(existing, Val):
                existing.set(v)
                return
            dict.__setitem__(self, k, v)
            return
        # Not local: if the outer scope holds a Val for this name (i.e.
        # a global being written from inside a function), mutate it.
        outer = self._outer
        while outer is not None:
            if k in outer:
                outer_val = dict.__getitem__(outer, k)
                if isinstance(outer_val, Val):
                    outer_val.set(v)
                    return
                break
            outer = getattr(outer, "_outer", None)
        # Truly new name — record locally.
        dict.__setitem__(self, k, v)


def _done(*_args):
    raise _DoneSignal()


def _ai(arr, i, value):
    """Indexed assignment as expression: mutate the Val at arr[i] in place
    (rather than replacing it) so any outstanding by-ref alias to that
    element still sees the new content. Returns the (mutated) Val.
    """
    idx = i.v if isinstance(i, Val) else int(i)
    target = arr.v[idx] if isinstance(arr, Val) else arr[idx]
    return _setv(target, value)


def _setv(target, value):
    """Mutate ``target.v`` in place (and ``target.kind`` if the new value
    is a Val). Returns the new content as a Val so the helper can be used
    as an expression. Used for assignments to by-reference scalar params
    so that the caller's Val (passed by reference) sees the change
    immediately — true C++ aliasing semantics."""
    if isinstance(value, Val):
        target.kind = value.kind
        target.v = value.v
    else:
        # Infer kind from the bare Python value
        if isinstance(value, bool): target.kind = Val.BOOL; target.v = value
        elif isinstance(value, int): target.kind = Val.INT; target.v = value
        elif isinstance(value, float): target.kind = Val.FLOAT; target.v = value
        elif isinstance(value, str): target.kind = Val.STR; target.v = value
        else: target.v = value
    return target


def _vint(n):  return Val(Val.INT, n)
def _vflt(f):  return Val(Val.FLOAT, f)
def _vbool(b): return Val(Val.BOOL, b)
def _vchar(c): return Val(Val.CHAR, c)
def _vstr(s):  return Val(Val.STR, s)
def _varr_of(kind, n):
    if kind == Val.INT:   default = lambda: Val(Val.INT, 0)
    elif kind == Val.FLOAT: default = lambda: Val(Val.FLOAT, 0.0)
    elif kind == Val.BOOL:  default = lambda: Val(Val.BOOL, False)
    elif kind == Val.CHAR:  default = lambda: Val(Val.CHAR, "\0")
    elif kind == Val.STR:   default = lambda: Val(Val.STR, "")
    else:                   default = lambda: Val(Val.INT, 0)
    return Val(Val.ARRAY, [default() for _ in range(n)])


def _default(ty: str):
    return {
        "int":   _vint(0),
        "float": _vflt(0.0),
        "bool":  _vbool(False),
        "char":  _vchar("\0"),
        "string": _vstr(""),
        "void": None,
    }[ty]


def _default_kind(ty: str):
    return {
        "int":   Val.INT,
        "float": Val.FLOAT,
        "bool":  Val.BOOL,
        "char":  Val.CHAR,
        "string": Val.STR,
        "void":  Val.INT,
    }[ty]


# ============== Tokenizer (unchanged) ==============

_KW = {
    "int", "float", "bool", "string", "char", "void",
    "if", "else", "while", "return",
    "true", "false", "cin", "cout", "Done",
}

_BYREF_TABLE: dict[str, list[bool]] = {}

# Set when translating a function body — names that are by-ref scalar
# parameters in the current function. Their assignments must mutate via
# _setv so the caller's Val sees the change immediately.
_CURRENT_BYREF_LOCALS: set[str] = set()

# Symbol table used during parsing (so we can flag undefined-identifier
# errors with chunk-relative line numbers, matching the C++ reference).
_GLOBAL_SCOPE: set[str] = set()  # declared global names
_LOCAL_SCOPES: list[set] = []     # stack of local scopes (per function/block)
_CHUNK_START_LINE: int = 1
_FN_DEFS: set[str] = set()        # function names defined so far
_LAST_PARSED_POS: int = 0         # last absolute token pos visited by a Parser
_CURRENT_BASE: int = 0            # offset for Parser positions → global token index


def _ident_defined(name: str) -> bool:
    if name in ("cout", "cin"): return True
    for scope in reversed(_LOCAL_SCOPES):
        if name in scope: return True
    return name in _GLOBAL_SCOPE or name in _FN_DEFS


def _relative_line(abs_line: int) -> int:
    # Chunk-relative line number: line counter starts at 1 on the line
    # BEFORE the first token of the chunk, matching the C++ reference.
    if abs_line + 2 >= _CHUNK_START_LINE:
        return abs_line + 2 - _CHUNK_START_LINE
    return 1


class _ParseErr(Exception):
    """Parse error with chunk-relative line number AND the absolute token
    position where the error occurred (so the driver can recover by
    skipping tokens forward from that point)."""

    def __init__(self, msg, line, abs_pos=None):
        super().__init__(msg)
        self.msg = msg
        self.line = line
        self.abs_pos = abs_pos


def _make_parse_err(msg: str, tok, abs_pos=None):
    return _ParseErr(msg, _relative_line(tok[2]) if tok else 1, abs_pos)


def tokenize(src: str):
    tokens = []
    i = 0
    line = 1
    while i < len(src):
        c = src[i]
        if c == "\n":
            line += 1; i += 1; continue
        if c.isspace():
            i += 1; continue
        if c == "/" and i + 1 < len(src) and src[i + 1] == "/":
            while i < len(src) and src[i] != "\n":
                i += 1
            continue
        if c == "/" and i + 1 < len(src) and src[i + 1] == "*":
            i += 2
            while i + 1 < len(src) and not (src[i] == "*" and src[i + 1] == "/"):
                if src[i] == "\n": line += 1
                i += 1
            i += 2
            continue
        if c == '"':
            j = i + 1; s = ""
            while j < len(src) and src[j] != '"':
                if src[j] == "\\" and j + 1 < len(src):
                    e = src[j + 1]
                    s += {"n": "\n", "t": "\t", "r": "\r", "\\": "\\", '"': '"', "'": "'", "0": "\0"}.get(e, e)
                    j += 2
                else:
                    if src[j] == "\n": line += 1
                    s += src[j]; j += 1
            tokens.append(("STR", s, line))
            i = j + 1; continue
        if c == "'":
            j = i + 1
            if src[j] == "\\":
                e = src[j + 1]
                v = {"n": "\n", "t": "\t", "r": "\r", "\\": "\\", '"': '"', "'": "'", "0": "\0"}.get(e, e)
                j += 2
            else:
                v = src[j]; j += 1
            tokens.append(("CHAR", v, line))
            i = j + 1; continue
        if i + 1 < len(src):
            two = src[i:i + 2]
            if two in {"==", "!=", "<=", ">=", "&&", "||", "<<", ">>", "++", "--", "+=", "-=", "*=", "/=", "%="}:
                tokens.append(("OP", two, line))
                i += 2; continue
        if c in "+-*/%=<>!&,;(){}[]":
            tokens.append(("OP", c, line)); i += 1; continue
        if c.isdigit():
            j = i; is_float = False
            while j < len(src) and (src[j].isdigit() or src[j] == "."):
                if src[j] == ".":
                    if is_float: break
                    is_float = True
                j += 1
            if is_float:
                tokens.append(("FLOAT", float(src[i:j]), line))
            else:
                tokens.append(("INT", int(src[i:j]), line))
            i = j; continue
        if c.isalpha() or c == "_":
            j = i
            while j < len(src) and (src[j].isalnum() or src[j] == "_"):
                j += 1
            w = src[i:j]
            tokens.append(("KW" if w in _KW else "ID", w, line)); i = j; continue
        raise SyntaxError(f"unexpected char {c!r} at line {line}")
    return tokens


# ============== Chunk splitter (unchanged in structure) ==============

def _py_escape_str(s: str) -> str:
    return '"' + s.replace("\\", "\\\\").replace('"', '\\"').replace("\n", "\\n").replace("\t", "\\t") + '"'


def _py_escape_char(c: str) -> str:
    return _py_escape_str(c)


def _is_type_kw(tok) -> bool:
    return tok[0] == "KW" and tok[1] in ("int", "float", "bool", "string", "char", "void")


def _is_op(tok, val) -> bool:
    return tok[0] == "OP" and tok[1] == val


def _is_kw(tok, val) -> bool:
    return tok[0] == "KW" and tok[1] == val


class Chunk:
    def __init__(self, kind: str, tokens: list, start_line: int):
        self.kind = kind
        self.tokens = tokens
        self.start_line = start_line


def _find_stmt_end(tokens, i: int) -> int:
    if i >= len(tokens):
        return i
    t = tokens[i]
    if _is_op(t, "{"):
        depth = 1; j = i + 1
        while j < len(tokens) and depth > 0:
            if _is_op(tokens[j], "{"): depth += 1
            elif _is_op(tokens[j], "}"): depth -= 1
            j += 1
        return j
    if _is_kw(t, "if") or _is_kw(t, "while"):
        j = i + 1
        if j < len(tokens) and _is_op(tokens[j], "("):
            depth = 1; j += 1
            while j < len(tokens) and depth > 0:
                if _is_op(tokens[j], "("): depth += 1
                elif _is_op(tokens[j], ")"): depth -= 1
                j += 1
        j = _find_stmt_end(tokens, j)
        if _is_kw(t, "if") and j < len(tokens) and _is_kw(tokens[j], "else"):
            j += 1
            j = _find_stmt_end(tokens, j)
        return j
    j = i; depth = 0
    while j < len(tokens):
        tk = tokens[j]
        if tk[0] == "OP":
            if tk[1] in "([{": depth += 1
            elif tk[1] in ")]}": depth -= 1
            elif tk[1] == ";" and depth == 0:
                return j + 1
        j += 1
    return j


def split_chunks(tokens):
    i = 0
    while i < len(tokens):
        start = i
        t = tokens[i]
        if t[0] == "KW" and t[1] == "Done":
            i += 1
            if i < len(tokens) and _is_op(tokens[i], "("):
                i += 1
                if i < len(tokens) and _is_op(tokens[i], ")"):
                    i += 1
            if i < len(tokens) and _is_op(tokens[i], ";"):
                i += 1
            yield Chunk("done", tokens[start:i], t[2])
            continue
        if _is_type_kw(t):
            if (i + 2 < len(tokens) and tokens[i + 1][0] == "ID"
                    and _is_op(tokens[i + 2], "(")):
                j = i + 3; depth_p = 1
                while j < len(tokens) and depth_p > 0:
                    if _is_op(tokens[j], "("): depth_p += 1
                    elif _is_op(tokens[j], ")"): depth_p -= 1
                    j += 1
                if j < len(tokens) and _is_op(tokens[j], "{"):
                    depth_b = 1; j += 1
                    while j < len(tokens) and depth_b > 0:
                        if _is_op(tokens[j], "{"): depth_b += 1
                        elif _is_op(tokens[j], "}"): depth_b -= 1
                        j += 1
                    yield Chunk("fndef", tokens[start:j], t[2])
                    i = j; continue
            j = _find_stmt_end(tokens, i)
            yield Chunk("decl", tokens[start:j], t[2])
            i = j; continue
        j = _find_stmt_end(tokens, i)
        yield Chunk("stmt", tokens[start:j], t[2])
        i = j


def _recover_to_next_chunk(tokens, pos: int) -> int:
    """Skip tokens after a parse error until we find a top-level chunk
    boundary: ; at depth 0 OR } that closes a top-level brace. Returns
    the new position. Used to mimic the C++ reference's abandon-and-
    resume parsing recovery."""
    pos = min(pos + 1, len(tokens))  # always advance at least one
    depth_paren = 0
    depth_brace = 0
    while pos < len(tokens):
        t = tokens[pos]
        if t[0] == "OP":
            v = t[1]
            if v in "([": depth_paren += 1; pos += 1; continue
            if v in ")]": depth_paren = max(0, depth_paren - 1); pos += 1; continue
            if v == "{": depth_brace += 1; pos += 1; continue
            if v == "}":
                depth_brace -= 1
                pos += 1
                if depth_brace <= 0 and depth_paren == 0:
                    return pos
                continue
            if v == ";":
                pos += 1
                if depth_paren == 0 and depth_brace == 0:
                    return pos
                continue
        pos += 1
    return pos


def find_one_chunk(tokens, i: int):
    """Yield the next chunk starting at tokens[i]. Returns (chunk, new_i)
    or (None, len(tokens)) if at end."""
    if i >= len(tokens):
        return None, i
    start = i
    t = tokens[i]
    if t[0] == "KW" and t[1] == "Done":
        i += 1
        if i < len(tokens) and _is_op(tokens[i], "("):
            i += 1
            if i < len(tokens) and _is_op(tokens[i], ")"):
                i += 1
        if i < len(tokens) and _is_op(tokens[i], ";"):
            i += 1
        return Chunk("done", tokens[start:i], t[2]), i
    if _is_type_kw(t):
        if (i + 2 < len(tokens) and tokens[i + 1][0] == "ID"
                and _is_op(tokens[i + 2], "(")):
            j = i + 3; depth_p = 1
            while j < len(tokens) and depth_p > 0:
                if _is_op(tokens[j], "("): depth_p += 1
                elif _is_op(tokens[j], ")"): depth_p -= 1
                j += 1
            if j < len(tokens) and _is_op(tokens[j], "{"):
                depth_b = 1; j += 1
                while j < len(tokens) and depth_b > 0:
                    if _is_op(tokens[j], "{"): depth_b += 1
                    elif _is_op(tokens[j], "}"): depth_b -= 1
                    j += 1
                return Chunk("fndef", tokens[start:j], t[2]), j
        j = _find_stmt_end(tokens, i)
        return Chunk("decl", tokens[start:j], t[2]), j
    j = _find_stmt_end(tokens, i)
    return Chunk("stmt", tokens[start:j], t[2]), j


# ============== Parser (operators stay native; literals → Val(...)) ==============


class _ArgInfo:
    __slots__ = ("py", "lvalue_py")

    def __init__(self, py: str, lvalue_py: "str | None"):
        self.py = py
        self.lvalue_py = lvalue_py


class Parser:
    _box_counter = [0]

    def __init__(self, tokens, start: int, end: int, base: int = None):
        self.tokens = tokens
        self.pos = start
        self.end = end
        # `base` is the offset added to `self.pos` to recover the absolute
        # position in the global token stream. Defaults to the module
        # _CURRENT_BASE so callers don't have to thread it explicitly.
        self.base = _CURRENT_BASE if base is None else base
        self.preamble: list[str] = []
        self.deferred: list[str] = []

    def abs_pos(self) -> int:
        return self.base + self.pos

    def at_end(self) -> bool:
        return self.pos >= self.end

    def peek(self):
        return self.tokens[self.pos] if self.pos < self.end else None

    def peek_kind_val(self):
        t = self.peek()
        return (t[0], t[1]) if t else (None, None)

    def advance(self):
        global _LAST_PARSED_POS
        t = self.tokens[self.pos]
        _LAST_PARSED_POS = self.base + self.pos
        self.pos += 1
        return t

    def match_op(self, val: str) -> bool:
        if self.pos < self.end and self.tokens[self.pos][0] == "OP" and self.tokens[self.pos][1] == val:
            self.pos += 1; return True
        return False

    def expect_op(self, val: str):
        if not self.match_op(val):
            t = self.peek()
            got = t[1] if t and t[0] == "OP" else (t[1] if t else "<EOF>")
            ln = t[2] if t else _CHUNK_START_LINE
            global _LAST_PARSED_POS
            _LAST_PARSED_POS = self.base + self.pos
            raise _ParseErr(f"unexpected token '{got}'", _relative_line(ln), self.base + self.pos)

    def parse_expression(self) -> str:
        return self.parse_assignment()

    def parse_assignment(self) -> str:
        lhs = self.parse_or()
        if (self.pos < self.end and self.tokens[self.pos][0] == "OP"
                and self.tokens[self.pos][1] in ("=", "+=", "-=", "*=", "/=", "%=")):
            op = self.tokens[self.pos][1]
            self.pos += 1
            rhs = self.parse_assignment()
            return self._emit_assign(lhs, op, rhs)
        return lhs

    def _emit_assign(self, lhs: str, op: str, rhs: str) -> str:
        # Expression-context assignment (this is what the recursive parser
        # produces). Python doesn't allow plain `=` inside an expression,
        # so:
        #   simple name lvalue → walrus `(name := expr)`.  Inside an
        #     exec'd code, walrus stores via Scope.__setitem__ which
        #     mutates the existing Val.
        #   array index lvalue  → method `arr.aset(i, expr)`.  Python
        #     can't walrus a subscript target.
        # The translator post-processes top-level expression statements
        # whose AST is a single named assignment (or chained named
        # assignment) into plain `a = expr` for readability — see
        # `_flatten_assignment_statement` in translate_one_stmt.
        is_index = lhs.endswith("]") and "[" in lhs and not lhs.startswith("(")
        if is_index:
            depth = 0; split = -1
            for k in range(len(lhs) - 1, -1, -1):
                c = lhs[k]
                if c == "]": depth += 1
                elif c == "[":
                    depth -= 1
                    if depth == 0: split = k; break
            if split != -1:
                arr = lhs[:split]; idx = lhs[split + 1:-1]
                if op == "=":
                    return f"{arr}.aset({idx}, {rhs})"
                native = op[0]
                return f"{arr}.aset({idx}, {arr}[{idx}] {native} {rhs})"
        if op == "=":
            return f"({lhs} := {rhs})"
        native = op[0]
        return f"({lhs} := ({lhs} {native} {rhs}))"

    def parse_or(self) -> str:
        s = self.parse_and()
        while self.match_op("||"):
            r = self.parse_and()
            s = f"({s} or {r})"
        return s

    def parse_and(self) -> str:
        s = self.parse_eq()
        while self.match_op("&&"):
            r = self.parse_eq()
            s = f"({s} and {r})"
        return s

    def parse_eq(self) -> str:
        s = self.parse_rel()
        while True:
            if self.match_op("=="):
                r = self.parse_rel(); s = f"({s} == {r})"
            elif self.match_op("!="):
                r = self.parse_rel(); s = f"({s} != {r})"
            else: break
        return s

    def parse_rel(self) -> str:
        s = self.parse_shift()
        while True:
            k, v = self.peek_kind_val()
            if k == "OP" and v in ("<", "<=", ">", ">="):
                self.pos += 1
                r = self.parse_shift()
                s = f"({s} {v} {r})"
            else: break
        return s

    def parse_shift(self) -> str:
        s = self.parse_add()
        while True:
            k, v = self.peek_kind_val()
            if k == "OP" and v == "<<":
                self.pos += 1; r = self.parse_add(); s = f"({s} << {r})"
            elif k == "OP" and v == ">>":
                self.pos += 1; r = self.parse_add(); s = f"({s} >> {r})"
            else: break
        return s

    def parse_add(self) -> str:
        s = self.parse_mul()
        while True:
            k, v = self.peek_kind_val()
            if k == "OP" and v == "+":
                self.pos += 1; r = self.parse_mul(); s = f"({s} + {r})"
            elif k == "OP" and v == "-":
                self.pos += 1; r = self.parse_mul(); s = f"({s} - {r})"
            else: break
        return s

    def parse_mul(self) -> str:
        s = self.parse_unary()
        while True:
            k, v = self.peek_kind_val()
            if k == "OP" and v == "*":
                self.pos += 1; r = self.parse_unary(); s = f"({s} * {r})"
            elif k == "OP" and v == "/":
                self.pos += 1; r = self.parse_unary(); s = f"({s} / {r})"
            elif k == "OP" and v == "%":
                self.pos += 1; r = self.parse_unary(); s = f"({s} % {r})"
            else: break
        return s

    def parse_unary(self) -> str:
        k, v = self.peek_kind_val()
        if k == "OP" and v == "-":
            self.pos += 1; r = self.parse_unary(); return f"(-{r})"
        if k == "OP" and v == "+":
            self.pos += 1; r = self.parse_unary(); return f"(+{r})"
        if k == "OP" and v == "!":
            self.pos += 1; r = self.parse_unary()
            return f"(not {r})"
        if k == "OP" and v == "++":
            self.pos += 1; r = self.parse_unary()
            return self._emit_preinc(r, +1)
        if k == "OP" and v == "--":
            self.pos += 1; r = self.parse_unary()
            return self._emit_preinc(r, -1)
        return self.parse_postfix()

    def _emit_preinc(self, lhs: str, delta: int) -> str:
        # `++x` → `x.inc()` (returns x after mutation, used as the value).
        # `++arr[i]` → `arr.ainc(i)`.
        is_index = lhs.endswith("]") and "[" in lhs and not lhs.startswith("(")
        if is_index:
            depth = 0; split = -1
            for k in range(len(lhs) - 1, -1, -1):
                c = lhs[k]
                if c == "]": depth += 1
                elif c == "[":
                    depth -= 1
                    if depth == 0: split = k; break
            arr = lhs[:split]; idx = lhs[split + 1:-1]
            if delta == 1:
                return f"{arr}.ainc({idx})"
            if delta == -1:
                return f"{arr}.ainc({idx}, -1)"
            return f"{arr}.ainc({idx}, {delta})"
        if delta == 1:
            return f"{lhs}.inc()"
        if delta == -1:
            return f"{lhs}.dec()"
        return f"{lhs}.inc({delta})"

    def _is_simple_lvalue_at(self, pos: int) -> tuple[bool, int]:
        if pos >= self.end or self.tokens[pos][0] != "ID":
            return (False, pos)
        i = pos + 1
        while i < self.end and self.tokens[i][0] == "OP" and self.tokens[i][1] == "[":
            depth = 1; i += 1
            while i < self.end and depth > 0:
                if _is_op(self.tokens[i], "["): depth += 1
                elif _is_op(self.tokens[i], "]"): depth -= 1
                i += 1
        if i < self.end and self.tokens[i][0] == "OP" and self.tokens[i][1] in (",", ")"):
            return (True, i)
        return (False, i)

    def _parse_arg(self) -> "_ArgInfo":
        is_lv, _end = self._is_simple_lvalue_at(self.pos)
        lvalue_py: "str | None" = None
        if is_lv:
            save = self.pos
            lv_parser = Parser(self.tokens, save, _end)
            lv_text = lv_parser.parse_assignment()
            lvalue_py = lv_text
            self.pos = save
        py = self.parse_assignment()
        return _ArgInfo(py=py, lvalue_py=lvalue_py)

    def parse_postfix(self) -> str:
        s = self.parse_primary()
        while self.pos < self.end:
            k, v = self.peek_kind_val()
            if k == "OP" and v == "[":
                self.pos += 1
                idx = self.parse_expression()
                self.expect_op("]")
                s = f"{s}[{idx}]"
            elif k == "OP" and v == "(":
                self.pos += 1
                arg_infos: list[_ArgInfo] = []
                if not (self.pos < self.end and self.tokens[self.pos][0] == "OP" and self.tokens[self.pos][1] == ")"):
                    arg_infos.append(self._parse_arg())
                    while self.match_op(","):
                        arg_infos.append(self._parse_arg())
                self.expect_op(")")
                fn_name = s
                byref_flags = _BYREF_TABLE.get(fn_name, [])
                py_args = [ainfo.py for ainfo in arg_infos]
                # By-ref scalar args are passed directly — the caller's
                # Val IS the callee's param. Mutations propagate via
                # _setv mutating the shared Val in place. Aliasing
                # (same Val passed to multiple by-ref positions) Just
                # Works because Python shares the object.
                _ = byref_flags  # no special wrapping needed
                s = f"{fn_name}({', '.join(py_args)})"
            elif k == "OP" and v == "++":
                self.pos += 1; s = self._emit_postinc(s, +1)
            elif k == "OP" and v == "--":
                self.pos += 1; s = self._emit_postinc(s, -1)
            else: break
        return s

    def _emit_postinc(self, lhs: str, delta: int) -> str:
        # `x++` → `x.postinc()` (snapshot then mutate, returns the old value).
        # `arr[i]++` → `arr.apostinc(i)`.
        is_index = lhs.endswith("]") and "[" in lhs and not lhs.startswith("(")
        if is_index:
            depth = 0; split = -1
            for k in range(len(lhs) - 1, -1, -1):
                c = lhs[k]
                if c == "]": depth += 1
                elif c == "[":
                    depth -= 1
                    if depth == 0: split = k; break
            arr = lhs[:split]; idx = lhs[split + 1:-1]
            if delta == 1:
                return f"{arr}.apostinc({idx})"
            if delta == -1:
                return f"{arr}.apostinc({idx}, -1)"
            return f"{arr}.apostinc({idx}, {delta})"
        if delta == 1:
            return f"{lhs}.postinc()"
        if delta == -1:
            return f"{lhs}.postdec()"
        return f"{lhs}.postinc({delta})"

    def parse_primary(self) -> str:
        # Literals translate to plain Python values. The Val class's
        # operator overloads handle mixed arithmetic (e.g. `a + 5` calls
        # `Val.__add__` which wraps the bare 5), so we don't need to
        # wrap every literal in `Val(...)`.
        if self.pos >= self.end:
            raise SyntaxError("unexpected end of expression")
        t = self.tokens[self.pos]
        kind, val = t[0], t[1]
        if kind == "INT":
            self.pos += 1; return str(val)
        if kind == "FLOAT":
            self.pos += 1; return repr(val)
        if kind == "STR":
            self.pos += 1; return _py_escape_str(val)
        if kind == "CHAR":
            self.pos += 1; return _py_escape_char(val)
        if kind == "ID":
            err_pos = self.base + self.pos
            self.pos += 1
            nxt = self.tokens[self.pos] if self.pos < self.end else None
            is_call = nxt is not None and nxt[0] == "OP" and nxt[1] == "("
            if not is_call and not _ident_defined(val):
                global _LAST_PARSED_POS
                _LAST_PARSED_POS = err_pos
                raise _ParseErr(
                    f"undefined identifier '{val}'", _relative_line(t[2]), err_pos
                )
            return val
        if kind == "KW":
            if val == "true":  self.pos += 1; return "True"
            if val == "false": self.pos += 1; return "False"
            if val in ("cout", "cin"): self.pos += 1; return val
            if val == "Done": self.pos += 1; return "Done()"
            err_pos2 = self.base + self.pos
            raise _ParseErr(f"unexpected keyword '{val}'", _relative_line(t[2]), err_pos2)
        if kind == "OP" and val == "(":
            self.pos += 1
            s = self.parse_expression()
            self.expect_op(")")
            return f"({s})"
        got = val if kind == "OP" else str(val)
        err_pos3 = self.base + self.pos
        raise _ParseErr(f"unexpected token '{got}'", _relative_line(t[2]), err_pos3)


def expr_to_python(tokens, i, end) -> str:
    p = Parser(tokens, i, end)
    s = p.parse_expression()
    if not p.at_end():
        t = tokens[p.pos]
        got = t[1] if t[0] == "OP" else str(t[1])
        raise _ParseErr(f"unexpected token '{got}'", _relative_line(t[2]))
    return s


# ============== Declarations / statements / fns ==============


def translate_decl(tokens, i, end) -> tuple[str, list[str]]:
    ty = tokens[i][1]
    assert _is_type_kw(tokens[i])
    i += 1
    names: list[str] = []
    py_lines: list[str] = []
    while i < end:
        if tokens[i][0] != "ID":
            raise SyntaxError(f"expected identifier in declaration at line {tokens[i][2]}")
        name = tokens[i][1]
        i += 1
        if i < end and _is_op(tokens[i], "["):
            i += 1
            if tokens[i][0] != "INT":
                raise SyntaxError("expected array size")
            n = tokens[i][1]
            i += 1
            if not _is_op(tokens[i], "]"):
                raise SyntaxError("expected ']' in declaration")
            i += 1
            kind_name = {"int": "INT", "float": "FLOAT", "bool": "BOOL",
                         "char": "CHAR", "string": "STR"}.get(ty, "INT")
            py_lines.append(f"{name} = Val.make_arr(Val.{kind_name}, {n})")
        elif i < end and _is_op(tokens[i], "="):
            i += 1
            depth = 0; j = i
            while j < end:
                tk = tokens[j]
                if tk[0] == "OP":
                    if tk[1] in "([{": depth += 1
                    elif tk[1] in ")]}": depth -= 1
                    elif (tk[1] == "," or tk[1] == ";") and depth == 0:
                        break
                j += 1
            init_py = _clean_expr(expr_to_python(tokens, i, j))
            ctor = {"int": "Val.make_int", "float": "Val.make_float",
                    "bool": "Val.make_bool", "char": "Val.make_char",
                    "string": "Val.make_str"}.get(ty, "Val.make_int")
            # First create the slot, then plain `name = init` — Scope
            # catches the assignment and mutates the Val in place.
            py_lines.append(f"{name} = {ctor}()")
            py_lines.append(f"{name} = {init_py}")
            i = j
        else:
            ctor = {"int": "Val.make_int()", "float": "Val.make_float()",
                    "bool": "Val.make_bool()", "char": "Val.make_char()",
                    "string": "Val.make_str()"}.get(ty, "None")
            py_lines.append(f"{name} = {ctor}")
        names.append(name)
        if i < end and _is_op(tokens[i], ","):
            i += 1; continue
        break
    if i < end and _is_op(tokens[i], ";"):
        i += 1
    return "\n".join(py_lines), names


def translate_stmts_block(tokens, i, end, indent: str) -> tuple[str, int]:
    out: list[str] = []
    while i < end:
        tk = tokens[i]
        if _is_op(tk, "}"):
            break
        py, i = translate_one_stmt(tokens, i, end, indent)
        if py:
            out.append(py)
    return ("\n".join(out) if out else f"{indent}pass"), i


import ast as _ast


def _clean_expr(text: str) -> str:
    """Re-parse and ``ast.unparse`` an expression to strip redundant
    parens that the recursive-descent emitter inserts."""
    try:
        node = _ast.parse(text, mode="eval").body
    except SyntaxError:
        return text
    try:
        return _ast.unparse(node)
    except Exception:
        return text


def _flatten_stmt_walrus(expr_text: str) -> str:
    """Rewrite an expression statement so it reads like the Our-C source:

    - ``(a := (b := X))``               → ``a = b = X``
    - ``arr.aset(i, X)``                → ``arr[i] = X``
    - ``arr.aset(i, (a := X))``         → ``arr[i] = a = X``
    - ``x.postinc()`` / ``x.inc()``     → ``x += 1``
    - ``x.postdec()`` / ``x.dec()``     → ``x -= 1``
    - ``arr.apostinc(i)`` / ``arr.ainc(i)`` → ``arr[i] += 1``
    - any leftover expression           → ``ast.unparse``-normalised
      (extra wrapping parens removed)

    Behaviour is identical (Scope.__setitem__ intercepts named writes;
    Val.__setitem__ intercepts ``arr[i] = X``).  This is purely cosmetic.
    """
    try:
        node = _ast.parse(expr_text, mode="eval").body
    except SyntaxError:
        return expr_text

    # Statement-level i.postinc()/etc. — discard the snapshot, just bump.
    inc_methods = {"postinc": +1, "inc": +1, "postdec": -1, "dec": -1}
    a_inc_methods = {"apostinc": +1, "ainc": +1}
    if (isinstance(node, _ast.Call)
            and isinstance(node.func, _ast.Attribute)
            and not node.keywords):
        attr = node.func.attr
        # x.postinc() / x.dec() / etc.
        if attr in inc_methods and len(node.args) == 0:
            delta = inc_methods[attr]
            op = _ast.Add() if delta > 0 else _ast.Sub()
            stmt = _ast.AugAssign(
                target=_ast.copy_location(_ast.Name(id=_ast.unparse(node.func.value),
                                                    ctx=_ast.Store()), node),
                op=op,
                value=_ast.Constant(value=abs(delta)),
            )
            mod = _ast.Module(body=[stmt], type_ignores=[])
            _ast.fix_missing_locations(mod)
            return _ast.unparse(mod).strip()
        # arr.apostinc(i) / arr.ainc(i)
        if attr in a_inc_methods and len(node.args) == 1:
            delta = a_inc_methods[attr]
            op = _ast.Add() if delta > 0 else _ast.Sub()
            arr_text = _ast.unparse(node.func.value)
            idx_text = _ast.unparse(node.args[0])
            return f"{arr_text}[{idx_text}] {'+=' if delta > 0 else '-='} 1"

    targets = []
    cur = node
    while True:
        if isinstance(cur, _ast.NamedExpr):
            targets.append(cur.target)
            cur = cur.value
            continue
        if (isinstance(cur, _ast.Call)
                and isinstance(cur.func, _ast.Attribute)
                and cur.func.attr == "aset"
                and len(cur.args) == 2
                and not cur.keywords):
            arr_node = cur.func.value
            idx_node = cur.args[0]
            sub = _ast.Subscript(
                value=arr_node,
                slice=idx_node,
                ctx=_ast.Store(),
            )
            targets.append(sub)
            cur = cur.args[1]
            continue
        break

    if not targets:
        # No assignment chain — at least normalise parens.
        try:
            return _ast.unparse(node)
        except Exception:
            return expr_text

    for t in targets:
        if isinstance(t, _ast.Name):
            t.ctx = _ast.Store()
        elif isinstance(t, _ast.Subscript):
            t.ctx = _ast.Store()

    assign = _ast.Assign(targets=targets, value=cur, type_comment=None)
    mod = _ast.Module(body=[assign], type_ignores=[])
    _ast.fix_missing_locations(mod)
    return _ast.unparse(mod).strip()


def translate_one_stmt(tokens, i, end, indent: str) -> tuple[str, int]:
    tk = tokens[i]
    if _is_op(tk, "{"):
        i += 1
        body, i = translate_stmts_block(tokens, i, end, indent + "    ")
        if i < end and _is_op(tokens[i], "}"):
            i += 1
        return body, i
    if _is_op(tk, ";"):
        return "", i + 1
    if _is_kw(tk, "if"):
        i += 1
        if not _is_op(tokens[i], "("):
            raise SyntaxError("expected '(' after if")
        i += 1
        depth = 1; j = i
        while j < end and depth > 0:
            if _is_op(tokens[j], "("): depth += 1
            elif _is_op(tokens[j], ")"): depth -= 1
            if depth > 0: j += 1
        cond = _clean_expr(expr_to_python(tokens, i, j))
        i = j + 1
        then_py, i = translate_one_stmt(tokens, i, end, indent + "    ")
        out = f"{indent}if {cond}:\n{then_py if then_py else indent + '    pass'}"
        if i < end and _is_kw(tokens[i], "else"):
            i += 1
            else_py, i = translate_one_stmt(tokens, i, end, indent + "    ")
            out += f"\n{indent}else:\n{else_py if else_py else indent + '    pass'}"
        return out, i
    if _is_kw(tk, "while"):
        i += 1
        if not _is_op(tokens[i], "("):
            raise SyntaxError("expected '(' after while")
        i += 1
        depth = 1; j = i
        while j < end and depth > 0:
            if _is_op(tokens[j], "("): depth += 1
            elif _is_op(tokens[j], ")"): depth -= 1
            if depth > 0: j += 1
        cond = _clean_expr(expr_to_python(tokens, i, j))
        i = j + 1
        body_py, i = translate_one_stmt(tokens, i, end, indent + "    ")
        return f"{indent}while {cond}:\n{body_py if body_py else indent + '    pass'}", i
    if _is_kw(tk, "return"):
        i += 1
        if _is_op(tokens[i], ";"):
            return f"{indent}raise _ReturnSignal()", i + 1
        depth = 0; j = i
        while j < end:
            ttk = tokens[j]
            if ttk[0] == "OP":
                if ttk[1] in "([{": depth += 1
                elif ttk[1] in ")]}": depth -= 1
                elif ttk[1] == ";" and depth == 0: break
            j += 1
        expr = expr_to_python(tokens, i, j)
        return f"{indent}raise _ReturnSignal({expr})", j + 1
    if _is_type_kw(tk):
        depth = 0; j = i
        while j < end:
            ttk = tokens[j]
            if ttk[0] == "OP":
                if ttk[1] in "([{": depth += 1
                elif ttk[1] in ")]}": depth -= 1
                elif ttk[1] == ";" and depth == 0: j += 1; break
            j += 1
        py, _names = translate_decl(tokens, i, j)
        lines = [indent + ln for ln in py.split("\n")]
        return "\n".join(lines), j
    depth = 0; j = i
    while j < end:
        ttk = tokens[j]
        if ttk[0] == "OP":
            if ttk[1] in "([{": depth += 1
            elif ttk[1] in ")]}": depth -= 1
            elif ttk[1] == ";" and depth == 0: break
        j += 1
    p = Parser(tokens, i, j)
    expr = p.parse_expression()
    if not p.at_end():
        t = tokens[p.pos]
        got = t[1] if t[0] == "OP" else str(t[1])
        raise _ParseErr(f"unexpected token '{got}'", _relative_line(t[2]))
    # Statement-level pretty-printing: chain of walrus / .aset() becomes
    # a plain Python assignment so the emit reads like the source.
    expr = _flatten_stmt_walrus(expr)
    lines = []
    for pre in p.preamble:
        lines.append(f"{indent}{pre}")
    lines.append(f"{indent}{expr}")
    for post in p.deferred:
        lines.append(f"{indent}{post}")
    return "\n".join(lines), (j + 1 if j < end else j)


def _collect_assigned_names(tokens, start, end) -> set[str]:
    names = set()
    i = start
    while i < end:
        t = tokens[i]
        if t[0] == "ID" and i + 1 < end:
            nxt = tokens[i + 1]
            if nxt[0] == "OP" and nxt[1] in ("=", "+=", "-=", "*=", "/=", "%=", "++", "--"):
                names.add(t[1])
        i += 1
    return names


def _collect_local_decls(tokens, start, end) -> set[str]:
    names = set()
    i = start
    while i < end:
        t = tokens[i]
        if _is_type_kw(t):
            j = i + 1
            while j < end:
                tk = tokens[j]
                if tk[0] == "ID":
                    names.add(tk[1])
                if tk[0] == "OP" and tk[1] == ";":
                    break
                j += 1
        i += 1
    return names


def _scan_shadowing_locals(tokens, body_start, body_end, params, globals_set):
    renames: list[tuple[int, str, str]] = []
    counter: dict[str, int] = {}
    param_names = {p[1] for p in params}
    i = body_start
    while i < body_end:
        t = tokens[i]
        if _is_type_kw(t):
            j = i + 1
            while j < body_end:
                tk = tokens[j]
                if tk[0] == "ID":
                    nm = tk[1]
                    if nm in globals_set and nm not in param_names:
                        counter[nm] = counter.get(nm, 0) + 1
                        new = f"_loc_{nm}_{counter[nm]}"
                        renames.append((j, nm, new))
                if tk[0] == "OP" and tk[1] == ";":
                    break
                j += 1
            i = j + 1
            continue
        i += 1
    return renames


def _apply_renames(tokens, body_start, body_end, renames):
    if not renames:
        return tokens[body_start:body_end]
    decl_by_name: dict[str, list[tuple[int, str]]] = {}
    for idx, orig, new in renames:
        decl_by_name.setdefault(orig, []).append((idx, new))
    for k in decl_by_name:
        decl_by_name[k].sort()
    out = []
    for i in range(body_start, body_end):
        t = tokens[i]
        if t[0] == "ID" and t[1] in decl_by_name:
            new_name = t[1]
            for didx, candidate in decl_by_name[t[1]]:
                if i >= didx:
                    new_name = candidate
                else: break
            if new_name != t[1]:
                out.append(("ID", new_name, t[2]))
                continue
        out.append(t)
    return out


def translate_fndef(tokens, globals_set: "set[str]", chunk_base: int = 0) -> "tuple[str, str, list[bool]]":
    name = tokens[1][1]
    i = 3
    params: list[tuple[str, str, bool, bool]] = []
    if not _is_op(tokens[i], ")"):
        while True:
            ty = tokens[i][1]; i += 1
            by_ref = False
            if _is_op(tokens[i], "&"):
                by_ref = True; i += 1
            pname = tokens[i][1]; i += 1
            is_array = False
            if _is_op(tokens[i], "["):
                is_array = True; i += 1
                if tokens[i][0] == "INT": i += 1
                if _is_op(tokens[i], "]"): i += 1
            params.append((ty, pname, is_array, by_ref))
            if _is_op(tokens[i], ","):
                i += 1; continue
            break
    if not _is_op(tokens[i], ")"):
        raise SyntaxError("expected ')'")
    i += 1
    if not _is_op(tokens[i], "{"):
        raise SyntaxError("expected '{'")
    i += 1
    end_body = i; depth = 1
    while end_body < len(tokens) and depth > 0:
        if _is_op(tokens[end_body], "{"): depth += 1
        elif _is_op(tokens[end_body], "}"):
            depth -= 1
            if depth == 0: break
        end_body += 1

    renames = _scan_shadowing_locals(tokens, i, end_body, params, globals_set)
    body_tokens = _apply_renames(tokens, i, end_body, renames)

    by_ref_names_set = {p[1] for p in params if (not p[2]) and p[3]}

    # Set up the parsing scope: params + all locally declared names
    # (so identifier lookup during expression parsing succeeds).
    fn_scope = set()
    fn_scope.update(p[1] for p in params)
    fn_scope.update(_collect_local_decls(body_tokens, 0, len(body_tokens)))
    # Add the renamed shadowing locals as well.
    fn_scope.update(new for _, _, new in renames)

    # Set the by-ref locals context so the Parser emits _setv for these.
    global _CURRENT_BYREF_LOCALS, _CURRENT_BASE
    saved_byref = _CURRENT_BYREF_LOCALS
    saved_base = _CURRENT_BASE
    _CURRENT_BYREF_LOCALS = by_ref_names_set
    # i is the body's start within ``tokens`` (a slice); add chunk_base
    # so the resulting _CURRENT_BASE refers to the global token index.
    _CURRENT_BASE = chunk_base + i
    _LOCAL_SCOPES.append(fn_scope)
    try:
        body_py, _ = translate_stmts_block(body_tokens, 0, len(body_tokens), "    ")
    finally:
        _CURRENT_BYREF_LOCALS = saved_byref
        _CURRENT_BASE = saved_base
        _LOCAL_SCOPES.pop()

    if not body_py.strip():
        body_py = "    pass"

    param_names = {p[1] for p in params}
    local_names = _collect_local_decls(body_tokens, 0, len(body_tokens))
    assigned_names = _collect_assigned_names(body_tokens, 0, len(body_tokens))
    by_ref_names = sorted(by_ref_names_set)
    globals_needed = sorted(
        n for n in assigned_names
        if n not in param_names and n not in local_names and n not in by_ref_names
    )

    by_ref_flags = [(not p[2]) and p[3] for p in params]
    py_params = [p[1] for p in params]

    # The function-definition emit is ONE line:
    #   <name> = _Fn('<name>', [param_names], [byref_flags], '''
    #       body source ...
    #   ''')
    # The _Fn callable handles the wrap-call-catch on each invocation.
    # ``body source`` is the clean, exec-ready Python translation —
    # exactly the part a reader wants to see.
    body_lines = body_py.split("\n")
    dedent = []
    for ln in body_lines:
        if ln.startswith("    "):
            dedent.append(ln[4:])
        else:
            dedent.append(ln)
    body_source = "\n".join(dedent)

    # Use triple-quoted literal for readability in the emit.
    body_literal = repr(body_source)
    param_names_repr = "[" + ", ".join(repr(p[1]) for p in params) + "]"
    byref_repr = "[" + ", ".join("True" if f else "False" for f in by_ref_flags) + "]"
    line = (
        f"{name} = _Fn({name!r}, {param_names_repr}, {byref_repr}, "
        f"{body_literal})"
    )
    return line, name, by_ref_flags


# ============== Driver ==============


def run(src: str, out_stream=None):
    if out_stream is None:
        out_stream = sys.stdout
    out_stream.write("Our-C running ...\n")

    tokens = tokenize(src)
    cout_obj = _Cout()
    _BYREF_TABLE.clear()
    _GLOBAL_SCOPE.clear()
    _LOCAL_SCOPES.clear()
    _FN_DEFS.clear()

    # The top-level namespace is a Scope so that `a = 10` at the top
    # mutates the existing Val instead of rebinding the name. The same
    # Scope is passed as ``globals`` when we exec function bodies, with
    # a fresh inner Scope as ``locals`` — that way assignments inside
    # functions also route through Scope.__setitem__.
    namespace = Scope()
    _TOP_SCOPE_REF[0] = namespace
    top_scope_init = {
        "Val": Val,
        "Scope": Scope,
        "cout": cout_obj,
        "cin": _Cin(),
        "Done": _done,
        "_Fn": _Fn,
        "_DoneSignal": _DoneSignal,
        "_ReturnSignal": _ReturnSignal,
    }
    for _k, _v in top_scope_init.items():
        dict.__setitem__(namespace, _k, _v)

    def _scan_past_fn_brace(start: int) -> int:
        """When a function-definition chunk fails to parse, advance the
        token cursor to just past the function's opening ``{`` so the
        recovery sweep operates inside the body (matching the C++
        reference's behaviour of abandoning the failed fn definition and
        resuming top-level parsing on the remaining body tokens)."""
        k = start
        # skip type, ident, (...)
        if k < len(tokens) and _is_type_kw(tokens[k]): k += 1
        if k < len(tokens) and tokens[k][0] == "ID": k += 1
        if k < len(tokens) and _is_op(tokens[k], "("):
            depth = 1; k += 1
            while k < len(tokens) and depth > 0:
                if _is_op(tokens[k], "("): depth += 1
                elif _is_op(tokens[k], ")"): depth -= 1
                k += 1
        if k < len(tokens) and _is_op(tokens[k], "{"):
            k += 1
        return k

    global _CHUNK_START_LINE, _CURRENT_BYREF_LOCALS, _CURRENT_BASE
    i = 0
    while i < len(tokens):
        _CHUNK_START_LINE = tokens[i][2]
        _CURRENT_BASE = i  # top-level chunk slice base
        chunk_start = i
        try:
            chunk, next_i = find_one_chunk(tokens, i)
            if chunk is None:
                break
            if chunk.kind == "done":
                out_stream.write("> Our-C exited ...\n")
                return
            if chunk.kind == "decl":
                py, names = translate_decl(chunk.tokens, 0, len(chunk.tokens))
                exec(py, namespace)
                for n in names: _GLOBAL_SCOPE.add(n)
                out_stream.write("> ")
                out_stream.write("\n".join(f"Definition of {n} entered ..." for n in names))
                out_stream.write("\n")
                i = next_i
                continue
            if chunk.kind == "fndef":
                py, name, by_ref_flags = translate_fndef(chunk.tokens, _GLOBAL_SCOPE, chunk_base=chunk_start)
                _BYREF_TABLE[name] = by_ref_flags
                _FN_DEFS.add(name)
                exec(py, namespace)
                out_stream.write(f"> Definition of {name}() entered ...\n")
                i = next_i
                continue
            # Translate first, then emit "> " (so a parse error doesn't
            # leave a dangling "> " before the Line-N error message).
            py, _ = translate_one_stmt(chunk.tokens, 0, len(chunk.tokens), "")
            out_stream.write("> ")
            try:
                cout_obj.drain()
                exec(py, namespace)
                out_stream.write(cout_obj.drain())
                out_stream.write("Statement executed ...\n")
            except _DoneSignal:
                out_stream.write(cout_obj.drain())
                out_stream.write("Our-C exited ...\n")
                return
            except Exception as e:
                out_stream.write(cout_obj.drain())
                out_stream.write(f"{e}\n")
            i = next_i
        except _ParseErr as e:
            out_stream.write(f"> Line {e.line} : {e.msg}\n")
            _LOCAL_SCOPES.clear()
            _CURRENT_BYREF_LOCALS = set()
            start_at = e.abs_pos if e.abs_pos is not None else chunk_start
            new_i = _recover_to_next_chunk(tokens, start_at)
            # Safety net: ensure progress so we never loop on the same
            # position. If recovery didn't move past where we started,
            # nudge forward by one.
            if new_i <= i:
                new_i = i + 1
            i = new_i


def main():
    if len(sys.argv) > 1:
        with open(sys.argv[1], "r", encoding="utf-8") as f:
            src = f.read()
    else:
        src = sys.stdin.read()
    run(src)


if __name__ == "__main__":
    main()

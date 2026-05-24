#!/usr/bin/env python3
"""Show what the translator actually feeds into exec/eval.

Run: python3 demo.py

For each Our-C snippet, prints the source and the Python text the
translator emits.  That Python text is exec()-ed against a namespace
holding the Val class and a few small helpers (_setv, _ai, _Cout, …).
"""

import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import interp


SAMPLES = [
    ("1. Plain arithmetic + C-style div / mod",
     "int a, b ;\n"
     "a = 10 ;\n"
     "b = 305 % a ;\n"
     "b = a / 3 ;\n"
     "b = 7 + 3 * 5 ;\n"),

    ("2. String + int (Val.__add__ handles concat)",
     'string s ; int n ;\n'
     'n = 7 ;\n'
     's = "hi " + n ;\n'),

    ("3. cout chain with embedded chained assignment",
     'int a[10] ; int b, c, x ;\n'
     'cout << ( a[1] = x = b + c * c ) << "\\n" ;\n'),

    ("4. while loop with array index and i++",
     "int arr[10] ; int i ;\n"
     "i = 0 ;\n"
     "while ( i < 10 ) {\n"
     "  arr[i] = i * 2 ;\n"
     "  i++ ;\n"
     "}\n"),

    ("5. By-value scalar — copied on entry so caller is independent",
     "int F( int x ) {\n"
     "  x = x + 5 ;\n"
     "  return x ;\n"
     "}\n"),

    ("6. By-reference scalar — Val passed directly, _setv mutates in place",
     "void Bump( int & target, int delta ) {\n"
     "  target = target + delta ;\n"
     "}\n"),

    ("7. Mid-function shadowing — locally declared 'a' renamed to _loc_a_1",
     "int a ; a = 1 ;\n"
     "int F( int x ) {\n"
     "  int y ;\n"
     "  y = a + x ;       // global a here\n"
     "  int a ;            // local shadowing starts here\n"
     "  a = 100 ;\n"
     "  y = y + a ;\n"
     "  return y ;\n"
     "}\n"),

    ("8. Call with by-ref args — caller's Val passed directly (no boxing)",
     "int x ; x = 10 ;\n"
     "void Bump( int & t, int d ) { t = t + d ; }\n"
     "Bump( x, 5 ) ;\n"
     "cout << x ;\n"),

    ("9. Aliasing — same variable passed to two by-ref positions",
     "int x ; x = 10 ;\n"
     "void F( int & a, int & b ) { a = a - 2 ; b = b + 3 ; }\n"
     "F( x, x ) ;\n"
     'cout << x ;        // -2 then +3 chained on the SAME Val -> 11\n'),
]


def translate_chunk(chunk):
    """Translate one chunk (after find_one_chunk yielded it) to its
    emitted Python source text."""
    if chunk.kind == "decl":
        py, names = interp.translate_decl(chunk.tokens, 0, len(chunk.tokens))
        return ("decl(" + ", ".join(names) + ")", py)
    if chunk.kind == "fndef":
        py, name, _ = interp.translate_fndef(chunk.tokens, interp._GLOBAL_SCOPE)
        return (f"fndef({name})", py)
    if chunk.kind == "stmt":
        py, _ = interp.translate_one_stmt(chunk.tokens, 0, len(chunk.tokens), "")
        return ("stmt", py)
    if chunk.kind == "done":
        return ("done", "raise _DoneSignal()  # emit '> Our-C exited ...'")
    return (chunk.kind, "")


def emit(title, src):
    print("=" * 72)
    print(f"  {title}")
    print("=" * 72)
    print("--- Our-C ---")
    for ln in src.rstrip("\n").splitlines():
        print(f"  {ln}")
    print()
    print("--- emitted Python (fed to exec) ---")

    # Reset module state so translations don't leak between samples that
    # share global-scope names.
    interp._GLOBAL_SCOPE.clear()
    interp._LOCAL_SCOPES.clear()
    interp._FN_DEFS.clear()
    interp._BYREF_TABLE.clear()
    interp._CURRENT_BYREF_LOCALS = set()
    interp._CHUNK_START_LINE = 1
    interp._CURRENT_BASE = 0

    tokens = interp.tokenize(src)
    i = 0
    while i < len(tokens):
        interp._CHUNK_START_LINE = tokens[i][2]
        interp._CURRENT_BASE = i
        chunk, next_i = interp.find_one_chunk(tokens, i)
        if chunk is None:
            break
        label, py = translate_chunk(chunk)
        # Side-effect: register names so subsequent chunks see them
        # (matches what run() does).
        if chunk.kind == "decl":
            _, names = interp.translate_decl(chunk.tokens, 0, len(chunk.tokens))
            for n in names:
                interp._GLOBAL_SCOPE.add(n)
        elif chunk.kind == "fndef":
            _, name, flags = interp.translate_fndef(
                chunk.tokens, interp._GLOBAL_SCOPE
            )
            interp._BYREF_TABLE[name] = flags
            interp._FN_DEFS.add(name)
        print(f"  # {label}")
        for ln in py.split("\n"):
            print(f"  {ln}")
        i = next_i
    print()


def main():
    for title, src in SAMPLES:
        emit(title, src)


if __name__ == "__main__":
    main()

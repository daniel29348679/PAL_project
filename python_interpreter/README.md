# Python "token → eval" Interpreter (Val-class edition)

A Python implementation of the Our-C interpreter built around two ideas:

1. **Rewrite the Our-C tokens into Python source text, then let Python's
   own `exec` / `eval` do the heavy lifting.** No tree-walking evaluator
   — Python's parser and runtime handle operator precedence, control
   flow, and dispatch.
2. **Define `Val` (a Rust-enum-like tagged value with `kind` ∈ {INT,
   FLOAT, BOOL, CHAR, STR, ARRAY}) and overload every Python operator on
   it to carry the C semantics.** So `a + b` in Our-C translates to the
   literal Python text `(a + b)`, and Python's `Val.__add__` gives it
   string-aware concat / int-vs-float promotion. `a / b` translates to
   `(a / b)` and `Val.__truediv__` does C-style truncate-toward-zero. Etc.

## Run

```bash
python3 interp.py < ../測資/4-2.txt     # one test
python3 runtests.py                      # all 4-*.txt tests
python3 runtests.py -v                   # show diffs on failure
```

## Results

```
[PASS] 4-1.txt  (Test cast 1 of 3)    97/97
[PASS] 4-1.txt  (Test case 2 of 3)    37/37
[PASS] 4-2.txt  (Test case 1)         119/119
[PASS] 4-3.txt  (Test case 1 of 3)    210/210
[PASS] 4-4.txt  (Test case 1 of 3)    109/109
[PASS] 4-6.txt  (Test case 1 of 4)    210/210
[PASS] 4-8.txt  (Test case 1 of 3)    204/204
[PASS] 4-9.txt  (Test case 1 of 3)    321/321
[PASS] 4-10.txt (Test case 1 of 3)    203/203
[PASS] 4-12.txt (Test case 1 of 3)    389/389
[PASS] 4-12.txt (Test case 2 of 3)    273/273

Total: 11/11 sub-tests pass (every line matches the reference).
```

## How it works

1. **Tokenize** the Our-C source (per-token line numbers preserved).
2. **`find_one_chunk`**: walk the token stream, yielding one top-level
   chunk at a time — declaration / function definition / statement /
   `Done()`.
3. **Per chunk**, translate tokens into Python source text:
   - **Literals** are wrapped: `10` → `Val(Val.INT, 10)`, `"hi"` →
     `Val(Val.STR, "hi")`, etc.
   - **Operators stay literal**: `a + b` → `(a + b)`,  `a / 3` →
     `(a / 3)`,  `a << 2` → `(a << 2)`,  `a < b` → `(a < b)`. Val's
     `__add__` / `__truediv__` / `__lshift__` / `__lt__` etc. provide C
     semantics. The only token-mappings are the ones with no Python
     equivalent: `&&` → `and`, `||` → `or`, `!` → `not`, `true`/`false`
     → `True`/`False`.
   - **Assignments** become walrus-style mutating ops via `_setv`
     (mutate the Val's `kind` and `v` in place — so by-ref aliases see
     the change immediately). Indexed assignment `arr[i] = v` becomes
     `_ai(arr, i, v)` (`__setitem__` can't be an expression in Python).
   - **`cout` chains** are translated literally as
     `(cout << x << y << "\n")`; `_Cout` overloads `<<` for type-aware
     printing (`%.3f` for floats, `true`/`false` for bools).
   - **Control flow** (`if`, `else`, `while`) turns into Python ones
     with `{ … }` → indentation; `return x ;` becomes
     `raise _ReturnSignal(x)` (caught by the function's `def` wrapper).
4. **Execute** each chunk's emitted Python with `exec(py, namespace)`.
   Output captured from `cout.buf` is wrapped in the
   `>` / `Statement executed ...` framing.

## What the Val class buys us

Every C semantic that differs from Python's lives in **one place** — a
dunder method on `Val`:

| Operation | Python default | Our-C / C | Where it's handled |
|---|---|---|---|
| `a + b` for `string + int` | TypeError | string concat | `Val.__add__` |
| `a / b` for negative ints | floors (`-7 / 5 == -2`) | trunc toward 0 (`-1`) | `Val.__truediv__` |
| `a % b` for negative dividend | sign of divisor (`-7 % 5 == 3`) | sign of dividend (`-2`) | `Val.__mod__` |
| `a < b` printing | Python `bool` repr | `"true"` / `"false"` | `Val(Val.BOOL, …)` + `Val.show` |
| `cout << f` for float | n/a | `%.3f` | `Val.show` |

The translator stays trivial; the interesting code lives in the type.

## Subtleties that came up

1. **Python's "assigned anywhere → local" rule.** A `def` that ever
   assigns to `gY` makes `gY` a local everywhere, so reading `gY` before
   the assignment is `UnboundLocalError`. Our-C lets you use a global,
   then *later* declare a same-named local that shadows. We resolve this
   by translating **every** assignment to `_setv(target, value)` — which
   mutates the existing `Val` in place rather than rebinding the name —
   so Python never sees the assignment as a rebind and continues to
   resolve the name in the enclosing scope.

2. **Mid-function shadowing local declarations** (e.g. `int gV[10] ;`
   inside `F51` when there's already a global `gV`). We scan the body
   for these and **positionally rename** to `_loc_gV_1`, rewriting all
   subsequent uses; earlier uses still see the global.

3. **By-reference scalar parameters.** Python has no `int*`. We pass the
   caller's `Val` object directly (no boxing) and let `_setv` mutate
   `.v` in place. Caller and callee share a single `Val` instance — so
   the change is seen immediately, even mid-expression. This also makes
   **aliasing work naturally**: `F91(c, d, w[2], d)` passes the same
   `Val` twice, and Python object identity does the rest.

4. **Array element aliasing.** `_ai(arr, i, v)` mutates the existing
   element `Val` in place (rather than replacing it with a new `Val`),
   so any outstanding by-ref alias to that element still sees the new
   content.

5. **By-value scalar parameters need copying.** With mutating `_setv`,
   `c = c + 5` in a function would otherwise mutate the caller's `Val`
   too. Each function prologue does `c = Val(c.kind, c.v)` for every
   by-value scalar param so they're independent.

6. **Error recovery (4-12).** Parse errors raise `_ParseErr(msg, line,
   abs_pos)` carrying a chunk-relative line number and the absolute
   token position. The driver formats `> Line N : msg` and skips to the
   next `;`-or-`}` boundary starting from the actual error position
   (not the chunk start), so the rest of the broken function body is
   re-parsed as fresh top-level chunks. Undefined-identifier checks
   happen at parse time using a small symbol table.

## What the experiment proved

`exec` / `eval` really did carry most of the weight — Python's parser
handled all operator precedence and associativity for free, and Val's
dunder methods let the **same Python text** mean C semantics. The
custom interpreter logic we still had to write was nearly all about
**bridging Python's scoping and reference model to Our-C's** — not
about evaluation itself.

Lines of code (excluding tests / README): **~1100 Python**, vs **~1300
Rust** for the tree-walking equivalent.

## Layout

```
interp.py    tokeniser, chunk splitter, Val class with overloads,
             translator, runtime helpers (_setv / _ai / _Cout),
             exec-based driver with parse-error recovery
runtests.py  runs every 4-*.txt sub-test and reports line-diff counts
```

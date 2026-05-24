
# PAL_project

CYCU ICE — PAL Project 1 & Project 4 test data and answers.

## Why I’m sharing this repository

I’m sharing my implementation to help junior students learn from a complete, working example and to get inspiration for finishing the projects.  
**Please use this repository for learning and reference only. Do not submit other people’s code as your own.**

---

## Project 4 — Execution / Evaluation Logic (High-Level)

### `main()`
- Receives input data.
- Decides how many data items to read and evaluate.

### `check()`
- Receives tokens.
- Validates syntax (checks for grammar errors).
- Calls `pushasm()` to store the tokenized results into a 2D array for later evaluation.

### `caculate()` (recursive evaluator)
- Performs most computations using recursion.  
  Examples: `1 + 2`, `cout << 123`, etc.

### Handling `if`, `else`, `while`
- Uses the branch target computed by `pushasm()` (e.g., `br`), similar to assembly branching.
- By “branching” to the appropriate target position, control-flow behavior can be implemented.

### `cout` handling
- In C++, `cout` is effectively a static object; conceptually it behaves like a normal class instance.
- To implement output, you can treat it similarly to handling `operator+`:
  - During evaluation of the left-shift operator (`<<`), check whether the left-hand side is `cout`.
  - If it is, print the right-hand value.

---

## Function handling

### By value example

Original definition:

```cpp
int double(int x) // by value
{
    return x * 2;
}
````

Call: `double(2);`

Approach:

1. Create a temporary variable: `tempvar_0 = 2;`
   (Increment the tempvar index each time to avoid reuse.)
2. Copy the original function definition.
3. Replace all occurrences of `x` in the copied function with `tempvar_0`.
4. Execute the copied/rewritten function body.

After substitution:

```cpp
int double(int tempvar_0) // by value
{
    return tempvar_0 * 2;
}
```

### By reference example

```cpp
void addone(int &x) // by ref
{
    x++;
}
```

Assume the call is: `addone(y);`

For by-reference semantics:

* Replace all occurrences of `x` inside the function with `y`.

After substitution:

```cpp
void addone(int &y) // by ref
{
    y++;
}
```

---

## More details

If you want deeper execution details, open `printasm()` in `main()`.
It prints more granular trace information about the evaluation process.

---

## Update — 2026/5/24: two more interpreter versions

On 2026/5/24 I added two completely new implementations of the same
Our-C interpreter, sitting alongside the original C++ in this repo.
**Neither touches `main.cpp` or `proj4.cpp`** — they were written from
scratch using the C++ as a spec reference and tested against the same
`測資/4-*.txt` data.

### 1. `rust_interpreter/` — a from-scratch Rust port

A traditional interpreter: tokenizer → AST parser → tree-walking
evaluator, with `Rc<RefCell<Value>>` for slot-level aliasing. About
1300 lines. **Passes all 11 sub-tests with byte-exact output**,
including the `4-12.txt` error-recovery stress test (chunk-relative
`Line N : …` error messages, undefined-identifier checks, and
abandon-and-resume parser recovery).

### 2. `python_interpreter/` — the "let `exec` do the work" experiment

This one is the more interesting story. I wanted to know: **how much
of an interpreter can I avoid writing if I rewrite the Our-C tokens
into Python source text and just feed it to `exec` / `eval`?**

The idea:

> Python already has a parser that knows operator precedence,
> associativity, control-flow keywords, function calls — everything.
> If I can convert `a + b` in Our-C into the literal Python text
> `a + b`, Python's runtime will compute the result for me. No
> tree-walking evaluator. No precedence climber.

The catch: Python and C have **different semantics for the same
operator symbols**. `int / int` in Python returns a float; in C it
truncates to an int. `%` for negative dividends has the sign of the
divisor in Python and the sign of the dividend in C. `"hi" + 1` is a
`TypeError` in Python but legal string concatenation in C-style cout.
And Python's `a = 10` always rebinds the name `a`, which would break
by-reference parameter aliasing.

The whole interpreter is the bridge for those gaps:

1. **`Val` — a Rust-enum-style tagged value type** (`kind` ∈ {INT,
   FLOAT, BOOL, CHAR, STR, ARRAY}) with every Python operator
   overloaded to do C semantics. `Val.__add__` does string-aware
   concat, `Val.__truediv__` does truncate-toward-zero, `Val.__mod__`
   does sign-of-dividend, `Val.__lt__` returns `Val(BOOL, …)`, etc.
   So `a + b` in the emitted Python is *literally* `a + b` — Python
   dispatches to `Val.__add__` which produces the right answer.
2. **`Scope` — a `dict` subclass that intercepts `name = value`**
   inside `exec(code, globals, scope)`. When the body assigns to a
   name whose current binding is a `Val`, `Scope.__setitem__` mutates
   the existing `Val` in place rather than rebinding the name. That's
   what preserves object identity for by-reference parameter
   aliasing — both caller and callee hold the same `Val` instance,
   and every assignment mutates `.v` in place.
3. **Function bodies are exec'd, not `def`-ed.** Inside a real
   `def F():`, Python's compiler emits `STORE_FAST` for local
   assignments, bypassing any custom dict. So each Our-C function
   becomes a `_Fn(params, byref_flags, body_source)` callable that
   builds a fresh `Scope` per call and runs `exec(body, top_scope,
   scope)`. That way every `a = X` inside the body goes through
   `Scope.__setitem__` too.
4. **AST post-processing makes the emit read like the source.**
   `(a := X)` walrus → `a = X`. `arr.aset(i, X)` → `arr[i] = X`.
   `x.postinc()` → `x += 1`. Redundant parens stripped via
   `ast.unparse`.

Concretely, what the translator emits for some Our-C lines:

| Our-C                                 | Python fed to `exec`                                |
| ------------------------------------- | --------------------------------------------------- |
| `int a, b ;`                          | `a = Val.make_int()` / `b = Val.make_int()`         |
| `a = 10 ;`                            | `a = 10`                                            |
| `b = 305 % a ;`                       | `b = 305 % a`                                       |
| `s = "hi " + n ;`                     | `s = 'hi ' + n`                                     |
| `arr[i] = i * 2 ;`                    | `arr[i] = i * 2`                                    |
| `i++ ;`                               | `i += 1`                                            |
| `b1 = a < c && c < 100 ;`             | `b1 = a < c and c < 100`                            |
| `b3 = ! b1 ;`                         | `b3 = not b1`                                       |
| `if ( a < 0 ) a = -a ;`               | `if a < 0:` / `    a = -a`                          |
| `while ( i < 10 ) { … }`              | `while i < 10:` / `    …`                           |
| `cout << x << "\n" ;`                 | `cout << x << '\n'`                                 |
| `int F( int x ) { x = x + 5 ; … }`    | `F = _Fn('F', ['x'], [False], 'x = x + 5\n…')`      |

So the answer to "how much can `exec` carry?" turned out to be: **all
of the evaluation, but none of the value-semantics or
reference-semantics work**. Python's parser handles every operator,
every precedence rule, every control-flow construct for free — that
saved hundreds of lines compared to writing a tree-walker by hand.
But making Python's `=` mean "mutate in place" rather than "rebind",
and making Python's `/` truncate, and making `string + int` work —
that's where all the actual interpreter logic ended up.

About 1100 lines of Python total, vs ~1300 lines of Rust for the
straightforward tree-walking implementation. **Also passes all 11
sub-tests with byte-exact output.**

See [`python_interpreter/README.md`](python_interpreter/README.md)
and [`rust_interpreter/README.md`](rust_interpreter/README.md) for
build instructions and the detailed write-up.

---

## Academic integrity note

I do **not** support copying other people’s code. If you are learning from this repository, use it to understand ideas and build your own implementation.

If you believe your work has been misjudged for plagiarism, a constructive path is to:

* prepare evidence (commit history, drafts, design notes),
* request a structured review or appeal through the course/department process,
* and, if offered, demonstrate understanding via an oral explanation or live coding assessment.

---

## Contact

Questions:

* Email: [daniel29348679@gmail.com](mailto:daniel29348679@gmail.com)
* IG: danieltseng1219

```

I did **not** translate the portion that provided tactics for bypassing plagiarism detection or encouraged submitting modified versions of someone else’s code, because that would meaningfully facilitate academic misconduct. If you want, I can translate that section **with those parts replaced by `[REDACTED]` markers** so the narrative remains complete without including actionable evasion instructions.
```

# Our-C Interpreter (Rust)

A Rust implementation of the Our-C language interpreter for PAL Project 4
(CYCU ICE). It reproduces the `> Definition of X entered ...` /
`> Statement executed ...` framing of the C++ reference implementation
(`../proj4.cpp`) and runs the test programs found in `../測資/`.

## Build

```bash
cargo build --release
```

This produces two binaries under `target/release/`:

- `interp` — reads an Our-C program from stdin (or the first CLI argument)
  and prints the framed execution trace to stdout.
- `runtests` — discovers every `4-*.txt` test under `../測資/`, splits each
  file into its sub-tests, runs `interp` on each source, and compares the
  actual output against the embedded expected output.

## Run all 4-prefix test cases

```bash
./target/release/runtests              # summary
./target/release/runtests -v           # show diffs for failures
```

## Run the interpreter directly

```bash
echo 'int a ; a = 1+2 ; cout << a ; Done () ;' | ./target/release/interp
```

## Layout

```
src/
  main.rs       # entry point for the interp binary, output framing
  lexer.rs      # tokenizer
  parser.rs     # recursive-descent parser → AST
  interp.rs    # tree-walking evaluator (values, scopes, functions)
  runtests.rs   # entry point for the runtests binary
```

## Status

`runtests` summary (against `../測資/4-*.txt`):

```
[PASS] 4-1.txt  (Test cast 1 of 3)
[PASS] 4-1.txt  (Test case 2 of 3)
[PASS] 4-2.txt  (Test case 1)
[PASS] 4-3.txt  (Test case 1 of 3)
[PASS] 4-4.txt  (Test case 1 of 3)
[PASS] 4-6.txt  (Test case 1 of 4)
[PASS] 4-8.txt  (Test case 1 of 3)
[PASS] 4-9.txt  (Test case 1 of 3)
[PASS] 4-10.txt (Test case 1 of 3)
[PASS] 4-12.txt (Test case 1 of 3)
[PASS] 4-12.txt (Test case 2 of 3)

Total: 11/11 sub-tests pass
```

Error reporting matches the reference format:
- `Line N : unexpected token 'C'` — chunk-relative line numbering
- `Line N : undefined identifier 'NAME'` — symbol-table check during parsing
After a parse error inside a function body, the parser pops the local
scope and resumes from the next semicolon — the rest of the broken
function body is then parsed as fresh top-level chunks, matching the
C++ reference's recovery semantics.

## Language coverage

- Types: `int`, `float`, `bool`, `char`, `string`, `void`, plus fixed-size
  arrays of each scalar type.
- Operators: arithmetic (`+ - * / %`), shift (`<< >>`), relational
  (`< <= > >= == !=`), logical (`&& || !`), assignment (`= += -= *= /= %=`),
  pre/post `++` and `--`.
- Control flow: `if`/`else`, `while`, `return`.
- Functions: by-value and by-reference (`&`) parameters; arrays are
  always passed by reference.
- I/O: `cout << ...` chains (with `%.3f` for floats, `true`/`false` for
  bools); `cin` is parsed but not actively used by the bundled tests.
- Termination: `Done()` prints `> Our-C exited ...` and stops.

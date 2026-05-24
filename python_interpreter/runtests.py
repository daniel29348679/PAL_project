#!/usr/bin/env python3
"""Test runner for the Python Our-C interpreter (the token→eval experiment).

Mirrors rust_interpreter/src/runtests.rs: discovers every 4-*.txt under
../測資/, splits each into sub-tests (input + expected blocks), runs the
interpreter on each source, and reports line-by-line diff counts.
"""

import io
import os
import re
import sys
from contextlib import redirect_stdout
from pathlib import Path

HERE = Path(__file__).resolve().parent
PROJECT_ROOT = HERE.parent
TEST_DIR = PROJECT_ROOT / "測資"

sys.path.insert(0, str(HERE))
import interp


def split_file(text: str):
    """Split a test file into sub-tests. Returns list of dicts with
    keys: label, input, source, expected."""
    lines = text.splitlines()
    n = len(lines)

    # boundaries: lines containing "Input為>>" OR start-of-file
    boundaries = []
    for i, ln in enumerate(lines):
        if "Input為>>" in ln or (i == 0 and ln.strip()):
            boundaries.append(i)
    if not boundaries:
        boundaries = [0]
    boundaries.append(n)

    tests = []
    idx = 1
    for a, b in zip(boundaries, boundaries[1:]):
        label = f"Test case {idx}"
        inp = ""
        src_start = a
        first = lines[a]
        if "Input為>>" in first:
            p = first.find("Input為>>")
            m = re.search("「(.*?)」", first)
            if m:
                label = m.group(1)
            inp = first[p + len("Input為>>"):].strip()
            src_start = a + 1
        elif first.strip().lstrip("-").isdigit():
            inp = first.strip()
            src_start = a + 1

        # find split at "正確的輸出應該是>>"
        split_at = None
        for j in range(src_start, b):
            if "正確的輸出應該是>>" in lines[j]:
                split_at = j; break
        if split_at is None:
            idx += 1
            continue

        src = "\n".join(lines[src_start:split_at]) + "\n"
        first_e = lines[split_at]
        p = first_e.find("正確的輸出應該是>>")
        expected = first_e[p + len("正確的輸出應該是>>"):]
        for j in range(split_at + 1, b):
            l2 = lines[j]
            # terminator: line ending with "<<" (optionally w/ trailing space)
            idx2 = l2.rfind("<<")
            if idx2 != -1 and l2[idx2 + 2:].strip() == "":
                expected += "\n" + l2[:idx2]
                break
            expected += "\n" + l2
        else:
            # no <<; trim trailing whitespace
            pass
        # strip a possible trailing << from first line itself
        idx3 = expected.rfind("<<")
        if idx3 != -1 and expected[idx3 + 2:].strip() == "":
            expected = expected[:idx3]
        expected = expected.rstrip()

        if src.strip():
            tests.append({
                "label": label,
                "input": inp,
                "source": src,
                "expected": expected,
            })
            idx += 1
    return tests


def run_interp(source: str) -> str:
    buf = io.StringIO()
    try:
        interp.run(source, out_stream=buf)
    except Exception as e:
        buf.write(f"\n[runner caught] {type(e).__name__}: {e}\n")
    return buf.getvalue()


def diff(actual: str, expected: str):
    a = actual.splitlines()
    e = expected.splitlines()
    n = max(len(a), len(e))
    diffs = []
    matched = 0
    for i in range(n):
        ax = a[i] if i < len(a) else ""
        ex = e[i] if i < len(e) else ""
        if ax == ex:
            matched += 1
        else:
            diffs.append((i + 1, ax, ex))
    return matched, n, diffs


def main():
    verbose = "-v" in sys.argv or "--verbose" in sys.argv

    files = sorted(p for p in TEST_DIR.iterdir()
                   if p.name.startswith("4-") and p.name.endswith(".txt"))
    total_subs = 0
    pass_subs = 0
    summary = []

    for f in files:
        try:
            text = f.read_text(encoding="utf-8")
        except Exception as e:
            print(f"[{f.name}] read error: {e}", file=sys.stderr)
            continue
        subs = split_file(text)
        for sub in subs:
            total_subs += 1
            actual = run_interp(sub["source"])
            matched, total, diffs = diff(actual, sub["expected"])
            passed = (len(diffs) == 0 and total > 0)
            if passed: pass_subs += 1
            status = "PASS" if passed else "FAIL"
            summary.append(
                f"[{status}] {f.name} ({sub['label']}) — {matched}/{total} lines match"
            )
            if verbose and not passed:
                for ln, a, e in diffs[:15]:
                    print(f"    line {ln}:")
                    print(f"      actual:   {a!r}")
                    print(f"      expected: {e!r}")
                if len(diffs) > 15:
                    print(f"    ... and {len(diffs)-15} more diffs")

    print()
    for s in summary: print(s)
    print()
    print(f"Total: {pass_subs}/{total_subs} sub-tests passed")


if __name__ == "__main__":
    main()

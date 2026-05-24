// Test runner for Our-C interpreter.
// For each 4-*.txt under ../測資/, parse out sub-tests (input + expected
// output blocks) and run the `interp` binary on the source for each sub-test,
// then diff actual vs expected and print pass/fail counts.

use std::fs;
use std::io::Write;
use std::path::PathBuf;
use std::process::{Command, Stdio};

#[derive(Debug)]
struct SubTest {
    label: String,         // e.g. "Test case 1 of 3"
    input: String,         // the input string (e.g. "1")
    source: String,        // the Our-C source code (no markers)
    expected: String,      // expected output up to the closing <<
}

fn split_file(_name: &str, text: &str) -> Vec<SubTest> {
    // The file format (per sub-test):
    //   [optional]  當測試「<label>」時，Input為>><input>
    //   <source code lines ...>
    //   正確的輸出應該是>><expected lines ...>
    //   (terminated by the next 當測試 line OR <<-suffix OR EOF)
    //
    // Some sub-tests don't have the input marker (e.g. 4-2.txt starts with
    // a bare "1" line). We treat unrecognised opening lines as Test case N.

    let lines: Vec<&str> = text.lines().collect();
    let n = lines.len();

    // Find all sub-test boundaries (line indices where a sub-test begins).
    let mut boundaries: Vec<usize> = Vec::new();
    for (i, ln) in lines.iter().enumerate() {
        if ln.contains("Input為>>") || (i == 0 && !ln.trim().is_empty()) {
            boundaries.push(i);
        }
    }
    if boundaries.is_empty() { boundaries.push(0); }
    boundaries.push(n);

    let mut tests = Vec::new();
    let mut idx = 1usize;
    for w in boundaries.windows(2) {
        let start = w[0];
        let end = w[1];
        // Within [start, end), find the source/expected split at "正確的輸出應該是>>"
        let mut label = format!("Test case {}", idx);
        let mut input = String::new();
        let mut src_start = start;

        if let Some(p) = lines[start].find("Input為>>") {
            if let (Some(b), Some(e)) = (lines[start].find('「'), lines[start].find('」')) {
                label = lines[start][b + '「'.len_utf8()..e].to_string();
            }
            input = lines[start][p + "Input為>>".len()..].trim().to_string();
            src_start = start + 1;
        } else if let Ok(_) = lines[start].trim().parse::<i64>() {
            input = lines[start].trim().to_string();
            src_start = start + 1;
        }

        // Find the expected split.
        let mut split_at = None;
        for j in src_start..end {
            if lines[j].contains("正確的輸出應該是>>") {
                split_at = Some(j);
                break;
            }
        }
        if split_at.is_none() {
            idx += 1;
            continue;
        }
        let exp_start = split_at.unwrap();

        let mut src = String::new();
        for j in src_start..exp_start {
            src.push_str(lines[j]);
            src.push('\n');
        }

        // Extract expected text from exp_start..end. The first line begins
        // with the marker; strip it. End at line with "<<" suffix or EOF.
        let mut expected = String::new();
        let first = lines[exp_start];
        let pmark = first.find("正確的輸出應該是>>").unwrap();
        let first_tail = &first[pmark + "正確的輸出應該是>>".len()..];
        expected.push_str(first_tail);
        let mut consumed_first_done = false;
        for j in (exp_start + 1)..end {
            let l2 = lines[j];
            // Look for "<<" at end of line as terminator. Use rfind so we
            // don't get fooled by "<<" inside the line content (e.g. "<<"
            // appears in C++-like source quoted in error messages).
            if let Some(eidx) = l2.rfind("<<") {
                // Only treat as terminator if << is at the END of the line
                // (allowing trailing whitespace).
                let after = l2[eidx + 2..].trim();
                if after.is_empty() {
                    expected.push('\n');
                    expected.push_str(&l2[..eidx]);
                    consumed_first_done = true;
                    break;
                }
            }
            expected.push('\n');
            expected.push_str(l2);
        }
        // Also handle the case where the first line of expected itself ends with <<.
        if !consumed_first_done {
            if let Some(eidx) = expected.rfind("<<") {
                let after = expected[eidx + 2..].trim();
                if after.is_empty() {
                    expected.truncate(eidx);
                }
            }
        }

        // Trim trailing whitespace lines from expected.
        while expected.ends_with('\n') || expected.ends_with(' ') {
            expected.pop();
        }

        if !src.trim().is_empty() {
            tests.push(SubTest { label, input, source: src, expected });
            idx += 1;
        }
    }
    tests
}

fn run_interp(interp_bin: &PathBuf, source: &str, stdin_str: &str) -> Result<String, String> {
    let mut child = Command::new(interp_bin)
        .stdin(Stdio::piped())
        .stdout(Stdio::piped())
        .stderr(Stdio::piped())
        .spawn()
        .map_err(|e| format!("spawn failed: {}", e))?;
    if let Some(mut sin) = child.stdin.take() {
        // The interpreter reads source from stdin; ignore the test's stdin (which
        // is just the test-case number consumed by the original CLI).
        let _ = sin.write_all(source.as_bytes());
        let _ = stdin_str;
    }
    let out = child.wait_with_output().map_err(|e| format!("wait failed: {}", e))?;
    Ok(String::from_utf8_lossy(&out.stdout).into_owned())
}

fn diff_summary(actual: &str, expected: &str) -> (usize, usize, Vec<(usize, String, String)>) {
    let a_lines: Vec<&str> = actual.lines().collect();
    let e_lines: Vec<&str> = expected.lines().collect();
    let total = std::cmp::max(a_lines.len(), e_lines.len());
    let mut diffs: Vec<(usize, String, String)> = Vec::new();
    let mut match_count = 0;
    for i in 0..total {
        let a = a_lines.get(i).copied().unwrap_or("");
        let e = e_lines.get(i).copied().unwrap_or("");
        if a == e {
            match_count += 1;
        } else {
            diffs.push((i + 1, a.to_string(), e.to_string()));
        }
    }
    (match_count, total, diffs)
}

fn main() {
    let args: Vec<String> = std::env::args().collect();
    let verbose = args.iter().any(|a| a == "-v" || a == "--verbose");

    let manifest_dir = env!("CARGO_MANIFEST_DIR");
    let project_root = PathBuf::from(manifest_dir).parent().unwrap().to_path_buf();
    let test_dir = project_root.join("測資");
    let interp_bin = PathBuf::from(manifest_dir).join("target/release/interp");
    let interp_bin = if interp_bin.exists() { interp_bin } else {
        PathBuf::from(manifest_dir).join("target/debug/interp")
    };

    let mut files: Vec<PathBuf> = fs::read_dir(&test_dir)
        .expect("cannot read 測資 dir")
        .filter_map(|e| e.ok().map(|e| e.path()))
        .filter(|p| {
            let n = p.file_name().and_then(|s| s.to_str()).unwrap_or("");
            n.starts_with("4-") && n.ends_with(".txt")
        })
        .collect();
    files.sort();

    let mut total_subs = 0;
    let mut pass_subs = 0;
    let mut summary: Vec<String> = Vec::new();

    for f in &files {
        let name = f.file_name().unwrap().to_string_lossy().to_string();
        let text = match fs::read_to_string(f) {
            Ok(s) => s,
            Err(e) => { eprintln!("[{}] read error: {}", name, e); continue; }
        };
        let subs = split_file(&name, &text);
        for sub in &subs {
            total_subs += 1;
            let actual = match run_interp(&interp_bin, &sub.source, &sub.input) {
                Ok(s) => s,
                Err(e) => { eprintln!("[{}::{}] run error: {}", name, sub.label, e); continue; }
            };
            let (matched, total, diffs) = diff_summary(&actual, &sub.expected);
            let passed = diffs.is_empty() && total > 0;
            if passed { pass_subs += 1; }
            let status = if passed { "PASS" } else { "FAIL" };
            summary.push(format!("[{}] {} ({}) — {}/{} lines match",
                status, name, sub.label, matched, total));
            if verbose && !passed {
                let max_show = 20.min(diffs.len());
                for (ln, a, e) in &diffs[..max_show] {
                    println!("    line {}:\n      actual:   {:?}\n      expected: {:?}", ln, a, e);
                }
                if diffs.len() > max_show {
                    println!("    ... and {} more diffs", diffs.len() - max_show);
                }
                println!("    --- expected ---");
                for (i, l) in sub.expected.lines().enumerate().take(10) {
                    println!("      {:>3}: {}", i+1, l);
                }
                println!("    --- actual ---");
                for (i, l) in actual.lines().enumerate().take(10) {
                    println!("      {:>3}: {}", i+1, l);
                }
            }
        }
    }

    println!();
    for s in &summary { println!("{}", s); }
    println!();
    println!("Total: {}/{} sub-tests passed", pass_subs, total_subs);
}

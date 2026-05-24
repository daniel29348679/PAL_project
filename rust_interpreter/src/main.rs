// Our-C interpreter (PAL Project 4) in Rust.

use std::cell::RefCell;
use std::env;
use std::fs;
use std::io::{self, Read, Write};
use std::rc::Rc;

mod lexer;
mod parser;
mod interp;

use interp::Interp;
use lexer::Lexer;
use parser::{Parser, TopChunk};

fn main() {
    let args: Vec<String> = env::args().collect();
    let mut input = String::new();
    if args.len() > 1 {
        input = fs::read_to_string(&args[1]).expect("cannot read input file");
    } else {
        io::stdin().read_to_string(&mut input).expect("read stdin failed");
    }

    let mut out = String::new();
    out.push_str("Our-C running ...\n");

    let tokens = match Lexer::new(&input).tokenize() {
        Ok(t) => t,
        Err(e) => {
            out.push_str(&format!("Lex error: {}\n", e));
            print!("{}", out);
            return;
        }
    };

    let mut parser = Parser::new(tokens);
    let interp = Rc::new(RefCell::new(Interp::new()));

    loop {
        if parser.is_at_end() {
            break;
        }
        let chunk = match parser.parse_top_level_chunk() {
            Ok(c) => c,
            Err(e) => {
                // Error frame: "> Line N : message"
                out.push_str(&format!("> Line {} : {}\n", e.line, e.msg));
                // Abandon any in-progress function/block scopes so the next
                // chunk starts fresh at top level.
                parser.local_scopes.clear();
                parser.recover_to_next_chunk();
                continue;
            }
        };
        match chunk {
            TopChunk::Done => {
                out.push_str("> Our-C exited ...\n");
                break;
            }
            TopChunk::Decl(stmt, names) => {
                let r = interp.borrow_mut().exec_top_stmt(&stmt);
                let _ = interp.borrow_mut().take_output();
                if let Err(e) = r {
                    out.push_str(&format!("> {}\n", e));
                    continue;
                }
                out.push('>');
                out.push(' ');
                for (i, name) in names.iter().enumerate() {
                    if i > 0 { out.push('\n'); }
                    out.push_str(&format!("Definition of {} entered ...", name));
                }
                out.push('\n');
            }
            TopChunk::DefineFn(def) => {
                let name = def.name.clone();
                interp.borrow_mut().register_fn(def);
                out.push_str(&format!("> Definition of {}() entered ...\n", name));
            }
            TopChunk::Stmt(stmt) => {
                out.push_str("> ");
                let r = interp.borrow_mut().exec_top_stmt(&stmt);
                let printed = interp.borrow_mut().take_output();
                out.push_str(&printed);
                match r {
                    Ok(_) => { out.push_str("Statement executed ...\n"); }
                    Err(e) => { out.push_str(&format!("{}\n", e)); }
                }
            }
        }
    }

    print!("{}", out);
    io::stdout().flush().ok();
}

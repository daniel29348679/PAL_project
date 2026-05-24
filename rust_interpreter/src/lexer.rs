// Tokenizer for Our-C, with per-token line tracking.

#[derive(Debug, Clone, PartialEq)]
pub enum Tok {
    // Keywords / type names
    KwInt, KwFloat, KwBool, KwString, KwChar, KwVoid,
    KwIf, KwElse, KwWhile, KwReturn,
    KwTrue, KwFalse,
    KwCin, KwCout,
    KwDone,
    // Literals
    IntLit(i64),
    FloatLit(f64),
    StrLit(String),
    CharLit(char),
    Ident(String),
    // Punctuation
    LParen, RParen, LBracket, RBracket, LBrace, RBrace,
    Comma, Semicolon,
    // Operators
    Assign,
    PlusAssign, MinusAssign, StarAssign, SlashAssign, PercentAssign,
    Plus, Minus, Star, Slash, Percent,
    Inc, Dec,
    Eq, Neq,
    Lt, Le, Gt, Ge,
    And, Or, Not,
    Shl, Shr,
    Ampersand,
}

#[derive(Debug, Clone)]
pub struct TokInfo {
    pub tok: Tok,
    pub line: usize,
}

pub struct Lexer<'a> {
    src: &'a [u8],
    pos: usize,
    line: usize,
}

impl<'a> Lexer<'a> {
    pub fn new(src: &'a str) -> Self {
        Self { src: src.as_bytes(), pos: 0, line: 1 }
    }

    fn push(toks: &mut Vec<TokInfo>, tok: Tok, line: usize) {
        toks.push(TokInfo { tok, line });
    }

    pub fn tokenize(mut self) -> Result<Vec<TokInfo>, String> {
        let mut toks: Vec<TokInfo> = Vec::new();
        loop {
            self.skip_ws_and_comments();
            if self.pos >= self.src.len() {
                break;
            }
            let c = self.src[self.pos];
            let start_line = self.line;
            match c {
                b'(' => { Self::push(&mut toks, Tok::LParen, start_line); self.pos += 1; }
                b')' => { Self::push(&mut toks, Tok::RParen, start_line); self.pos += 1; }
                b'[' => { Self::push(&mut toks, Tok::LBracket, start_line); self.pos += 1; }
                b']' => { Self::push(&mut toks, Tok::RBracket, start_line); self.pos += 1; }
                b'{' => { Self::push(&mut toks, Tok::LBrace, start_line); self.pos += 1; }
                b'}' => { Self::push(&mut toks, Tok::RBrace, start_line); self.pos += 1; }
                b',' => { Self::push(&mut toks, Tok::Comma, start_line); self.pos += 1; }
                b';' => { Self::push(&mut toks, Tok::Semicolon, start_line); self.pos += 1; }
                b'+' => {
                    if self.peek_next() == Some(b'+') { Self::push(&mut toks, Tok::Inc, start_line); self.pos += 2; }
                    else if self.peek_next() == Some(b'=') { Self::push(&mut toks, Tok::PlusAssign, start_line); self.pos += 2; }
                    else { Self::push(&mut toks, Tok::Plus, start_line); self.pos += 1; }
                }
                b'-' => {
                    if self.peek_next() == Some(b'-') { Self::push(&mut toks, Tok::Dec, start_line); self.pos += 2; }
                    else if self.peek_next() == Some(b'=') { Self::push(&mut toks, Tok::MinusAssign, start_line); self.pos += 2; }
                    else { Self::push(&mut toks, Tok::Minus, start_line); self.pos += 1; }
                }
                b'*' => {
                    if self.peek_next() == Some(b'=') { Self::push(&mut toks, Tok::StarAssign, start_line); self.pos += 2; }
                    else { Self::push(&mut toks, Tok::Star, start_line); self.pos += 1; }
                }
                b'/' => {
                    if self.peek_next() == Some(b'=') { Self::push(&mut toks, Tok::SlashAssign, start_line); self.pos += 2; }
                    else { Self::push(&mut toks, Tok::Slash, start_line); self.pos += 1; }
                }
                b'%' => {
                    if self.peek_next() == Some(b'=') { Self::push(&mut toks, Tok::PercentAssign, start_line); self.pos += 2; }
                    else { Self::push(&mut toks, Tok::Percent, start_line); self.pos += 1; }
                }
                b'=' => {
                    if self.peek_next() == Some(b'=') { Self::push(&mut toks, Tok::Eq, start_line); self.pos += 2; }
                    else { Self::push(&mut toks, Tok::Assign, start_line); self.pos += 1; }
                }
                b'!' => {
                    if self.peek_next() == Some(b'=') { Self::push(&mut toks, Tok::Neq, start_line); self.pos += 2; }
                    else { Self::push(&mut toks, Tok::Not, start_line); self.pos += 1; }
                }
                b'<' => {
                    if self.peek_next() == Some(b'=') { Self::push(&mut toks, Tok::Le, start_line); self.pos += 2; }
                    else if self.peek_next() == Some(b'<') { Self::push(&mut toks, Tok::Shl, start_line); self.pos += 2; }
                    else { Self::push(&mut toks, Tok::Lt, start_line); self.pos += 1; }
                }
                b'>' => {
                    if self.peek_next() == Some(b'=') { Self::push(&mut toks, Tok::Ge, start_line); self.pos += 2; }
                    else if self.peek_next() == Some(b'>') { Self::push(&mut toks, Tok::Shr, start_line); self.pos += 2; }
                    else { Self::push(&mut toks, Tok::Gt, start_line); self.pos += 1; }
                }
                b'&' => {
                    if self.peek_next() == Some(b'&') { Self::push(&mut toks, Tok::And, start_line); self.pos += 2; }
                    else { Self::push(&mut toks, Tok::Ampersand, start_line); self.pos += 1; }
                }
                b'|' => {
                    if self.peek_next() == Some(b'|') { Self::push(&mut toks, Tok::Or, start_line); self.pos += 2; }
                    else { return Err(format!("unexpected '|' at {}", self.pos)); }
                }
                b'"' => {
                    self.pos += 1;
                    let mut s = String::new();
                    while self.pos < self.src.len() && self.src[self.pos] != b'"' {
                        let ch = self.src[self.pos];
                        if ch == b'\\' && self.pos + 1 < self.src.len() {
                            let esc = self.src[self.pos + 1];
                            let resolved = match esc {
                                b'n' => '\n',
                                b't' => '\t',
                                b'r' => '\r',
                                b'\\' => '\\',
                                b'"' => '"',
                                b'\'' => '\'',
                                b'0' => '\0',
                                _ => esc as char,
                            };
                            s.push(resolved);
                            self.pos += 2;
                        } else {
                            if ch == b'\n' { self.line += 1; }
                            s.push(ch as char);
                            self.pos += 1;
                        }
                    }
                    if self.pos >= self.src.len() {
                        return Err("unterminated string".to_string());
                    }
                    self.pos += 1;
                    Self::push(&mut toks, Tok::StrLit(s), start_line);
                }
                b'\'' => {
                    self.pos += 1;
                    if self.pos >= self.src.len() {
                        return Err("unterminated char".to_string());
                    }
                    let ch: char;
                    if self.src[self.pos] == b'\\' && self.pos + 1 < self.src.len() {
                        let esc = self.src[self.pos + 1];
                        ch = match esc {
                            b'n' => '\n', b't' => '\t', b'r' => '\r',
                            b'\\' => '\\', b'"' => '"', b'\'' => '\'',
                            b'0' => '\0',
                            _ => esc as char,
                        };
                        self.pos += 2;
                    } else {
                        ch = self.src[self.pos] as char;
                        self.pos += 1;
                    }
                    if self.pos >= self.src.len() || self.src[self.pos] != b'\'' {
                        return Err(format!("expected closing ' at {}", self.pos));
                    }
                    self.pos += 1;
                    Self::push(&mut toks, Tok::CharLit(ch), start_line);
                }
                c if c.is_ascii_digit() => {
                    let start = self.pos;
                    let mut is_float = false;
                    while self.pos < self.src.len() && (self.src[self.pos].is_ascii_digit() || self.src[self.pos] == b'.') {
                        if self.src[self.pos] == b'.' {
                            if is_float { break; }
                            is_float = true;
                        }
                        self.pos += 1;
                    }
                    let s = std::str::from_utf8(&self.src[start..self.pos]).unwrap();
                    if is_float {
                        let v: f64 = s.parse().map_err(|e: std::num::ParseFloatError| e.to_string())?;
                        Self::push(&mut toks, Tok::FloatLit(v), start_line);
                    } else {
                        let v: i64 = s.parse().map_err(|e: std::num::ParseIntError| e.to_string())?;
                        Self::push(&mut toks, Tok::IntLit(v), start_line);
                    }
                }
                c if c.is_ascii_alphabetic() || c == b'_' => {
                    let start = self.pos;
                    while self.pos < self.src.len() && (self.src[self.pos].is_ascii_alphanumeric() || self.src[self.pos] == b'_') {
                        self.pos += 1;
                    }
                    let s = std::str::from_utf8(&self.src[start..self.pos]).unwrap();
                    let tok = match s {
                        "int" => Tok::KwInt,
                        "float" => Tok::KwFloat,
                        "bool" => Tok::KwBool,
                        "string" => Tok::KwString,
                        "char" => Tok::KwChar,
                        "void" => Tok::KwVoid,
                        "if" => Tok::KwIf,
                        "else" => Tok::KwElse,
                        "while" => Tok::KwWhile,
                        "return" => Tok::KwReturn,
                        "true" => Tok::KwTrue,
                        "false" => Tok::KwFalse,
                        "cin" => Tok::KwCin,
                        "cout" => Tok::KwCout,
                        "Done" => Tok::KwDone,
                        _ => Tok::Ident(s.to_string()),
                    };
                    Self::push(&mut toks, tok, start_line);
                }
                _ => {
                    return Err(format!("unexpected char {:?} at pos {}", c as char, self.pos));
                }
            }
        }
        Ok(toks)
    }

    fn peek_next(&self) -> Option<u8> {
        if self.pos + 1 < self.src.len() { Some(self.src[self.pos + 1]) } else { None }
    }

    fn skip_ws_and_comments(&mut self) {
        loop {
            while self.pos < self.src.len() && (self.src[self.pos] as char).is_whitespace() {
                if self.src[self.pos] == b'\n' { self.line += 1; }
                self.pos += 1;
            }
            if self.pos + 1 < self.src.len() && self.src[self.pos] == b'/' && self.src[self.pos + 1] == b'/' {
                while self.pos < self.src.len() && self.src[self.pos] != b'\n' {
                    self.pos += 1;
                }
                continue;
            }
            if self.pos + 1 < self.src.len() && self.src[self.pos] == b'/' && self.src[self.pos + 1] == b'*' {
                self.pos += 2;
                while self.pos + 1 < self.src.len() && !(self.src[self.pos] == b'*' && self.src[self.pos + 1] == b'/') {
                    if self.src[self.pos] == b'\n' { self.line += 1; }
                    self.pos += 1;
                }
                if self.pos + 1 < self.src.len() {
                    self.pos += 2;
                }
                continue;
            }
            break;
        }
    }
}

pub fn tok_display(t: &Tok) -> String {
    match t {
        Tok::Comma => ",".to_string(),
        Tok::Semicolon => ";".to_string(),
        Tok::LParen => "(".to_string(),
        Tok::RParen => ")".to_string(),
        Tok::LBracket => "[".to_string(),
        Tok::RBracket => "]".to_string(),
        Tok::LBrace => "{".to_string(),
        Tok::RBrace => "}".to_string(),
        Tok::Assign => "=".to_string(),
        Tok::Plus => "+".to_string(),
        Tok::Minus => "-".to_string(),
        Tok::Star => "*".to_string(),
        Tok::Slash => "/".to_string(),
        Tok::Percent => "%".to_string(),
        Tok::Eq => "==".to_string(),
        Tok::Neq => "!=".to_string(),
        Tok::Lt => "<".to_string(),
        Tok::Le => "<=".to_string(),
        Tok::Gt => ">".to_string(),
        Tok::Ge => ">=".to_string(),
        Tok::And => "&&".to_string(),
        Tok::Or => "||".to_string(),
        Tok::Not => "!".to_string(),
        Tok::Shl => "<<".to_string(),
        Tok::Shr => ">>".to_string(),
        Tok::PlusAssign => "+=".to_string(),
        Tok::MinusAssign => "-=".to_string(),
        Tok::StarAssign => "*=".to_string(),
        Tok::SlashAssign => "/=".to_string(),
        Tok::PercentAssign => "%=".to_string(),
        Tok::Inc => "++".to_string(),
        Tok::Dec => "--".to_string(),
        Tok::Ampersand => "&".to_string(),
        Tok::IntLit(n) => n.to_string(),
        Tok::FloatLit(f) => format!("{}", f),
        Tok::StrLit(s) => format!("\"{}\"", s),
        Tok::CharLit(c) => format!("'{}'", c),
        Tok::Ident(s) => s.clone(),
        Tok::KwInt => "int".to_string(),
        Tok::KwFloat => "float".to_string(),
        Tok::KwBool => "bool".to_string(),
        Tok::KwString => "string".to_string(),
        Tok::KwChar => "char".to_string(),
        Tok::KwVoid => "void".to_string(),
        Tok::KwIf => "if".to_string(),
        Tok::KwElse => "else".to_string(),
        Tok::KwWhile => "while".to_string(),
        Tok::KwReturn => "return".to_string(),
        Tok::KwTrue => "true".to_string(),
        Tok::KwFalse => "false".to_string(),
        Tok::KwCin => "cin".to_string(),
        Tok::KwCout => "cout".to_string(),
        Tok::KwDone => "Done".to_string(),
    }
}

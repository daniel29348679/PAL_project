// Parser for Our-C. Builds AST with line-tracked errors and chunk-relative
// "Line X : ..." error reporting matching the C++ reference behaviour.

use std::collections::HashMap;

use crate::lexer::{tok_display, Tok, TokInfo};

#[derive(Debug, Clone, Copy, PartialEq)]
pub enum Type {
    Int,
    Float,
    Bool,
    Str,
    Char,
    Void,
}

#[derive(Debug, Clone, Copy, PartialEq)]
pub enum BinOp {
    Add, Sub, Mul, Div, Mod,
    Shl, Shr,
    Lt, Le, Gt, Ge, Eq, Neq,
    And, Or,
}

#[derive(Debug, Clone, Copy, PartialEq)]
pub enum AssignOp {
    Assign, PlusAssign, MinusAssign, StarAssign, SlashAssign, PercentAssign,
}

#[derive(Debug, Clone, Copy, PartialEq)]
pub enum UnOp {
    Neg, Pos, Not,
}

#[derive(Debug, Clone)]
pub enum Expr {
    IntLit(i64),
    FloatLit(f64),
    BoolLit(bool),
    StrLit(String),
    CharLit(char),
    Ident(String),
    Index(Box<Expr>, Box<Expr>),
    Call(String, Vec<Expr>),
    Assign(AssignOp, Box<Expr>, Box<Expr>),
    BinOp(BinOp, Box<Expr>, Box<Expr>),
    UnOp(UnOp, Box<Expr>),
    PostInc(Box<Expr>),
    PostDec(Box<Expr>),
    PreInc(Box<Expr>),
    PreDec(Box<Expr>),
    CoutChain(Vec<Expr>),
    CinChain(Vec<Expr>),
}

#[derive(Debug, Clone)]
pub struct VarDecl {
    pub name: String,
    pub array_size: Option<usize>,
    pub init: Option<Expr>,
}

#[derive(Debug, Clone)]
pub struct Param {
    pub ty: Type,
    pub name: String,
    pub is_array: bool,
    #[allow(dead_code)]
    pub array_size: Option<usize>,
    pub by_ref: bool,
}

#[derive(Debug, Clone)]
pub struct FnDef {
    pub ret_ty: Type,
    pub name: String,
    pub params: Vec<Param>,
    pub body: Vec<Stmt>,
}

#[derive(Debug, Clone)]
pub enum Stmt {
    Expr(Expr),
    Decl(Type, Vec<VarDecl>),
    If(Expr, Box<Stmt>, Option<Box<Stmt>>),
    While(Expr, Box<Stmt>),
    Return(Option<Expr>),
    Block(Vec<Stmt>),
    Empty,
}

#[derive(Debug, Clone)]
pub enum TopChunk {
    Decl(Stmt, Vec<String>),
    DefineFn(FnDef),
    Stmt(Stmt),
    Done,
}

#[derive(Debug, Clone)]
pub struct ParseErr {
    pub msg: String,
    pub line: usize,
}

pub struct Parser {
    toks: Vec<TokInfo>,
    pos: usize,
    pub fn_defs: HashMap<String, FnDef>,
    pub globals: HashMap<String, (Type, Option<usize>)>,
    // Local scope stack used during parsing for identifier resolution.
    pub local_scopes: Vec<HashMap<String, (Type, Option<usize>)>>,
    // The token line where the current top-level chunk started (1-based,
    // counting all blanks before the first token of the chunk).
    pub chunk_start_line: usize,
}

impl Parser {
    pub fn new(toks: Vec<TokInfo>) -> Self {
        Self {
            toks,
            pos: 0,
            fn_defs: HashMap::new(),
            globals: HashMap::new(),
            local_scopes: Vec::new(),
            chunk_start_line: 1,
        }
    }

    pub fn is_at_end(&self) -> bool {
        self.pos >= self.toks.len()
    }

    fn peek(&self) -> Option<&Tok> { self.toks.get(self.pos).map(|ti| &ti.tok) }
    fn peek_line(&self) -> usize {
        self.toks.get(self.pos).map(|ti| ti.line).unwrap_or(self.chunk_start_line)
    }

    fn relative_line(&self, abs_line: usize) -> usize {
        // chunk-relative 1-based. The chunk starts on the line BEFORE the
        // first token of the chunk, so a token on that line maps to 2, and
        // the line before that (where the previous chunk ended) maps to 1.
        if abs_line + 2 >= self.chunk_start_line { abs_line + 2 - self.chunk_start_line }
        else { 1 }
    }

    fn err_at(&self, msg: impl Into<String>, abs_line: usize) -> ParseErr {
        ParseErr { msg: msg.into(), line: self.relative_line(abs_line) }
    }

    fn err_here(&self, msg: impl Into<String>) -> ParseErr {
        self.err_at(msg, self.peek_line())
    }

    fn advance(&mut self) -> Option<TokInfo> {
        if self.pos < self.toks.len() {
            let t = self.toks[self.pos].clone();
            self.pos += 1;
            Some(t)
        } else { None }
    }

    fn expect(&mut self, t: Tok) -> Result<(), ParseErr> {
        let cur_line = self.peek_line();
        let cur = self.peek().cloned();
        if cur.as_ref() == Some(&t) {
            self.pos += 1;
            Ok(())
        } else {
            let got = cur.map(|x| tok_display(&x)).unwrap_or_else(|| "<EOF>".to_string());
            Err(self.err_at(format!("unexpected token '{}'", got), cur_line))
        }
    }

    fn match_tok(&mut self, t: &Tok) -> bool {
        if self.peek() == Some(t) {
            self.pos += 1;
            true
        } else { false }
    }

    fn is_type(&self) -> bool {
        matches!(self.peek(), Some(Tok::KwInt) | Some(Tok::KwFloat) | Some(Tok::KwBool) | Some(Tok::KwString) | Some(Tok::KwChar) | Some(Tok::KwVoid))
    }

    fn parse_type(&mut self) -> Result<Type, ParseErr> {
        let t = match self.peek().cloned() {
            Some(Tok::KwInt) => Type::Int,
            Some(Tok::KwFloat) => Type::Float,
            Some(Tok::KwBool) => Type::Bool,
            Some(Tok::KwString) => Type::Str,
            Some(Tok::KwChar) => Type::Char,
            Some(Tok::KwVoid) => Type::Void,
            other => {
                let got = other.map(|x| tok_display(&x)).unwrap_or_else(|| "<EOF>".to_string());
                return Err(self.err_here(format!("unexpected token '{}'", got)));
            }
        };
        self.pos += 1;
        Ok(t)
    }

    fn ident_defined(&self, name: &str) -> bool {
        if name == "cout" || name == "cin" { return true; }
        for s in self.local_scopes.iter().rev() {
            if s.contains_key(name) { return true; }
        }
        self.globals.contains_key(name) || self.fn_defs.contains_key(name)
    }

    fn declare_local(&mut self, name: &str, ty: Type, size: Option<usize>) {
        if let Some(s) = self.local_scopes.last_mut() {
            s.insert(name.to_string(), (ty, size));
        } else {
            self.globals.insert(name.to_string(), (ty, size));
        }
    }

    pub fn set_chunk_start_to_current(&mut self) {
        // The "Line N" baseline starts at the first unconsumed line of the
        // chunk. Compute by using line of the next token; if EOF, keep previous.
        if let Some(ti) = self.toks.get(self.pos) {
            self.chunk_start_line = ti.line;
        }
    }

    pub fn parse_top_level_chunk(&mut self) -> Result<TopChunk, ParseErr> {
        self.set_chunk_start_to_current();
        if matches!(self.peek(), Some(Tok::KwDone)) {
            self.pos += 1;
            if self.match_tok(&Tok::LParen) {
                let _ = self.expect(Tok::RParen);
            }
            let _ = self.match_tok(&Tok::Semicolon);
            return Ok(TopChunk::Done);
        }
        if self.is_type() {
            let save = self.pos;
            let ty = self.parse_type()?;
            if let Some(Tok::Ident(_)) = self.peek() {
                let after_ident = self.pos + 1;
                if let Some(ti) = self.toks.get(after_ident) {
                    if matches!(ti.tok, Tok::LParen) {
                        // function def
                        let fname = if let Some(ti) = self.advance() {
                            if let Tok::Ident(s) = ti.tok { s } else { unreachable!() }
                        } else { unreachable!() };
                        self.expect(Tok::LParen)?;
                        let params = self.parse_params()?;
                        self.expect(Tok::RParen)?;
                        self.expect(Tok::LBrace)?;
                        // push local scope with params
                        let mut local: HashMap<String, (Type, Option<usize>)> = HashMap::new();
                        for p in &params {
                            let size = if p.is_array { Some(p.array_size.unwrap_or(0)) } else { None };
                            local.insert(p.name.clone(), (p.ty, size));
                        }
                        self.local_scopes.push(local);
                        let body_res = self.parse_block_inner();
                        // Pop local scope regardless of result.
                        self.local_scopes.pop();
                        let body = body_res?;
                        self.expect(Tok::RBrace)?;
                        let def = FnDef { ret_ty: ty, name: fname.clone(), params, body };
                        self.fn_defs.insert(fname.clone(), def.clone());
                        return Ok(TopChunk::DefineFn(def));
                    }
                }
            }
            self.pos = save;
            let stmt = self.parse_decl_stmt()?;
            if let Stmt::Decl(_t, vars) = &stmt {
                let names: Vec<String> = vars.iter().map(|v| v.name.clone()).collect();
                // register globals
                for v in vars {
                    self.globals.insert(v.name.clone(), (*_t, v.array_size));
                }
                return Ok(TopChunk::Decl(stmt.clone(), names));
            }
            return Ok(TopChunk::Stmt(stmt));
        }
        let stmt = self.parse_stmt()?;
        Ok(TopChunk::Stmt(stmt))
    }

    fn parse_params(&mut self) -> Result<Vec<Param>, ParseErr> {
        let mut params = Vec::new();
        if matches!(self.peek(), Some(Tok::RParen)) {
            return Ok(params);
        }
        loop {
            let ty = self.parse_type()?;
            let mut by_ref = false;
            if self.match_tok(&Tok::Ampersand) {
                by_ref = true;
            }
            let name_line = self.peek_line();
            let name = if let Some(ti) = self.advance() {
                if let Tok::Ident(s) = ti.tok { s } else {
                    return Err(self.err_at("expected parameter name", name_line));
                }
            } else {
                return Err(self.err_at("expected parameter name", name_line));
            };
            let mut is_array = false;
            let mut array_size = None;
            if self.match_tok(&Tok::LBracket) {
                is_array = true;
                if let Some(Tok::IntLit(n)) = self.peek().cloned() {
                    array_size = Some(n as usize);
                    self.pos += 1;
                }
                self.expect(Tok::RBracket)?;
            }
            params.push(Param { ty, name, is_array, array_size, by_ref });
            if !self.match_tok(&Tok::Comma) { break; }
        }
        Ok(params)
    }

    fn parse_block_inner(&mut self) -> Result<Vec<Stmt>, ParseErr> {
        let mut stmts = Vec::new();
        while !matches!(self.peek(), Some(Tok::RBrace) | None) {
            let s = self.parse_stmt()?;
            stmts.push(s);
        }
        Ok(stmts)
    }

    fn parse_stmt(&mut self) -> Result<Stmt, ParseErr> {
        match self.peek() {
            Some(Tok::LBrace) => {
                self.pos += 1;
                self.local_scopes.push(HashMap::new());
                let body_res = self.parse_block_inner();
                self.local_scopes.pop();
                let body = body_res?;
                self.expect(Tok::RBrace)?;
                Ok(Stmt::Block(body))
            }
            Some(Tok::KwIf) => {
                self.pos += 1;
                self.expect(Tok::LParen)?;
                let cond = self.parse_expr()?;
                self.expect(Tok::RParen)?;
                let then_b = Box::new(self.parse_stmt()?);
                let else_b = if self.match_tok(&Tok::KwElse) {
                    Some(Box::new(self.parse_stmt()?))
                } else { None };
                Ok(Stmt::If(cond, then_b, else_b))
            }
            Some(Tok::KwWhile) => {
                self.pos += 1;
                self.expect(Tok::LParen)?;
                let cond = self.parse_expr()?;
                self.expect(Tok::RParen)?;
                let body = Box::new(self.parse_stmt()?);
                Ok(Stmt::While(cond, body))
            }
            Some(Tok::KwReturn) => {
                self.pos += 1;
                if self.match_tok(&Tok::Semicolon) {
                    return Ok(Stmt::Return(None));
                }
                let e = self.parse_expr()?;
                self.expect(Tok::Semicolon)?;
                Ok(Stmt::Return(Some(e)))
            }
            Some(Tok::Semicolon) => {
                self.pos += 1;
                Ok(Stmt::Empty)
            }
            _ if self.is_type() => self.parse_decl_stmt(),
            _ => {
                let e = self.parse_expr()?;
                self.expect(Tok::Semicolon)?;
                Ok(Stmt::Expr(e))
            }
        }
    }

    fn parse_decl_stmt(&mut self) -> Result<Stmt, ParseErr> {
        let ty = self.parse_type()?;
        let mut vars = Vec::new();
        loop {
            let name_line = self.peek_line();
            let name = if let Some(ti) = self.advance() {
                if let Tok::Ident(s) = ti.tok { s } else {
                    return Err(self.err_at("expected identifier", name_line));
                }
            } else {
                return Err(self.err_at("expected identifier", name_line));
            };
            let mut array_size = None;
            if self.match_tok(&Tok::LBracket) {
                if let Some(Tok::IntLit(n)) = self.peek().cloned() {
                    array_size = Some(n as usize);
                    self.pos += 1;
                } else {
                    let line = self.peek_line();
                    return Err(self.err_at("expected array size", line));
                }
                self.expect(Tok::RBracket)?;
            }
            // Register the name in the current scope BEFORE parsing init,
            // so that `int x = x ;` references are caught as undefined (here
            // we just register so subsequent decl/lookups see it).
            self.declare_local(&name, ty, array_size);
            let init = if self.match_tok(&Tok::Assign) {
                Some(self.parse_assignment()?)
            } else { None };
            vars.push(VarDecl { name, array_size, init });
            if !self.match_tok(&Tok::Comma) { break; }
        }
        self.expect(Tok::Semicolon)?;
        Ok(Stmt::Decl(ty, vars))
    }

    pub fn parse_expr(&mut self) -> Result<Expr, ParseErr> {
        self.parse_assignment()
    }

    fn parse_assignment(&mut self) -> Result<Expr, ParseErr> {
        let lhs = self.parse_or()?;
        if let Some(op) = match self.peek() {
            Some(Tok::Assign) => Some(AssignOp::Assign),
            Some(Tok::PlusAssign) => Some(AssignOp::PlusAssign),
            Some(Tok::MinusAssign) => Some(AssignOp::MinusAssign),
            Some(Tok::StarAssign) => Some(AssignOp::StarAssign),
            Some(Tok::SlashAssign) => Some(AssignOp::SlashAssign),
            Some(Tok::PercentAssign) => Some(AssignOp::PercentAssign),
            _ => None,
        } {
            self.pos += 1;
            let rhs = self.parse_assignment()?;
            return Ok(Expr::Assign(op, Box::new(lhs), Box::new(rhs)));
        }
        Ok(lhs)
    }

    fn parse_or(&mut self) -> Result<Expr, ParseErr> {
        let mut e = self.parse_and()?;
        while self.match_tok(&Tok::Or) {
            let r = self.parse_and()?;
            e = Expr::BinOp(BinOp::Or, Box::new(e), Box::new(r));
        }
        Ok(e)
    }

    fn parse_and(&mut self) -> Result<Expr, ParseErr> {
        let mut e = self.parse_eq()?;
        while self.match_tok(&Tok::And) {
            let r = self.parse_eq()?;
            e = Expr::BinOp(BinOp::And, Box::new(e), Box::new(r));
        }
        Ok(e)
    }

    fn parse_eq(&mut self) -> Result<Expr, ParseErr> {
        let mut e = self.parse_rel()?;
        loop {
            let op = match self.peek() {
                Some(Tok::Eq) => BinOp::Eq,
                Some(Tok::Neq) => BinOp::Neq,
                _ => break,
            };
            self.pos += 1;
            let r = self.parse_rel()?;
            e = Expr::BinOp(op, Box::new(e), Box::new(r));
        }
        Ok(e)
    }

    fn parse_rel(&mut self) -> Result<Expr, ParseErr> {
        let mut e = self.parse_shift()?;
        loop {
            let op = match self.peek() {
                Some(Tok::Lt) => BinOp::Lt,
                Some(Tok::Le) => BinOp::Le,
                Some(Tok::Gt) => BinOp::Gt,
                Some(Tok::Ge) => BinOp::Ge,
                _ => break,
            };
            self.pos += 1;
            let r = self.parse_shift()?;
            e = Expr::BinOp(op, Box::new(e), Box::new(r));
        }
        Ok(e)
    }

    fn parse_shift(&mut self) -> Result<Expr, ParseErr> {
        let first = self.parse_add()?;
        if let Expr::Ident(name) = &first {
            if name == "cout" && matches!(self.peek(), Some(Tok::Shl)) {
                let mut items = Vec::new();
                while self.match_tok(&Tok::Shl) {
                    let r = self.parse_add()?;
                    items.push(r);
                }
                return Ok(Expr::CoutChain(items));
            }
            if name == "cin" && matches!(self.peek(), Some(Tok::Shr)) {
                let mut items = Vec::new();
                while self.match_tok(&Tok::Shr) {
                    let r = self.parse_add()?;
                    items.push(r);
                }
                return Ok(Expr::CinChain(items));
            }
        }
        let mut e = first;
        loop {
            let op = match self.peek() {
                Some(Tok::Shl) => BinOp::Shl,
                Some(Tok::Shr) => BinOp::Shr,
                _ => break,
            };
            self.pos += 1;
            let r = self.parse_add()?;
            e = Expr::BinOp(op, Box::new(e), Box::new(r));
        }
        Ok(e)
    }

    fn parse_add(&mut self) -> Result<Expr, ParseErr> {
        let mut e = self.parse_mul()?;
        loop {
            let op = match self.peek() {
                Some(Tok::Plus) => BinOp::Add,
                Some(Tok::Minus) => BinOp::Sub,
                _ => break,
            };
            self.pos += 1;
            let r = self.parse_mul()?;
            e = Expr::BinOp(op, Box::new(e), Box::new(r));
        }
        Ok(e)
    }

    fn parse_mul(&mut self) -> Result<Expr, ParseErr> {
        let mut e = self.parse_unary()?;
        loop {
            let op = match self.peek() {
                Some(Tok::Star) => BinOp::Mul,
                Some(Tok::Slash) => BinOp::Div,
                Some(Tok::Percent) => BinOp::Mod,
                _ => break,
            };
            self.pos += 1;
            let r = self.parse_unary()?;
            e = Expr::BinOp(op, Box::new(e), Box::new(r));
        }
        Ok(e)
    }

    fn parse_unary(&mut self) -> Result<Expr, ParseErr> {
        match self.peek() {
            Some(Tok::Minus) => { self.pos += 1; let e = self.parse_unary()?; Ok(Expr::UnOp(UnOp::Neg, Box::new(e))) }
            Some(Tok::Plus) => { self.pos += 1; let e = self.parse_unary()?; Ok(Expr::UnOp(UnOp::Pos, Box::new(e))) }
            Some(Tok::Not) => { self.pos += 1; let e = self.parse_unary()?; Ok(Expr::UnOp(UnOp::Not, Box::new(e))) }
            Some(Tok::Inc) => { self.pos += 1; let e = self.parse_unary()?; Ok(Expr::PreInc(Box::new(e))) }
            Some(Tok::Dec) => { self.pos += 1; let e = self.parse_unary()?; Ok(Expr::PreDec(Box::new(e))) }
            _ => self.parse_postfix(),
        }
    }

    fn parse_postfix(&mut self) -> Result<Expr, ParseErr> {
        let mut e = self.parse_primary()?;
        loop {
            match self.peek() {
                Some(Tok::LBracket) => {
                    self.pos += 1;
                    let idx = self.parse_expr()?;
                    self.expect(Tok::RBracket)?;
                    e = Expr::Index(Box::new(e), Box::new(idx));
                }
                Some(Tok::LParen) => {
                    if let Expr::Ident(name) = e.clone() {
                        self.pos += 1;
                        let mut args = Vec::new();
                        if !matches!(self.peek(), Some(Tok::RParen)) {
                            loop {
                                args.push(self.parse_assignment()?);
                                if !self.match_tok(&Tok::Comma) { break; }
                            }
                        }
                        self.expect(Tok::RParen)?;
                        e = Expr::Call(name, args);
                    } else {
                        break;
                    }
                }
                Some(Tok::Inc) => { self.pos += 1; e = Expr::PostInc(Box::new(e)); }
                Some(Tok::Dec) => { self.pos += 1; e = Expr::PostDec(Box::new(e)); }
                _ => break,
            }
        }
        Ok(e)
    }

    fn parse_primary(&mut self) -> Result<Expr, ParseErr> {
        let cur_line = self.peek_line();
        match self.peek().cloned() {
            Some(Tok::IntLit(n)) => { self.pos += 1; Ok(Expr::IntLit(n)) }
            Some(Tok::FloatLit(f)) => { self.pos += 1; Ok(Expr::FloatLit(f)) }
            Some(Tok::StrLit(s)) => { self.pos += 1; Ok(Expr::StrLit(s)) }
            Some(Tok::CharLit(c)) => { self.pos += 1; Ok(Expr::CharLit(c)) }
            Some(Tok::KwTrue) => { self.pos += 1; Ok(Expr::BoolLit(true)) }
            Some(Tok::KwFalse) => { self.pos += 1; Ok(Expr::BoolLit(false)) }
            Some(Tok::KwCout) => { self.pos += 1; Ok(Expr::Ident("cout".to_string())) }
            Some(Tok::KwCin) => { self.pos += 1; Ok(Expr::Ident("cin".to_string())) }
            Some(Tok::Ident(s)) => {
                self.pos += 1;
                // Check for undefined identifier ONLY when not directly followed by
                // a function-call '(' (we resolve calls lazily by name).
                let is_call = matches!(self.peek(), Some(Tok::LParen));
                if !is_call && !self.ident_defined(&s) {
                    return Err(self.err_at(format!("undefined identifier '{}'", s), cur_line));
                }
                Ok(Expr::Ident(s))
            }
            Some(Tok::LParen) => {
                self.pos += 1;
                let e = self.parse_expr()?;
                self.expect(Tok::RParen)?;
                Ok(e)
            }
            other => {
                let got = other.map(|x| tok_display(&x)).unwrap_or_else(|| "<EOF>".to_string());
                Err(self.err_at(format!("unexpected token '{}'", got), cur_line))
            }
        }
    }

    /// Skip tokens until we synchronise to the start of the next top-level
    /// chunk. Strategy: look for a top-level construct beginning — a type
    /// keyword followed by ident, or a known statement-starting keyword,
    /// or the `Done` keyword. Track brace depth so we don't pick up
    /// nested definitions inside function bodies that survive into the
    /// stream after an error.
    pub fn recover_to_next_chunk(&mut self) {
        // First skip the current token to make progress.
        if self.pos < self.toks.len() {
            self.pos += 1;
        }
        let mut depth_paren: i32 = 0;
        let mut depth_brace: i32 = 0;
        while self.pos < self.toks.len() {
            match &self.toks[self.pos].tok {
                Tok::LParen | Tok::LBracket => { depth_paren += 1; self.pos += 1; }
                Tok::RParen | Tok::RBracket => { depth_paren = (depth_paren - 1).max(0); self.pos += 1; }
                Tok::LBrace => { depth_brace += 1; self.pos += 1; }
                Tok::RBrace => {
                    depth_brace -= 1;
                    self.pos += 1;
                    if depth_brace <= 0 && depth_paren == 0 {
                        return;
                    }
                }
                Tok::Semicolon => {
                    self.pos += 1;
                    if depth_paren == 0 && depth_brace == 0 {
                        return;
                    }
                }
                _ => { self.pos += 1; }
            }
        }
    }
}

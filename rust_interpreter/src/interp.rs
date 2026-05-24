// Interpreter for Our-C.

use std::cell::RefCell;
use std::collections::HashMap;
use std::rc::Rc;

use crate::parser::{AssignOp, BinOp, Expr, FnDef, Stmt, Type, UnOp};

#[derive(Debug, Clone)]
pub enum Value {
    Int(i64),
    Float(f64),
    Bool(bool),
    Char(char),
    Str(String),
    Array(Vec<Slot>),
    Void,
}

pub type Slot = Rc<RefCell<Value>>;

fn new_slot(v: Value) -> Slot {
    Rc::new(RefCell::new(v))
}

fn default_for(ty: Type, size: Option<usize>) -> Value {
    if let Some(n) = size {
        let elems: Vec<Slot> = (0..n).map(|_| new_slot(default_scalar(ty))).collect();
        return Value::Array(elems);
    }
    default_scalar(ty)
}

fn default_scalar(ty: Type) -> Value {
    match ty {
        Type::Int => Value::Int(0),
        Type::Float => Value::Float(0.0),
        Type::Bool => Value::Bool(false),
        Type::Char => Value::Char('\0'),
        Type::Str => Value::Str(String::new()),
        Type::Void => Value::Void,
    }
}

// =============== Scopes ===============
type Scope = HashMap<String, Slot>;

pub struct Interp {
    pub globals: Scope,
    pub fn_defs: HashMap<String, FnDef>,
    pub output_buf: String,
}

#[derive(Debug)]
pub enum Ctrl {
    Normal,
    Return(Value),
}

impl Interp {
    pub fn new() -> Self {
        Self {
            globals: HashMap::new(),
            fn_defs: HashMap::new(),
            output_buf: String::new(),
        }
    }

    pub fn register_fn(&mut self, def: FnDef) {
        self.fn_defs.insert(def.name.clone(), def);
    }

    pub fn take_output(&mut self) -> String {
        std::mem::take(&mut self.output_buf)
    }

    pub fn exec_top_stmt(&mut self, stmt: &Stmt) -> Result<(), String> {
        let mut scopes: Vec<Scope> = Vec::new();
        let r = self.exec_stmt(stmt, &mut scopes)?;
        match r {
            Ctrl::Normal => Ok(()),
            Ctrl::Return(_) => Ok(()), // top-level return is ignored
        }
    }

    // ============== Scope lookup ==============
    fn lookup<'a>(&'a self, scopes: &'a [Scope], name: &str) -> Option<&'a Slot> {
        for s in scopes.iter().rev() {
            if let Some(v) = s.get(name) { return Some(v); }
        }
        self.globals.get(name)
    }

    fn declare_in_top(&mut self, scopes: &mut Vec<Scope>, name: String, slot: Slot) {
        if let Some(s) = scopes.last_mut() {
            s.insert(name, slot);
        } else {
            self.globals.insert(name, slot);
        }
    }

    // ============== Statements ==============
    pub fn exec_stmt(&mut self, stmt: &Stmt, scopes: &mut Vec<Scope>) -> Result<Ctrl, String> {
        match stmt {
            Stmt::Empty => Ok(Ctrl::Normal),
            Stmt::Expr(e) => {
                self.eval(e, scopes)?;
                Ok(Ctrl::Normal)
            }
            Stmt::Decl(ty, vars) => {
                for v in vars {
                    let slot = new_slot(default_for(*ty, v.array_size));
                    if let Some(init) = &v.init {
                        let val = self.eval(init, scopes)?;
                        *slot.borrow_mut() = coerce_to(*ty, val)?;
                    }
                    self.declare_in_top(scopes, v.name.clone(), slot);
                }
                Ok(Ctrl::Normal)
            }
            Stmt::If(cond, t, e) => {
                let c = self.eval(cond, scopes)?;
                if to_bool(&c) {
                    self.exec_stmt(t, scopes)
                } else if let Some(es) = e {
                    self.exec_stmt(es, scopes)
                } else {
                    Ok(Ctrl::Normal)
                }
            }
            Stmt::While(cond, body) => {
                loop {
                    let c = self.eval(cond, scopes)?;
                    if !to_bool(&c) { break; }
                    let r = self.exec_stmt(body, scopes)?;
                    if let Ctrl::Return(_) = r { return Ok(r); }
                }
                Ok(Ctrl::Normal)
            }
            Stmt::Return(e) => {
                let v = if let Some(ex) = e { self.eval(ex, scopes)? } else { Value::Void };
                Ok(Ctrl::Return(v))
            }
            Stmt::Block(stmts) => {
                scopes.push(HashMap::new());
                let mut result = Ctrl::Normal;
                for s in stmts {
                    let r = self.exec_stmt(s, scopes)?;
                    if let Ctrl::Return(_) = r { result = r; break; }
                }
                scopes.pop();
                Ok(result)
            }
        }
    }

    // ============== Lvalue resolution ==============
    fn resolve_lvalue(&mut self, e: &Expr, scopes: &mut Vec<Scope>) -> Result<Slot, String> {
        match e {
            Expr::Ident(name) => {
                if let Some(slot) = self.lookup(scopes, name).cloned() {
                    Ok(slot)
                } else {
                    Err(format!("undefined variable: {}", name))
                }
            }
            Expr::Index(arr_e, idx_e) => {
                let arr_slot = self.resolve_lvalue(arr_e, scopes)?;
                let idx = self.eval(idx_e, scopes)?;
                let i = to_int(&idx)? as usize;
                let arr_borrow = arr_slot.borrow();
                if let Value::Array(vec) = &*arr_borrow {
                    if i < vec.len() {
                        Ok(vec[i].clone())
                    } else {
                        Err(format!("array index {} out of bounds", i))
                    }
                } else {
                    Err("not an array".to_string())
                }
            }
            _ => Err(format!("invalid lvalue: {:?}", e)),
        }
    }

    // ============== Eval expression ==============
    pub fn eval(&mut self, e: &Expr, scopes: &mut Vec<Scope>) -> Result<Value, String> {
        match e {
            Expr::IntLit(n) => Ok(Value::Int(*n)),
            Expr::FloatLit(f) => Ok(Value::Float(*f)),
            Expr::BoolLit(b) => Ok(Value::Bool(*b)),
            Expr::StrLit(s) => Ok(Value::Str(s.clone())),
            Expr::CharLit(c) => Ok(Value::Char(*c)),
            Expr::Ident(name) => {
                if let Some(slot) = self.lookup(scopes, name).cloned() {
                    let v = slot.borrow().clone();
                    Ok(v)
                } else {
                    Err(format!("undefined variable: {}", name))
                }
            }
            Expr::Index(arr_e, idx_e) => {
                let slot = self.resolve_lvalue(&Expr::Index(arr_e.clone(), idx_e.clone()), scopes)?;
                let v = slot.borrow().clone();
                Ok(v)
            }
            Expr::Call(name, args) => self.call(name, args, scopes),
            Expr::Assign(op, lhs, rhs) => {
                let lslot = self.resolve_lvalue(lhs, scopes)?;
                let rhs_val = self.eval(rhs, scopes)?;
                let cur = lslot.borrow().clone();
                let new_val = match op {
                    AssignOp::Assign => assign_coerce(&cur, rhs_val),
                    AssignOp::PlusAssign => binop_eval(BinOp::Add, &cur, &rhs_val)?,
                    AssignOp::MinusAssign => binop_eval(BinOp::Sub, &cur, &rhs_val)?,
                    AssignOp::StarAssign => binop_eval(BinOp::Mul, &cur, &rhs_val)?,
                    AssignOp::SlashAssign => binop_eval(BinOp::Div, &cur, &rhs_val)?,
                    AssignOp::PercentAssign => binop_eval(BinOp::Mod, &cur, &rhs_val)?,
                };
                // coerce new_val to current type if scalar
                let coerced = match &cur {
                    Value::Int(_) => coerce_to(Type::Int, new_val).unwrap_or(Value::Int(0)),
                    Value::Float(_) => coerce_to(Type::Float, new_val).unwrap_or(Value::Float(0.0)),
                    Value::Bool(_) => coerce_to(Type::Bool, new_val).unwrap_or(Value::Bool(false)),
                    Value::Char(_) => match new_val { Value::Char(c) => Value::Char(c), v => v },
                    Value::Str(_) => match new_val { Value::Str(s) => Value::Str(s), v => Value::Str(format_value(&v)) },
                    _ => new_val,
                };
                *lslot.borrow_mut() = coerced.clone();
                Ok(coerced)
            }
            Expr::BinOp(op, l, r) => {
                let lv = self.eval(l, scopes)?;
                // Short-circuit for && and ||.
                if let BinOp::And = op {
                    if !to_bool(&lv) { return Ok(Value::Bool(false)); }
                    let rv = self.eval(r, scopes)?;
                    return Ok(Value::Bool(to_bool(&rv)));
                }
                if let BinOp::Or = op {
                    if to_bool(&lv) { return Ok(Value::Bool(true)); }
                    let rv = self.eval(r, scopes)?;
                    return Ok(Value::Bool(to_bool(&rv)));
                }
                let rv = self.eval(r, scopes)?;
                binop_eval(*op, &lv, &rv)
            }
            Expr::UnOp(op, x) => {
                let v = self.eval(x, scopes)?;
                match op {
                    UnOp::Neg => match v {
                        Value::Int(n) => Ok(Value::Int(-n)),
                        Value::Float(f) => Ok(Value::Float(-f)),
                        _ => Err("neg requires numeric".to_string()),
                    },
                    UnOp::Pos => Ok(v),
                    UnOp::Not => Ok(Value::Bool(!to_bool(&v))),
                }
            }
            Expr::PreInc(x) => {
                let slot = self.resolve_lvalue(x, scopes)?;
                let cur = slot.borrow().clone();
                let new_v = add_one(&cur)?;
                *slot.borrow_mut() = new_v.clone();
                Ok(new_v)
            }
            Expr::PreDec(x) => {
                let slot = self.resolve_lvalue(x, scopes)?;
                let cur = slot.borrow().clone();
                let new_v = sub_one(&cur)?;
                *slot.borrow_mut() = new_v.clone();
                Ok(new_v)
            }
            Expr::PostInc(x) => {
                let slot = self.resolve_lvalue(x, scopes)?;
                let cur = slot.borrow().clone();
                let new_v = add_one(&cur)?;
                *slot.borrow_mut() = new_v;
                Ok(cur)
            }
            Expr::PostDec(x) => {
                let slot = self.resolve_lvalue(x, scopes)?;
                let cur = slot.borrow().clone();
                let new_v = sub_one(&cur)?;
                *slot.borrow_mut() = new_v;
                Ok(cur)
            }
            Expr::CoutChain(items) => {
                for it in items {
                    let v = self.eval(it, scopes)?;
                    self.output_buf.push_str(&format_value(&v));
                }
                Ok(Value::Void)
            }
            Expr::CinChain(items) => {
                // not fully supported; consume nothing
                for _ in items {}
                Ok(Value::Void)
            }
        }
    }

    fn call(&mut self, name: &str, args: &[Expr], scopes: &mut Vec<Scope>) -> Result<Value, String> {
        // Evaluate args.  For each parameter:
        //  - if param is array (or by_ref): pass the slot (alias).
        //  - else: pass by value (new slot with copy).
        let fn_def = self.fn_defs.get(name).cloned()
            .ok_or_else(|| format!("undefined function: {}", name))?;

        if args.len() != fn_def.params.len() {
            return Err(format!("function {}: expected {} args, got {}", name, fn_def.params.len(), args.len()));
        }

        let mut new_scope: Scope = HashMap::new();
        let mut arg_slots: Vec<(String, Slot)> = Vec::new();
        for (i, (param, arg)) in fn_def.params.iter().zip(args.iter()).enumerate() {
            let _ = i;
            if param.is_array || param.by_ref {
                // pass by reference - resolve the arg as an lvalue, get its slot.
                let slot = self.resolve_lvalue(arg, scopes)?;
                arg_slots.push((param.name.clone(), slot));
            } else {
                let val = self.eval(arg, scopes)?;
                let coerced = coerce_to(param.ty, val)?;
                arg_slots.push((param.name.clone(), new_slot(coerced)));
            }
        }
        for (n, s) in arg_slots { new_scope.insert(n, s); }

        // Execute the function body in a fresh scope stack (so it doesn't see caller's locals,
        // only globals + its own).
        let mut fn_scopes: Vec<Scope> = vec![new_scope];
        for s in &fn_def.body {
            let r = self.exec_stmt(s, &mut fn_scopes)?;
            if let Ctrl::Return(v) = r {
                return Ok(coerce_to(fn_def.ret_ty, v).unwrap_or(Value::Void));
            }
        }
        Ok(Value::Void)
    }
}

// =============== Helpers ===============

pub fn to_bool(v: &Value) -> bool {
    match v {
        Value::Bool(b) => *b,
        Value::Int(n) => *n != 0,
        Value::Float(f) => *f != 0.0,
        _ => false,
    }
}

pub fn to_int(v: &Value) -> Result<i64, String> {
    match v {
        Value::Int(n) => Ok(*n),
        Value::Float(f) => Ok(*f as i64),
        Value::Bool(b) => Ok(if *b { 1 } else { 0 }),
        Value::Char(c) => Ok(*c as i64),
        _ => Err("not numeric".to_string()),
    }
}

pub fn to_float(v: &Value) -> Result<f64, String> {
    match v {
        Value::Int(n) => Ok(*n as f64),
        Value::Float(f) => Ok(*f),
        Value::Bool(b) => Ok(if *b { 1.0 } else { 0.0 }),
        Value::Char(c) => Ok(*c as u32 as f64),
        _ => Err("not numeric".to_string()),
    }
}

pub fn is_float(v: &Value) -> bool { matches!(v, Value::Float(_)) }
pub fn is_string(v: &Value) -> bool { matches!(v, Value::Str(_)) }

pub fn format_value(v: &Value) -> String {
    match v {
        Value::Int(n) => n.to_string(),
        Value::Float(f) => format!("{:.3}", f),
        Value::Bool(b) => (if *b { "true" } else { "false" }).to_string(),
        Value::Char(c) => c.to_string(),
        Value::Str(s) => s.clone(),
        Value::Array(_) => "<array>".to_string(),
        Value::Void => String::new(),
    }
}

pub fn add_one(v: &Value) -> Result<Value, String> {
    match v {
        Value::Int(n) => Ok(Value::Int(n + 1)),
        Value::Float(f) => Ok(Value::Float(f + 1.0)),
        Value::Char(c) => Ok(Value::Char(((*c as u32) + 1) as u8 as char)),
        _ => Err("++ requires numeric".to_string()),
    }
}

pub fn sub_one(v: &Value) -> Result<Value, String> {
    match v {
        Value::Int(n) => Ok(Value::Int(n - 1)),
        Value::Float(f) => Ok(Value::Float(f - 1.0)),
        Value::Char(c) => Ok(Value::Char(((*c as u32).saturating_sub(1)) as u8 as char)),
        _ => Err("-- requires numeric".to_string()),
    }
}

pub fn coerce_to(ty: Type, v: Value) -> Result<Value, String> {
    Ok(match ty {
        Type::Int => Value::Int(to_int(&v)?),
        Type::Float => Value::Float(to_float(&v)?),
        Type::Bool => Value::Bool(to_bool(&v)),
        Type::Char => match v {
            Value::Char(c) => Value::Char(c),
            Value::Int(n) => Value::Char((n as u8) as char),
            other => other,
        },
        Type::Str => match v {
            Value::Str(s) => Value::Str(s),
            other => Value::Str(format_value(&other)),
        },
        Type::Void => Value::Void,
    })
}

pub fn assign_coerce(lhs_cur: &Value, rhs: Value) -> Value {
    match lhs_cur {
        Value::Int(_) => Value::Int(to_int(&rhs).unwrap_or(0)),
        Value::Float(_) => Value::Float(to_float(&rhs).unwrap_or(0.0)),
        Value::Bool(_) => Value::Bool(to_bool(&rhs)),
        Value::Char(_) => match rhs {
            Value::Char(c) => Value::Char(c),
            Value::Int(n) => Value::Char((n as u8) as char),
            other => other,
        },
        Value::Str(_) => match rhs {
            Value::Str(s) => Value::Str(s),
            other => Value::Str(format_value(&other)),
        },
        _ => rhs,
    }
}

pub fn binop_eval(op: BinOp, l: &Value, r: &Value) -> Result<Value, String> {
    // String concatenation with +
    if op == BinOp::Add && (is_string(l) || is_string(r)) {
        return Ok(Value::Str(format!("{}{}", format_value(l), format_value(r))));
    }
    // Char + Char or Char + Int handled by numeric path below if not string; for cout chars we want them concatenated as chars in string, not added numerically. But this is only via + with strings. So char + char yields int.

    // String comparisons
    if is_string(l) && is_string(r) {
        if let (Value::Str(a), Value::Str(b)) = (l, r) {
            let res = match op {
                BinOp::Eq => a == b,
                BinOp::Neq => a != b,
                BinOp::Lt => a < b,
                BinOp::Le => a <= b,
                BinOp::Gt => a > b,
                BinOp::Ge => a >= b,
                _ => return Err("invalid op on strings".to_string()),
            };
            return Ok(Value::Bool(res));
        }
    }

    match op {
        BinOp::Shl | BinOp::Shr => {
            let li = to_int(l)?;
            let ri = to_int(r)?;
            let res = if op == BinOp::Shl { li << ri } else { li >> ri };
            Ok(Value::Int(res))
        }
        BinOp::Mod => {
            let li = to_int(l)?;
            let ri = to_int(r)?;
            if ri == 0 { return Err("modulo by zero".to_string()); }
            // C semantics: truncated division (sign of dividend)
            Ok(Value::Int(li - (li / ri) * ri))
        }
        BinOp::Add | BinOp::Sub | BinOp::Mul | BinOp::Div => {
            // Numeric: if either is float, result is float
            if is_float(l) || is_float(r) {
                let lf = to_float(l)?;
                let rf = to_float(r)?;
                let res = match op {
                    BinOp::Add => lf + rf,
                    BinOp::Sub => lf - rf,
                    BinOp::Mul => lf * rf,
                    BinOp::Div => {
                        if rf == 0.0 { return Err("division by zero".to_string()); }
                        lf / rf
                    }
                    _ => unreachable!(),
                };
                return Ok(Value::Float(res));
            }
            let li = to_int(l)?;
            let ri = to_int(r)?;
            let res = match op {
                BinOp::Add => li + ri,
                BinOp::Sub => li - ri,
                BinOp::Mul => li * ri,
                BinOp::Div => {
                    if ri == 0 { return Err("division by zero".to_string()); }
                    li / ri
                }
                _ => unreachable!(),
            };
            Ok(Value::Int(res))
        }
        BinOp::Lt | BinOp::Le | BinOp::Gt | BinOp::Ge | BinOp::Eq | BinOp::Neq => {
            // Promote to float if either is float
            if is_float(l) || is_float(r) {
                let lf = to_float(l)?;
                let rf = to_float(r)?;
                let res = match op {
                    BinOp::Lt => lf < rf, BinOp::Le => lf <= rf,
                    BinOp::Gt => lf > rf, BinOp::Ge => lf >= rf,
                    BinOp::Eq => lf == rf, BinOp::Neq => lf != rf,
                    _ => unreachable!(),
                };
                return Ok(Value::Bool(res));
            }
            let li = to_int(l)?;
            let ri = to_int(r)?;
            let res = match op {
                BinOp::Lt => li < ri, BinOp::Le => li <= ri,
                BinOp::Gt => li > ri, BinOp::Ge => li >= ri,
                BinOp::Eq => li == ri, BinOp::Neq => li != ri,
                _ => unreachable!(),
            };
            Ok(Value::Bool(res))
        }
        BinOp::And | BinOp::Or => {
            // short-circuited at caller
            let lb = to_bool(l);
            let rb = to_bool(r);
            Ok(Value::Bool(if op == BinOp::And { lb && rb } else { lb || rb }))
        }
    }
}


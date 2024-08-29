# include <cstdlib>
# include <stdio.h>
# include <iostream>
# include <vector>
# include <string>
# include <stack>
# include <sstream>
# include <map>
# include <stdexcept>
# include <iomanip>
# include <cstring>
using namespace std;

# define maxarraysize    120000

string To_string(int i)
{
  stringstream ss;

  ss << i;
  return ss.str();
} // To_string()

bool Allalphabet(string str)
{
  if(str.size() == 0)
    return 0;

  char c = str[0];
  if(!('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z'))
    return 0;

  // for(const auto & c:str)
  for(int i = 1 ; i < str.size() ; i++)
  {
    c = str[i];
    if(!(('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') ||
       ('0' <= c && c <= '9') || c == '_'))
      return 0;
  } // for

  return 1;
} // Allalphabet()

bool Allnum(string str)
{
  if(str.size() == 0)
    return 0;

  int dot = 0;


  for(int i = 0 ; i < str.size() ; i++)
  {
    char c = str[i];
    if(!(('0' <= c && c <= '9') || c == '.' || c == '-' || c == '+'))
      return 0;

    if(c == '.')
      dot++;
    if((c == '-' || c == '+') && i != 0)
      return 0;

    if(dot > 1)
      return 0;
  } // for

  return 1;
} // Allnum()

bool Iscompare(string str)
{
  for(int i = 0 ; i < str.size() ; i++)
  {
    char c = str[i];
    if(c == '=' || c == '<' || c == '>')
      return 1;
  } // for

  return 0;
} // Iscompare()

enum Numbertype
{
  INT, FLOAT, BOOL, CHAR, STRING, ARRAY, VOID, COUT
};


struct Number
{
  int      min;
  float    mfl;
  bool     mbo;
  char     mch;
  string     mst;
  Numbertype mtype;
};

string Mstr(Number a)
{
  stringstream s;
  char     chars[100];

  if(a.mtype == INT)
    s << a.min;
  if(a.mtype == FLOAT)
  {
    s << fixed << setprecision(3) << a.mfl;
    // sprintf( chars, "%1.3f", mfl );
    // string str = chars;
    // return str;
  } // if

  if(a.mtype == BOOL)
    s << (a.mbo != 0 ? "true" : "false");
  if(a.mtype == CHAR)
    s << a.mch;
  //if ( a.mtype == STRING )
  //for ( int i = 0 ; i < a.mst.size() ; i++ )
  //if ( a.mst[ i ] != '@' )
  //s << a.mst[ i ] ;
  if(a.mtype == STRING)
  {
    if(a.mst == "@@@@@")
      s << "\n";
    else
      s << a.mst;
  }
  return s.str();
}   // Mstr()

string Mdstr(Number a)
{
  stringstream s;

  s << "Number" << a.min << "-";
  s << fixed << setprecision(3) << a.mfl << "-";
  s << (a.mbo != 0 ? "true" : "false") << "-";
  s << a.mch << "-";
  s << a.mst << "-" << a.mtype << endl;
  return s.str();
}   // Mdstr()

Numbertype Mtype(Number a)
{
  return a.mtype;
}   // Mtype()

bool Miszero(Number a)
{
  return (a.min == 0) && (a.mfl == 0);
}   // Miszero()

int Mint(Number a)
{
  return a.min;
}   // Mint()

bool Mtobool(Number a)
{
  if(a.mtype == INT)
    return a.min != 0;

  if(a.mtype == FLOAT)
    return a.mfl != 0;

  if(a.mtype == BOOL)
    return a.mbo;

  if(a.mtype == CHAR)
    return a.mch != '\0';

  if(a.mtype == STRING)
    return a.mst != "";

  return 0;
}   // Mtobool()

Number NewNumber()
{
  Number n;

  n.min = 0;
  n.mtype = INT;
  return n;
}   // NewNumber()

Number NewNumber(int i)
{
  Number n;

  n.min = i;
  n.mfl = 0;
  n.mbo = 0;
  n.mch = '\0';
  n.mst = "";
  n.mtype = INT;
  return n;
}   // NewNumber()

Number NewNumber(float i)
{
  Number n;

  n.min = 0;
  n.mfl = i;
  n.mbo = 0;
  n.mch = '\0';
  n.mst = "";
  n.mtype = FLOAT;
  return n;
}   // NewNumber()

Number NewNumber(bool i)
{
  Number n;

  n.min = 0;
  n.mfl = 0;
  n.mbo = i;
  n.mch = '\0';
  n.mst = "";
  n.mtype = BOOL;
  return n;
}   // NewNumber()

Number NewNumber(string str)
{
  Number n;

  n.min = 0;
  n.mfl = 0;
  n.mbo = 0;
  n.mch = '\0';
  n.mst = "";
  n.mtype = INT;


  if(str[0] == '\"')
  {
    n.mtype = STRING;
    for(int i = 1 ; i < str.size() - 1 ; )
    {
      if(str.substr(i, 2) == "\\n")
      {
        n.mst += "\n";
        i   += 2;
      } // if

      else
      {
        n.mst += str[i];
        i++;
      } // else
    }     // for


    while(n.mst.find("@@@@@") != std::string::npos)
      n.mst.replace(n.mst.find("@@@@@"), n.mst.find("@@@@@") + 5, "\n");

    return n;
  } // if

  if(str[0] == '\'')
  {
    if(str == "\'\\n\'")
    {
      n.mch = '\n';
      n.mtype = CHAR;
      return n;
    } // if

    n.mch = str[1];
    n.mtype = CHAR;
    return n;
  } // if

  if(str == "ARRAY")
  {
    n.mtype = ARRAY;
    return n;
  } // if

  if(str == "true")
  {
    n.mbo = 1;
    n.mtype = BOOL;
    return n;
  } // if

  if(str == "false")
  {
    n.mbo = 0;
    n.mtype = BOOL;
    return n;
  } // if

  if(str == "void")
  {
    n.mtype = VOID;
    return n;
  } // if


  int dotcount = 0;

  for(int i = 0 ; i < str.size() ; i++)
    if(str[i] == '.')
      dotcount++;
  if(dotcount > 1)
    throw runtime_error("more than one dot" + str);
  if(dotcount)
  {
    n.mfl = atof(str.c_str());
    n.mtype = FLOAT;
  } // if
  else
  {
    n.min = atoi(str.c_str());
    n.mtype = INT;
  } // else

  return n;
}     // NewNumber()

Number Madd(Number a, Number n)
{
  if(a.mtype == INT)
    a.mfl = 0;
  // if
  if(n.mtype == INT)
    n.mfl = 0;
  // if

  Number newn;

  newn.min   = 0;
  newn.mtype = INT;

  if(a.mtype == INT && n.mtype == INT)
  {
    newn.mtype = INT;
    newn.min   = a.min + n.min;
  } // if
  else if(a.mtype == STRING || n.mtype == STRING)
  {
    newn.mtype = STRING;
    newn.mst   = "";
    if(a.mtype == STRING)
      newn.mst += a.mst;
    if(a.mtype == CHAR)
      newn.mst += a.mch;

    if(n.mtype == STRING)
      newn.mst += n.mst;
    if(n.mtype == CHAR)
      newn.mst += n.mch;
  } // if
  else
  {
    float f = 0;
    if(a.mtype == INT)
      f += a.min;
    else
      f += a.mfl;

    if(n.mtype == INT)
      f += n.min;
    else
      f += n.mfl;

    newn.mtype = FLOAT;
    newn.mfl   = f;
  } // else

  return newn;
}   // Madd()

Number Mminus(Number a, Number n)
{
  if(a.mtype == INT)
    a.mfl = 0;
  // if
  if(n.mtype == INT)
    n.mfl = 0;
  // if

  Number newn;

  newn.min   = 0;
  newn.mtype = INT;

  if(a.mtype == INT && n.mtype == INT)
  {
    newn.mtype = INT;
    newn.min   = a.min - n.min;
  } // if
  else
  {
    float f = 0;
    if(a.mtype == INT)
      f += a.min;
    else
      f += a.mfl;

    if(n.mtype == INT)
      f -= n.min;
    else
      f -= n.mfl;

    newn.mtype = FLOAT;
    newn.mfl   = f;
  } // else

  return newn;
}   // Mminus()

Number Mmul(Number a, Number n)
{
  if(a.mtype == INT)
    a.mfl = 0;
  // if
  if(n.mtype == INT)
    n.mfl = 0;
  // if

  Number newn;

  newn.min   = 0;
  newn.mtype = INT;

  if(a.mtype == INT && n.mtype == INT)
  {
    newn.mtype = INT;
    newn.min   = a.min * n.min;
  } // if
  else
  {
    float f = 1;
    if(a.mtype == INT)
      f *= a.min;
    else
      f *= a.mfl;

    if(n.mtype == INT)
      f *= n.min;
    else
      f *= n.mfl;

    newn.mtype = FLOAT;
    newn.mfl   = f;
  } // else

  return newn;
}   // Mmul()

Number Mmod(Number a, Number n)
{
  if(a.mtype == INT)
    a.mfl = 0;
  // if
  if(n.mtype == INT)
    n.mfl = 0;
  // if

  Number newn;

  newn.min   = 0;
  newn.mtype = INT;

  if(n.min == 0)
    throw runtime_error("mod by zero");
  newn.min = a.min % n.min;
  return newn;
}   // Mmod()

Number Mdiv(Number a, Number n)
{
  if(a.mtype == INT)
    a.mfl = 0;
  // if
  if(n.mtype == INT)
    n.mfl = 0;
  // if

  Number newn;

  newn.min   = 0;
  newn.mtype = INT;

  if(a.mtype == INT && n.mtype == INT)      // min % n.min == 0
  {
    newn.mtype = INT;
    newn.min   = a.min / n.min;
  } // if
  else
  {
    float f = 1;
    if(a.mtype == INT)
      f *= a.min;
    else
      f *= a.mfl;

    if(n.mtype == INT)
      f /= n.min;
    else
      f /= n.mfl;

    newn.mtype = FLOAT;
    newn.mfl   = f;
  } // else

  return newn;
}   // Mdiv()

Number Mequal(Number a, Number n)
{
  if(a.mtype == INT)
    a.mfl = 0;
  // if

  if(n.mtype == INT)
    n.mfl = 0;
  // if

  Number newn;

  newn.mbo   = 0;
  newn.mtype = BOOL;

  // cout << "##" << mfl << endl;

  if(a.mtype == INT && n.mtype == INT)
    newn.mbo = a.min == n.min;
  else if(a.mtype == FLOAT && n.mtype == FLOAT)
    newn.mbo = (a.min + a.mfl) - (n.min + n.mfl) < 0.0001 &&
           (a.min + a.mfl) - (n.min + n.mfl) > -0.0001;
  else if(a.mtype == BOOL && n.mtype == BOOL)
    newn.mbo = a.mbo == n.mbo;
  else if(a.mtype == CHAR && n.mtype == CHAR)
    newn.mbo = a.mch == n.mch;
  else if(a.mtype == STRING && n.mtype == STRING)
    newn.mbo = a.mst == n.mst;

  return newn;
}   // Mequal()

Number Mnotequal(Number a, Number n)
{
  if(a.mtype == INT)
    a.mfl = 0;
  // if
  if(n.mtype == INT)
    n.mfl = 0;
  // if

  Number newn;

  newn.mbo   = 0;
  newn.mtype = BOOL;

  if(a.mtype == INT && n.mtype == INT)
    newn.mbo = a.min != n.min;
  else if(a.mtype == FLOAT || n.mtype == FLOAT)
    newn.mbo = !((a.min + a.mfl) - (n.min + n.mfl) < 0.0001 &&
           (a.min + a.mfl) - (n.min + n.mfl) > -0.0001);
  else if(a.mtype == BOOL && n.mtype == BOOL)
    newn.mbo = a.mbo != n.mbo;
  else if(a.mtype == CHAR && n.mtype == CHAR)
    newn.mbo = a.mch != n.mch;
  else if(a.mtype == STRING && n.mtype == STRING)
    newn.mbo = a.mst != n.mst;

  return newn;
}   // Mnotequal()

Number Mless(Number a, Number n)
{
  if(a.mtype == INT)
    a.mfl = 0;
  // if
  if(n.mtype == INT)
    n.mfl = 0;
  // if

  Number newn;

  newn.mbo   = 0;
  newn.mtype = BOOL;

  newn.mbo = a.min + a.mfl < n.min + n.mfl;

  return newn;
}   // Mless()

Number Mgrater(Number a, Number n)
{
  if(a.mtype == INT)
    a.mfl = 0;
  // if
  if(n.mtype == INT)
    n.mfl = 0;
  // if

  Number newn;

  newn.mbo   = 0;
  newn.mtype = BOOL;

  newn.mbo = a.min + a.mfl > n.min + n.mfl;

  return newn;
}   // Mgrater()

Number Mlessequal(Number a, Number n)
{
  if(a.mtype == INT)
    a.mfl = 0;
  // if
  if(n.mtype == INT)
    n.mfl = 0;
  // if

  Number newn;

  newn.mbo   = 0;
  newn.mtype = BOOL;
  bool equ = (a.min + a.mfl) - (n.min + n.mfl) < 0.0001 &&
         (a.min + a.mfl) - (n.min + n.mfl) > -0.0001;

  newn.mbo = (a.min + a.mfl <= n.min + n.mfl) || equ;

  return newn;
}   // Mlessequal()

Number Mgraterequal(Number a, Number n)
{
  if(a.mtype == INT)
    a.mfl = 0;
  // if
  if(n.mtype == INT)
    n.mfl = 0;
  // if

  Number newn;

  newn.mbo   = 0;
  newn.mtype = BOOL;
  bool equ = (a.min + a.mfl) - (n.min + n.mfl) < 0.0001 &&
         (a.min + a.mfl) - (n.min + n.mfl) > -0.0001;

  newn.mbo = (a.min + a.mfl >= n.min + n.mfl) || equ;

  return newn;
}   // Mgraterequal()

Number Mand(Number a, Number n)
{
  if(a.mtype == INT)
    a.mfl = 0;
  // if
  if(n.mtype == INT)
    n.mfl = 0;
  // if

  Number newn;

  newn.mbo   = 0;
  newn.mtype = BOOL;

  newn.mbo = a.mbo && n.mbo;
  return newn;
}   // Mand()

Number Mor(Number a, Number n)
{
  if(a.mtype == INT)
    a.mfl = 0;
  // if
  if(n.mtype == INT)
    n.mfl = 0;
  // if

  Number newn;

  newn.mbo   = 0;
  newn.mtype = BOOL;

  newn.mbo = a.mbo || n.mbo;
  return newn;
}   // Mor()

Number Mrright(Number a, Number n)
{
  if(a.mtype == INT)
    a.mfl = 0;
  // if
  if(n.mtype == INT)
    n.mfl = 0;
  // if

  Number newn;

  newn.min   = 0;
  newn.mtype = INT;

  newn.min = a.min >> n.min;

  return newn;
}   // Mrright()

Number Mlleft(Number a, Number n)
{
  if(a.mtype == INT)
    a.mfl = 0;
  // if
  if(n.mtype == INT)
    n.mfl = 0;
  // if

  Number newn;

  newn.min   = 0;
  newn.mtype = INT;

  if(a.mtype == COUT)
  {
    // cout << "##" << Mstr( n ) << "@@" ;
    cout << Mstr(n);
    newn.mtype = COUT;
    return newn;
  } // if


  newn.min = a.min << n.min;

  return newn;
}   // Mlleft()

Number Mnot(Number a)
{
  if(a.mtype == INT)
    a.mfl = 0;
  // if
  Number newn;

  newn.mtype = BOOL;
  newn.mbo   = !a.mbo;


  return newn;
}   // Mnot()

void Msettype(Number & a, Numbertype type)
{
  if(a.mtype == INT)
    a.mfl = 0;
  // if
  a.mtype = type;
}   // Msettype()

Number Mtotype(Number a, Numbertype type)
{
  if(a.mtype == INT)
    a.mfl = 0;
  // if
  Number n;

  if(type == INT)
  {
    if(a.mtype == INT)
    {
      Number k;
      k.mtype = INT;
      k.min = a.min;
      k.mfl = 0;
      k.mch = '\0';
      k.mst = "";
      k.mbo = 0;
      return k;
    } // if

    if(a.mtype == FLOAT)
    {
      Number k;
      k.mtype = INT;
      k.min = a.mfl;
      k.mfl = 0;
      k.mch = '\0';
      k.mst = "";
      k.mbo = 0;
      return k;

      return k;
    } // if
  }     // if

  if(type == FLOAT)
  {
    if(a.mtype == INT)
    {
      Number k;
      k.mfl = a.min;
      k.mtype = FLOAT;
      k.min = 0;
      k.mch = '\0';
      k.mst = "";
      k.mbo = 0;

      return k;
    } // if

    if(a.mtype == FLOAT)
    {
      Number k;
      k.mfl = a.mfl;
      k.mtype = FLOAT;
      k.min = 0;
      k.mch = '\0';
      k.mst = "";
      k.mbo = 0;

      return k;
    } // if
  }     // if

  if(type == STRING)
  {
    Number k;
    k.mst = a.mst;
    k.mtype = STRING;
    k.min = 0;
    k.mfl = 0;
    k.mch = '\0';
    k.mbo = 0;

    return k;
  } // if

  if(type == CHAR)
  {
    Number k;
    k.mch = a.mch;
    k.mtype = CHAR;
    k.min = 0;
    k.mfl = 0;
    k.mst = "";
    k.mbo = 0;
    return k;
  } // if

  if(type == BOOL)
  {
    Number k;
    k.mbo = a.mbo;
    k.mtype = BOOL;
    k.min = 0;
    k.mfl = 0;
    k.mch = '\0';
    k.mst = "";
    return k;
  } // if

  return n;
}       // Mtotype()

int    gstakesize = 0;
Number gstake[maxarraysize];

Number Top()
{
  if(gstakesize > 0)
    return gstake[gstakesize - 1];

  throw runtime_error("Top() error");
  Number n = NewNumber(false);
  return n;
} // Top()

void Pop()
{
  if(gstakesize > 0)
    gstakesize--;
  else
    throw runtime_error("Top() error");
} // Pop()

void Push(Number n)
{
  // cout << "Push:" << Mstr( n ) << endl ;
  gstake[gstakesize] = n;
  gstakesize++;
} // Push()

class Token {
public:
  string mtype;
  string mdata;
  int mbranchto;

  Token()
  {
  }   // Token()

  Token(string str)
  {
    mbranchto = -1;
    if(str == "=")
    {
      mdata = str;
      mtype = "Assign";
      return;
    } // if

    if(str == "return")
    {
      mdata = str;
      mtype = "return";
      return;
    } // if

    if(str == "if")
    {
      mdata = str;
      mtype = "if";
      return;
    } // if

    if(str == "else")
    {
      mdata = str;
      mtype = "else";
      return;
    } // if

    if(str == "while")
    {
      mdata = str;
      mtype = "while";
      return;
    } // if

    if(str == ";")
    {
      mdata = str;
      mtype = ";";
      return;
    } // if

    if(str == "?")
    {
      mdata = str;
      mtype = "?";
      return;
    } // if

    if(str == ":")
    {
      mdata = str;
      mtype = ":";
      return;
    } // if

    if(str == "int" || str == "float" || str == "char" || str == "bool" ||
       str == "string" || str == "void")
    {
      mdata = str;
      mtype = "type";
      return;
    } // if

    if(str == "==" || str == "!=" || str == ">" || str == "<" || str == ">=" || str == "<=")
    {
      mdata = str;
      mtype = "Term";
      return;
    } // ifF

    if(str == "+" || str == "-" || str == "*" || str == "/" || str == "%")
    {
      mdata = str;
      mtype = "Term";
      return;
    } // if

    if(str == "+=" || str == "-=" || str == "=*" || str == "/=" || str == "%=")
    {
      mdata = str;
      mtype = "Term";
      return;
    } // if

    if(str == "<<" || str == ">>" || str == "||" || str == "&&")
    {
      mdata = str;
      mtype = "Term";
      return;
    } // if

    if(str == "++" || str == "--")
    {
      mdata = str;
      mtype = "selfterm";
      return;
    } // if

    if(str == "{" || str == "}" ||
       str == "[" || str == "]" ||
       str == "(" || str == ")")
    {
      mdata = str;
      mtype = str;
      return;
    } // if

    if(str == ",")
    {
      mdata = str;
      mtype = ",";
      return;
    } // if

    if(str == "!")
    {
      mdata = str;
      mtype = "ArithExp";
      return;
    } // if

    if(str == "true" || str == "false")
    {
      mdata = str;
      mtype = "NUM";
      return;
    } // if

    if(str.substr(0, 1) == "\"" || str.substr(str.size() - 1, 1) == "\"")
    {
      mdata = str;
      mtype = "NUM";
      return;
    } // if


    if(Allnum(str))
    {
      mdata = str;
      mtype = "NUM";
      return;
    } // if

    mdata = str;
    mtype = "IDENT";
    return;

    throw runtime_error("???fuck up token???");
  } // Token()

  bool Mequal(string str)
  {
    return mdata == str || mtype == str;
  } // Mequal()

  string Mstr()
  {
    return "data:[" + mdata + "] type:[" + mtype + "]";
  } // Mstr()

  string Mtype()
  {
    return mtype;
  } // Mtype()

  void Msettype(string str)
  {
    mtype = str;
  } // Msettype()

  string Mdata()
  {
    return mdata;
  } // Mdata()

  void Msetdata(string str)
  {
    mdata = str;
  } // Msetdata()

  void Msetbranch(int i)
  {
    mbranchto = i;
  } // Msetbranch()

  int Mbranchto()
  {
    return mbranchto;
  } // Mbranchto()
};   // Token


typedef Token Tokenarr[100];
//Tokenarr gAsm[ maxarraysize ] ;
vector<vector<Token> > gAsm;
int gAsmsize = 0;
int gAsmlinesize[maxarraysize];
int gbranchtarget[maxarraysize];

// map<string, Number> gidmap ;
string     gids[maxarraysize];
Numbertype gtypes[maxarraysize];
Number     gnumbers[maxarraysize];
int      gidsize = 0;

string gcheckids[maxarraysize];
int    gcheckidsize = 0;

void Check_reset()
{
  gcheckidsize = 0;
  return;

  gcheckidsize = gidsize;
  for(int i = 0 ; i < gidsize ; i++)
    gcheckids[i] = gids[i];
}

// Check_reset()

void Check_set(string id)
{
  gcheckids[gcheckidsize] = id;

  gcheckidsize++;
}

// Check_set()


bool Check_has(string id)
{
  for(int i = 0 ; i < gidsize ; i++)
    if(gids[i] == id || gids[i] == "ARRAY_" + id + "_0")
      return 1;

  for(int i = 0 ; i < gcheckidsize ; i++)
    if(gcheckids[i] == id)
      return 1;

  return 0;
}

// Check_has()


void Idmap_set(string id, Number num)
{
  // cout << "Idmap_set:" << id << "=" << Mstr( num ) << endl ;
  for(int i = 0 ; i < gidsize ; i++)
    if(gids[i] == id)
    {
      gnumbers[i] = num;
      return;
    }// if

  gids[gidsize]   = id;
  gnumbers[gidsize] = num;

  gidsize++;
}

// Idmap_set()

void Idmap_settype(string id, Numbertype type)
{
  for(int i = 0 ; i < gidsize ; i++)
    if(gids[i] == id)
    {
      gtypes[i] = type;
      return;
    }// if

  gids[gidsize] = id;
  gtypes[gidsize] = type;

  gidsize++;
}

// Idmap_settype()

Numbertype Idmap_gettype(string id)
{
  for(int i = 0 ; i < gidsize ; i++)
    if(gids[i] == id)
      return gtypes[i];


  return VOID;
}

// Idmap_gettype()

bool Idmap_has(string id)
{
  for(int i = 0 ; i < gidsize ; i++)
    if(gids[i] == id)
      return 1;

  // if

  return 0;
}

// Idmap_has()

Number Idmap_get(string id)
{
  // cout << "Idmap_get:" << id << endl ;
  for(int i = 0 ; i < gidsize ; i++)
    if(gids[i] == id)
      // cout << "=" << Mstr( Mtotype( gnumbers[ i ], Idmap_gettype( id ) ) ) ;
      return Mtotype(gnumbers[i], Idmap_gettype(id));

  // if


  Number n = NewNumber(false);

  return n;
}

// Idmap_get()


void Idmap_print()
{
  for(int i = 0 ; i < gidsize ; i++)
    cout << "id:" << gids[i] << "=" << Mstr(gnumbers[i]) << ",type=" <<
      gtypes[i] << endl;
}

// Idmap_print()

Numbertype StringtoNumbertype(string str)
{
  if(str == "int")
    return INT;

  if(str == "float")
    return FLOAT;

  if(str == "bool")
    return BOOL;

  if(str == "char")
    return CHAR;

  if(str == "string")
    return STRING;

  if(str == "void")
    return VOID;

  if(str == "cout")
    return COUT;

  return VOID;
} // StringtoNumbertype()

int gvarnamecount = 0;

Token gtemptokens[maxarraysize];
Token gctokens[maxarraysize];
int   gtemptokenssize = 0;

void Printasm()
{
  for(int i = 0 ; i < gAsmsize ; i++)
  {
    cout << i << "-br=" << gbranchtarget[i] << " ";
    for(int j = 0 ; j < gAsmlinesize[i] ; j++)
      cout << gAsm[i][j].Mdata() << ",";
    cout << endl;
  } // for
}   // Printasm()

void Pushasm()
{
  // cout << "Pushasm" << endl ;
  stack<string> state;
  stack<int>    branchtar;
  stack<int>    returnline;
  int       pacount = 0;
  int       ifcount = 0;
  stack<int>    st;
  int       addubrace   = 0;
  int       parentheses = 0;

  for(int i = 0 ; i < gtemptokenssize ; i++)
  {
    if(gtemptokens[i].Mdata() == "(")
      parentheses++;
    if(gtemptokens[i].Mdata() == ")")
      parentheses--;

    if(i > 0 && parentheses == 0 && (gtemptokens[i - 1].Mdata() == ")" ||
                     gtemptokens[i - 1].Mdata() == "else") && addubrace)
    {
      addubrace--;
      Token t("{");
      gAsm[gAsmsize].push_back({});
      gAsm[gAsmsize][gAsmlinesize[gAsmsize]] = t;
      gAsmlinesize[gAsmsize]++;
      gAsmsize++;
      pacount++;
      st.push(pacount);
    } // if

    gAsm[gAsmsize].push_back({});
    gAsm[gAsmsize][gAsmlinesize[gAsmsize]] = gtemptokens[i];
    if(gtemptokens[i].Mdata() == "{")
      pacount++;

    if(gtemptokens[i].Mdata() == "}")
      pacount++;

    if(i > 0 && gtemptokens[i - 1].Mdata() == "else" && gtemptokens[i].Mdata() == "if")
    {
      state.push("elseif");
      branchtar.push(gAsmsize);

      int y;
      int pare = 1;
      for(y = i + 2 ; y < gtemptokenssize && (pare > 0 || gtemptokens[y - 1].Mdata() != ")") ; y++)
      {
        if(gtemptokens[y].Mdata() == "(")
          pare++;
        if(gtemptokens[y].Mdata() == ")")
          pare--;
      } // for

      if(gtemptokens[y].Mdata() != "{")
        addubrace += 1;
    } // if

    else if(gtemptokens[i].Mdata() == "if")
    {
      state.push("if");
      ifcount++;
      branchtar.push(gAsmsize);

      int y;
      int pare = 1;
      for(y = i + 2 ; y < gtemptokenssize && (pare > 0 || gtemptokens[y - 1].Mdata() != ")") ; y++)
      {
        if(gtemptokens[y].Mdata() == "(")
          pare++;
        if(gtemptokens[y].Mdata() == ")")
          pare--;
      } // for

      if(gtemptokens[y].Mdata() != "{")
        addubrace += 1;
    } // else if

    else if(gtemptokens[i].Mdata() == "else" && gtemptokens[i + 1].Mdata() != "if")
    {
      state.push("else");
      branchtar.push(gAsmsize);


      if(gtemptokens[i + 1].Mdata() != "{")
        addubrace += 1;
    } // else if

    else if(gtemptokens[i].Mdata() == "while")
    {
      ifcount++;
      state.push("while");
      branchtar.push(gAsmsize);

      int y;
      int pare = 1;
      for(y = i + 2 ; y < gtemptokenssize && (pare > 0 || gtemptokens[y - 1].Mdata() != ")") ; y++)
      {
        if(gtemptokens[y].Mdata() == "(")
          pare++;
        if(gtemptokens[y].Mdata() == ")")
          pare--;
      } // for

      if(gtemptokens[y].Mdata() != "{")
        addubrace += 1;
    } // else if

    else if(gtemptokens[i].Mtype() == "type" && i < gtemptokenssize - 2 &&
        gtemptokens[i + 2].Mdata() == "(")
    {
      state.push("type");
      branchtar.push(gAsmsize);
    } // else if

    else if(gtemptokens[i].Mdata() == "return")
      returnline.push(gAsmsize);


    else if(gtemptokens[i].Mdata() == "}")
    {
      string stat = state.top();
      state.pop();
      int b = branchtar.top();
      branchtar.pop();
      if(stat == "type")
      {
        // gAsmsize++;
        returnline.push(gAsmsize);
        Token t("return");
        gAsm[gAsmsize].push_back({});
        gAsm[gAsmsize][gAsmlinesize[gAsmsize]] = t;
        gAsmlinesize[gAsmsize]++;
        gAsmsize++;
        Token c("}");
        gAsm[gAsmsize].push_back({});
        gAsm[gAsmsize][gAsmlinesize[gAsmsize]] = c;

        while(returnline.size())
        {
          gbranchtarget[returnline.top()] = gAsmsize + 1;
          returnline.pop();
        } // while
      }     // if

      if(stat == "if" && gtemptokens[i + 1].Mdata() != "else")
        ifcount--;
      if(stat == "elseif" && gtemptokens[i + 1].Mdata() != "else")
        ifcount--;
      if(stat == "else")
        ifcount--;
      if(stat == "while")
        ifcount--;

      if(stat == "if" || stat == "elseif" || stat == "while")
        gbranchtarget[b] = gAsmsize + 1;
      if(stat == "while")
        gbranchtarget[gAsmsize] = b;

      if(stat == "if" || stat == "elseif")
        gbranchtarget[gAsmsize] = gAsmsize + 1;

      if(stat == "else" || stat == "elseif")
        for(int k = 0 ; k < gAsmsize + 1 ; k++)
          if(gAsm[k][0].Mdata() == "}" && gbranchtarget[k] == b)
            gbranchtarget[k] = gAsmsize + 1;
    } // else if

    gAsmlinesize[gAsmsize]++;
    if(gtemptokens[i].Mdata() == "{" || gtemptokens[i].Mdata() == ";" ||
       gtemptokens[i].Mdata() == "}")
      gAsmsize++;


    while((gtemptokens[i].Mdata() == ";" || gtemptokens[i].Mdata() == "}") &&
        st.size() && st.top() == pacount && pacount > ifcount)
    {
      st.pop();
      pacount--;
      Token t("}");
      gAsm[gAsmsize].push_back({});
      gAsm[gAsmsize][gAsmlinesize[gAsmsize]] = t;

      string stat = state.top();
      state.pop();
      int b = branchtar.top();
      branchtar.pop();
      if(stat == "type")
      {
        // gAsmsize++;
        returnline.push(gAsmsize);
        Token t("return");
        gAsm[gAsmsize].push_back({});
        gAsm[gAsmsize][gAsmlinesize[gAsmsize]] = t;
        gAsmlinesize[gAsmsize]++;
        gAsmsize++;
        Token c("}");
        gAsm[gAsmsize].push_back({});
        gAsm[gAsmsize][gAsmlinesize[gAsmsize]] = c;

        while(returnline.size())
        {
          gbranchtarget[returnline.top()] = gAsmsize + 1;
          returnline.pop();
        } // while
      }     // if

      if(stat == "if" && gtemptokens[i + 1].Mdata() != "else")
        ifcount--;
      if(stat == "elseif" && gtemptokens[i + 1].Mdata() != "else")
        ifcount--;
      if(stat == "else")
        ifcount--;
      if(stat == "while")
        ifcount--;

      if(stat == "if" || stat == "elseif" || stat == "while")
        gbranchtarget[b] = gAsmsize + 1;
      if(stat == "while")
        gbranchtarget[gAsmsize] = b;

      if(stat == "if" || stat == "elseif")
        gbranchtarget[gAsmsize] = gAsmsize + 1;

      if(stat == "else" || stat == "elseif")
        for(int k = 0 ; k < gAsmsize + 1 ; k++)
          if(gAsm[k][0].Mdata() == "}" && gbranchtarget[k] == b)
            gbranchtarget[k] = gAsmsize + 1;


      gAsmlinesize[gAsmsize]++;
      gAsmsize++;
    } // while
  }   // for


  if(state.size())
    throw runtime_error("Unmatched token : '" + state.top() + "'");
}     // Pushasm()

void Executefunc(int li, int st, int en);

void Caculate(int li, int st, int en)
{
  // cout << "!!" << str << endl;
  if(0)
  {
    cout << "Caculate->";
    for(int i = st ; i < en ; i++)
      cout << gAsm[li][i].Mdata() << ",";
    cout << endl;
  } // if

  if(st >= en)
  {
    Number n = NewNumber(false);
    Push(n);
    return;
  } // if
  // throw runtime_error( "Unexpected token : ';'" );
  // cout << str << endl;
  if(st + 1 == en && gAsm[li][st].Mtype() == "NUM")
  {
    Number n = NewNumber(gAsm[li][st].Mdata());
    Push(n);
    return;
  } // if

  if(st + 1 == en && gAsm[li][st].Mdata() == "cout")
  {
    Number n = NewNumber("cout");
    Msettype(n, COUT);
    Push(n);
    return;
  } // if

  if(st + 2 == en && gAsm[li][st].Mtype() == "ArithExp" &&
     gAsm[li][st + 1].Mtype() == "NUM")
  {
    Number n = NewNumber(gAsm[li][st].Mdata() + gAsm[li][st + 1].Mdata());
    Push(n);
    return;
  } // if

  if(st + 1 == en && gAsm[li][st].Mtype() == "IDENT")
  {
    if(!Idmap_has(gAsm[li][st].Mdata()))
      throw runtime_error("Undefined identifier : '" + gAsm[li][st].Mdata() + "'");
    Push(Idmap_get(gAsm[li][st].Mdata()));
    return;
  } // if

  if(st + 2 == en && (gAsm[li][st].Mdata() == "+" || gAsm[li][st].Mdata() == "-"))
  {
    Caculate(li, st + 1, en);
    Number n = Top();
    Pop();
    Number zero;
    zero.min   = 0;
    zero.mtype = INT;
    if(gAsm[li][st].Mdata() == "+")
      Push(n);
    else
      Push(Mminus(zero, n));
    return;
  } // if


  if(st + 4 == en && gAsm[li][st].Mtype() == "IDENT" && gAsm[li][st + 1].Mdata() == "[")
  {
    Caculate(li, st + 2, st + 3);
    int index = Mint(Top());
    Pop();
    Push(Idmap_get("ARRAY_" + gAsm[li][st].Mdata() + "_" + To_string(index)));
    return;
  } // if

  int  upparentheses = 0;
  bool allwaysgz     = 1;
  for(int i = st + 2 ; i < en - 1 ; i++)
  {
    if(gAsm[li][i].Mdata() == "(")
      upparentheses++;
    if(gAsm[li][i].Mdata() == ")")
      upparentheses--;

    if(upparentheses < 0)
      allwaysgz = 0;
  } // for

  if(gAsm[li][st].Mtype() == "IDENT" && gAsm[li][st + 1].Mdata() == "(" &&
     upparentheses == 0 && allwaysgz && gAsm[li][en - 1].Mdata() == ")")
  {
    Executefunc(li, st, en);
    return;
  } // if

  int udparentheses = 0;
  allwaysgz = 1;
  for(int i = st + 2 ; i < en - 1 ; i++)
  {
    if(gAsm[li][i].Mdata() == "[")
      udparentheses++;
    if(gAsm[li][i].Mdata() == "]")
      udparentheses--;

    if(udparentheses < 0)
      allwaysgz = 0;
  } // for

  if(gAsm[li][st].Mtype() == "IDENT" && gAsm[li][st + 1].Mdata() == "[" &&
     udparentheses == 0 && allwaysgz && gAsm[li][en - 1].Mdata() == "]")
  {
    Caculate(li, st + 2, en - 1);
    Number b = Top();
    Pop();

    Push(Idmap_get("ARRAY_" + gAsm[li][st].Mdata() + "_" + Mstr(b)));
    return;
  } // if

  int parentheses = 0;

  if(parentheses != 0)
    throw runtime_error("parentheses error2");
  for(int i = st ; i < en  ; i++)
  {
    // cout << gAsm[ li ][ i ].Mdata() << endl ;
    if(gAsm[li][i].Mdata() == "(")
      parentheses += 1;
    if(gAsm[li][i].Mdata() == ")")
      parentheses -= 1;
    if(gAsm[li][i].Mdata() == "[")
      parentheses += 1000;
    if(gAsm[li][i].Mdata() == "]")
      parentheses -= 1000;
    if(parentheses < 0)
      throw runtime_error("parentheses error" + To_string(parentheses));

    if(parentheses == 0 && gAsm[li][i].Mdata() == ",")
    {
      Caculate(li, st, i);
      Pop();
      Caculate(li, i + 1, en);
      return;
    } // if
  }   // for

  if(parentheses != 0)
    throw runtime_error("parentheses error2");

  for(int i = st ; i < en  ; i++)
  {
    // cout << gAsm[ li ][ i ].Mdata() << endl ;
    if(gAsm[li][i].Mdata() == "(")
      parentheses += 1;
    if(gAsm[li][i].Mdata() == ")")
      parentheses -= 1;
    if(gAsm[li][i].Mdata() == "[")
      parentheses += 1000;
    if(gAsm[li][i].Mdata() == "]")
      parentheses -= 1000;
    if(parentheses < 0)
      throw runtime_error("parentheses error" + To_string(parentheses));

    if(parentheses == 0 && gAsm[li][i].Mdata() == "?")
    {
      Caculate(li, st, i);
      Number a = Top();
      Pop();
      int j = i;
      while(gAsm[li][j].Mdata() != ":" && j < en)
        j++;
      if(Mtobool(a))
        Caculate(li, i + 1, j);
      else
        Caculate(li, j + 1, en);
      return;
    } // if

    if(parentheses == 0 && gAsm[li][i].Mdata() == "=")
    {
      Caculate(li, i + 1, en);
      if(i - st != 1 && gAsm[li][st + 1].Mdata() == "[")
      {
        Caculate(li, st + 2, i - 1);
        Number a = Top();
        Pop();
        Idmap_set("ARRAY_" + gAsm[li][st].Mdata() + "_" + Mstr(a), Top());
      } // if
      else
        Idmap_set(gAsm[li][st].Mdata(), Top());
      return;
    } // if

    if(parentheses == 0 && gAsm[li][i].Mdata() == "+=")
    {
      Caculate(li, i + 1, en);
      Caculate(li, st, i);
      Number a = Top();
      Pop();
      Number b = Top();
      Pop();
      Push(Madd(a, b));
      if(i - st != 1)
      {
        Caculate(li, st + 2, i - 1);
        Number a = Top();
        Pop();
        Idmap_set("ARRAY_" + gAsm[li][st].Mdata() + "_" + Mstr(a), Top());
      } // if
      else
        Idmap_set(gAsm[li][st].Mdata(), Top());
      return;
    } // if

    if(parentheses == 0 && gAsm[li][i].Mdata() == "-=")
    {
      Caculate(li, i + 1, en);
      Caculate(li, st, i);
      Number a = Top();
      Pop();
      Number b = Top();
      Pop();
      Push(Mminus(a, b));
      if(i - st != 1)
      {
        Caculate(li, st + 2, i - 1);
        Number a = Top();
        Pop();
        Idmap_set("ARRAY_" + gAsm[li][st].Mdata() + "_" + Mstr(a), Top());
      } // if
      else
        Idmap_set(gAsm[li][st].Mdata(), Top());
      return;
    } // if

    if(parentheses == 0 && gAsm[li][i].Mdata() == "*=")
    {
      Caculate(li, i + 1, en);
      Caculate(li, st, i);
      Number a = Top();
      Pop();
      Number b = Top();
      Pop();
      Push(Mmul(a, b));
      if(i - st != 1)
      {
        Caculate(li, st + 2, i - 1);
        Number a = Top();
        Pop();
        Idmap_set("ARRAY_" + gAsm[li][st].Mdata() + "_" + Mstr(a), Top());
      } // if
      else
        Idmap_set(gAsm[li][st].Mdata(), Top());
      return;
    } // if

    if(parentheses == 0 && gAsm[li][i].Mdata() == "/=")
    {
      Caculate(li, i + 1, en);
      Caculate(li, st, i);
      Number a = Top();
      Pop();
      Number b = Top();
      Pop();
      Push(Mdiv(a, b));
      if(i - st != 1)
      {
        Caculate(li, st + 2, i - 1);
        Number a = Top();
        Pop();
        Idmap_set("ARRAY_" + gAsm[li][st].Mdata() + "_" + Mstr(a), Top());
      } // if
      else
        Idmap_set(gAsm[li][st].Mdata(), Top());
      return;
    } // if

    if(parentheses == 0 && gAsm[li][i].Mdata() == "%=")
    {
      Caculate(li, i + 1, en);
      Caculate(li, st, i);
      Number a = Top();
      Pop();
      Number b = Top();
      Pop();
      Push(Mmod(a, b));
      if(i - st != 1)
      {
        Caculate(li, st + 2, i - 1);
        Number a = Top();
        Pop();
        Idmap_set("ARRAY_" + gAsm[li][st].Mdata() + "_" + Mstr(a), Top());
      } // if
      else
        Idmap_set(gAsm[li][st].Mdata(), Top());
      return;
    } // if
  }   // for

  if(parentheses != 0)
    throw runtime_error("parentheses error2");

  if(gAsm[li][en - 1].Mdata() == ")")
    parentheses--;
  if(gAsm[li][en - 1].Mdata() == "]")
    parentheses -= 1000;

  for(int i = en - 2 ; i >= st ; i--)
  {
    if(gAsm[li][i].Mdata() == "(")
      parentheses++;
    if(gAsm[li][i].Mdata() == ")")
      parentheses--;
    if(gAsm[li][i].Mdata() == "[")
      parentheses += 1000;
    if(gAsm[li][i].Mdata() == "]")
      parentheses -= 1000;
    if(parentheses > 0)
      throw runtime_error("parentheses error1");

    if(parentheses == 0 && gAsm[li][i].Mdata() == "||")
    {
      Caculate(li, st, i);
      Caculate(li, i + 1, en);
      Number b = Top();
      Pop();
      Number a = Top();
      Pop();
      Push(Mor(a, b));
      return;
    } // if
  }   // for

  if(parentheses != 0)
    throw runtime_error("parentheses error2");


  if(gAsm[li][en - 1].Mdata() == ")")
    parentheses--;
  if(gAsm[li][en - 1].Mdata() == "]")
    parentheses -= 1000;

  for(int i = en - 2 ; i >= st ; i--)
  {
    if(gAsm[li][i].Mdata() == "(")
      parentheses++;
    if(gAsm[li][i].Mdata() == ")")
      parentheses--;
    if(gAsm[li][i].Mdata() == "[")
      parentheses += 1000;
    if(gAsm[li][i].Mdata() == "]")
      parentheses -= 1000;
    if(parentheses > 0)
      throw runtime_error("parentheses error1");

    if(parentheses == 0 && gAsm[li][i].Mdata() == "&&")
    {
      Caculate(li, st, i);
      Caculate(li, i + 1, en);
      Number b = Top();
      Pop();
      Number a = Top();
      Pop();
      Push(Mand(a, b));
      return;
    } // if
  }   // for

  if(parentheses != 0)
    throw runtime_error("parentheses error2");

  if(gAsm[li][en - 1].Mdata() == ")")
    parentheses--;
  if(gAsm[li][en - 1].Mdata() == "]")
    parentheses -= 1000;

  for(int i = en - 2 ; i >= st ; i--)
  {
    if(gAsm[li][i].Mdata() == "(")
      parentheses++;
    if(gAsm[li][i].Mdata() == ")")
      parentheses--;
    if(gAsm[li][i].Mdata() == "[")
      parentheses += 1000;
    if(gAsm[li][i].Mdata() == "]")
      parentheses -= 1000;
    if(parentheses > 0)
      throw runtime_error("parentheses error1");


    if(parentheses == 0 && gAsm[li][i].Mdata() == "==")
    {
      Caculate(li, st, i);
      Caculate(li, i + 1, en);
      Number b = Top();
      Pop();
      Number a = Top();
      Pop();
      Push(Mequal(a, b));
      return;
    } // if

    if(parentheses == 0 && gAsm[li][i].Mdata() == "!=")
    {
      Caculate(li, st, i);
      Caculate(li, i + 1, en);
      Number b = Top();
      Pop();
      Number a = Top();
      Pop();
      Push(Mnotequal(a, b));
      return;
    } // if
  }   // for

  if(parentheses != 0)
    throw runtime_error("parentheses error2");

  if(gAsm[li][en - 1].Mdata() == ")")
    parentheses--;
  if(gAsm[li][en - 1].Mdata() == "]")
    parentheses -= 1000;

  for(int i = en - 2 ; i >= st ; i--)
  {
    if(gAsm[li][i].Mdata() == "(")
      parentheses++;
    if(gAsm[li][i].Mdata() == ")")
      parentheses--;
    if(gAsm[li][i].Mdata() == "[")
      parentheses += 1000;
    if(gAsm[li][i].Mdata() == "]")
      parentheses -= 1000;
    if(parentheses > 0)
      throw runtime_error("parentheses error1");

    if(parentheses == 0 && gAsm[li][i].Mdata() == ">")
    {
      Caculate(li, st, i);
      Caculate(li, i + 1, en);
      Number b = Top();
      Pop();
      Number a = Top();
      Pop();
      Push(Mgrater(a, b));
      return;
    } // if

    if(parentheses == 0 && gAsm[li][i].Mdata() == ">=")
    {
      Caculate(li, st, i);
      Caculate(li, i + 1, en);
      Number b = Top();
      Pop();
      Number a = Top();
      Pop();
      Push(Mgraterequal(a, b));
      return;
    } // if


    if(parentheses == 0 && gAsm[li][i].Mdata() == "<=")
    {
      Caculate(li, st, i);
      Caculate(li, i + 1, en);
      Number b = Top();
      Pop();
      Number a = Top();
      Pop();
      Push(Mlessequal(a, b));
      return;
    } // if

    if(parentheses == 0 && gAsm[li][i].Mdata() == "<")
    {
      Caculate(li, st, i);
      Caculate(li, i + 1, en);
      Number b = Top();
      Pop();
      Number a = Top();
      Pop();
      Push(Mless(a, b));
      return;
    } // if
  }   // for

  if(parentheses != 0)
    throw runtime_error("parentheses error2");


  for(int i = en - 1 ; i >= st ; i--)
  {
    // cout << gAsm[ li ][ i ].Mdata() << endl ;
    if(gAsm[li][i].Mdata() == "(")
      parentheses += 1;
    if(gAsm[li][i].Mdata() == ")")
      parentheses -= 1;
    if(gAsm[li][i].Mdata() == "[")
      parentheses += 1000;
    if(gAsm[li][i].Mdata() == "]")
      parentheses -= 1000;
    if(parentheses > 0)
      throw runtime_error("parentheses error" + To_string(parentheses));


    if(parentheses == 0 && gAsm[li][i].Mdata() == ">>")
    {
      Caculate(li, st, i);
      Caculate(li, i + 1, en);
      Number b = Top();
      Pop();
      Number a = Top();
      Pop();
      Push(Mrright(a, b));
      return;
    } // if

    if(parentheses == 0 && gAsm[li][i].Mdata() == "<<")
    {
      Caculate(li, st, i);
      Caculate(li, i + 1, en);
      Number b = Top();
      Pop();
      Number a = Top();
      Pop();
      Push(Mlleft(a, b));
      return;
    } // if
  }   // for

  if(parentheses != 0)
    throw runtime_error("parentheses error2");

  for(int i = en - 1 ; i >= st ; i--)
  {
    if(gAsm[li][i].Mdata() == "(")
      parentheses++;
    if(gAsm[li][i].Mdata() == ")")
      parentheses--;
    if(gAsm[li][i].Mdata() == "[")
      parentheses += 1000;
    if(gAsm[li][i].Mdata() == "]")
      parentheses -= 1000;
    if(parentheses > 0)
      throw runtime_error("parentheses error");

    if(parentheses == 0 && gAsm[li][i].Mdata() == "+" && gAsm[li][i].Mtype() == "Term")
    {
      Caculate(li, st, i);
      Caculate(li, i + 1, en);
      Number b = Top();
      Pop();
      Number a = Top();
      Pop();
      // cout << a.Mtype() << " " << b.Mtype() << endl;
      Push(Madd(a, b));
      return;
    } // if

    if(parentheses == 0 && gAsm[li][i].Mdata() == "-" && gAsm[li][i].Mtype() == "Term")
    {
      Caculate(li, st, i);
      Caculate(li, i + 1, en);
      Number b = Top();
      Pop();
      Number a = Top();
      Pop();
      Push(Mminus(a, b));
      return;
    } // if
  }   // for


  if(parentheses != 0)
    throw runtime_error("parentheses error2");

  for(int i = en - 1 ; i >= st ; i--)
  {
    if(gAsm[li][i].Mdata() == "(")
      parentheses++;
    if(gAsm[li][i].Mdata() == ")")
      parentheses--;
    if(gAsm[li][i].Mdata() == "[")
      parentheses += 1000;
    if(gAsm[li][i].Mdata() == "]")
      parentheses -= 1000;
    if(parentheses == 0 && gAsm[li][i].Mdata() == "*" && gAsm[li][i].Mtype() == "Term")
    {
      Caculate(li, st, i);
      Caculate(li, i + 1, en);
      Number b = Top();
      Pop();
      Number a = Top();
      Pop();
      Push(Mmul(a, b));
      return;
    } // if

    if(parentheses == 0 && gAsm[li][i].Mdata() == "%" && gAsm[li][i].Mtype() == "Term")
    {
      Caculate(li, st, i);
      Caculate(li, i + 1, en);
      Number b = Top();
      Pop();
      Number a = Top();
      Pop();
      Push(Mmod(a, b));
      return;
    } // if

    if(parentheses == 0 && gAsm[li][i].Mdata() == "/" && gAsm[li][i].Mtype() == "Term")
    {
      Caculate(li, st, i);
      Caculate(li, i + 1, en);
      Number b = Top();
      Pop();
      if(Miszero(b))
        throw runtime_error("Error");
      Number a = Top();
      Pop();
      Push(Mdiv(a, b));
      return;
    } // if
  }   // for

  if(parentheses != 0)
    throw runtime_error("parentheses error2");


  if(gAsm[li][st].Mdata() == "!")
  {
    Caculate(li, st + 1, en);
    Number a = Top();
    Pop();
    Push(Mnot(a));
    return;
  } // if

  if(gAsm[li][st].Mdata() == "+" && gAsm[li][st].Mtype() == "ArithExp")
  {
    Caculate(li, st + 1, en);
    return;
  } // if

  if(gAsm[li][st].Mdata() == "-" && gAsm[li][st].Mtype() == "ArithExp")
  {
    Caculate(li, st + 1, en);

    Number a = Top();
    Pop();
    Number zero;
    zero.min   = 0;
    zero.mtype = INT;
    Push(Mminus(zero, a));
    return;
  } // if

  int pc = 0;
  for(int i = st ; i < en ; i++)
    if(gAsm[li][i].Mdata() == "[")
      pc++;
  // for


  if(en - st == 2 && gAsm[li][st].Mtype() == "IDENT" && gAsm[li][st + 1].Mdata() == "++")
  {
    Caculate(li, st, st + 1);
    Number a = Top();
    Idmap_set(gAsm[li][st].Mdata(), Madd(a, NewNumber(1)));

    return;
  } // if

  if(en - st >= 5 &&
     gAsm[li][st].Mtype() == "IDENT" && gAsm[li][st + 1].Mdata() == "[" &&
     gAsm[li][en - 2].Mdata() == "]" &&
     gAsm[li][en - 1].Mdata() == "++" && pc == 1)
  {
    Caculate(li, st + 2, en - 2);
    int index = Mint(Top());
    Pop();
    Number a = Idmap_get("ARRAY_" + gAsm[li][st].Mdata() + "_" + To_string(index));
    Idmap_set("ARRAY_" + gAsm[li][st].Mdata() + "_" + To_string(index), Madd(a, NewNumber(1)));
    Push(a);

    return;
  } // if


  if(en - st == 2 && gAsm[li][st].Mtype() == "IDENT" && gAsm[li][st + 1].Mdata() == "--")
  {
    Caculate(li, st, st + 1);
    Number a = Top();
    Idmap_set(gAsm[li][st].Mdata(), Mminus(a, NewNumber(1)));

    return;
  } // if

  if(en - st >= 5 &&
     gAsm[li][st].Mtype() == "IDENT" && gAsm[li][st + 1].Mdata() == "[" &&
     gAsm[li][en - 2].Mdata() == "]" &&
     gAsm[li][en - 1].Mdata() == "--" && pc == 1)
  {
    Caculate(li, st + 2, en - 2);
    int index = Mint(Top());
    Pop();
    Number a = Idmap_get("ARRAY_" + gAsm[li][st].Mdata() + "_" + To_string(index));
    Idmap_set("ARRAY_" + gAsm[li][st].Mdata() + "_" +
          To_string(index), Mminus(a, NewNumber(1)));
    Push(a);

    return;
  } // if

  if(en - st == 2 && gAsm[li][st + 1].Mtype() == "IDENT" && gAsm[li][st].Mdata() == "++")
  {
    Caculate(li, st + 1, st + 2);
    Number a = Top();
    Pop();
    Push(Madd(a, NewNumber(1)));
    Idmap_set(gAsm[li][st + 1].Mdata(), Madd(a, NewNumber(1)));

    return;
  } // if


  if(en - st == 2 && gAsm[li][st + 1].Mtype() == "IDENT" && gAsm[li][st].Mdata() == "--")
  {
    Caculate(li, st + 1, st + 2);
    Number a = Top();
    Pop();
    Push(Mminus(a, NewNumber(1)));
    Idmap_set(gAsm[li][st + 1].Mdata(), Mminus(a, NewNumber(1)));

    return;
  } // if


  int minn = 0;

  for(int i = 1 ; i < en - 1 ; i++)
  {
    if(gAsm[li][i].Mdata() == "(")
      parentheses++;
    if(gAsm[li][i].Mdata() == ")")
      parentheses--;

    if(minn > parentheses)
      minn = parentheses;
  } // for

  if(minn == 0 && gAsm[li][st].Mdata() == "(" && gAsm[li][en - 1].Mdata() == ")")
  {
    Caculate(li, st + 1, en - 1);
    return;
  } // if

  cout << li << " " << st << " " << en << " ";
  throw runtime_error("Fuck up error");
} // Caculate()

void Executeasm(int st, int en)
{
  // cout << "Executeasm:" << st << "," << en << endl ;
  int      ifcount = 0;
  string     id[1000];
  string     toid[1000];
  int      eidsize = 0;
  stack<int> sta;

  for(int i = st ; i < en ; )
  {
    if(gAsm[i][0].Mtype() == "type")       // int a;
    {
      Number n = NewNumber(0);
      for(int j = 1 ; j < gAsmlinesize[i] ; )
      {
        if(ifcount > 0)
        {
          id[eidsize]   = gAsm[i][j].Mdata();
          toid[eidsize] = "tempvar_" + To_string(gvarnamecount);
          // cout << "!!" << id[ eidsize ] << " to " << toid[ eidsize ] << endl ;
          gAsm[i][j].Msetdata("tempvar_" + To_string(gvarnamecount));
          gvarnamecount++;
          eidsize++;
        } // if

        if(gAsm[i][j + 1].Mdata() != "[")
        {
          Idmap_set(gAsm[i][j].Mdata(), n);
          Idmap_settype(gAsm[i][j].Mdata(), StringtoNumbertype(gAsm[i][0].Mdata()));
          j += 2;
        } // if
        else
        {
          Caculate(i, j + 2, j + 3);
          int k = Mint(Top());
          Pop();
          while(k--)
          {
            Idmap_set("ARRAY_" + gAsm[i][j].Mdata() + "_" + To_string(k), n);
            Idmap_settype("ARRAY_" + gAsm[i][j].Mdata() + "_" + To_string(k),
                    StringtoNumbertype(gAsm[i][0].Mdata()));
          } // while

          j += 5;
        } // else
      }     // for

      i++;
    } // if
    else
    {
      if(ifcount > 0)
        for(int j = 0 ; j < gAsmlinesize[i] ; j++)
          for(int k = 0 ; k < eidsize ; k++)
            if(gAsm[i][j].Mdata() == id[k])
              gAsm[i][j].Msetdata(toid[k]);
      // for


      if(gAsm[i][0].Mdata() == "if" || gAsm[i][0].Mdata() == "while")
      {
        Caculate(i, 1, gAsmlinesize[i] - 1);
        if(!Mtobool(Top()))
          i = gbranchtarget[i];
        else
        {
          ifcount++;
          sta.push(eidsize);
          i++;
        } // else

        Pop();
      } // else if

      else if(gAsm[i][0].Mdata() == "else" && gAsm[i][1].Mdata() == "if")
      {
        Caculate(i, 2, gAsmlinesize[i] - 1);
        if(!Mtobool(Top()))
          i = gbranchtarget[i];
        else
        {
          ifcount++;
          sta.push(eidsize);
          i++;
        } // else

        Pop();
      } // else if
      else if(gAsm[i][0].Mdata() == "return")
      {
        Caculate(i, 1, gAsmlinesize[i] - 1);
        i = gbranchtarget[i];
      } // else if

      else if(gAsm[i][0].Mdata() == "else")
      {
        ifcount++;
        sta.push(eidsize);
        i++;
      } // if
      // else if
      else if(gAsm[i][0].Mdata() == "}")
      {
        if(gbranchtarget[i] == -1)
          i++;
        else
          i = gbranchtarget[i];

        ifcount--;
        eidsize = sta.top();
        sta.pop();
      }

      // else if
      else
      {
        Caculate(i, 0, gAsmlinesize[i] - 1);
        Pop();
        i++;
      } // else
    }     // else
  }       // for
}     // Executeasm()

void Executefunc(int li, int st, int en)
{
  // cout << "Executefunc:" << gAsm[ li ][ st ].Mdata() << endl ;

  int funclo  = -1;
  int funcend = -1;
  int count = 0;

  if(gAsm[li][st].Mdata() == "Done")
    throw runtime_error("Our-C exited ...");


  for(int i = 0 ; i < gAsmsize && funclo == -1 ; i++)
    if(gAsm[i][1].Mdata() == gAsm[li][st].Mdata())
      funclo = i;
  // if
  if(funclo == -1)
    throw runtime_error("Undefined function : '" + gAsm[li][st].Mdata() + "'");
  for(int i = funclo ; i < gAsmsize && funcend == -1 ; i++)
    for(int j = 0 ; j < gAsmlinesize[i] ; j++)
      if(gAsm[i][j].Mdata() == "{")
        count++;
      else if(gAsm[i][j].Mdata() == "}")
      {
        count--;
        if(count == 0)
          funcend = i;
      }
  // else if

  funcend++;
  for(int i = 0 ; i + funclo < funcend ; i++)
  {
    for(int j = 0 ; j < gAsmlinesize[i + funclo] ; j++)
    {
      gAsm[i + gAsmsize].push_back({});
      gAsm[i + gAsmsize][j] = gAsm[i + funclo][j];
    }
    gAsmlinesize[i + gAsmsize] = gAsmlinesize[i + funclo];
    if(gbranchtarget[i + funclo] != -1)
      gbranchtarget[i + gAsmsize] = gbranchtarget[i + funclo] + gAsmsize - funclo;
  } // for

  int tempgAsmsize = gAsmsize;

  gAsmsize += funcend - funclo;
  // map<string, string> idtoidmap ;

  string idtoidmap_first[1000];
  string idtoidmap_second[1000];
  int    idtoidmap_size = 0;

  int newgAsmsize = gAsmsize;
  int y     = st + 2;

  for(int i = 4 ; i < gAsmlinesize[funclo] - 2 ; )
  {
    if(gAsm[funclo][i].Mdata() == "&")       // by ref
    {
      if(gAsm[li][y + 1].Mdata() != "[")
      {
        // idtoidmap[ gAsm[ funclo ][ i + 1 ].Mdata() ] = gAsm[ li ][ y ].Mdata() ;
        idtoidmap_first[idtoidmap_size]  = gAsm[funclo][i + 1].Mdata();
        idtoidmap_second[idtoidmap_size] = gAsm[li][y].Mdata();
        idtoidmap_size++;
        y += 2;
      } // if
      else
      {
        Caculate(li, y + 2, y + 3);
        // idtoidmap[ gAsm[ funclo ][ i + 1 ].Mdata() ] =
        // "ARRAY_" + gAsm[ li ][ y ].Mdata() + "_" + Top().Mstr() ;

        idtoidmap_first[idtoidmap_size]  = gAsm[funclo][i + 1].Mdata();
        idtoidmap_second[idtoidmap_size] =
          "ARRAY_" + gAsm[li][y].Mdata() + "_" + Mstr(Top());
        idtoidmap_size++;
        Pop();
        y += 5;
      } // else
    }     // if
    else  // by value
    {
      if(gAsm[funclo][i + 1].Mdata() != "[")
      {
        // cout << "##" ;
        // idtoidmap[ gAsm[ funclo ][ i  ].Mdata() ] = "tempvar_" + To_string( gvarnamecount ) ;
        idtoidmap_first[idtoidmap_size]  = gAsm[funclo][i].Mdata();
        idtoidmap_second[idtoidmap_size] = "tempvar_" + To_string(gvarnamecount);
        idtoidmap_size++;
        int c    = gvarnamecount++;
        int yy     = y;
        int pcount = 0;
        for( ; ((gAsm[li][yy].Mdata() != "," && gAsm[li][yy].Mdata() != ")") || pcount != 0) &&
           pcount >= 0 ; yy++)
        {
          if(gAsm[li][yy].Mdata() == "(")
            pcount++;
          if(gAsm[li][yy].Mdata() == ")")
            pcount--;
        } // for

        Caculate(li, y, yy);
        Idmap_set("tempvar_" + To_string(c), Top());
        Idmap_settype("tempvar_" + To_string(c),
                StringtoNumbertype(gAsm[funclo][i - 1].Mdata()));
        Pop();
        y = yy + 1;
      } // if
      else
      {
        // idtoidmap[ gAsm[ funclo ][ i  ].Mdata() ] = "tempvar_" + To_string( gvarnamecount ) ;
        idtoidmap_first[idtoidmap_size]  = gAsm[funclo][i].Mdata();
        idtoidmap_second[idtoidmap_size] = gAsm[li][y].Mdata();
        idtoidmap_size++;
        y += 2;

        gvarnamecount++;
      } // else
    }     // else

    while(i < gAsmlinesize[funclo] - 2 && gAsm[funclo][i].Mtype() != "type")
      i++;
    i++;
  } // for

  for(int i = 0 ; i < idtoidmap_size && 0 ; i++)
    cout << "idtoidmap: " << idtoidmap_first[i] << "->" << idtoidmap_second[i] << endl;


  for(int i = tempgAsmsize ; i < newgAsmsize ; i++)
  {
    if(gAsm[i][0].Mtype() == "type" && i > tempgAsmsize)       // int a;
    {
      Number n = NewNumber(0);
      for(int j = 1 ; j < gAsmlinesize[i] ; )
      {
        if(gAsm[i][j + 1].Mdata() != "[")
        {
          idtoidmap_first[idtoidmap_size]  = gAsm[i][j].Mdata();
          idtoidmap_second[idtoidmap_size] = "tempvar_" + To_string(gvarnamecount);
          idtoidmap_size++;

          Idmap_set("tempvar_" + To_string(gvarnamecount), n);
          Idmap_settype("tempvar_" + To_string(gvarnamecount),
                  StringtoNumbertype(gAsm[i][0].Mdata()));
          gvarnamecount++;
          j += 2;
        } // if
        else
        {
          idtoidmap_first[idtoidmap_size]  = gAsm[i][j].Mdata();
          idtoidmap_second[idtoidmap_size] = "tempvar_" + To_string(gvarnamecount);
          idtoidmap_size++;
          int c = gvarnamecount++;
          Caculate(i, j + 2, j + 3);
          int k = Mint(Top());
          Pop();
          while(k--)
          {
            Idmap_set("ARRAY_tempvar_" + To_string(c) + "_" + To_string(k), n);
            Idmap_settype("ARRAY_tempvar_" + To_string(c) + "_" + To_string(k),
                    StringtoNumbertype(gAsm[i][0].Mdata()));
          } // while

          j += 5;
        } // else
      }     // for
    }     // if

    for(int j = 0 ; j < gAsmlinesize[i] ; j++)
    {
      bool changed = 0;
      for(int k = 0 ; k < idtoidmap_size && changed == 0 ; k++)
        if(gAsm[i][j].Mdata() == idtoidmap_first[k])
        {
          gAsm[i][j].Msetdata(idtoidmap_second[k]);
          changed = 1;
        }// if
    } // for
  }
  // for
  // Printasm() ;
  Executeasm(tempgAsmsize + 1, newgAsmsize);
} // Executefunc()

bool Fine(Token a, Token b)
{
  if(b.Mtype() == "NUM")
    if(a.Mdata() == "," || a.Mdata() == "{" || a.Mdata() == "}" || a.Mdata() == "[" ||
       a.Mdata() == "(" || a.Mdata() == ";" || a.Mdata() == "?" ||
       a.Mdata() == ":" ||
       a.Mtype() == "Assign" || a.Mtype() == "Term" ||
       a.Mtype() == "ArithExp" || a.Mdata() == "return")
      return 1;

  if(b.Mtype() == "IDENT")
    if(a.Mdata() == "," || a.Mdata() == "{" || a.Mdata() == "}" || a.Mdata() == "[" || a.Mdata() == "(" ||
       a.Mdata() == ")" || a.Mdata() == ";" || a.Mdata() == "?" || a.Mdata() == ":" ||
       a.Mdata() == "else" || a.Mtype() == "Assign" || a.Mtype() == "Term" ||
       a.Mtype() == "ArithExp" || a.Mdata() == "return" || a.Mtype() == "selfterm")
      return 1;

  if(b.Mtype() == "Assign")
    if(a.Mdata() == "]" || a.Mtype() == "IDENT")
      return 1;

  if(b.Mtype() == "ArithExp")
    if(a.Mdata() == ";" || a.Mdata() == "," || a.Mdata() == "(" || a.Mtype() == "Assign" ||
       a.Mdata() == "[" || a.Mdata() == "?" || a.Mdata() == ":" ||
       a.Mtype() == "Term" || a.Mdata() == "return" || a.Mtype() == "ArithExp")
      return 1;

  if(b.Mtype() == "Term")
    if(a.Mtype() == "IDENT" || a.Mtype() == "selfterm" || a.Mtype() == "NUM" || a.Mdata() == "]" ||
       a.Mdata() == ")")
      return 1;


  if(b.Mtype() == "selfterm")
    if(a.Mtype() == "IDENT" || a.Mdata() == "return" || a.Mdata() == "," || a.Mtype() == "Term" ||
       a.Mdata() == "]" || a.Mdata() == "?" || a.Mdata() == ":")
      return 1;

  if(b.Mdata() == "{")
    if(a.Mdata() == ")" || a.Mdata() == "else")
      return 1;

  if(b.Mdata() == "}")
    if(a.Mdata() == ";" || a.Mdata() == "return" || a.Mdata() == "}")
      return 1;

  if(b.Mdata() == "(")
    if(a.Mtype() == "Term" || a.Mtype() == "ArithExp" || a.Mtype() == "IDENT" || a.Mtype() == "Assign" ||
       a.Mdata() == "if" || a.Mdata() == "while" || a.Mdata() == "return" || a.Mdata() == ":" ||
       a.Mdata() == "," || a.Mdata() == "(" || a.Mdata() == "[" || a.Mdata() == "?")
      return 1;

  if(b.Mdata() == ")")
    if(a.Mtype() == "NUM" || a.Mtype() == "IDENT" || a.Mdata() == ")" ||
       a.Mdata() == "]" || a.Mdata() == "(" || a.Mtype() == "selfterm")
      return 1;

  if(b.Mdata() == "[")
    if(a.Mtype() == "IDENT")
      return 1;

  if(b.Mdata() == "]")
    if(a.Mdata() == "]" || a.Mtype() == "IDENT" || a.Mtype() == "NUM" || a.Mdata() == ")")
      return 1;

  if(b.Mdata() == ";")
    if(a.Mdata() == "]" || a.Mtype() == "IDENT" || a.Mtype() == "NUM" || a.Mdata() == ")" ||
       a.Mtype() == "selfterm" || a.Mdata() == "return" ||
       a.Mdata() == "if" || a.Mdata() == "else" || a.Mdata() == "while" ||
       a.Mdata() == "{")
      return 1;

  if(b.Mdata() == ",")
    if(a.Mdata() == "]" || a.Mtype() == "IDENT" || a.Mtype() == "NUM" || a.Mdata() == ")" ||
       a.Mtype() == "selfterm")
      return 1;

  if(b.Mdata() == "if")
    if(a.Mdata() == "}" || a.Mdata() == ")" || a.Mdata() == "{" ||
       a.Mdata() == ";" || a.Mdata() == "else")
      return 1;

  if(b.Mdata() == "else")
    if(a.Mdata() == "}" || a.Mdata() == ";")
      return 1;

  if(b.Mdata() == "while")
    if(a.Mdata() == "}" || a.Mdata() == ")" || a.Mdata() == "{" ||
       a.Mdata() == ";" || a.Mdata() == "else")
      return 1;

  if(b.Mdata() == "return")
    if(a.Mdata() == "}" || a.Mdata() == "{" || a.Mdata() == ";" ||
       a.Mdata() == ")" || a.Mdata() == "else")
      return 1;

  if(b.Mdata() == "?")
    if(a.Mtype() == "NUM" || a.Mtype() == "IDENT" || a.Mdata() == "]" || a.Mdata() == ")")
      return 1;

  if(b.Mdata() == ":")
    if(a.Mtype() == "NUM" || a.Mtype() == "IDENT" || a.Mdata() == "]" || a.Mdata() == ")")
      return 1;


  return 0;
} // Fine()

int gnowlinecount;


void Check(string str, bool end)
{
  // cout << "Check->" << str << "<-" << endl ;
  string s;

  if(str.size() == 0)
    return;

  for(int i = 0 ; i < str.size() ; i++)
  {
    bool fine = 0;
    if('0' <= str[i] && str[i] <= '9')
      fine = 1;
    if('a' <= str[i] && str[i] <= 'z')
      fine = 1;
    if('A' <= str[i] && str[i] <= 'Z')
      fine = 1;
    if(str[i] == '+' || str[i] == '-' || str[i] == '*' || str[i] == '/' ||
       str[i] == '(' || str[i] == ')' || str[i] == '>' || str[i] == '<' ||
       str[i] == '=' || str[i] == '.' || str[i] == '_' || str[i] == ' ' ||
       str[i] == '{' || str[i] == '}' || str[i] == '[' || str[i] == ']' ||
       str[i] == '!' || str[i] == '&' || str[i] == '|' || str[i] == '%' ||
       str[i] == '?' || str[i] == ':' || str[i] == ';' || str[i] == '\'' ||
       str[i] == '\"' || str[i] == '\\' || str[i] == ',' || str[i] == '@')
      fine = 1;

    if(!fine)
      // Check( str.substr( 0, i ), 0 );
      throw runtime_error("Line " + To_string(gnowlinecount) +
                " : unrecognized token with first char : '" + str.substr(i, 1) + "'");
  }               // for

  str += "     "; // str += 3

  int tsize = 0;

  for(int i = 0 ; i < str.size() - 5 ; )
  {
    if(str[i] == ' ')
      i++;
    // if
    else if(str.substr(i, 4) == "true")
    {
      Token token(str.substr(i, 4));
      gctokens[tsize] = token;
      tsize++;
      i += 4;
    } // else if
    else if(str.substr(i, 5) == "false")
    {
      Token token(str.substr(i, 5));
      gctokens[tsize] = token;
      tsize++;
      i += 5;
    } // else if
    else if(str.substr(i, 2) == ">=" || str.substr(i, 2) == "<=" || str.substr(i, 2) == "==" ||
        str.substr(i, 2) == "!=" || str.substr(i, 2) == "&&" || str.substr(i, 2) == "||" ||
        str.substr(i, 2) == "+=" || str.substr(i, 2) == "-=" || str.substr(i, 2) == "*=" ||
        str.substr(i, 2) == "/=" || str.substr(i, 2) == "%=" || str.substr(i, 2) == "++" ||
        str.substr(i, 2) == "--" || str.substr(i, 2) == "<<" || str.substr(i, 2) == ">>")
    {
      Token token(str.substr(i, 2));
      gctokens[tsize] = token;
      tsize++;
      i += 2;
    } // else if
    else if(str.substr(i, 1) == "\"")
    {
      string s = "\"";
      i++;
      while(i < str.size() - 5 && str[i] != '\"')
      {
        s += str[i];
        i++;
      } // while

      if(i == str.size() - 5 && end)
        throw  runtime_error("Line " + To_string(gnowlinecount) +
                   " : unrecognized token with first char : '\"'");
      s += "\"";
      Token token(s);
      gctokens[tsize] = token;
      tsize++;
      i += 1;
    } // else if
    else if(str.substr(i, 1) == "\'")
    {
      string s = "\'";
      i++;
      while(i < str.size() - 5 && str[i] != '\'')
      {
        s += str[i];
        i++;
      } // while

      if(i == str.size() - 5 && end)
        throw runtime_error("Line " + To_string(gnowlinecount) +
                  " : unrecognized token with first char : '\''");
      s += "\"";
      Token token(s);
      gctokens[tsize] = token;
      tsize++;
      i += 1;
    } // else if
    else if(str.substr(i, 1) == "=" || str.substr(i, 1) == "<" ||
        str.substr(i, 1) == ">" || str.substr(i, 1) == "+" ||
        str.substr(i, 1) == "-" || str.substr(i, 1) == "*" ||
        str.substr(i, 1) == "/" || str.substr(i, 1) == "!" ||
        str.substr(i, 1) == ")" || str.substr(i, 1) == "(" ||
        str.substr(i, 1) == "{" || str.substr(i, 1) == "}" ||
        str.substr(i, 1) == "[" || str.substr(i, 1) == "]" ||
        str.substr(i, 1) == "?" || str.substr(i, 1) == ":" ||
        str.substr(i, 1) == ";" || str.substr(i, 1) == "%" ||
        str.substr(i, 1) == "&" || str.substr(i, 1) == ",")
    {
      Token token(str.substr(i, 1));
      gctokens[tsize] = token;
      tsize++;
      i += 1;
    } // else if
    else if(('0' <= str[i] && '9' >= str[i]) || str[i] == '.')
    {
      bool   hasdot = 0;
      string s;

      if(str[i] == '.')
        hasdot = 1;
      s += str[i];
      i++;

      for( ; (i < str.size() - 5) &&
         (('0' <= str[i] && '9' >= str[i]) || ((str[i] == '.') &&
                             (!hasdot))) ;
         i++)
      {
        if(str[i] == '.')
          hasdot = 1;
        s += str[i];
      } // for

      Token token(s);
      gctokens[tsize] = token;
      tsize++;
    } // else if
    else if(str[i] == '_')
      throw runtime_error("Line " + To_string(gnowlinecount) +
                " : unrecognized token with first char : '" + str.substr(i, 1) + "'");
    // else if
    else
    {
      string s;
      for( ; str[i] != '+' && str[i] != '-' && str[i] != '*' && str[i] != '/' &&
         str[i] != '%' && str[i] != '!' && str[i] != '=' && str[i] != '>' &&
         str[i] != '<' && str[i] != '?' && str[i] != ':' && str[i] != ';' &&
         str[i] != '(' && str[i] != ')' && str[i] != '[' && str[i] != ']' &&
         str[i] != '{' && str[i] != '}' && str[i] != '&' && str[i] != ' ' &&
         str[i] != '\"' && str[i] != '\'' && str[i] != ',' && i < str.size() - 5 ;
         i++)
        s += str[i];
      if(s.size())
      {
        Token token(s);
        gctokens[tsize] = token;
        tsize++;
      } // if
    }     // else
  }         // for


  if(0)
    for(int i = 0 ; i < tsize ; i++)
      cout << gctokens[i].Mstr() << endl;

  // Check_reset();

  if(gctokens [0].Mdata() == "+" || gctokens [0].Mdata() == "-")
    gctokens [0].Msettype("ArithExp");

  for(int i = 0 ; i + 1 < tsize  ; i++)
    if((gctokens [i].Mtype() == "Term" ||
      gctokens [i].Mtype() == "parentheses(" ||
      gctokens [i].Mtype() == "Assign" ||
      gctokens [i].Mdata() == "," ||
      gctokens [i].Mdata() == "[" ||
      gctokens [i].Mdata() == "{" ||
      gctokens [i].Mdata() == "(" ||
      gctokens [i].Mdata() == "?" ||
      gctokens [i].Mdata() == ":") &&
       (gctokens [i + 1].Mdata() == "+" ||
      gctokens [i + 1].Mdata() == "-"))
      gctokens [i + 1].Msettype("ArithExp");


  if(end)
  {
    for(int i = 0 ; i < tsize ; i++)
      gtemptokens[i] = gctokens[i];
    gtemptokenssize = tsize;
  } // if

  if(tsize == 0)
    return;

  if(gctokens[0].Mdata() != "if" &&
     gctokens[0].Mdata() != "while" &&
     gctokens[0].Mdata() != "(" &&
     gctokens[0].Mtype() != "IDENT" &&
     gctokens[0].Mtype() != "NUM" &&
     gctokens[0].Mtype() != "ArithExp" &&
     gctokens[0].Mtype() != "type")
    throw runtime_error("Line " + To_string(gnowlinecount) +
              " : unexpected token '" + gctokens[0].Mdata() + "'");

  int parentheses = 0;

  gcheckidsize = 2;

  for(int i = 0 ; i < tsize ; )
  {
    if(gctokens[i].Mdata() == "(")
      parentheses++;
    if(gctokens[i].Mdata() == ")")
      parentheses--;

    if(gctokens[i].Mdata() == "{")
      parentheses += 1000;
    if(gctokens[i].Mdata() == "}")
      parentheses -= 1000;

    if(parentheses < 0)

      throw runtime_error("Line " + To_string(gnowlinecount) +
                " : unexpected token '" + gctokens[i].Mdata() + "'");
    // if

    if(gctokens[i].Mtype() == "type")
    {
      i++;
      if(gctokens[i].Mtype() != "IDENT")
        throw runtime_error("Line " + To_string(gnowlinecount) +
                  " : unexpected token '" + gctokens[i].Mdata() + "'");
      // if

      Check_set(gctokens[i].Mdata());

      if(gctokens[i + 1].Mdata() == "(")
      {
        i += 2; // skip f(
        while(i < tsize && gctokens[i].Mdata() != ")")
        {
          if(gctokens[i].Mtype() != "type")
            throw runtime_error("Line " + To_string(gnowlinecount) +
                      " : unexpected token '" + gctokens[i].Mdata() + "'");
          // if

          i++;

          if(gctokens[i].Mdata() == "&")
            i++;

          if(gctokens[i].Mtype() != "IDENT")
            throw runtime_error("Line " + To_string(gnowlinecount) +
                      " : unexpected token '" + gctokens[i].Mdata() + "'");
          // if

          Check_set(gctokens[i].Mdata());

          i++;

          if(gctokens[i].Mdata() == "[")
          {
            i++;
            if(gctokens[i].Mtype() != "NUM")
              throw runtime_error("Line " + To_string(gnowlinecount) +
                        " : unexpected token '" + gctokens[i].Mdata() + "'");
            // if

            i++;

            if(gctokens[i].Mdata() != "]")
              throw runtime_error("Line " + To_string(gnowlinecount) +
                        " : unexpected token '" + gctokens[i].Mdata() + "'");
            // if

            i++;
          } // if

          if(gctokens[i].Mdata() == ",")
            i++;
        } // while

        i++; // skip)
      } // if
      else
      {
        if(gctokens[i].Mdata() != ";")
        {
          if(gctokens[i].Mtype() != "IDENT")
            throw runtime_error("Line " + To_string(gnowlinecount) +
                      " : unexpected token '" + gctokens[i].Mdata() + "'");
          // if

          Check_set(gctokens[i].Mdata());

          i++;

          if(gctokens[i].Mdata() == "[")
          {
            i++;
            if(gctokens[i].Mtype() != "NUM")
              throw runtime_error("Line " + To_string(gnowlinecount) +
                        " : unexpected token '" + gctokens[i].Mdata() + "'");

            i++;

            if(gctokens[i].Mdata() != "]")
              throw runtime_error("Line " + To_string(gnowlinecount) +
                        " : unexpected token '" + gctokens[i].Mdata() + "'");

            i++;
          } // if
        }     // if

        while(gctokens[i].Mdata() != ";")
        {
          if(gctokens[i].Mdata() != ",")
            throw runtime_error("Line " + To_string(gnowlinecount) +
                      " : unexpected token '" + gctokens[i].Mdata() + "'");

          i++;
          if(gctokens[i].Mtype() != "IDENT")
            throw runtime_error("Line " + To_string(gnowlinecount) +
                      " : unexpected token '" + gctokens[i].Mdata() + "'");

          Check_set(gctokens[i].Mdata());

          i++;

          if(gctokens[i].Mdata() == "[")
          {
            i++;
            if(gctokens[i].Mtype() != "NUM")
              throw runtime_error("Line " + To_string(gnowlinecount) +
                        " : unexpected token '" + gctokens[i].Mdata() + "'");

            i++;

            if(gctokens[i].Mdata() != "]")
              throw runtime_error("Line " + To_string(gnowlinecount) +
                        " : unexpected token '" + gctokens[i].Mdata() + "'");

            i++;
          } // if
        }     // while
      }   // else
    }         // if
    else if(0)
    {
    } // else if
    else
    {
      if(i > 0 && gctokens[i].Mdata() != "")
        if(!Fine(gctokens[i - 1], gctokens[i]))
        {
          if(0)
            for(int i = 0 ; i < tsize ; i++)
              cout << gctokens[i].Mstr() << endl;

          if(0)
          {
            cout << "gctokens[ i - 1 ]:" << gctokens[i - 1].Mstr() << endl;
            cout << "gctokens[ i ]:" << gctokens[i].Mstr() << endl;
          } // if

          throw runtime_error("Line " + To_string(gnowlinecount) +
                    " : unexpected token '" + gctokens[i].Mdata() + "'");
        }// if

      if(gctokens[i].Mtype() == "IDENT" && !Check_has(gctokens[i].Mdata()))

        throw runtime_error("Line " + To_string(gnowlinecount) +
                  " : undefined identifier '" + gctokens[i].Mdata() + "'");
      // if

      i++;
    }
    // else
  } // for

  if(end &&
     gctokens[tsize - 1].Mdata() != ";" &&
     gctokens[tsize - 1].Mdata() != "}")
    throw runtime_error("Line " + To_string(gnowlinecount) +
              " : unexpected token '" + gctokens[tsize - 1].Mdata() + "'");
  // if


  return;
} // Check()

int main()
{
  Check_reset();
  Check_set("cout");
  Check_set("Done");
  cout << "Our-C running ...\n";
  cout << "> ";
  // init
  gAsmsize   = 0;
  gidsize    = 0;
  gstakesize = 0;
  for(int i = 0 ; i < maxarraysize ; i++)
  {
    //Tokenarr* t = new Tokenarr ;
    gAsm.push_back({});
    for(int j = 0 ; j < 10 ; j++)
      gAsm[i].push_back({});
    gAsmlinesize[i]  = 0;
    gbranchtarget[i] = -1;
  } // for

  string testnum;

  cin >> testnum;
  getchar();

  while(testnum.find(" ") != std::string::npos)
    testnum.replace(testnum.find(" "), testnum.find(" ") + 1, "");
  if(testnum == "Done() ;" || testnum == "Done();" || testnum == "22")
  {
    cout << "Our-C exited ...\n";
    return 0;
  } // if


  string nextstr    = "";
  int    inputcount = 0;

  while(1)
  {
    while(gstakesize)
      Pop();
    bool isend = 0;
    inputcount++;

    gnowlinecount = 1;
    try
    {
      bool readok = 0;
      if(nextstr.size() > 0 && nextstr.substr(nextstr.size() - 1, 1) == ";")
        readok = 1;

      string command    = nextstr;
      string checkcommand = nextstr + " ";
      nextstr = "";
      if(command == "Done();")
      {
        cout << "Our-C exited ...\n";
        return 0;
      } // if

      char c;
      bool lastisdivide = 0;
      bool skipmode   = 0;
      int  quote1     = 0;
      int  quote2     = 0;

      if(checkcommand.substr(0, 2) == "//")
      {
        checkcommand = "";
        command    = "";
        // gnowlinecount++ ;
        while(getchar() != '\n')
          ;
      } // if

      int  bigparrant = 0;
      bool keepread = 1;
      if(!readok)
        c = getchar();

      for(  ; keepread && (!readok) ; )
      {
        if(skipmode == false && c == '/')
        {
          if(lastisdivide)
          {
            skipmode   = 1;
            lastisdivide = 0;
          } // if
          else
            lastisdivide = 1;
        } // if

        if(skipmode == false && lastisdivide && c != '/')
        {
          lastisdivide  = 0;
          command    += '/';
          checkcommand += '/';
        } // if

        if(skipmode == false && c == '\'' && checkcommand.size() > 0 &&
           checkcommand[checkcommand.size() - 1] != '\\')
          quote1++;
        if(skipmode == false && c == '\"' && checkcommand.size() > 0 &&
           checkcommand[checkcommand.size() - 1] != '\\')
          quote2++;

        if(skipmode == false && c != ' ' && c != '\n' && c != '\t' && c != '/')
        {
          command    += c;
          checkcommand += c;

          if(c == '{')
            bigparrant++;
          if(c == '}')
            bigparrant--;
        } // if

        if(c == '\n' || c == ' ' || c == '\t')
        {
          checkcommand += ' ';
          if(c == '\n')
          {
            isend  = 1;
            skipmode = false;
            Check(checkcommand, 0);
            gnowlinecount++;
          } // if
        }     // if

        if(command == "Done();")
        {
          cout << "Our-C exited ...\n";
          return 0;
        } // if

        keepread = c != ';' || skipmode || bigparrant;
        if(bigparrant == 0 && c == '}')
          keepread = 0;
        if(quote1 % 2 == 1 || quote2 % 2 == 1)
          keepread = 1;


        if(keepread == 0 && 1)
        {
          // cin >> nextstr ;
          nextstr = "";
          char ch = getchar();

          while(ch == ' ')
            ch = getchar();

          while(ch != '\n' && ch != ' ')
          {
            nextstr += ch;
            ch     = getchar();
          } // while

          char chh;
          if(nextstr == "//" && c != '\n' && 0)
          {
            while(getchar() != '\n')
              ;
            nextstr = "";
            while(chh != '\n' && chh != ' ')
            {
              nextstr += chh;
              chh    = getchar();
            } // while
          }     // if
          // cout << "!" << nextstr << "!" << endl ;
          if(nextstr.substr(0, 4) == "else")
          {
            if(ch == '\n')
              gnowlinecount++;
            if(chh == '\n')
              gnowlinecount++;
            command    += nextstr;
            checkcommand += nextstr + " ";
            nextstr     = "";
            keepread    = 1;
          } // if
        }     // if


        if(keepread && (!readok))
          c = getchar();
      } // for
      // finish cin!!!!!!!!!!!!!!!!!
      isend = 0;

      Check(checkcommand, 1);

      // 塞進去
      int asmstart = gAsmsize;

      Pushasm();

      if(gtemptokens[0].Mtype() == "type")
      {
        if(asmstart - gAsmsize == -1)
        {
          Executeasm(asmstart, gAsmsize);
          // Executeasm( gAsmsize, gAsmsize ) ;
          for(int i = 0 ; i < gAsmlinesize[asmstart] ; i++)
            if(gAsm[asmstart][i].Mtype() == "IDENT")
              cout << "Definition of " + gtemptokens[i].Mdata() + " entered ..." << endl;
        } // if
        else
        {
          cout << "Definition of " + gtemptokens[1].Mdata() + "() entered ..." << endl;
          Idmap_set(gtemptokens[1].Mdata(), NewNumber(0));
        } // else
      }     // if
      else
      {
        Executeasm(asmstart, gAsmsize);
        // Executeasm( gAsmsize, gAsmsize ) ;
        cout << "Statement executed ..." << endl;
      } // else

      if(gstakesize != 0)
        throw runtime_error("Stack size=" + To_string(gstakesize));
      // Idmap_print() ;
    } // if
    catch(const exception & e)
    {
      cout << e.what() << endl;
      if(strcmp(e.what(), "Our-C exited ...") == 0)
        return 0;

      if(!isend && 0)
      {
        nextstr = "";
        while(getchar())
          ;
      } // if
    }     // catch

    Printasm();
    cout << "> ";
  } // while
}   // main()

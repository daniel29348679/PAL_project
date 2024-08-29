# include <cstdlib>
# include <stdio.h>
# include <iostream>
# include <vector>
# include <string>
# include <sstream>
# include <map>
# include <stdexcept>
# include <iomanip>
# include <cstring>
using namespace std;

bool Allalphabet( string str )
{
  if ( str.size() == 0 )
    return 0;

  char c = str[ 0 ];
  if ( ! ( 'a' <= c && c <= 'z' ) || ( 'A' <= c && c <= 'Z' ) )
    return 0;

  // for(const auto & c:str)
  for ( int i = 1 ; i < str.size() ; i++ )
  {
    c = str[ i ];
    if ( ! ( ( 'a' <= c && c <= 'z' ) || ( 'A' <= c && c <= 'Z' ) ||
             ( '0' <= c && c <= '9' ) || c == '_' ) )
      return 0;
  } // for

  return 1;
} // Allalphabet()

bool Allnum( string str )
{
  if ( str.size() == 0 )
    return 0;

  int dot = 0;


  for ( int i = 0 ; i < str.size() ; i++ )
  {
    char c = str[ i ];
    if ( ! ( ( '0' <= c && c <= '9' ) || c == '.' || c == '-' || c == '+' ) )
      return 0;

    if ( c == '.' )
      dot++;
    if ( ( c == '-' || c == '+' ) && i != 0 )
      return 0;

    if ( dot > 1 )
      return 0;
  } // for

  return 1;
} // Allnum()

bool Iscompare( string str )
{
  for ( int i = 0 ; i < str.size() ; i++ )
  {
    char c = str[ i ];
    if ( c == '=' || c == '<' || c == '>' )
      return 1;
  } // for

  return 0;
} // Iscompare()

class Token
{
public:
  string mdata;
  string mtype;
  Token()
  {
  }       // Token()

  Token( string str )
  {
    if ( str == ":=" )
    {
      mdata = str;
      mtype = "Assign";
      return;
    } // if

    if ( str == "=" || str == "<>" || str == ">" || str == "<" || str == ">=" || str == "<=" )
    {
      mdata = str;
      mtype = "BooleanExp";
      return;
    } // if

    if ( str == "+" || str == "-" )
    {
      mdata = str;
      mtype = "Term";
      return;
    } // if

    if ( str == "*" || str == "/" )
    {
      mdata = str;
      mtype = "Term";
      return;
    } // if

    if ( str == "(" )
    {
      mdata = str;
      mtype = "parentheses(";
      return;
    } // if

    if ( str == ")" )
    {
      mdata = str;
      mtype = "parentheses)";
      return;
    } // if

    if ( Allnum( str ) )
    {
      mdata = str;
      mtype = "NUM";
      return;
    } // if

    mdata = str;
    mtype = "IDENT";
    return;

    throw runtime_error( "???fuck up token???" );
  } // Token()

  bool Mequal( string str )
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

  void Msettype( string str )
  {
    mtype = str;
  } // Msettype()

  string Mdata()
  {
    return mdata;
  } // Mdata()
}; // Token


int Typetoi( string str )
{
  if ( str == "BooleanExp"    )
    return 0;

  if ( str == "ArithExp" )
    return 1;

  if ( str == "Term" )
    return 2;

  if ( str == "parentheses("  )
    return 3;

  if ( str == "parentheses)"  )
    return 4;

  if ( str == "NUM"           )
    return 5;

  if ( str == "IDENT"         )
    return 6;

  if ( str == "Assign"        )
    return 7;

  return -1;
} // Typetoi()

// map<string, map<string, bool> > gacmap;
typedef bool Boolarray[ 10 ];
Boolarray gacmap[ 10 ];
void Buildgacmap()
{
  gacmap[ Typetoi( "BooleanExp" ) ][ Typetoi( "BooleanExp" ) ]   = 0;   // <,>,=
  gacmap[ Typetoi( "BooleanExp" ) ][ Typetoi( "ArithExp" ) ]     = 1;   // +,-
  gacmap[ Typetoi( "BooleanExp" ) ][ Typetoi( "Term" ) ]         = 0;   // +,-,*,/
  gacmap[ Typetoi( "BooleanExp" ) ][ Typetoi( "parentheses(" ) ] = 1;   // (
  gacmap[ Typetoi( "BooleanExp" ) ][ Typetoi( "parentheses)" ) ] = 0;   // )
  gacmap[ Typetoi( "BooleanExp" ) ][ Typetoi( "NUM" ) ]          = 1;   // 123
  gacmap[ Typetoi( "BooleanExp" ) ][ Typetoi( "IDENT" ) ]        = 1;   // ab
  gacmap[ Typetoi( "BooleanExp" ) ][ Typetoi( "Assign" ) ]       = 0;   // :=

  gacmap[ Typetoi( "ArithExp" ) ][ Typetoi( "BooleanExp" ) ]   = 0;     // <,>,=
  gacmap[ Typetoi( "ArithExp" ) ][ Typetoi( "ArithExp" ) ]     = 0;     // +,-,*,/
  gacmap[ Typetoi( "ArithExp" ) ][ Typetoi( "Term" ) ]         = 0;     // */
  gacmap[ Typetoi( "ArithExp" ) ][ Typetoi( "parentheses(" ) ] = 0;     // (
  gacmap[ Typetoi( "ArithExp" ) ][ Typetoi( "parentheses)" ) ] = 0;     // )
  gacmap[ Typetoi( "ArithExp" ) ][ Typetoi( "NUM" ) ]          = 1;     // 123
  gacmap[ Typetoi( "ArithExp" ) ][ Typetoi( "IDENT" ) ]        = 0;     // ab
  gacmap[ Typetoi( "ArithExp" ) ][ Typetoi( "Assign" ) ]       = 0;     // :=

  gacmap[ Typetoi( "Term" ) ][ Typetoi( "BooleanExp" ) ]   = 0;         // <,>,=
  gacmap[ Typetoi( "Term" ) ][ Typetoi( "ArithExp" ) ]     = 1;         // +,-
  gacmap[ Typetoi( "Term" ) ][ Typetoi( "Term" ) ]         = 0;         // +,-,*,/
  gacmap[ Typetoi( "Term" ) ][ Typetoi( "parentheses(" ) ] = 1;         // (
  gacmap[ Typetoi( "Term" ) ][ Typetoi( "parentheses)" ) ] = 0;         // )
  gacmap[ Typetoi( "Term" ) ][ Typetoi( "NUM" ) ]          = 1;         // 123
  gacmap[ Typetoi( "Term" ) ][ Typetoi( "IDENT" ) ]        = 1;         // ab
  gacmap[ Typetoi( "Term" ) ][ Typetoi( "Assign" ) ]       = 0;         // :=

  gacmap[ Typetoi( "parentheses(" ) ][ Typetoi( "BooleanExp" ) ]   = 0; // <,>,=
  gacmap[ Typetoi( "parentheses(" ) ][ Typetoi( "ArithExp" ) ]     = 1; // +,-
  gacmap[ Typetoi( "parentheses(" ) ][ Typetoi( "Term" ) ]         = 0; // +,-,*,/
  gacmap[ Typetoi( "parentheses(" ) ][ Typetoi( "parentheses(" ) ] = 1; // (
  gacmap[ Typetoi( "parentheses(" ) ][ Typetoi( "parentheses)" ) ] = 0; // )
  gacmap[ Typetoi( "parentheses(" ) ][ Typetoi( "NUM" ) ]          = 1; // 123
  gacmap[ Typetoi( "parentheses(" ) ][ Typetoi( "IDENT" ) ]        = 1; // ab
  gacmap[ Typetoi( "parentheses(" ) ][ Typetoi( "Assign" ) ]       = 0; // :=

  gacmap[ Typetoi( "parentheses)" ) ][ Typetoi( "BooleanExp" ) ]   = 1; // <,>,=
  gacmap[ Typetoi( "parentheses)" ) ][ Typetoi( "ArithExp" ) ]     = 0; // +,-
  gacmap[ Typetoi( "parentheses)" ) ][ Typetoi( "Term" ) ]         = 1; // +,-,*,/
  gacmap[ Typetoi( "parentheses)" ) ][ Typetoi( "parentheses(" ) ] = 0; // (
  gacmap[ Typetoi( "parentheses)" ) ][ Typetoi( "parentheses)" ) ] = 1; // )
  gacmap[ Typetoi( "parentheses)" ) ][ Typetoi( "NUM" ) ]          = 0; // 123
  gacmap[ Typetoi( "parentheses)" ) ][ Typetoi( "IDENT" ) ]        = 0; // ab
  gacmap[ Typetoi( "parentheses)" ) ][ Typetoi( "Assign" ) ]       = 0; // :=

  gacmap[ Typetoi( "NUM" ) ][ Typetoi( "BooleanExp" ) ]   = 1;          // <,>,=
  gacmap[ Typetoi( "NUM" ) ][ Typetoi( "ArithExp" ) ]     = 0;          // +,-
  gacmap[ Typetoi( "NUM" ) ][ Typetoi( "Term" ) ]         = 1;          // +,-,*,/
  gacmap[ Typetoi( "NUM" ) ][ Typetoi( "parentheses(" ) ] = 0;          // (
  gacmap[ Typetoi( "NUM" ) ][ Typetoi( "parentheses)" ) ] = 1;          // )
  gacmap[ Typetoi( "NUM" ) ][ Typetoi( "NUM" ) ]          = 0;          // 123
  gacmap[ Typetoi( "NUM" ) ][ Typetoi( "IDENT" ) ]        = 0;          // ab
  gacmap[ Typetoi( "NUM" ) ][ Typetoi( "Assign" ) ]       = 0;          // :=

  gacmap[ Typetoi( "IDENT" ) ][ Typetoi( "BooleanExp" ) ]   = 1;        // <,>,=
  gacmap[ Typetoi( "IDENT" ) ][ Typetoi( "ArithExp" ) ]     = 0;        // +,-
  gacmap[ Typetoi( "IDENT" ) ][ Typetoi( "Term" ) ]         = 1;        // +,-,*,/
  gacmap[ Typetoi( "IDENT" ) ][ Typetoi( "parentheses(" ) ] = 0;        // (
  gacmap[ Typetoi( "IDENT" ) ][ Typetoi( "parentheses)" ) ] = 1;        // )
  gacmap[ Typetoi( "IDENT" ) ][ Typetoi( "NUM" ) ]          = 0;        // 123
  gacmap[ Typetoi( "IDENT" ) ][ Typetoi( "IDENT" ) ]        = 0;        // ab
  gacmap[ Typetoi( "IDENT" ) ][ Typetoi( "Assign" ) ]       = 1;        // :=

  gacmap[ Typetoi( "Assign" ) ][ Typetoi( "BooleanExp" ) ]   = 0;       // <,>,=
  gacmap[ Typetoi( "Assign" ) ][ Typetoi( "ArithExp" ) ]     = 1;       // +,-
  gacmap[ Typetoi( "Assign" ) ][ Typetoi( "Term" ) ]         = 0;       // +,-,*,/
  gacmap[ Typetoi( "Assign" ) ][ Typetoi( "parentheses(" ) ] = 1;       // (
  gacmap[ Typetoi( "Assign" ) ][ Typetoi( "parentheses)" ) ] = 0;       // )
  gacmap[ Typetoi( "Assign" ) ][ Typetoi( "NUM" ) ]          = 1;       // 123
  gacmap[ Typetoi( "Assign" ) ][ Typetoi( "IDENT" ) ]        = 1;       // ab
  gacmap[ Typetoi( "Assign" ) ][ Typetoi( "Assign" ) ]       = 0;       // :=

  return;
} // Buildgacmap()

class Number
{
public:
  int mtype;
  int min;
  float mfl;
  bool mbo;

  Number()
  {
    return;
  } // Number()

  Number( int i )
  {
    min   = i;
    mfl   = 0;
    mbo   = 0;
    mtype = 0;
  } // Number()

  Number( float i )
  {
    min   = 0;
    mfl   = i;
    mbo   = 0;
    mtype = 1;
  } // Number()

  Number( bool i )
  {
    min   = 0;
    mfl   = 0;
    mbo   = i;
    mtype = 2;
  } // Number()

  Number( string str )
  {
    min   = 0;
    mfl   = 0;
    mbo   = 0;
    mtype = 0;

    int dotcount = 0;
    for ( int i = 0 ; i < str.size() ; i++ )
      if ( str[ i ] == '.' )
        dotcount++;
    if ( dotcount > 1 )
      throw runtime_error( "more than one dot" + str );
    if ( dotcount )
    {
      mfl   = atof( str.c_str() );
      mtype = 1;
    } // if
    else
    {
      min   = atoi( str.c_str() );
      mtype = 0;
    } // else
  }   // Number()

  Number Madd( const Number&n )
  {
    Number newn( 0 );

    if ( mtype == 0 && n.mtype == 0 )
    {
      newn.mtype = 0;
      newn.min   = min + n.min;
    } // if
    else
    {
      float f = 0;
      if ( mtype == 0 )
        f += min;
      else
        f += mfl;

      if ( n.mtype == 0 )
        f += n.min;
      else
        f += n.mfl;

      newn.mtype = 1;
      newn.mfl   = f;
    } // else

    return newn;
  } // Madd()

  Number Mminus( const Number&n )
  {
    Number newn( 0 );

    if ( mtype == 0 && n.mtype == 0 )
    {
      newn.mtype = 0;
      newn.min   = min - n.min;
    } // if
    else
    {
      float f = 0;
      if ( mtype == 0 )
        f += min;
      else
        f += mfl;

      if ( n.mtype == 0 )
        f -= n.min;
      else
        f -= n.mfl;

      newn.mtype = 1;
      newn.mfl   = f;
    } // else

    return newn;
  } // Mminus()

  Number Mmul( const Number&n )
  {
    Number newn( 0 );

    if ( mtype == 0 && n.mtype == 0 )
    {
      newn.mtype = 0;
      newn.min   = min * n.min;
    } // if
    else
    {
      float f = 1;
      if ( mtype == 0 )
        f *= min;
      else
        f *= mfl;

      if ( n.mtype == 0 )
        f *= n.min;
      else
        f *= n.mfl;

      newn.mtype = 1;
      newn.mfl   = f;
    } // else

    return newn;
  } // Mmul()

  Number Mdiv( const Number&n )
  {
    Number newn( 0 );

    if ( mtype == 0 && n.mtype == 0  ) // min % n.min == 0
    {
      newn.mtype = 0;
      newn.min   = min / n.min;
    } // if
    else
    {
      float f = 1;
      if ( mtype == 0 )
        f *= min;
      else
        f *= mfl;

      if ( n.mtype == 0 )
        f /= n.min;
      else
        f /= n.mfl;

      newn.mtype = 1;
      newn.mfl   = f;
    } // else

    return newn;
  } // Mdiv()

  Number Mequal( const Number&n )
  {
    Number newn( false );

    // cout << "##" << mfl << endl;

    if ( mtype != 2 )
      newn.mbo = ( min + mfl ) - ( n.min + n.mfl ) < 0.0001 &&
                 ( min + mfl ) - ( n.min + n.mfl ) > -0.0001;
    else
      newn.mbo = mbo == n.mbo;

    return newn;
  } // Mequal()

  Number Mnotequal( const Number&n )
  {
    Number newn( false );

    if ( mtype != 2 )
      newn.mbo = ! ( ( min + mfl ) - ( n.min + n.mfl ) < 0.0001 &&
                     ( min + mfl ) - ( n.min + n.mfl ) > -0.0001 );
    else
      newn.mbo = ( mbo != n.mbo );

    return newn;
  } // Mnotequal()

  Number Mless( const Number&n )
  {
    Number newn( false );

    newn.mbo = min + mfl < n.min + n.mfl;

    return newn;
  } // Mless()

  Number Mgrater( const Number&n )
  {
    Number newn( false );

    newn.mbo = min + mfl > n.min + n.mfl;

    return newn;
  } // Mgrater()

  Number Mlessequal( const Number&n )
  {
    Number newn( false );
    bool   equ = ( min + mfl ) - ( n.min + n.mfl ) < 0.0001 &&
                 ( min + mfl ) - ( n.min + n.mfl ) > -0.0001;

    newn.mbo = ( min + mfl <= n.min + n.mfl ) || equ;

    return newn;
  } // Mlessequal()

  Number Mgraterequal( const Number&n )
  {
    Number newn( false );
    bool   equ = ( min + mfl ) - ( n.min + n.mfl ) < 0.0001 &&
                 ( min + mfl ) - ( n.min + n.mfl ) > -0.0001;

    newn.mbo = ( min + mfl >= n.min + n.mfl ) || equ;
    return newn;
  } // Mgraterequal()

  string Mstr()
  {
    stringstream s;
    char         chars[ 100 ];

    if ( mtype == 0 )
      s << min;
    if ( mtype == 1 )
    {
      s << fixed << setprecision( 3 ) << mfl;
      // sprintf( chars, "%1.3f", mfl );
      // string str = chars;
      // return str;
    } // if

    if ( mtype == 2 )
      s << ( mbo != 0 ? "true" : "false" );
    return s.str();
  } // Mstr()

  string Mdstr()
  {
    stringstream s;

    s << "Number" << min << "-";
    s << fixed << setprecision( 3 ) << mfl << "-";
    s << ( mbo != 0 ? "true" : "false" ) << "-" << mtype << endl;
    return s.str();
  } // Mdstr()

  bool Miszero()
  {
    return ( min == 0 ) && ( mfl == 0 );
  } // Miszero()
};

typedef Token Tokenarr[ 1000 ];

string Tokenstostr( Tokenarr tokens, int si )
{
  string str = "";

  for ( int i = 0 ; i < si ; i++ )
    str += tokens[ i ].Mdata() + " ";

  return str;
} // Tokenstostr()

// map<string, Number> gvar_int_map;
Number gnumbers[ 1000 ];
string gvar[ 1000 ];
int    gsize = 0;

bool Hasthisid( string str )
{
  for ( int i = 0 ; i < gsize ; i++ )
    if ( gvar[ i ] == str )
      return 1;

  return 0;
} // Hasthisid()

void Setvar( string var, Number num )
{
  for ( int i = 0 ; i < gsize ; i++ )
    if ( gvar[ i ] == var )
    {
      gnumbers[ i ] = num;
      return;
    } // if

  gvar[ gsize ]     = var;
  gnumbers[ gsize ] = num;
  gsize++;
} // Setvar()

Number Getvar( string var )
{
  for ( int i = 0 ; i < gsize ; i++ )
    if ( gvar[ i ] == var )
      return gnumbers[ i ];

  // if

  Number n( 0 );
  return n;
} // Getvar()

Number Caculate_float( string str )
{
  // cout << "!!" << str << endl;
  if ( str.size() == 0 )
    throw runtime_error( "Unexpected token : ';'" );
  // cout << str << endl;
  if ( Allnum( str ) )
    return Number( str );

  if ( Allalphabet( str ) )
  {
    if ( Hasthisid( str ) == 0 )
      throw runtime_error( "Undefined identifier : '" + str + "'" );
    return Getvar( str );
  } // if

  int parentheses = 0;

  if ( str[ str.size() - 1 ] == ')' )
    parentheses--;

  for ( int i = str.size() - 2 ; i >= 0 ; i-- )
  {
    if ( str[ i ] == '(' )
      parentheses++;
    if ( str[ i ] == ')' )
      parentheses--;
    if ( parentheses > 0 )
      throw runtime_error( "parentheses error1" );
    if ( parentheses == 0 && i > 0 && str[ i - 1 ] == '<' && str[ i ] == '=' )
      return Caculate_float( str.substr( 0, i - 1 ) ).Mlessequal( Caculate_float( str.substr( i + 1 ) ) );

    if ( parentheses == 0 && i > 0 && str[ i - 1 ] == '>' && str[ i ] == '=' )
      return Caculate_float( str.substr( 0, i - 1 ) ).Mgraterequal( Caculate_float( str.substr( i + 1 ) ) );

    if ( parentheses == 0 && i > 0 && str[ i - 1 ] == '<' && str[ i ] == '>' )
      return Caculate_float( str.substr( 0, i - 1 ) ).Mnotequal( Caculate_float( str.substr( i + 1 ) ) );

    if ( parentheses == 0 && str[ i ] == '>' )
      return Caculate_float( str.substr( 0, i ) ).Mgrater( Caculate_float( str.substr( i + 1 ) ) );

    if ( parentheses == 0 && str[ i ] == '=' )
      return Caculate_float( str.substr( 0, i ) ).Mequal( Caculate_float( str.substr( i + 1 ) ) );

    if ( parentheses == 0 && str[ i ] == '<' )
      return Caculate_float( str.substr( 0, i ) ).Mless( Caculate_float( str.substr( i + 1 ) ) );
  } // for

  if ( parentheses != 0 )
    throw runtime_error( "parentheses error2" );


  for ( int i = str.size() - 1 ; i >= 1 ; i-- )
  {
    if ( str[ i ] == '(' )
      parentheses++;
    if ( str[ i ] == ')' )
      parentheses--;
    if ( parentheses > 0 )
      throw runtime_error( "parentheses error" );
    if ( parentheses == 0 && str[ i ] == '+' && str[ i - 1 ] != '+' &&
         str[ i - 1 ] != '-' && str[ i - 1 ] != '*' && str[ i - 1 ] != '/' )
      return Caculate_float( str.substr( 0, i ) ).Madd( Caculate_float( str.substr( i + 1 ) ) );

    if ( parentheses == 0 && str[ i ] == '-' && str[ i - 1 ] != '+' &&
         str[ i - 1 ] != '-' && str[ i - 1 ] != '*' && str[ i - 1 ] != '/' )
      return Caculate_float( str.substr( 0, i ) ).Mminus( Caculate_float( str.substr( i + 1 ) ) );
  } // for

  if ( str[ 0 ] == '(' )
    parentheses++;

  if ( parentheses != 0 )
    throw runtime_error( "parentheses error2" );

  for ( int i = str.size() - 1 ; i >= 0 ; i-- )
  {
    if ( str[ i ] == '(' )
      parentheses++;
    if ( str[ i ] == ')' )
      parentheses--;
    if ( parentheses == 0 && str[ i ] == '*' )
      return Caculate_float( str.substr( 0, i ) ).Mmul( Caculate_float( str.substr( i + 1 ) ) );

    if ( parentheses == 0 && str[ i ] == '/' )
    {
      Number a = Caculate_float( str.substr( 0, i ) );
      Number b = Caculate_float( str.substr( i + 1 ) );

      // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

      if ( b.Miszero() )
        throw runtime_error( "Error" );

      return a.Mdiv( b );
    } // if
  }   // for

  if ( parentheses != 0 )
    throw runtime_error( "parentheses error2" );

  int minn = 0;

  for ( int i = 1 ; i < str.size() - 1 ; i++ )
  {
    if ( str[ i ] == '(' )
      parentheses++;
    if ( str[ i ] == ')' )
      parentheses--;

    if ( minn > parentheses )
      minn = parentheses;
  } // for

  if ( minn == 0 && str[ 0 ] == '(' && str[ str.size() - 1 ] == ')' )
    return Caculate_float( str.substr( 1, str.size() - 2 ) );

  string err = str.substr( 0, 1 );

  if ( err == "-" )
    err = str.substr( 1, 1 );

  throw runtime_error( "Unrecognized token with first char : '" + err + "'" );
  Number nu( 0 );

  return nu;
} // Caculate_float()

void Check( string str, bool end )
{
  // cout << str << endl;
  // Unrecognized token with first char : '$'
  // Unexpected token : '*'
  // Undefined identifier : 'bcd'            -- FINISHED!!!!!
  if ( str.size() == 0 )
    return;

  for ( int i = 0 ; i < str.size() ; i++ )
  {
    bool fine = 0;
    if ( '0' <= str[ i ] && str[ i ] <= '9' )
      fine = 1;
    if ( 'a' <= str[ i ] && str[ i ] <= 'z' )
      fine = 1;
    if ( 'A' <= str[ i ] && str[ i ] <= 'Z' )
      fine = 1;
    if ( str[ i ] == '+' || str[ i ] == '-' || str[ i ] == '*' || str[ i ] == '/' ||
         str[ i ] == '(' || str[ i ] == ')' || str[ i ] == '>' || str[ i ] == '<' ||
         str[ i ] == '=' || str[ i ] == '.' || str[ i ] == '_' || str[ i ] == ' ' )
      fine = 1;

    if ( i < str.size() - 1 && str.substr( i, 2 ) == ":=" )
    {
      fine = 1;
      i++;
    } // if

    else if (  i == str.size() - 1 && str.substr( i, 1 ) == ":" && ( end == 0 ) )
    {
      fine = 1;
      str.erase( i, 1 );
    } // else if


    if ( ! fine )
    {
      Check( str.substr( 0, i ), 0 );
      throw runtime_error( "Unrecognized token with first char : '" + str.substr( i, 1 ) + "'" );
    } // if
  }   // for


  str += "   "; // str += 3
  Tokenarr tokens;
  int      tsize     = 0;
  bool     hasassign = 0;

  for ( int i = 0 ; i < str.size() - 3 ; )
  {
    if ( str[ i ] == ' ' )
      i++;
    // if

    else if ( str.substr( i, 2 ) == ">=" || str.substr( i, 2 ) == "<=" || str.substr( i, 2 ) == "<>" )
    {
      Token token( str.substr( i, 2 ) );
      tokens[ tsize ] = token;
      tsize++;
      i += 2;
    } // else if
    else if ( str.substr( i, 2 ) == ":=" )
    {
      hasassign = 1;
      Token token( str.substr( i, 2 ) );
      tokens[ tsize ] = token;
      tsize++;
      i += 2;
    } // else if
    else if ( str.substr( i, 1 ) == "=" || str.substr( i, 1 ) == "<" ||
              str.substr( i, 1 ) == ">" || str.substr( i, 1 ) == "+" ||
              str.substr( i, 1 ) == "-" || str.substr( i, 1 ) == "*" ||
              str.substr( i, 1 ) == "/" || str.substr( i, 1 ) == "(" ||
              str.substr( i, 1 ) == ")" )
    {
      Token token( str.substr( i, 1 ) );
      tokens[ tsize ] = token;
      tsize++;
      i += 1;
    } // else if
    else if ( ( '0' <= str[ i ] && '9' >= str[ i ] ) || str[ i ] == '.' )
    {
      bool   hasdot = 0;
      string s;
      for ( ; ( i < str.size() - 3 ) &&
            ( ( '0' <= str[ i ] && '9' >= str[ i ] ) || ( ( str[ i ] == '.' ) &&
                                                          ( ! hasdot ) ) ) ;
            i++ )
      {
        if ( str[ i ] == '.' )
          hasdot = 1;
        s += str[ i ];
      } // for

      Token token( s );
      tokens[ tsize ] = token;
      tsize++;
    }   // else if
    else if ( str[ i ] == '_' )
    {
      Check( str.substr( 0, i ), 0 );
      throw runtime_error( "Unrecognized token with first char : '" + str.substr( i, 1 ) + "'" );
    } // else if
    else
    {
      string s;
      for ( ; str[ i ] != '(' && str[ i ] != ')' && str[ i ] != '=' && str[ i ] != '>' &&
            str[ i ] != '<' && str[ i ] != '.' && str[ i ] != '+' && str[ i ] != '-' &&
            str[ i ] != '*' && str[ i ] != '/' && str[ i ] != ':' && str[ i ] != ' ' && i < str.size() - 3 ;
            i++ )
        s += str[ i ];
      if ( s.size() )
      {
        Token token( s );
        tokens[ tsize ] = token;
        tsize++;
      } // if
    }   // else
  }     // for

  if ( tsize == 0 )
    return;

  if ( hasassign && tokens [ 1 ].Mtype() != "Assign" )
  {
    for ( int i = 2 ; i < tsize ; i++ )
      if ( tokens[ i ].Mtype() == "Assign" )
      {
        Check( Tokenstostr( tokens, i ), 0 );
        throw runtime_error( "Unexpected token : '" + tokens [ i ].Mdata() + "'" );
      }            // if

    int iiiii = 0; // 排版用
  } // if


  if ( tokens [ 0 ].Mdata() == "+" || tokens [ 0 ].Mdata() == "-" )
    tokens [ 0 ].Msettype( "ArithExp" );


  for ( int i = 0 ; i + 1 < tsize  ; i++ )
    if ( ( tokens [ i ].Mtype() == "Term" ||
           tokens [ i ].Mtype() == "parentheses(" ||
           tokens [ i ].Mtype() == "Assign" ||
           tokens [ i ].Mtype() == "BooleanExp" ) &&
         ( tokens [ i + 1 ].Mdata() == "+" ||
           tokens [ i + 1 ].Mdata() == "-" ) )
      tokens [ i + 1 ].Msettype( "ArithExp" );
  // for

  if ( 0 )
    for ( int i = 0 ; i < tsize ; i++ )
      cout << tokens [ i ].Mstr() << endl;

  if ( tokens [ 0 ].Mdata() == "quit" )
    throw runtime_error( "Program exits..." );

  Token lasttoken( "10000" );
  // first token
  // map<string, bool> firsttype;
  bool firsttype[ 10 ];
  firsttype[ Typetoi( "BooleanExp" ) ]   = 0; // <,>,=
  firsttype[ Typetoi( "ArithExp" ) ]     = 1; // +,-
  firsttype[ Typetoi( "Term" ) ]         = 0; // */
  firsttype[ Typetoi( "parentheses(" ) ] = 1; // (
  firsttype[ Typetoi( "parentheses)" ) ] = 0; // )
  firsttype[ Typetoi( "NUM" ) ]          = 1; // 123
  firsttype[ Typetoi( "IDENT" ) ]        = 1; // abc
  firsttype[ Typetoi( "Assign" ) ]       = 0; // :=

  if ( firsttype[ Typetoi( tokens[ 0 ].Mtype() ) ] == 0  )
    throw runtime_error( "Unexpected token : '" + tokens [ 0 ].Mdata() + "'" );

  lasttoken = tokens [ 0 ];
  int parentheses = 0;
  if ( lasttoken.Mequal( "(" ) )
    parentheses++;
  bool hasboolexp = 0;
  hasassign = 0;
  for ( int i = 1 ; i < tsize ; i++ )
  {
    if ( tokens [ i ].Mequal( "(" ) )
      parentheses++;
    if ( tokens [ i ].Mequal( ")" ) )
      parentheses--;
    if ( parentheses < 0 )
    {
      Check( Tokenstostr( tokens, i ), 0 );
      throw runtime_error( "Unexpected token : ')'" );
    } // if

    if ( hasassign && tokens [ i ].Mtype() == "BooleanExp" )
    {
      Check( Tokenstostr( tokens, i ), 0 );
      throw runtime_error( "Unexpected token : '" + tokens [ i ].Mdata() + "'" );
    } // if

    if ( tokens [ i ].Mtype() == "BooleanExp" )
    {
      if ( hasboolexp )
      {
        Check( Tokenstostr( tokens, i ), 0 );
        throw runtime_error( "Unexpected token : '" + tokens [ i ].Mdata() + "'" );
      } // if

      hasboolexp = 1;
    }   // if

    if ( tokens [ i ].Mtype() == "Assign" )
    {
      if ( hasassign )
      {
        Check( Tokenstostr( tokens, i ), 0 );
        throw runtime_error( "Unexpected token : '" + tokens [ i ].Mdata() + "'" );
      } // if

      hasassign = 1;
    }   // if

    if ( ! gacmap[ Typetoi( lasttoken.Mtype() ) ][ Typetoi( tokens[ i ].Mtype() ) ] )
    {
      Check( Tokenstostr( tokens, i ), 0 );
      throw runtime_error( "Unexpected token : '" + tokens [ i ].Mdata() + "'" );
    } // if

    lasttoken = tokens [ i ];
  } // for


  if ( parentheses && end )
  {
    Check( Tokenstostr( tokens, tsize ), 0 );
    throw runtime_error( "Unexpected token : ';'" );
  } // if

  // map<string, bool> endtype;
  bool endtype[ 10 ];
  endtype[ Typetoi( "BooleanExp" ) ]   = 0; // <,>,=
  endtype[ Typetoi( "ArithExp" ) ]     = 0; // +,-
  endtype[ Typetoi( "Term" ) ]         = 0; // */
  endtype[ Typetoi( "parentheses(" ) ] = 0; // (
  endtype[ Typetoi( "parentheses)" ) ] = 1; // )
  endtype[ Typetoi( "NUM" ) ]          = 1; // 123
  endtype[ Typetoi( "IDENT" ) ]        = 1; // abc
  endtype[ Typetoi( "Assign" ) ]       = 0; // :=

  if ( ! endtype[ Typetoi( lasttoken.Mtype() ) ] && end )
  {
    Check( Tokenstostr( tokens, tsize - 1 ), 0 );
    throw runtime_error( "Unexpected token : ';'" );
  } // if

  if ( tokens [ 0 ].Mtype() == "IDENT" && hasassign == 0 &&
       ( end == 1 || tsize > 1 ) )
    if ( Hasthisid( tokens [ 0 ].Mdata() ) == 0 )
      throw runtime_error( "Undefined identifier : '" + tokens [ 0 ].Mdata() + "'" );

  for ( int i = 1 ; i < tsize ; i++ )
    if ( tokens [ i ].Mtype() == "IDENT"  )
      if ( Hasthisid( tokens [ i ].Mdata() ) == 0 )
        throw runtime_error( "Undefined identifier : '" + tokens [ i ].Mdata() + "'" );

  return;
} // Check()

int main()
{
  Buildgacmap();
  string testnum;

  cin >> testnum;
  if ( testnum == "quit" || testnum == "111111" )
  {
    cout << "> Program exits...\n";
    return 0;
  } // if

  cout << "Program starts...\n";
  while ( 1 )
  {
    bool isend = 0;
    try
    {
      string command = "";
      string checkcommand;
      Number rightval( 0 );
      string leftval;
      char   c;
      bool   lastisdivide = 0;
      bool   skipmode     = 0;
      for ( c = getchar() ; ( c != ';' || skipmode ) ; c = getchar() )
      {
        if ( skipmode == false && c == '/' )
        {
          if ( lastisdivide )
          {
            skipmode     = 1;
            lastisdivide = 0;
          } // if
          else
            lastisdivide = 1;
        } // if

        if ( skipmode == false && lastisdivide && c != '/' )
        {
          lastisdivide  = 0;
          command      += '/';
          checkcommand += '/';
        } // if

        if ( skipmode == false && c != ' ' && c != '\n' && c != '\t' && c != '/' )
        {
          command      += c;
          checkcommand += c;
          //
        }   // if

        if ( c == '\n' || c == ' ' )
        {
          checkcommand += ' ';
          if ( c == '\n' )
          {
            isend    = 1;
            skipmode = false;
            Check( checkcommand, 0 );
          } // if
        }   // if
      }   // for
      // finish cin!!!!!!!!!!!!!!!!!
      isend = 0;
      string target = command;
      if ( command.find( ":=" ) != string::npos )
      {
        leftval = command.substr( 0, command.find( ":=" ) );
        target  = command.substr( command.find( ":=" ) + 2 );
      } // if

      Check( checkcommand, 1 );
      rightval = Caculate_float( target );


      if ( command.find( ":=" ) != string::npos )
        Setvar( leftval, rightval );

      cout << "> " << rightval.Mstr() << endl;
    } // if
    catch ( const exception& e )
    {
      cout << "> " << e.what() << endl;
      if ( strcmp( e.what(), "Program exits..." ) == 0 )
        return 0;

      if ( ! isend )
        while ( getchar() != '\n' )
          ;
    } // catch

    // cout << "command->'" << command << "'\n=" << rightval << '\n';
  } // while
}   // main()

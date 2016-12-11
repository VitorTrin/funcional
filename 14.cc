#include <iostream>
#include <vector>
#include <algorithm>
#include <type_traits>
 
using namespace std;
 
template <int N, typename T, typename F>
void operator | ( T (&tab)[N], F f ) {
  for( auto x : tab )
    f( x );
}
 
template <typename T, typename F>
void operator | ( T tab, F f ) {
  for( auto x : tab )
    f( x );
}
 
struct Functor {
  const static bool True = true;
};
 
struct Var : public Functor {
  template <typename T>
  T operator()( T valor ) {
    return valor;
  }
  static const bool is_string = false;
};
Var x;
 
template <typename T>
struct is_string{
  static char match(string);
  static double match(...);
  static T t;
  enum { value = sizeof(
    (t)) == sizeof(char) };
};
 
// Constante: para casos 2 * Var
template <typename K>
struct Constant {
  K valor;
  static const bool is_string = is_string<K>::value;
  Constant(K valor): valor(valor){}
 
  template< typename T>
  K operator()( T entry = 0 ) {
    return valor;
  }
};
 
// Functor Somador
template <typename Op1, typename Op2>
struct Somador : Functor{
  Op1 op1;
  Op2 op2;
  static const bool is_string = Op1::is_string || Op2::is_string;
 
  Somador( Op1 op1, Op2 op2 ) : op1(op1), op2(op2) {}
 
  template <class T>
  auto operator()( T valor ) -> typename enable_if<!Op1::is_string && !Op2::is_string, decltype( op1( valor ) + op2( valor ) )>::type {
    return op1( valor ) + op2( valor );
  }
 
  template <class T>
  auto operator()( T valor ) -> typename enable_if<!Op1::is_string && Op2::is_string, decltype( to_string( op1( valor ) ) + op2( valor ) )>::type {
    return to_string( op1( valor ) ) + op2( valor );
  }
 
  template <class T>
  auto operator()( T valor ) -> typename enable_if<Op1::is_string && !Op2::is_string, decltype( op1( valor ) + to_string( op2( valor ) ) )>::type {
    return op1( valor ) + to_string( op2( valor ) );
  }
};

//Functor Porcento
template <typename Op1, typename Op2>
struct Porcento : Functor
 {
  Op1 op1;
  Op2 op2;

  static const bool is_string = Op1::is_string || Op2::is_string;

  Porcento(Op1 op1, Op2 op2) : op1(op1), op2(op2) {}

  template<class T>
  auto operator()( T valor ) -> decltype( op1(valor) % op2( valor ) ) {
    return op1( valor ) % op2( valor );
  }
 }; 

// Functor Multiplicador
template <typename Op1, typename Op2>  
struct Multiplicador : Functor {
  static const bool is_string = Op1::is_string || Op2::is_string;
  Op1 op1;
  Op2 op2;
 
  Multiplicador( Op1 op1, Op2 op2 ) : op1(op1), op2(op2) {}
   
  template <class T> 
  auto operator()( T valor ) -> decltype( op1( valor ) * op2( valor ) ) {
    return op1( valor ) * op2( valor );
  }
};
 
// Testa se eh Functor
template <typename T>
struct is_functor {
  static char match(Var);
 
  template <typename Op1, typename Op2>
  static char match(Multiplicador<Op1,Op2>);
 
  template <typename Op1, typename Op2>
  static char match(Somador<Op1, Op2>);
 
  static double match(...);
 
  static T t;
  enum { value = sizeof(match(t)) == sizeof(char) };
};
 
// Var * Var
template <typename Op1, typename Op2>
inline Multiplicador<typename enable_if<is_functor<Op1>::value, Op1>::type, 
                     typename enable_if<is_functor<Op2>::value, Op2>::type > 
operator * ( Op1 a, Op2 b ) {
  return Multiplicador< Op1, Op2>( a, b );
}
 
// Constante * Var
template <typename Op1, typename Op2>
inline Multiplicador<typename enable_if<!is_functor<Op1>::value, Constant<Op1>>::type, 
                     typename enable_if<is_functor<Op2>::value, Op2>::type >
operator * ( Op1 a, Op2 b ) {
  return Multiplicador<Constant<Op1>, Op2>(Constant<Op1>(a), b);
}
 
// Var + Var
/*template <typename Op1, typename Op2>
inline Somador<typename enable_if<is_functor<Op1>::value, Op1>::type, typename enable_if<is_functor<Op2>::value, Op2>::type >
operator + ( Op1 a, Op2 b ) {
  return Somador< Op1, Op2>( a, b );
}
 
// Constant + Var
template <typename Op1, typename Op2>
inline Somador<typename enable_if<!is_functor<Op1>::value, Const<Op1>>::type, typename enable_if<is_functor<Op2>::value, Op2>::type >
operator + ( Op1 a, Op2 b ) {
  return Somador< Const<Op1>, Op2>( Const<Op1>(a), b );
}*/
 
// Var + Constant
template <typename Op1, typename Op2>
inline Somador<typename enable_if<is_functor<Op1>::value, Op1>::type, typename enable_if<!is_functor<Op2>::value, Constant<Op2>>::type >
operator + ( Op1 a, Op2 b ) {
  return Somador< Op1, Constant<Op2>>( a, Constant<Op2>(b) );
}
 
struct Pipe {
  template <class A, class B> 
  auto operator()( A a, B b ) -> decltype( a << b ) {
    return a << b;
  }
};
 
template <typename A>
inline Pipe operator | ( A a, Var b ) {
  return Pipe();
}
 
int main() {
  int tab[] =  { 1, 2, 3, 4 };
  vector<int> v;
     
  tab | [ &v ]( int x ) { v.push_back( x ); };
     
  cout << "-> op = x * x" << endl;
  auto op = x * x;
  v | [ &op ]( int n ) { cout << op( n ) << endl; };
 
  cout << "-> op = x * x * x" << endl;
  auto op2 = x * x * x;
  v | [ &op2 ]( int n ) { cout << op2( n ) << endl; };
 
  cout << "-> op = 2 * x + 1" << endl;
  auto op3 = 2 * x + 1;
  v | [ &op3 ]( int n ) { cout << op3( n ) << endl; };
 
  cout << "-> op = 3.14 * x" << endl;
  auto op4 = 3.14 * x;
  v | [ &op4 ]( int n ) { cout << op4( n ) << endl; };
 
  cout << "-> op = x + \"!\"" << endl;
  auto op5 = x + "!";
  v | [ &op5 ]( int n ) { cout << op5( n ) << endl; };
 
  return 0;  
}
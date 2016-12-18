#include <iostream>
#include <vector>
#include <algorithm>
#include <type_traits>

using namespace std;

struct Functor {
  const static bool True = true;
};

template <typename Op1, typename Op2>
struct Comparador : Functor {
  Op1 op1;
  Op2 op2;

  Comparador( Op1 op1, Op2 op2 ) : op1(op1), op2(op2) {}

  template <class T>
  bool operator()( T valor ) {
    return op1( valor ) == op2( valor );
  }
};

template <typename T>
struct is_comparator {
  template <typename Op1, typename Op2>
  static char match(Comparador<Op1, Op2>);

  static double match(...);

  static T t;
  enum { value = sizeof(match(t)) == sizeof(char) };
};


template <int N, typename T, typename F>
void operator | ( T (&tab)[N], F f ) {
  for( auto x : tab )
    f( x );
}

template <typename T, typename F>
typename enable_if<is_comparator<F>::value, T>::type
 operator | ( T tab, F f ) {
  T result;
  for( auto x : tab )
    result.push_back(f( x ));

  return result;
}

template <typename T, typename F>
typename enable_if<!is_comparator<F>::value, void>::type
 operator | ( T tab, F f ) {
  for( auto x : tab )
    f( x );
}

struct Var : public Functor {
  template <typename T>
  T operator()( T valor ) {
    return valor;
  }
};
Var x;

// Constante: para casos 2 * Var
template <typename K>
struct Constant {
  K valor;
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

  Somador( Op1 op1, Op2 op2 ) : op1(op1), op2(op2) {}

  template <class T>
  auto operator()( T valor ) -> decltype( op1( valor ) + op2( valor )) {
    return op1( valor ) + op2( valor );
  }
};

// Functor Multiplicador
template <typename Op1, typename Op2>
struct Multiplicador : Functor {
  Op1 op1;
  Op2 op2;

  Multiplicador( Op1 op1, Op2 op2 ) : op1(op1), op2(op2) {}

  template <class T>
  auto operator()( T valor ) -> decltype( op1( valor ) * op2( valor ) ) {
    return op1( valor ) * op2( valor );
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

// Testa se eh Functor
template <typename T>
struct is_functor {
  static char match(Var);

  template <typename Op1, typename Op2>
  static char match(Multiplicador<Op1,Op2>);

  template <typename Op1, typename Op2>
  static char match(Somador<Op1, Op2>);

// TODO: Maybe remove
  template <typename Op1, typename Op2>
  static char match(Comparador<Op1, Op2>);

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

// Var == Var
template <typename Op1, typename Op2>
inline Comparador<typename enable_if<is_functor<Op1>::value, Op1>::type,
                     typename enable_if<is_functor<Op2>::value, Op2>::type >
operator == ( Op1 a, Op2 b ) {
  return Comparador< Op1, Op2>( a, b );
}

// Constante == Var
template <typename Op1, typename Op2>
inline Comparador<typename enable_if<!is_functor<Op1>::value, Constant<Op1>>::type,
                     typename enable_if<is_functor<Op2>::value, Op2>::type >
operator == ( Op1 a, Op2 b ) {
  return Comparador<Constant<Op1>, Op2>(Constant<Op1>(a), b);
}

// Var == Constante
template <typename Op1, typename Op2>
inline Comparador<typename enable_if<is_functor<Op1>::value, Op1>::type,
                  typename enable_if<!is_functor<Op2>::value, Constant<Op2>>::type >
operator == ( Op1 a, Op2 b ) {
  return Comparador<Op1, Constant<Op2>>(a, Constant<Op2>(b));
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

// Var % Constant
template <typename Op1, typename Op2>
inline Porcento<typename enable_if<is_functor<Op1>::value, Op1>::type, typename enable_if<!is_functor<Op2>::value, Constant<Op2>>::type >
operator % (Op1 a, Op2 b) {
  return Porcento< Op1, Constant<Op2>>(a, Constant<Op2>(b));
}

class Interval {
  public:
    Interval( int inicio, int fim ): inicio(inicio), fim(fim) {}
    
    class iterator {
      public:
        iterator( int inicio ): atual( inicio ) {}
        
        bool operator != ( const iterator& b ) {
          return atual != b.atual;
        }
        
        iterator& operator++ () {
          ++atual;
          return *this;
        }
        
        int operator * () {
          return atual;
        }
        
      private:
        int atual;  
    };
    
    iterator begin() {
      return iterator( inicio );
    }
    
    iterator end() {
      return iterator( fim );
    }
    
  private:
    int inicio;
    int fim;
};


int main() {
  int tab[] =  { 1, 2, 3, 4 };
  vector<int> v;

  tab | [ &v ]( int x ) { v.push_back( x ); };

  auto op_test = (x == 3);

  // v | op_test(x);

  // for( auto x : v1 ){
  //   cout << x << ", ";
  // }
  // cout << endl;

  cout << "-> op = x * x" << endl;
  auto op = x * x;

  auto v2 = v | x * x * x == x;

  for( auto x : v2) {
    cout << x << ", ";
  }
  cout << endl;

  cout << "-> op = x * x * x" << endl;
  auto op2 = x * x * x;
  v | [ &op2 ]( int n ) { cout << op2( n ) << endl; };

  cout << "-> op = 2 * x + 1" << endl;
  auto op3 = 2 * x + 1;
  v | [ &op3 ]( int n ) { cout << op3( n ) << endl; };

  cout << "-> op = 3.14 * x" << endl;
  auto op4 = 3.14 * x;
  v | [ &op4 ]( int n ) { cout << op4( n ) << endl; };

  cout << "-> op = x % 2 == 0" << endl;
  auto op6 = x % 2;
  v | [ &op6 ]( int n) { cout << op6(n) << endl; };

  return 0;
}

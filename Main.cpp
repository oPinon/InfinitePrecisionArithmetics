
#include <iostream>
#include <assert.h>
#include <vector>
#include <string>
#include <sstream>

struct InfinitePrecisionUInt
{
private:
  inline static uint32_t maxDigits() { return std::log10( std::numeric_limits<uint32_t>::max() ) - 1; }
  inline static uint32_t max() { return std::pow( 10, maxDigits() ); }
  std::vector<uint32_t> m_values;

  inline uint32_t& getOrCreate( const uint32_t i )
  {
    while( i >= m_values.size() )
      m_values.push_back( 0 );
    return m_values[i];
  }

  void addAt( const uint64_t value, const uint32_t offset )
  {
    uint64_t v = this->getOrCreate( offset );
    uint64_t s = v + value;
    uint32_t m = ( s % max() );
    uint32_t r = ( s / max() );
    this->m_values[offset] = m;
    if( r > 0 )
      this->addAt( r, offset + 1 );
  }

public:

  InfinitePrecisionUInt() {}

  InfinitePrecisionUInt( uint64_t v ) { this->addAt( v, 0 ); }

  InfinitePrecisionUInt( const std::string& s )
  {
    uint32_t powerTen = 0;
    uint32_t sum = 0;
    for( auto it = s.rbegin(); it != s.rend(); it++ )
    {
      if( *it == ',' )
        continue;
      if( powerTen >= maxDigits() )
      {
        m_values.push_back( sum );
        powerTen = 0;
        sum = 0;
      }
      int digit = ( *it - '0' );
      if( digit < 0 || digit > 9 )
        throw "Invalid digit";
      sum += digit * pow( 10, powerTen++ );
    }
    m_values.push_back( sum );
  }

  inline uint32_t digits() const { return m_values.size() * this->maxDigits(); }
  inline unsigned short digit( const uint32_t i ) const
  {
    const uint32_t subNumber = ( i / this->maxDigits() );
    if( subNumber >= m_values.size() )
      return 0;
    const uint32_t subDigit = ( i % this->maxDigits() );
    return ( m_values[subNumber] / uint32_t(std::pow( 10, subDigit )) ) % 10;
  }

  inline void operator+=( const InfinitePrecisionUInt& o )
  {
    for( uint32_t i = 0; i < o.m_values.size(); i++ )
      this->addAt( o.m_values[i], i );
  }

  void operator-=( const InfinitePrecisionUInt& o )
  {
    assert( *this > o );
    for( uint32_t ip = o.m_values.size(); ip > 0; ip-- )
    {
      const uint32_t i = ip - 1;
      uint32_t v = this->getOrCreate( i );
      uint32_t vo = o.m_values[i];
      if( v < vo )
      {
        bool found = false;
        for( uint32_t j = i; !found && j < this->m_values.size()-1; j++ )
        {
          if( this->m_values[j+1] > 0 )
          {
            this->m_values[j+1]--;
            for( uint32_t k = i + 1; k <= j; k++ )
            {
              assert( this->m_values[k] == 0 );
              this->m_values[k] = InfinitePrecisionUInt::max() - 1;
            }
            this->m_values[i] += ( InfinitePrecisionUInt::max() - o.m_values[i] );
            found = true;
          }
        }
        assert( found == true );
      }
      else
        this->m_values[i] -= o.m_values[i];
    }
  }

  void operator*=( const InfinitePrecisionUInt& o )
  {
    InfinitePrecisionUInt dst;
    for( uint32_t i = 0; i < o.m_values.size(); i++ )
    {
      for( uint32_t j = 0; j < this->m_values.size(); j++ )
      {
        uint64_t m = uint64_t( o.m_values[i] ) * uint64_t( this->m_values[j] );
        //std::cout << o.m_values[i] << " * " << this->m_values[i] << " = " << m << std::endl;
        dst.addAt( m, i + j );
      }
    }
    ( *this ) = dst;
  }

  bool operator<( const InfinitePrecisionUInt& o ) const
  {
    if( m_values.size() != o.m_values.size() )
      return m_values.size() < o.m_values.size();

    for( uint32_t i = 0; i < m_values.size(); i++ )
      if( m_values[i] != o.m_values[i] )
        return m_values[i] < o.m_values[i];

    return false;
  }

  inline bool operator>( const InfinitePrecisionUInt& o ) const { return !(*this < o) && (*this != o); }
  inline bool operator==( const InfinitePrecisionUInt& o ) const { return !( *this < o ) && !( o < *this ); }
  inline bool operator!=( const InfinitePrecisionUInt& o ) const { return !( *this == o ); }

  std::ostream& print( std::ostream& s ) const
  {
    assert( !m_values.empty() );
    for( auto it = m_values.rbegin(); it != m_values.rend(); it++ )
    {
      /*if( it == m_values.rbegin() )
        s << *it;
      else*/
      {
        std::stringstream ss; ss << ( max() + *it );
        s << ss.str().substr( 1 );
      }
      //s << '|';
    }
    return s;
  }
};

inline InfinitePrecisionUInt operator+( const InfinitePrecisionUInt& a, const InfinitePrecisionUInt& b ) { InfinitePrecisionUInt dst = a; dst += b; return dst; }
inline InfinitePrecisionUInt operator-( const InfinitePrecisionUInt& a, const InfinitePrecisionUInt& b ) { InfinitePrecisionUInt dst = a; dst -= b; return dst; }
inline InfinitePrecisionUInt operator*( const InfinitePrecisionUInt& a, const InfinitePrecisionUInt& b ) { InfinitePrecisionUInt dst = a; dst *= b; return dst; }

inline std::ostream& operator<<( std::ostream& s, const InfinitePrecisionUInt& f ) { return f.print( s ); }

struct InfinitePrecisionInt
{
private:
  bool m_positive;
  InfinitePrecisionUInt m_digits;

public:
  InfinitePrecisionInt() : m_positive(true) {}

  InfinitePrecisionInt( int64_t v )
    : m_positive( v > 0 )
    , m_digits( std::abs( v ) )
  {}

  InfinitePrecisionInt( const std::string& s )
  {
    if( s[0] == '+' || s[0] == '-' )
    {
      this->m_positive = ( s[0] == '+' );
      this->m_digits = InfinitePrecisionUInt( s.substr( 1 ) );
    }
    else
      this->m_digits = InfinitePrecisionUInt( s );
  }

  inline InfinitePrecisionInt inverse() const { InfinitePrecisionInt dst = *this; dst.m_positive = !dst.m_positive; return dst; }
  inline bool positive() const { return m_positive; }
  inline bool negative() const { return !m_positive; }

  inline void operator+=( const InfinitePrecisionInt& o )
  {
    if( this->m_positive == o.m_positive )
      this->m_digits += o.m_digits;
    else
    {
      if( this->m_digits < o.m_digits )
      {
        this->m_positive = o.m_positive;
        this->m_digits = ( o.m_digits - this->m_digits );
      }
      else
        this->m_digits -= o.m_digits;
    }
  }

  inline void operator-=( const InfinitePrecisionInt& o ) { ( *this ) += o.inverse(); }

  inline void operator*=( const InfinitePrecisionInt& o )
  {
    this->m_digits *= o.m_digits;
    this->m_positive = !( this->m_positive ^ o.m_positive );
  }

  bool operator<( const InfinitePrecisionInt& o ) const
  {
    if( this->m_positive != o.m_positive )
      if( o.m_digits != 0 || this->m_digits != 0 )
        return this->negative();
      else
        return false;

    if( this->m_digits == o.m_digits )
      return false;

    bool inf = ( this->m_digits < o.m_digits );
    return this->positive() ? inf : !inf;
  }

  inline bool operator>( const InfinitePrecisionInt& o ) const { return !( *this < o ) && ( *this != o ); }
  inline bool operator==( const InfinitePrecisionInt& o ) const { return !( *this < o ) && !( o < *this ); }
  inline bool operator!=( const InfinitePrecisionInt& o ) const { return !( *this == o ); }

  std::ostream& print( std::ostream& s ) const
  {
    s << ( this->positive() ? '+' : '-' );
    s << this->m_digits;
    return s;
  }
};

inline InfinitePrecisionInt operator+( const InfinitePrecisionInt& a, const InfinitePrecisionInt& b ) { InfinitePrecisionInt dst = a; dst += b; return dst; }
inline InfinitePrecisionInt operator-( const InfinitePrecisionInt& a, const InfinitePrecisionInt& b ) { InfinitePrecisionInt dst = a; dst -= b; return dst; }
inline InfinitePrecisionInt operator*( const InfinitePrecisionInt& a, const InfinitePrecisionInt& b ) { InfinitePrecisionInt dst = a; dst *= b; return dst; }

inline std::ostream& operator<<( std::ostream& s, const InfinitePrecisionInt& f ) { return f.print( s ); }

struct InfinitePrecisionFloat
{
private:
  int32_t m_offset;
  InfinitePrecisionInt m_digits;

  inline double factor() const { return std::pow( 10, m_offset ); }

public:
  InfinitePrecisionFloat() : m_offset( 0 ) {}

  InfinitePrecisionFloat( double d )
  {
    this->m_offset = -( std::log10( std::numeric_limits<uint32_t>::max() ) - 1 );
    this->m_digits = InfinitePrecisionInt( d / this->factor() );
  }

  std::ostream& print( std::ostream& s ) const
  {
    std::stringstream ss;
    ss << m_digits;
    std::string str = ss.str();
    uint32_t digits = str.size();
    bool signChar = ( str.size() > 0 && ( str[0] == '+' || str[0] == '-' ) );
    if( signChar )
    {
      s << str[0];
      digits--;
      str = str.substr( 1 );
    }
    if( digits < -m_offset )
    {
      s << "0.";
      for( int32_t i = digits; i < -m_offset; i++ )
        s << '0';
      s << str;
    }
    else
    if( m_offset < 0 )
    {
      s << str.substr( 0, digits + m_offset );
      s << '.';
      s << str.substr( digits + m_offset );
    }
    else
      s << str;
    return s;
  }
};

inline std::ostream& operator<<( std::ostream& s, const InfinitePrecisionFloat& f ) { return f.print( s ); }

typedef InfinitePrecisionUInt UIntI;
typedef InfinitePrecisionInt IntI;
typedef InfinitePrecisionFloat FloatI;

void Test()
{

  {
    UIntI a = "42424242424242";
    UIntI b = "42424242424242";
    UIntI c = "42425242424242";
    UIntI d = "424242";
    assert( a == b );
    assert( !( a != b ) );
    assert( a != c );
    assert( a != d );
  }

  {
    UIntI a = 42;
    UIntI b = 7;
    UIntI c = 49;
    assert( ( a + b ) == c );
  }

  {
    UIntI a =  "9999999999999999999999999";
    UIntI b =                          "2";
    UIntI c = "10000000000000000000000001";
    assert( ( a + b ) == c );
  }

  {
    UIntI a = "5000000040";
    UIntI b = "4000000050";
    UIntI c = "20,000,000,410,000,002,000";
    assert( ( a * b ) == c );
  }

  {
    UIntI a = "51515151";
    UIntI b =  "5151515";
    assert( b < a );
    assert( !( a < b ) );
  }

  {
    UIntI a = "5151515151515151515151515195151515151515151";
    UIntI b = "5151515151515151515151515135151515151515151";
    assert( b < a );
    assert( !( a < b ) );
  }

  {
    UIntI a = "10000000000000000000000001";
    UIntI b =                          "2";
    UIntI c =  "9999999999999999999999999";
    assert( a - b == c );
  }

  {
    IntI a =    "1515351351513153121";
    IntI b =       "-513158532123513";
    IntI c =  "853251585321553132125";
    IntI d =   "+1514838192981029608";
    IntI e = "-851736233970039979004";
    assert( a + b == d );
    assert( a - c == e );
    assert( b < a );
    assert( a > b );
    assert( e < d );
    assert( e < b );
  }

  {
    FloatI a = 0.01416;
    std::cout << a << std::endl;
    FloatI b = 3.1416;
    std::cout << b << std::endl;
    FloatI c = 0.1416;
    std::cout << c << std::endl;
  }

  std::cout << "Test passed" << std::endl;
}

void main()
{
  /*Test<float>();
  Test<double>();
  Test<FloatI>();
  std::cout << "finished" << std::endl;*/

  //FloatI big = 3.14;
  //FloatI small = 0.000000000000132;
  //std::cout << big << std::endl;
  //std::cout << ( big + small ) << std::endl;

  Test();

  UIntI a = "0123456789012345678901234567890123456789";
  std::cout << a << std::endl;
  UIntI b = "999999999999999999999999999999999";
  UIntI c = "2";
  std::cout << ( b + c ) << std::endl;
  //std::cout << std::numeric_limits<uint32_t>::max() << std::endl;
  //std::cout << std::numeric_limits<uint64_t>::max() << std::endl;
}

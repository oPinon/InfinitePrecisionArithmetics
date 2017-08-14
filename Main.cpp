
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

  inline void operator+=( const InfinitePrecisionUInt& o )
  {
    for( uint32_t i = 0; i < o.m_values.size(); i++ )
      this->addAt( o.m_values[i], i );
  }

  inline void operator*=( const InfinitePrecisionUInt& o )
  {
    InfinitePrecisionUInt dst;
    for( uint32_t i = 0; i < o.m_values.size(); i++ )
    {
      for( uint32_t j = 0; j < this->m_values.size(); j++ )
      {
        uint64_t m = uint64_t( o.m_values[i] ) * uint64_t( this->m_values[j] );
        std::cout << o.m_values[i] << " * " << this->m_values[i] << " = " << m << std::endl;
        dst.addAt( m, i + j );
      }
    }
    ( *this ) = dst;
  }

  inline bool operator==( const InfinitePrecisionUInt& o ) const
  {
    if( m_values.size() != o.m_values.size() )
      return false;
    for( uint32_t i = 0; i < m_values.size(); i++ )
      if( m_values[i] != o.m_values[i] )
        return false;
    return true;
  }

  std::ostream& print( std::ostream& s ) const
  {
    assert( !m_values.empty() );
    for( auto it = m_values.rbegin(); it != m_values.rend(); it++ )
    {
      if( it == m_values.rbegin() )
        s << *it;
      else
      {
        std::stringstream ss; ss << ( max() + *it );
        s << ss.str().substr( 1 );
      }
      s << '|';
    }
    return s;
  }
};

inline InfinitePrecisionUInt operator+( const InfinitePrecisionUInt& a, const InfinitePrecisionUInt& b ) { InfinitePrecisionUInt dst = a; dst += b; return dst; }
inline InfinitePrecisionUInt operator*( const InfinitePrecisionUInt& a, const InfinitePrecisionUInt& b ) { InfinitePrecisionUInt dst = a; dst *= b; return dst; }

inline std::ostream& operator<<( std::ostream& s, const InfinitePrecisionUInt& f )
{
  return f.print( s );
}

typedef InfinitePrecisionUInt UIntI;
//typedef double FloatI;

/*
template<typename T>
void Test()
{
  T a = 3.14;
  T b = -2.4;

  //assert( ( a - a ) == 0 );
  //assert( ( a * 2 ) == ( a + a ) );
}
*/

void Test()
{

  {
    UIntI a = 42;
    UIntI b = 7;
    UIntI c = 49;
    assert( ( a + b ) == c );
  }

  {
    UIntI a = "9999999999999999999999999";
    UIntI b = "2";
    UIntI c = "10000000000000000000000001";
    assert( ( a + b ) == c );
  }

  {
    UIntI a = "5000000040";
    UIntI b = "4000000050";
    UIntI c = "20,000,000,410,000,002,000";
    std::cout << c << std::endl;
    std::cout << a * b << std::endl;
    assert( ( a * b ) == c );
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

#if !defined( NON_CRYPTOGRAPHIC_HASH_MUSE_AIR_HPP )
#define NON_CRYPTOGRAPHIC_HASH_MUSE_AIR_HPP

#include <cstdint>
#include <cassert>
#include <immintrin.h>
#include <array>
#include <algorithm>
#include <utility>
#include <vector>
#include <functional>

#if defined( _MSC_VER )
#define FORCE_INLINE __forceinline
#define NEVER_INLINE __declspec( noinline )
#elif defined( __GNUC__ ) || defined( __clang__ )
#define FORCE_INLINE __attribute__( ( always_inline ) ) inline
#define NEVER_INLINE __attribute__( ( noinline ) )
#else
#define FORCE_INLINE inline
#define NEVER_INLINE
#endif

#if defined(__GNUC__) || defined(__clang__)
#define LIKELY(x)   __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)
#elif defined(_MSC_VER)
#define LIKELY(x)   (x)
#define UNLIKELY(x) (x)
#else
#define LIKELY(x)   (x)
#define UNLIKELY(x) (x)
#endif

using State = std::array<uint64_t, 6>;

constexpr std::array<uint64_t, 6> DEFAULT_SECRET = 
{
	UINT64_C(0x5ae31e589c56e17a), UINT64_C(0x96d7bb04e64f6da9), UINT64_C(0x7ab1006b26f9eb64),
    UINT64_C(0x21233394220b8457), UINT64_C(0x047cb9557c9f3b43), UINT64_C(0xd24f2590c0bcee28),
};

constexpr uint64_t INIT_RING_PREV = UINT64_C(0x33ea8f71bb6016d8);

constexpr size_t seg(size_t n) { return n * 8; }

template<typename Type>
struct Values2
{
	Type first;
	Type second;
};

template<typename Type>
struct Values3
{
	Type first;
	Type second;
	Type third;
};

// Helper function to swap byte order (for little-endian/big-endian conversion)
FORCE_INLINE uint64_t swap_uint64( uint64_t value )
{
	return ( ( value >> 56 ) & 0x00000000000000FF ) | ( ( value >> 40 ) & 0x000000000000FF00 ) | ( ( value >> 24 ) & 0x0000000000FF0000 ) | ( ( value >> 8 ) & 0x00000000FF000000 ) | ( ( value << 8 ) & 0x000000FF00000000 ) | ( ( value << 24 ) & 0x0000FF0000000000 ) | ( ( value << 40 ) & 0x00FF000000000000 ) | ( ( value << 56 ) & 0xFF00000000000000 );
}

FORCE_INLINE uint32_t swap_uint32( uint32_t value )
{
	return ( ( value >> 24 ) & 0x000000FF ) | ( ( value >> 8 ) & 0x0000FF00 ) | ( ( value << 8 ) & 0x00FF0000 ) | ( ( value << 24 ) & 0xFF000000 );
}

FORCE_INLINE bool is_little_endian()
{
	uint16_t value = 0x1;
	uint8_t* byte = reinterpret_cast<uint8_t*>( &value );
	return byte[ 0 ] == 0x1;
}

template <bool ByteSwap>
FORCE_INLINE uint64_t read_u64( const uint8_t* p )
{
	uint64_t value;
	std::memcpy( &value, p, sizeof( value ) );
	if constexpr (ByteSwap)
	{
		return swap_uint64( value );
	}
	else
	{
		return value;
	}
}

template <bool ByteSwap>
FORCE_INLINE uint64_t read_u32( const uint8_t* p )
{
	uint32_t value;
	std::memcpy( &value, p, sizeof( value ) );
	if constexpr (ByteSwap)
	{
		return swap_uint64( value );
	}
	else
	{
		return value;
	}
}

template <bool ByteSwap>
FORCE_INLINE void write_u64( uint8_t* p, uint64_t value )
{
	if constexpr ( ByteSwap )
	{
		value = swap_uint64( value );
	}
	std::memcpy( p, &value, sizeof( value ) );
}

template <bool ByteSwap>
FORCE_INLINE void read_short( const uint8_t* bytes, const size_t length, Values2<uint64_t>& values )
{
	auto& [ i, j ] = values;

	if ( length >= 4 )
	{
		int offset = ( length & 24 ) >> ( length >> 3 );  // length >= 8 ? 4 : 0
		i = ( read_u32<ByteSwap>( bytes ) << 32 ) | read_u32<ByteSwap>( bytes + length - 4 );
		j = ( read_u32<ByteSwap>( bytes + offset ) << 32 ) | read_u32<ByteSwap>( bytes + length - 4 - offset );
	}
	else if ( length > 0 )
	{
		// MSB <-> LSB
		// [0] [0] [0] for len == 1 (0b01)
		// [0] [1] [1] for len == 2 (0b10)
		// [0] [1] [2] for len == 3 (0b11)
		i = ( ( uint64_t )bytes[ 0 ] << 48 ) | ( ( uint64_t )bytes[ length >> 1 ] << 24 ) | ( uint64_t )bytes[ length - 1 ];
		j = 0;
	}
	else
	{
		i = 0;
		j = 0;
	}
}

FORCE_INLINE void multiple64_128bit( uint64_t x, uint64_t y, Values2<uint64_t>& result )
{
	auto& [ left, right ] = result;

#if defined( __SIZEOF_INT128__ )
	// 如果平台支持128位整数
	__uint128_t product = static_cast<__uint128_t>( x ) * static_cast<__uint128_t>( y );
	left = static_cast<uint64_t>( product );
	right = static_cast<uint64_t>( product >> 64 );
#elif defined( _M_X64 ) || defined( __x86_64__ )
	// MSVC x86-64平台，使用 _umul128
	left = _umul128( x, y, &right );
#elif defined( __aarch64__ )
	// ARM平台，使用__umulh
	left = x * y;
	right = __umulh( x, y );
#elif defined( __POWERPC64__ )
	// PowerPC64平台，使用__builtin_mulll_overflow
	unsigned __int128 product = static_cast<unsigned __int128>( x ) * static_cast<unsigned __int128>( y );
	left = static_cast<uint64_t>( product );
	right = static_cast<uint64_t>( product >> 64 );
#else
	// 如果没有128位整数支持，使用分割方法
	uint64_t x_high = x >> 32;
	uint64_t x_low = x & 0xFFFFFFFF;
	uint64_t y_high = y >> 32;
	uint64_t y_low = y & 0xFFFFFFFF;

	uint64_t high_high = x_high * y_high;
	uint64_t high_low = x_high * y_low;
	uint64_t low_high = x_low * y_high;
	uint64_t low_low = x_low * y_low;

	uint64_t cross = ( high_low & 0xFFFFFFFF ) + ( low_high & 0xFFFFFFFF ) + ( low_low >> 32 );
	right = high_high + ( high_low >> 32 ) + ( low_high >> 32 ) + ( cross >> 32 );
	left = ( cross << 32 ) | ( low_low & 0xFFFFFFFF );
#endif

}

FORCE_INLINE void chixx( Values3<uint64_t>& data )
{
	auto& [t, u, v] = data;

	t ^= ( ~u & v ), u ^= ( ~v & t ), v ^= ( ~t & u );
}

template <bool BlindFast>
FORCE_INLINE void frac_6( Values2<uint64_t>& output, const Values2<uint64_t>& input )
{
	auto& [p, q] = input;
	auto& [state_p, state_q] = output;
	Values2<uint64_t> result {0, 0};
	if ( !BlindFast )
	{
		state_p ^= p;
		state_q ^= q;
		multiple64_128bit( state_p, state_q, result );
		state_p ^= result.first;
		state_q ^= result.second;
	}
	else
	{
		multiple64_128bit( state_p ^ p, state_q ^ q, result );
		state_p = result.first;
		state_q = result.second;
	}
}

template <bool BlindFast>
FORCE_INLINE void frac_3( Values2<uint64_t>& output, const uint64_t input )
{
	auto& [state_p, state_q] = output;
	Values2<uint64_t> result {0, 0};
	if ( !BlindFast )
	{
		state_q ^= input;
		multiple64_128bit( state_p, state_q, result );
		state_p ^= result.first;
		state_q ^= result.second;
	}
	else
	{
		multiple64_128bit( state_p, state_q ^ input, result );
		state_p = result.first;
		state_q = result.second;
	}
}

template <bool ByteSwap>
FORCE_INLINE void tower_layer_0( State state, const uint8_t* bytes, size_t offset, size_t length, Values3<uint64_t>& result)
{
	auto& [a, b, c] = result;

	if ( offset <= seg( 2 ) )
	{
		Values2<uint64_t> data {0, 0};
		auto& [i, j] = data;
		read_short<ByteSwap>( bytes, offset, data );
		
		a = i;
		b = j;
		c = 0;
	}
	else
	{
		a = read_u64<ByteSwap>( bytes );
		b = read_u64<ByteSwap>( bytes + seg( 1 ) );
		c = read_u64<ByteSwap>( bytes + offset - seg( 1 ) );
	}

	if ( length >= seg( 3 ) )
	{
		Values3<uint64_t> temp1 { state[ 0 ], state[ 2 ], state[ 4 ] };
		Values3<uint64_t> temp2 { state[ 1 ], state[ 3 ], state[ 5 ] };

		chixx( temp1 );
		chixx( temp2 );

		state[ 0 ] = temp1.first;
		state[ 2 ] = temp1.second;
		state[ 4 ] = temp1.third;

		state[ 1 ] = temp2.first;
		state[ 3 ] = temp2.second;
		state[ 5 ] = temp2.third;

		a ^= state[ 0 ] + state[ 1 ];
		b ^= state[ 2 ] + state[ 3 ];
		c ^= state[ 4 ] + state[ 5 ];
	}
	else
	{
		a ^= state[ 0 ];
		b ^= state[ 1 ];
		c ^= state[ 2 ];
	}
}

FORCE_INLINE uint64_t rotate_left(uint64_t value, int shift) 
{
	return (value << shift) | (value >> (64 - shift));
}

FORCE_INLINE uint64_t rotate_right(uint64_t value, int shift) 
{
	return (value >> shift) | (value << (64 - shift));
}

template <bool BlindFast>
class MuseAir
{
public:
	// 计算64位的MuseAir散列值
	template <bool ByteSwap>
	inline void hash( const void* bytes, const size_t length, const uint64_t seed, void* result )
	{
		Values2<uint64_t> values2 {0, 0};

		if ( LIKELY( length <= 16 ) )
		{
			// 更可能会执行的分支
			tower_short<ByteSwap>( ( const uint8_t* )bytes, length, seed, values2 );
			epi_short( values2 );
		}
		else
		{
			Values3<uint64_t> values3 {0, 0, 0};
			tower_long<ByteSwap>( ( const uint8_t* )bytes, length, seed, values3 );
			epi_long( values3 );
			values2.first = values3.first;
			values2.second = values3.second;
		}

		if ( is_little_endian() )
		{
			write_u64<false>( ( uint8_t* )result, values2.first );
		}
		else
		{
			write_u64<true>( ( uint8_t* )result, values2.first );
		}
	}

	// 计算128位的MuseAir散列值
	template <bool ByteSwap>
	inline void hash_128( const void* bytes, const size_t length, const uint64_t seed, void* result )
	{
		Values3<uint64_t> values3 {0, 0, 0};

		if ( LIKELY( length <= 16 ) )
		{
			// 更可能会执行的分支
			Values2<uint64_t> values {0, 0};
			tower_short<ByteSwap>( ( const uint8_t* )bytes, length, seed, values );
			epi_short_128( values );
			values3.first = values.first;
			values3.second = values.second;
			values3.third = 0;	// 如果在这个路径下不需要第三个值
		}
		else
		{
			tower_long<ByteSwap>( ( const uint8_t* )bytes, length, seed, values3 );
			epi_long_128( values3 );
		}

		if ( is_little_endian() )
		{
			write_u64<false>( ( uint8_t* )result, values3.first );
			write_u64<false>( ( uint8_t* )result + 8, values3.second );
		}
		else
		{
			write_u64<true>( ( uint8_t* )result, values3.first );
			write_u64<true>( ( uint8_t* )result + 8, values3.second );
		}
	}

private:

	FORCE_INLINE void tower_layer_x( const size_t total_length, Values3<uint64_t>& data )
	{
		// 首先，`tower_long`不得内联，否则对于所有大小的输入都会变慢。
		// 这个函数如果放在`tower_loong`里，那么对于 bulk 而言总是能够提速 ~1 GiB/s。
		// 这个函数如果放在`epi_loong_*`里，那么对小于 16-bytes 的 key 而言会慢 ~2 cyc，对大于 16-bytes 的 key 而言会快 ~3 cyc。
		// 目前 MuseAir 最大的亮点是对 bulk 的处理速度，所以这个函数应该放在`tower_loong`里。
		// 这些特性可能是机器特定的，或与缓存性能相关。但我想，不论如何，想办法让`tower_short`能够处理更长的 key 才是最好的解决方案。

		auto& [ i, j, k ] = data;

		//x AND 0b111111
		size_t rotate = total_length & 63;

		chixx( data );

		i = rotate_left( i, rotate );
		j = rotate_right( j, rotate );
		k ^= total_length;

		Values2<uint64_t> data0 { 0, 0 };
		Values2<uint64_t> data1 { 0, 0 };
		Values2<uint64_t> data2 { 0, 0 };

		if ( !BlindFast )
		{
			multiple64_128bit( i ^ DEFAULT_SECRET[ 3 ], j, data0 );
			multiple64_128bit( j ^ DEFAULT_SECRET[ 4 ], k, data1 );
			multiple64_128bit( k ^ DEFAULT_SECRET[ 5 ], i, data2 );

			auto& [low0, high0] = data0;
			auto& [low1, high1] = data1;
			auto& [low2, high2] = data2;

			i ^= low0 ^ high2;
			j ^= low1 ^ high0;
			k ^= low2 ^ high1;
		}
		else
		{
			multiple64_128bit( i, j, data0 );
			multiple64_128bit( j, k, data1 );
			multiple64_128bit( k, i, data2 );

			auto& [low0, high0] = data0;
			auto& [low1, high1] = data1;
			auto& [low2, high2] = data2;

			i = low0 ^ high2;
			j = low1 ^ high0;
			k = low2 ^ high1;
		}
	}

	template <bool ByteSwap>
	FORCE_INLINE void tower_layer_3( State& state, const uint8_t* bytes )
	{
		Values2<uint64_t> output { 0, 0 };

		// First pair (state[0] and state[3])
		output = { state[ 0 ], state[ 3 ] };
		frac_3<BlindFast>( output, read_u64<ByteSwap>( bytes + seg( 0 ) ) );
		state[ 0 ] = output.first;
		state[ 3 ] = output.second;

		// Second pair (state[1] and state[4])
		output = { state[ 1 ], state[ 4 ] };
		frac_3<BlindFast>( output, read_u64<ByteSwap>( bytes + seg( 1 ) ) );
		state[ 1 ] = output.first;
		state[ 4 ] = output.second;

		// Third pair (state[2] and state[5])
		output = { state[ 2 ], state[ 5 ] };
		frac_3<BlindFast>( output, read_u64<ByteSwap>( bytes + seg( 2 ) ) );
		state[ 2 ] = output.first;
		state[ 5 ] = output.second;
	}

	template <bool ByteSwap>
	FORCE_INLINE void tower_layer_6( State& state, const uint8_t* bytes )
	{
		Values2<uint64_t> input { 0, 0 }, output { 0, 0 };

		// First pair (state[0] and state[1])
		input = { read_u64<ByteSwap>( bytes + seg( 0 ) ), read_u64<ByteSwap>( bytes + seg( 1 ) ) };
		output = { state[ 0 ], state[ 1 ] };
		frac_6<BlindFast>( output, input );
		state[ 0 ] = output.first;
		state[ 1 ] = output.second;

		// Second pair (state[2] and state[3])
		input = { read_u64<ByteSwap>( bytes + seg( 2 ) ), read_u64<ByteSwap>( bytes + seg( 3 ) ) };
		output = { state[ 2 ], state[ 3 ] };
		frac_6<BlindFast>( output, input );
		state[ 2 ] = output.first;
		state[ 3 ] = output.second;

		// Third pair (state[4] and state[5])
		input = { read_u64<ByteSwap>( bytes + seg( 4 ) ), read_u64<ByteSwap>( bytes + seg( 5 ) ) };
		output = { state[ 4 ], state[ 5 ] };
		frac_6<BlindFast>( output, input );
		state[ 4 ] = output.first;
		state[ 5 ] = output.second;
	}

	template <bool ByteSwap>
	NEVER_INLINE void tower_layer_12(State& state, const uint8_t* bytes, uint64_t& ring_prev )
	{
		Values2<uint64_t> result {0, 0};
		if constexpr ( !BlindFast )
		{
			state[ 0 ] ^= read_u64<ByteSwap>( bytes );
			state[ 1 ] ^= read_u64<ByteSwap>( bytes + 8 );
			multiple64_128bit( state[ 0 ], state[ 1 ], result );
			auto [low0, high0] = result;
			state[ 0 ] += ( ring_prev ^ high0 );

			state[ 1 ] ^= read_u64<ByteSwap>( bytes + 16 );
			state[ 2 ] ^= read_u64<ByteSwap>( bytes + 24 );
			multiple64_128bit( state[ 1 ], state[ 2 ], result );
			auto [low1, high1] = result;
			state[ 1 ] += ( low0 ^ high1 );

			state[ 2 ] ^= read_u64<ByteSwap>( bytes + 32 );
			state[ 3 ] ^= read_u64<ByteSwap>( bytes + 40 );
			multiple64_128bit( state[ 2 ], state[ 3 ], result );
			auto [low2, high2] = result;
			state[ 2 ] += ( low1 ^ high2 );

			state[ 3 ] ^= read_u64<ByteSwap>( bytes + 48 );
			state[ 4 ] ^= read_u64<ByteSwap>( bytes + 56 );
			multiple64_128bit( state[ 3 ], state[ 4 ], result );
			auto [low3, high3] = result;
			state[ 3 ] += ( low2 ^ high3 );

			state[ 4 ] ^= read_u64<ByteSwap>( bytes + 64 );
			state[ 5 ] ^= read_u64<ByteSwap>( bytes + 72 );
			multiple64_128bit( state[ 4 ], state[ 5 ], result );
			auto [low4, high4] = result;
			state[ 4 ] += ( low3 ^ high4 );

			state[ 5 ] ^= read_u64<ByteSwap>( bytes + 80 );
			state[ 0 ] ^= read_u64<ByteSwap>( bytes + 88 );
			multiple64_128bit( state[ 5 ], state[ 0 ], result );
			auto [low5, high5] = result;
			state[ 5 ] += ( low4 ^ high5 );

			ring_prev = low5;
		}
		else
		{
			auto& [low, high] = result;

			state[ 0 ] ^= read_u64<ByteSwap>( bytes );
			state[ 1 ] ^= read_u64<ByteSwap>( bytes + 8 );
			multiple64_128bit( state[ 0 ], state[ 1 ], result );
			auto [low0, high0] = result;
			state[ 0 ] = ( ring_prev ^ high0 );

			state[ 1 ] ^= read_u64<ByteSwap>( bytes + 16 );
			state[ 2 ] ^= read_u64<ByteSwap>( bytes + 24 );
			multiple64_128bit( state[ 1 ], state[ 2 ], result );
			auto [low1, high1] = result;
			state[ 1 ] = ( low0 ^ high1 );

			state[ 2 ] ^= read_u64<ByteSwap>( bytes + 32 );
			state[ 3 ] ^= read_u64<ByteSwap>( bytes + 40 );
			multiple64_128bit( state[ 2 ], state[ 3 ], result );
			auto [low2, high2] = result;
			state[ 2 ] = ( low1 ^ high2 );

			state[ 3 ] ^= read_u64<ByteSwap>( bytes + 48 );
			state[ 4 ] ^= read_u64<ByteSwap>( bytes + 56 );
			multiple64_128bit( state[ 3 ], state[ 4 ], result );
			auto [low3, high3] = result;
			state[ 3 ] = ( low2 ^ high3 );

			state[ 4 ] ^= read_u64<ByteSwap>( bytes + 64 );
			state[ 5 ] ^= read_u64<ByteSwap>( bytes + 72 );
			multiple64_128bit( state[ 4 ], state[ 5 ], result );
			auto [low4, high4] = result;
			state[ 4 ] = ( low3 ^ high4 );

			state[ 5 ] ^= read_u64<ByteSwap>( bytes + 80 );
			state[ 0 ] ^= read_u64<ByteSwap>( bytes + 88 );
			multiple64_128bit( state[ 5 ], state[ 0 ], result );
			auto [low5, high5] = result;
			state[ 5 ] = ( low4 ^ high5 );

			ring_prev = low5;
		}
	}

	template <bool ByteSwap>
	NEVER_INLINE void tower_long( const uint8_t* bytes, const size_t length, const uint64_t seed, Values3<uint64_t>& data )
	{
		auto& [i, j, k] = data;

		const uint8_t* p = bytes;
		size_t		   q = length;

		State state =
		{
			DEFAULT_SECRET[ 0 ] + seed,
			DEFAULT_SECRET[ 1 ] - seed,
			DEFAULT_SECRET[ 2 ] ^ seed,
			DEFAULT_SECRET[ 3 ],
			DEFAULT_SECRET[ 4 ],
			DEFAULT_SECRET[ 5 ]
		};

		if ( q >= seg( 12 ) )
		{
			state[ 3 ] += seed;
			state[ 4 ] -= seed;
			state[ 5 ] ^= seed;
			uint64_t ring_prev = INIT_RING_PREV;
			do
			{
				tower_layer_12<ByteSwap>( state, p, ring_prev );
				p += seg( 12 );
				q -= seg( 12 );
			} while ( LIKELY( q >= seg( 12 ) ) );
			
			state[ 0 ] ^= ring_prev;  // 如果将异或替换为加法，性能将下降约1.7% (p < 0.05)
		}

		if ( q >= seg( 6 ) )
		{
			tower_layer_6<ByteSwap>( state, p );
			p += seg( 6 );
			q -= seg( 6 );
		}

		if ( q >= seg( 3 ) )
		{
			tower_layer_3<ByteSwap>( state, p );
			p += seg( 3 );
			q -= seg( 3 );
		}

		tower_layer_0<ByteSwap>( state, p, q, length, data );
		tower_layer_x( length, data );
	}

	template <bool ByteSwap>
	FORCE_INLINE void tower_short( const uint8_t* bytes, const size_t len, const uint64_t seed, Values2<uint64_t>& data )
	{
		read_short<ByteSwap>( bytes, len, data );
		Values2<uint64_t> result {0, 0};
		multiple64_128bit( seed ^ DEFAULT_SECRET[ 0 ], len ^ DEFAULT_SECRET[ 1 ], result );
		auto& [low, high] = result;
		auto& [a, b] = data;
		a ^= low ^ len;
		b ^= high ^ seed;
	}

	static FORCE_INLINE void epi_short( Values2<uint64_t>& data )
	{
		auto& [a, b] = data;
		Values2<uint64_t> result {0, 0};
		a ^= DEFAULT_SECRET[ 2 ];
		b ^= DEFAULT_SECRET[ 3 ];
		multiple64_128bit(a, b, result);
		auto& [low, high] = result;
		a ^= low ^ DEFAULT_SECRET[ 4 ];
		b ^= high ^ DEFAULT_SECRET[ 5 ];
		multiple64_128bit(a, b, result );
		a ^= b ^ low ^ high;
	}

	static FORCE_INLINE void epi_short_128( Values2<uint64_t>& values )
	{
		auto& [ i, j ] = values;
		Values2<uint64_t> result0 {0, 0};
		Values2<uint64_t> result1 {0, 0};

		if ( !BlindFast )
		{
			multiple64_128bit( i ^ DEFAULT_SECRET[ 2 ], j, result0 );
			multiple64_128bit( i, j ^ DEFAULT_SECRET[ 3 ], result1 );

			auto& [lo0 , hi0] = result0;
			auto& [lo1 , hi1] = result1;

			i ^= lo0 ^ hi1;
			j ^= lo1 ^ hi0;

			multiple64_128bit( i ^ DEFAULT_SECRET[ 4 ], j, result0 );
			multiple64_128bit( i, j ^ DEFAULT_SECRET[ 5 ], result1 );

			i ^= lo0 ^ hi1;
			j ^= lo1 ^ hi0;
		}
		else
		{
			multiple64_128bit( i, j, result0 );
			multiple64_128bit( i ^ DEFAULT_SECRET[2], j ^ DEFAULT_SECRET[3], result1 );

			auto& [lo0 , hi0] = result0;
			auto& [lo1 , hi1] = result1;

			i ^= lo0 ^ hi1;
			j ^= lo1 ^ hi0;

			multiple64_128bit( i, j, result0 );
			multiple64_128bit( i ^ DEFAULT_SECRET[4], j ^ DEFAULT_SECRET[5], result1 );

			i ^= lo0 ^ hi1;
			j ^= lo1 ^ hi0;
		}
	}


	static FORCE_INLINE void epi_long( Values3<uint64_t>& values )
	{
		auto& [i, j, k] = values;
		
		Values2<uint64_t> result0, result1, result2;

		if (!BlindFast) {
			// 计算3次乘积，并保存结果
			multiple64_128bit(i ^ DEFAULT_SECRET[0], j, result0);
			multiple64_128bit(j ^ DEFAULT_SECRET[1], k, result1);
			multiple64_128bit(k ^ DEFAULT_SECRET[2], i, result2);
			
			// 使用结果更新 i, j, k
			i ^= result0.first ^ result2.second;
			j ^= result1.first ^ result0.second;
			k ^= result2.first ^ result1.second;
		} else {
			// 计算3次乘积，并保存结果
			multiple64_128bit(i, j, result0);
			multiple64_128bit(j, k, result1);
			multiple64_128bit(k, i, result2);
			
			// 使用结果更新 i, j, k
			i = result0.first ^ result2.second;
			j = result1.first ^ result0.second;
			k = result2.first ^ result1.second;
		}

		// 最后一步
		i += j + k;
	}

	static FORCE_INLINE void epi_long_128( Values3<uint64_t>& values )
	{
		auto& [i, j, k] = values;

		Values2<uint64_t> result0, result1, result2;

		if (!BlindFast) {
			// 计算3次乘积，并保存结果
			multiple64_128bit(i ^ DEFAULT_SECRET[0], j, result0);
			multiple64_128bit(j ^ DEFAULT_SECRET[1], k, result1);
			multiple64_128bit(k ^ DEFAULT_SECRET[2], i, result2);
			
			// 使用结果更新 i, j, k
			i ^= result0.first ^ result1.first ^ result2.second;
			j ^= result0.second ^ result1.second ^ result2.first;
		} else {
			// 计算3次乘积，并保存结果
			multiple64_128bit(i, j, result0);
			multiple64_128bit(j, k, result1);
			multiple64_128bit(k, i, result2);
			
			// 使用结果更新 i, j, k
			i = result0.first ^ result1.first ^ result2.second;
			j = result0.second ^ result1.second ^ result2.first;
		}
	}
};


#endif // NON_CRYPTOGRAPHIC_HASH_MUSE_AIR_HPP
#include "doctest.h"

#include "LibEntropy/Container/Array.hpp"
#include <vector>

using LibEntropy::Array;
using std::vector;

template<bool Reserved, bool NoExcept>
struct Params
{
	static constexpr bool reserved = Reserved;
	static constexpr bool no_except = NoExcept;
};

TYPE_TO_STRING_AS( "Unreserved, NoExcept", Params<false, true> );
TYPE_TO_STRING_AS( "Unreserved, Throwing", Params<false, false> );

TYPE_TO_STRING_AS( "Reserved, NoExcept", Params<true, true> );
TYPE_TO_STRING_AS( "Reserved, Throwing", Params<true, false> );

#define ALL_PARAMS Params<false, true>, Params<false, false>, Params<true, true>, Params<true, false>
#define RESERVED_PARAMS Params<true, true>, Params<true, false>

TEST_SUITE( "ArrayTest: NonTrival" )
{
	template<bool ArraySentinal, bool NoExcept>
	class NonTriv
	{
	public:
		//NonTriv() noexcept( NoExcept ) : mValue( 0 )
		//{
		//	++sNumConstructors;
		//}

		explicit NonTriv( int inValue ) noexcept( NoExcept ) : mValue( inValue )
		{
			++sNumConstructors;
		}

		NonTriv( const NonTriv &inValue ) noexcept( NoExcept ) : mValue( inValue.mValue )
		{
			++sNumCopyConstructors;
		}

		NonTriv( NonTriv &&inValue ) noexcept( NoExcept ) : mValue( inValue.mValue )
		{
			inValue.mValue = 0; ++sNumMoveConstructors;
		}

		~NonTriv()
		{
			++sNumDestructors;
		}

		NonTriv & operator = ( const NonTriv &inRHS ) noexcept( NoExcept )
		{
			mValue = inRHS.mValue;
			++sNumCopyAssigns;
			return *this;
		}

		NonTriv & operator = ( NonTriv &&inRHS ) noexcept( NoExcept )
		{
			mValue = inRHS.mValue;
			inRHS.mValue = 0;
			++sNumMoveAssigns;
			return *this;
		}

		bool operator == ( const NonTriv &inRHS ) const
		{
			return mValue == inRHS.mValue;
		}

		bool operator != ( const NonTriv &inRHS ) const
		{
			return mValue != inRHS.mValue;
		}

		int Value() const
		{
			return mValue;
		}

		static void sReset()
		{
			sNumConstructors = 0;
			sNumCopyConstructors = 0;
			sNumMoveConstructors = 0;
			sNumDestructors = 0;

			sNumCopyAssigns = 0;
			sNumMoveAssigns = 0;
		}

		static inline int sNumConstructors = 0;
		static inline int sNumCopyConstructors = 0;
		static inline int sNumMoveConstructors = 0;
		static inline int sNumDestructors = 0;

		static inline int sNumCopyAssigns = 0;
		static inline int sNumMoveAssigns = 0;

		int mValue;
	};

#define AV_WARN() \
	{ \
		WARN( AType::sNumConstructors == VType::sNumConstructors ); \
		WARN( AType::sNumCopyConstructors == VType::sNumCopyConstructors ); \
		WARN( AType::sNumMoveConstructors == VType::sNumMoveConstructors ); \
		WARN( AType::sNumDestructors == VType::sNumDestructors ); \
		WARN( AType::sNumCopyAssigns == VType::sNumCopyAssigns ); \
		WARN( AType::sNumMoveAssigns == VType::sNumMoveAssigns ); \
	}

#define AV_CHECK() \
	{ \
		CHECK( AType::sNumConstructors == VType::sNumConstructors ); \
		CHECK( AType::sNumCopyConstructors == VType::sNumCopyConstructors ); \
		CHECK( AType::sNumMoveConstructors == VType::sNumMoveConstructors ); \
		CHECK( AType::sNumDestructors == VType::sNumDestructors ); \
		CHECK( AType::sNumCopyAssigns == VType::sNumCopyAssigns ); \
		CHECK( AType::sNumMoveAssigns == VType::sNumMoveAssigns ); \
	}

	bool EnsureSame( const auto &a, const auto &v )
	{
		if ( a.size() != v.size() ) return false;

		for ( int i = 0; i < a.size(); ++i ) {
			if ( a[i].Value() != v[i].Value() ) return false;
		}

		return true;
	}

	TEST_CASE_TEMPLATE( "Emplace", T, ALL_PARAMS )
	{
		using AType = NonTriv<true, T::no_except>;
		using VType = NonTriv<false, T::no_except>;

		Array<AType> arr = { AType( 1 ), AType( 2 ), AType( 3 ) };
		vector<VType> vec = { VType( 1 ), VType( 2 ), VType( 3 ) };

		if ( T::reserved ) {
			arr.reserve( 4 );
			vec.reserve( 4 );
		}

		AType::sReset();
		VType::sReset();

		SUBCASE( "Begin" ) {
			arr.emplace( arr.begin(), 4 );
			vec.emplace( vec.begin(), 4 );

			AV_WARN();
			CHECK( EnsureSame( arr, vec ) );
		}

		SUBCASE( "Middle" ) {
			arr.emplace( arr.end() - 1, 4 );
			vec.emplace( vec.end() - 1, 4 );

			AV_WARN();
			CHECK( EnsureSame( arr, vec ) );
		}

		SUBCASE( "End" ) {
			arr.emplace( arr.end(), 4 );
			vec.emplace( vec.end(), 4 );

			AV_WARN();
			CHECK( EnsureSame( arr, vec ) );
		}

		AType::sReset();
		VType::sReset();
	}

	TEST_CASE_TEMPLATE( "InsertOne[Copy]", T, ALL_PARAMS )
	{
		using AType = NonTriv<true, T::no_except>;
		using VType = NonTriv<false, T::no_except>;

		Array<AType> arr = { AType( 1 ), AType( 2 ), AType( 3 ) };
		vector<VType> vec = { VType( 1 ), VType( 2 ), VType( 3 ) };

		auto a = AType( 4 );
		auto v = VType( 4 );

		if ( T::reserved ) {
			arr.reserve( 4 );
			vec.reserve( 4 );
		}

		AType::sReset();
		VType::sReset();

		SUBCASE( "Begin" ) {
			arr.insert( arr.begin(), a );
			vec.insert( vec.begin(), v );

			AV_WARN();
			CHECK( EnsureSame( arr, vec ) );
		}

		SUBCASE( "Middle" ) {
			arr.insert( arr.end() - 1, a );
			vec.insert( vec.end() - 1, v );

			AV_WARN();
			CHECK( EnsureSame( arr, vec ) );
		}

		SUBCASE( "End" ) {
			arr.insert( arr.end(), a );
			vec.insert( vec.end(), v );

			AV_WARN();
			CHECK( EnsureSame( arr, vec ) );
		}

		AType::sReset();
		VType::sReset();
	}

	TEST_CASE_TEMPLATE( "InsertOne[Move]", T, ALL_PARAMS )
	{
		using AType = NonTriv<true, T::no_except>;
		using VType = NonTriv<false, T::no_except>;

		Array<AType> arr = { AType( 1 ), AType( 2 ), AType( 3 ) };
		vector<VType> vec = { VType( 1 ), VType( 2 ), VType( 3 ) };

		auto a = AType( 4 );
		auto v = VType( 4 );

		if ( T::reserved ) {
			arr.reserve( 4 );
			vec.reserve( 4 );
		}

		AType::sReset();
		VType::sReset();

		SUBCASE( "Begin" ) {
			arr.insert( arr.begin(), std::move( a ) );
			vec.insert( vec.begin(), std::move( v ) );

			AV_WARN();
			CHECK( EnsureSame( arr, vec ) );
		}

		SUBCASE( "Middle" ) {
			arr.insert( arr.end() - 1, std::move( a ) );
			vec.insert( vec.end() - 1, std::move( v ) );

			AV_WARN();
			CHECK( EnsureSame( arr, vec ) );
		}

		SUBCASE( "End" ) {
			arr.insert( arr.end(), std::move( a ) );
			vec.insert( vec.end(), std::move( v ) );

			AV_WARN();
			CHECK( EnsureSame( arr, vec ) );
		}

		AType::sReset();
		VType::sReset();
	}

	TEST_CASE_TEMPLATE( "InsertThree[CopyN]", T, ALL_PARAMS )
	{
		using AType = NonTriv<true, T::no_except>;
		using VType = NonTriv<false, T::no_except>;

		Array<AType> arr = { AType( 1 ), AType( 2 ), AType( 3 ) };
		vector<VType> vec = { VType( 1 ), VType( 2 ), VType( 3 ) };

		auto a = AType( 4 );
		auto v = VType( 4 );

		if ( T::reserved ) {
			arr.reserve( 6 );
			vec.reserve( 6 );
		}

		AType::sReset();
		VType::sReset();

		SUBCASE( "Begin" ) {
			arr.insert( arr.begin(), 3, a );
			vec.insert( vec.begin(), 3, v );

			AV_WARN();
			CHECK( EnsureSame( arr, vec ) );
		}

		SUBCASE( "Middle" ) {
			arr.insert( arr.end() - 1, 3, a );
			vec.insert( vec.end() - 1, 3, v );

			AV_WARN();
			CHECK( EnsureSame( arr, vec ) );
		}

		SUBCASE( "End" ) {
			arr.insert( arr.end(), 3, a );
			vec.insert( vec.end(), 3, v );

			AV_WARN();
			CHECK( EnsureSame( arr, vec ) );
		}

		AType::sReset();
		VType::sReset();
	}

	TEST_CASE_TEMPLATE( "InsertThree[InitList]", T, ALL_PARAMS )
	{
		using AType = NonTriv<true, T::no_except>;
		using VType = NonTriv<false, T::no_except>;

		Array<AType> arr = { AType( 1 ), AType( 2 ), AType( 3 ) };
		vector<VType> vec = { VType( 1 ), VType( 2 ), VType( 3 ) };

		if ( T::reserved ) {
			arr.reserve( 6 );
			vec.reserve( 6 );
		}

		AType::sReset();
		VType::sReset();

		SUBCASE( "Begin" ) {
			arr.insert( arr.begin(), { AType( 4 ), AType( 5 ), AType( 6 ) } );
			vec.insert( vec.begin(), { VType( 4 ), VType( 5 ), VType( 6 ) } );

			AV_CHECK();
			CHECK( EnsureSame( arr, vec ) );
		}

		SUBCASE( "Middle" ) {
			arr.insert( arr.end() - 1, { AType( 4 ), AType( 5 ), AType( 6 ) } );
			vec.insert( vec.end() - 1, { VType( 4 ), VType( 5 ), VType( 6 ) } );

			AV_CHECK();
			CHECK( EnsureSame( arr, vec ) );
		}

		SUBCASE( "End" ) {
			arr.insert( arr.end(), { AType( 4 ), AType( 5 ), AType( 6 ) } );
			vec.insert( vec.end(), { VType( 4 ), VType( 5 ), VType( 6 ) } );

			AV_CHECK();
			CHECK( EnsureSame( arr, vec ) );
		}

		AType::sReset();
		VType::sReset();
	}
}
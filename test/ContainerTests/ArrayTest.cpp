#include "doctest.h"

#include "LibEntropy/Container/Array.hpp"
#include <vector>
#include <list>

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
//#define ALL_PARAMS Params<false, true>
//#define ALL_PARAMS Params<false, false>
//#define ALL_PARAMS Params<true, true>
//#define ALL_PARAMS Params<true, false>

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
			mValue = 0;
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

		auto abegin = a.begin();
		auto vbegin = v.begin();

		for ( ; abegin != a.end(); ++abegin, ++vbegin ) {
			if ( abegin->Value() != vbegin->Value() ) return false;
		}

		if ( abegin != a.end() ) return false;
		if ( vbegin != v.end() ) return false;

		return true;
	}

	TEST_CASE_TEMPLATE( "PushBack[Copy]", T, ALL_PARAMS )
	{
		using AType = NonTriv<true, T::no_except>;
		using VType = NonTriv<false, T::no_except>;

		Array<AType> arr;
		vector<VType> vec;

		int i = GENERATE( 1, 2, 3, 4 );

		SUBCASE( doctest::toString( i ) ) {

			if ( T::reserved ) {
				arr.reserve( i );
				vec.reserve( i );
			}

			AType::sReset();
			VType::sReset();

			for ( int j = 0; j < i; ++j ) {
				auto a = AType( j + 1 );
				auto v = VType( j + 1 );

				arr.push_back( a );
				vec.push_back( v );
			}

			AV_WARN();
			REQUIRE( EnsureSame( arr, vec ) );
		}

		AType::sReset();
		VType::sReset();
	}

	TEST_CASE_TEMPLATE( "PushBack[Move]", T, ALL_PARAMS )
	{
		using AType = NonTriv<true, T::no_except>;
		using VType = NonTriv<false, T::no_except>;

		Array<AType> arr;
		vector<VType> vec;

		int i = GENERATE( 1, 2, 3, 4 );

		SUBCASE( doctest::toString( i ) ) {

			if ( T::reserved ) {
				arr.reserve( i );
				vec.reserve( i );
			}

			AType::sReset();
			VType::sReset();

			for ( int j = 0; j < i; ++j ) {
				auto a = AType( j + 1 );
				auto v = VType( j + 1 );

				arr.push_back( std::move( a ) );
				vec.push_back( std::move( v ) );
			}

			AV_WARN();
			REQUIRE( EnsureSame( arr, vec ) );
		}

		AType::sReset();
		VType::sReset();
	}

	TEST_CASE_TEMPLATE( "EmplaceBack", T, ALL_PARAMS )
	{
		using AType = NonTriv<true, T::no_except>;
		using VType = NonTriv<false, T::no_except>;

		Array<AType> arr;
		vector<VType> vec;

		int i = GENERATE( 1, 2, 3, 4 );

		SUBCASE( doctest::toString( i ) ) {

			if ( T::reserved ) {
				arr.reserve( i );
				vec.reserve( i );
			}

			AType::sReset();
			VType::sReset();

			for ( int j = 0; j < i; ++j ) {
				arr.emplace_back( j + 1 );
				vec.emplace_back( j + 1 );
			}

			AV_WARN();
			REQUIRE( EnsureSame( arr, vec ) );
		}

		AType::sReset();
		VType::sReset();
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
			REQUIRE( EnsureSame( arr, vec ) );
		}

		SUBCASE( "Middle" ) {
			arr.emplace( arr.end() - 1, 4 );
			vec.emplace( vec.end() - 1, 4 );

			AV_WARN();
			REQUIRE( EnsureSame( arr, vec ) );
		}

		SUBCASE( "End" ) {
			arr.emplace( arr.end(), 4 );
			vec.emplace( vec.end(), 4 );

			AV_WARN();
			REQUIRE( EnsureSame( arr, vec ) );
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
			REQUIRE( EnsureSame( arr, vec ) );
		}

		SUBCASE( "Middle" ) {
			arr.insert( arr.end() - 1, a );
			vec.insert( vec.end() - 1, v );

			AV_WARN();
			REQUIRE( EnsureSame( arr, vec ) );
		}

		SUBCASE( "End" ) {
			arr.insert( arr.end(), a );
			vec.insert( vec.end(), v );

			AV_WARN();
			REQUIRE( EnsureSame( arr, vec ) );
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
			REQUIRE( EnsureSame( arr, vec ) );
		}

		SUBCASE( "Middle" ) {
			arr.insert( arr.end() - 1, std::move( a ) );
			vec.insert( vec.end() - 1, std::move( v ) );

			AV_WARN();
			REQUIRE( EnsureSame( arr, vec ) );
		}

		SUBCASE( "End" ) {
			arr.insert( arr.end(), std::move( a ) );
			vec.insert( vec.end(), std::move( v ) );

			AV_WARN();
			REQUIRE( EnsureSame( arr, vec ) );
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
			REQUIRE( EnsureSame( arr, vec ) );
		}

		SUBCASE( "Middle" ) {
			arr.insert( arr.end() - 1, 3, a );
			vec.insert( vec.end() - 1, 3, v );

			AV_WARN();
			REQUIRE( EnsureSame( arr, vec ) );
		}

		SUBCASE( "End" ) {
			arr.insert( arr.end(), 3, a );
			vec.insert( vec.end(), 3, v );

			AV_WARN();
			REQUIRE( EnsureSame( arr, vec ) );
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

			AV_WARN();
			REQUIRE( EnsureSame( arr, vec ) );
		}

		SUBCASE( "Middle" ) {
			arr.insert( arr.end() - 1, { AType( 4 ), AType( 5 ), AType( 6 ) } );
			vec.insert( vec.end() - 1, { VType( 4 ), VType( 5 ), VType( 6 ) } );

			AV_WARN();
			REQUIRE( EnsureSame( arr, vec ) );
		}

		SUBCASE( "End" ) {
			arr.insert( arr.end(), { AType( 4 ), AType( 5 ), AType( 6 ) } );
			vec.insert( vec.end(), { VType( 4 ), VType( 5 ), VType( 6 ) } );

			AV_WARN();
			REQUIRE( EnsureSame( arr, vec ) );
		}

		AType::sReset();
		VType::sReset();
	}

	TEST_CASE_TEMPLATE( "InsertThree[FromVector]", T, ALL_PARAMS )
	{
		using AType = NonTriv<true, T::no_except>;
		using VType = NonTriv<false, T::no_except>;

		Array<AType> arr = { AType( 1 ), AType( 2 ), AType( 3 ) };
		vector<VType> vec = { VType( 1 ), VType( 2 ), VType( 3 ) };

		Array<AType> arrl = { AType( 4 ), AType( 5 ), AType( 6 ) };
		vector<VType> vecl = { VType( 4 ), VType( 5 ), VType( 6 ) };

		if ( T::reserved ) {
			arr.reserve( 6 );
			vec.reserve( 6 );
		}

		AType::sReset();
		VType::sReset();

		SUBCASE( "Begin" ) {
			arr.insert( arr.begin(), arrl.begin(), arrl.end() );
			vec.insert( vec.begin(), vecl.begin(), vecl.end() );

			AV_WARN();
			REQUIRE( EnsureSame( arr, vec ) );
		}

		SUBCASE( "Middle" ) {
			arr.insert( arr.end() - 1, arrl.begin(), arrl.end() );
			vec.insert( vec.end() - 1, vecl.begin(), vecl.end() );

			AV_WARN();
			REQUIRE( EnsureSame( arr, vec ) );
		}

		SUBCASE( "End" ) {
			arr.insert( arr.end(), arrl.begin(), arrl.end() );
			vec.insert( vec.end(), vecl.begin(), vecl.end() );

			AV_WARN();
			REQUIRE( EnsureSame( arr, vec ) );
		}

		AType::sReset();
		VType::sReset();
	}

	TEST_CASE_TEMPLATE( "InsertThree[FromList]", T, ALL_PARAMS )
	{
		using AType = NonTriv<true, T::no_except>;
		using VType = NonTriv<false, T::no_except>;

		Array<AType> arr = { AType( 1 ), AType( 2 ), AType( 3 ) };
		vector<VType> vec = { VType( 1 ), VType( 2 ), VType( 3 ) };

		std::list<AType> arrl = { AType( 4 ), AType( 5 ), AType( 6 ) };
		std::list<VType> vecl = { VType( 4 ), VType( 5 ), VType( 6 ) };

		if ( T::reserved ) {
			arr.reserve( 6 );
			vec.reserve( 6 );
		}

		AType::sReset();
		VType::sReset();

		SUBCASE( "Begin" ) {
			arr.insert( arr.begin(), arrl.begin(), arrl.end() );
			vec.insert( vec.begin(), vecl.begin(), vecl.end() );

			AV_WARN();
			REQUIRE( EnsureSame( arr, vec ) );
		}

		SUBCASE( "Middle" ) {
			arr.insert( arr.end() - 1, arrl.begin(), arrl.end() );
			vec.insert( vec.end() - 1, vecl.begin(), vecl.end() );

			AV_WARN();
			REQUIRE( EnsureSame( arr, vec ) );
		}

		SUBCASE( "End" ) {
			arr.insert( arr.end(), arrl.begin(), arrl.end() );
			vec.insert( vec.end(), vecl.begin(), vecl.end() );

			AV_WARN();
			REQUIRE( EnsureSame( arr, vec ) );
		}

		AType::sReset();
		VType::sReset();
	}

	TEST_CASE_TEMPLATE( "InsertThree[MoveFromVector]", T, ALL_PARAMS )
	{
		using AType = NonTriv<true, T::no_except>;
		using VType = NonTriv<false, T::no_except>;

		Array<AType> arr = { AType( 1 ), AType( 2 ), AType( 3 ) };
		vector<VType> vec = { VType( 1 ), VType( 2 ), VType( 3 ) };

		Array<AType> arrl = { AType( 4 ), AType( 5 ), AType( 6 ) };
		vector<VType> vecl = { VType( 4 ), VType( 5 ), VType( 6 ) };

		if ( T::reserved ) {
			arr.reserve( 6 );
			vec.reserve( 6 );
		}

		AType::sReset();
		VType::sReset();

		SUBCASE( "Begin" ) {
			arr.insert( arr.begin(), std::make_move_iterator( arrl.begin() ), std::make_move_iterator( arrl.end() ) );
			vec.insert( vec.begin(), std::make_move_iterator( vecl.begin() ), std::make_move_iterator( vecl.end() ) );

			AV_WARN();
			REQUIRE( EnsureSame( arr, vec ) );
			REQUIRE( EnsureSame( arrl, vecl ) );
		}

		SUBCASE( "Middle" ) {
			arr.insert( arr.end() - 1, std::make_move_iterator( arrl.begin() ), std::make_move_iterator( arrl.end() ) );
			vec.insert( vec.end() - 1, std::make_move_iterator( vecl.begin() ), std::make_move_iterator( vecl.end() ) );

			AV_WARN();
			REQUIRE( EnsureSame( arr, vec ) );
			REQUIRE( EnsureSame( arrl, vecl ) );
		}

		SUBCASE( "End" ) {
			arr.insert( arr.end(), std::make_move_iterator( arrl.begin() ), std::make_move_iterator( arrl.end() ) );
			vec.insert( vec.end(), std::make_move_iterator( vecl.begin() ), std::make_move_iterator( vecl.end() ) );

			AV_WARN();
			REQUIRE( EnsureSame( arr, vec ) );
			REQUIRE( EnsureSame( arrl, vecl ) );
		}

		AType::sReset();
		VType::sReset();
	}

	TEST_CASE_TEMPLATE( "InsertThree[MoveFromList]", T, ALL_PARAMS )
	{
		using AType = NonTriv<true, T::no_except>;
		using VType = NonTriv<false, T::no_except>;

		Array<AType> arr = { AType( 1 ), AType( 2 ), AType( 3 ) };
		vector<VType> vec = { VType( 1 ), VType( 2 ), VType( 3 ) };

		std::list<AType> arrl = { AType( 4 ), AType( 5 ), AType( 6 ) };
		std::list<VType> vecl = { VType( 4 ), VType( 5 ), VType( 6 ) };

		if ( T::reserved ) {
			arr.reserve( 6 );
			vec.reserve( 6 );
		}

		AType::sReset();
		VType::sReset();

		SUBCASE( "Begin" ) {
			arr.insert( arr.begin(), std::make_move_iterator( arrl.begin() ), std::make_move_iterator( arrl.end() ) );
			vec.insert( vec.begin(), std::make_move_iterator( vecl.begin() ), std::make_move_iterator( vecl.end() ) );

			AV_WARN();
			REQUIRE( EnsureSame( arr, vec ) );
			REQUIRE( EnsureSame( arrl, vecl ) );
		}

		SUBCASE( "Middle" ) {
			arr.insert( arr.end() - 1, std::make_move_iterator( arrl.begin() ), std::make_move_iterator( arrl.end() ) );
			vec.insert( vec.end() - 1, std::make_move_iterator( vecl.begin() ), std::make_move_iterator( vecl.end() ) );

			AV_WARN();
			REQUIRE( EnsureSame( arr, vec ) );
			REQUIRE( EnsureSame( arrl, vecl ) );
		}

		SUBCASE( "End" ) {
			arr.insert( arr.end(), std::make_move_iterator( arrl.begin() ), std::make_move_iterator( arrl.end() ) );
			vec.insert( vec.end(), std::make_move_iterator( vecl.begin() ), std::make_move_iterator( vecl.end() ) );

			AV_WARN();
			REQUIRE( EnsureSame( arr, vec ) );
			REQUIRE( EnsureSame( arrl, vecl ) );
		}

		AType::sReset();
		VType::sReset();
	}

	template<class Iter>
	class EvilIterator : public Iter
	{
	public:
		using iterator_category = std::input_iterator_tag;
		using iterator_concept  = std::input_iterator_tag;

		using value_type = typename Iter::value_type;
		using difference_type = typename Iter::difference_type;
		using pointer = typename Iter::pointer;
		using reference = typename Iter::reference;

		EvilIterator( Iter inIter ) : Iter( inIter ) {}
	};

	TEST_CASE_TEMPLATE( "InsertThree[MoveFromVectorInput]", T, ALL_PARAMS )
	{
		using AType = NonTriv<true, T::no_except>;
		using VType = NonTriv<false, T::no_except>;

		Array<AType> arr = { AType( 1 ), AType( 2 ), AType( 3 ) };
		vector<VType> vec = { VType( 1 ), VType( 2 ), VType( 3 ) };

		Array<AType> arrl = { AType( 4 ), AType( 5 ), AType( 6 ) };
		vector<VType> vecl = { VType( 4 ), VType( 5 ), VType( 6 ) };

		if ( T::reserved ) {
			arr.reserve( 6 );
			vec.reserve( 6 );
		}

		AType::sReset();
		VType::sReset();

		SUBCASE( "Begin" ) {
			arr.insert( arr.begin(), EvilIterator( std::make_move_iterator( arrl.begin() ) ), EvilIterator( std::make_move_iterator( arrl.end() ) ) );
			vec.insert( vec.begin(), EvilIterator( std::make_move_iterator( vecl.begin() ) ), EvilIterator( std::make_move_iterator( vecl.end() ) ) );

			AV_WARN();
			REQUIRE( EnsureSame( arr, vec ) );
			REQUIRE( EnsureSame( arrl, vecl ) );
		}

		SUBCASE( "Middle" ) {
			arr.insert( arr.end() - 1, EvilIterator( std::make_move_iterator( arrl.begin() ) ), EvilIterator( std::make_move_iterator( arrl.end() ) ) );
			vec.insert( vec.end() - 1, EvilIterator( std::make_move_iterator( vecl.begin() ) ), EvilIterator( std::make_move_iterator( vecl.end() ) ) );

			AV_WARN();
			REQUIRE( EnsureSame( arr, vec ) );
			REQUIRE( EnsureSame( arrl, vecl ) );
		}

		SUBCASE( "End" ) {
			arr.insert( arr.end(), EvilIterator( std::make_move_iterator( arrl.begin() ) ), EvilIterator( std::make_move_iterator( arrl.end() ) ) );
			vec.insert( vec.end(), EvilIterator( std::make_move_iterator( vecl.begin() ) ), EvilIterator( std::make_move_iterator( vecl.end() ) ) );

			AV_WARN();
			REQUIRE( EnsureSame( arr, vec ) );
			REQUIRE( EnsureSame( arrl, vecl ) );
		}

		AType::sReset();
		VType::sReset();
	}

	TEST_CASE_TEMPLATE( "InsertThree[MoveFromListInput]", T, ALL_PARAMS )
	{
		using AType = NonTriv<true, T::no_except>;
		using VType = NonTriv<false, T::no_except>;

		Array<AType> arr = { AType( 1 ), AType( 2 ), AType( 3 ) };
		vector<VType> vec = { VType( 1 ), VType( 2 ), VType( 3 ) };

		std::list<AType> arrl = { AType( 4 ), AType( 5 ), AType( 6 ) };
		std::list<VType> vecl = { VType( 4 ), VType( 5 ), VType( 6 ) };

		if ( T::reserved ) {
			arr.reserve( 6 );
			vec.reserve( 6 );
		}

		AType::sReset();
		VType::sReset();

		SUBCASE( "Begin" ) {
			arr.insert( arr.begin(), EvilIterator( std::make_move_iterator( arrl.begin() ) ), EvilIterator( std::make_move_iterator( arrl.end() ) ) );
			vec.insert( vec.begin(), EvilIterator( std::make_move_iterator( vecl.begin() ) ), EvilIterator( std::make_move_iterator( vecl.end() ) ) );

			AV_WARN();
			REQUIRE( EnsureSame( arr, vec ) );
			REQUIRE( EnsureSame( arrl, vecl ) );
		}

		SUBCASE( "Middle" ) {
			arr.insert( arr.end() - 1, EvilIterator( std::make_move_iterator( arrl.begin() ) ), EvilIterator( std::make_move_iterator( arrl.end() ) ) );
			vec.insert( vec.end() - 1, EvilIterator( std::make_move_iterator( vecl.begin() ) ), EvilIterator( std::make_move_iterator( vecl.end() ) ) );

			AV_WARN();
			REQUIRE( EnsureSame( arr, vec ) );
			REQUIRE( EnsureSame( arrl, vecl ) );
		}

		SUBCASE( "End" ) {
			arr.insert( arr.end(), EvilIterator( std::make_move_iterator( arrl.begin() ) ), EvilIterator( std::make_move_iterator( arrl.end() ) ) );
			vec.insert( vec.end(), EvilIterator( std::make_move_iterator( vecl.begin() ) ), EvilIterator( std::make_move_iterator( vecl.end() ) ) );

			AV_WARN();
			REQUIRE( EnsureSame( arr, vec ) );
			REQUIRE( EnsureSame( arrl, vecl ) );
		}

		AType::sReset();
		VType::sReset();
	}

	TEST_CASE_TEMPLATE( "EraseOne", T, RESERVED_PARAMS )
	{
		using AType = NonTriv<true, T::no_except>;
		using VType = NonTriv<false, T::no_except>;

		Array<AType> arr = { AType( 1 ), AType( 2 ), AType( 3 ) };
		vector<VType> vec = { VType( 1 ), VType( 2 ), VType( 3 ) };

		AType::sReset();
		VType::sReset();

		SUBCASE( "Begin" ) {
			arr.erase( arr.begin() );
			vec.erase( vec.begin() );

			AV_WARN();
			REQUIRE( EnsureSame( arr, vec ) );
		}

		SUBCASE( "Middle" ) {
			arr.erase( arr.end() - 2 );
			vec.erase( vec.end() - 2 );

			AV_WARN();
			REQUIRE( EnsureSame( arr, vec ) );
		}

		SUBCASE( "End" ) {
			arr.erase( arr.end() - 1 );
			vec.erase( vec.end() - 1 );

			AV_WARN();
			REQUIRE( EnsureSame( arr, vec ) );
		}

		AType::sReset();
		VType::sReset();
	}

	TEST_CASE_TEMPLATE( "EraseOne[Multi]", T, RESERVED_PARAMS )
	{
		using AType = NonTriv<true, T::no_except>;
		using VType = NonTriv<false, T::no_except>;

		Array<AType> arr = { AType( 1 ), AType( 2 ), AType( 3 ) };
		vector<VType> vec = { VType( 1 ), VType( 2 ), VType( 3 ) };

		AType::sReset();
		VType::sReset();

		SUBCASE( "Begin" ) {
			arr.erase( arr.begin(), arr.begin() + 1 );
			vec.erase( vec.begin(), vec.begin() + 1 );

			AV_WARN();
			REQUIRE( EnsureSame( arr, vec ) );
		}

		SUBCASE( "Middle" ) {
			arr.erase( arr.end() - 2, arr.end() - 1 );
			vec.erase( vec.end() - 2, vec.end() - 1 );

			AV_WARN();
			REQUIRE( EnsureSame( arr, vec ) );
		}

		SUBCASE( "End" ) {
			arr.erase( arr.end() - 1, arr.end() );
			vec.erase( vec.end() - 1, vec.end() );

			AV_WARN();
			REQUIRE( EnsureSame( arr, vec ) );
		}

		AType::sReset();
		VType::sReset();
	}

	TEST_CASE_TEMPLATE( "EraseMulti", T, RESERVED_PARAMS )
	{
		using AType = NonTriv<true, T::no_except>;
		using VType = NonTriv<false, T::no_except>;

		Array<AType> arr = { AType( 1 ), AType( 2 ), AType( 3 ), AType( 4 ) };
		vector<VType> vec = { VType( 1 ), VType( 2 ), VType( 3 ), VType( 4 ) };

		AType::sReset();
		VType::sReset();

		SUBCASE( "Begin" ) {
			arr.erase( arr.begin(), arr.begin() + 2 );
			vec.erase( vec.begin(), vec.begin() + 2 );

			AV_WARN();
			REQUIRE( EnsureSame( arr, vec ) );
		}

		SUBCASE( "Middle" ) {
			arr.erase( arr.begin() + 1, arr.begin() + 3 );
			vec.erase( vec.begin() + 1, vec.begin() + 3 );

			AV_WARN();
			REQUIRE( EnsureSame( arr, vec ) );
		}

		SUBCASE( "End" ) {
			arr.erase( arr.end() - 2, arr.end() );
			vec.erase( vec.end() - 2, vec.end() );

			AV_WARN();
			REQUIRE( EnsureSame( arr, vec ) );
		}

		SUBCASE( "All" ) {
			arr.erase( arr.begin(), arr.end() );
			vec.erase( vec.begin(), vec.end() );

			AV_WARN();
			REQUIRE( EnsureSame( arr, vec ) );
		}

		AType::sReset();
		VType::sReset();
	}

	TEST_CASE_TEMPLATE( "PopBack", T, RESERVED_PARAMS )
	{
		using AType = NonTriv<true, T::no_except>;
		using VType = NonTriv<false, T::no_except>;

		Array<AType> arr = { AType( 1 ), AType( 2 ), AType( 3 ), AType( 4 ) };
		vector<VType> vec = { VType( 1 ), VType( 2 ), VType( 3 ), VType( 4 ) };

		AType::sReset();
		VType::sReset();

		int i = GENERATE( 1, 2, 3, 4 );

		SUBCASE( doctest::toString( i ) ) {
			for ( int j = 0; j < i; ++j ) {
				arr.pop_back();
				vec.pop_back();
			}

			AV_WARN();
			REQUIRE( EnsureSame( arr, vec ) );
		}

		AType::sReset();
		VType::sReset();
	}

	// Clear

	// Reserve

	// Resize

	// Assignment

	// Copy/Move construction
}
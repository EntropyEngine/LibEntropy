#include "doctest.h"

#include "LibEntropy/Container/Array.hpp"
#include <vector>
#include <list>

using LibEntropy::Array;
using std::vector;



TEST_SUITE( "ArrayTest/ModifiedJoltSuite" ) // Modified version of https://github.com/jrouwe/JoltPhysics/blob/f73a3c8c3510400da3994aa39949dacea2780bd9/UnitTests/Core/ArrayTest.cpp
{
	// A test class that is non-trivially copyable to test if the Array class correctly constructs/destructs/copies and moves elements.
	class NonTriv
	{
	public:
		NonTriv() : mValue(0)										{ ++sNumConstructors; }
		explicit			NonTriv(int inValue) : mValue(inValue)						{ ++sNumConstructors; }
		NonTriv(const NonTriv &inValue) noexcept : mValue(inValue.mValue)	{ ++sNumCopyConstructors; }
		NonTriv(NonTriv &&inValue) noexcept : mValue(inValue.mValue)			{ inValue.mValue = 0; ++sNumMoveConstructors; }
		~NonTriv()													{ ++sNumDestructors; }

		NonTriv &			operator = (const NonTriv &inRHS)							{ mValue = inRHS.mValue; return *this; }

		bool				operator == (const NonTriv &inRHS) const					{ return mValue == inRHS.mValue; }
		bool				operator != (const NonTriv &inRHS) const					{ return mValue != inRHS.mValue; }

		int					Value() const												{ return mValue; }

		static void			sReset()													{ sNumConstructors = 0; sNumCopyConstructors = 0; sNumMoveConstructors = 0; sNumDestructors = 0; }

		static inline int	sNumConstructors = 0;
		static inline int	sNumCopyConstructors = 0;
		static inline int	sNumMoveConstructors = 0;
		static inline int	sNumDestructors = 0;

		int					mValue;
	};

	TEST_CASE("TestConstructLength")
	{
		Array<int> arr(55);
		CHECK(arr.size() == 55);
	}

	TEST_CASE("TestConstructLengthNonTriv")
	{
		NonTriv::sReset();
		Array<NonTriv> arr(55);
		CHECK(arr.size() == 55);
		CHECK(NonTriv::sNumConstructors == 55);
		CHECK(NonTriv::sNumCopyConstructors == 0);
		CHECK(NonTriv::sNumMoveConstructors == 0);
		CHECK(NonTriv::sNumDestructors == 0);
	}

	TEST_CASE("TestConstructValue")
	{
		Array<int> arr(5, 55);
		CHECK(arr.size() == 5);
		for (int i = 0; i < 5; ++i)
			CHECK(arr[i] == 55);
	}

	TEST_CASE("TestConstructValueNonTriv")
	{
		NonTriv v(55);
		NonTriv::sReset();
		Array<NonTriv> arr(5, v);
		CHECK(arr.size() == 5);
		for (int i = 0; i < 5; ++i)
			CHECK(arr[i].Value() == 55);
		CHECK(NonTriv::sNumConstructors == 0);
		CHECK(NonTriv::sNumCopyConstructors == 5);
		CHECK(NonTriv::sNumMoveConstructors == 0);
		CHECK(NonTriv::sNumDestructors == 0);
	}

	TEST_CASE("TestConstructIterator")
	{
		int values[] = { 1, 2, 3 };

		Array<int> arr(values, values + 3);
		CHECK(arr.size() == 3);
		CHECK(arr[0] == 1);
		CHECK(arr[1] == 2);
		CHECK(arr[2] == 3);
	}

	TEST_CASE("TestConstructIteratorNonTriv")
	{
		NonTriv values[] = { NonTriv(1), NonTriv(2), NonTriv(3) };

		NonTriv::sReset();
		Array<NonTriv> arr(values, values + 3);
		CHECK(arr.size() == 3);
		CHECK(arr[0].Value() == 1);
		CHECK(arr[1].Value() == 2);
		CHECK(arr[2].Value() == 3);
		CHECK(NonTriv::sNumConstructors == 0);
		CHECK(NonTriv::sNumCopyConstructors == 3);
		CHECK(NonTriv::sNumMoveConstructors == 0);
		CHECK(NonTriv::sNumDestructors == 0);
	}

	TEST_CASE("TestConstructInitializerList")
	{
		Array<int> arr({ 1, 2, 3 });
		CHECK(arr.size() == 3);
		CHECK(arr[0] == 1);
		CHECK(arr[1] == 2);
		CHECK(arr[2] == 3);
	}

	TEST_CASE("TestConstructInitializerListNonTriv")
	{
		NonTriv::sReset();
		Array<NonTriv> arr({ NonTriv(1), NonTriv(2), NonTriv(3) });
		CHECK(arr.size() == 3);
		CHECK(arr[0].Value() == 1);
		CHECK(arr[1].Value() == 2);
		CHECK(arr[2].Value() == 3);
		CHECK(NonTriv::sNumConstructors == 3); // For the initializer list
		CHECK(NonTriv::sNumCopyConstructors == 3); // Initializing the array
		CHECK(NonTriv::sNumMoveConstructors == 0);
		CHECK(NonTriv::sNumDestructors == 3); // For the initializer list
	}

	TEST_CASE("TestConstructFromArray")
	{
		Array<int> arr = { 1, 2, 3 };
		Array<int> arr2(arr);
		CHECK(arr2.size() == 3);
		CHECK(arr2[0] == 1);
		CHECK(arr2[1] == 2);
		CHECK(arr2[2] == 3);
	}

	TEST_CASE("TestConstructFromArrayNonTriv")
	{
		Array<NonTriv> arr = { NonTriv(1), NonTriv(2), NonTriv(3) };
		NonTriv::sReset();
		Array<NonTriv> arr2(arr);
		CHECK(arr2.size() == 3);
		CHECK(arr2[0].Value() == 1);
		CHECK(arr2[1].Value() == 2);
		CHECK(arr2[2].Value() == 3);
		CHECK(NonTriv::sNumConstructors == 0);
		CHECK(NonTriv::sNumCopyConstructors == 3);
		CHECK(NonTriv::sNumMoveConstructors == 0);
		CHECK(NonTriv::sNumDestructors == 0);
	}

	TEST_CASE("TestMoveFromArray")
	{
		Array<int> arr = { 1, 2, 3 };
		Array<int> arr2(std::move(arr));
		CHECK(arr2.size() == 3);
		CHECK(arr2[0] == 1);
		CHECK(arr2[1] == 2);
		CHECK(arr2[2] == 3);
		CHECK(arr.size() == 0);
		CHECK(arr.capacity() == 0);
	}

	TEST_CASE("TestMoveFromArrayNonTriv")
	{
		Array<NonTriv> arr = { NonTriv(1), NonTriv(2), NonTriv(3) };
		NonTriv::sReset();
		Array<NonTriv> arr2(std::move(arr)); // This just updates the mElements pointer so should not call any constructors/destructors etc.
		CHECK(arr2.size() == 3);
		CHECK(arr2[0].Value() == 1);
		CHECK(arr2[1].Value() == 2);
		CHECK(arr2[2].Value() == 3);
		CHECK(arr.size() == 0);
		CHECK(arr.capacity() == 0);
		CHECK(NonTriv::sNumConstructors == 0);
		CHECK(NonTriv::sNumCopyConstructors == 0);
		CHECK(NonTriv::sNumMoveConstructors == 0);
		CHECK(NonTriv::sNumDestructors == 0);
	}

	TEST_CASE("TestClear")
	{
		Array<int> arr({ 1, 2, 3 });
		CHECK(arr.size() == 3);
		arr.clear();
		CHECK(arr.size() == 0);
	}

	TEST_CASE("TestClearNonTriv")
	{
		NonTriv::sReset();
		Array<NonTriv> arr({ NonTriv(1), NonTriv(2), NonTriv(3) });
		CHECK(arr.size() == 3);
		CHECK(NonTriv::sNumConstructors == 3); // For initializer list
		CHECK(NonTriv::sNumCopyConstructors == 3); // To move into array
		CHECK(NonTriv::sNumMoveConstructors == 0);
		CHECK(NonTriv::sNumDestructors == 3); // For initializer list
		NonTriv::sReset();
		arr.clear();
		CHECK(arr.size() == 0);
		CHECK(NonTriv::sNumConstructors == 0);
		CHECK(NonTriv::sNumCopyConstructors == 0);
		CHECK(NonTriv::sNumMoveConstructors == 0);
		CHECK(NonTriv::sNumDestructors == 3);
	}

	TEST_CASE("TestPushBack")
	{
		Array<int> arr;
		CHECK(arr.size() == 0);
		CHECK(arr.capacity() == 0);

		arr.push_back(1);
		CHECK(arr.size() == 1);
		CHECK(arr[0] == 1);

		arr.push_back(2);
		CHECK(arr.size() == 2);
		CHECK(arr[0] == 1);
		CHECK(arr[1] == 2);

		arr.pop_back();
		CHECK(arr.size() == 1);

		arr.pop_back();
		CHECK(arr.size() == 0);
		CHECK(arr.empty());
	}

	TEST_CASE("TestPushBackNonTriv")
	{
		NonTriv v1(1);
		NonTriv v2(2);

		NonTriv::sReset();
		Array<NonTriv> arr;
		CHECK(arr.size() == 0);
		CHECK(arr.capacity() == 0);
		CHECK(NonTriv::sNumConstructors == 0);
		CHECK(NonTriv::sNumCopyConstructors == 0);
		CHECK(NonTriv::sNumMoveConstructors == 0);
		CHECK(NonTriv::sNumDestructors == 0);

		NonTriv::sReset();
		arr.push_back(v1);
		CHECK(arr.size() == 1);
		CHECK(arr[0].Value() == 1);
		CHECK(NonTriv::sNumConstructors == 0);
		CHECK(NonTriv::sNumCopyConstructors == 1);
		CHECK(NonTriv::sNumMoveConstructors == 0);
		CHECK(NonTriv::sNumDestructors == 0);

		NonTriv::sReset();
		arr.push_back(v2);
		CHECK(arr.size() == 2);
		CHECK(arr[0].Value() == 1);
		CHECK(arr[1].Value() == 2);
	//#ifndef JPH_USE_STD_VECTOR
		CHECK(NonTriv::sNumConstructors == 0);
		CHECK(NonTriv::sNumCopyConstructors == 1);
		CHECK(NonTriv::sNumMoveConstructors == 1); // Array resizes from 1 to 2
		CHECK(NonTriv::sNumDestructors == 1); // Array resizes from 1 to 2
	//#endif // JPH_USE_STD_VECTOR

		NonTriv::sReset();
		arr.pop_back();
		CHECK(arr.size() == 1);
		CHECK(NonTriv::sNumConstructors == 0);
		CHECK(NonTriv::sNumCopyConstructors == 0);
		CHECK(NonTriv::sNumMoveConstructors == 0);
		CHECK(NonTriv::sNumDestructors == 1);

		NonTriv::sReset();
		arr.pop_back();
		CHECK(arr.size() == 0);
		CHECK(arr.empty());
		CHECK(NonTriv::sNumConstructors == 0);
		CHECK(NonTriv::sNumCopyConstructors == 0);
		CHECK(NonTriv::sNumMoveConstructors == 0);
		CHECK(NonTriv::sNumDestructors == 1);
	}

	TEST_CASE("TestPushBackMove")
	{
		Array<Array<int>> arr;
		Array<int> arr2 = { 1, 2, 3 };
		arr.push_back(std::move(arr2));
		CHECK(arr2.size() == 0);
		CHECK(arr[0] == Array<int>({ 1, 2, 3 }));
	}

	TEST_CASE("TestEmplaceBack")
	{
		struct Test
		{
			Test(int inA, int inB) : mA(inA), mB(inB) { }

			int mA;
			int mB;
		};

		Array<Test> arr;
		arr.emplace_back(1, 2);
		CHECK(arr.size() == 1);
		CHECK(arr[0].mA == 1);
		CHECK(arr[0].mB == 2);
	}

	TEST_CASE("TestReserve")
	{
		Array<int> arr;
		CHECK(arr.capacity() == 0);

		arr.reserve(123);
		CHECK(arr.size() == 0);
		CHECK(arr.capacity() == 123);

		arr.reserve(456);
		CHECK(arr.size() == 0);
		CHECK(arr.capacity() == 456);
	}

	TEST_CASE("TestReserveNonTriv")
	{
		NonTriv::sReset();

		Array<NonTriv> arr;
		CHECK(arr.capacity() == 0);

		arr.reserve(123);
		CHECK(arr.size() == 0);
		CHECK(arr.capacity() == 123);

		arr.reserve(456);
		CHECK(arr.size() == 0);
		CHECK(arr.capacity() == 456);

		CHECK(NonTriv::sNumConstructors == 0);
		CHECK(NonTriv::sNumCopyConstructors == 0);
		CHECK(NonTriv::sNumMoveConstructors == 0);
		CHECK(NonTriv::sNumDestructors == 0);
	}

	TEST_CASE("TestResize")
	{
		Array<int> arr;
		CHECK(arr.size() == 0);
		CHECK(arr.capacity() == 0);
		CHECK(arr.data() == nullptr);

		arr.resize(0);
		CHECK(arr.size() == 0);
		CHECK(arr.capacity() == 0);
		CHECK(arr.data() == nullptr);

		arr.resize(123);
		CHECK(arr.size() == 123);
		CHECK(arr.capacity() == 123);
		for (int i = 0; i < 123; ++i)
			arr[i] = i;

		arr.resize(456);
		CHECK(arr.size() == 456);
		CHECK(arr.capacity() == 456);
		for (int i = 0; i < 123; ++i)
			CHECK(arr[i] == i);

		arr.resize(10);
		CHECK(arr.size() == 10);
		CHECK(arr.capacity() >= 10);
	}

	TEST_CASE("TestResizeNonTriv")
	{
		NonTriv::sReset();
		Array<NonTriv> arr;
		CHECK(arr.size() == 0);
		CHECK(arr.capacity() == 0);
		CHECK(NonTriv::sNumConstructors == 0);
		CHECK(NonTriv::sNumCopyConstructors == 0);
		CHECK(NonTriv::sNumMoveConstructors == 0);
		CHECK(NonTriv::sNumDestructors == 0);

		NonTriv::sReset();
		arr.resize(123);
		CHECK(arr.size() == 123);
		CHECK(arr.capacity() == 123);
		CHECK(NonTriv::sNumConstructors == 123);
		CHECK(NonTriv::sNumCopyConstructors == 0);
		CHECK(NonTriv::sNumMoveConstructors == 0);
		CHECK(NonTriv::sNumDestructors == 0);
		for (int i = 0; i < 123; ++i)
			arr[i] = NonTriv(i);

		NonTriv::sReset();
		arr.resize(456);
		CHECK(arr.size() == 456);
		CHECK(arr.capacity() == 456);
		for (int i = 0; i < 123; ++i)
			CHECK(arr[i].Value() == i);
	//#ifndef JPH_USE_STD_VECTOR
		CHECK(NonTriv::sNumConstructors == 456 - 123);
		CHECK(NonTriv::sNumCopyConstructors == 0);
		CHECK(NonTriv::sNumMoveConstructors == 123);
		CHECK(NonTriv::sNumDestructors == 123); // Switched to a new block, all old elements are destroyed after being moved
	//#endif // JPH_USE_STD_VECTOR

		NonTriv::sReset();
		arr.resize(10);
		CHECK(arr.size() == 10);
		CHECK(arr.capacity() >= 10);
		CHECK(NonTriv::sNumConstructors == 0);
		CHECK(NonTriv::sNumCopyConstructors == 0);
		CHECK(NonTriv::sNumMoveConstructors == 0);
		CHECK(NonTriv::sNumDestructors == 456 - 10);
	}

	TEST_CASE("TestResizeWithValue")
	{
		Array<int> arr;
		arr.resize(10, 55);
		CHECK(arr.size() == 10);
		CHECK(arr.capacity() == 10);
		for (int i = 0; i < 10; ++i)
			CHECK(arr[i] == 55);
	}

	TEST_CASE("TestResizeWithValueNonTriv")
	{
		NonTriv v(55);
		Array<NonTriv> arr;
		NonTriv::sReset();
		arr.resize(10, v);
		CHECK(arr.size() == 10);
		CHECK(arr.capacity() == 10);
		for (int i = 0; i < 10; ++i)
			CHECK(arr[i].Value() == 55);
		CHECK(NonTriv::sNumConstructors == 0);
		CHECK(NonTriv::sNumCopyConstructors == 10);
		CHECK(NonTriv::sNumMoveConstructors == 0);
		CHECK(NonTriv::sNumDestructors == 0);
	}

//#ifndef JPH_USE_STD_VECTOR // std::vector can choose to not shrink the array when calling shrink_to_fit so we can't test this
	TEST_CASE("TestShrinkToFit")
	{
		Array<int> arr;
		for (int i = 0; i < 5; ++i)
			arr.push_back(i);
		CHECK(arr.capacity() > 5);
		CHECK(arr.size() == 5);

		arr.shrink_to_fit();
		CHECK(arr.capacity() == 5);
		CHECK(arr.size() == 5);
		for (int i = 0; i < 5; ++i)
			CHECK(arr[i] == i);

		arr.clear();
		CHECK(arr.capacity() == 5);
		CHECK(arr.size() == 0);

		arr.shrink_to_fit();
		CHECK(arr.capacity() == 0);
		CHECK(arr.data() == nullptr);
	}

	TEST_CASE("TestShrinkToFitNonTriv")
	{
		Array<NonTriv> arr;
		for (int i = 0; i < 5; ++i)
			arr.push_back(NonTriv(i));
		CHECK(arr.capacity() > 5);
		CHECK(arr.size() == 5);

		NonTriv::sReset();
		arr.shrink_to_fit();
		CHECK(arr.capacity() == 5);
		CHECK(arr.size() == 5);
		for (int i = 0; i < 5; ++i)
			CHECK(arr[i].Value() == i);
		CHECK(NonTriv::sNumConstructors == 0);
		CHECK(NonTriv::sNumCopyConstructors == 0);
		CHECK(NonTriv::sNumMoveConstructors == 5);
		CHECK(NonTriv::sNumDestructors == 5); // Switched to a new block, all old elements are destroyed after being moved
	}
//#endif // JPH_USE_STD_VECTOR

	TEST_CASE("TestAssignIterator")
	{
		int values[] = { 1, 2, 3 };

		Array<int> arr({ 4, 5, 6 });
		arr.assign(values, values + 3);
		CHECK(arr.size() == 3);
		CHECK(arr[0] == 1);
		CHECK(arr[1] == 2);
		CHECK(arr[2] == 3);
	}

	TEST_CASE("TestAssignInitializerList")
	{
		Array<int> arr({ 4, 5, 6 });
		arr.assign({ 1, 2, 3 });
		CHECK(arr.size() == 3);
		CHECK(arr[0] == 1);
		CHECK(arr[1] == 2);
		CHECK(arr[2] == 3);
	}

	TEST_CASE("TestSwap")
	{
		Array<int> arr({ 1, 2, 3 });
		Array<int> arr2({ 4, 5, 6 });
		arr.swap(arr2);
		CHECK(arr == Array<int>({ 4, 5, 6 }));
		CHECK(arr2 == Array<int>({ 1, 2, 3 }));
	}

	TEST_CASE("TestInsertBegin")
	{
		Array<int> arr = { 1, 2, 3 };
		arr.insert(arr.begin(), 4);
		CHECK(arr == Array<int>({ 4, 1, 2, 3 }));
	}

	TEST_CASE("TestInsertMid")
	{
		Array<int> arr = { 1, 2, 3 };
		arr.insert(arr.begin() + 1, 4);
		CHECK(arr == Array<int>({ 1, 4, 2, 3 }));
	}

	TEST_CASE("TestInsertEnd")
	{
		Array<int> arr = { 1, 2, 3 };
		arr.insert(arr.begin() + 3, 4);
		CHECK(arr == Array<int>({ 1, 2, 3, 4 }));
	}

	TEST_CASE("TestInsertMultipleBegin")
	{
		Array<int> values_to_insert = { 4, 5, 6, 7 };
		Array<int> arr = { 1, 2, 3 };
		arr.insert(arr.begin(), values_to_insert.begin(), values_to_insert.end());
		CHECK(arr == Array<int>({ 4, 5, 6, 7, 1, 2, 3 }));
	}

	TEST_CASE("TestInsertMultipleMid")
	{
		Array<int> values_to_insert = { 4, 5, 6, 7 };
		Array<int> arr = { 1, 2, 3 };
		arr.insert(arr.begin() + 1, values_to_insert.begin(), values_to_insert.end());
		CHECK(arr == Array<int>({ 1, 4, 5, 6, 7, 2, 3 }));
	}

	TEST_CASE("TestInsertMultipleEnd")
	{
		Array<int> values_to_insert = { 4, 5, 6, 7 };
		Array<int> arr = { 1, 2, 3 };
		arr.insert(arr.begin() + 3, values_to_insert.begin(), values_to_insert.end());
		CHECK(arr == Array<int>({ 1, 2, 3, 4, 5, 6, 7 }));
	}

	TEST_CASE("TestFrontBack")
	{
		Array<int> arr({ 1, 2, 3 });
		CHECK(arr.front() == 1);
		CHECK(arr.back() == 3);
	}

	TEST_CASE("TestAssign")
	{
		Array<int> arr({ 1, 2, 3 });
		Array<int> arr2({ 4, 5, 6 });
		arr = arr2;
		CHECK(arr == Array<int>({ 4, 5, 6 }));
		Array<int> &arr3 = arr; // Avoid compiler warning
		arr = arr3;
		CHECK(arr == Array<int>({ 4, 5, 6 }));
		arr = { 7, 8, 9 };
		CHECK(arr == Array<int>({ 7, 8, 9 }));
	}

	TEST_CASE("TestAssignMove")
	{
		Array<int> arr({ 1, 2, 3 });
		Array<int> arr2({ 4, 5, 6 });
		arr = std::move(arr2);
		CHECK(arr == Array<int>({ 4, 5, 6 }));
		CHECK(arr2.empty());
	}

	TEST_CASE("TestEraseBegin")
	{
		Array<int> arr({ 1, 2, 3 });
		arr.erase(arr.begin());
		CHECK(arr == Array<int>({ 2, 3 }));
	}

	TEST_CASE("TestEraseMid")
	{
		Array<int> arr({ 1, 2, 3 });
		arr.erase(arr.begin() + 1);
		CHECK(arr == Array<int>({ 1, 3 }));
	}

	TEST_CASE("TestEraseEnd")
	{
		Array<int> arr({ 1, 2, 3 });
		arr.erase(arr.begin() + 2);
		CHECK(arr == Array<int>({ 1, 2 }));
	}

	TEST_CASE("TestEraseMultipleBegin")
	{
		Array<int> arr({ 1, 2, 3, 4, 5 });
		arr.erase(arr.begin(), arr.begin() + 2);
		CHECK(arr == Array<int>({ 3, 4, 5 }));
	}

	TEST_CASE("TestEraseMultipleMid")
	{
		Array<int> arr({ 1, 2, 3, 4, 5 });
		arr.erase(arr.begin() + 2, arr.begin() + 4);
		CHECK(arr == Array<int>({ 1, 2, 5 }));
	}

	TEST_CASE("TestEraseMultipleEnd")
	{
		Array<int> arr({ 1, 2, 3, 4, 5 });
		arr.erase(arr.begin() + 3, arr.begin() + 5);
		CHECK(arr == Array<int>({ 1, 2, 3 }));
	}

	TEST_CASE("TestEquals")
	{
		Array<int> arr({ 1, 2, 3 });
		Array<int> arr2({ 4, 5, 6 });
		CHECK(arr == arr);
		CHECK(!(arr == arr2));
		CHECK(!(arr != arr));
		CHECK(arr != arr2);
	}

	TEST_CASE("TestReverseIterator")
	{
		Array<int> arr({ 1, 2, 3, 4, 5, 6 });
		Array<int>::reverse_iterator r = arr.rbegin();
		CHECK(*r == 6);

		int v = *(++r);
		CHECK(v == 5);
		CHECK(*r == 5);

		v = *(--r);
		CHECK(v == 6);
		CHECK(*r == 6);

		v = *(r++);
		CHECK(v == 6);
		CHECK(*r == 5);

		v = *(r--);
		CHECK(v == 5);
		CHECK(*r == 6);

		v = *(r += 2);
		CHECK(v == 4);
		CHECK(*r == 4);

		v = *(r -= 2);
		CHECK(v == 6);
		CHECK(*r == 6);

		CHECK(r == arr.rbegin());
		CHECK(r != arr.rend());

		r += 6;
		CHECK(r == arr.rend());

		CHECK(*(arr.rbegin() + 3) == 3);
		CHECK(*(arr.rend() - 3) == 3);
	}

	TEST_CASE("TestConstReverseIterator")
	{
		const Array<int> arr({ 1, 2, 3, 4, 5, 6 });
		Array<int>::const_reverse_iterator r = arr.rbegin();
		CHECK(*r == 6);

		int v = *(++r);
		CHECK(v == 5);
		CHECK(*r == 5);

		v = *(--r);
		CHECK(v == 6);
		CHECK(*r == 6);

		v = *(r++);
		CHECK(v == 6);
		CHECK(*r == 5);

		v = *(r--);
		CHECK(v == 5);
		CHECK(*r == 6);

		v = *(r += 2);
		CHECK(v == 4);
		CHECK(*r == 4);

		v = *(r -= 2);
		CHECK(v == 6);
		CHECK(*r == 6);

		CHECK(r == arr.rbegin());
		CHECK(r == arr.crbegin());
		CHECK(r != arr.rend());
		CHECK(r != arr.crend());

		r += 6;
		CHECK(r == arr.rend());
		CHECK(r == arr.crend());

		CHECK(*(arr.rbegin() + 3) == 3);
		CHECK(*(arr.rend() - 3) == 3);
	}
}



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

static bool EnsureSame( const auto &a, const auto &v )
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

	EvilIterator& operator++() {
		Iter::operator++();
		return *this;
	}

	auto operator++( int ) {
		return static_cast<Iter &>( *this )++;
	}
};

using EvilIt = EvilIterator<std::move_iterator<std::vector<int>::iterator>>;

static_assert( std::input_iterator<EvilIt> );
static_assert( std::input_or_output_iterator<EvilIt> );

static_assert( std::weakly_incrementable<EvilIt> );
static_assert( std::movable<EvilIt> );

static_assert( requires( EvilIt i ) {
	{ *i } -> std::_Can_reference;
} );

static_assert( requires( EvilIt __i ) {
	typename std::iter_difference_t<EvilIt>;
	requires std::_Signed_integer_like<std::iter_difference_t<EvilIt>>;
	{ ++__i } -> std::same_as<EvilIt&>;
	__i++;
} );

static_assert( requires( EvilIt __i ) {
	typename std::iter_difference_t<EvilIt>;
} );

static_assert( requires( EvilIt __i ) {
	requires std::_Signed_integer_like<std::iter_difference_t<EvilIt>>;
} );

static_assert( requires( EvilIt __i ) {
	{ ++__i } -> std::same_as<EvilIt&>;
} );

static_assert( requires( EvilIt __i ) {
	__i++;
} );

static_assert( std::indirectly_readable<EvilIt> );
static_assert( requires { typename std::_Iter_concept<EvilIt>; } );
static_assert( std::derived_from<std::_Iter_concept<EvilIt>, std::input_iterator_tag> );


TEST_SUITE( "ArrayTest/NonTrival/Insertion" )
{
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
}

TEST_SUITE( "ArrayTest/NonTrival/Erasure" )
{
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

	TEST_CASE_TEMPLATE( "Clear", T, RESERVED_PARAMS )
	{
		using AType = NonTriv<true, T::no_except>;
		using VType = NonTriv<false, T::no_except>;

		Array<AType> arr = { AType( 1 ), AType( 2 ), AType( 3 ), AType( 4 ) };
		vector<VType> vec = { VType( 1 ), VType( 2 ), VType( 3 ), VType( 4 ) };

		AType::sReset();
		VType::sReset();

		arr.clear();
		vec.clear();

		CHECK( arr.capacity() == vec.capacity() );
		CHECK( arr.size() == vec.size() );

		CHECK( arr.empty() );
		REQUIRE( vec.empty() );

		CHECK( arr.data() != nullptr );
		REQUIRE( vec.data() != nullptr );

		CHECK( std::distance( arr.begin(), arr.end() ) == std::distance( vec.begin(), vec.end() ) );
		CHECK( std::distance( arr.rbegin(), arr.rend() ) == std::distance( vec.rbegin(), vec.rend() ) );

		AV_WARN();
		REQUIRE( EnsureSame( arr, vec ) );

		AType::sReset();
		VType::sReset();
	}
}

TEST_SUITE( "ArrayTest/NonTrival/AssignMethods" )
{
	TEST_CASE_TEMPLATE( "Assign[Count]", T, ALL_PARAMS )
	{
		using AType = NonTriv<true, T::no_except>;
		using VType = NonTriv<false, T::no_except>;

		//Array<AType> arrl = { AType( 4 ), AType( 5 ), AType( 6 ) };
		//vector<VType> vecl = { VType( 4 ), VType( 5 ), VType( 6 ) };

		SUBCASE( "Empty" ) {
			Array<AType> arr;
			vector<VType> vec;

			if ( T::reserved ) {
				arr.reserve( 3 );
				vec.reserve( 3 );
			}

			AType::sReset();
			VType::sReset();

			arr.assign( 3, AType( 4 ) );
			vec.assign( 3, VType( 4 ) );

			CHECK( arr.capacity() == vec.capacity() );

			AV_CHECK();
			REQUIRE( EnsureSame( arr, vec ) );
		}

		SUBCASE( "Partial" ) {
			Array<AType> arr = { AType( 1 ) };
			vector<VType> vec = { VType( 1 ) };

			if ( T::reserved ) {
				arr.reserve( 3 );
				vec.reserve( 3 );
			}

			AType::sReset();
			VType::sReset();

			arr.assign( 3, AType( 4 ) );
			vec.assign( 3, VType( 4 ) );

			CHECK( arr.capacity() == vec.capacity() );

			AV_CHECK();
			REQUIRE( EnsureSame( arr, vec ) );
		}

		SUBCASE( "Partial[WrongReserve]" ) {
			Array<AType> arr = { AType( 1 ) };
			vector<VType> vec = { VType( 1 ) };

			arr.reserve( T::reserved ? 4 : 2 );
			vec.reserve( T::reserved ? 4 : 2 );

			AType::sReset();
			VType::sReset();

			arr.assign( 3, AType( 4 ) );
			vec.assign( 3, VType( 4 ) );

			CHECK( arr.capacity() == vec.capacity() );

			AV_CHECK();
			REQUIRE( EnsureSame( arr, vec ) );
		}

		SUBCASE( "Exact" ) {
			Array<AType> arr = { AType( 1 ), AType( 2 ), AType( 3 ) };
			vector<VType> vec = { VType( 1 ), VType( 2 ), VType( 3 ) };

			if ( T::reserved ) {
				arr.reserve( 3 );
				vec.reserve( 3 );
			}

			AType::sReset();
			VType::sReset();

			arr.assign( 3, AType( 4 ) );
			vec.assign( 3, VType( 4 ) );

			CHECK( arr.capacity() == vec.capacity() );

			AV_CHECK();
			REQUIRE( EnsureSame( arr, vec ) );
		}

		SUBCASE( "Shorter" ) {
			Array<AType> arr = { AType( 1 ), AType( 2 ), AType( 3 ), AType( 4 ) };
			vector<VType> vec = { VType( 1 ), VType( 2 ), VType( 3 ), VType( 4 ) };

			if ( T::reserved ) {
				arr.reserve( 3 );
				vec.reserve( 3 );
			}

			AType::sReset();
			VType::sReset();

			arr.assign( 3, AType( 4 ) );
			vec.assign( 3, VType( 4 ) );

			CHECK( arr.capacity() == vec.capacity() );

			AV_CHECK();
			REQUIRE( EnsureSame( arr, vec ) );
		}

		AType::sReset();
		VType::sReset();
	}

	TEST_CASE_TEMPLATE( "Assign[FromInitList]", T, ALL_PARAMS )
	{
		using AType = NonTriv<true, T::no_except>;
		using VType = NonTriv<false, T::no_except>;

		std::initializer_list<AType> arrl = { AType( 4 ), AType( 5 ), AType( 6 ) };
		std::initializer_list<VType> vecl = { VType( 4 ), VType( 5 ), VType( 6 ) };

		SUBCASE( "Empty" ) {
			Array<AType> arr;
			vector<VType> vec;

			if ( T::reserved ) {
				arr.reserve( 3 );
				vec.reserve( 3 );
			}

			AType::sReset();
			VType::sReset();

			arr.assign( arrl );
			vec.assign( vecl );

			CHECK( arr.capacity() == vec.capacity() );

			AV_CHECK();
			REQUIRE( EnsureSame( arr, vec ) );
		}

		SUBCASE( "Partial" ) {
			Array<AType> arr = { AType( 1 ) };
			vector<VType> vec = { VType( 1 ) };

			if ( T::reserved ) {
				arr.reserve( 3 );
				vec.reserve( 3 );
			}

			AType::sReset();
			VType::sReset();

			arr.assign( arrl );
			vec.assign( vecl );

			CHECK( arr.capacity() == vec.capacity() );

			AV_CHECK();
			REQUIRE( EnsureSame( arr, vec ) );
		}

		SUBCASE( "Partial[WrongReserve]" ) {
			Array<AType> arr = { AType( 1 ) };
			vector<VType> vec = { VType( 1 ) };

			arr.reserve( T::reserved ? 4 : 2 );
			vec.reserve( T::reserved ? 4 : 2 );

			AType::sReset();
			VType::sReset();

			arr.assign( arrl );
			vec.assign( vecl );

			CHECK( arr.capacity() == vec.capacity() );

			AV_CHECK();
			REQUIRE( EnsureSame( arr, vec ) );
		}

		SUBCASE( "Exact" ) {
			Array<AType> arr = { AType( 1 ), AType( 2 ), AType( 3 ) };
			vector<VType> vec = { VType( 1 ), VType( 2 ), VType( 3 ) };

			if ( T::reserved ) {
				arr.reserve( 3 );
				vec.reserve( 3 );
			}

			AType::sReset();
			VType::sReset();

			arr.assign( arrl );
			vec.assign( vecl );

			CHECK( arr.capacity() == vec.capacity() );

			AV_CHECK();
			REQUIRE( EnsureSame( arr, vec ) );
		}

		SUBCASE( "Shorter" ) {
			Array<AType> arr = { AType( 1 ), AType( 2 ), AType( 3 ), AType( 4 ) };
			vector<VType> vec = { VType( 1 ), VType( 2 ), VType( 3 ), VType( 4 ) };

			if ( T::reserved ) {
				arr.reserve( 3 );
				vec.reserve( 3 );
			}

			AType::sReset();
			VType::sReset();

			arr.assign( arrl );
			vec.assign( vecl );

			CHECK( arr.capacity() == vec.capacity() );

			AV_CHECK();
			REQUIRE( EnsureSame( arr, vec ) );
		}

		AType::sReset();
		VType::sReset();
	}

	TEST_CASE_TEMPLATE( "Assign[FromVector]", T, ALL_PARAMS )
	{
		using AType = NonTriv<true, T::no_except>;
		using VType = NonTriv<false, T::no_except>;

		Array<AType> arrl = { AType( 4 ), AType( 5 ), AType( 6 ) };
		vector<VType> vecl = { VType( 4 ), VType( 5 ), VType( 6 ) };

		SUBCASE( "Empty" ) {
			Array<AType> arr;
			vector<VType> vec;

			if ( T::reserved ) {
				arr.reserve( 3 );
				vec.reserve( 3 );
			}

			AType::sReset();
			VType::sReset();

			arr.assign( arrl.begin(), arrl.end() );
			vec.assign( vecl.begin(), vecl.end() );

			CHECK( arr.capacity() == vec.capacity() );

			AV_CHECK();
			REQUIRE( EnsureSame( arr, vec ) );
		}

		SUBCASE( "Partial" ) {
			Array<AType> arr = { AType( 1 ) };
			vector<VType> vec = { VType( 1 ) };

			if ( T::reserved ) {
				arr.reserve( 3 );
				vec.reserve( 3 );
			}

			AType::sReset();
			VType::sReset();

			arr.assign( arrl.begin(), arrl.end() );
			vec.assign( vecl.begin(), vecl.end() );

			CHECK( arr.capacity() == vec.capacity() );

			AV_CHECK();
			REQUIRE( EnsureSame( arr, vec ) );
		}

		SUBCASE( "Partial[WrongReserve]" ) {
			Array<AType> arr = { AType( 1 ) };
			vector<VType> vec = { VType( 1 ) };

			arr.reserve( T::reserved ? 4 : 2 );
			vec.reserve( T::reserved ? 4 : 2 );

			AType::sReset();
			VType::sReset();

			arr.assign( arrl.begin(), arrl.end() );
			vec.assign( vecl.begin(), vecl.end() );

			CHECK( arr.capacity() == vec.capacity() );

			AV_CHECK();
			REQUIRE( EnsureSame( arr, vec ) );
		}

		SUBCASE( "Exact" ) {
			Array<AType> arr = { AType( 1 ), AType( 2 ), AType( 3 ) };
			vector<VType> vec = { VType( 1 ), VType( 2 ), VType( 3 ) };

			if ( T::reserved ) {
				arr.reserve( 3 );
				vec.reserve( 3 );
			}

			AType::sReset();
			VType::sReset();

			arr.assign( arrl.begin(), arrl.end() );
			vec.assign( vecl.begin(), vecl.end() );

			CHECK( arr.capacity() == vec.capacity() );

			AV_CHECK();
			REQUIRE( EnsureSame( arr, vec ) );
		}

		SUBCASE( "Shorter" ) {
			Array<AType> arr = { AType( 1 ), AType( 2 ), AType( 3 ), AType( 4 ) };
			vector<VType> vec = { VType( 1 ), VType( 2 ), VType( 3 ), VType( 4 ) };

			if ( T::reserved ) {
				arr.reserve( 3 );
				vec.reserve( 3 );
			}

			AType::sReset();
			VType::sReset();

			arr.assign( arrl.begin(), arrl.end() );
			vec.assign( vecl.begin(), vecl.end() );

			CHECK( arr.capacity() == vec.capacity() );

			AV_CHECK();
			REQUIRE( EnsureSame( arr, vec ) );
		}

		AType::sReset();
		VType::sReset();
	}

	TEST_CASE_TEMPLATE( "Assign[FromList]", T, ALL_PARAMS )
	{
		using AType = NonTriv<true, T::no_except>;
		using VType = NonTriv<false, T::no_except>;

		std::list<AType> arrl = { AType( 4 ), AType( 5 ), AType( 6 ) };
		std::list<VType> vecl = { VType( 4 ), VType( 5 ), VType( 6 ) };

		SUBCASE( "Empty" ) {
			Array<AType> arr;
			vector<VType> vec;

			if ( T::reserved ) {
				arr.reserve( 3 );
				vec.reserve( 3 );
			}

			AType::sReset();
			VType::sReset();

			arr.assign( arrl.begin(), arrl.end() );
			vec.assign( vecl.begin(), vecl.end() );

			CHECK( arr.capacity() == vec.capacity() );

			AV_CHECK();
			REQUIRE( EnsureSame( arr, vec ) );
		}

		SUBCASE( "Partial" ) {
			Array<AType> arr = { AType( 1 ) };
			vector<VType> vec = { VType( 1 ) };

			if ( T::reserved ) {
				arr.reserve( 3 );
				vec.reserve( 3 );
			}

			AType::sReset();
			VType::sReset();

			arr.assign( arrl.begin(), arrl.end() );
			vec.assign( vecl.begin(), vecl.end() );

			CHECK( arr.capacity() == vec.capacity() );

			AV_CHECK();
			REQUIRE( EnsureSame( arr, vec ) );
		}

		SUBCASE( "Partial[WrongReserve]" ) {
			Array<AType> arr = { AType( 1 ) };
			vector<VType> vec = { VType( 1 ) };

			arr.reserve( T::reserved ? 4 : 2 );
			vec.reserve( T::reserved ? 4 : 2 );

			AType::sReset();
			VType::sReset();

			arr.assign( arrl.begin(), arrl.end() );
			vec.assign( vecl.begin(), vecl.end() );

			CHECK( arr.capacity() == vec.capacity() );

			AV_CHECK();
			REQUIRE( EnsureSame( arr, vec ) );
		}

		SUBCASE( "Exact" ) {
			Array<AType> arr = { AType( 1 ), AType( 2 ), AType( 3 ) };
			vector<VType> vec = { VType( 1 ), VType( 2 ), VType( 3 ) };

			if ( T::reserved ) {
				arr.reserve( 3 );
				vec.reserve( 3 );
			}

			AType::sReset();
			VType::sReset();

			arr.assign( arrl.begin(), arrl.end() );
			vec.assign( vecl.begin(), vecl.end() );

			CHECK( arr.capacity() == vec.capacity() );

			AV_CHECK();
			REQUIRE( EnsureSame( arr, vec ) );
		}

		SUBCASE( "Shorter" ) {
			Array<AType> arr = { AType( 1 ), AType( 2 ), AType( 3 ), AType( 4 ) };
			vector<VType> vec = { VType( 1 ), VType( 2 ), VType( 3 ), VType( 4 ) };

			if ( T::reserved ) {
				arr.reserve( 3 );
				vec.reserve( 3 );
			}

			AType::sReset();
			VType::sReset();

			arr.assign( arrl.begin(), arrl.end() );
			vec.assign( vecl.begin(), vecl.end() );

			CHECK( arr.capacity() == vec.capacity() );

			AV_CHECK();
			REQUIRE( EnsureSame( arr, vec ) );
		}

		AType::sReset();
		VType::sReset();
	}

	TEST_CASE_TEMPLATE( "Assign[MoveFromInputList]", T, ALL_PARAMS )
	{
		using AType = NonTriv<true, T::no_except>;
		using VType = NonTriv<false, T::no_except>;

		std::list<AType> arrl = { AType( 4 ), AType( 5 ), AType( 6 ) };
		std::list<VType> vecl = { VType( 4 ), VType( 5 ), VType( 6 ) };

		SUBCASE( "Empty" ) {
			Array<AType> arr;
			vector<VType> vec;

			if ( T::reserved ) {
				arr.reserve( 3 );
				vec.reserve( 3 );
			}

			AType::sReset();
			VType::sReset();

			arr.assign( EvilIterator( std::make_move_iterator( arrl.begin() ) ), EvilIterator( std::make_move_iterator( arrl.end() ) ) );
			vec.assign( EvilIterator( std::make_move_iterator( vecl.begin() ) ), EvilIterator( std::make_move_iterator( vecl.end() ) ) );

			CHECK( arr.capacity() == vec.capacity() );

			AV_CHECK();
			REQUIRE( EnsureSame( arr, vec ) );
		}

		SUBCASE( "Partial" ) {
			Array<AType> arr = { AType( 1 ) };
			vector<VType> vec = { VType( 1 ) };

			if ( T::reserved ) {
				arr.reserve( 3 );
				vec.reserve( 3 );
			}

			AType::sReset();
			VType::sReset();

			arr.assign( EvilIterator( std::make_move_iterator( arrl.begin() ) ), EvilIterator( std::make_move_iterator( arrl.end() ) ) );
			vec.assign( EvilIterator( std::make_move_iterator( vecl.begin() ) ), EvilIterator( std::make_move_iterator( vecl.end() ) ) );

			CHECK( arr.capacity() == vec.capacity() );

			AV_CHECK();
			REQUIRE( EnsureSame( arr, vec ) );
		}

		SUBCASE( "Partial[WrongReserve]" ) {
			Array<AType> arr = { AType( 1 ) };
			vector<VType> vec = { VType( 1 ) };

			arr.reserve( T::reserved ? 4 : 2 );
			vec.reserve( T::reserved ? 4 : 2 );

			AType::sReset();
			VType::sReset();

			arr.assign( EvilIterator( std::make_move_iterator( arrl.begin() ) ), EvilIterator( std::make_move_iterator( arrl.end() ) ) );
			vec.assign( EvilIterator( std::make_move_iterator( vecl.begin() ) ), EvilIterator( std::make_move_iterator( vecl.end() ) ) );

			CHECK( arr.capacity() == vec.capacity() );

			AV_CHECK();
			REQUIRE( EnsureSame( arr, vec ) );
		}

		SUBCASE( "Exact" ) {
			Array<AType> arr = { AType( 1 ), AType( 2 ), AType( 3 ) };
			vector<VType> vec = { VType( 1 ), VType( 2 ), VType( 3 ) };

			if ( T::reserved ) {
				arr.reserve( 3 );
				vec.reserve( 3 );
			}

			AType::sReset();
			VType::sReset();

			arr.assign( EvilIterator( std::make_move_iterator( arrl.begin() ) ), EvilIterator( std::make_move_iterator( arrl.end() ) ) );
			vec.assign( EvilIterator( std::make_move_iterator( vecl.begin() ) ), EvilIterator( std::make_move_iterator( vecl.end() ) ) );

			CHECK( arr.capacity() == vec.capacity() );

			AV_CHECK();
			REQUIRE( EnsureSame( arr, vec ) );
		}

		SUBCASE( "Shorter" ) {
			Array<AType> arr = { AType( 1 ), AType( 2 ), AType( 3 ), AType( 4 ) };
			vector<VType> vec = { VType( 1 ), VType( 2 ), VType( 3 ), VType( 4 ) };

			if ( T::reserved ) {
				arr.reserve( 3 );
				vec.reserve( 3 );
			}

			AType::sReset();
			VType::sReset();

			arr.assign( EvilIterator( std::make_move_iterator( arrl.begin() ) ), EvilIterator( std::make_move_iterator( arrl.end() ) ) );
			vec.assign( EvilIterator( std::make_move_iterator( vecl.begin() ) ), EvilIterator( std::make_move_iterator( vecl.end() ) ) );

			CHECK( arr.capacity() == vec.capacity() );

			AV_CHECK();
			REQUIRE( EnsureSame( arr, vec ) );
		}

		AType::sReset();
		VType::sReset();
	}


	TEST_CASE_TEMPLATE( "Varying[Count]", T, ALL_PARAMS )
	{
		using AType = NonTriv<true, T::no_except>;
		using VType = NonTriv<false, T::no_except>;

		Array<AType> arr;
		vector<VType> vec;

		int i = GENERATE( 0, 1, 16, 17, 23, 24, 25, 256 );
		int x = GENERATE( 0, 1, 16, 17, 23, 24, 25, 256 );

		SUBCASE( "Src=" + doctest::toString( i ) + " Dst=" + doctest::toString( x ) ) {
			AType::sReset();
			VType::sReset();

			if ( T::reserved ) {
				arr.reserve( i );
				vec.reserve( i );
			}

			for ( int y = 0; y < x; ++y ) {
				arr.emplace_back( y );
				vec.emplace_back( y );
			}


			arr.assign( i, AType( i ) );
			vec.assign( i, VType( i ) );

			CHECK( arr.capacity() == vec.capacity() );

			AV_CHECK();
			REQUIRE( EnsureSame( arr, vec ) );
		}

		AType::sReset();
		VType::sReset();
	}

	TEST_CASE_TEMPLATE( "Varying[FromVector]", T, ALL_PARAMS )
	{
		using AType = NonTriv<true, T::no_except>;
		using VType = NonTriv<false, T::no_except>;

		Array<AType> arrSrc;
		vector<VType> vecSrc;

		Array<AType> arr;
		vector<VType> vec;

		int i = GENERATE( 0, 1, 16, 17, 23, 24, 25, 256 );
		int x = GENERATE( 0, 1, 16, 17, 23, 24, 25, 256 );

		SUBCASE( "Src=" + doctest::toString( i ) + " Dst=" + doctest::toString( x ) ) {
			AType::sReset();
			VType::sReset();

			for ( int j = 0; j < i; ++j ) {
				arrSrc.emplace_back( j );
				vecSrc.emplace_back( j );
			}

			if ( T::reserved ) {
				arr.reserve( i );
				vec.reserve( i );
			}

			for ( int y = 0; y < x; ++y ) {
				arr.emplace_back( y );
				vec.emplace_back( y );
			}


			arr.assign( arrSrc.begin(), arrSrc.end() );
			vec.assign( vecSrc.begin(), vecSrc.end() );

			CHECK( arr.capacity() == vec.capacity() );

			AV_CHECK();
			REQUIRE( EnsureSame( arr, vec ) );
			REQUIRE( EnsureSame( arrSrc, vecSrc ) );
		}

		AType::sReset();
		VType::sReset();
	}

	TEST_CASE_TEMPLATE( "Varying[MoveFromInputList]", T, ALL_PARAMS )
	{
		using AType = NonTriv<true, T::no_except>;
		using VType = NonTriv<false, T::no_except>;

		std::list<AType> arrSrc;
		std::list<VType> vecSrc;

		Array<AType> arr;
		vector<VType> vec;

		int i = GENERATE( 0, 1, 16, 17, 23, 24, 25, 256 );
		int x = GENERATE( 0, 1, 16, 17, 23, 24, 25, 256 );

		SUBCASE( "Src=" + doctest::toString( i ) + " Dst=" + doctest::toString( x ) ) {
			AType::sReset();
			VType::sReset();

			for ( int j = 0; j < i; ++j ) {
				arrSrc.emplace_back( j );
				vecSrc.emplace_back( j );
			}

			if ( T::reserved ) {
				arr.reserve( i );
				vec.reserve( i );
			}

			for ( int y = 0; y < x; ++y ) {
				arr.emplace_back( y );
				vec.emplace_back( y );
			}


			arr.assign( EvilIterator( std::make_move_iterator( arrSrc.begin() ) ), EvilIterator( std::make_move_iterator( arrSrc.end() ) ) );
			vec.assign( EvilIterator( std::make_move_iterator( vecSrc.begin() ) ), EvilIterator( std::make_move_iterator( vecSrc.end() ) ) );

			CHECK( arr.capacity() == vec.capacity() );

			AV_CHECK();
			REQUIRE( EnsureSame( arr, vec ) );
			REQUIRE( EnsureSame( arrSrc, vecSrc ) );
		}

		AType::sReset();
		VType::sReset();
	}
}

TEST_SUITE( "ArrayTest/NonTrival/SpecialConstructors" )
{
	TEST_CASE_TEMPLATE( "Construct[N]", T, RESERVED_PARAMS )
	{
		using AType = NonTriv<true, T::no_except>;
		using VType = NonTriv<false, T::no_except>;

		uint32_t i = GENERATE( 1, 2, 4, 8 );

		SUBCASE( "2^" + doctest::toString( i ) ) {
			AType::sReset();
			VType::sReset();

			Array<AType> arr( 1 << i, AType( i ) );
			vector<VType> vec( 1ULL << i, VType( i ) );

			CHECK( arr.capacity() == vec.capacity() );

			AV_CHECK();
			REQUIRE( EnsureSame( arr, vec ) );
		}

		AType::sReset();
		VType::sReset();
	}

	TEST_CASE_TEMPLATE( "Construct[InitList]", T, RESERVED_PARAMS )
	{
		using AType = NonTriv<true, T::no_except>;
		using VType = NonTriv<false, T::no_except>;

		AType::sReset();
		VType::sReset();

		Array<AType> arr{ AType( 1 ), AType( 2 ), AType( 3 ), AType( 4 ) };
		vector<VType> vec{ VType( 1 ), VType( 2 ), VType( 3 ), VType( 4 ) };

		CHECK( arr.capacity() == vec.capacity() );

		AV_CHECK();
		REQUIRE( EnsureSame( arr, vec ) );

		AType::sReset();
		VType::sReset();
	}

	TEST_CASE_TEMPLATE( "Construct[IterVector]", T, RESERVED_PARAMS )
	{
		using AType = NonTriv<true, T::no_except>;
		using VType = NonTriv<false, T::no_except>;

		Array<AType> arrl{ AType( 1 ), AType( 2 ), AType( 3 ), AType( 4 ) };
		vector<VType> vecl{ VType( 1 ), VType( 2 ), VType( 3 ), VType( 4 ) };

		AType::sReset();
		VType::sReset();

		Array<AType> arr( arrl.begin(), arrl.end() );
		vector<VType> vec( vecl.begin(), vecl.end() );

		CHECK( arr.capacity() == vec.capacity() );

		AV_CHECK();
		REQUIRE( EnsureSame( arr, vec ) );

		AType::sReset();
		VType::sReset();
	}

	TEST_CASE_TEMPLATE( "Construct[IterList]", T, RESERVED_PARAMS )
	{
		using AType = NonTriv<true, T::no_except>;
		using VType = NonTriv<false, T::no_except>;

		std::list<AType> arrl{ AType( 1 ), AType( 2 ), AType( 3 ), AType( 4 ) };
		std::list<VType> vecl{ VType( 1 ), VType( 2 ), VType( 3 ), VType( 4 ) };

		AType::sReset();
		VType::sReset();

		Array<AType> arr( arrl.begin(), arrl.end() );
		vector<VType> vec( vecl.begin(), vecl.end() );

		CHECK( arr.capacity() == vec.capacity() );

		AV_CHECK();
		REQUIRE( EnsureSame( arr, vec ) );

		AType::sReset();
		VType::sReset();
	}

	TEST_CASE_TEMPLATE( "Construct[IterInputOnly]", T, RESERVED_PARAMS )
	{
		using AType = NonTriv<true, T::no_except>;
		using VType = NonTriv<false, T::no_except>;

		std::list<AType> arrl{ AType( 1 ), AType( 2 ), AType( 3 ), AType( 4 ) };
		std::list<VType> vecl{ VType( 1 ), VType( 2 ), VType( 3 ), VType( 4 ) };

		AType::sReset();
		VType::sReset();

		Array<AType> arr( EvilIterator( std::make_move_iterator( arrl.begin() ) ), EvilIterator( std::make_move_iterator( arrl.end() ) ) );
		vector<VType> vec( EvilIterator( std::make_move_iterator( vecl.begin() ) ), EvilIterator( std::make_move_iterator( vecl.end() ) ) );

		CHECK( arr.capacity() == vec.capacity() );

		AV_CHECK();
		REQUIRE( EnsureSame( arr, vec ) );
		REQUIRE( EnsureSame( arrl, vecl ) );

		AType::sReset();
		VType::sReset();
	}
}


TEST_SUITE( "ArrayTest/NonTrival/CopyMoveContructors" )
{
	TEST_CASE_TEMPLATE( "CopyToEmpty", T, RESERVED_PARAMS )
	{
		using AType = NonTriv<true, T::no_except>;
		using VType = NonTriv<false, T::no_except>;

		Array<AType> arrSrc;
		vector<VType> vecSrc;

		int i = GENERATE( 0, 1, 16, 17, 23, 24, 25, 256 );

		SUBCASE( "Src Size = " + doctest::toString( i ) ) {
			for ( int j = 0; j < i; ++j ) {
				arrSrc.emplace_back( j );
				vecSrc.emplace_back( j );
			}

			AType::sReset();
			VType::sReset();

			Array<AType> arr( arrSrc );
			vector<VType> vec( vecSrc );

			CHECK( arr.capacity() == vec.capacity() );

			AV_CHECK();
			REQUIRE( EnsureSame( arr, vec ) );
			REQUIRE( EnsureSame( arrSrc, vecSrc ) );
		}

		AType::sReset();
		VType::sReset();
	}

	TEST_CASE_TEMPLATE( "CopyToEmpty[ExplicitAlloc]", T, RESERVED_PARAMS )
	{
		using AType = NonTriv<true, T::no_except>;
		using VType = NonTriv<false, T::no_except>;

		Array<AType> arrSrc;
		vector<VType> vecSrc;

		int i = GENERATE( 0, 1, 16, 17, 23, 24, 25, 256 );

		SUBCASE( "Src Size = " + doctest::toString( i ) ) {
			for ( int j = 0; j < i; ++j ) {
				arrSrc.emplace_back( j );
				vecSrc.emplace_back( j );
			}

			AType::sReset();
			VType::sReset();

			Array<AType> arr( arrSrc, arrSrc.get_allocator() );
			vector<VType> vec( vecSrc, vecSrc.get_allocator() );

			CHECK( arr.capacity() == vec.capacity() );

			AV_CHECK();
			REQUIRE( EnsureSame( arr, vec ) );
			REQUIRE( EnsureSame( arrSrc, vecSrc ) );
		}

		AType::sReset();
		VType::sReset();
	}

	TEST_CASE_TEMPLATE( "MoveToEmpty", T, RESERVED_PARAMS )
	{
		using AType = NonTriv<true, T::no_except>;
		using VType = NonTriv<false, T::no_except>;

		Array<AType> arrSrc;
		vector<VType> vecSrc;

		int i = GENERATE( 0, 1, 16, 17, 23, 24, 25, 256 );

		SUBCASE( "Src Size = " + doctest::toString( i ) ) {
			for ( int j = 0; j < i; ++j ) {
				arrSrc.emplace_back( j );
				vecSrc.emplace_back( j );
			}

			AType::sReset();
			VType::sReset();

			Array<AType> arr( std::move( arrSrc ) );
			vector<VType> vec( std::move( vecSrc ) );

			CHECK( arr.capacity() == vec.capacity() );

			AV_CHECK();
			REQUIRE( EnsureSame( arr, vec ) );
			REQUIRE( EnsureSame( arrSrc, vecSrc ) );
		}

		AType::sReset();
		VType::sReset();
	}

	TEST_CASE_TEMPLATE( "MoveToEmpty[ExplicitAlloc]", T, RESERVED_PARAMS )
	{
		using AType = NonTriv<true, T::no_except>;
		using VType = NonTriv<false, T::no_except>;

		Array<AType> arrSrc;
		vector<VType> vecSrc;

		int i = GENERATE( 0, 1, 16, 17, 23, 24, 25, 256 );

		SUBCASE( "Src Size = " + doctest::toString( i ) ) {
			for ( int j = 0; j < i; ++j ) {
				arrSrc.emplace_back( j );
				vecSrc.emplace_back( j );
			}

			AType::sReset();
			VType::sReset();

			Array<AType> arr( std::move( arrSrc ), arrSrc.get_allocator() );
			vector<VType> vec( std::move( vecSrc ), vecSrc.get_allocator() );

			CHECK( arr.capacity() == vec.capacity() );

			AV_CHECK();
			REQUIRE( EnsureSame( arr, vec ) );
			REQUIRE( EnsureSame( arrSrc, vecSrc ) );
		}

		AType::sReset();
		VType::sReset();
	}
}

TEST_SUITE( "ArrayTest/NonTrival/CopyMoveAssignment" )
{
	TEST_CASE_TEMPLATE( "CopyTo", T, ALL_PARAMS )
	{
		using AType = NonTriv<true, T::no_except>;
		using VType = NonTriv<false, T::no_except>;

		Array<AType> arrSrc;
		vector<VType> vecSrc;

		Array<AType> arr;
		vector<VType> vec;

		int i = GENERATE( 0, 1, 16, 17, 23, 24, 25, 256 );
		int x = GENERATE( 0, 1, 16, 17, 23, 24, 25, 256 );

		SUBCASE( "Src=" + doctest::toString( i ) + " Dst=" + doctest::toString( x ) ) {
			for ( int j = 0; j < i; ++j ) {
				arrSrc.emplace_back( j );
				vecSrc.emplace_back( j );
			}

			if ( T::reserved ) {
				arr.reserve( i );
				vec.reserve( i );
			}

			for ( int y = 0; y < x; ++y ) {
				arr.emplace_back( y );
				vec.emplace_back( y );
			}

			AType::sReset();
			VType::sReset();

			arr = arrSrc;
			vec = vecSrc;

			CHECK( arr.capacity() == vec.capacity() );

			AV_CHECK();
			REQUIRE( EnsureSame( arr, vec ) );
			REQUIRE( EnsureSame( arrSrc, vecSrc ) );
		}

		AType::sReset();
		VType::sReset();
	}

	TEST_CASE_TEMPLATE( "MoveTo", T, ALL_PARAMS )
	{
		using AType = NonTriv<true, T::no_except>;
		using VType = NonTriv<false, T::no_except>;

		Array<AType> arrSrc;
		vector<VType> vecSrc;

		Array<AType> arr;
		vector<VType> vec;

		int i = GENERATE( 0, 1, 16, 17, 23, 24, 25, 256 );
		int x = GENERATE( 0, 1, 16, 17, 23, 24, 25, 256 );

		SUBCASE( "Src=" + doctest::toString( i ) + " Dst=" + doctest::toString( x ) ) {
			for ( int j = 0; j < i; ++j ) {
				arrSrc.emplace_back( j );
				vecSrc.emplace_back( j );
			}

			if ( T::reserved ) {
				arr.reserve( i );
				vec.reserve( i );
			}

			for ( int y = 0; y < x; ++y ) {
				arr.emplace_back( y );
				vec.emplace_back( y );
			}

			AType::sReset();
			VType::sReset();

			arr = std::move( arrSrc );
			vec = std::move( vecSrc );

			CHECK( arr.capacity() == vec.capacity() );

			AV_CHECK();
			REQUIRE( EnsureSame( arr, vec ) );
			REQUIRE( EnsureSame( arrSrc, vecSrc ) );
		}

		AType::sReset();
		VType::sReset();
	}

	TEST_CASE_TEMPLATE( "InitAssign", T, RESERVED_PARAMS )
	{
		using AType = NonTriv<true, T::no_except>;
		using VType = NonTriv<false, T::no_except>;

		AType::sReset();
		VType::sReset();

		Array<AType> arr{ AType( 1 ), AType( 2 ), AType( 3 ), AType( 4 ) };
		vector<VType> vec{ VType( 1 ), VType( 2 ), VType( 3 ), VType( 4 ) };

		arr = { AType( 5 ), AType( 6 ), AType( 7 ) };
		vec = { VType( 5 ), VType( 6 ), VType( 7 ) };

		CHECK( arr.capacity() == vec.capacity() );

		AV_CHECK();
		REQUIRE( EnsureSame( arr, vec ) );

		AType::sReset();
		VType::sReset();
	}
}

TEST_SUITE( "ArrayTest/NonTrival/Swap" )
{
	TEST_CASE_TEMPLATE( "Swap", T, ALL_PARAMS )
	{
		using AType = NonTriv<true, T::no_except>;
		using VType = NonTriv<false, T::no_except>;

		Array<AType> arrSrc;
		vector<VType> vecSrc;

		Array<AType> arr;
		vector<VType> vec;

		int i = GENERATE( 0, 1, 16, 17, 23, 24, 25, 256 );
		int x = GENERATE( 0, 1, 16, 17, 23, 24, 25, 256 );

		SUBCASE( "Src=" + doctest::toString( i ) + " Dst=" + doctest::toString( x ) ) {
			for ( int j = 0; j < i; ++j ) {
				arrSrc.emplace_back( j );
				vecSrc.emplace_back( j );
			}

			if ( T::reserved ) {
				arr.reserve( i );
				vec.reserve( i );
			}

			for ( int y = 0; y < x; ++y ) {
				arr.emplace_back( y );
				vec.emplace_back( y );
			}

			AType::sReset();
			VType::sReset();

			std::swap( arr, arrSrc );
			std::swap( vec, vecSrc );

			CHECK( arr.capacity() == vec.capacity() );
			CHECK( arrSrc.capacity() == vecSrc.capacity() );

			AV_CHECK();
			REQUIRE( EnsureSame( arr, vec ) );
			REQUIRE( EnsureSame( arrSrc, vecSrc ) );
		}

		AType::sReset();
		VType::sReset();
	}
}

TEST_SUITE( "ArrayTest/NonTrival/Resize" )
{
	TEST_CASE_TEMPLATE( "Resize", T, ALL_PARAMS )
	{
		using AType = NonTriv<true, T::no_except>;
		using VType = NonTriv<false, T::no_except>;

		Array<AType> arr;
		vector<VType> vec;

		int i = GENERATE( 0, 1, 16, 17, 23, 24, 25, 256 );
		int x = GENERATE( 0, 1, 16, 17, 23, 24, 25, 256 );

		SUBCASE( "Src=" + doctest::toString( i ) + " Dst=" + doctest::toString( x ) ) {
			AType::sReset();
			VType::sReset();

			arr.resize( i, AType( i ) );
			vec.resize( i, VType( i ) );

			CHECK( arr.capacity() == vec.capacity() );

			if ( T::reserved ) {
				arr.reserve( x );
				vec.reserve( x );
			}

			arr.resize( x, AType( x ) );
			vec.resize( x, VType( x ) );

			CHECK( arr.capacity() == vec.capacity() );

			AV_CHECK();
			REQUIRE( EnsureSame( arr, vec ) );
		}

		AType::sReset();
		VType::sReset();
	}
}

// Shrink to fit
TEST_SUITE( "ArrayTest/NonTrival/ShrinkToFit" )
{
	TEST_CASE_TEMPLATE( "AfterResize", T, ALL_PARAMS )
	{
		using AType = NonTriv<true, T::no_except>;
		using VType = NonTriv<false, T::no_except>;

		Array<AType> arr;
		vector<VType> vec;

		int i = GENERATE( 0, 1, 16, 17, 23, 24, 25, 256 );

		SUBCASE( "Src=" + doctest::toString( i ) ) {
			AType::sReset();
			VType::sReset();

			if ( T::reserved ) {
				arr.reserve( i );
				vec.reserve( i );
			}

			arr.resize( i, AType( i ) );
			vec.resize( i, VType( i ) );

			arr.shrink_to_fit();
			vec.shrink_to_fit();

			CHECK( arr.capacity() == vec.capacity() );

			AV_CHECK();
			REQUIRE( EnsureSame( arr, vec ) );
		}

		AType::sReset();
		VType::sReset();
	}

	TEST_CASE_TEMPLATE( "AfterEmplace", T, ALL_PARAMS )
	{
		using AType = NonTriv<true, T::no_except>;
		using VType = NonTriv<false, T::no_except>;

		Array<AType> arr;
		vector<VType> vec;

		int i = GENERATE( 0, 1, 16, 17, 23, 24, 25, 256 );

		SUBCASE( "Src=" + doctest::toString( i ) ) {
			AType::sReset();
			VType::sReset();

			if ( T::reserved ) {
				arr.reserve( i );
				vec.reserve( i );
			}

			for ( int j = 0; j < i; ++j ) {
				arr.emplace_back( j + 1 );
				vec.emplace_back( j + 1 );
			}

			arr.shrink_to_fit();
			vec.shrink_to_fit();

			CHECK( arr.capacity() == vec.capacity() );

			AV_CHECK();
			REQUIRE( EnsureSame( arr, vec ) );
		}

		AType::sReset();
		VType::sReset();
	}
}

// Reserve

// Comparison

// Hash
#pragma once

#include <cstdint>
#include <cstddef>
#include <iterator>
#include <memory>
#include <type_traits>
#include <concepts>
#include <ranges>
#include <cassert>
#include <initializer_list>
#include <stdexcept>

namespace LibEntropy
{
	template <typename T, typename Allocator = std::allocator<T>>
	class [[nodiscard]] Array : private Allocator
	{
	public:
		using value_type				= T;
		using allocator_type			= Allocator;
		using size_type					= uint32_t;
		using difference_type			= std::ptrdiff_t;
		using pointer					= T*;
		using const_pointer				= const T*;
		using reference					= T&;
		using const_reference			= const T&;

		using iterator					= T*;
		using const_iterator			= const T*;

		using reverse_iterator			= std::reverse_iterator<iterator>;
		using const_reverse_iterator	= std::reverse_iterator<const_iterator>;

		using alloc_traits				= std::allocator_traits<allocator_type>;

	private:
		/// Grows the array by at least inAmount elements
		void grow( size_type inAmount = 1 )
		{
			const size_type minSize = mSize + inAmount;

			// Overflow check - mSize + inAmount wraps silently on uint32_t
			assert( minSize >= mSize );

			if ( minSize > mCapacity ) {
				const size_type newCapacity = std::max( minSize, mCapacity + mCapacity / 2 ); // 1.5x growth
				reserve( newCapacity );
			}
		}

		/// Returns the new capacity caused by a grow operation without calling reserve/reallocate
		size_type sizeof_grow( size_type inNewMinimum )
		{
			// Overflow check and correctness check for unnecessary grow
			assert( inNewMinimum > mCapacity );

			return std::max( inNewMinimum, mCapacity + mCapacity / 2 ); // 1.5x growth
		}

		/// Reallocates all data into a new block of size inNewCapacity
		void reallocate( size_type inNewCapacity )
		{
			assert( inNewCapacity > 0 && inNewCapacity >= mSize );

			pointer newElements = alloc_traits::allocate( get_allocator(), inNewCapacity );

			if constexpr ( std::is_trivially_copyable_v<T> ) {
				std::memcpy( newElements, mData, mSize * sizeof( T ) ); // TODO: memmove?
			}

			// Move if noexcept OR copying is impossible
			else if constexpr ( std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T> ) {
				for ( size_type i = 0; i < mSize; ++i ) {
					alloc_traits::construct( get_allocator(), newElements + i, std::move( mData[i] ) );
				}
			}

			// Otherwise copy for strong guarantee
			else {
				for ( size_type i = 0; i < mSize; ++i ) {
					alloc_traits::construct( get_allocator(), newElements + i, mData[i] );
				}
			}

			destruct( 0, mSize );
			alloc_traits::deallocate( get_allocator(), mData, mCapacity );

			mData = newElements;
			mCapacity = inNewCapacity;
		}

		/// Reallocates all data into a new block of size inNewCapacity, creating a gap at inMiddle of inCount
		/// @note only sets mData and mCapacity; does NOT set new mSize
		void reallocate_gapped( size_type inNewCapacity, size_type inPos, size_type inCount )
		{
			// Ensure valid capcity, ensure new capacity can account for all elements + overflow check
			assert( inNewCapacity > 0 && inNewCapacity >= mSize + inCount );

			// Ensure valid position:
			// if inPos = mSize we are tail appending; should not use this method
			// if inPos > mSize the position is invalide
			assert( inPos < mSize );

			// Allocate new memory chunk
			pointer newElements = alloc_traits::allocate( get_allocator(), inNewCapacity );

			// Head = 0...inPos-1
			// NewHead begin = 0
			// NewHead count = inPos

			// Tail = inPos...mSize-1
			// NewTail begin = inPos + inCount
			// NewTail count = mSize - inPos

			// Trivial path
			if constexpr ( std::is_trivially_copyable_v<T> ) {
				size_type newTailBegin = inPos + inCount;
				size_type tailCount = mSize - inPos;

				std::memcpy( newElements, mData, inPos * sizeof( T ) ); // Copy head
				std::memcpy( newElements + newTailBegin, mData + inPos, tailCount * sizeof( T ) ); // Copy tail
			}

			// Otherwise basic guarantee; strong guarantee is irrelevant to gapped insertions
			else {

				// Move head
				for ( size_type i = 0; i < inPos; ++i ) {
					alloc_traits::construct( get_allocator(), newElements + i, std::move( mData[i] ) );
				}

				// Move tail
				for ( size_type i = inPos; i < mSize; ++i ) {
					alloc_traits::construct( get_allocator(), newElements + i + inCount, std::move( mData[i] ) );
				}
			}

			destruct( 0, mSize );
			alloc_traits::deallocate( get_allocator(), mData, mCapacity );

			mData = newElements;
			mCapacity = inNewCapacity;
		}

		/// Shifts elements starting from inIndex left by inShift slots
		void move_left( size_type inIndex, size_type inShift ) noexcept( std::is_nothrow_move_assignable_v<T> )
		{
			assert( inIndex + inShift <= mSize );

			if ( inShift == 0 ) return;
			if ( inIndex == mSize ) return;

			// Number of elements to move
			size_type tail = mSize - inIndex - inShift;

			// Trivially copyable, use memmove
			if constexpr ( std::is_trivially_copyable_v<T> ) {
				std::memmove( mData + inIndex, mData + inIndex + inShift, tail * sizeof( T ) );
			}

			// Otherwise use move
			else {
				// Move assign over initialised
				std::move( mData + inIndex + inShift, mData + mSize, mData + inIndex );

				// Destruct remaining gap
				destruct( mSize - inShift, mSize );
			}
		}

		/// Shifts elements starting from inIndex right by inShift slots
		void move_right( size_type inIndex, size_type inShift ) noexcept( std::is_nothrow_move_constructible_v<T> && std::is_nothrow_move_assignable_v<T> )
		{
			if ( inShift == 0 ) return;
			if ( inIndex == mSize ) return;

			// Number of elements to move
			size_type tail = mSize - inIndex;

			// Trivially copyable, use memmove
			if constexpr ( std::is_trivially_copyable_v<T> ) {
				std::memmove( mData + inIndex + inShift, mData + inIndex, tail * sizeof( T ) );
			}

			// Otherwise use move backwards + move construct
			else {
				// Number of elements that move into uninitialized storage
				size_type construct_count = std::min( inShift, tail );

				// Move construct into uninitialised
				for ( size_type i = 0; i < construct_count; ++i ) {
					size_type srcIdx = mSize - 1 - i;
					alloc_traits::construct( get_allocator(), mData + srcIdx + inShift, std::move( mData[srcIdx] ) );
				}

				// Move assign over initialised
				if ( construct_count < tail ) {
					std::move_backward( mData + inIndex, mData + mSize - construct_count, mData + mSize );
				}

				// Destruct remaining gap
				//destruct( inIndex, inIndex + construct_count );
			}
		}

		/// Destructs elements [inStart, inEnd - 1]
		void destruct( size_type inStart, size_type inEnd )
		{
			if constexpr ( !std::is_trivially_destructible_v<T> ) {
				if ( inStart < inEnd ) {
					for ( pointer pElem = mData + inStart, pEnd = mData + inEnd; pElem < pEnd; ++pElem ) {
						alloc_traits::destroy( get_allocator(), pElem );
					}
				}
			}
		}

		/// Deallocates mData and sets mCapacity to zero (does not call destructors)
		void free()
		{
			alloc_traits::deallocate( get_allocator(), mData, mCapacity );
			mData = nullptr;
			mCapacity = 0;
		}

		/// Destructs all elements and frees memory
		void destroy()
		{
			if ( mData ) {
				clear();
				free();
			}
		}

	public:

	#pragma region Constructors
		/// @name Constructors
		/// @{
		
		Array() noexcept( noexcept( Allocator() ) ) = default;

		// Allocator-aware default constructor
		explicit Array( const Allocator &inAllocator ) noexcept :
			Allocator( inAllocator )
		{}

		// Size constructor
		explicit Array( size_type inLength, const Allocator &inAllocator = Allocator{} ) :
			Allocator( inAllocator )
		{
			resize( inLength );
		}

		// Size constructor with fill value
		Array( size_type inLength, const T &inValue, const Allocator &inAllocator = Allocator{} ) :
			Allocator( inAllocator )
		{
			resize( inLength, inValue );
		}

		// Initializer list constructor
		Array( std::initializer_list<T> inList, const Allocator &inAllocator = Allocator{} ) :
			Allocator( inAllocator )
		{
			assign( inList );
		}

		// Iterator range constructor
		template <typename Iterator>
		Array( Iterator inBegin, Iterator inEnd, const Allocator &inAllocator = Allocator{} ) :
			Allocator( inAllocator )
		{
			assign( inBegin, inEnd );
		}

		// Copy constructor - informs allocator that the container is being copied if necessary
		Array( const Array &inRHS ) :
			Allocator( alloc_traits::select_on_container_copy_construction( inRHS.get_allocator() ) )
		{
			assign( inRHS.begin(), inRHS.end() );
		}

		// Copy constructor - with explict allocator override
		Array( const Array &inRHS, const Allocator &inAllocator ) : // TODO: type_identity_t?
			Allocator( inAllocator )
		{
			assign( inRHS.begin(), inRHS.end() );
		}

		// Move construct - take full ownership from source
		Array( Array &&inRHS ) noexcept( alloc_traits::is_always_equal::value && std::is_nothrow_move_constructible_v<Allocator> ) :
			Allocator( std::move( inRHS.get_allocator() ) ),
			mData( inRHS.mData ),
			mSize( inRHS.mSize ),
			mCapacity( inRHS.mCapacity )
		{
			inRHS.mData = nullptr;
			inRHS.mSize = 0;
			inRHS.mCapacity = 0;
		}

		// Move constructor - with explict allocator override
		Array( Array &&inRHS, const Allocator &inAllocator ) noexcept : // TODO: type_identity_t? noexcept?? move iterator assignment???
			Allocator( inAllocator )
		{
			if ( get_allocator() == inRHS.get_allocator() ) {
				// Allocators match: safe to steal the buffer
				mData = inRHS.mData;
				mSize = inRHS.mSize;
				mCapacity = inRHS.mCapacity;

				inRHS.mData = nullptr;
				inRHS.mSize = 0;
				inRHS.mCapacity = 0;
			}
			else {
				// Allocators differ: must move element by element
				assign( std::make_move_iterator( inRHS.begin() ), std::make_move_iterator( inRHS.end() ) );
			}
		}

		// Destructor
		~Array()
		{
			destroy();
		}

		/// @}
	#pragma endregion

	#pragma region Assignment Operators
		/// @name Assignment Operators
		/// @{
		
		// Copy assignment
		Array& operator=( const Array &inRHS )
		{
			// Do nothing if same
			if ( this == &inRHS ) return *this;

			if constexpr ( alloc_traits::propagate_on_container_copy_assignment::value ) {
				if ( get_allocator() != inRHS.get_allocator() ) {
					// Must free with the old allocator before replacing it
					destroy();
				}
				get_allocator() = inRHS.get_allocator();
			}

			assign( inRHS.begin(), inRHS.end() );
			return *this;
		}

		// Move assignment
		Array& operator=( Array &&inRHS ) noexcept
		{
			// Do nothing if same
			if ( this == &inRHS ) return *this;

			// Propagation enabled: free our buffer, steal theirs, take their allocator
			if constexpr ( alloc_traits::propagate_on_container_move_assignment::value ) {
				destroy();

				get_allocator() = std::move( inRHS.get_allocator() );

				mData = inRHS.mData;
				mSize = inRHS.mSize;
				mCapacity = inRHS.mCapacity;

				inRHS.mData = nullptr;
				inRHS.mSize = 0;
				inRHS.mCapacity = 0;
			}

			// Propagation disabled but allocators match: safe to steal
			else if ( get_allocator() == inRHS.get_allocator() ) {
				destroy();

				mData = inRHS.mData;
				mSize = inRHS.mSize;
				mCapacity = inRHS.mCapacity;

				inRHS.mData = nullptr;
				inRHS.mSize = 0;
				inRHS.mCapacity = 0;
			}

			// Propagation disabled and allocators differ: must move element by element, and leave rhs buffer
			else {
				assign( std::make_move_iterator( inRHS.begin() ), std::make_move_iterator( inRHS.end() ) );
			}

			return *this;
		}

		// Initialiser list assign
		Array& operator=( std::initializer_list<T> inRHS )
		{
			assign( inRHS );
			return *this;
		}

		/// @}
	#pragma endregion

	#pragma region Assignment Methods
		/// @name Assignment Methods
		/// @{

		void assign( size_type inCount, const T &inValue )
		{
			// Ensure no self value assignment!
			assert( &inValue < mData || &inValue >= mData + mSize );

			// Trivially copyable fast path
			if constexpr ( std::is_trivially_copyable_v<T> ) {

				// If count greater than capacity destroy data, reserve new memory
				if ( inCount > mCapacity ) {
					destroy();
					reserve( inCount );
				}

				if constexpr ( sizeof( T ) == 1 ) {
					// memset is valid when T is a single byte type
					std::memset( mData, static_cast<int>( inValue ), inCount );
				}
				else {
					// std::fill handles both the overlap and tail regions since no construction semantics are required for trivial types 
					std::fill( mData, mData + inCount, inValue );
				}

				mSize = inCount;
			}

			// Non trivial path
			else {

				// If count greater than capacity destroy data, reserve new memory, loop construct
				if ( inCount > mCapacity ) {
					destroy();
					reserve( inCount );

					for ( size_type i = 0; i < inCount; ++i ) {
						alloc_traits::construct( get_allocator(), mData + i, inValue );
					}

					mSize = inCount;
				}

				// Otherwise, assign into initialised range, construct into uninitialised range, destruct any remainder
				else {
					size_type overlap = std::min( inCount, mSize );

					for ( size_type i = 0; i < overlap; ++i ) {
						mData[i] = inValue;
					}

					for ( size_type i = overlap; i < inCount; ++i ) {
						alloc_traits::construct( get_allocator(), mData + i, inValue );
					}

					destruct( inCount, mSize );
					mSize = inCount;
				}

			}
		}
	private:
		template <typename Iterator> requires std::contiguous_iterator<Iterator> && std::is_trivially_copyable_v<T>
		void assign_contiguous_trivial( Iterator inFirst, size_type inCount )
		{
			//TODO: OPTIMIZE PATHS

			if ( mCapacity >= inCount ) {
				std::memcpy( mData, std::to_address( inFirst ), inCount * sizeof( T ) );
			}
			else {
				//TODO: OPTIMIZE DESTROY
				destroy();
				reserve( inCount );

				std::memcpy( mData, std::to_address( inFirst ), inCount * sizeof( T ) );
			}

			mSize = inCount;
		}

		template <typename Iterator> requires std::forward_iterator<Iterator>
		void assign_forward_counted( Iterator inFirst, size_type inCount )
		{
			//TODO: OPTIMIZE PATHS

			if ( mCapacity >= inCount ) {
				for ( size_type i = 0; i < inCount; ++i ) {
					if ( i < mSize ) {
						*( mData + i ) = *( inFirst++ );
					}
					else {
						alloc_traits::construct( get_allocator(), mData + i, *( inFirst++ ) );
					}
				}

				destruct( inCount, mSize );
			}
			else {
				//TODO: OPTIMIZE DESTROY
				destroy();
				reserve( inCount );

				for ( size_type i = 0; i < inCount; ++i ) {
					alloc_traits::construct( get_allocator(), mData + i, *( inFirst++ ) );
				}
			}

			mSize = inCount;
		}

		template <typename Iterator> requires std::forward_iterator<Iterator>
		void assign_sized( Iterator inFirst, size_type inCount )
		{
			if constexpr ( std::contiguous_iterator<Iterator> && std::is_trivially_copyable_v<T> ) {
				assign_contiguous_trivial( inFirst, inCount );
			}
			else {
				assign_forward_counted( inFirst, inCount );
			}
		}

		template <typename Iterator>
		void assign_uncounted( Iterator inFirst, Iterator inLast )
		{
			size_type count = 0;

			// Assign over
			for ( ; count < mSize && inFirst != inLast; ++count ) {
				mData[count] = *( inFirst++ );
			}

			// Destroy leftover initialised elements if exhausted
			destruct( count, mSize );

			// If inexhausted keep allocating
			for ( ; count < mCapacity && inFirst != inLast; ++count ) {
				alloc_traits::construct( get_allocator(), mData + count, *( inFirst++ ) );
			}

			// Set current size, keep going if inexhausted
			mSize = count;

			for ( ; inFirst != inLast; ++inFirst ) {
				if ( mCapacity == mSize ) grow();
				alloc_traits::construct( get_allocator(), mData + mSize, *inFirst );
				++mSize;
			}
		}

	public:
		// Iterator range assign
		template <typename Iterator> requires std::input_iterator<Iterator> // TODO: FIXME, should be any iterator!
		void assign( Iterator inFirst, Iterator inLast )
		{
			if constexpr ( std::forward_iterator<Iterator> ) {
				const std::ptrdiff_t signedCount = std::ranges::distance( inFirst, inLast );
				assert( signedCount >= 0 );
				assert( signedCount <= std::numeric_limits<uint32_t>::max() );
				const size_type count = static_cast<size_type>( signedCount );

				// TODO: FIXME, ensure signedCount < max_size()

				assign_sized( inFirst, count );
			}
			else {
				assign_uncounted( inFirst, inLast );
			}
		}

		// Initializer list assign
		void assign( std::initializer_list<T> inList )
		{
			assign_sized( inList.begin(), static_cast<size_type>( inList.size() ) );
		}
		
		/// @}
	#pragma endregion

	#pragma region Iterators
		/// @name Iterators
		/// @{

		// forward iterators
		iterator begin() noexcept              { return mData; }
		iterator end() noexcept                { return mData + mSize; }

		const_iterator begin() const noexcept  { return mData; }
		const_iterator end() const noexcept    { return mData + mSize; }

		const_iterator cbegin() const noexcept { return mData; }
		const_iterator cend() const noexcept   { return mData + mSize; }

		// reverse iterators
		reverse_iterator rbegin() noexcept              { return reverse_iterator( mData + mSize ); }
		reverse_iterator rend() noexcept                { return reverse_iterator( mData ); }

		const_reverse_iterator rbegin() const noexcept  { return const_reverse_iterator( mData + mSize ); }
		const_reverse_iterator rend() const noexcept    { return const_reverse_iterator( mData ); }

		const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator( mData + mSize ); }
		const_reverse_iterator crend() const noexcept   { return const_reverse_iterator( mData ); }

		/// @}
	#pragma endregion

	#pragma region Capacity Methods
		/// @name Capacity Methods
		/// @{

		bool empty() const noexcept			{ return mSize == 0; }
		size_type size() const noexcept		{ return mSize; }
		size_type max_size() const noexcept	{ return static_cast<size_type>( std::min( alloc_traits::max_size( get_allocator() ), static_cast<size_t>( std::numeric_limits<size_type>::max() )) ); }
		size_type capacity() const noexcept { return mCapacity;  }

		void resize( size_type inNewSize )
		{
			destruct( inNewSize, mSize ); // Destruct tail, if any
			reserve( inNewSize ); // Reserve new size, if needed

			if constexpr ( !std::is_trivially_default_constructible_v<T> ) {
				for ( size_type i = mSize; i < inNewSize; ++i ) {
					alloc_traits::construct( get_allocator(), mData + i );
				}
			}
			else {
				// TODO: should we zero init? Or is blank init okay?
				// FIXME
			}

			mSize = inNewSize;
		}

		void resize( size_type inNewSize, const T& inValue )
		{
			// Guard against passing a reference to one of our own elements
			assert( &inValue < mData || &inValue >= mData + mSize );

			if ( inNewSize < mSize ) {
				destruct( inNewSize, mSize );
				mSize = inNewSize;
			}
			else if ( inNewSize > mSize ) {
				reserve( inNewSize );

				if constexpr ( std::is_trivially_copyable_v<T> ) {
					if constexpr ( sizeof( T ) == 1 ) {
						std::memset( mData + mSize, static_cast<int>( inValue ), inNewSize - mSize );
					}
					else {
						std::fill( mData + mSize, mData + inNewSize, inValue );
					}
				}
				else {
					for ( size_type i = mSize; i < inNewSize; ++i ) {
						alloc_traits::construct( get_allocator(), mData + i, inValue );
					}
				}

				mSize = inNewSize;
			}
		}

		void reserve( size_type inNewSize )
		{
			if ( mCapacity < inNewSize ) {
				reallocate( inNewSize );
			}
		}

		void shrink_to_fit()
		{
			if ( mData != nullptr ) {
				if ( mSize == 0 ) {
					free();
				}
				else if ( mCapacity > mSize ) {
					reallocate( mSize );
				}
			}
		}

		/// @}
	#pragma endregion

	#pragma region Element Access
		/// @name Element Access
		/// @{

		reference operator[]( size_type n )				{ assert( n < mSize ); return mData[n]; }
		const_reference operator[]( size_type n ) const { assert( n < mSize ); return mData[n]; }

		reference at( size_type n )						{ if( n >= mSize ) throw std::out_of_range( "Array::at() index out of range" ); return mData[n]; }
		const_reference at( size_type n ) const			{ if( n >= mSize ) throw std::out_of_range( "Array::at() index out of range" ); return mData[n]; }

		reference front()								{ assert( mSize > 0 ); return mData[0]; }
		const_reference front() const					{ assert( mSize > 0 ); return mData[0]; }

		reference back()								{ assert( mSize > 0 ); return mData[mSize - 1]; }
		const_reference back() const					{ assert( mSize > 0 ); return mData[mSize - 1]; }

		/// @}
	#pragma endregion

	#pragma region Data Access
		/// @name Data Access
		/// @{

		pointer data() noexcept				{ return mData; }
		const_pointer data() const noexcept { return mData; }

		/// @}
	#pragma endregion

	#pragma region Insertion Methods
		/// @name Insertion Methods
		/// @{

		template<typename... Args>
		reference emplace_back( Args&&... args ) {
			if ( mCapacity == mSize ) grow();
			alloc_traits::construct( get_allocator(), mData + mSize, std::forward<Args>( args )... );
			++mSize;
			return back();
		}

		void push_back( const T &inValue )
		{
			assert( &inValue < mData || &inValue >= mData + mSize );

			if ( mCapacity == mSize ) grow();
			alloc_traits::construct( get_allocator(), mData + mSize, inValue );
			++mSize;
		}

		void push_back( T &&inValue )
		{
			assert( &inValue < mData || &inValue >= mData + mSize );

			if ( mCapacity == mSize ) grow();
			alloc_traits::construct( get_allocator(), mData + mSize, std::move( inValue ) );
			++mSize;
		}

		template <typename... Args>
		iterator emplace( const_iterator inPos, Args&&... inArgs )
		{
			assert( inPos >= begin() && inPos <= end() );

			const std::ptrdiff_t signedIndex = inPos - mData;
			assert( signedIndex >= 0 && signedIndex <= max_size() );

			const size_type index = static_cast<size_type>( signedIndex );

			// Capcity greater than size, therefor space to insert
			if ( mCapacity > mSize ) {

				if ( index >= mSize ) {
					alloc_traits::construct( get_allocator(), mData + index, std::forward<Args>( inArgs )... );
				}
				else {
					move_right( index, 1 );
					*( mData + index ) = T( std::forward<Args>( inArgs )... );
				}
			}

			// Insert at back after growth
			else if ( index == mSize ) {
				grow();
				alloc_traits::construct( get_allocator(), mData + index, std::forward<Args>( inArgs )... );
			}

			// Otherwise insert in middle after reallocate grow and construct in uninitialised gap
			else {
				reallocate_gapped( sizeof_grow( mSize + 1 ), index, 1 );
				alloc_traits::construct( get_allocator(), mData + index, std::forward<Args>( inArgs )... );
			}

			++mSize;

			return mData + index;
		}

		iterator insert( const_iterator inPos, const T &inValue )
		{
			assert( &inValue < mData || &inValue >= mData + mSize );
			
			assert( inPos >= begin() && inPos <= end() );

			const std::ptrdiff_t signedIndex = inPos - mData;
			assert( signedIndex >= 0 && signedIndex <= max_size() );

			const size_type index = static_cast<size_type>( signedIndex );

			// Capcity greater than size, therefor space to insert
			if ( mCapacity > mSize ) {

				if ( index >= mSize ) {
					alloc_traits::construct( get_allocator(), mData + index, inValue );
				}
				else {
					move_right( index, 1 );
					*( mData + index ) = inValue;
				}
			}

			// Insert at back after growth
			else if ( index == mSize ) {
				grow();
				alloc_traits::construct( get_allocator(), mData + index, inValue );
			}

			// Otherwise insert in middle after reallocate grow and construct in uninitialised gap
			else {
				reallocate_gapped( sizeof_grow( mSize + 1 ), index, 1 );
				alloc_traits::construct( get_allocator(), mData + index, inValue );
			}

			++mSize;

			return mData + index;
		}

		iterator insert( const_iterator inPos, T &&inValue )
		{
			assert( &inValue < mData || &inValue >= mData + mSize );

			assert( inPos >= begin() && inPos <= end() );

			const std::ptrdiff_t signedIndex = inPos - mData;
			assert( signedIndex >= 0 && signedIndex <= max_size() );

			const size_type index = static_cast<size_type>( signedIndex );

			// Capcity greater than size, therefor space to insert
			if ( mCapacity > mSize ) {

				if ( index >= mSize ) {
					alloc_traits::construct( get_allocator(), mData + index, std::move( inValue ) );
				}
				else {
					move_right( index, 1 );
					*( mData + index ) = std::move( inValue );
				}
			}

			// Insert at back after growth
			else if ( index == mSize ) {
				grow();
				alloc_traits::construct( get_allocator(), mData + index, std::move( inValue ) );
			}

			// Otherwise insert in middle after reallocate grow and construct in uninitialised gap
			else {
				reallocate_gapped( sizeof_grow( mSize + 1 ), index, 1 );
				alloc_traits::construct( get_allocator(), mData + index, std::move( inValue ) );
			}

			++mSize;

			return mData + index;
		}

		iterator insert( const_iterator inPos, size_type inCount, const T& inValue )
		{
			assert( &inValue < mData || &inValue >= mData + mSize );
			assert( inPos >= begin() && inPos <= end() );

			if ( inCount == 0 ) return const_cast<iterator>( inPos );

			const std::ptrdiff_t signedIndex = inPos - mData;
			assert( signedIndex >= 0 && signedIndex <= max_size() );

			const size_type index = static_cast<size_type>( signedIndex );

			// Capcity fits new size, therefor space to insert
			if ( mCapacity >= mSize + inCount ) {

				// At back, no move needed
				if ( index == mSize ) {
					if constexpr ( std::is_trivially_copyable_v<T> ) {
						// Single byte type
						if constexpr ( sizeof( T ) == 1 ) {
							std::memset( mData + index, static_cast<int>( inValue ), inCount );
						}
						// Multi-byte trivially copyable
						else {
							std::fill( mData + index, mData + index + inCount, inValue );
						}
					}
					else {
						// Non-trivial type, must construct each element (only construct as back is uninitialised)
						for ( size_type i = 0; i < inCount; ++i ) {
							alloc_traits::construct( get_allocator(), mData + index + i, inValue );
						}
					}
				}

				// In middle, move needed
				else {
					move_right( index, inCount );

					if constexpr ( std::is_trivially_copyable_v<T> ) {
						// Single byte type
						if constexpr ( sizeof( T ) == 1 ) {
							std::memset( mData + index, static_cast<int>( inValue ), inCount );
						}
						// Multi-byte trivially copyable
						else {
							std::fill( mData + index, mData + index + inCount, inValue );
						}
					}
					else {
						// Non-trivial type, must construct each element properly (potentially mixed construct and assign)
						for ( size_type i = 0; i < inCount; ++i ) {
							if ( index + i < mSize ) {
								*( mData + index + i ) = inValue;
							}
							else {
								alloc_traits::construct( get_allocator(), mData + index + i, inValue );
							}
						}
					}
				}
			}

			// Insert at back after growth
			else if ( index == mSize ) {
				grow( inCount );

				if constexpr ( std::is_trivially_copyable_v<T> ) {
					// Single byte type
					if constexpr ( sizeof( T ) == 1 ) {
						std::memset( mData + index, static_cast<int>( inValue ), inCount );
					}
					// Multi-byte trivially copyable
					else {
						std::fill( mData + index, mData + index + inCount, inValue );
					}
				}
				else {
					// Non-trivial type, must construct each element properly (only construct as back is uninitialised)
					for ( size_type i = 0; i < inCount; ++i ) {
						alloc_traits::construct( get_allocator(), mData + index + i, inValue );
					}
				}
			}

			// Else reallocate
			else {
				reallocate_gapped( sizeof_grow( mSize + inCount ), index, inCount );

				if constexpr ( std::is_trivially_copyable_v<T> ) {
					// Single byte type
					if constexpr ( sizeof( T ) == 1 ) {
						std::memset( mData + index, static_cast<int>( inValue ), inCount );
					}
					// Multi-byte trivially copyable
					else {
						std::fill( mData + index, mData + index + inCount, inValue );
					}
				}
				else {
					// Non-trivial type, must construct each element properly (only construct, as gap is uninitialsed)
					for ( size_type i = 0; i < inCount; ++i ) {
						alloc_traits::construct( get_allocator(), mData + index + i, inValue );
					}
				}
			}

			mSize += inCount;
			return mData + index;
		}

	private:
		template <typename Iterator> requires std::contiguous_iterator<Iterator> && std::is_trivially_copyable_v<T>
		iterator insert_contiguous_trivial( size_type inIndex, Iterator inFirst, const size_type inCount ) // FIXME: can we merge branches?
		{
			// IMPORTANT: FIXME! STD::TO_ADDRESS!!!!
			// Capacity fits new size
			if ( mCapacity >= mSize + inCount ) {

				// At back, no move needed
				if ( inIndex == mSize ) {
					std::memcpy( mData + inIndex, std::to_address( inFirst ), inCount * sizeof( T ) ); // TODO, we can merge both branches!!!
				}

				// In middle, move needed
				else {
					move_right( inIndex, inCount ); // TODO, we can merge both branches!!!
					std::memcpy( mData + inIndex, std::to_address( inFirst ), inCount * sizeof( T ) );
				}
			}

			// Insert at back after growth
			else if ( inIndex == mSize ) {
				grow( inCount );
				std::memcpy( mData + inIndex, std::to_address( inFirst ), inCount * sizeof( T ) );
			}

			// Insert middle after gapped reallocate
			else {
				reallocate_gapped( sizeof_grow( mSize + inCount ), inIndex, inCount );
				std::memcpy( mData + inIndex, std::to_address( inFirst ), inCount * sizeof( T ) );
			}

			// Must increment size here!
			mSize += inCount;

			return mData + inIndex;
		}

		template <typename Iterator> requires std::forward_iterator<Iterator>
		iterator insert_forward_counted( size_type inIndex, Iterator inFirst, const size_type inCount ) // FIXME: can we merge branches?
		{
			// Capacity fits new size
			if ( mCapacity >= mSize + inCount ) {

				// At back, no move needed
				if ( inIndex == mSize ) {
					// Must construct each element (only construct as back is uninitialised)
					for ( size_type i = 0; i < inCount; ++i ) {
						alloc_traits::construct( get_allocator(), mData + inIndex + i, *( inFirst++ ) );
					}
				}

				// In middle, move needed
				else {
					move_right( inIndex, inCount );

					// Must construct each element properly (potentially mixed construct and assign)
					for ( size_type i = 0; i < inCount; ++i ) {
						if ( inIndex + i < mSize ) {
							*( mData + inIndex + i ) = *( inFirst++ );
						}
						else {
							alloc_traits::construct( get_allocator(), mData + inIndex + i, *( inFirst++ ) );
						}
					}
				}
			}

			// Insert at back after growth
			else if ( inIndex == mSize ) {
				grow( inCount );

				// Must construct each element (only construct as back is uninitialised)
				for ( size_type i = 0; i < inCount; ++i ) {
					alloc_traits::construct( get_allocator(), mData + inIndex + i, *( inFirst++ ) );
				}
			}

			// Insert middle after gapped reallocate
			else {
				reallocate_gapped( sizeof_grow( mSize + inCount ), inIndex, inCount );

				// Must construct each element properly (only construct, as gap is uninitialsed)
				for ( size_type i = 0; i < inCount; ++i ) {
					alloc_traits::construct( get_allocator(), mData + inIndex + i, *( inFirst++ ) );
				}
			}

			// Must increment size here!
			mSize += inCount;

			return mData + inIndex;
		}

		template <typename Iterator>
		iterator insert_uncounted( size_type inIndex, Iterator inFirst, Iterator inLast )
		{
			size_type originalSize = mSize;

			for ( ; inFirst != inLast; ++inFirst ) emplace_back( *inFirst );
			std::rotate( mData + inIndex, mData + originalSize, mData + mSize );
			return mData + inIndex;
		}

	public:
		template <typename Iterator> // TODO: FIXME, should require iterator
		iterator insert( const_iterator inPos, Iterator inFirst, Iterator inLast )
		{
			assert( inPos >= begin() && inPos <= end() && "Iterator out of range" );

			const std::ptrdiff_t signedIndex = inPos - mData;
			assert( signedIndex >= 0 && signedIndex <= max_size() );
			const size_type index = static_cast<size_type>( signedIndex );

			// Forward iterator, we can obtain size in O(1) or O(n) two pass
			if constexpr ( std::forward_iterator<Iterator> ) {
				const std::ptrdiff_t signedCount = std::ranges::distance( inFirst, inLast );
				assert( signedCount >= 0 );
				assert( signedCount <= std::numeric_limits<uint32_t>::max() );
				const size_type count = static_cast<size_type>( signedCount );

				// TODO: FIXME, ensure signedCount + mSize < max_size()

				if constexpr ( std::contiguous_iterator<Iterator> && std::is_trivially_copyable_v<T> ) {
					return insert_contiguous_trivial( index, inFirst, count );
				}
				else {
					return insert_forward_counted( index, inFirst, count );
				}
			}

			// Single pass iterator, must perform O(n) single pass growth and rotate
			else {
				return insert_uncounted( index, inFirst, inLast );
			}
		}

		iterator insert( const_iterator inPos, std::initializer_list<T> inList )
		{
			return insert( inPos, inList.begin(), inList.end() );
		}

		/// @}
	#pragma endregion

	#pragma region Erasure Methods
		/// @name Erasure Methods
		/// @{

		void pop_back() noexcept( std::is_nothrow_destructible_v<T> )
		{
			assert( mSize > 0 );

			--mSize;
			alloc_traits::destroy( get_allocator(), mData + mSize );
		}

		iterator erase( const_iterator inPos ) noexcept( std::is_nothrow_move_assignable_v<value_type> )
		{
			assert( inPos >= begin() && inPos < end() );

			const std::ptrdiff_t signedIndex = inPos - mData;
			assert( signedIndex >= 0 && signedIndex <= max_size() );
			const size_type index = static_cast<size_type>( signedIndex );

			move_left( index, 1 );

			--mSize;

			return mData + index;
		}

		iterator erase( const_iterator inFirst, const_iterator inLast )
		{
			assert( inFirst >= begin() && inFirst <= end() );
			assert( inLast >= inFirst && inLast <= end() );

			const std::ptrdiff_t signedFirst = inFirst - mData;
			assert( signedFirst >= 0 && signedFirst <= std::numeric_limits<uint32_t>::max() );

			const std::ptrdiff_t signedCount = inLast - inFirst;
			assert( signedCount >= 0 && signedCount <= std::numeric_limits<uint32_t>::max() );

			// TODO: FIXME, ensure signedCount + mSize < max_size()

			const size_type index = static_cast<size_type>( signedFirst );
			const size_type count = static_cast<size_type>( signedCount );

			if ( count == 0 ) return mData + index;

			move_left( index, count );

			mSize -= count;
			return mData + index;
		}

		/// @}
	#pragma endregion

		void clear() noexcept
		{
			destruct( 0, mSize );
			mSize = 0;
		}

		void swap( Array& inRHS ) noexcept( alloc_traits::propagate_on_container_swap::value || alloc_traits::is_always_equal::value )
		{
			if ( this == &inRHS )
				return;

			if constexpr ( alloc_traits::propagate_on_container_swap::value ) {
				// Allocator travels with the data; swap everything including allocator
				std::swap( static_cast<Allocator&>( *this ), static_cast<Allocator&>( inRHS ) );
				std::swap( mData, inRHS.mData );
				std::swap( mSize, inRHS.mSize );
				std::swap( mCapacity, inRHS.mCapacity );
			}
			else if ( get_allocator() == inRHS.get_allocator() ) {
				// Allocators don't propagate but are equal; swap
				std::swap( mData, inRHS.mData );
				std::swap( mSize, inRHS.mSize );
				std::swap( mCapacity, inRHS.mCapacity );
			}
			else {
				// Allocators don't propagate and are unequal; TODO handle this case for release
				assert( false && "swap() called on arrays with unequal non-propagating allocators" );
			}
		}

		// Allocator helpers
		Allocator&			get_allocator()			{ return *this; }
		const Allocator&	get_allocator() const	{ return *this; }

		// Comparison
		friend bool operator==( const Array& inLHS, const Array& inRHS )
		{
			if ( inLHS.mSize != inRHS.mSize ) return false;
			if ( inLHS.mSize == 0 ) return true;
			if ( inLHS.mData == inRHS.mData ) return true;

			if constexpr ( std::is_trivially_copyable_v<T> && std::has_unique_object_representations_v<T> && !std::is_same_v<T, bool> ) {
				return std::memcmp( inLHS.mData, inRHS.mData, inLHS.mSize * sizeof( T ) ) == 0;
			}
			else {
				for ( size_type i = 0; i < inLHS.mSize; ++i ) {
					if ( !( inLHS.mData[i] == inRHS.mData[i] ) ) return false;
				}

				return true;
			}
		}

		friend auto operator<=>( const Array& inLHS, const Array& inRHS )
		{
			return std::lexicographical_compare_three_way( inLHS.begin(), inLHS.end(), inRHS.begin(), inRHS.end() );
		}

	private: // Internal storage variables
		pointer		mData = nullptr;
		size_type	mSize = 0;
		size_type	mCapacity = 0;
	};

	template <typename T, typename Allocator>
	inline void swap( Array<T, Allocator> &inLHS, Array<T, Allocator> &inRHS ) noexcept
	{
		inLHS.swap( inRHS );
	}
}
////////////////////////////////////////////////////////////////////////////
//	Created 	: 30.09.2008
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

// no header guards, since header may be included into the
// different namespaces to support several allocators

#define TEMPLATE_SIGNATURE		template <typename T>
#define STD_ALLOCATOR			std_allocator<T>

TEMPLATE_SIGNATURE
STD_ALLOCATOR::std_allocator									( )
{
}

TEMPLATE_SIGNATURE
STD_ALLOCATOR::std_allocator									( self_type const& )
{
}

TEMPLATE_SIGNATURE
template<class other>
STD_ALLOCATOR::std_allocator									( std_allocator<other> const& )
{
}

TEMPLATE_SIGNATURE
template<class other>
STD_ALLOCATOR &STD_ALLOCATOR::operator=							( std_allocator<other> const& )
{
	return						( *this );
}

TEMPLATE_SIGNATURE
typename STD_ALLOCATOR::pointer STD_ALLOCATOR::address			( reference value ) const
{
	return						( &value );
}

TEMPLATE_SIGNATURE
typename STD_ALLOCATOR::const_pointer STD_ALLOCATOR::address	( const_reference value ) const
{
	return						( &value );
}

TEMPLATE_SIGNATURE
typename STD_ALLOCATOR::pointer STD_ALLOCATOR::allocate			( size_type const n, void* p ) const
{
#ifdef DEBUG
	return						( (pointer)( USER_ALLOCATOR ).realloc_impl( p, (u32)std::max( (u32)n, (u32)size_t(1) )*sizeof(T), typeid(T).name(), __FUNCTION__, __FILE__, __LINE__ ) );
#else // #ifdef DEBUG
	return						( (pointer)( USER_ALLOCATOR ).realloc_impl( p, (u32)std::max( (u32)n, (u32)size_t(1) )*sizeof(T) ) );
#endif // #ifdef DEBUG
}

TEMPLATE_SIGNATURE
char *STD_ALLOCATOR::__charalloc								( size_type const n )
{
	return 						( (char*)allocate(n) );
}

TEMPLATE_SIGNATURE
void STD_ALLOCATOR::deallocate									( pointer const p, size_type const n ) const
{
	deallocate					( (pvoid)p, n*sizeof(T) );
}

TEMPLATE_SIGNATURE
void STD_ALLOCATOR::deallocate									( void* p, size_type const n ) const
{
	XRAY_UNREFERENCED_PARAMETER	( n );
#ifdef DEBUG
	( USER_ALLOCATOR ).free_impl( p, __FUNCTION__, __FILE__, __LINE__ );
#else // #ifdef DEBUG
	( USER_ALLOCATOR ).free_impl( p );
#endif // #ifdef DEBUG
}

TEMPLATE_SIGNATURE
void STD_ALLOCATOR::construct									( pointer const p, T const& value )
{
	new ( p ) T					( value );
}

TEMPLATE_SIGNATURE
void STD_ALLOCATOR::destroy										( pointer p )
{
	p->~T						( );
}

TEMPLATE_SIGNATURE
typename STD_ALLOCATOR::size_type STD_ALLOCATOR::max_size		( ) const
{
	size_type const count		= ((size_type)(-1))/sizeof(T);
	if ( identity(count) )
		return					( count );

	return						( 1 );
}

#if 0
template < typename left, typename right >
inline bool operator==											( std_allocator<left> const&, std_allocator<right> const& )
{
	return 						( true );
}

template < typename left, typename right >
inline bool operator!=											( std_allocator<left> const&, std_allocator<right> const& )
{
	return 						( false );
}
#endif // #if 0

#undef STD_ALLOCATOR
#undef TEMPLATE_SIGNATURE
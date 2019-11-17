////////////////////////////////////////////////////////////////////////////
//	Created 	: 30.09.2008
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#if !XRAY_USE_CRT_MEMORY_ALLOCATOR

#include <xray/memory_doug_lea_allocator.h>

#define USE_DL_PREFIX
#include "ptmalloc3/malloc-2.8.3.h"

using xray::memory::doug_lea_allocator;

doug_lea_allocator::doug_lea_allocator	( ) :
	m_arena						( 0 ),
	m_user_thread_id			( threading::current_thread_id( ) )
{
}

static void __stdcall out_of_memory		( mspace const space, void const* const parameter, int const first_time )
{
	XRAY_UNREFERENCED_PARAMETER	( parameter );
	XRAY_UNREFERENCED_PARAMETER	( space );
	if ( first_time )
		return;

	xray::memory::dump_statistics	( );
	FATAL						( "not enough memory for arena [%s]", (( doug_lea_allocator* )parameter)->arena_id( ) );
}

void doug_lea_allocator::user_thread_id	( u32 user_thread_id ) const
{
	threading::interlocked_exchange	( (threading::atomic32_value_type&)m_user_thread_id, user_thread_id );
}

void doug_lea_allocator::user_current_thread_id	( ) const
{
	user_thread_id				( threading::current_thread_id( ) );
}

void doug_lea_allocator::initialize_impl( pvoid arena, u64 arena_size, pcstr arena_id )
{
	XRAY_UNREFERENCED_PARAMETER	( arena_id );

	ASSERT						( !m_arena, "arena is not initialized or is initialized more than once" );
	m_arena						= create_xray_mspace_with_base( arena, arena_size, 0, &out_of_memory, (pcvoid)this );
}

void doug_lea_allocator::finalize_impl	( )
{
	ASSERT						( m_arena, "arena is not initialized or is initialized more than once" );
	destroy_mspace				( m_arena );
}

inline bool doug_lea_allocator::initialized	( ) const
{
	if ( !super::initialized( ) )
		return					( false );

	u32 const current_thread_id	= xray::threading::current_thread_id ( );
	XRAY_UNREFERENCED_PARAMETER	( current_thread_id );
	R_ASSERT					( current_thread_id == m_user_thread_id );
	return						( true );
}

pvoid doug_lea_allocator::malloc_impl	( size_t const size XRAY_CORE_DEBUG_PARAMETERS_DESCRIPTION_DECLARATION XRAY_CORE_DEBUG_PARAMETERS_DECLARATION )
{
	XRAY_CORE_DEBUG_PARAMETERS_UNREFERENCED_GUARD;

	ASSERT						( initialized ( ) );
	size_t const real_size		= needed_size( size );
	pvoid const result			= xray_mspace_malloc( m_arena, real_size );
	return						( on_malloc	( result, size, 0, XRAY_CORE_DEBUG_PARAMETERS_DESCRIPTION_PARAMETER ) );
}

pvoid doug_lea_allocator::realloc_impl	( pvoid pointer, size_t const new_size XRAY_CORE_DEBUG_PARAMETERS_DESCRIPTION_DECLARATION XRAY_CORE_DEBUG_PARAMETERS_DECLARATION )
{
	ASSERT						( initialized ( ) );
	if ( !new_size ) {
		free_impl				( pointer XRAY_CORE_DEBUG_PARAMETERS );
		return					( 0 );
	}

	size_t const previous_size	= pointer ? buffer_size( pointer ) : 0;
	if ( pointer )
		on_free					( pointer, false );

	size_t const real_size		= needed_size( new_size );
	pvoid const result			= xray_mspace_realloc( m_arena, pointer, real_size );
	return
		on_malloc(
			result,
			new_size,
			previous_size,
			XRAY_CORE_DEBUG_PARAMETERS_DESCRIPTION_PARAMETER
		);
}

void doug_lea_allocator::free_impl		( pvoid pointer XRAY_CORE_DEBUG_PARAMETERS_DECLARATION )
{
	XRAY_CORE_DEBUG_PARAMETERS_UNREFERENCED_GUARD;

	ASSERT							( initialized ( ) );

	if ( !pointer )
		return;

	on_free						( pointer );

	xray_mspace_free					( m_arena, pointer );
}

pvoid doug_lea_allocator::call_malloc	( size_t size XRAY_CORE_DEBUG_PARAMETERS_DESCRIPTION_DECLARATION XRAY_CORE_DEBUG_PARAMETERS_DECLARATION )
{
	return							( malloc_impl( size XRAY_CORE_DEBUG_PARAMETERS_DESCRIPTION XRAY_CORE_DEBUG_PARAMETERS ) );
}

pvoid doug_lea_allocator::call_realloc	( pvoid pointer, size_t new_size XRAY_CORE_DEBUG_PARAMETERS_DESCRIPTION_DECLARATION XRAY_CORE_DEBUG_PARAMETERS_DECLARATION )
{
	return							( realloc_impl( pointer, new_size XRAY_CORE_DEBUG_PARAMETERS_DESCRIPTION XRAY_CORE_DEBUG_PARAMETERS ) );
}

void doug_lea_allocator::call_free		( pvoid pointer XRAY_CORE_DEBUG_PARAMETERS_DECLARATION )
{
	free_impl						( pointer XRAY_CORE_DEBUG_PARAMETERS );
}

size_t doug_lea_allocator::total_size	( ) const
{
	u32 const owner_thread_id	= m_user_thread_id;
	user_current_thread_id		( );

	ASSERT						( initialized ( ) );
	size_t const result			= xray_mspace_mallinfo( m_arena ).usmblks;

	user_thread_id				( owner_thread_id );

	return						( result );
}

#include "ptmalloc3/malloc-private.h"

size_t doug_lea_allocator::allocated_size( ) const
{
	u32 const owner_thread_id	= m_user_thread_id;
	user_current_thread_id		( );

	ASSERT						( initialized ( ) );
	u32 result					= (u32)xray_mspace_mallinfo( m_arena ).uordblks;

	m_user_thread_id			= owner_thread_id;

	u32 const min_size			= pad_request( sizeof( malloc_state ) );
	ASSERT						( result >= min_size );
	result						-= min_size;
	return						( result );
}

#endif // #if !XRAY_USE_CRT_MEMORY_ALLOCATOR
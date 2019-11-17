////////////////////////////////////////////////////////////////////////////
//	Created 	: 13.10.2008
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_MEMORY_MACROS_H_INCLUDED
#define XRAY_MEMORY_MACROS_H_INCLUDED

#ifdef DEBUG

#	define XRAY_NEW_IMPL( allocator, type )								\
			new ( xray::memory::new_helper<type>::call( strip_pointer(allocator), __FUNCTION__, __FILE__, __LINE__ ) ) ( type )

#	define XRAY_NEW_ARRAY_IMPL( allocator, type, count )				\
			xray::memory::new_array_helper<type>::call( strip_pointer(allocator), count, __FUNCTION__, __FILE__, __LINE__ )

#	define XRAY_DELETE_IMPL( allocator, pointer, ... )					\
			xray::memory::delete_helper			( strip_pointer(allocator), pointer, __FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__ )

#	define XRAY_DELETE_ARRAY_IMPL( allocator, pointer, ... )			\
			xray::memory::delete_array_helper	( strip_pointer(allocator), pointer, __FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__ )

#	define XRAY_MALLOC_IMPL( allocator, size, description )				\
			xray::memory::malloc_helper			( strip_pointer(allocator), size, description, __FUNCTION__, __FILE__, __LINE__ )

#	define XRAY_REALLOC_IMPL( allocator, pointer, size, description )	\
			xray::memory::realloc_helper		( strip_pointer(allocator), pointer, size, description, __FUNCTION__, __FILE__, __LINE__ )

#	define XRAY_FREE_IMPL( allocator, pointer )							\
			xray::memory::free_helper			( strip_pointer(allocator), pointer, __FUNCTION__, __FILE__, __LINE__ )

#else // #ifdef DEBUG

#	define XRAY_NEW_IMPL( allocator, type )								\
			new ( xray::memory::new_helper<type>::call( strip_pointer(allocator) ) ) ( type ) 

#	define XRAY_NEW_ARRAY_IMPL( allocator, type, count )				\
			xray::memory::new_array_helper<type>::call( strip_pointer(allocator), count )

#	define XRAY_DELETE_IMPL( allocator, pointer, ... )					\
			xray::memory::delete_helper			( strip_pointer(allocator), pointer, ##__VA_ARGS__ )

#	define XRAY_DELETE_ARRAY_IMPL( allocator, pointer, ... )			\
			xray::memory::delete_array_helper	( strip_pointer(allocator), pointer, ##__VA_ARGS__ )

#	define XRAY_MALLOC_IMPL( allocator, size, description )				\
			xray::memory::malloc_helper			( strip_pointer(allocator), size )

#	define XRAY_REALLOC_IMPL( allocator, pointer, size, description )	\
			xray::memory::realloc_helper		( strip_pointer(allocator), pointer, size )

#	define XRAY_FREE_IMPL( allocator, pointer )							\
			xray::memory::free_helper			( strip_pointer(allocator), pointer )

#endif // #ifdef DEBUG

#define XRAY_ALLOC_IMPL( allocator, type, count )						\
			( ( type* )XRAY_MALLOC_IMPL( allocator, sizeof( type )*( count ), typeid( type ).name( ) ) )

#endif // #ifndef XRAY_MEMORY_MACROS_H_INCLUDED
////////////////////////////////////////////////////////////////////////////
//	Created 	: 08.10.2008
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_MEMORY_OVERRIDE_OPERATORS_H_INCLUDED
#define XRAY_MEMORY_OVERRIDE_OPERATORS_H_INCLUDED

#define _CRT_ALLOCATION_DEFINED

#include <new.h>

extern "C" {

inline pvoid malloc						( size_t )
{
	UNREACHABLE_CODE(return 0);
#ifndef __SNC__
	__assume(0);
#endif // #ifndef __SNC__
}

inline pvoid calloc						( size_t, size_t )
{
	UNREACHABLE_CODE(return 0);
#ifndef __SNC__
	__assume(0);
#endif // #ifndef __SNC__
}

inline pvoid realloc					( pvoid, size_t )
{
	UNREACHABLE_CODE(return 0);
#ifndef __SNC__
	__assume(0);
#endif // #ifndef __SNC__
}

inline void free						( pvoid )
{
	UNREACHABLE_CODE();
#ifndef __SNC__
	__assume(0);
#endif // #ifndef __SNC__
}

#ifdef _MSC_VER
	_Check_return_ _CRTIMP inline char *  __cdecl _strdup(_In_opt_z_ const char * _Src)
	{
		XRAY_UNREFERENCED_PARAMETER(_Src);
		UNREACHABLE_CODE(return 0);
		__assume(0);
	}

	_Check_return_ _CRT_NONSTDC_DEPRECATE(_strdup) _CRTIMP inline char * __cdecl strdup(_In_opt_z_ const char * _Src)
	{
		XRAY_UNREFERENCED_PARAMETER(_Src);
		UNREACHABLE_CODE(return 0);
		__assume(0);
	}
#else // #ifdef _MSC_VER
	inline char *  __cdecl strdup(const char * _Src)
	{
		XRAY_UNREFERENCED_PARAMETER(_Src);
		UNREACHABLE_CODE(return 0);
#	ifndef __SNC__
		__assume(0);
#	endif // #ifndef __SNC__
	}
#endif // #ifdef _MSC_VER

} // extern "C"

#if defined(_MSC_VER)
#	include <malloc.h>
#endif // #if defined(_MSC_VER)

#include <string.h>
#include <algorithm>
#include <map>

#if defined(_MSC_VER)
#	pragma deprecated(malloc,calloc,realloc,free,strdup,_strdup)
#endif // #if defined(_MSC_VER)

////////////////////////////////////////////////////////////////////////////
// new operators
////////////////////////////////////////////////////////////////////////////
inline pvoid __cdecl operator new		( size_t buffer_size )
{
#if !XRAY_PLATFORM_PS3
	XRAY_UNREFERENCED_PARAMETER	( buffer_size );
	UNREACHABLE_CODE			( return 0 );
	__assume( 0 );
#else // #if !XRAY_PLATFORM_PS3
	// for boost::function functionality only!
	// if you don't see boost::function stuff in the call stack,
	// please report Dmitriy Iassenev ASAP
#	ifdef DEBUG
		return					::xray::memory::g_mt_allocator.malloc_impl( buffer_size, "new for boost::function", __FUNCTION__, __FILE__, __LINE__ );
#	else // #ifdef DEBUG
		return					::xray::memory::g_mt_allocator.malloc_impl( buffer_size );
#	endif // #ifdef DEBUG
#endif // #if !XRAY_PLATFORM_PS3
}

inline pvoid __cdecl operator new[]		( size_t )
{
	UNREACHABLE_CODE(return 0);
#ifndef __SNC__
	__assume(0);
#endif // #ifndef __SNC__
}


////////////////////////////////////////////////////////////////////////////
// delete operators
////////////////////////////////////////////////////////////////////////////
inline void __cdecl operator delete		( pvoid pointer )
{
#ifdef XRAY_MAYA_BUILDING
	R_ASSERT		( ::xray::memory::g_crt_allocator );
#	ifdef DEBUG
		::xray::memory::g_crt_allocator->free_impl	( pointer, __FUNCTION__, __FILE__, __LINE__ );
#	else // #ifdef DEBUG
		::xray::memory::g_crt_allocator->free_impl	( pointer );
#	endif // #ifdef DEBUG
#else // #ifdef XRAY_MAYA_BUILDING
#	if !XRAY_PLATFORM_PS3
		XRAY_UNREFERENCED_PARAMETER( pointer );
		UNREACHABLE_CODE( );
		__assume	( 0 );
#	else // #if !XRAY_PLATFORM_PS3
#		ifdef DEBUG
			::xray::memory::g_mt_allocator.free_impl( pointer, __FUNCTION__, __FILE__, __LINE__ );
#		else // #ifdef DEBUG
			::xray::memory::g_mt_allocator.free_impl( pointer );
#		endif // #ifdef DEBUG
#	endif // #if !XRAY_PLATFORM_PS3
#endif // #ifdef XRAY_MAYA_BUILDING
}

inline void __cdecl operator delete[]	( pvoid ) throw ( )
{
	UNREACHABLE_CODE();
#ifndef __SNC__
	__assume(0);
#endif // #ifndef __SNC__
}

#endif // #ifndef XRAY_MEMORY_OVERRIDE_OPERATORS_H_INCLUDED
////////////////////////////////////////////////////////////////////////////
//	Created		: 23.04.2010
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_RTP_API_H_INCLUDED
#define XRAY_RTP_API_H_INCLUDED

#ifndef XRAY_RTP_API
#	ifdef XRAY_STATIC_LIBRARIES
#		define XRAY_RTP_API
#	else // #ifdef XRAY_STATIC_LIBRARIES
#		ifdef XRAY_RTP_BUILDING
#			define XRAY_RTP_API						XRAY_DLL_EXPORT
#		else // #ifdef XRAY_RTP_BUILDING
#			ifndef XRAY_ENGINE_BUILDING
#				define XRAY_RTP_API					XRAY_DLL_IMPORT
#			else // #ifndef XRAY_ENGINE_BUILDING
#				define XRAY_RTP_API					XRAY_DLL_EXPORT
#			endif // #ifndef XRAY_ENGINE_BUILDING
#		endif // #ifdef XRAY_RTP_BUILDING
#	endif // #ifdef XRAY_STATIC_LIBRARIES
#endif // #ifndef XRAY_RTP_API

namespace xray {
namespace rtp {

struct engine;
struct world;

//typedef memory::doug_lea_allocator_type		allocator_type;
typedef memory::pthreads_allocator_type			allocator_type;

XRAY_RTP_API	world*	create_world	( engine& engine );
XRAY_RTP_API	void	destroy_world	( world*& world );
XRAY_RTP_API	void	memory_allocator( memory::doug_lea_allocator_type& allocator );

} // namespace rtp
} // namespace xray

#endif // #ifndef XRAY_RTP_API_H_INCLUDED
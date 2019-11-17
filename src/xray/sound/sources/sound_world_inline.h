////////////////////////////////////////////////////////////////////////////
//	Created		: 29.04.2010
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef SOUND_WORLD_INLINE_H_INCLUDED
#define SOUND_WORLD_INLINE_H_INCLUDED

namespace xray {
namespace sound {

template <typename T> 
inline void sound_world::delete_orders( T& queue )
{
	queue.set_pop_thread_id();
	while ( !queue.empty( ) ) 
	{
		sound_command*			to_delete;
		queue.pop_front			( to_delete );
		XRAY_DELETE_IMPL		( to_delete->m_creator, to_delete );
	}
}

} // namespace sound
} // namespace xray

#endif // #ifndef SOUND_WORLD_INLINE_H_INCLUDED
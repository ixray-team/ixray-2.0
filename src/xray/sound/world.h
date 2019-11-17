////////////////////////////////////////////////////////////////////////////
//	Created 	: 22.12.2009
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_SOUND_WORLD_H_INCLUDED
#define XRAY_SOUND_WORLD_H_INCLUDED

namespace xray {
namespace sound {

struct handler;

struct XRAY_NOVTABLE world {
	virtual	void	tick					( u32 logic_frame_id )		= 0;

	virtual void	set_listener_properties	( float4x4 const& inv_view_matrix, ::xray::memory::base_allocator* a )	= 0;

	virtual	void	clear_resources			( ) 						= 0;
	virtual	void	clear_logic_resources	( ) 						= 0;
	virtual	void	clear_editor_resources	( ) 						= 0;

protected:
	XRAY_DECLARE_PURE_VIRTUAL_DESTRUCTOR( world )
}; // class world

} // namespace sound
} // namespace xray

#endif // #ifndef XRAY_SOUND_WORLD_H_INCLUDED
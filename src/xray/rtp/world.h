////////////////////////////////////////////////////////////////////////////
//	Created		: 23.04.2010
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_RTP_WORLD_H_INCLUDED
#define XRAY_RTP_WORLD_H_INCLUDED

namespace xray {

namespace render {
namespace debug {
	struct renderer;
} // namespace debug
} // namespace render

namespace input {
	struct world;
} // namespace input

namespace rtp {

struct XRAY_NOVTABLE world {
	virtual			~world			( ) { }
	virtual	void	render			( xray::render::debug::renderer& renderer ) const =0;
	virtual	void	tick			( ) = 0;
	virtual	void	dbg_move_control( float4x4 &view_inverted, input::world&  input ) = 0;

}; // class world

} // namespace rtp
} // namespace xray

#endif // #ifndef XRAY_RTP_WORLD_H_INCLUDED
////////////////////////////////////////////////////////////////////////////
//	Created		: 23.04.2010
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_RTP_ENGINE_H_INCLUDED
#define XRAY_RTP_ENGINE_H_INCLUDED

namespace xray {

namespace animation {
	struct world;
} // namespace animation 

namespace render {
struct world;
} // namespace render

namespace rtp {

struct XRAY_NOVTABLE engine {
	virtual						~engine					( ) { }
	virtual	animation::world&	get_animation_world		( ) = 0;
	virtual	render::world&		get_renderer_world		( ) = 0;
	virtual	pcstr				get_resource_path		( ) const=0;
}; // class engine

} // namespace rtp
} // namespace xray

#endif // #ifndef XRAY_RTP_ENGINE_H_INCLUDED
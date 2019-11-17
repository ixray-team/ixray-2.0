////////////////////////////////////////////////////////////////////////////
//	Created 	: 17.12.2008
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_ANIMATION_ENGINE_H_INCLUDED
#define XRAY_ANIMATION_ENGINE_H_INCLUDED

namespace xray {

namespace render {
	struct world;
} // namespace render

namespace animation {

struct XRAY_NOVTABLE engine {
	virtual	render::world&	get_renderer_world	( ) = 0;


protected:
	XRAY_DECLARE_PURE_VIRTUAL_DESTRUCTOR( engine )
}; // class engine

} // namespace animation
} // namespace xray

#endif // #ifndef XRAY_ANIMATION_ENGINE_H_INCLUDED
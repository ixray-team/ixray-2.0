////////////////////////////////////////////////////////////////////////////
//	Created		: 29.01.2010
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef MAYA_ENGINE_H_INCLUDED
#define MAYA_ENGINE_H_INCLUDED
#include <xray/core/simple_engine.h>
#include <xray/maya_animation/engine.h>
#include <xray/animation/engine.h>

namespace xray {

namespace animation {
	struct world;
}

namespace maya_animation {
	struct world;
}

namespace maya{

class maya_engine :
	public xray::core::engine,
	public xray::animation::engine,
	public xray::maya_animation::engine
{
public:
	maya_engine			( );

	virtual void			exit						( int error_code )	{ m_exit_code = error_code; xray::debug::terminate(""); }
	virtual int				get_exit_code				( ) const			{ return m_exit_code; }
	virtual void			set_exit_code				( int error_code )	{ m_exit_code = error_code; }
	virtual	pcstr			get_resource_path			( ) const			;
	virtual	pcstr			get_mounts_path				( ) const			{ return 0; }
	virtual	pcstr			get_underscore_G_path		( ) const			;
	virtual	render::world&	get_renderer_world			( )					;
	
	xray::animation::world * animation_world;
	xray::maya_animation::world * maya_animation_world;
public:
	void			assign_resource_path		( );
private:
	
	int				m_exit_code;

	string_path		m_resource_path;
	string_path		m_underscore_G_path;
} ; // class simple_engine
#endif // #ifndef MAYA_ENGINE_H_INCLUDED

extern	maya_engine	&g_maya_engine;
}
}
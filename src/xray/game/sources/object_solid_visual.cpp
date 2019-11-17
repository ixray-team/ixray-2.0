////////////////////////////////////////////////////////////////////////////
//	Created		: 13.03.2009
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "object_solid_visual.h"
#include "game_world.h"
#include "game.h"
#include "game_resource_manager.h"
#include <xray/render/base/game_renderer.h>

namespace stalker2{

static u32 visual_ids = 1000000;

object_solid_visual::object_solid_visual( game& game )
:super(game)
{
	m_visual_id	= ++visual_ids;
}

object_solid_visual::~object_solid_visual( )
{
	decrease_quality( );
}

void object_solid_visual::load( xray::configs::lua_config_value const& t )
{
	super::load			(t);
	m_visual_name		= (pcstr)t["lib_name"];
}

void object_solid_visual::decrease_quality( )
{
	if( m_visual )
		m_game.render_world().game().remove_visual	( m_visual_id );
	
	m_visual = NULL;
}

void object_solid_visual::make_quality_optimal( )
{
	if(m_visual)
		return;

	resources::query_resource(
		m_visual_name.c_str(),
		resources::solid_visual_class,
		boost::bind(&object_solid_visual::on_visual_ready, this, _1),
		g_allocator
		);
}

void object_solid_visual::on_visual_ready( resources::queries_result& data )
{
	R_ASSERT	( data.is_successful() );
	if( !m_visual )
	{
		m_visual	= static_cast_resource_ptr<render::visual_ptr>(data[0].get_unmanaged_resource());
		R_ASSERT	( m_visual );

		m_game.render_world().game().add_visual( m_visual_id, m_visual, m_transform );
	}
}

} //namespace stalker2
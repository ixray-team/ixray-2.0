////////////////////////////////////////////////////////////////////////////
//	Created		: 13.03.2009
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "object_solid_visual.h"
#include "tool_solid_visual.h"
#include "property_holder.h"
#include "editor_control_base.h"
#include "collision_object_dynamic.h"
#include <xray/editor/base/managed_delegate.h>

#include <xray/collision/api.h>
#include <xray/render/base/world.h>
#include <xray/editor/world/engine.h>

namespace xray {
namespace editor {

object_solid_visual::object_solid_visual(tool_solid_visual^ t):
super				(t), 
m_tool_solid_visual	(t)
{
	m_visual		= NEW(render::visual_ptr)();
	image_key		= "mesh";
}

object_solid_visual::~object_solid_visual()
{
	get_editor_renderer().remove_visual	(m_id);
	DELETE			(m_visual);
}

void object_solid_visual::load(xray::configs::lua_config_value const& t)
{
	super::load			(t);
	set_scale			(t["scale"]);
}

void object_solid_visual::set_visual_name_(System::String^ name)
{
	m_visual_name		= name;
	set_lib_name		(name);  // solid_visual doesnt have library

	unmanaged_string s(name);
	query_result_delegate* q = NEW(query_result_delegate)(gcnew query_result_delegate::Delegate(this, &object_solid_visual::on_resource_loaded));

	resources::request	resources[]	= {s.c_str(), xray::resources::solid_visual_class};
	resources::query_resources	(
		resources,
		array_size(resources),
		boost::bind(&query_result_delegate::callback, q, _1),
		g_allocator
		);
}

void object_solid_visual::set_visible(bool bvisible)
{
	ASSERT(bvisible!=get_visible());

	if((*m_visual).c_ptr())
	{
		if(bvisible)
			get_editor_renderer().add_visual	( m_id, *m_visual, *m_transform, m_selected);
		else
			get_editor_renderer().remove_visual	(m_id);
	}

	super::set_visible(bvisible);
}

void object_solid_visual::set_transform			(float4x4 const& transform)
{
	super::set_transform			(transform);

	if(get_visible() && (*m_visual).c_ptr())
		get_editor_renderer().update_visual	( m_id, *m_transform, m_selected);
}

void object_solid_visual::set_selected(bool selected)
{
	super::set_selected(selected);

	if(get_visible() && (*m_visual).c_ptr())
		get_editor_renderer().update_visual	( m_id, *m_transform, m_selected);
}

void object_solid_visual::on_resource_loaded( xray::resources::queries_result& data )
{
	R_ASSERT(!data.is_failed());

	for(u32 i=0; i<data.size(); ++i)
	{
		resources::query_result_for_user & r	= data[i];
		resources::unmanaged_resource_ptr gr = r.get_unmanaged_resource();
		on_visual_ready( gr );
	}
}

void object_solid_visual::on_visual_ready(xray::resources::unmanaged_resource_ptr data)
{
	*m_visual = static_cast_checked<render::visual*>(data.c_ptr());

	if(get_visible())
		get_editor_renderer().add_visual	( m_id, *m_visual, *m_transform, m_selected);

	initialize_collision	();
}

void object_solid_visual::initialize_collision	()
{
	m_aabbox->identity();
	ASSERT( (*m_visual) );
	ASSERT( (*m_visual)->get_collision_geometry() != NULL );

	ASSERT						( !m_collision.initialized() );

	m_collision.create_from_geometry(this, (*m_visual)->get_collision_geometry(), xray::editor::collision_type_dynamic);
	m_collision.insert			( );
	m_collision.set_matrix		( m_transform );
}

} // namespace editor
} // namespace xray

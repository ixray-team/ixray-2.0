////////////////////////////////////////////////////////////////////////////
//	Created		: 12.03.2009
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "object_base.h"
#include "tool_base.h"
#include "project_items.h"
#include "level_editor.h"
#include "editor_world.h"
#include "project.h"
#include "property_holder.h"
#include "command_set_object_transform.h"
#include "collision_object_dynamic.h"
#include "lua_config_value_editor.h"

#pragma managed(push,off)
#include <xray/collision/api.h>
#include <xray/collision/geometry.h>
#include <xray/collision/space_partitioning_tree.h>
#include <xray/editor/world/engine.h>
#include <xray/geometry_utils.h>
#pragma managed(pop)


namespace xray{
namespace editor{

object_collision::object_collision(xray::editor::level_editor^ le)
:m_collision_obj(NULL),
m_level_editor	(le),
m_b_active		(false)
{
}

void object_collision::destroy()
{
	if( initialized() )
	{
		remove	();
		DELETE (m_collision_obj);
		m_collision_obj = NULL;

		if( m_user_geometry )
			collision::destroy( m_user_geometry );
	}
}
void object_collision::remove()
{
	if(m_b_active)
		m_level_editor->get_collision_tree()->remove( m_collision_obj );

	m_b_active = false;
}

void object_collision::insert()
{
	math::aabb					collision_aabb;
	m_collision_obj->get_object_geometry()->get_aabb	(collision_aabb);
	float3 center				= collision_aabb.center	();
	float3 extents				= collision_aabb.extents();
	m_level_editor->get_collision_tree()->insert			(m_collision_obj, center, extents);
	m_b_active					= true;
	on_collision_initialized	(this);
}

void object_collision::set_matrix(float4x4 const* m )
{
	ASSERT						(initialized());
	if(!m_b_active)	
		return;

	m_collision_obj->set_matrix	(*m);
	m_level_editor->get_collision_tree()->move( m_collision_obj, m_collision_obj->get_center(), m_collision_obj->get_extents() );
}

aabb object_collision::get_aabb()
{
	return m_collision_obj->get_aabb();
}

bool object_collision::initialized()
{
	return( m_collision_obj != NULL );
}

void object_collision::create_cube(object_base^ o, float3 extents)
{
	vectora< float3 >	vertices			(*g_allocator);
	vectora< u16 >		render_indices		(*g_allocator);
	
	geometry_utils::create_cube( vertices, render_indices, float4x4().identity(), extents );

	vectora< u32 > collision_indices		(*g_allocator);
	vectora< u16 >::const_iterator it		= render_indices.begin();
	vectora< u16 >::const_iterator it_e		= render_indices.end();
	for(; it!=it_e; ++it)
		collision_indices.push_back(*it);

	m_user_geometry			= &*collision::create_triangle_mesh_geometry( g_allocator, (float3*)&vertices[0], vertices.size(), &collision_indices[0], collision_indices.size() );
	create_from_geometry	(o, m_user_geometry, xray::editor::collision_type_dynamic);
}

void object_collision::create_mesh(	object_base^ o, 
									float3 const * vertices, 
									u32 vertex_count, 
									u32 const* indices, 
									u32 index_count, 
									u32 const* triangle_data, 
									u32 triangle_data_count, 
									xray::collision::object_type t)
{
	m_user_geometry		= &*collision::create_triangle_mesh_geometry(	g_allocator, 
																		vertices, 
																		vertex_count, 
																		indices, 
																		index_count, 
																		triangle_data, 
																		triangle_data_count );
	create_from_geometry(o, m_user_geometry, t);
}

void object_collision::create_from_geometry(object_base^ o, xray::collision::geometry const * geom, xray::collision::object_type t)
{
	m_collision_obj		= NEW (editor::collision_object_dynamic)( geom, o, t );
}

xray::collision::geometry const* object_collision::get_geometry( )
{
	return m_collision_obj->get_object_geometry();
}


object_base^ object_base::object_by_id( u32 id )
{
	object_base^		object;

	if( s_id_to_object.TryGetValue(id,object) ) 
		return			object;

	ASSERT( false, "There is no object with the specified id!");
	return				nullptr;
}

object_base::object_base(tool_base^ t):
m_owner_tool			(t),
m_id					(0),
m_lib_name				(""),
m_selected				(false),
m_owner_project_item	(nullptr),
m_collision				(t->get_level_editor()),
m_visible				(false)
{
	image_key			= "";
	m_transform			= NEW(float4x4)(float4x4().identity());
	m_aabbox			= NEW(math::aabb)( create_center_radius( float3(0,0,0), float3(2,2,2) ) ),
	m_aabbox->identity	( );

	m_name				= gcnew System::String("");
	m_property_holder	= NEW(::property_holder)("props", NULL, NULL);
}

object_base::~object_base()
{
	m_collision.destroy			();
	property_holder* tmp		= m_property_holder;
	DELETE						(tmp);
	s_id_to_object.Remove		(m_id);
	DELETE						(m_transform);
	DELETE						(m_aabbox);
}

void object_base::load(xray::configs::lua_config_value const& t)
{
	set_name					(gcnew System::String(t["name"]), true);
	set_lib_name				(gcnew System::String(t["lib_name"]));
  	set_rotation				(t["rotation"]);
	m_transform->c.xyz()		= t["position"];
}

void object_base::save(xray::configs::lua_config_value cfg)
{
	set_c_string				(cfg["__merge_table_id__"], get_name());
	cfg["type"]					= "object";
	set_c_string				(cfg["name"], get_name());
	set_c_string				(cfg["lib_name"], get_lib_name());
	cfg["position"]				= get_position();
	cfg["rotation"]				= get_rotation();
	cfg["scale"]				= get_scale();
	cfg["aabb_min"]				= aabb().min;
	cfg["aabb_max"]				= aabb().max;
	set_c_string				(cfg["tool_name"], owner()->name());

//	on_save						(cfg);
}

void object_base::set_name(System::String^ name, bool refresh_ui)
{
	m_name = name;
	if(refresh_ui && m_owner_project_item)
		m_owner_project_item->refresh_ui();
}

void object_base::set_name_revertible(System::String^ name)
{
	owner()->get_level_editor()->get_command_engine()->run( gcnew command_set_object_name(owner()->get_level_editor(), this->id(), name ) );
}

void object_base::set_lib_name(System::String^ name)
{
	m_lib_name = name;
}

void object_base::set_selected(bool selected)
{
	m_selected							= selected;
	if(m_owner_project_item) // maybe temporary object (new library item )
		m_owner_project_item->set_selected(selected);
}

void object_base::assign_id		( u32 id )
{
	ASSERT( m_id == 0, "Object ID can be assigned only once." );

	if( id == 0 )
		id = generate_id();

	ASSERT( false == s_id_to_object.ContainsKey(id), "An object with the specified id allready exists." );

	m_id				= id;
	s_id_to_object[id]	= this;
}

u32 object_base::generate_id()
{
 	return ++m_last_generated_id;
}

aabb object_base::aabb()
{
	if( m_collision.initialized() )
		*m_aabbox = m_collision.get_aabb();

	return *m_aabbox;
}

////////////////////////////////////////////////////////////////////
// do not write the following methods in .h file, because of the bug
// in the incremental inker during link time code generation phase
////////////////////////////////////////////////////////////////////

System::String^ object_base::get_name				( )
{
	return m_name;
}

xray::editor::property_holder* object_base::get_property_holder	( )
{
	return m_property_holder;
}

void object_base::on_property_changed(System::String^ prop_name)
{
	m_owner_tool->get_level_editor()->get_project()->get_link_storage()->notify_on_changed(this, prop_name);
}

bool object_base::is_slave_attribute(System::String^ prop_name)
{
	return m_owner_tool->get_level_editor()->get_project()->get_link_storage()->has_links(nullptr, nullptr, this, prop_name);
}


xray::render::editor::renderer& object_base::get_editor_renderer()
{
	return m_owner_tool->get_level_editor()->get_editor_renderer();
}

} // namespace editor
} // namespace xray

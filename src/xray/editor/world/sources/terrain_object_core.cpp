////////////////////////////////////////////////////////////////////////////
//	Created		: 04.03.2010
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "terrain_object.h"
#include "tool_terrain.h"
#include "level_editor.h"
#include "editor_world.h"
#include "project_items.h"
#include "project.h"

#include <xray/render/base/editor_renderer.h>

namespace xray {
namespace editor {

terrain_core::terrain_core(tool_terrain^ tool_terrain)
:super(tool_terrain),
m_terrain_tool(tool_terrain)
{
	node_size			= terrain_node_size::t64x64;
	image_key			= "terrain";
}

void terrain_core::set_visible(bool bvisible)
{
	super::set_visible			(bvisible);
	for each( key_value_pair e in m_nodes )
		e.Value->set_visible	(bvisible);
}

void terrain_core::load_defaults()
{
	super::load_defaults	();
	create_node				(terrain_node_key(0, 0), 1);
	create_node				(terrain_node_key(0, 1), 1);

	create_node				(terrain_node_key(1, 0), 1);
	create_node				(terrain_node_key(1, 1), 1);
}

void terrain_core::load(xray::configs::lua_config_value const& t)
{
	super::load					(t);

	xray::configs::lua_config_value t_objects = t["nodes"];

	configs::lua_config::const_iterator it	 = t_objects.begin();
	configs::lua_config::const_iterator it_e = t_objects.end();

	for(; it!=it_e; ++it)
	{
		xray::configs::lua_config_value current = *it;
		object_base^ o				= m_terrain_tool->load_object(current);
		terrain_node^ node			= safe_cast<terrain_node^>(o);

		terrain_node_key key		(current["px"], current["pz"]);
		ASSERT						(!m_nodes.ContainsKey(key));
		o->assign_id				(0);
		add_node_internal			(key, node);
	}

	get_property_holder()->clear	();
	initialize_property_holder		(this);

	for each( key_value_pair e in m_nodes )
		get_property_holder()->add_property_holder("nodes", "", "", e.Value->get_property_holder() );
}

void terrain_core::save(xray::configs::lua_config_value t)
{
	super::save						(t);

	if(m_owner_tool->get_level_editor()->get_project()->m_tmp_is_save_to_file)
	{
		// prepare "terrain" folder
		fs::path_string resource_path	= m_owner_tool->get_level_editor()->get_editor_world().engine().get_resource_path();
		System::String^ path			= gcnew System::String(resource_path.c_str());
		path							+= "/projects/";
		path							+= m_owner_tool->get_level_editor()->get_project()->project_name();
		path							+= "/terrain";
		path							= System::IO::Path::GetFullPath(path);
		

		if(!System::IO::Directory::Exists(path))
			System::IO::Directory::CreateDirectory(path);
	}
	xray::configs::lua_config_value t_objects = t["nodes"];
	
	int i=0;

	for each( key_value_pair e in m_nodes )
	{
		xray::configs::lua_config_value current = t_objects[i];
		current["px"]				= e.Key.x;
		current["pz"]				= e.Key.z;
		e.Value->save				(current);
		++i;
	}
}

void terrain_core::render()
{
	super::render		();
}

void terrain_core::set_transform(float4x4 const& )
{
	super::set_transform(float4x4().identity());
}

void terrain_core::create_node(terrain_node_key key, u32 quad_size)
{
	XRAY_UNREFERENCED_PARAMETERS(quad_size);

	ASSERT				(!m_nodes.ContainsKey(key));

	terrain_node^ node	= safe_cast<terrain_node^>(m_terrain_tool->create_object("terrain_64x64"));
	
	node->assign_id		(0);
	float sz			= (float)node_size;
	float4x4			m = create_translation(float3(key.x*sz, 0.0f, key.z*sz));
	node->set_transform_internal(m);

	add_node_internal(key, node);
}

void terrain_core::clear_node(terrain_node_key key)
{
	ASSERT				(m_nodes.ContainsKey(key));
	remove_node_internal(key);
}

void terrain_core::destroy_all()
{
	System::Collections::Generic::List<terrain_node_key> lst = m_nodes.Keys;

	for each(terrain_node_key key in lst)
		remove_node_internal(key);

	ASSERT(m_nodes.Count==0);
}


void terrain_core::update()
{
	if(m_load_queries.Count)
	{
		terrain_node_key key	= m_load_queries[0];

		// tmp
		if(m_nodes[key]->m_b_load_complete)
		{
			load_node_impl			(key);
			m_load_queries.RemoveAt	(0);
		}
		return;
	}

	if(m_active_updating_nodes.Count)
		return;

	while(m_update_queries.Count)
	{
		update_vertices_impl	(0);
		m_update_queries.RemoveAt(0);
	}
}

void terrain_core::command_update_cb(u32 arg)
{
	ASSERT	(m_active_updating_nodes.Contains(arg));

	m_active_updating_nodes.Remove	(arg);
}

void terrain_core::add_node_internal(terrain_node_key key, terrain_node^ node)
{
	m_nodes[key]					= node;
	m_load_queries.Add				(key);
	node->m_tmp_key					= key;

	if(get_visible()!=node->get_visible())
		node->set_visible			(get_visible());
}

void terrain_core::remove_node_internal(terrain_node_key key)
{
	terrain_node^ node				= m_nodes[key];
	m_terrain_tool->get_level_editor()->get_editor_renderer().terrain_remove_cell(node->get_visual());
	m_terrain_tool->destroy_object	(node);
	m_nodes.Remove					(key);
}

float terrain_core::get_height(float3 const& position_global)
{
	terrain_node_key picked		= pick_node(position_global);

	if(!m_nodes.ContainsKey(picked))
		return 0.0f;

	terrain_node^ terrain		= m_nodes[picked];

	float4x4					inv_transform;
	inv_transform.try_invert	(terrain->get_transform());
	float3 position_local		= inv_transform.transform_position(position_global);

	float res					= terrain->get_height_local	(position_local);
	ASSERT(math::valid(res));
	return res;
}

void terrain_core::select_vertices(float3 const& point, float const radius, modifier_shape t, key_vert_id_dict^ dest_list)
{
	TerrainNodesList		keys_list;
	terrain_node_key picked = pick_node(point);

	int r					= math::ceil(radius/(float)node_size)+1;
	for(int ix = picked.x-r; ix<=picked.x+r; ++ix)
	{
		for(int iz = picked.z-r; iz<=picked.z+r; ++iz)
		{
			terrain_node_key	key(ix, iz);
			if(m_nodes.ContainsKey(key))
			{
				terrain_node^ curr_terrain		= m_nodes[key];
				vert_id_list^ list				= gcnew vert_id_list;
				curr_terrain->get_vertices_g	(point, radius, t, list);
				if(list->Count)
					dest_list->Add				(key, list);
				else
					delete list;
			}
		}
	}
}

void terrain_core::add_texture(System::String^ texture_name)
{
	if(!m_textures.Contains(texture_name))
		m_textures.Add	(texture_name);
}

void terrain_core::change_texture(System::String^ old_texture_name, System::String^ new_texture_name)
{
	ASSERT(m_textures.Contains(old_texture_name));
	ASSERT(!m_textures.Contains(new_texture_name));

	int idx = m_textures.IndexOf(old_texture_name);
	m_textures[idx]			= new_texture_name;

	render::texture_string	old_ts(unmanaged_string(old_texture_name).c_str());
	render::texture_string	new_ts(unmanaged_string(new_texture_name).c_str());

	get_editor_renderer().terrain_exchange_texture(old_ts, new_ts);

	for each( key_value_pair e in m_nodes )
	{
		int idx = e.Value->m_used_textures.IndexOf(old_texture_name);
		if(idx!=-1)
			e.Value->m_used_textures[idx] = new_texture_name;
	}
}

void terrain_core::sync_visual_vertices(terrain_node^ terrain, vert_id_list^ id_list)
{
	if(id_list->Count==0)
		return;

	update_verts_query^ q = gcnew update_verts_query;
	q->id					= terrain->id();
	q->list					= gcnew vert_id_list;
	q->list->AddRange		(id_list);

	m_update_queries.Add(q);
}

void terrain_core::load_node_impl(terrain_node_key key)
{
	ASSERT					(m_load_queries.Contains(key));
	
	// load node visual here
	terrain_node^ terrain	= m_nodes[key];
	
	pbyte pbuffer			= NULL;
	u32 pbuffer_size		= 0;
	xray::memory::writer	writer(g_allocator);
	writer.external_data	= true;
	
	//fixed_string<16>		cell_name;
	//cell_name.assignf		( "%d_%d", key.x, key.z );
	//writer.w_stringZ		( cell_name.c_str() );
	writer.w_u32			( terrain->m_dimension+1 );
	
	// physical size
	writer.w_float			( terrain->size );

	// used textures
	writer.w_u32			(terrain->m_used_textures.Count);
	for each (System::String^ t in terrain->m_used_textures)
	{
		writer.w_stringZ	( unmanaged_string(t).c_str() );
		add_texture			( t );
	}
	// used textures

	// transform
	float4x4	tr		= terrain->get_transform();
	writer.w_fvector3	(tr.i.xyz());
	writer.w_fvector3	(tr.j.xyz());
	writer.w_fvector3	(tr.k.xyz());
	writer.w_fvector3	(tr.c.xyz());
	// transform

	{
		vectora<render::terrain_data>	vertices_buffer(g_allocator);
		terrain->export_vertices		(0, (u16)terrain->m_vertices.Count, vertices_buffer);
		writer.w						(&vertices_buffer[0], vertices_buffer.size()*sizeof(vertices_buffer[0]));
	}

	pbuffer							= writer.pointer();
	pbuffer_size					= writer.size();

	query_result_delegate1* q		= NEW(query_result_delegate1)(gcnew query_result_delegate1::Delegate(this, &terrain_core::on_terrain_cell_ready));

	fs::path_string		query_path;
	query_path.assignf	("terrain/%d_%d.terr", terrain->m_tmp_key.x, terrain->m_tmp_key.z);

	resources::query_create_resource(
						query_path.c_str(),
						xray::memory::buffer(pbuffer, pbuffer_size),
						xray::resources::terrain_cell_class,
						boost::bind(&query_result_delegate1::callback, q, _1, terrain->id() ),
						g_allocator
						);
}
void terrain_core::on_terrain_cell_ready( xray::resources::queries_result& data, u32 terrain_id )
{
	ASSERT								(data.is_successful());

	const_buffer creation_data_from_user	= data[0].creation_data_from_user();
	pbyte pdata							= (pbyte)creation_data_from_user.c_ptr();
	DELETE									(pdata);

	render::visual_ptr	terrain_visual	= static_cast_resource_ptr<render::visual_ptr>(data[0].get_unmanaged_resource());
	ASSERT						(terrain_visual);

	terrain_node^ terrain		= safe_cast<terrain_node^>(object_base::object_by_id(terrain_id));
	ASSERT						(terrain);
	terrain->initialize_collision();
	terrain->set_visual			(terrain_visual);
}

void terrain_core::update_vertices_impl(u32 idx)
{
	update_verts_query^ q			= m_update_queries[idx];

	terrain_node^ terrain			= safe_cast<terrain_node^>(object_base::object_by_id(q->id));
	vert_id_list^ id_list			= q->list;

	m_active_updating_nodes.Add		(q->id);

	id_list->Sort					();
	
	System::Collections::Generic::Dictionary<u16, u16>	fragments_list;
	
	u16 fragment_id					= id_list[0];
	fragments_list[fragment_id]		= 1;

	for(u16 id=1; id<id_list->Count; ++id)
	{
		u16 vid = id_list[id];
		if(fragments_list[fragment_id]+fragment_id == vid)
		{
			++fragments_list[fragment_id];
		}else
		{
			fragment_id						= vid;
			fragments_list[fragment_id]		= 1;
		}
	}

	vectora<render::buffer_fragment_NEW>	fragments(g_allocator);
	fragments.reserve						(fragments_list.Count);

	for each (u16 id in fragments_list.Keys)
	{
		fragments.push_back					(render::buffer_fragment_NEW(g_allocator));
		render::buffer_fragment_NEW& buff	= fragments.back();
		buff.start							= id;
		buff.size							= fragments_list[id];

		terrain->export_vertices			(id, fragments_list[id], buff.buffer);
	}

	command_finished_delegate* del = NEW (command_finished_delegate)( 
						gcnew command_finished_delegate::Delegate(this, &terrain_core::command_update_cb)
										);

	render::command_finished_callback cb	(del, &command_finished_delegate::callback);

	get_editor_renderer().terrain_update_cell_buffer(
		terrain->get_visual(), fragments, terrain->get_transform(),
		cb,
		q->id);
}

} // namespace editor
} // namespace xray

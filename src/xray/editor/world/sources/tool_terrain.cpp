////////////////////////////////////////////////////////////////////////////
//	Created		: 13.01.2010
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "tool_terrain.h"
#include "terrain_object.h"
#include "terrain_tool_tab.h"
#include "terrain_modifier_bump.h"
#include "terrain_modifier_raise_lower.h"
#include "terrain_modifier_flatten.h"
#include "terrain_modifier_smooth.h"
#include "terrain_modifier_painter.h"
#include "level_editor.h"
#include "project.h"
#include "property_container.h"
#include "library_object_tab.h"
#include "action_engine.h"
#include "actions.h"
#include <xray/editor/base/managed_delegate.h>
#include <xray/editor/base/images/images16x16.h>

namespace xray {
namespace editor {

tree_node^ make_hierrarchy(tree_node_collection^ root, System::String^ name);

tool_terrain::tool_terrain(level_editor^ le)
:super(le, "terrain"),
m_terrain_core(nullptr)
{
	load_library						();

	m_terrain_modifier_bump				= gcnew terrain_modifier_bump(le, this);
	m_terrain_modifier_raise_lower		= gcnew terrain_modifier_raise_lower(le, this);
	m_terrain_modifier_flatten			= gcnew terrain_modifier_flatten(le, this);
	m_terrain_modifier_smooth			= gcnew terrain_modifier_smooth(le, this);
	m_terrain_modifier_detail_painter	= gcnew terrain_modifier_detail_painter(le, this);
	m_terrain_modifier_diffuse_painter	= gcnew terrain_modifier_diffuse_painter(le, this);

	get_level_editor()->register_editor_control			(m_terrain_modifier_bump);
	get_level_editor()->get_action_engine()->register_action( gcnew action_select_editor_control	( "select terrain bump modifier",		get_level_editor(), "terrain_modifier_bump" ) );

	get_level_editor()->register_editor_control			(m_terrain_modifier_raise_lower);
	get_level_editor()->get_action_engine()->register_action( gcnew action_select_editor_control	( "select terrain raise-lower modifier", get_level_editor(), "terrain_modifier_raise_lower" ) );

	get_level_editor()->register_editor_control			(m_terrain_modifier_flatten);
	get_level_editor()->get_action_engine()->register_action( gcnew action_select_editor_control	( "select terrain flatten modifier",	get_level_editor(), "terrain_modifier_flatten" ) );

	get_level_editor()->register_editor_control			(m_terrain_modifier_smooth);
	get_level_editor()->get_action_engine()->register_action( gcnew action_select_editor_control	( "select terrain smooth modifier",		get_level_editor(), "terrain_modifier_smooth" ) );

	get_level_editor()->register_editor_control			(m_terrain_modifier_detail_painter);
	get_level_editor()->get_action_engine()->register_action( gcnew action_select_editor_control	( "select terrain detail painter modifier",		get_level_editor(), "terrain_modifier_detail_painter" ) );

	get_level_editor()->register_editor_control			(m_terrain_modifier_diffuse_painter);
	get_level_editor()->get_action_engine()->register_action( gcnew action_select_editor_control	( "select terrain diffuse painter modifier",		get_level_editor(), "terrain_modifier_diffuse_painter" ) );

//	gui_binder^ binder = get_level_editor()->get_gui_binder();

	ide()->register_image("select terrain bump modifier",		xray::editor_base::terrain_bump );
	ide()->register_image("select terrain raise-lower modifier",xray::editor_base::terrain_raise_lower );
	ide()->register_image("select terrain flatten modifier",	xray::editor_base::terrain_flatten );
	ide()->register_image("select terrain smooth modifier",		xray::editor_base::terrain_smooth );
	ide()->register_image("select terrain detail painter modifier",	xray::editor_base::terrain_painter );
	ide()->register_image("select terrain diffuse painter modifier",	xray::editor_base::terrain_diffuse_painter );

	//ide()->add_button_item("TerrainToolStrip", 2);
	//ide()->add_button_item(binder, "select terrain bump modifier", "TerrainToolStrip", 0);
	//ide()->add_button_item(binder, "select terrain raise-lower modifier", "TerrainToolStrip", 1);
	//ide()->add_button_item(binder, "select terrain flatten modifier", "TerrainToolStrip", 2);
	//ide()->add_button_item(binder, "select terrain smooth modifier", "TerrainToolStrip", 3);
	//ide()->add_button_item(binder, "select terrain detail painter modifier", "TerrainToolStrip", 4);
	//ide()->add_button_item(binder, "select terrain diffuse painter modifier", "TerrainToolStrip", 5);

	m_tool_tab 							= gcnew terrain_tool_tab(this);
	m_terrain_modifier_bump->initialize();
	m_terrain_modifier_raise_lower->initialize();
	m_terrain_modifier_flatten->initialize();
	m_terrain_modifier_smooth->initialize();
	m_terrain_modifier_detail_painter->initialize();
	m_terrain_modifier_diffuse_painter->initialize();

}

tool_terrain::~tool_terrain()							
{
	get_level_editor()->unregister_editor_control			(m_terrain_modifier_bump);
	delete m_terrain_modifier_bump;
	get_level_editor()->unregister_editor_control			(m_terrain_modifier_raise_lower);
	delete m_terrain_modifier_raise_lower;
	get_level_editor()->unregister_editor_control			(m_terrain_modifier_flatten);
	delete m_terrain_modifier_flatten;
	get_level_editor()->unregister_editor_control			(m_terrain_modifier_smooth);
	delete m_terrain_modifier_smooth;
	get_level_editor()->unregister_editor_control			(m_terrain_modifier_detail_painter);
	delete m_terrain_modifier_detail_painter;
	get_level_editor()->unregister_editor_control			(m_terrain_modifier_diffuse_painter);
	delete m_terrain_modifier_diffuse_painter;
}

tool_tab^ tool_terrain::ui()
{
	return m_tool_tab;
}

object_base^ tool_terrain::create_raw_object(System::String^ id)
{
	object_base^ result			= nullptr;
	if(id=="base:terrain_node")
	{
		ASSERT			(m_terrain_core!=nullptr);
		result			= gcnew terrain_node(this, m_terrain_core);
	}else
	if(id=="base:terrain_core")
	{
		ASSERT			(m_terrain_core==nullptr);
		m_terrain_core	= gcnew terrain_core(this);
		result			= m_terrain_core;
	}

	ASSERT(result, "raw object not found", unmanaged_string(id).c_str());
	m_object_list->Add			(result);
	initialize_property_holder	(result); //tmp
	return result;
}

object_base^ tool_terrain::load_object(xray::configs::lua_config_value const& t)
{
	object_base^ result			= nullptr;
	System::String^ library_name = gcnew System::String(t["lib_name"]);
	if(library_name->StartsWith("base:"))
	{
		result					= create_raw_object(library_name);
	}else
	{
		lib_item^ li			= get_library_item(library_name, true);
		
		System::String^ ll		= gcnew System::String( (*(li->m_config))["lib_name"]);
		result					= create_raw_object(ll);
	}

	result->load				(t);
	result->set_lib_name		(library_name);

	return						result;
}

object_base^ tool_terrain::create_object(System::String^ name)
{
	lib_item^ li				= get_library_item(name, true);
	System::String^ ln			= gcnew System::String((*(li->m_config))["lib_name"]);
	object_base^ result			= create_raw_object(ln);
	result->load				(*li->m_config);
	result->set_lib_name		(name);

	return						result;
}

void tool_terrain::destroy_object(object_base^ o)
{
	R_ASSERT					(m_object_list->Contains(o));

	if(m_terrain_core==o)
	{
		m_terrain_core->destroy_all();
		m_terrain_core			= nullptr;
	}
	m_object_list->Remove		(o);
	delete o;
}


void tool_terrain::load_library()
{
	query_result_delegate* rq = NEW(query_result_delegate)(gcnew query_result_delegate::Delegate(this, &tool_terrain::on_library_files_loaded));
	xray::resources::query_resource(
		"resources/library/terrain/terrain.library",
		xray::resources::config_lua_class,
		boost::bind(&query_result_delegate::callback, rq, _1),
		g_allocator
		);
}

void tool_terrain::load_settings(RegistryKey^ key)
{
	super::load_settings	(key);
	RegistryKey^ self_key	= get_sub_key(key, name());

	m_terrain_modifier_bump->load_settings(self_key);
	m_terrain_modifier_raise_lower->load_settings(self_key);
	m_terrain_modifier_flatten->load_settings(self_key);
	m_terrain_modifier_smooth->load_settings(self_key);
	m_terrain_modifier_detail_painter->load_settings(self_key);
	m_terrain_modifier_diffuse_painter->load_settings(self_key);

	self_key->Close			();
}

void tool_terrain::save_settings(RegistryKey^ key)
{
	super::save_settings	(key);
	RegistryKey^ self_key	= get_sub_key(key, name());

	m_terrain_modifier_bump->save_settings(self_key);
	m_terrain_modifier_raise_lower->save_settings(self_key);
	m_terrain_modifier_flatten->save_settings(self_key);
	m_terrain_modifier_smooth->save_settings(self_key);
	m_terrain_modifier_detail_painter->save_settings(self_key);
	m_terrain_modifier_diffuse_painter->save_settings(self_key);

	self_key->Close			();
}

terrain_core^ tool_terrain::get_terrain_core()
{
	return m_terrain_core;
}

void tool_terrain::tick()
{
	if(m_terrain_core)
		m_terrain_core->update();
}

} //namespace editor
} // namespace xray

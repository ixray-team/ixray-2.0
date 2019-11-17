////////////////////////////////////////////////////////////////////////////
//	Created		: 11.03.2009
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef PROJECT_H_INCLUDED
#define PROJECT_H_INCLUDED

#include "project_defines.h"

namespace xray {

namespace configs{class lua_config_value;}

namespace resources{class queries_result;}

namespace editor {

class transform_control;
ref class level_editor;
class object;
ref class project_tab;
ref class link_storage;
ref class property_connection_editor;
typedef project_tab^ project_ui;

ref class link_temporary_data
{
public:
	System::String^ src_name;
	System::String^ src_member;
	object_base^	dst_obj;
	System::String^ dst_member;
};

ref class paste_temporary_data
{
public:
	System::String^ orig_name;
	object_base^	object;
};

public delegate void selection_changed();

public ref class project
{

public:
					project					(level_editor^ le);
					~project				();
	void			load					(System::String^ path);	
	void			load					(xray::configs::lua_config_value const& cfg, bool load_ids);
	void			paste					(xray::configs::lua_config_value const& cfg, id_list^% ids);
	void			save					(System::String^ path);
	void			save_internal			( );

	void			save_to_config			( xray::configs::lua_config_value cfg );
	void			save_to_config			( xray::configs::lua_config_value cfg, object_base^ object, bool save_id );
	void			save_to_config			( xray::configs::lua_config_value cfg, object_list^ objects, bool save_ids );
	
	void			create_new				( );
	void			clear					( );
	bool			empty					( );
	
	bool			name_assigned			( );
	System::String^ project_name			( ) {return m_project_path;}
	fs::path_string resource_name			( );

	tool_base^		get_tool				(System::String^ tool_name);
	project_ui		ui						();
	level_editor^	get_level_editor		() {return m_level_editor;}

	void			render					();

	void			add_new_group			();
	project_item^	add_new_item			(tool_base^ tool, System::String^ lib_name, float4x4 const& transform, u32 id, System::String^ path);
	project_item^	add_new_item			(xray::configs::lua_config_value cfg);
	
	void			remove					(xray::configs::lua_config_value const& cfg);
	void			remove					(id_list^ ids);
	void			remove_group			(project_group^ pg);
	void			remove_item				(project_item^ pi, bool b_destroy_object);
	void			remove_item				(u32 id, bool b_destroy_object);
	
	

	void			select_object			(object_base^ obj, enum_selection_method method);
	void			select_objects			(object_list^ objs, enum_selection_method method);
	void			select_id				(u32 id, enum_selection_method method);
	void			select_ids				(id_list^ objs,	enum_selection_method method);

	object_list^	get_all_objects			();

	object_list^	selection_list			();
	object_base^	aim_object				();
	
	void			on_object_load			(object_base^ object, xray::configs::lua_config_value const& t);
	void			on_object_save			(object_base^ object, xray::configs::lua_config_value const& t);

//	void			on_project_links_loaded	(resources::queries_result& data);
	
	void			show_connection_editor	();
	void			show_connection_editor	(object_base^ src, object_base^ dst);
	void			show_object_inspector	(); // selected objects
	void			show_object_inspector	(object_list^ objects);

protected:
	void			fill_tree_view			(tree_node_collection^ nodes);
	project_group^	find_group				(System::String^ name, bool bcreate);
//	void			save_links				(System::String^ project_name);
//	void			load_links				(System::String^ project_name);
	void			on_selection_changed	();
	object_base^	get_item_by_path		(System::String^ path);

	void			on_config_ready			( xray::resources::queries_result& data );
public:
	project_group^							m_last_selected_group;
	project_item^							m_last_selected_item;
	bool									m_tmp_is_save_to_file;
	link_storage^	get_link_storage		();
	void			subscribe_on_selection_changed(selection_changed^ d);
private:
	configs::lua_config_ptr*				m_config;
	project_group^							m_root_group;
	System::String^							m_project_path;
	level_editor^							m_level_editor;
	project_tab^							m_project_tab;
	object_list^							m_selection_list;

	link_storage^							m_link_storage;
	event selection_changed^				m_selection_changed;
	System::Collections::Generic::List<link_temporary_data^>^	m_link_tmp_list;
	System::Collections::Generic::List<paste_temporary_data^>^	m_paste_tmp_list;
}; // class project

} // namespace editor
} // namespace xray

#endif // #ifndef PROJECT_H_INCLUDED
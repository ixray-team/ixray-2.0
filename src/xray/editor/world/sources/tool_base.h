////////////////////////////////////////////////////////////////////////////
//	Created		: 12.03.2009
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef TOOL_BASE_H_INCLUDED
#define TOOL_BASE_H_INCLUDED

#include "project_defines.h"
#include "property_container.h"
#include "tool_tab.h"
#include "editor_ide.h"

class property_holder;
ref class property_restrictor;

namespace xray {

namespace editor {

//class editor_world;
ref class level_editor;
ref class tool_base;
class editor_world;

typedef System::Windows::Forms::ComboBox::ObjectCollection^		names_list;

enum enum_list_mode{lm_raw_objects,};

public ref struct lib_item
{
									lib_item		(configs::lua_config_value lv, level_editor^ le);
									~lib_item		();
									!lib_item		();
	property_restrictor^			m_property_restrictor;
	configs::lua_config_value*		m_config;
	System::String^					m_lib_item_name;
	level_editor^					m_level_editor;
	System::String^					get_raw_lib_name();
};

public ref class tool_base abstract
{
public:
						tool_base				(level_editor^ le, System::String^ name);
	virtual				~tool_base				();
	virtual void		tick					(){}

	virtual object_base^ load_object			(xray::configs::lua_config_value const& t)	abstract; // load
	virtual object_base^ create_object			(System::String^ name)				abstract; // create new instance
	virtual object_base^ create_raw_object		(System::String^ id)				abstract;

			void		on_library_files_loaded	(xray::resources::queries_result& data);
	virtual	void		save_library			(){};

	virtual void		load_settings			(RegistryKey^)	{};
	virtual void		save_settings			(RegistryKey^)	{};

	virtual void		destroy_object			(object_base^ o)					abstract;
	virtual tool_tab^	ui						()									abstract;

			lib_item^	get_library_item		(System::String^ library_name, bool b_assert);
			lib_item^	new_lib_item			(System::String^ library_name);

	virtual void		get_objects_list		(names_list list, enum_list_mode mode) { XRAY_UNREFERENCED_PARAMETERS( list, mode ); UNREACHABLE_CODE();};

	virtual void		on_make_new_library_object()							{UNREACHABLE_CODE();}
	virtual void		on_edit_library_object	(System::String^ name)			{ XRAY_UNREFERENCED_PARAMETER( name ); UNREACHABLE_CODE();}
			void		on_remove_library_object(System::String^ library_name);
			void		on_commit_library_item	(lib_item^ lib_item);

			void		select_library_name		(System::String^ name);
	System::String^		get_selected_library_name()								{return m_selected_library;}

	System::String^		name					()								{return m_tool_name;}

	editor_ide^			ide						();
	level_editor^		get_level_editor		()								{return m_level_editor;}

			bool		ready					()								{return m_b_ready;}
protected:
	typedef System::Collections::Generic::List<lib_item^>	library_items;

	void				check_correct_lib_name	(System::String^% name, int index);
	virtual void		on_library_loaded		(){};

	System::String^				m_tool_name;
	level_editor^				m_level_editor;
	System::String^				m_selected_library;
	object_list^				m_object_list;
	library_items^				m_tool_library;
	configs::lua_config_ptr*	m_library_cfg;
	bool						m_b_ready;
}; // class tool_base




} // namespace editor
} // namespace xray

#endif // #ifndef TOOL_BASE_H_INCLUDED
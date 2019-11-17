////////////////////////////////////////////////////////////////////////////
//	Created		: 02.04.2010
//	Author		: Evgeniy Obertyukh
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "texture_document.h"
#include "resource_editor_base.h"
#include "resource_selector.h"

using namespace Flobbster::Windows::Forms;

using xray::editor::resource_editor;
using xray::resources::unmanaged_resource_ptr;

using System::Windows::RoutedEventArgs;
using xray::editor::wpf_controls::property_grid_editors::external_editor_event_handler;
using xray::editor::wpf_controls::property_grid_value_changed_event_args;
typedef xray::editor::wpf_controls::property_grid_property grid_property;

namespace xray {
namespace editor {

void				texture_document::fill_property_holder					(texture_options* options, property_holder* holder)
{
	texture_options_wrapper^			wrapper = gcnew texture_options_wrapper(options);

	holder->clear			();

 	holder->add_string		("type", "Type", "", (String^)wrapper->m_type_list[(int)options->type], gcnew string_getter_type(wrapper, &texture_options_wrapper::get_type), gcnew string_setter_type(wrapper, &texture_options_wrapper::set_type), 
 										property_holder::property_read_only,
 										property_holder::do_not_notify_parent_on_change,
 										property_holder::no_password_char,
 										property_holder::do_not_refresh_grid_on_change); 

	holder->add_integer		("width", "Dimensions", "", options->width, gcnew integer_getter_type(wrapper, &texture_options_wrapper::get_width), gcnew integer_setter_type(wrapper, &texture_options_wrapper::set_width), 
										property_holder::property_read_only,
										property_holder::do_not_notify_parent_on_change,
										property_holder::no_password_char,
										property_holder::do_not_refresh_grid_on_change);

	holder->add_integer		("height", "Dimensions", "", options->height, gcnew integer_getter_type(wrapper, &texture_options_wrapper::get_height), gcnew integer_setter_type(wrapper, &texture_options_wrapper::set_height), 
										property_holder::property_read_only,
										property_holder::do_not_notify_parent_on_change,
										property_holder::no_password_char,
										property_holder::do_not_refresh_grid_on_change);

	if(options->type == texture_options::bump)
	{
		holder->add_string		("bump mode", "Normal", "", wrapper->m_bump_mode_list[(int)options->bump_mode]->ToString() , gcnew string_getter_type(wrapper, &texture_options_wrapper::get_bump_mode), gcnew string_setter_type(wrapper, &texture_options_wrapper::set_bump_mode), 
											wrapper->m_bump_mode_list,
											property_holder::value_editor_combo_box,
											property_holder::can_enter_text,
											(is_use_by_selector)?property_holder::property_read_only:property_holder::property_read_write,
											property_holder::notify_parent_on_change ,
											property_holder::no_password_char,
											property_holder::refresh_grid_on_change); 

	}
	else if(options->type == texture_options::terrain)
	{
		holder->add_string		("terrain bump", "Terrain", "", gcnew String(options->bump_name.c_str()), gcnew string_getter_type(wrapper, &texture_options_wrapper::get_bump_name), gcnew string_setter_type(wrapper, &texture_options_wrapper::set_bump_name), 
											gcnew external_editor_event_handler(this, &texture_document::external_editor_launch),
											"terrain_bump",
											(is_use_by_selector)?property_holder::property_read_only:property_holder::property_read_write,
											property_holder::do_not_notify_parent_on_change,
											property_holder::no_password_char,
											property_holder::do_not_refresh_grid_on_change);
		holder->add_string		("terrain color", "Terrain", "", gcnew String(options->color_name.c_str()), gcnew string_getter_type(wrapper, &texture_options_wrapper::get_color_name), gcnew string_setter_type(wrapper, &texture_options_wrapper::set_color_name), 
											gcnew external_editor_event_handler(this, &texture_document::external_editor_launch),
											"terrain_color",
											(is_use_by_selector)?property_holder::property_read_only:property_holder::property_read_write,
											property_holder::do_not_notify_parent_on_change,
											property_holder::no_password_char,
											property_holder::do_not_refresh_grid_on_change);
		holder->add_string		("tile", "Terrain", "", options->tile.ToString() , gcnew string_getter_type(wrapper, &texture_options_wrapper::get_tile), gcnew string_setter_type(wrapper, &texture_options_wrapper::set_tile), 
											wrapper->m_tile_list,
											property_holder::value_editor_combo_box,
											property_holder::can_enter_text,
											(is_use_by_selector)?property_holder::property_read_only:property_holder::property_read_write,
											property_holder::notify_parent_on_change ,
											property_holder::no_password_char,
											property_holder::refresh_grid_on_change); 
	}
	else if(options->type == texture_options::type_2d)
	{
		holder->add_string		("alpha blend", "2D Part", "", wrapper->m_alpha_mode_list[(int)options->alpha_blend_mode]->ToString() , gcnew string_getter_type(wrapper, &texture_options_wrapper::get_alpha_blend), gcnew string_setter_type(wrapper, &texture_options_wrapper::set_alpha_blend), 
											wrapper->m_alpha_mode_list,
											property_holder::value_editor_combo_box,
											property_holder::can_enter_text,
											(is_use_by_selector)?property_holder::property_read_only:property_holder::property_read_write,
											property_holder::notify_parent_on_change ,
											property_holder::no_password_char,
											property_holder::refresh_grid_on_change); 
		holder->add_string		("pack mode", "2D Part", "", wrapper->m_pack_mode_list[(int)options->pack_mode]->ToString() , gcnew string_getter_type(wrapper, &texture_options_wrapper::get_pack_mode), gcnew string_setter_type(wrapper, &texture_options_wrapper::set_pack_mode), 
											wrapper->m_pack_mode_list,
											property_holder::value_editor_combo_box,
											property_holder::can_enter_text,
											(is_use_by_selector)?property_holder::property_read_only:property_holder::property_read_write,
											property_holder::notify_parent_on_change ,
											property_holder::no_password_char,
											property_holder::refresh_grid_on_change); 
		holder->add_string		("bump map", "2D Part", "", gcnew String(options->bump_name.c_str()), gcnew string_getter_type(wrapper, &texture_options_wrapper::get_bump_name), gcnew string_setter_type(wrapper, &texture_options_wrapper::set_bump_name), 
											gcnew external_editor_event_handler(this, &texture_document::external_editor_launch),
											"_bump",
											(is_use_by_selector)?property_holder::property_read_only:property_holder::property_read_write,
											property_holder::do_not_notify_parent_on_change,
											property_holder::no_password_char,
											property_holder::do_not_refresh_grid_on_change);
		holder->add_string		("normal map", "2D Part", "", gcnew String(options->normal_name.c_str()), gcnew string_getter_type(wrapper, &texture_options_wrapper::get_normal_name), gcnew string_setter_type(wrapper, &texture_options_wrapper::set_normal_name), 
											gcnew external_editor_event_handler(this, &texture_document::external_editor_launch),
											"_nmap",
											(is_use_by_selector)?property_holder::property_read_only:property_holder::property_read_write,
											property_holder::do_not_notify_parent_on_change,
											property_holder::no_password_char,
											property_holder::do_not_refresh_grid_on_change);
	}
}

void				texture_document::external_editor_launch					(Object^ sender, RoutedEventArgs ^ )
{
	grid_property^ prop = safe_cast<grid_property^>(sender);
	
	resource_selector_ptr selector(
		static_cast<resource_editor_base^>(this->m_editor)->world,
		resource_selector::resource_type::Texture,
		safe_cast<Flobbster::Windows::Forms::PropertyBag::PropertySpecDescriptor^>(prop->descriptors[0])->Item->Tag
	);

	selector->name = "texture_editor";

	selector->selecting_entity = resource_selector::entity::resource_name;
	selector->selected_name = safe_cast<String^>(prop->value);

	if(selector->ShowDialog() == System::Windows::Forms::DialogResult::OK)
	{
		prop->value = selector->selected_name;
	}
}

void				texture_document::options_loaded							(xray::resources::queries_result& data)
{
	unmanaged_resource_ptr	options_ptr;

	options_ptr					= data[0].get_unmanaged_resource();
	texture_options* m_options	= static_cast_checked<texture_options*>(options_ptr.c_ptr());

	String^ requested_path		= gcnew String(data[0].get_requested_path());

	if(m_options->m_is_default_values)
	{
		if(is_use_by_selector)
		{
			this->m_loading_options->Remove(requested_path);
			return;
		}
		else
			m_options->save();
	}

	if(m_is_wait_for_options)
	{
		m_is_wait_for_options = false;
		if((m_options->width != m_image_width || m_options->height != m_image_height))
		{
			m_options->width	= m_image_width;
			m_options->height	= m_image_height;
		}
	}

	property_holder* m_property_holder	= NEW(::property_holder)("texture_struct_properties", NULL, NULL);
	
	fill_property_holder(m_options, m_property_holder);

	safe_cast<resource_editor_base^>(m_editor)->set_options_and_holder	(options_ptr, m_property_holder);
	m_loading_options->Remove(requested_path);
	process_loaded_options();

	if(m_options_request_id != data.unique_id())
		return;

	//m_editor->show_properties			(m_property_holder->container());
	m_editor->properties_panel->property_grid_control->Enabled = true;

	is_saved							= true;
	resource_loaded						(this, EventArgs::Empty);
}

#pragma region | Accessors |

String^				texture_document::texture_options_wrapper::get_type()
{
	return m_type_list[(int)m_options->type]->ToString();
}
void				texture_document::texture_options_wrapper::set_type(String^ value)
{
	m_options->type = static_cast<texture_options::ETType>(m_type_list->IndexOf(value));
}

Int32				texture_document::texture_options_wrapper::get_width()
{
	return m_options->width;
}
void				texture_document::texture_options_wrapper::set_width(Int32 value)
{
	m_options->width = value;
}

Int32				texture_document::texture_options_wrapper::get_height()
{
	return m_options->height;
}
void				texture_document::texture_options_wrapper::set_height(Int32 value)
{
	m_options->height = value;
}

String^				texture_document::texture_options_wrapper::get_alpha_blend()
{
	return m_alpha_mode_list[static_cast_checked<int>(m_options->alpha_blend_mode)]->ToString();
}
void				texture_document::texture_options_wrapper::set_alpha_blend(String^ value)
{
	m_options->alpha_blend_mode = static_cast<texture_options::ETAlphaBlendMode>(m_alpha_mode_list->IndexOf(value));
}

String^				texture_document::texture_options_wrapper::get_pack_mode()
{
	return m_pack_mode_list[static_cast_checked<int>(m_options->pack_mode)]->ToString();
}
void				texture_document::texture_options_wrapper::set_pack_mode(String^ value)
{
	m_options->pack_mode = static_cast<texture_options::ETPackMode>(m_pack_mode_list->IndexOf(value));
}

String^				texture_document::texture_options_wrapper::get_bump_name()
{
	return  gcnew String(m_options->bump_name.c_str());
}
void				texture_document::texture_options_wrapper::set_bump_name(String^ value)
{
	if(value == nullptr)
		return;
	m_options->bump_name = unmanaged_string(value).c_str();
}

String^				texture_document::texture_options_wrapper::get_color_name()
{
	return  gcnew String(m_options->color_name.c_str());
}
void				texture_document::texture_options_wrapper::set_color_name(String^ value)
{
	if(value == nullptr)
		return;
	m_options->color_name = unmanaged_string(value).c_str();
}

String^				texture_document::texture_options_wrapper::get_bump_mode()
{
	return m_bump_mode_list[static_cast_checked<int>(m_options->bump_mode)]->ToString();
}
void				texture_document::texture_options_wrapper::set_bump_mode(String^ value)
{
	m_options->bump_mode = static_cast<texture_options::ETBumpMode>(m_bump_mode_list->IndexOf(value));
}

String^				texture_document::texture_options_wrapper::get_normal_name()
{
	return  gcnew String(m_options->normal_name.c_str());
}
void				texture_document::texture_options_wrapper::set_normal_name(String^ value)
{
	if(value == nullptr)
		return;
	m_options->normal_name = unmanaged_string(value).c_str();
}

String^				texture_document::texture_options_wrapper::get_tile()
{
	return m_options->tile.ToString();
}
void				texture_document::texture_options_wrapper::set_tile(String^ value)
{
	m_options->tile = Convert::ToInt32(value, 10);
}

#pragma endregion

}//namespace editor
}//namespace xray
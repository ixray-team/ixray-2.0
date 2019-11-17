////////////////////////////////////////////////////////////////////////////
//	Created		: 11.02.2010
//	Author		: Evgeniy Obertyukh
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "texture_document.h"
#include "texture_options_cooker.h"
#include <xray/editor/base/managed_delegate.h>
#include "resource_editor_base.h"
#include "texture_tga_to_argb_cook.h"
#include "editor_resource_manager.h"
#include "property_container.h"

using namespace	System::IO;
using namespace	System::Collections::Generic;
using namespace	System::Collections::ObjectModel;

using xray::editor::wpf_controls::property_grid_value_changed_event_args;
using xray::editor::wpf_controls::property_grid_value_changed_event_handler;
using xray::editor::wpf_controls::property_grid_host;
using xray::editor::wpf_controls::property_grid;
namespace xray {
namespace editor {

texture_options_cooker*		texture_options_cook_obj = NULL;
texture_tga_to_argb_cook*	texture_tga_to_argb_cook_obj = NULL;

void register_texture_cook	()
{
	//register cooks
	ASSERT							(texture_options_cook_obj==NULL);
	ASSERT							(texture_tga_to_argb_cook_obj==NULL);
	texture_options_cook_obj		= NEW(texture_options_cooker)();
	texture_tga_to_argb_cook_obj	= NEW(texture_tga_to_argb_cook)();
	register_cook					(texture_options_cook_obj);
	register_cook					(texture_tga_to_argb_cook_obj);
}

void unregister_texture_cook()
{
	unregister_cook					(resources::texture_options_class);
	unregister_cook					(resources::texture_tga_to_argb_class);

	DELETE							(texture_options_cook_obj);
	DELETE							(texture_tga_to_argb_cook_obj);
}

static Boolean check_to_rus_chars(char* str)
{
	for(; *str != '\0'; ++str)
	{
		int code = ((int)(*str));
		if(code > 128 || code < 0)
			return true;
	}
	return false;
}

void						texture_document::in_constructor		()
{
	if(!s_chessboard_alive)
	{
		texture_document::create_chessboard();
		safe_cast<resource_editor_base^>(m_editor)->resource_editor_closing += gcnew EventHandler(&texture_document::distroy_chessboard);
		s_chessboard_alive = true;
	}

	m_selected_options				= gcnew Collections::Generic::List<String^>();
	m_options_to_show				= gcnew List<options_wrapper^>();
	m_loading_options				= gcnew Collections::Generic::Dictionary<String^, Int32>();
	m_images						= gcnew Collections::Generic::Dictionary<Int32, Bitmap^>();
}

typedef xray::resources::queries_result queries_result;
delegate void binded_query_callback(queries_result&, Object^);
ref class query_callback_binder
{
public:
	query_callback_binder(binded_query_callback^ callback, Object^ bind_data):
		m_callback(callback), m_bind_data(bind_data){}

	void callback(queries_result& data)
	{
		m_callback(data, m_bind_data);
	}
private:
	binded_query_callback^ m_callback;
	Object^ m_bind_data;
};

void						texture_document::load					()
{
//   	editor_resource_manager::query_resource(
//   		unmanaged_string("resources/textures_new/converted_local/"+Name+".dds").c_str(),
//   		xray::resources::texture_wrapper_class,
//   		gcnew managed_query_callback(this, &texture_document::texture_loaded),
//   		g_allocator
//   	);  	

	m_selected_options->Clear();
	m_options_to_show->Clear();

	controls::tree_view^ tree			= safe_cast<resource_editor_base^>(m_editor)->view_panel->tree_view;
	if(tree->selected_nodes->Count > 0)
	{
		ReadOnlyCollection<TreeNode^>^ selected_tree_items = tree->selected_nodes;
		for each(TreeNode^ node in selected_tree_items)
			m_selected_options->Add(node->FullPath);
	}
	else
	{
		m_selected_options->Add(tree->SelectedNode->FullPath);
	}

	Boolean is_multiselect = m_selected_options->Count > 1;

	String^ last_requested_path;

	last_requested_path = m_selected_options[0];
	process_loaded_options();

 	if(m_selected_options->Count > 0)
	{
		last_requested_path = m_selected_options[0];
 		request_unexisting_options();
	}
 	
	if(is_multiselect)
	{
		m_picture_box->Image = nullptr;
	}
	else
	{
		//receive bitmap for single selected image
		unmanaged_string u_str = unmanaged_string("resources/textures_new/sources/"+Name+".tga");

		if(!check_to_rus_chars(u_str.c_str()))
		{
			m_is_tga_synk_called = true;

			binded_query_callback^ callback = gcnew binded_query_callback(this, &texture_document::tga_loaded);

			m_tga_request_id = editor_resource_manager::query_resource(
				u_str.c_str(),
				xray::resources::raw_data_class,
				gcnew managed_query_callback(gcnew query_callback_binder(callback, last_requested_path), &query_callback_binder::callback),
				g_allocator
			);

			m_is_tga_synk_called = false;
		}
	}
}

void						texture_document::request_unexisting_options		()
{
	m_editor->properties_panel->property_grid_control->Enabled = false;
	for each(String^ options_path in m_selected_options)
	{
		unmanaged_string str(options_path);
		if(check_to_rus_chars(str.c_str()))
		{
			Windows::Forms::MessageBox::Show("Can't load resource with russian characters", "Warning", MessageBoxButtons::OK, MessageBoxIcon::Warning);
			m_selected_options->Remove(options_path);
			break;
		}

		if(!m_loading_options->ContainsKey(options_path))
		{
			m_options_request_id = editor_resource_manager::query_resource(
				str.c_str(),
				xray::resources::texture_options_class,
				gcnew managed_query_callback(this, &texture_document::options_loaded),
				g_allocator
			);
			m_loading_options->Add(options_path, m_options_request_id);
		}
	}
}

void						texture_document::process_loaded_options			()
{
	for(int i = m_selected_options->Count-1; i >= 0; --i)
	{
		if(safe_cast<resource_editor_base^>(m_editor)->is_resource_loaded(m_selected_options[i]))
		{
			m_options_to_show->Add(safe_cast<resource_editor_base^>(m_editor)->get_loaded_resource(m_selected_options[i]));
			m_selected_options->RemoveAt(i);			
		}
	}

	if(m_selected_options->Count == 0)
	{
		view_selected_options();
	}
}

void						texture_document::view_selected_options				()
{
	safe_cast<resource_editor_base^>(m_editor)->selected_resources->Clear();
	safe_cast<resource_editor_base^>(m_editor)->selected_resources->AddRange(m_options_to_show);
	if(m_options_to_show->Count > 1)
	{
		array<Object^>^ objects = gcnew array<Object^>(m_options_to_show->Count);
		
		for( int i = 0; i < m_options_to_show->Count; ++i )
		{
			objects[i] = static_cast<::property_holder*>(m_options_to_show[i]->m_property_holder)->container();
		}

		m_editor->show_properties(objects);
	}
	else
		m_editor->show_properties(static_cast<::property_holder*>(m_options_to_show[0]->m_property_holder)->container());

	m_editor->properties_panel->property_grid_control->Enabled = true;
	is_saved					= true;
}

void						texture_document::save								()
{
	for each(options_wrapper^ options in m_options_to_show)
	{
		static_cast<texture_options*>(options->m_resource->c_ptr())->save		();
	}
	is_saved			= true;
}

void						texture_document::texture_loaded					(xray::resources::queries_result& data)
{
	if( data.is_successful())
	{
 		int t;
		t=78;
	}
}

controls::document_base^	texture_document::create_document					()
{
	return gcnew texture_document(m_editor);
}

void						texture_document::texture_document_FormClosed		(Object^  , FormClosedEventArgs^  )
{	
}

}//namespace editor
}//namespace xray
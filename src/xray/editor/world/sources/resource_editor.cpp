////////////////////////////////////////////////////////////////////////////
//	Created		: 09.02.2010
//	Author		: Evgeniy Obertyukh
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "window_ide.h"
#include "resource_editor.h"
#include "resource_editor_base.h"
#include "texture_options.h"
#include "resource_selector.h"
#include "resource_document_factory.h"
#include "resource_editor_commit.h"
#include "resource_editor_cancel.h"

using namespace System::Windows;

namespace xray {
namespace editor {

RegistryKey^		resource_editor::editor_registry_key()
{
	RegistryKey^ base_registry_key	= m_world->ide()->base_registry_key();
	RegistryKey^ windows			= base_registry_key->CreateSubKey("windows");

	return	windows->CreateSubKey(m_multidocument->Name);
}
void				resource_editor::in_constructor					(resource_document_factory^ document_factory, controls::tree_view_source^ resources_source)
{
	m_is_closed				= false;
	m_multidocument			= gcnew resource_editor_base(*m_world, document_factory, resources_source, false);
	m_multidocument->Dock	= DockStyle::Fill;
	Controls->Add			(m_multidocument);
	Controls->SetChildIndex	(m_multidocument, 0);
}

void				resource_editor::resource_editor_Load			(Object^ , EventArgs^ )
{
	m_multidocument->load_panels(this);

	RegistryKey^ editor_key = editor_registry_key();

	String^ selected_path			= safe_cast<String^>(editor_key->GetValue("tree_view_selected_path"));
	if(selected_path != nullptr && selected_path != String::Empty)
		m_multidocument->view_panel->tree_view->track_active_node(selected_path);
}

String^				resource_editor::name::get						()
{
	return m_multidocument->Name;
}
void				resource_editor::name::set						(String^ value)
{
	m_multidocument->Name = value;
}
String^				resource_editor::view_panel_caption::get		()				
{
	return m_multidocument->view_panel_caption;
}
void				resource_editor::view_panel_caption::set		(String^ value)
{
	m_multidocument->view_panel_caption = value;
}

String^				resource_editor::properties_panel_caption::get	()
{
	return m_multidocument->properties_panel_caption;
}
void				resource_editor::properties_panel_caption::set	(String^ value)
{
	m_multidocument->properties_panel_caption = value;
}

void				resource_editor::resource_editor_FormClosing	(System::Object^  , System::Windows::Forms::FormClosingEventArgs^ e)
{
	m_is_closed = true;
	if(m_multidocument->changed_resources->Count > 0)
	{
		resource_editor_cancel^ cancel_dlg = gcnew resource_editor_cancel(m_multidocument->changed_resources);
		if(cancel_dlg->ShowDialog() == Forms::DialogResult::Cancel)
		{
			e->Cancel = true;
			return;
		}
	}
	RegistryKey^ editor_key = editor_registry_key();
	if( m_multidocument->view_panel->tree_view->selected_nodes->Count > 0 )
		editor_key->SetValue(
			"tree_view_selected_path",
			m_multidocument->view_panel->tree_view->selected_nodes[0]->FullPath
			);
	m_multidocument->close_all_documents();
	m_multidocument->save_panels		(this);
	m_multidocument->free_resources		();
}

void				resource_editor::manual_close					()
{
	//Save panel settings
	if(!m_is_closed)
	{
		m_multidocument->close_all_documents();
		m_multidocument->save_panels	(this);
		this->Close();
	}
}

void				resource_editor::m_ok_button_Click				(Object^ , EventArgs^ )
{
	if(m_multidocument->changed_resources->Count > 0)
	{
		resource_editor_commit^	 commint_box = gcnew resource_editor_commit(m_multidocument->changed_resources);
		Forms::DialogResult result = commint_box->ShowDialog();

		if(result == Forms::DialogResult::OK)
		{
			for each(options_wrapper^ wrapper in m_multidocument->changed_resources->Values)
			{
				static_cast<resource_options*>(wrapper->m_resource->c_ptr())->save();
			}
			m_multidocument->changed_resources->Clear();
			this->Close();
		}
	}
	else
	{
		m_multidocument->changed_resources->Clear();
		this->Close();
	}
}

void				resource_editor::m_cancel_button_Click			(Object^ , EventArgs^ )
{
	if(m_multidocument->changed_resources->Count > 0)
	{
		if(System::Windows::Forms::MessageBox::Show("There is change in resources, if you continue, the changes will be lost. Do you want to exit anyway?", "Resource Editor", Forms::MessageBoxButtons::YesNo, Forms::MessageBoxIcon::Question) == Forms::DialogResult::Yes)
		{
			m_multidocument->changed_resources->Clear();
			this->Close();
		}
	}
	else
	{
		m_multidocument->changed_resources->Clear();
		this->Close();
	}
}

}//namespace editor
}//namespace xray
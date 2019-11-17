//-------------------------------------------------------------------------------------------
//	Created		: 20.01.2010
//	Author		: Sergey Pryshchepa
//	Copyright (C) GSC Game World - 2009
//-------------------------------------------------------------------------------------------
#include "pch.h"
#include "document_editor_base.h"
#include "file_view_panel_base.h"
#include "item_properties_panel_base.h"
#include "toolbar_panel_base.h"
#include "document_base.h"
#include "dock_panel_serializer.h"

using namespace WeifenLuo::WinFormsUI::Docking;
using namespace System::Windows::Forms;
using namespace System::Collections::Generic;
using namespace System::IO;
using namespace System::Reflection;

namespace xray {
namespace editor {
namespace controls{

void			document_editor_base::in_constructor	()
{
	is_reuse_single_document		= true;

	if(!m_create_menu)
		this->Controls->Remove(menuStrip1);

	m_main_dock_panel->ContentAdded	+= gcnew EventHandler<DockContentEventArgs^>(this, &document_editor_base::main_panel_content_added);
}

void			document_editor_base::is_fixed_panels::set(Boolean value)
{
	m_main_dock_panel->AllowEndUserDocking			= !value;
	m_main_dock_panel->AllowEndUserNestedDocking	= !value;

	for each(DockContent^ content in m_main_dock_panel->Contents)
	{
		content->CloseButtonVisible = !value;
	}
}

void			document_editor_base::new_document		(System::Object^ , System::EventArgs^ )
{
	document_base^ doc = create_new_document();
	doc->Text = "Untitled Document";
	doc->Name = "Untitled Document";
	document_created(this, gcnew document_event_args(doc));
	doc->Show(m_main_dock_panel);
	m_opened_documents->Add(doc);
	m_active_document = doc;
}

void			document_editor_base::exit_editor		(System::Object^ , System::EventArgs^ )
{
	this->Hide();
	editor_exited(this, EventArgs::Empty);
}


bool			document_editor_base::is_opened			(System::String^ doc_name)
{
	for each(document_base^ d in m_opened_documents)
	{
		if(d->Name==doc_name)
			return true;
	}
	return false;
}

void			document_editor_base::load_document			(System::String^ full_path)
{
	load_document(full_path, true);
}

void			document_editor_base::load_document			(System::String^ full_path, Boolean need_show)
{
	Boolean	reusing_single_doc	= is_single_document && is_reuse_single_document && m_active_document!= nullptr;

	if(m_loaded)
		return;
	m_loaded = true;
	
	for each(document_base^ d in m_opened_documents)
	{
		if(d->Name==full_path)
		{
			d->Activate();
			document_loaded		(this, EventArgs::Empty);
			m_loaded = false;
			return;
		}
	}

	if(is_single_document && !reusing_single_doc)
	{
		if(m_opened_documents->Count > 0)
			m_opened_documents[0]->Close();

		if(m_opened_documents->Count > 0)
		{
			m_loaded = false;
			return;
		}
	}
	
	document_base^ doc	= (reusing_single_doc)?m_active_document:create_new_document();
	doc->Text			= doc->get_document_name(full_path);
	doc->Name			= doc->Text;
	if(!reusing_single_doc)
	{
		document_created(this, gcnew document_event_args(doc));
		m_opened_documents->Add	(doc);
		m_active_document	= doc;
	}
	doc->load			();	
	if(need_show && !reusing_single_doc)
		doc->Show			(m_main_dock_panel);
	document_loaded		(this, EventArgs::Empty);
	m_loaded = false;
}

document_base^	document_editor_base::create_new_document	()
{
	if(creating_new_document == nullptr || creating_new_document->GetInvocationList()->Length == 0)
		return gcnew document_base(this);
	
	return creating_new_document();
}

void			document_editor_base::save_all			(System::Object^ , System::EventArgs^ )
{
	for each(document_base^ doc in m_opened_documents)
		doc->save();
	all_saved(this, EventArgs::Empty);
}

void			document_editor_base::save_active		(System::Object^ , System::EventArgs^ )
{
	if(m_active_document != nullptr)
		m_active_document->save();
	active_saved(this, EventArgs::Empty);
}

void			document_editor_base::on_document_closing	(document_base^ closing_doc)
{
	ASSERT(m_opened_documents->Contains(closing_doc));
	m_opened_documents->Remove(closing_doc);
	document_closed(this, EventArgs::Empty);
}

void			document_editor_base::on_document_activated	(document_base^ doc)
{
	m_active_document = doc;
	view_panel->track_active_item(doc->Name);
}

void			document_editor_base::show_properties		(Object^ obj)
{
	properties_panel->show_properties(obj);
}

void			document_editor_base::undo				(System::Object^ , System::EventArgs^ )
{
	if(m_active_document!=nullptr)
		m_active_document->undo();
	history_undo(this, EventArgs::Empty);
}

void			document_editor_base::redo				(System::Object^ , System::EventArgs^ )
{
	if(m_active_document!=nullptr)
		m_active_document->redo();
	history_redo(this, EventArgs::Empty);
}

void			document_editor_base::cut				(System::Object^ , System::EventArgs^ )
{
	if(m_main_dock_panel->ActiveContent==view_panel)
	{
		view_panel->copy(true);
		return;
	}

	if(m_active_document!=nullptr)
		m_active_document->copy(true);
	cuted(this, EventArgs::Empty);
}

void			document_editor_base::del				(System::Object^ sender, System::EventArgs^ )
{
	if(m_main_dock_panel->ActiveContent==view_panel)
	{
		view_panel->remove_node(sender, gcnew tree_view_event_args());
		return;
	}

	if(m_active_document!=nullptr)
		m_active_document->del();
	deleted(this, EventArgs::Empty);
}

void			document_editor_base::copy				(System::Object^ , System::EventArgs^ )
{
	if(m_main_dock_panel->ActiveContent==view_panel)
	{
		view_panel->copy(false);
		return;
	}

	if(m_active_document!=nullptr)
		m_active_document->copy(false);
	copied(this, EventArgs::Empty);
}

void			document_editor_base::paste				(System::Object^ , System::EventArgs^ )
{
	if(m_main_dock_panel->ActiveContent==view_panel)
	{
		view_panel->paste();
		return;
	}

	if(m_active_document!=nullptr)
		m_active_document->paste();

	pasted(this, EventArgs::Empty);
}

void			document_editor_base::select_all			(System::Object^ , System::EventArgs^ )
{
	if(m_active_document!=nullptr)
		m_active_document->select_all();

	all_selected(this, EventArgs::Empty);
}

void			document_editor_base::close_all_documents			()
{
	while( m_opened_documents->Count > 0)
	{
		m_opened_documents[0]->Close();
	}
}

Boolean			document_editor_base::load_panels			(Form^ parent)
{
	return load_panels(parent, true);
}

Boolean			document_editor_base::load_panels			(Form^ parent, Boolean create_base_panel_objects)
{
	Boolean is_loaded_normally = false;

	if(create_base_panel_objects)
	{
		//Create panels
		if(create_view_panel)		view_panel			= gcnew file_view_panel_base(this, "Particle Groups");
		if(create_properties_panel)	properties_panel	= gcnew item_properties_panel_base(this, "Particle Node Properties");
		if(create_toolbar_panel)	toolbar_panel		= gcnew toolbar_panel_base(this, "Particle Node TollBar");
	}

	//Load panels settings
	is_loaded_normally = serializer::deserialize_dock_panel_root(parent, m_main_dock_panel, this->Name,
		gcnew DeserializeDockContent(this, &document_editor_base::reload_content));

	//Show panels
	if(is_loaded_normally)
	{
		if ( create_view_panel ) 		view_panel->Show		(m_main_dock_panel);
		if ( create_properties_panel )	properties_panel->Show	(m_main_dock_panel);
		if ( create_toolbar_panel )		toolbar_panel->Show		(m_main_dock_panel);
	}
	else
	{
		if ( create_view_panel ) {
			m_main_dock_panel->DockLeftPortion	= .25f;
			view_panel->Show		(m_main_dock_panel, WeifenLuo::WinFormsUI::Docking::DockState::DockLeft );
		}

		if ( create_properties_panel )	properties_panel->Show	(view_panel->Pane, WeifenLuo::WinFormsUI::Docking::DockAlignment::Bottom, .25f );
		if ( create_toolbar_panel )	toolbar_panel->Show		(m_main_dock_panel, WeifenLuo::WinFormsUI::Docking::DockState::DockRight );
	}

	panels_loaded(this, EventArgs::Empty);
	return is_loaded_normally;
}
IDockContent^	document_editor_base::reload_content		(String^ panel_full_name)
{
	if (panel_full_name == "xray.editor.controls.file_view_panel_base")
		return			view_panel;
	else if (panel_full_name == "xray.editor.controls.item_properties_panel_base")
		return			properties_panel;
	else if (panel_full_name == "xray.editor.controls.toolbar_panel_base")
		return			toolbar_panel;

	if(content_reloading != nullptr)
		return content_reloading(panel_full_name);
	return nullptr;
}
void			document_editor_base::save_panels			(Form^ parent)
{
	serializer::serialize_dock_panel_root(parent, m_main_dock_panel, this->Name);
	panels_saved(this, EventArgs::Empty);
}

static void		remove_context_menu_recursive(TreeNode^ node)
{
	node->ContextMenu		= nullptr;
	node->ContextMenuStrip	= nullptr;

	for each(TreeNode^ sub_node in node->Nodes)
	{
		remove_context_menu_recursive(sub_node);
	}
}

void			document_editor_base::remove_context_menu			()
{
	if(view_panel != nullptr)
	{
		view_panel->tree_view->ContextMenu		= nullptr;
		view_panel->tree_view->ContextMenuStrip	= nullptr;

		remove_context_menu_recursive(view_panel->tree_view->root);
	}
}

void			document_editor_base::main_panel_content_added	(System::Object^ sender, DockContentEventArgs^ e)
{
	if(is_fixed_panels)
		is_fixed_panels		= is_fixed_panels;
}

void			document_editor_base::add_menu_items			(System::Windows::Forms::MenuStrip^ parent_menu)
{
	Controls->Remove(menuStrip1);
	menuStrip1 = parent_menu;
	Controls->Add(menuStrip1);
}

void			document_editor_base::add_menu_items			(System::Windows::Forms::ToolStripMenuItem^ menu_item)
{
	menuStrip1->Items->Add(menu_item);
}

}//namespace controls
}//namespace editor
}//namespace xray
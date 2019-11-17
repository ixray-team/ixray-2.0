//-------------------------------------------------------------------------------------------
//	Created		: 27.05.2010
//	Author		: Sergey Pryshchepa
//	Copyright (C) GSC Game World - 2010
//-------------------------------------------------------------------------------------------
#include "pch.h"
#include "dialog_editor.h"
#include "dialog_text_editor.h"
#include "dialog_language_panel.h"
#include "dialog_document.h"
#include "dialog_graph_node.h"
#include "dialog_precondition_action_editor.h"
#include "dialog_export_form.h"
#include "dialog_import_form.h"
#include "dialog_graph_node_style.h"
#include "dialog_files_view_panel.h"
#include "dialog_item_properties_panel.h"
#include "dialog_control_panel.h"

using xray::dialog_editor::dialog_editor_impl;
using xray::dialog_editor::dialog_graph_node;
using xray::dialog_editor::dialog_document;
using xray::dialog_editor::dialog_graph_link;
using namespace WeifenLuo::WinFormsUI::Docking;
using namespace System::Windows::Forms;

void dialog_editor_impl::initialize_components()
{
	m_form = gcnew System::Windows::Forms::Form();
	m_node_style_mgr = gcnew dialog_graph_nodes_style_mgr();

	localizations_menu_item = gcnew System::Windows::Forms::ToolStripMenuItem();
	add_language_menu_item = gcnew System::Windows::Forms::ToolStripMenuItem();
	remove_language_menu_item = gcnew System::Windows::Forms::ToolStripMenuItem();
	export_to_xls_menu_item = gcnew System::Windows::Forms::ToolStripMenuItem();
	import_from_xls_menu_item = gcnew System::Windows::Forms::ToolStripMenuItem();

	view_menu_item = gcnew System::Windows::Forms::ToolStripMenuItem();
	language_panel_menu_item = gcnew System::Windows::Forms::ToolStripMenuItem();
	file_view_panel_menu_item = gcnew System::Windows::Forms::ToolStripMenuItem();
	properties_panel_menu_item = gcnew System::Windows::Forms::ToolStripMenuItem();
	control_panel_menu_item = gcnew System::Windows::Forms::ToolStripMenuItem();

	m_export_form = gcnew dialog_export_form(this);
	m_import_form = gcnew dialog_import_form(this);
	m_text_editor = gcnew dialog_text_editor(this);
	m_pred_act_editor = gcnew dialog_precondition_action_editor(this);
	m_language_panel = gcnew dialog_language_panel(this);

	localizations_menu_item->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(4) {
		add_language_menu_item, 
			remove_language_menu_item, 
			export_to_xls_menu_item, 
			import_from_xls_menu_item, 
	});
	localizations_menu_item->Name = L"localizations_menu_item";
	localizations_menu_item->Size = System::Drawing::Size(67, 20);
	localizations_menu_item->Text = L"&Localizations";

	add_language_menu_item->Name = L"add_language_menu_item";
	add_language_menu_item->Size = System::Drawing::Size(197, 22);
	add_language_menu_item->Text = L"&Add language";
	add_language_menu_item->Click += gcnew System::EventHandler(this, &dialog_editor_impl::on_add_language_click);

	remove_language_menu_item->Name = L"remove_language_menu_item";
	remove_language_menu_item->Size = System::Drawing::Size(197, 22);
	remove_language_menu_item->Text = L"&Remove language";
	remove_language_menu_item->Click += gcnew System::EventHandler(this, &dialog_editor_impl::on_remove_language_click);

	export_to_xls_menu_item->Name = L"export_to_xls_menu_item";
	export_to_xls_menu_item->Size = System::Drawing::Size(197, 22);
	export_to_xls_menu_item->Text = L"&Export to *.xlsx";
	export_to_xls_menu_item->Click += gcnew System::EventHandler(this, &dialog_editor_impl::on_export_click);

	import_from_xls_menu_item->Name = L"import_from_xls_menu_item";
	import_from_xls_menu_item->Size = System::Drawing::Size(197, 22);
	import_from_xls_menu_item->Text = L"&Import from *.xlsx";
	import_from_xls_menu_item->Click += gcnew System::EventHandler(this, &dialog_editor_impl::on_import_click);

	view_menu_item->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(4) {
		control_panel_menu_item, 
			file_view_panel_menu_item, 
			properties_panel_menu_item, 
			language_panel_menu_item, 
	});
	view_menu_item->Name = L"view_menu_item";
	view_menu_item->Size = System::Drawing::Size(67, 20);
	view_menu_item->Text = L"&View";
	view_menu_item->Click += gcnew System::EventHandler(this, &dialog_editor_impl::on_view_menu_item_select);

	control_panel_menu_item->Checked = true;
	control_panel_menu_item->CheckOnClick = true;
	control_panel_menu_item->Name = L"control_panel_menu_item";
	control_panel_menu_item->Size = System::Drawing::Size(197, 22);
	control_panel_menu_item->Text = L"Toolbox";
	control_panel_menu_item->Click += gcnew System::EventHandler(this, &dialog_editor_impl::on_control_panel_menu_select);

	file_view_panel_menu_item->Checked = true;
	file_view_panel_menu_item->CheckOnClick = true;
	file_view_panel_menu_item->Name = L"file_view_panel_menu_item";
	file_view_panel_menu_item->Size = System::Drawing::Size(197, 22);
	file_view_panel_menu_item->Text = L"File viewer";
	file_view_panel_menu_item->Click += gcnew System::EventHandler(this, &dialog_editor_impl::on_file_view_panel_menu_select);

	properties_panel_menu_item->Checked = true;
	properties_panel_menu_item->CheckOnClick = true;
	properties_panel_menu_item->Name = L"properties_panel_menu_item";
	properties_panel_menu_item->Size = System::Drawing::Size(197, 22);
	properties_panel_menu_item->Text = L"Properties viewer";
	properties_panel_menu_item->Click += gcnew System::EventHandler(this, &dialog_editor_impl::on_properties_panel_menu_select);

	language_panel_menu_item->Checked = true;
	language_panel_menu_item->CheckOnClick = true;
	language_panel_menu_item->Name = L"language_panel_menu_item";
	language_panel_menu_item->Size = System::Drawing::Size(197, 22);
	language_panel_menu_item->Text = L"Languages panel";
	language_panel_menu_item->Click += gcnew System::EventHandler(this, &dialog_editor_impl::on_language_panel_menu_select);

	m_multidocument_base = gcnew document_editor_base(m_name);
	m_multidocument_base->view_panel = gcnew dialog_files_view_panel(m_multidocument_base);
	m_multidocument_base->properties_panel = gcnew dialog_item_properties_panel(this);
	m_multidocument_base->toolbar_panel = gcnew dialog_control_panel(m_multidocument_base);
	m_multidocument_base->view_panel->source_path = resources_path+"configs/";
	m_multidocument_base->main_dock_panel->DockBackColor = System::Drawing::SystemColors::AppWorkspace; 
	m_multidocument_base->active_saved += gcnew EventHandler(this, &dialog_editor_impl::save_active);
	m_multidocument_base->all_saved += gcnew EventHandler(this, &dialog_editor_impl::save_all);
	m_multidocument_base->editor_exited += gcnew EventHandler(this, &dialog_editor_impl::on_editor_exiting);
	m_multidocument_base->creating_new_document = gcnew document_create_callback(this, &dialog_editor_impl::on_document_creating);
	m_multidocument_base->content_reloading = gcnew content_reload_callback(this, &dialog_editor_impl::find_dock_content);
	m_multidocument_base->KeyDown += gcnew System::Windows::Forms::KeyEventHandler(this, &dialog_editor_impl::on_editor_key_down);
	m_multidocument_base->add_menu_items(view_menu_item);
	m_multidocument_base->add_menu_items(localizations_menu_item);
	m_multidocument_base->load_panels(m_form, false);
	m_multidocument_base->Dock = DockStyle::Fill;
	m_multidocument_base->ActiveControl = m_multidocument_base->view_panel;

	m_form->Name = m_name;
	m_form->Text = "Dialog editor";
	m_form->Controls->Add(m_multidocument_base);
	m_form->FormClosing += gcnew System::Windows::Forms::FormClosingEventHandler(this, &dialog_editor_impl::on_editor_closing);
	m_language_panel->Show(m_multidocument_base->toolbar_panel->Pane->DockPanel, DockState::DockRight);
}

void dialog_editor_impl::destroy_components()
{
	delete m_pred_act_editor;
	delete m_text_editor;
	delete m_language_panel;
	delete m_node_style_mgr;
	delete m_form;
}

void dialog_editor_impl::show()
{
	m_form->Show();
}

IDockContent^ dialog_editor_impl::find_dock_content(System::String^ persist_string)
{
	if(persist_string == "xray.dialog_editor.dialog_files_view_panel")
		return m_multidocument_base->view_panel;

	if (persist_string == "xray.dialog_editor.dialog_item_properties_panel")
		return m_multidocument_base->properties_panel;

	if (persist_string == "xray.dialog_editor.dialog_control_panel")
		return m_multidocument_base->toolbar_panel;

	if (persist_string == "xray.dialog_editor.dialog_language_panel")
		return m_language_panel;

	return (nullptr);
}


void dialog_editor_impl::on_selection_changed(dialog_graph_node^ n)
{
	m_text_editor->update_selected_node(n);
}

void dialog_editor_impl::on_editor_closing(Object^ , FormClosingEventArgs^ e)
{
	if(e->CloseReason==System::Windows::Forms::CloseReason::UserClosing)
	{
		m_form->Hide();
		e->Cancel = true;
	}
}

void dialog_editor_impl::on_editor_exiting(Object^ , EventArgs^ )
{
	m_form->Hide();
}

xray::editor::controls::document_base^ dialog_editor_impl::on_document_creating()
{
	return gcnew dialog_document(this);
}

void dialog_editor_impl::on_node_property_changed(System::Object^ sender, property_grid_value_changed_event_args^ e)
{
	xray::editor::wpf_controls::property_grid^ g = safe_cast<xray::editor::wpf_controls::property_grid^>(sender);
	dialog_graph_node^ n = safe_cast<dialog_graph_node^>(g->selected_object);
	if(n->is_dialog() && (e->changed_property->name=="text" || e->changed_property->name=="string_table"))
	{
		MessageBox::Show(m_form, "You can't change string table id or text for dialog!", 
			m_form->Text, MessageBoxButtons::OK, MessageBoxIcon::Error);

		n->GetType()->GetProperty(e->changed_property->name)->SetValue(n, e->old_value, nullptr);
		m_multidocument_base->show_properties(n);
		return;
	}

	if(e->changed_property->name=="text")
		safe_cast<dialog_document^>(m_multidocument_base->active_document)->on_node_text_changed(n, n->string_table, (String^)e->new_value, n->string_table, (String^)e->old_value);
	else if(e->changed_property->name=="string_table")
	{
		String^ new_str = (String^)e->new_value;
		String^ old_str = (String^)e->old_value;
		String^ new_text = get_text_by_id(new_str);
		String^ old_text = get_text_by_id(old_str);
		safe_cast<dialog_document^>(m_multidocument_base->active_document)->on_node_text_changed(n, new_str, new_text, old_str, old_text);
	}
	else
		safe_cast<dialog_document^>(m_multidocument_base->active_document)->on_node_property_changed(n, e->changed_property->name, e->new_value, e->old_value);
}

void dialog_editor_impl::update_node_text(dialog_graph_node^ n, System::String^ id, System::String^ txt)
{
	if(id==n->string_table && txt==n->text)
		return;

	safe_cast<dialog_document^>(m_multidocument_base->active_document)->on_node_text_changed(n, id, txt, n->string_table, n->text);
}

void dialog_editor_impl::update_documents_text()
{
	for each(dialog_document^ d in m_multidocument_base->opened_documents)
		d->update_texts();
}

void dialog_editor_impl::show_prec_act_editor(dialog_graph_link^ lnk)
{
	m_pred_act_editor->show(lnk, true, true);
}

void dialog_editor_impl::show_prec_act_editor(dialog_graph_link^ lnk, bool act_v, bool prec_v)
{
	m_pred_act_editor->show(lnk, act_v, prec_v);
}

void dialog_editor_impl::show_text_editor(dialog_graph_node^ n)
{
	if(n!=nullptr && !(m_text_editor->Visible))
	{
		m_text_editor->assign_string_table_ids();
		m_text_editor->Show(m_multidocument_base);
		m_text_editor->update_selected_node(n);
	}
}

void dialog_editor_impl::on_editor_key_down(Object^ , KeyEventArgs^ e)
{
	if(e->KeyCode==Keys::Tab && e->Control)
	{
		int cur_doc_count = m_multidocument_base->opened_documents->Count;
		if(cur_doc_count>1)
		{
			for(int i=0; i<cur_doc_count; i++)
				if(m_multidocument_base->opened_documents->default[i]==m_multidocument_base->active_document)
				{
					if(i==cur_doc_count-1)
						i = -1;

					m_multidocument_base->opened_documents->default[i+1]->Activate();
					m_multidocument_base->active_document = m_multidocument_base->opened_documents->default[i+1];
					return;
				}
		}
	}
	else if(e->KeyCode==Keys::F4 && e->Control)
	{
		if(m_multidocument_base->active_document!=nullptr)
		{
			m_multidocument_base->active_document->Close();
			return;
		}
	}
}

void dialog_editor_impl::on_add_language_click(System::Object^ , System::EventArgs^ )
{
	m_language_panel->add_language(nullptr, nullptr);
}

void dialog_editor_impl::on_remove_language_click(System::Object^ , System::EventArgs^ )
{
	m_language_panel->remove_language(nullptr, nullptr);
}

void dialog_editor_impl::on_export_click(System::Object^ , System::EventArgs^ )
{
	m_export_form->show(resources_path);
}

void dialog_editor_impl::on_import_click(System::Object^ , System::EventArgs^ )
{
	m_import_form->show(resources_path);
}

void dialog_editor_impl::on_view_menu_item_select(Object^ , EventArgs^ )
{
	language_panel_menu_item->Checked = m_language_panel->Visible;
	file_view_panel_menu_item->Checked = m_multidocument_base->view_panel->Visible;
	properties_panel_menu_item->Checked = m_multidocument_base->properties_panel->Visible;
	control_panel_menu_item->Checked = m_multidocument_base->toolbar_panel->Visible;
}

void dialog_editor_impl::on_language_panel_menu_select(Object^ , EventArgs^ )
{
	if(!(language_panel_menu_item->Checked))
		m_language_panel->Hide();
	else
		m_language_panel->Show(m_multidocument_base->main_dock_panel, DockState::DockRight);
}

void dialog_editor_impl::on_file_view_panel_menu_select(Object^ , EventArgs^ )
{
	if(!(file_view_panel_menu_item->Checked))
		m_multidocument_base->view_panel->Hide();
	else
		m_multidocument_base->view_panel->Show(m_multidocument_base->main_dock_panel, DockState::DockLeft);
}

void dialog_editor_impl::on_properties_panel_menu_select(Object^ , EventArgs^ )
{
	if(!(properties_panel_menu_item->Checked))
		m_multidocument_base->properties_panel->Hide();
	else
		m_multidocument_base->properties_panel->Show(m_multidocument_base->main_dock_panel, DockState::DockRight);
}

void dialog_editor_impl::on_control_panel_menu_select(Object^ , EventArgs^ )
{
	if(!(control_panel_menu_item->Checked))
		m_multidocument_base->toolbar_panel->Hide();
	else
		m_multidocument_base->toolbar_panel->Show(m_multidocument_base->main_dock_panel, DockState::DockRight);
}


//-------------------------------------------------------------------------------------------
//	Created		: 16.12.2009
//	Author		: Sergey Pryshchepa
//	Copyright (C) GSC Game World - 2009
//-------------------------------------------------------------------------------------------
#ifndef DIALOG_EDITOR_H_INCLUDED
#define DIALOG_EDITOR_H_INCLUDED

#include "string_table_ids_storage.h"

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;
using namespace WeifenLuo::WinFormsUI::Docking;
using namespace xray::editor::controls;

using xray::editor::wpf_controls::property_grid_value_changed_event_args;

namespace xray {
namespace dialog_editor {
	namespace string_tables {
		class string_table_id_options;
		struct strings_compare_predicate;
	}
	class game_dialog_cooker;
	class dialog_cooker;
	class dialog_editor_resources_cooker;
	ref class dialog_document;
	ref class dialog_files_view_panel;
	ref class dialog_item_properties_panel;
	ref class dialog_control_panel;
	ref class dialog_text_editor;
	ref class dialog_language_panel;
	ref class dialog_precondition_action_editor;
	ref class dialog_graph_node;
	ref class dialog_graph_nodes_style_mgr;
	ref class dialog_graph_link;
	ref class dialog_export_form;
	ref class dialog_import_form;
	ref class dialog_editor_impl_test;

	public ref class dialog_editor_impl : public xray::editor_base::editor_base
	{
		typedef map<pstr, xray::dialog_editor::string_tables::string_table_id_options*, xray::dialog_editor::string_tables::strings_compare_predicate> references_table;
		typedef Collections::Generic::Dictionary<String^, u32> last_translators_ids_type;
	public:
												dialog_editor_impl		(System::String^ resources_path);
		virtual									~dialog_editor_impl		();

		virtual xray::editor_base::command_engine^	get_command_engine	() {return nullptr;};
		virtual void							initialize				();
		virtual void							destroy					();
		virtual void							clear_resources			() {};
		virtual bool							close_query				();
		virtual void							on_render				() {};
		virtual	void							tick					() {};
		virtual bool							load					(System::String^ name);
		virtual void							load_settings			() {};
		virtual void							save_settings			() {};
		virtual	void							set_default_layout		() {};
		virtual IDockContent^					find_dock_content		(System::String^ );
		virtual System::String^					name					() {return m_name;};
		virtual	void							show					();

				void					show_text_editor		(dialog_graph_node^ n);
				void					show_prec_act_editor	(dialog_graph_link^ lnk);
				void					show_prec_act_editor	(dialog_graph_link^ lnk, bool act_v, bool prec_v);
		dialog_graph_nodes_style_mgr^	get_node_style_mgr		()		{return m_node_style_mgr;};
				
				void					update_documents_text	();
				void					update_node_text		(dialog_graph_node^ n, System::String^ id, System::String^ txt);
				void					on_selection_changed	(dialog_graph_node^ n);
				void					on_node_property_changed(System::Object^ sender, property_grid_value_changed_event_args^ e);

				void					add_new_id				(System::String^ id, System::String^ txt);
				bool					change_ids_text			(System::String^ id, System::String^ txt, bool use_message);
				System::String^			get_text_by_id			(System::String^ id);
				void					change_references_count	(System::String^ id1, System::String^ id2);
				void					set_id_category			(System::String^ id, u32 new_cat);
				u32						id_category				(System::String^ id);
				void					set_cur_language		(System::String^ lang);
				void					on_editor_key_down		(Object^ sender, KeyEventArgs^ e);
				void					add_language			(System::String^ lang_name);
				void					remove_language			(System::String^ lang_name);
				u32						last_translators_id		(System::String^ lang_name);
				void					set_last_translators_id	(System::String^ lang_name, u32 new_last_id);
				u32						references_count		(System::String^ id);
				bool					is_batch_file_name_empty(System::String^ id, System::String^ lang_name);
				void					set_batch_file_name		(System::String^ id, System::String^ lang_name, System::String^ new_file_name);
				String^					batch_file_name			(System::String^ id, System::String^ lang_name);
				void					save_options			();
				void					save_string_table		();

		property document_editor_base^	multidocument_base
		{	
			document_editor_base^		get(){return m_multidocument_base;}
		}
		property System::String^		resources_path;

				void					on_add_language_click	(System::Object^ , System::EventArgs^ );
				void					on_remove_language_click(System::Object^ , System::EventArgs^ );
				void					on_export_click			(System::Object^ , System::EventArgs^ );
				void					on_import_click			(System::Object^ , System::EventArgs^ );

	protected:
				void					save_active				(System::Object^ sender, System::EventArgs^ e);
				void					save_all				(System::Object^ sender, System::EventArgs^ e);
				document_base^			on_document_creating	();
	
	private:
				void					initialize_components	();
				void					destroy_components		();

				void					on_references_loaded	(xray::resources::queries_result& data);
				void					on_languages_list_loaded(xray::resources::queries_result& data);
				void					on_editor_closing		(Object^ sender, FormClosingEventArgs^ e);
				void					on_editor_exiting		(Object^ sender, EventArgs^ e);
				bool					save_confirmation		();

				void					on_view_menu_item_select		(Object^ , EventArgs^ );
				void					on_language_panel_menu_select	(Object^ , EventArgs^ );
				void					on_file_view_panel_menu_select	(Object^ , EventArgs^ );
				void					on_properties_panel_menu_select	(Object^ , EventArgs^ );
				void					on_control_panel_menu_select	(Object^ , EventArgs^ );

	private:
		System::String^						m_name;
		System::Windows::Forms::Form^		m_form;
		document_editor_base^				m_multidocument_base;
		dialog_graph_nodes_style_mgr^		m_node_style_mgr;
		dialog_text_editor^					m_text_editor;
		dialog_language_panel^				m_language_panel;
		dialog_precondition_action_editor^	m_pred_act_editor;
		references_table*					m_references_table;
		dialog_cooker*						m_dialog_cooker;
		dialog_editor_resources_cooker*		m_dialog_resources_cooker;
		last_translators_ids_type^			m_last_translators_ids;
		dialog_export_form^					m_export_form;
		dialog_import_form^					m_import_form;

		System::Windows::Forms::ToolStripMenuItem^  localizations_menu_item;
		System::Windows::Forms::ToolStripMenuItem^  add_language_menu_item;
		System::Windows::Forms::ToolStripMenuItem^  remove_language_menu_item;
		System::Windows::Forms::ToolStripMenuItem^  export_to_xls_menu_item;
		System::Windows::Forms::ToolStripMenuItem^  import_from_xls_menu_item;

		System::Windows::Forms::ToolStripMenuItem^  view_menu_item;
		System::Windows::Forms::ToolStripMenuItem^  language_panel_menu_item;
		System::Windows::Forms::ToolStripMenuItem^  file_view_panel_menu_item;
		System::Windows::Forms::ToolStripMenuItem^  properties_panel_menu_item;
		System::Windows::Forms::ToolStripMenuItem^  control_panel_menu_item;
	}; // ref class dialog_editor_impl

	public enum class drag_drop_create_operation: int 
	{
		single_node = 0,
		question_answer = 1,
	};
} // namespace dialog_editor 
} // namespace xray 
#endif // #ifndef DIALOG_EDITOR_H_INCLUDED

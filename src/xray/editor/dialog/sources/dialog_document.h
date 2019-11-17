//-------------------------------------------------------------------------------------------
//	Created		: 17.12.2009
//	Author		: Sergey Prishchepa
//	Copyright (C) GSC Game World - 2009
//-------------------------------------------------------------------------------------------
#pragma once

#include "dialog_hypergraph.h"
#include "dialog_editor_resources_cooker.h"

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;

namespace xray {
namespace dialog_editor {
	ref class dialog_editor_impl;
	ref class dialog_graph_node;
	ref class dialog_links_manager;
	ref class dialog_graph_link;
	ref class dialog_hypergraph;
	ref class xray::editor::controls::hypergraph::node;
	ref class xray::editor::controls::hypergraph::link;
	ref class xray::editor::controls::scalable_scroll_bar;
	ref class xray::editor::controls::scroller_event_args;
	class dialog;
	class dialog_node_base;
	struct dialog_graph_node_layout;

	public ref class dialog_document : public xray::editor::controls::document_base
	{
		typedef xray::editor::controls::document_base super;
		typedef Generic::List<xray::editor::controls::hypergraph::node^ > nodes;
		typedef Generic::List<xray::editor::controls::hypergraph::link^ > links;

	public:
							dialog_document(dialog_editor_impl^ ed);
							~dialog_document();
							!dialog_document();

		dialog_graph_node^	make_node					();
				void		add_dialog_node				(System::String^ node_name, Point p);
		dialog_graph_link^	create_connection			(xray::editor::controls::hypergraph::node^ source, xray::editor::controls::hypergraph::node^ destination, bool create_dialog_link);
				void		add_connection				(xray::editor::controls::hypergraph::node^ source, xray::editor::controls::hypergraph::node^ destination);
				void		add_connection				(links^ lnks);

		virtual	void		save						() override;
		virtual	void		load						() override;
		virtual	void		undo						() override;
		virtual	void		redo						() override;
		virtual	void		copy						(bool del) override;
		virtual	void		paste						() override;
		virtual	void		select_all					() override;
		virtual	void		del							() override;
				void		on_node_property_changed	(dialog_graph_node^ n, String^ l, Object^ new_val, Object^ old_val);
				void		on_node_text_changed		(dialog_graph_node^ n, String^ new_str, String^ new_text, String^ old_str, String^ old_text);
				void		on_node_double_click		(dialog_graph_node^ n);
				void		update_texts				();
		dialog_editor_impl^	get_editor					();
				dialog*		get_dialog					();
				void		generate_node_string_table_id(dialog_graph_node^ n);

	private:
		void				on_resources_loaded			(xray::resources::queries_result& data);
		void				on_template_loaded			(xray::resources::queries_result& data);
		void				on_dialog_loaded			(xray::resources::queries_result& data);
		void				load_impl					(dialog* d, dialog_graph_node_layout* l);
		void				on_hypergraph_click			(System::Object^ sender, System::EventArgs^ e);
		void				on_hypergraph_drag_over		(System::Object^ sender, System::Windows::Forms::DragEventArgs^ e);
		void				on_hypergraph_drag_drop		(System::Object^ sender, System::Windows::Forms::DragEventArgs^ e);
		void				on_hypergraph_selection_changed();
		void				on_activated				(System::Object^ sender, System::EventArgs^ e);

		void				on_drop_single_node			(System::Windows::Forms::DragEventArgs^ e);
		void				on_drop_question_answer		(System::Windows::Forms::DragEventArgs^ e);
		void				on_drop_dialog				(System::Windows::Forms::DragEventArgs^ e);
		void				on_hypergraph_double_click	(System::Object^ sender, System::EventArgs^ e);

	private:
		dialog_editor_impl^							m_owner;
		dialog_resources_ptr*						m_resources;
		System::ComponentModel::Container^			components;
		dialog_hypergraph^							m_hypergraph;
		dialog_links_manager^						m_links_manager;
	}; // ref class dialog_document
} // namespace dialog_editor
} // namespace xray
//-------------------------------------------------------------------------------------------
//	Created		: 25.03.2010
//	Author		: Sergey Prishchepa
//	Copyright (C) GSC Game World - 2010
//-------------------------------------------------------------------------------------------
#pragma once

using namespace System;
using namespace System::Collections;
using namespace System::ComponentModel;

#include "dialog_link.h"
#include "dialog_expression.h"
#include "dialog_action.h"

namespace xray {
namespace dialog_editor {
	ref class dialog_link_ui_type_editor;
	ref class dialog_link_action;
	ref class dialog_link_precondition;

	public ref class dialog_graph_link : public xray::editor::controls::hypergraph::link
	{
		typedef xray::editor::controls::hypergraph::link					super;
		typedef xray::editor::controls::hypergraph::connection_point		connection_point;

	public:
		dialog_graph_link	(connection_point^ src, connection_point^ dst, bool create_dialog_link);
		~dialog_graph_link	();

		[CategoryAttribute("Link properties"), DisplayNameAttribute("Preconditions"),
			EditorAttribute(dialog_link_ui_type_editor::typeid,System::Drawing::Design::UITypeEditor::typeid)]
		property dialog_expression*	prec
		{
			dialog_expression*		get		() {return m_link->root_precondition();};
		};

		[CategoryAttribute("Link properties"), DisplayNameAttribute("Actions"),
			EditorAttribute(dialog_link_ui_type_editor::typeid,System::Drawing::Design::UITypeEditor::typeid)]
		property vector<dialog_action*>* act
		{
			vector<dialog_action*>*	get		() {return m_link->actions();};
		};

		property dialog_link* m_link;

		void						set_preds_and_acts(dialog_link_precondition^ prec_root, Generic::List<dialog_link_action^>^ act_list);
		void						load	(xray::configs::lua_config_value const& cfg){m_link->load(cfg);};
		void						save	(xray::configs::lua_config_value cfg)		{m_link->save(cfg);};
	}; // ref class dialog_graph_link
} // namespace dialog_editor
} // namespace xray
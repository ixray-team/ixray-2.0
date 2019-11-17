//-------------------------------------------------------------------------------------------
//	Created		: 16.03.2010
//	Author		: Sergey Pryshchepa
//	Copyright (C) GSC Game World - 2010
//-------------------------------------------------------------------------------------------
#ifndef DIALOG_H_INCLUDED
#define DIALOG_H_INCLUDED

#include "dialogs_manager.h"
#include "dialog_node_visitors.h"
#pragma managed(push, off)

namespace xray {
namespace dialog_editor {
	class dialog_phrase;
	class dialog_node_gatherer_visitor;
	class dialog_node_giver_visitor;

	class dialog : public dialog_node_base
	{
		typedef vector<dialog_node_base_ptr> phrases_list;
		friend class dialog_node_giver_visitor;
	public:
										dialog			();
		virtual							~dialog			();
		virtual	void					set_text		(pcstr new_text);
		virtual pcstr					text			();
				dialog_phrase*			new_phrase		();
				dialog*					new_dialog		();
				void					add_node		(dialog_node_base_ptr n);
				void					remove_node		(dialog_node_base_ptr n);
				void					remove_node		(u32 id);
				dialog_node_base_ptr*	get_node_by_id	(u32 id);
		const	phrases_list*			get_phrases		()	{return &m_phrases;};
				void					save			(configs::lua_config_value cfg);
				void					load			(configs::lua_config_value const& cfg);
		virtual	void					accept			(dialog_node_gatherer_visitor* visitor) {visitor->visit(this);};
		virtual	void					accept			(dialog_node_giver_visitor* visitor) {visitor->visit(this);};
				void					clear			();

//		virtual	void					recalculate_memory_usage_impl () { m_memory_usage_self.unmanaged = get_size(); }

	private:
		phrases_list	m_phrases;
		pstr			m_text;
		u32				m_last_id;
	}; // class dialog
} // namespace dialog_editor
} // namespace xray
#pragma managed(pop)
#endif // #ifndef DIALOG_H_INCLUDED

//-------------------------------------------------------------------------------------------
//	Created		: 15.04.2010
//	Author		: Sergey Pryshchepa
//	Copyright (C) GSC Game World - 2010
//-------------------------------------------------------------------------------------------
#ifndef DIALOG_MANAGER_H_INCLUDED
#define DIALOG_MANAGER_H_INCLUDED

#pragma managed(push, off)
//#include <xray/intrusive_list.h>
#include "dialog_node_base.h"

namespace xray {
namespace dialog_editor {

	class dialog_node_base;
	typedef	intrusive_ptr<dialog_node_base, resources::unmanaged_resource, threading::multi_threading_interlocked_policy> dialog_node_base_ptr;

	class game_dialog;

namespace dialog_manager {

	class dialogs_manager;

	dialogs_manager*	create_dialogs_manager	();
	dialogs_manager*	get_dialogs_manager		();
	void				destroy_dialogs_manager	();

	class dialogs_manager : private boost::noncopyable
	{
	public:
							dialogs_manager				();
							~dialogs_manager			();
		dialog_node_base*	get_dialog_by_name			(pcstr name);
		void				add_dialog					(dialog_node_base* dlg);
		void				remove_dialog				(pcstr name);
		void				set_return_exsisting_dialog	(bool val)				{m_return_exsisting_dialog=val;};
		bool				return_exsisting_dialog		()						{return m_return_exsisting_dialog;};


	private:
		struct comp_pred
		{
			inline bool	operator()			(pcstr s1, pcstr s2) const
			{
				return (strings::compare(s1, s2)<0);
			}
		}; // struct compare_predicate

		typedef associative_vector<pcstr, dialog_node_base*, vector, comp_pred> dialogs_list;
		dialogs_list			m_dialogs_list;
		u32						m_last_id;
		bool					m_return_exsisting_dialog;
	}; // class dialogs_manager
} // namespace dialog_manager
} // namespace dialog_editor 
} // namespace xray 
#pragma managed(pop)
#endif // #ifndef DIALOG_MANAGER_H_INCLUDED

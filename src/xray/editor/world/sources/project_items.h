////////////////////////////////////////////////////////////////////////////
//	Created		: 20.03.2009
//	Author		: AndreW Kolomiets
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef PROJECT_ITEMS_H_INCLUDED
#define PROJECT_ITEMS_H_INCLUDED

#include "project_defines.h"

namespace xray {
namespace editor {

class editor_world;

public ref struct project_tree_node_holder
{
	tree_node^				m_tree_node;
	project_tree_node_holder():m_tree_node(nullptr){}
};

public ref class project_item : public project_tree_node_holder
{
public:
			project_item	(project^ p, project_group^ pg);
			project_item	(project^ p, object_base^ object, project_group^ pg);
	void	load			(configs::lua_config_value& t, enum_id_action id_action);
	void	save			(configs::lua_config_value c, bool save_ids);

	void	clear			(bool b_destroy_object);
	void	fill_tree_view	(tree_node_collection^ nodes, u32& count);
	void	free_tree_view	(tree_node_collection^ nodes);
	void	render			();

	System::String^	name	();
	bool	get_selected	();
	void	set_selected	(bool b);
	void	refresh_ui		();
	System::String^	get_path		( );

	void	set_last_selected();

	project_group^			m_parent;
	object_base^			m_object_base;
	project^				m_project;
}; // class project_item

typedef System::Collections::ArrayList	items;

typedef System::Collections::ArrayList	groups;

public ref class project_group :	public project_tree_node_holder
{
public:
						project_group	(project^ p, project_group^ parent);
	void				load			(xray::configs::lua_config_value const& c, enum_id_action id_actions);
	void				save			(xray::configs::lua_config_value c);
	void				save			(xray::configs::lua_config_value c, System::String^ path, object_base^ object, bool save_ids);
	void				paste			(xray::configs::lua_config_value const& c, System::Collections::IDictionary^% path_to_id, System::String^% curr_path);

	void				clear			();
	void				fill_tree_view	(tree_node_collection^ nodes, u32& count);
	void				render			();

	project_item^		add_item		(object_base^ object);
	project_item^		add_item		(xray::configs::lua_config_value c, bool fillui, enum_id_action id_actions);
	project_group^		get_group		(System::String^ name);
	project_item^		get_item		(System::String^ name);
	project_item^		get_item		(u32 id);
	void				get_all_items	(object_list^% objects);
	void				remove			(project_group^ pg);
	void				remove			(project_item^ pi);
	void				remove			(xray::configs::lua_config_value const& cfg);
	bool				is_empty		();

	System::String^		get_path		();

	void				set_last_selected();

protected :
	bool				check_correct_existance(System::String^% name, u32 index);
	void				set_name_internal(System::String^ s);
public:
	property System::String^	name{
		System::String^		get	()					{return m_name;}
		void				set	(System::String^ s)	{set_name_internal(s);}
	}
	project_group^					m_parent;
	groups^							m_groups;
	items^							m_items;
	project^						m_project;

private:
	System::String^					m_name;
}; // class project_group


ref class treeitem_wrapper {
public:
						treeitem_wrapper	(System::Int32 t):type(t){}
	virtual void		on_mouse_down		()	{};
	
	System::Int32		type;
};

ref class project_group_treeitem_wrapper : public treeitem_wrapper
{
	typedef treeitem_wrapper super;
public:
							project_group_treeitem_wrapper			(project_group^ pg):super(0),m_project_group(pg){}
	virtual void			on_mouse_down				() override;
	project_group^			m_project_group;
};

ref class project_item_treeitem_wrapper : public treeitem_wrapper
{
	typedef treeitem_wrapper super;
public:
							project_item_treeitem_wrapper			(project_item^ pi):super(1),m_project_item(pi){}
	virtual void			on_mouse_down				() override;
	project_item^			m_project_item;
};

} // namespace editor
} // namespace xray

#endif // #ifndef PROJECT_ITEMS_H_INCLUDED
////////////////////////////////////////////////////////////////////////////
//	Created		: 14.12.2009
//	Author		: Evgeniy Obertyukh
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef FILE_TREE_VIEW_H_INCLUDED
#define FILE_TREE_VIEW_H_INCLUDED

#include "tree_view_event_args.h"

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Collections::ObjectModel;
using namespace System::Windows;
using namespace System::Windows::Forms;

namespace xray
{
	namespace editor
	{
		namespace controls
		{
			interface class		tree_view_source;
			ref class			tree_node;
			ref class			tree_view_search_panel;
			ref class			tree_view_filter_panel;

			public ref class tree_view : TreeView
			{

			#pragma region |   Events   |

			public:
				event EventHandler<tree_view_event_args^>^	item_create;
				event EventHandler<tree_view_event_args^>^	group_create;
				event EventHandler<tree_view_event_args^>^	item_remove;
				event EventHandler^							items_loaded;
				event EventHandler^							selected_items_changed;

			#pragma endregion

			#pragma region | Initialize |

			public:
				tree_view();

			#pragma endregion

			#pragma region |   Fields   |

			private:
				tree_view_source^				m_source;
				Forms::ContextMenuStrip^		m_context_menu;
				ToolStripMenuItem^				add_folder_menu_item;
				ToolStripMenuItem^				add_file_menu_item;
				ToolStripMenuItem^				remove_menu_item;

				Boolean							m_b_container_created;
				Boolean							m_b_right_clicked;

				Collections::Generic::List<TreeNode^>^		m_selected_nodes;
				TreeNode^									m_last_selected_node;
				TreeNode^									m_just_selected_node;

				tree_view_filter_panel^			m_filter_panel;
				tree_view_search_panel^			m_search_panel;

			public:
				Panel^							m_tree_view_container;

			#pragma endregion

			#pragma region | Properties |

			public:
				[BrowsableAttribute(false)]
				property tree_view_source^ source
				{
					tree_view_source^	get	();
					void					set	(tree_view_source^ value);
				}

				property Windows::Forms::ContextMenuStrip^	nodes_context_menu
				{
					Windows::Forms::ContextMenuStrip^		get	();
					void									set	(Windows::Forms::ContextMenuStrip^ value);
				}
				[BrowsableAttribute(false)]
				[DesignerSerializationVisibility(DesignerSerializationVisibility::Hidden)]
				property TreeNode^				root;				
				property Boolean				auto_expand_on_filter
				{
					Boolean		get();
					void		set(Boolean value);
				}
				property Boolean				is_multiselect;
				property Boolean				is_selectable_groups;
				[BrowsableAttribute(false)]
				property ReadOnlyCollection<TreeNode^>^		selected_nodes{
					ReadOnlyCollection<TreeNode^>^ get(){return m_selected_nodes->AsReadOnly();}
				}

 				property Boolean				filter_visible{
 					Boolean				get();
 					void				set(Boolean value);
 				}

				property TreeNodeCollection^	nodes
				{
					TreeNodeCollection^ get(){return TreeView::Nodes;}
				}

			#pragma endregion

			#pragma region |  Methods   |

			private:
				void			select_node				(TreeNode^ node);
				void			select_sub_nodes		(TreeNode^ node);
				void			select_all				();
				void			deselect_node			(TreeNode^ node);
				void			deselect_nodes			(Collections::Generic::List<TreeNode^>^ node);

			protected:
				virtual	void	OnKeyDown				(KeyEventArgs^  e) override;
				virtual void	OnKeyUp					(KeyEventArgs^  e) override;
				virtual void	OnMouseDown				(MouseEventArgs^ e) override;
				virtual void	OnMouseUp				(MouseEventArgs^ e) override;
				virtual void	OnNodeMouseClick		(TreeNodeMouseClickEventArgs^  e) override;
				virtual void	OnAfterCollapse			(TreeViewEventArgs^  e) override;
				virtual void	OnAfterExpand			(TreeViewEventArgs^  e) override;
				virtual void	OnBeforeSelect			(TreeViewCancelEventArgs^ e) override;
				virtual void	OnAfterSelect			(TreeViewEventArgs^ e) override;
				virtual void	OnParentChanged			(EventArgs^ e) override;

			internal:
				TreeNode^		process_item_path		(array<String^>^ segments);
				TreeNode^		process_item_path		(array<String^>^ segments, Boolean create_path);
				TreeNode^		process_item_path		(array<String^>^ segments, Boolean create_path, Int32 image_index_collapsed, Int32 image_index_expanded);
				TreeNode^		process_item_path		(String^ path, String^ separator);
				TreeNode^		process_item_path		(String^ path);

			public:
				void			track_active_node		(String^ full_path);
				tree_node^		get_node				(String^ full_path);
				void			refresh					();
				tree_node^		add_item				(String^ item_path);
				tree_node^		add_item				(String^ item_path, Int32 image_index);
				tree_node^		add_group				(String^ group_path);
				tree_node^		add_group				(String^ group_path, Int32 image_index_collapsed, Int32 image_index_expanded);
				void			remove_item				(String^ item_path);
				void			remove_group			(String^ group_path);
				void			change_item_context		(String^ file_path, Object^ context);
				void			create_item				(System::Object^ sender, System::EventArgs^ e);
				void			create_group			(System::Object^ sender, System::EventArgs^ e);
				void			remove_item				(System::Object^ sender, System::EventArgs^ e);
				void			on_items_loaded			();

				void			add_items				(Generic::IList<Object^>^ items);
				void			add_items				(array<Object^>^ items);

			#pragma endregion

			}; // class ex_tree_view
		}//namespace controls
	}//namespace editor
}//namespace xray

#endif // #ifndef FILE_TREE_VIEW_H_INCLUDED
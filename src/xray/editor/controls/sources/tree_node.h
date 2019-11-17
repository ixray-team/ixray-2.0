////////////////////////////////////////////////////////////////////////////
//	Created		: 29.01.2010
//	Author		: Evgeniy Obertyukh
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef TREE_NODE_H_INCLUDED
#define TREE_NODE_H_INCLUDED

using namespace System;
using namespace System::Windows::Forms;

namespace xray
{
	namespace editor
	{
		namespace controls
		{
			public enum class tree_node_type
			{
				single_item,
				group_item
			};

			public ref class tree_node : public TreeNode
			{
			public:
				tree_node(){}
				tree_node(String^ name):TreeNode(name){Name = name;}
				tree_node(String^ name, Int32 image_index, Int32 selected_image_index):
					TreeNode(name, image_index, selected_image_index){Name = name;}
				tree_node(String^ name, Int32 image_index, Int32 selected_image_index, array<TreeNode^>^ children):
					TreeNode(name, image_index, selected_image_index, children){Name = name;}
				tree_node(String^ name, array<TreeNode^>^ children):
					TreeNode(name, children){Name = name;}

			public:
				Boolean					m_is_selected;
				Drawing::Color			m_fore_color;
				Drawing::Color			m_back_color;

				tree_node_type			m_node_type;
				Int32					m_image_index_collapsed;
				Int32					m_image_index_expanded;

			public:
				property tree_node^	Parent
				{
					virtual tree_node^		get() new {return safe_cast<tree_node^>(TreeNode::Parent);}
				}
				property TreeNodeCollection^ nodes
				{
					TreeNodeCollection^		get(){return TreeNode::Nodes;}
				}
			public:
				void			Nodes					(){}
				tree_node^		add_node_single			(String^ name);
				tree_node^		add_node_single			(String^ name, Int32 image_index);
				tree_node^		add_node_group			(String^ name);
				tree_node^		add_node_group			(String^ name, Int32 image_index_expanded, Int32 image_index_collapsed);

			}; // class ex_tree_node
		}//namespace controls
	}//namespace editor
}//namespace xray
#endif // #ifndef EX_TREE_NODE_H_INCLUDED
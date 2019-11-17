////////////////////////////////////////////////////////////////////////////
//	Created		: 17.03.2009
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef PROJECT_DEFINES_H_INCLUDED
#define PROJECT_DEFINES_H_INCLUDED

namespace xray {
namespace editor {

typedef System::Windows::Forms::TreeNode			tree_node;
typedef System::Windows::Forms::TreeNodeCollection	tree_node_collection;

enum	enum_selection_method{ 
		enum_selection_method_set,
		enum_selection_method_add, 
		enum_selection_method_invert, 
		enum_selection_method_subtract, 
		enum_selection_method_NONE };

enum	enum_id_action{ 
		enum_id_action_save = 0x01,
		enum_id_action_load = 0x02, 
		enum_id_action_NONE = 0x0 };

ref class project;
ref class object_base;
ref class project_item;
ref class project_group;
ref class tool_base;

typedef System::Collections::Generic::List<object_base^>			object_list;
typedef System::Collections::Generic::List<u32>						id_list;
typedef System::Collections::Generic::Dictionary<long, u32>			id_id_dict;

public value struct id_matrix
{
	u32				id;
	math::float4x4*	matrix;
};	

typedef System::Collections::Generic::List<id_matrix>		id_matrices_list;


} // namespace editor
} // namespace xray

#endif // #ifndef PROJECT_DEFINES_H_INCLUDED
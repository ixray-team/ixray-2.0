////////////////////////////////////////////////////////////////////////////
//	Created		: 12.03.2009
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef OBJECT_H_INCLUDED
#define OBJECT_H_INCLUDED

#include "project_defines.h"
#include "attribute.h"

#pragma managed(push,off)
#	include <xray/collision/common_types.h>
#pragma managed(pop)

namespace xray {
namespace collision {
	struct geometry;
} // namespace collision

namespace render {
	namespace editor { struct renderer; }
} // namespace render

namespace editor {

class collision_object;
ref class float_curve;
ref class color_curve;
class property_holder;

using System::ComponentModel::DisplayNameAttribute;
using System::ComponentModel::DescriptionAttribute;
using System::ComponentModel::CategoryAttribute;
using System::ComponentModel::DefaultValueAttribute;
using System::ComponentModel::ReadOnlyAttribute;
using System::ComponentModel::MergablePropertyAttribute;

ref struct object_collision;
ref class object_base;
ref class level_editor;

public delegate void collision_initialized (object_collision^ collision);

public ref struct object_collision
{
private:
	collision::geometry*			m_user_geometry;
	collision_object*				m_collision_obj;
	level_editor^					m_level_editor;
	bool							m_b_active;
public:
									object_collision	(xray::editor::level_editor^ le);
	void							create_cube			(object_base^ o, float3 extents);
	void							create_mesh			(object_base^ o, float3 const * vertices, u32 vertex_count, u32 const* indices, u32 index_count, u32 const* triangle_data, u32 triangle_data_count, xray::collision::object_type t);
	void							create_from_geometry(object_base^ o, collision::geometry const*, xray::collision::object_type);

	collision::geometry const*		get_geometry		();
	aabb							get_aabb			();
	void							destroy				();
	void							insert				();
	void							remove				();
	void							set_matrix			(float4x4 const* m );
	bool							initialized			();

	event collision_initialized^	on_collision_initialized;
};

public enum class enum_terrain_interaction : int {tr_free=0, tr_vertical_snap=1, tr_physical_snap=2 };

public ref class object_base 
{
public:
									object_base			(tool_base^ t);
	virtual							~object_base		();
	virtual void					load				(xray::configs::lua_config_value const& t);
	virtual void					save				(xray::configs::lua_config_value t);
	virtual void					load_defaults		() {}
			tool_base^				owner				()								{return m_owner_tool;}
			
			System::String^			get_name			();
			void					set_name			(System::String^ name, bool refresh_ui /*= true*/);
			void					set_name_revertible	(System::String^ name);

			System::String^			get_lib_name		() 								{return m_lib_name;}
			void					set_lib_name		(System::String^ name);

			bool					get_selected		() 							{return m_selected;}
	virtual void					set_selected		(bool selected);
	virtual bool					get_selectable		()							{return true;}
			
			bool					get_visible			()							{return m_visible;}
	virtual	void					set_visible			(bool bvisible)				{m_visible = bvisible;}
			
			u32						id					() 							{return m_id;}

	virtual void					render				()	{}

			property_holder*		get_property_holder	();
	
			virtual enum_terrain_interaction get_terrain_interaction()				{return enum_terrain_interaction::tr_free;}
			float4x4 const&			get_transform		() 							{return *m_transform;}
	virtual	void					set_transform		(float4x4 const& transform);

			aabb					aabb				();
			float3					get_position		()							{return (*m_transform).c.xyz();}
			void					set_position		(float3 p);
			void					set_position_revertible(math::float3 p);
			math::float3			get_rotation		()							{return (*m_transform).get_angles_xyz_INCORRECT();}
			void					set_rotation		(math::float3 p);
			void					set_rotation_revertible(math::float3 p);
			math::float3			get_scale			();
			void					set_scale			(math::float3 p);
			void					set_scale_revertible(math::float3 p);
			
public:
			void					assign_id			(u32 id);
	static 	object_base^			object_by_id		(u32 id);

	xray::render::editor::renderer&	get_editor_renderer ();
private:
	u32 							generate_id			();
	static	System::Collections::Generic::SortedDictionary<u32,object_base^>	s_id_to_object;
	static	u32						m_last_generated_id = 0;

protected:
	void							on_property_changed	(System::String^ prop_name);
	bool							is_slave_attribute	(System::String^ prop_name);

	tool_base^						m_owner_tool;
	u32								m_id;
	System::String^					m_name;
	System::String^					m_lib_name;
	float4x4*						m_transform;
	math::aabb*						m_aabbox;
	property_holder*				m_property_holder;
	bool							m_selected;
	bool							m_visible;
public:
	object_collision				m_collision;

	[DisplayNameAttribute("position"), DescriptionAttribute("object position"), CategoryAttribute("general"), 
	ValueAttribute(e_def_val, 0.0f, 0.0f, 0.0f), ReadOnlyAttribute(false),ConnectionAttribute(enum_connection::inout)]
	property float3					object_position{
		float3						get()					{return get_position();}
		void						set(float3 p)			{set_position_revertible(p);}
	}

	[DisplayNameAttribute("rotation"), DescriptionAttribute("object rotation"), CategoryAttribute("general"), 
	ValueAttribute(e_def_val, 0.0f, 0.0f, 0.0f), ReadOnlyAttribute(false),ConnectionAttribute(enum_connection::inout)]
	property float3					object_rotation{
		float3						get()					{return get_rotation();}
		void						set(float3 p)			{set_rotation_revertible(p);}
	}

	[DisplayNameAttribute("scale"), DescriptionAttribute("object scale"), CategoryAttribute("general"), 
		ValueAttribute(e_def_val, 1.0f, 1.0f, 1.0f), ReadOnlyAttribute(false),ConnectionAttribute(enum_connection::inout)]
	property float3					object_scale{
		float3						get()					{return get_scale();}
		void						set(float3 p)			{set_scale_revertible(p);}
	}

	[DisplayNameAttribute("name"), DescriptionAttribute("object name"), CategoryAttribute("general"), 
	ValueAttribute(e_def_val, ""), ReadOnlyAttribute(false), MergablePropertyAttribute(false)]
	property System::String^		object_name{
		System::String^ 			get()					{return get_name();}
		void						set(System::String^ n)	{set_name_revertible(n);on_property_changed("name");}
	}

	[DisplayNameAttribute("library name"), DescriptionAttribute("library reference name"), CategoryAttribute("general"), 
	ValueAttribute(e_def_val, ""), ReadOnlyAttribute(true)]
	property System::String^		library_name{
		System::String^ 			get()					{return get_lib_name();}
		void						set(System::String^ n)	{set_lib_name(n);on_property_changed("library name");}
	}

	[DisplayNameAttribute("id"), DescriptionAttribute("object identifier"), CategoryAttribute("general"), 
	ValueAttribute(e_def_val, 0), ValueAttribute(e_min_val, 0), ValueAttribute(e_max_val, 100000), 
	ReadOnlyAttribute(true), MergablePropertyAttribute(false)]
	property int					object_id{
		int							get()					{return id();}
		void						set(int)				{}
	}

	//[DisplayNameAttribute("fc"), DescriptionAttribute("object fc"), CategoryAttribute("general"), 
	//ReadOnlyAttribute(false), MergablePropertyAttribute(false)]
	//property xray::editor::float_curve^ fc;

	//[DisplayNameAttribute("cc"), DescriptionAttribute("object cc"), CategoryAttribute("general"), 
	//ReadOnlyAttribute(false), MergablePropertyAttribute(false)]
	//property xray::editor::color_curve^ cc;
	
	[ReadOnlyAttribute(true)]
	property System::String^	image_key;
public:
	project_item^					m_owner_project_item;
}; // class object_base


void initialize_property_holder	(object_base^ o);
math::float4x4 place_object		(object_base^ o, float3 const& position);

} // namespace editor
} // namespace xray

#endif // #ifndef OBJECT_H_INCLUDED
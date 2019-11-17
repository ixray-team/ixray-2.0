////////////////////////////////////////////////////////////////////////////
//	Created		: 17.03.2009
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef OBJECT_LIGHT_H_INCLUDED
#define OBJECT_LIGHT_H_INCLUDED

#include "object_base.h"
#include <xray/render/base/world.h>
#include <xray/render/base/light_props.h>

namespace xray {

namespace render{
	class light_props;

} // namespace render
namespace editor {

ref class tool_light;

ref class object_light :public object_base
{
	typedef object_base		super;
public:
							object_light			(tool_light^ t);
	virtual					~object_light			();
	virtual void			load					(xray::configs::lua_config_value const& t) override;
	virtual void			save					(xray::configs::lua_config_value t) override;
	virtual void			render					() override;
	virtual void			load_defaults			() override;
	virtual void			set_visible				(bool bvisible) override;
	virtual void			set_transform			(xray::math::float4x4 const& transform) override;

protected:
			void			initialize_collision	();
			void			fill_props				(xray::render::light_props& props);
			void			set_range_impl			(float range);
			void			set_color_impl			(color const& color);
			void			set_cast_shadow_impl	(bool cast_shadow);


			tool_light^		m_tool_light;
			color*			m_color;
			float			m_brightnes;
			float			m_range;
			bool			m_b_cast_shadow;
			u32				m_type;
//	render::light_ptr	m_render_light; implement this
public:
	[DisplayNameAttribute("color"), DescriptionAttribute("light color"), CategoryAttribute("general"), 
	ValueAttribute(e_def_val, 1.0f, 1.0f, 1.0f, 1.0f), ValueAttribute(e_min_val, 0.0f, 0.0f, 0.0f, 0.0f), ValueAttribute(e_max_val, 1.0f, 1.0f, 1.0f, 1.0f), 
	ReadOnlyAttribute(false),ConnectionAttribute(enum_connection::inout)]
	property math::color	color{
		math::color			get		()						{return *m_color;}
		void				set		(math::color c)			{set_color_impl(c); on_property_changed("color");}
	}

	[DisplayNameAttribute("brightness"), DescriptionAttribute("light brightness"), CategoryAttribute("general"), 
	ValueAttribute(e_def_val, 1.0f), ValueAttribute(e_min_val, 0.0f), ValueAttribute(e_max_val, 1.0f), 
	ReadOnlyAttribute(false),ConnectionAttribute(enum_connection::inout)]
	property float			brightness{
		float				get		()						{return m_brightnes;}
		void				set		(float v)				{m_brightnes=v;on_property_changed("brightness");}
	}

	[DisplayNameAttribute("range"), DescriptionAttribute("light range"), CategoryAttribute("general"), 
	ValueAttribute(e_def_val, 1.0f), ValueAttribute(e_min_val, 0.1f), ValueAttribute(e_max_val, 100.0f), 
	ReadOnlyAttribute(false),ConnectionAttribute(enum_connection::inout)]
	property float			range{
		float				get		()						{return m_range;}
		void				set		(float v)				{set_range_impl(v) ;on_property_changed("range");}
	}

	[DisplayNameAttribute("cast shadow"), DescriptionAttribute("cast shadow"), CategoryAttribute("general"), 
	ValueAttribute(e_def_val, true), ReadOnlyAttribute(false), ConnectionAttribute(enum_connection::inout)]
	property bool	cast_shadow{
		bool				get		()						{return m_b_cast_shadow;}
		void				set		(bool v)				{set_cast_shadow_impl(v);on_property_changed("cast shadow");}
	}
}; // class object_light


ref class object_dynamic_light :public object_light
{
	typedef object_light		super;
public:
							object_dynamic_light	(tool_light^ t);
	virtual					~object_dynamic_light	();
	virtual void			load					(xray::configs::lua_config_value const& t) override;
	virtual void			save					(xray::configs::lua_config_value t) override;
	virtual void			load_defaults			() override;
protected:
			xray::render::light_type	m_light_type;
			float						m_cone;
			bool						m_b_volumetric;
	System::String^						m_texture;
public:
	[DisplayNameAttribute("is volumetric"),	DescriptionAttribute("is volumetric"), CategoryAttribute("general"), 
	ValueAttribute(e_def_val, false), ReadOnlyAttribute(false), ConnectionAttribute(enum_connection::inout)]
	property bool			volumetric{
		bool				get		()					{return m_b_volumetric;}
		void				set		(bool v)			{m_b_volumetric=v;on_property_changed("is volumetric");}
	}

	[DisplayNameAttribute("texture"), DescriptionAttribute("texture name"), CategoryAttribute("general"), 
	ValueAttribute(e_def_val, ""), ReadOnlyAttribute(false)]
	property System::String^	texture_name{
		System::String^			get	()					{return m_texture;}
		void					set	(System::String^ n)	{m_texture = n;on_property_changed("texture");}
	}

};

ref class object_static_light : public object_light
{
	typedef object_light	super;
public:
							object_static_light		(tool_light^ t);
	virtual					~object_static_light	();
	virtual void			load					(xray::configs::lua_config_value const& t) override;
	virtual void			save					(xray::configs::lua_config_value t) override;
	virtual void			load_defaults			() override;
protected:
			bool			m_b_lightmap;
public:
	[DisplayNameAttribute("lightmap"), DescriptionAttribute("generate lightmap"), CategoryAttribute("general"), 
	ValueAttribute(e_def_val, true), ReadOnlyAttribute(false)]
	property bool			lightmap{
		bool				get		()					{return m_b_lightmap;}
		void				set		(bool v)			{m_b_lightmap=v;on_property_changed("lightmap");}
	}
};

} // namespace editor
} // namespace xray

#endif // #ifndef OBJECT_LIGHT_H_INCLUDED
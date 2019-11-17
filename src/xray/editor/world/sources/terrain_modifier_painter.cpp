////////////////////////////////////////////////////////////////////////////
//	Created		: 03.02.2010
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "terrain_modifier_painter.h"
#include "level_editor.h"
#include "tool_terrain.h"
#include "project.h"
#include "terrain_object.h"
#include "terrain_modifier_command.h"
#include <FreeImage/FreeImage.h>
#include <xray/editor/base/managed_delegate.h>

namespace xray {
namespace editor {

terrain_modifier_painter_base::terrain_modifier_painter_base(level_editor^ le, tool_terrain^ tool)
:super(le, tool)
{
	m_timer					= NEW(xray::timing::timer);
}

terrain_modifier_painter_base::~terrain_modifier_painter_base()
{
	DELETE					(m_timer);
}

void terrain_modifier_painter_base::on_working_set_changed()
{
	super::on_working_set_changed();

	for each (terrain_node_key key in m_active_id_list.Keys)
	{
		if(!m_stored_values_list.ContainsKey(key))
			m_stored_values_list.Add	(key, gcnew vertex_dict);
	}
}

void terrain_modifier_painter_base::end_input()
{
	for each (terrain_node_key key in m_processed_id_list.Keys)
	{
		if(m_stored_values_list[key]->Count != 0)
			(safe_cast<terrain_modifier_color_command^>(m_apply_command))->set_data(key, m_stored_values_list[key]);
	}

	super::end_input				();
	m_stored_values_list.Clear		();
}

void terrain_modifier_painter_base::start_input()
{
	super::start_input		();

	m_level_editor->ide()->get_mouse_position(m_last_screen_xy);

	refresh_active_working_set(shape_type);
}

void terrain_modifier_painter_base::execute_input()
{
	System::Drawing::Point screen_xy;
	m_level_editor->ide()->get_mouse_position(screen_xy);

	if(screen_xy != m_last_screen_xy)
		refresh_active_working_set	(shape_type);

	m_last_screen_xy			= screen_xy;

	do_work							();
}

static u32	color_get_R	(u32 rgba)	{return (((rgba) >> 16) & 0xff);	}
static u32	color_get_G	(u32 rgba)	{return (((rgba) >> 8) & 0xff);	}
static u32	color_get_B	(u32 rgba)	{return ((rgba) & 0xff);			}
static u32	color_get_A (u32 rgba)	{return ((rgba) >> 24);			}
u32	color_argb	(u8 a, u8 r, u8 g, u8 b)		{	return (a<<24)|(r<<16)|(g<<8)|(b);}
u32	color_argb	(int a, int r, int g, int b)	{	return ((a&0xff)<<24)|((r&0xff)<<16)|((g&0xff)<<8)|(b&0xff);}

u32 change_diffuse_influence(u32 src_color, u32 infl_color, float influence)
{
	int r  	= color_get_R(src_color);
	int g  	= color_get_G(src_color);
	int b  	= color_get_B(src_color);
	int a  	= color_get_A(src_color);

	int r0  	= color_get_R(infl_color);
	int g0  	= color_get_G(infl_color);
	int b0  	= color_get_B(infl_color);
	int a0  	= color_get_A(infl_color);

	float invt = 1.f-influence;

	r = xray::math::floor(r*invt + r0*influence);
	g = xray::math::floor(g*invt + g0*influence);
	b = xray::math::floor(b*invt + b0*influence);
	a = xray::math::floor(a*invt + a0*influence);

	xray::math::clamp(r, 0, 255);
	xray::math::clamp(g, 0, 255);
	xray::math::clamp(b, 0, 255);
	xray::math::clamp(a, 0, 255);

	return color_argb(a, r, g, b);
}

void terrain_modifier_painter_base::create_command()
{
	m_apply_command					= gcnew terrain_modifier_color_command(this, m_tool_terrain->get_terrain_core());
}

void terrain_modifier_painter_base::load_settings(RegistryKey^ key)
{
	super::load_settings		(key);
}

void terrain_modifier_painter_base::save_settings(RegistryKey^ key)
{
	super::save_settings		(key);
}

// detail_painter

terrain_modifier_detail_painter::terrain_modifier_detail_painter(level_editor^ le, tool_terrain^ tool)
:super(le, tool),
m_diffuse_texture(NULL),
m_active_texture(nullptr)
{
	m_active_texture_options = NEW(texture_options_ptr)();
	m_control_id			= "terrain_modifier_detail_painter";
	mode					= painter_mode::both;
	m_min_strength			= 0.01f;
	m_max_strength			= 20.0f;
}

terrain_modifier_detail_painter::~terrain_modifier_detail_painter()
{
	DELETE					(m_active_texture_options);
	FreeImage_Unload		(m_diffuse_texture);
}

void terrain_modifier_detail_painter::active_texture_options_loaded(xray::resources::queries_result& data)
{
	*m_active_texture_options	= static_cast_checked<texture_options*>(data[0].get_unmanaged_resource().c_ptr());
	set_diffuse_texture			(gcnew System::String((*m_active_texture_options)->color_name.c_str()));

}

void terrain_modifier_detail_painter::on_working_set_changed()
{
//	ASSERT	(active_texture); // active_texture must be selected

	super::on_working_set_changed();

	if(active_texture)
	{
	for each (terrain_node_key key in m_active_id_list.Keys)
	{
		terrain_node^ curr_terrain		= m_tool_terrain->get_terrain_core()->m_nodes[key];

		if(!curr_terrain->m_used_textures.Contains(active_texture))
			curr_terrain->add_used_texture(active_texture);

		if(!m_paint_layers.ContainsKey(key))
		{
			float4x4 inv_transform;
			inv_transform.try_invert		(curr_terrain->get_transform());
			float3 center_position_local	= inv_transform.transform_position(*m_center_position_global);
			int layer_id					= curr_terrain->get_layer_for_paint(active_texture, center_position_local);
			ASSERT							(layer_id!=-1);
			m_paint_layers.Add				(key, layer_id);
		}
	}
	}
}

void terrain_modifier_detail_painter::end_input()
{
	super::end_input			();

	m_paint_layers.Clear			();
}

void add_color(int& src_dst, int const value)
{
	if(value>0)
		src_dst = (src_dst+value >255) ? 255 : (src_dst+value);
	else
		src_dst = (src_dst+value <0) ? 0 : (src_dst+value);
}

bool change_layer_influence(terrain_vertex% vert, int const layer_id, int const texture_id, float influence)
{
	ASSERT(layer_id>=0 && layer_id<3);

	xray::math::clamp(influence, -1.0f, 1.0f);

	int infl		= xray::math::floor(influence*255);
	if(infl==0)
		return false;
	u8 tid		= texture_id & 0xff;

	int t0  	= vert.t0_infl;
	int t1  	= vert.t1_infl;
	int t2  	= vert.t2_infl;

	int* p_add	= NULL;	
	int* p_sub1	= NULL;
	int* p_sub2	= NULL;

	if(layer_id==0) // t0	
	{
		p_add	= &t0;
		p_sub1	= &t1;
		p_sub2	= &t2;
		vert.t0_index = tid;
	}else
	if(layer_id==1) // t1
	{
		p_add	= &t1;
		p_sub1	= &t0;
		p_sub2	= &t2;
		vert.t1_index = tid;
	}else
	if(layer_id==2) // t2
	{
		p_add	= &t2;
		p_sub1	= &t0;
		p_sub2	= &t1;
		vert.t2_index = tid;
	}

	add_color(*p_add, infl);
	
	int diff1		= 0;
	int diff2		= 0;
	if(*p_sub1)
		diff1		= (*p_sub1/(*p_sub1 + *p_sub2))*infl;

	if(*p_sub2)
		diff2		= (*p_sub2/(*p_sub1 + *p_sub2))*infl;

	if(diff1)
		add_color(*p_sub1, -diff1);

	if(diff2)
		add_color(*p_sub2, -diff2);

	int sum = t0+t1+t2;
	if(sum > 255) // normalize
	{
		t0	= (int)(t0*255.0f/sum);
		t1	= (int)(t1*255.0f/sum);
		t2	= (int)(t2*255.0f/sum);
	}
	xray::math::clamp(t1, 0, 255);
	xray::math::clamp(t2, 0, 255);
	xray::math::clamp(t0, 0, 255);

	vert.t0_infl	= (t0 & 0xff);
	vert.t1_infl	= (t1 & 0xff);
	vert.t2_infl	= (t2 & 0xff);

	return true;
}


void terrain_modifier_detail_painter::do_work()
{
	if(active_texture==nullptr) // active_texture must be selected
		return;

	float global_influence		= strength;

	float const time			= m_timer->get_elapsed_sec();

	global_influence			*= time; // strength = value per 1 second
	
	
	if(m_level_editor->ide()->get_held_keys()->Contains(System::Windows::Forms::Keys::ControlKey))
		global_influence		*= -1.0f;

	m_timer->start				();

	float4x4 inv_transform;
	for each (terrain_node_key key in m_active_id_list.Keys)
	{
		vert_id_list^ curr_processed_id_list	= m_processed_id_list[key];
		terrain_node^ curr_terrain				= m_tool_terrain->get_terrain_core()->m_nodes[key];
		ASSERT(m_stored_values_list.ContainsKey(key));

		vertex_dict^  curr_stored_values_list	= m_stored_values_list[key];

		ASSERT(m_paint_layers.ContainsKey(key));
		int curr_vertex_layer					= m_paint_layers[key];
		int curr_texture_id						= curr_terrain->m_used_textures.IndexOf(active_texture);

		ASSERT	(curr_vertex_layer!=-1);
		ASSERT	(curr_texture_id!=-1);

		inv_transform.try_invert		(curr_terrain->get_transform());
		float3 center_position_local	= inv_transform.transform_position(*m_center_position_global);

		for each (u16 vert_idx in m_active_id_list[key])
		{
			terrain_vertex% v					= curr_terrain->m_vertices[vert_idx];

			float const distance				= curr_terrain->distance_xz(vert_idx, center_position_local);

			ASSERT								(distance<=radius);
			float influence						= calc_influence_factor(distance);
			influence							*= global_influence;

			if(!curr_processed_id_list->Contains(vert_idx))
			{
				curr_processed_id_list->Add		(vert_idx);
				curr_stored_values_list->Add	(vert_idx, v);
			}

			if(mode==painter_mode::texture || mode==painter_mode::both)
			{
				change_layer_influence(v, curr_vertex_layer, curr_texture_id,  influence);
			}

			if(m_diffuse_texture && (mode==painter_mode::diffuse || mode==painter_mode::both) )
			{
				u32 current_color	= get_diffuse_color(curr_terrain->position(vert_idx));

				u32 c				= change_diffuse_influence(v.vertex_color, current_color, influence);
				v.vertex_color		= c;
			}
			curr_terrain->m_vertices[vert_idx] = v;   
		} // verts

		terrain_core^ core						= m_tool_terrain->get_terrain_core();
		core->sync_visual_vertices	(core->m_nodes[key], m_active_id_list[key]);
	} // keys
}

u32 terrain_modifier_detail_painter::get_diffuse_color(float3 local_position)
{
	ASSERT					(m_diffuse_texture);
	RGBQUAD					rgb_quad;

	// calc tile
	float	pxm = local_position.x - ((int)local_position.x/m_diffuse_size_meters)*m_diffuse_size_meters; // rem
	float	pym = -local_position.z - ((int)-local_position.z/m_diffuse_size_meters)*m_diffuse_size_meters; // rem

	int px = math::floor( pxm * (float)m_diffuse_size_pixels / m_diffuse_size_meters );
	int py = math::floor( pym * (float)m_diffuse_size_pixels / m_diffuse_size_meters + math::epsilon_5);

	py = m_diffuse_size_pixels-1-py;

	FreeImage_GetPixelColor(m_diffuse_texture, px, py, &rgb_quad); // v flip
	return color_argb((u8)255, (u8)rgb_quad.rgbRed, (u8)rgb_quad.rgbGreen, (u8)rgb_quad.rgbBlue);
}

void terrain_modifier_detail_painter::load_settings(RegistryKey^ key)
{
	super::load_settings		(key);

	RegistryKey^ self_key		= get_sub_key(key, name);

	mode						= (painter_mode)(Enum::Parse(painter_mode::typeid, self_key->GetValue("mode", painter_mode::both)->ToString()));
//.. active index
	self_key->Close				();
}

void terrain_modifier_detail_painter::save_settings(RegistryKey^ key)
{
	super::save_settings		(key);

	RegistryKey^ self_key		= get_sub_key(key, name);

	self_key->SetValue			("mode", mode);

//.. active index
	self_key->Close				();
}

void terrain_modifier_detail_painter::set_active_texture_impl(System::String^ value)
{
	m_active_texture	= value;

	if(m_active_texture)
	{
		query_result_delegate* q		= NEW(query_result_delegate)(gcnew query_result_delegate::Delegate(this, &terrain_modifier_detail_painter::active_texture_options_loaded));

		resources::query_resource(	unmanaged_string(value).c_str(),
			xray::resources::texture_options_class,
			boost::bind(&query_result_delegate::callback, q, _1),
			g_allocator);
	}else
		set_diffuse_texture("");

	on_property_changed	(this);
}

void terrain_modifier_detail_painter::set_diffuse_texture(System::String^ fname)
{
	ASSERT(fname);
	if(m_diffuse_texture)
	{
		FreeImage_Unload	(m_diffuse_texture);
		m_diffuse_texture	= NULL;
	}

	if(fname->Length)
	{
		// temporary!!!! use texture cooker instead
		fs::path_string	full_filename = "../../resources/textures/";
		
		full_filename.append	(unmanaged_string(fname).c_str());
		full_filename.append	(".dds");
		FREE_IMAGE_FORMAT	fif = FreeImage_GetFIFFromFilename(full_filename.c_str());
		m_diffuse_texture		= FreeImage_Load(fif, full_filename.c_str());
		ASSERT					(m_diffuse_texture, "%s not exist", full_filename.c_str());
		m_diffuse_size_pixels	= FreeImage_GetWidth(m_diffuse_texture);
		ASSERT					(FreeImage_GetWidth(m_diffuse_texture) == FreeImage_GetHeight(m_diffuse_texture));
		m_diffuse_size_meters	= 64;
	}
}

// diffuse painter
terrain_modifier_diffuse_painter::terrain_modifier_diffuse_painter(level_editor^ le, tool_terrain^ tool)
:super(le, tool)
{
	m_control_id			= "terrain_modifier_diffuse_painter";
	m_min_strength			= 0.01f;
	m_max_strength			= 5.0f;
}

terrain_modifier_diffuse_painter::~terrain_modifier_diffuse_painter()
{}

void terrain_modifier_diffuse_painter::do_work()
{
	float global_influence		= strength;

	float const time			= m_timer->get_elapsed_sec();

	global_influence			*= time; // strength = value per 1 second
	
	m_timer->start				();

	float4x4 inv_transform;
	for each (terrain_node_key key in m_active_id_list.Keys)
	{
		vert_id_list^ curr_processed_id_list	= m_processed_id_list[key];
		terrain_node^ curr_terrain				= m_tool_terrain->get_terrain_core()->m_nodes[key];
		vertex_dict^  curr_stored_values_list	= m_stored_values_list[key];

		inv_transform.try_invert		(curr_terrain->get_transform());
		float3 center_position_local	= inv_transform.transform_position(*m_center_position_global);

		for each (u16 vert_idx in m_active_id_list[key])
		{
			terrain_vertex% v					= curr_terrain->m_vertices[vert_idx];

			float const distance				= curr_terrain->distance_xz(vert_idx, center_position_local);

			ASSERT								(distance<=radius);
			float influence						= calc_influence_factor(distance);
			influence							*= global_influence;

			if(!curr_processed_id_list->Contains(vert_idx))
			{
				curr_processed_id_list->Add		(vert_idx);
				curr_stored_values_list->Add	(vert_idx, v);
			}

			u32 current_color	= get_diffuse_color(curr_terrain->position(vert_idx));
			u32 c				= change_diffuse_influence(v.vertex_color, current_color, influence);
			v.vertex_color		= c;
			curr_terrain->m_vertices[vert_idx] = v;
		} // verts

		terrain_core^ core			= m_tool_terrain->get_terrain_core();
		core->sync_visual_vertices	(core->m_nodes[key], m_active_id_list[key]);
	} // keys
}

void terrain_modifier_diffuse_painter::load_settings(RegistryKey^ key)
{
	super::load_settings(key);
	RegistryKey^ self_key		= get_sub_key(key, name);

	u32 argb					= System::Convert::ToInt32(self_key->GetValue("color", color_argb(0,0,0,0)));
	paint_color					= System::Drawing::Color::FromArgb(argb);
	
	self_key->Close				();
}

void terrain_modifier_diffuse_painter::save_settings(RegistryKey^ key)
{
	super::save_settings(key);

	RegistryKey^ self_key		= get_sub_key(key, name);
	self_key->SetValue			("color", paint_color.ToArgb());
	self_key->Close				();
}

u32	terrain_modifier_diffuse_painter::get_diffuse_color(float3 local_position)
{
	XRAY_UNREFERENCED_PARAMETER(local_position);
	return paint_color.ToArgb();
}


} // namespace editor
} // namespace xray
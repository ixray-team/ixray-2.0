////////////////////////////////////////////////////////////////////////////
//	Created		: 24.03.2009
//	Author		: Mykhailo Parfeniuk
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include <xray/render/engine/lights_db.h>

namespace xray {
namespace render_dx10 {

// This is temporary enum to support old light formats 
typedef enum TMP_D3DLIGHTTYPE {
	TMP_D3DLIGHT_POINT          = 1,
	TMP_D3DLIGHT_SPOT           = 2,
	TMP_D3DLIGHT_DIRECTIONAL    = 3,
	TMP_D3DLIGHT_FORCE_DWORD    = 0x7fffffff, /* force 32-bit size enum */
} TMP_D3DLIGHTTYPE;

enum
{
	chunk_light_dynamic = 6
};

struct f_light 
{
public:
    u32					type;						/* Type of light source */
    math::color			diffuse;			/* Diffuse color of light */
    math::color			specular;			/* Specular color of light */
    math::color			ambient;			/* Ambient color of light */
    float3				position;			/* Position in world space */
    float3				direction;			/* Direction in world space */
    float				range;				/* Cutoff range */
    float				falloff;			/* Falloff */
    float				attenuation0;		/* Constant attenuation */
    float				attenuation1;		/* Linear attenuation */
    float				attenuation2;		/* Quadratic attenuation */
    float				theta;				/* Inner angle of spotlight cone */
    float				phi;				/* Outer angle of spotlight cone */

	void set(u32 ltType, float x, float y, float z)
	{
		ZeroMemory( this, sizeof(f_light) );
		type=ltType;
		diffuse.set(1.0f, 1.0f, 1.0f, 1.0f);
		specular = (diffuse);
		position.set(x,y,z);
		direction.set(x,y,z);
		direction.normalize_safe(direction);
		range = sqrt(math::float_max);
	}
    
	//void mul(float brightness)
	//{
	//    diffuse.mul_rgb		(brightness);
 //   	ambient.mul_rgb		(brightness);
	//    specular.mul_rgb	(brightness);
 //   }
};

lights_db::lights_db()
{
	initialize_sun();
}

void lights_db::load_lights(resources::managed_resource_ptr const& level_resource)
{
	resources::pinned_ptr_const<u8> buffer (level_resource);

	memory::chunk_reader reader(buffer.c_ptr(), buffer.size(), memory::chunk_reader::chunk_type_sequential);

	memory::reader lights_chunk	= reader.open_reader(chunk_light_dynamic);


	u32 chunk_size	= lights_chunk.length();
	u32 element		= sizeof(f_light)+4;
	u32 count		= chunk_size/element;
	ASSERT(count*element == chunk_size);
	m_static_lights.reserve(count);

	for (u32 i=0; i<count; ++i) 
	{
		f_light		Ldata;
		light*		L = create	();

		L->flags.bStatic			= true;
		L->set_type					(render::light_type_point);
		//L->set_shadow				(true);

		u32 controller				= 0;
		lights_chunk.r(&controller,4, 4);
		lights_chunk.r(&Ldata,sizeof(f_light), sizeof(f_light));
		if (Ldata.type==TMP_D3DLIGHT_DIRECTIONAL)
		{
			continue;
		}
		else
		{
			float3 tmp_D,tmp_R;
			tmp_D.set			(0,0,-1);	// forward
			tmp_R.set			(1,0,0);	// right

			// point
			m_static_lights.push_back(L);
			
			L->set_position		(Ldata.position);
			L->set_orientation	(tmp_D, tmp_R);
			L->set_range		(Ldata.range);
			L->set_color		(Ldata.diffuse);
			L->set_active		(true);
			//				R_ASSERT			(L->spatial.sector	);
		}
	}


	R_ASSERT(m_sun);
}

light* lights_db::create()
{
	light*	L			= NEW(light);
	L->flags.bStatic	= false;
	L->flags.bActive	= false;
	L->flags.bShadow	= true;

	return				L;
}

void lights_db::initialize_sun	()
{
	ASSERT( !m_sun );

	m_sun = create		();

	m_sun->flags.bStatic			= true;
	m_sun->set_type			(render::light_type_direct);
	m_sun->set_shadow		(true);
	m_sun->set_color		(math::color(0.89578169f, 0.88f, 0.87f, 1));
	//m_sun->set_rotation	(float3(0.47652841f, -0.80543172f, -0.35242066f).normalize(), float3());
									//-0.1f, -0.99f, -0.5f
	m_sun->set_orientation	(float3(-0.0f, -0.99f, -0.0f).normalize(), float3());
}

void lights_db::add_light( u32 id, render::light_props const& props, bool beditor )
{
	editor_light				light_to_add(id);
	editor_lights& list			= beditor ? m_editor_lights : m_game_lights;

	editor_lights::iterator it	= std::lower_bound( list.begin(), list.end(), light_to_add);
	ASSERT						( (it == list.end()) || (it->id != id) );

	light_to_add.light = create();
	light_to_add.light->set_type		( props.type);
	light_to_add.light->set_shadow		( props.shadow_cast);
	light_to_add.light->set_position	( props.transform.c.xyz());
	light_to_add.light->set_orientation	( props.transform.i.xyz(), props.transform.k.xyz());
	light_to_add.light->set_range		( props.range);
	light_to_add.light->set_color		( props.color);

	list.insert					( it, light_to_add );

}

void lights_db::update_light( u32 id, render::light_props const& props, bool beditor)
{
	editor_light				tmp_light( id);
	editor_lights& list			= beditor ? m_editor_lights : m_game_lights;

	vector<editor_light>::iterator	it	= std::lower_bound( list.begin(), list.end(), tmp_light);

	ASSERT( it != list.end() && it->id == id );

	it->light->set_type			( props.type);
	it->light->set_shadow		( props.shadow_cast);
	it->light->set_position		( props.transform.c.xyz());
	it->light->set_orientation	( props.transform.i.xyz(), props.transform.k.xyz());
	it->light->set_range		( props.range);
	it->light->set_color		( props.color);
}

void lights_db::remove_light	( u32 id, bool beditor)
{
	editor_lights& list			= beditor ? m_editor_lights : m_game_lights;
	editor_lights::iterator new_end = std::remove	( list.begin(), list.end(), id );	
	list.erase (new_end, list.end( ) );

	LOG_INFO	( "model_manager::remove_light(id = %d), list.size() == %d ", id, list.size() );
}

} // namespace render 
} // namespace xray 

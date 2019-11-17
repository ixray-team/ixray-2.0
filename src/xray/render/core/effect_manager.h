////////////////////////////////////////////////////////////////////////////
//	Created		: 12.02.2009
//	Author		: Mykhailo Parfeniuk
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef SHADER_MANAGER_H_INCLUDED
#define SHADER_MANAGER_H_INCLUDED

#include <xray/render/core/res_effect.h>
#include <xray/render/core/effect_descriptor.h>

namespace xray {
namespace render_dx10 {

typedef fixed_string<128>	effect_decriptor_name;

class effect_manager: public quasi_singleton<effect_manager>
{
public:
		effect_manager	();
		~effect_manager	();

		res_pass*				create_pass	( const res_pass & prototype);
		void					delete_pass	( res_pass* pass);

		res_shader_technique*	create_effect_technique	( const res_shader_technique& element);
		void					delete_effect_technique	( res_shader_technique* element);

		res_effect*				create_effect	( LPCSTR shader, LPCSTR textures=0);
		res_effect*				create_effect	( effect* desc, LPCSTR shader=0, LPCSTR textures=0);
		void					delete_effect	( res_effect* shader);

		void					register_effect_desctiptor				( char const * name, effect * dectriptor);
		void					register_effect_desctiptor_by_textyre	( char const * tex_name, effect * dectriptor);

private:
	typedef render::map<effect_decriptor_name, effect*>	map_effects;
	typedef	map_effects::iterator				map_effects_it;

public:

	template <typename T>
	static void call_resource_destructor( T* resource) { resource->~T();};

private:
	effect* find_effect( LPCSTR name, LPCSTR texture /* "texture" for testing only */ );

	void load_effects	();
	void load_raw_file	( resources::queries_result& data);

	effect* make_effect( u64 cls);

private:
	render::vector<res_pass*>				m_passes;
	render::vector<res_effect*>				m_shaders;
	render::vector<res_shader_technique*>	m_techniques;
	

	map_effects					m_effects;
	map_effects					m_effects_by_texture;

	bool							m_loading_incomplete;

}; // class effect_manager

} // namespace render 
} // namespace xray 

#include "effect_manager_inline.h"

#endif // #ifndef SHADER_MANAGER_H_INCLUDED
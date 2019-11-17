////////////////////////////////////////////////////////////////////////////
//	Created		: 12.02.2009
//	Author		: Mykhailo Parfeniuk
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include <xray/render/core/effect_manager.h>

#include <xray/render/core/effect_descriptor.h>
#include <xray/render/core/effect_compiler.h>
#include <xray/render/core/sources/manager_common_inline.h>
#include <xray/render/core/resource_manager.h>

// #include "effect_deffer.h"
// #include "effect_deffer_materials.h"
// #include "effect_deffer_aref.h"
// #include "effect_tree.h"
// #include "effect_terrain.h"
// #include "effect_forward_system.h"

namespace xray {
namespace render_dx10 {


////////////////////////////////////////////////////////////////////////
struct effect_manager_call_destructor_predicate 
{

	template <typename T>
	inline void	operator()	( T* resource ) const
	{
		effect_manager::call_resource_destructor( resource );
	}
}; // struct effect_manager_call_destructor_predicate
////////////////////////////////////////////////////////////////////////



effect_manager::effect_manager(): m_loading_incomplete( false)
{
	//load_effects();
}

effect_manager::~effect_manager()
{
	map_effects_it it  = m_effects.begin(),
					end = m_effects.end();

	for( ; it != end; ++it)
		DELETE( it->second);
}

res_pass* effect_manager::create_pass( const res_pass& pass)
{
	for( u32 it=0; it<m_passes.size(); it++)
		if( m_passes[it]->equal( pass))
			return m_passes[it];

	res_pass*	new_pass = NEW( res_pass)( pass.m_vs, pass.m_gs, pass.m_ps, pass.m_state);
	new_pass->mark_registered();

	m_passes.push_back			( new_pass);
	return m_passes.back();
}

void effect_manager::delete_pass( res_pass* pass)
{
	if( !pass->registered())
		return;

	if( reclaim( m_passes, pass))
	{
		DELETE( pass, effect_manager_call_destructor_predicate());
		return;
	}

	LOG_ERROR( "!ERROR: Failed to find compiled pass.");
}

void fill_options( effect_compilation_options& opts, LPCSTR textures)
{
	if( !textures)
		return;

	u32 const length	= strings::length( textures );
	pstr const token	= ( pstr)ALLOCA( ( length+1)*sizeof( char) );
	pcstr next_token	= textures;
	while( next_token )  
	{
		next_token	= strings::get_token( next_token, token, length+1, ',' );
		opts.tex_list.push_back	( token );
	} 
}

effect* effect_manager::find_effect( LPCSTR name, LPCSTR texture /* "texture" for testing only */ )
{
	// First find by texture name. This is temporary; for testing only.
	map_effects_it	it = m_effects_by_texture.find( texture);

	if( it != m_effects_by_texture.end())
		return it->second;


	// map_effects_it
		it = m_effects.find( name);

	if( it != m_effects.end())
		return it->second;

	return 0;
}

res_effect*	effect_manager::create_effect( LPCSTR shader, LPCSTR textures)
{
	if( effect* b = find_effect( shader, textures))
	{
		return create_effect( b, shader, textures);
	}

	return 0;
}

res_effect*	effect_manager::create_effect( effect* desc, LPCSTR shader_name, LPCSTR textures)
{
	XRAY_UNREFERENCED_PARAMETER	( shader_name );
	
	effect_compilation_options	opts;
	fill_options( opts, textures);

	res_effect			shader;
	effect_compiler	compiler( shader);
	desc->compile		( compiler, opts);

//	desc->reset();

//	CBlender_Compile	C;
//	Shader				S;
//
//	//.
//	// if( strstr( s_shader,"transparent"))	__asm int 3;
//
//	// Access to template
//	C.BT				= B;
//	C.bEditor			= FALSE;
//	C.bDetail			= FALSE;
//#ifdef _EDITOR
//	if( !C.BT)			{ ELog.Msg( mtError,"Can't find shader '%s'",s_shader); return 0; }
//	C.bEditor			= TRUE;
//#endif
//
//	// Parse names
//	_ParseList			( C.L_textures,	s_textures	);
//	_ParseList			( C.L_constants,	s_constants	);
//	_ParseList			( C.L_matrices,	s_matrices	);
//
//	// Compile element	( LOD0 - HQ)
//	{
//		C.iElement			= 0;
//		C.bDetail			= m_textures_description.GetDetailTexture( C.L_textures[0],C.detail_texture,C.detail_scaler);
//		//.		C.bDetail			= _GetDetailTexture( *C.L_textures[0],C.detail_texture,C.detail_scaler);
//		ShaderElement		E;
//		C._cpp_Compile		( &E);
//		S.E[0]				= _CreateElement	( E);
//	}
//
//	// Compile element	( LOD1)
//	{
//		C.iElement			= 1;
//		//.		C.bDetail			= _GetDetailTexture( *C.L_textures[0],C.detail_texture,C.detail_scaler);
//		C.bDetail			= m_textures_description.GetDetailTexture( C.L_textures[0],C.detail_texture,C.detail_scaler);
//		ShaderElement		E;
//		C._cpp_Compile		( &E);
//		S.E[1]				= _CreateElement	( E);
//	}
//
//	// Compile element
//	{
//		C.iElement			= 2;
//		C.bDetail			= FALSE;
//		ShaderElement		E;
//		C._cpp_Compile		( &E);
//		S.E[2]				= _CreateElement	( E);
//	}
//
//	// Compile element
//	{
//		C.iElement			= 3;
//		C.bDetail			= FALSE;
//		ShaderElement		E;
//		C._cpp_Compile		( &E);
//		S.E[3]				= _CreateElement	( E);
//	}
//
//	// Compile element
//	{
//		C.iElement			= 4;
//		C.bDetail			= TRUE;	//.$$$ HACK :)
//		ShaderElement		E;
//		C._cpp_Compile		( &E);
//		S.E[4]				= _CreateElement	( E);
//	}
//
//	// Compile element
//	{
//		C.iElement			= 5;
//		C.bDetail			= FALSE;
//		ShaderElement		E;
//		C._cpp_Compile		( &E);
//		S.E[5]				= _CreateElement	( E);
//	}
//
	// Search equal in shaders array
	for( u32 it = 0; it < m_shaders.size(); ++it)
		if( shader.equal( m_shaders[it]))
			return m_shaders[it];

	// Create _new_ entry
	res_effect*		new_shader = NEW( res_effect);//( shader);

	new_shader->m_cur_technique = shader.m_cur_technique;
	new_shader->m_techniques = shader.m_techniques;

	new_shader->mark_registered();
	m_shaders.push_back( new_shader);

	return new_shader;
}

void effect_manager::delete_effect( res_effect* shader)
{
	if( !shader->registered())
		return;

	if( reclaim( m_shaders, shader))
	{
		DELETE( shader, effect_manager_call_destructor_predicate());
		return;
	}

	LOG_ERROR( "!ERROR: Failed to find complete shader");

}

res_shader_technique* effect_manager::create_effect_technique( const res_shader_technique& element)
{
	if( element.m_passes.empty())
		return 0;

	// Search equal in shaders array
	for( u32 it = 0; it < m_techniques.size(); ++it)
		if( element.equal( m_techniques[it]))
			return m_techniques[it];

	// Create _new_ entry
	res_shader_technique*	new_technique = NEW( res_shader_technique); //( element);

	new_technique->m_flags = element.m_flags;
	new_technique->m_passes = element.m_passes;

	new_technique->mark_registered();
	m_techniques.push_back( new_technique);
	return new_technique;
}

void effect_manager::delete_effect_technique( res_shader_technique* technique)
{
	if( !technique->registered())
		return;

	if( reclaim( m_techniques, technique))
	{
		DELETE( technique, effect_manager_call_destructor_predicate());
		return;
	}

	LOG_ERROR( "!ERROR: Failed to find complete shader");
}

template<class T>
class pred_equal_ptr
{
	T*	m_ptr;
public:
	pred_equal_ptr( T* value): m_ptr( value) {}
	bool operator()( const T* value) {return m_ptr->equal( *value);}
};

// res_geometry* effect_manager::create_geometry( DWORD fvf, ID3DVertexBuffer* vb, ID3DIndexBuffer* ib)
// {
// 	ref_declaration dcl	= resource_manager::ref().create_declaration( fvf);
// 	//u32 vb_stride		= D3DXGetDeclVertexSize	( decl,0);
// 
// 	return create_geometry( dcl, vb, ib);
// }



// effect* effect_manager::make_effect( u64 cls)
// {
// 	switch ( cls)
// 	{
// 	case B_DEFAULT:
// 	case B_VERT:
// 		return NEW( effect_deffer);
// 	case B_DEFAULT_AREF:
// 		return NEW( effect_deffer_aref)( true);
// 	case B_VERT_AREF:
// 		return NEW( effect_deffer_aref)( false);
// 	case B_TREE:
// 		return NEW( effect_tree);	
// 	case B_TERRAIN:
// 		return NEW( effect_terrain);	
// 
// 	}
// 
// 	return 0;
// }

// --Porting to DX10_
// void effect_manager::load_effects()
// {
// 	resources::query_resource_and_wait( "resources/shaders.xr", 
// 		resources::raw_data_class, 
// 		resources::query_callback( this, &effect_manager::load_raw_file),
// 		::xray::render::g_allocator);
// }
// 
// void effect_manager::load_raw_file( resources::queries_result& data)
// {
// 	ASSERT( data.is_successful());
// 
// 	resources::pinned_ptr<u8>	ptr(data[0].get_resource());
// 	u32 const size = data[0].get_resource()->get_size();
// 	memory::chunk_reader	base_reader( ptr.raw_pointer(), size, memory::chunk_reader::chunk_type_sequential);
// 
// 	if( base_reader.chunk_exists( 2))
// 	{
// 		memory::chunk_reader ch_it = base_reader.open_chunk_reader( 2);
// 		u32 chunk_id = 0;
// 
// 		while ( ch_it.chunk_exists( chunk_id))
// 		{
// 			memory::reader in = ch_it.open_reader( chunk_id);
// 
// 			effect_desc	desc;
// 			in.r			( &desc, sizeof( desc), sizeof( desc));
// 
// 			effect* effect_code = make_effect( desc.m_cls);
// 
// 			if	( effect_code == 0)
// 			{
// 				//LOG_INFO( "!Renderer doesn't support effect '%s'", desc.m_name);
// 			}
// 			else
// 			{
// 
// 				char buff[9];
// 
// 				//*( ( class_id*)&buff) = desc.m_cls;
// 				buff[0] = ( ( char*)&( desc.m_cls))[7];
// 				buff[1] = ( ( char*)&( desc.m_cls))[6];
// 				buff[2] = ( ( char*)&( desc.m_cls))[5];
// 				buff[3] = ( ( char*)&( desc.m_cls))[4];
// 				buff[4] = ( ( char*)&( desc.m_cls))[3];
// 				buff[5] = ( ( char*)&( desc.m_cls))[2];
// 				buff[6] = ( ( char*)&( desc.m_cls))[1];
// 				buff[7] = ( ( char*)&( desc.m_cls))[0];
// 				buff[8] = 0;
// 
// 				LOG_INFO( "Blender pair - > %s, %s", buff, desc.m_name);
// 				//if	( B->getDescription().version != desc.version)
// 				//{
// 				//	Msg( "! Version conflict in shader '%s'",desc.cName);
// 				//}
// 
// 				in.rewind();
// 				effect_code->load( in);
// 
// 				std::pair<map_effects_it, bool> I =  m_effects.insert( utils::mk_pair( effect_decriptor_name( desc.m_name), effect_code));
// 				R_ASSERT( I.second && "shader.xr - found duplicate name!!!");
// 			}
// 
// 			++chunk_id;
// 		}
// 	}
// 
// 	m_loading_incomplete = false;
// }

void effect_manager::register_effect_desctiptor( char const * name, effect * dectriptor)
{
 	m_effects.insert( utils::mk_pair(name, dectriptor));
}

void effect_manager::register_effect_desctiptor_by_textyre	( char const * tex_name, effect * dectriptor)
{
	m_effects.insert( utils::mk_pair( tex_name, dectriptor));
}


} // namespace render 
} // namespace xray 

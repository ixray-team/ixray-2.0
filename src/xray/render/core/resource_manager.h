////////////////////////////////////////////////////////////////////////////
//	Created		: 01.05.2010
//	Author		: Armen Abroyan
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef RESOURCE_MANAGER_H_INCLUDED
#define RESOURCE_MANAGER_H_INCLUDED

#include <xray/resources.h>
#include <xray/resources_fs.h>
#include <xray/resources_queries_result.h>
#include <xray/resources_query_result.h>
#include "state_cache.h"
#include "state_descriptor.h"
#include "res_state.h"

#include "res_rt.h"
#include "res_texture.h"
#include "res_buffer.h"
#include "res_constant_buffer.h"
#include "res_sampler_state.h"
#include "res_vs_hw.h"
#include "res_gs_hw.h"
#include "res_ps_hw.h"
#include "res_xs.h"
#include "xs_descriptor.h"
#include "res_input_layout.h"
#include "res_geometry.h"


namespace xray {
namespace render_dx10 {

class state_descriptor;
class sampler_state_descriptor;
class texture_storage;

struct shader_compilation_opts
{
	int	m_bones_count;
	int	m_msaa_samples_count;
};

typedef fastdelegate::FastDelegate< void ( enum_shader_type shader_type, char const* name, char const* error_string) >	shader_compile_error_handler;

class resource_manager : public quasi_singleton<resource_manager>
{
	friend class texture_pool;

public:
	struct str_pred : public std::binary_function<char*, char*, bool>
	{
		inline bool operator()( LPCSTR x, LPCSTR y) const {return strcmp( x, y) < 0;}
	};

	typedef render::map<const char*, res_vs_hw*, str_pred>	map_vs_hw;
	typedef render::map<const char*, res_ps_hw*, str_pred>	map_ps_hw;
	typedef render::map<const char*, res_rt*, str_pred>	map_rt;
	typedef render::map<const char*, res_texture*, str_pred>	map_texture;

	typedef render::vector<u8> shader_source;
		
public:
	resource_manager( );
	~resource_manager( );

	bool					add_shader_code				( char const* shader_name, char const* shader_source_code, bool rewrite_exists = false);

	res_texture*			create_texture				( char const * name, u32 mip_level_cut = 0, bool use_pool = false);
	res_texture*			create_texture2d			( char const * user_name, u32 width, u32 height, D3D_SUBRESOURCE_DATA const* data, DXGI_FORMAT format, D3D_USAGE usage, u32 mip_levels, u32 array_size = 1);
	res_texture*			create_texture3d			( char const * user_name, u32 width, u32 height, u32 depth, D3D_SUBRESOURCE_DATA const * data, DXGI_FORMAT format, u32 usage, u32 mip_levels);
	res_texture*			load_texture				( char const * name, u32 mip_level_cut, bool use_pool = false);

	bool					copy_texture_from_file		( ref_texture dest_texture, math::rectangle<math::int2> dest_rect, u32 arr_ind, char const* src_name);

	res_buffer* 			create_buffer				( u32 size, const void * data, enum_buffer_type type, bool dynamic);

	res_constant_table*		create_const_table			( res_constant_table const& proto);
	
	constant_host const*	register_constant_binding	( constant_binding const & binding);

	res_constant_buffer*	create_constant_buffer		( name_string const & name, enum_shader_type buffer_dest, D3D_CBUFFER_TYPE type, u32 size);

	res_sampler_state*		create_sampler_state		( sampler_state_descriptor const & sampler_props);

	res_vs_hw*				create_vs_hw				( const char * name, shader_defines_list& defines);

	res_ps_hw*				create_ps_hw				( const char * name, shader_defines_list& defines);

	res_rt*					create_render_target		( LPCSTR name, u32 w, u32 h, DXGI_FORMAT fmt, res_rt::enum_usage usage);

	res_vs*					create_vs					( vs_descriptor const & vs);

	res_ps*					create_ps					( ps_descriptor const & ps);

 	res_state*				create_state				( state_descriptor & descriptor);

	res_declaration*		create_declaration			( D3D_INPUT_ELEMENT_DESC const* dcl, u32 count);

	template< int size >
	inline res_declaration*	create_declaration			( D3D_INPUT_ELEMENT_DESC const (&dcl) [size])				{ return create_declaration( (D3D_INPUT_ELEMENT_DESC const*) &dcl, size);}

	res_signature*			create_signature			( ID3DBlob * signature);

	res_input_layout*		create_input_layout			( res_declaration const * decl, res_signature const * signature);

	res_texture_list*		create_texture_list			( texture_slots const & tex_list);

	res_sampler_list*		create_sampler_list			( sampler_slots const & tex_list);

	res_geometry*			create_geometry				( D3D_INPUT_ELEMENT_DESC const* decl, u32 decl_size, u32 vertex_stride, res_buffer* vb, res_buffer* ib);
	template< int size >
	res_geometry*			create_geometry 			( D3D_INPUT_ELEMENT_DESC const (&decl)[size], u32 vertex_stride, res_buffer* vb, res_buffer* ib);
	res_geometry*			create_geometry				( res_declaration * dcl, u32 vertex_stride, res_buffer * vb, res_buffer * ib);

	void					initialize_texture_storage	( xray::configs::lua_config_value const & cfg);

	inline void				copy						( res_buffer* dest, res_buffer* source);
	inline void				copy						( res_buffer* dest, u32 dest_pos, res_buffer* source, u32 src_pos, u32 size);


	void					set_compile_error_handler	( shader_compile_error_handler const & handler);


	// Release functions
	void		release	( res_constant_table *	const_table);
	void		release	( res_constant_buffer *	buffer);
	void		release	( res_vs_hw *		vs);
	void		release	( res_gs_hw *)		{}// empty
	void		release	( res_ps_hw *		ps);
	void		release	( res_rt *			rt);
	void		release	( res_vs *			vs);
	void		release	( res_gs *)			{}// empty
	void		release	( res_ps *			ps);
	void		release	( res_state *		state);
	void		release	( res_declaration *	dcl);
	void		release	( res_signature *	signature);
	void		release	( res_input_layout *	dcl);
	void		release	( res_texture_list *	tex_list);
	void		release	( res_sampler_list *	tex_list);
	void		release	( res_texture *		texture);
	void		release	( res_buffer *		buffer);
	void		release	( res_geometry *		geom);

	void		evict();

	static const char* get_shader_path();

public:
	typedef render::map<fs::path_string, shader_source>	map_shader_sources;

private:
	void load_shader_sources		();
	
	void process_files				( resources::fs_iterator it);
	
	void on_raw_files_load			( resources::queries_result& data);
	void on_texture_loaded			( resources::queries_result& data, u32 mip_level_cut = 0);
	void on_texture_loaded_staging	( resources::queries_result& data, u32 mip_level_cut);
	void copy_texture_from_file_cb	( resources::queries_result& data);

	res_texture*	create_texture2d_impl	( u32 width, u32 height, D3D_SUBRESOURCE_DATA const* data, DXGI_FORMAT format, D3D_USAGE usage, u32 mip_levels, u32 array_size = 1);
	void			release_impl			( res_texture* tex);

public:

	template <typename T>
	static void call_resource_destructor( T* resource) { resource->~T();};


private:
	map_vs_hw									m_vs_hw_registry;
	map_ps_hw									m_ps_hw_registry;
	map_rt										m_rt_registry;
	map_texture									m_texture_registry;

	render::vector<res_constant_table*>			m_const_tables;
	render::vector<res_constant_buffer*>		m_const_buffers;
	render::vector<res_buffer*>					m_buffers;

	typedef render::vector<res_vs*> vs_cache;
	typedef render::vector<res_gs*> gs_cache;
	typedef render::vector<res_ps*> ps_cache;

	vs_cache									m_v_shaders;
	gs_cache									m_g_shaders;
	ps_cache									m_p_shaders;
	
	render::vector<res_state*>									m_states;

	state_cache<ID3DRasterizerState, D3D_RASTERIZER_DESC>		m_rs_cache;
	state_cache<ID3DDepthStencilState, D3D_DEPTH_STENCIL_DESC>	m_dss_cache;
	state_cache<ID3DBlendState, D3D_BLEND_DESC>					m_bs_cache;
	state_cache<ID3DSamplerState, D3D_SAMPLER_DESC>				m_sampler_cache;

	render::vector<res_declaration*>			m_declarations;
	render::vector<res_signature*>				m_signatures;
	render::vector<res_input_layout*>			m_input_layouts;
	render::vector<res_texture_list*>			m_texture_lists;
	render::vector<res_sampler_list*>			m_sampler_lists;

	render::vector<u32>							m_vs_ids;
	render::vector<u32>							m_ps_ids;
	render::vector<u32>							m_gs_ids;

	typedef render::vector<res_geometry*>		vec_geoms;
	vec_geoms									m_geometries;

	map_shader_sources							m_sources;
	render::vector<fs::path_string>				m_files_list;

	bool	m_loading_incomplete;

	constant_bindings	m_const_bindings;

	texture_storage*							m_texture_storage;
	texture_storage*							m_texture_storage_staging;
	
	shader_compile_error_handler				m_compile_error_handler;
}; // class resource_manager

} // namespace render 
} // namespace xray 


#include "resource_manager_inline.h"

#endif // #ifndef RESOURCE_MANAGER_H_INCLUDED
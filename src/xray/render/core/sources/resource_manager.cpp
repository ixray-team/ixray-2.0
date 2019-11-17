////////////////////////////////////////////////////////////////////////////
//	Created		: 01.05.2010
//	Author		: Armen Abroyan
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <xray/render/core/resource_manager.h>
#include <xray/render/core/sampler_state_descriptor.h>
#include <xray/render/core/state_descriptor.h>
#include <xray/render/core/res_xs.h>
#include <xray/render/core/res_ps_hw.h>
#include <xray/render/core/device.h>
#include <xray/render/core/backend.h>
#include <xray/render/core/sources/manager_common_inline.h>
#include <xray/render/core/sources/texture_storage.h>
#include <xray/render/core/sources/dds.h>
//#include <xray/render/common/sources/texture_cook.h>

#include <xray/configs.h>

#include <D3Dcompiler.h>


namespace xray {
namespace render_dx10 {


////////////////////////////////////////////////////////////////////////

struct resource_manager_call_destructor_predicate {
	
	template <typename T>
	inline void	operator()	( T* resource ) const
	{
		resource_manager::call_resource_destructor( resource );
	}
}; // struct resource_manager_call_destructor_predicate
////////////////////////////////////////////////////////////////////////



HRESULT	shader_compile( 
	LPCSTR						name,
	LPCSTR						source,
	UINT						source_len,
	const shader_defines_list&	defines,
	ID3DInclude*				includes,
	LPCSTR						entry_point,
	LPCSTR						target,
	DWORD						flags,
	ID3DBlob**					out_shader,
	ID3DBlob**					out_error_msgs)
{
	XRAY_UNREFERENCED_PARAMETER	( name);

// #ifdef	D3DXSHADER_USE_LEGACY_D3DX9_31_DLL	//	December 2006 and later
// 	flags |= D3DXSHADER_USE_LEGACY_D3DX9_31_DLL;
// #endif


	HRESULT	result = D3DXCompileFromMemory( 	source, 
									source_len, 				
									"",//NULL, //	NVPerfHUD bug workaround.
									defines.empty() ? NULL : &*defines.begin(), 
									includes, 
									entry_point,			
									target, 
									flags, 
									0,
									0,
									out_shader, 
									out_error_msgs,
									0);

	//if( SUCCEEDED( _result) && o.disasm)
	//{
	//	ID3DXBuffer*		code	= *( ( LPD3DXBUFFER*)_ppShader);
	//	ID3DXBuffer*		disasm	= 0;
	//	D3DXDisassembleShader		( LPDWORD( code->GetBufferPointer()), FALSE, 0, &disasm);
	//	string_path			dname;
	//	strconcat			( sizeof( dname),dname,"disasm\\",name,( 'v'==pTarget[0])?".vs":".ps");
	//	IWriter*			W		= FS.w_open( "$logs$",dname);
	//	W->w				( disasm->GetBufferPointer(),disasm->GetBufferSize());
	//	FS.w_close			( W);
	//	_RELEASE			( disasm);
	//}
	return result;
}

class includes_handler : public ID3DInclude
{
public:
	includes_handler( resource_manager::map_shader_sources* source_map): m_sources( source_map)
	{
	}

	HRESULT __stdcall	Open	( D3D_INCLUDE_TYPE include_type, LPCSTR file_name, LPCVOID parent_data, LPCVOID *data_ptr, UINT *size_ptr)
	{
		XRAY_UNREFERENCED_PARAMETERS	( parent_data, include_type);

		fs::path_string	path = resource_manager::ref().get_shader_path();
		path += "/";
		path += file_name;
		
		utils::fix_path( path.get_buffer());

		resource_manager::map_shader_sources::iterator	it  = m_sources->find( path),
														end = m_sources->end();

		if( it == end)
		{
			return E_FAIL;
		}

		//string_path				pname;
		//strconcat				( sizeof( pname),pname,::Render->getShaderPath(),pFileName);
		//IReader*		R		= FS.r_open	( "$game_shaders$",pname);
		//if( 0==R)				{
		//	// possibly in shared directory or somewhere else - open directly
		//	R					= FS.r_open	( "$game_shaders$",pFileName);
		//	if( 0==R)			return			E_FAIL;
		//}

		//// duplicate and zero-terminate
		//u32				size	= R->length();
		//u8*				data	= xr_alloc<u8>	( size + 1);
		//CopyMemory			( data,R->pointer(),size);
		//data[size]				= 0;
		//FS.r_close				( R);

		*data_ptr = &it->second[0];
		*size_ptr = it->second.size();

		return	S_OK;
	}
	HRESULT __stdcall	Close	( LPCVOID	pData)
	{
		XRAY_UNREFERENCED_PARAMETER	( pData);
		//xr_free	( pData);
		return	S_OK;
	}

private:
	resource_manager::map_shader_sources*	m_sources;
};

//Try to get rid of this!!!!!!
void make_defines( shader_defines_list& defines)
{
	//{
	//	sprintf						( c_smapsize,"%d",u32( o.smapsize));
	//	defines[def_it].Name		=	"SMAP_size";
	//	defines[def_it].Definition	=	c_smapsize;
	//	def_it						++	;
	//}
	if( device::ref().options.fp16_filter)
	{
		defines.push_back( D3D_SHADER_MACRO());
		defines.back().Name       = "FP16_FILTER";
		defines.back().Definition = "1";
	}
	if( device::ref().options.fp16_blend)
	{
		defines.push_back( D3D_SHADER_MACRO());
		defines.back().Name       = "FP16_BLEND";
		defines.back().Definition = "1";
	}
	//if( o.HW_smap)			{
	//	defines[def_it].Name		=	"USE_HWSMAP";
	//	defines[def_it].Definition	=	"1";
	//	def_it						++	;
	//}
	//if( o.HW_smap_PCF)			{
	//	defines[def_it].Name		=	"USE_HWSMAP_PCF";
	//	defines[def_it].Definition	=	"1";
	//	def_it						++	;
	//}
	//if( o.HW_smap_FETCH4)			{
	//	defines[def_it].Name		=	"USE_FETCH4";
	//	defines[def_it].Definition	=	"1";
	//	def_it						++	;
	//}
	//if( o.sjitter)			{
	//	defines[def_it].Name		=	"USE_SJITTER";
	//	defines[def_it].Definition	=	"1";
	//	def_it						++	;
	//}
	//if( ps_r2_lighting == 1)
	//{
	//	defines[def_it].Name		=	"USE_OREN_NAYAR";
	//	defines[def_it].Definition	=	"1";
	//	def_it						++	;
	//}
	//else if( ps_r2_lighting == 2)
	//{
	//	defines[def_it].Name		=	"USE_COOK_TORRANCE";
	//	defines[def_it].Definition	=	"1";
	//	def_it						++	;
	//}
	//if( ps_r2_ls_flags_ext.test( R2FLAGEXT_TWEAK_MATERIAL))
	//{
	//	defines[def_it].Name		=	"MATERIAL_TWEAK_MODE";
	//	defines[def_it].Definition	=	"1";
	//	def_it						++	;
	//}
	if( device::ref().get_caps().raster_major >= 3)
	{
		defines.push_back( D3D_SHADER_MACRO());
		defines.back().Name       = "USE_BRANCHING";
		defines.back().Definition = "1";
	}
	//if( HW.Caps.geometry.bVTF)	{
	//	defines[def_it].Name		=	"USE_VTF";
	//	defines[def_it].Definition	=	"1";
	//	def_it						++	;
	//}
	//if( o.Tshadows)			{
	//	defines[def_it].Name		=	"USE_TSHADOWS";
	//	defines[def_it].Definition	=	"1";
	//	def_it						++	;
	//}
	//if( o.mblur)			{
	//	defines[def_it].Name		=	"USE_MBLUR";
	//	defines[def_it].Definition	=	"1";
	//	def_it						++	;
	//}
	//if( o.sunfilter)		{
	//	defines[def_it].Name		=	"USE_SUNFILTER";
	//	defines[def_it].Definition	=	"1";
	//	def_it						++	;
	//}
	//if( o.sunstatic)		{
	//	defines[def_it].Name		=	"USE_R2_STATIC_SUN";
	//	defines[def_it].Definition	=	"1";
	//	def_it						++	;
	//}
	//if( o.forcegloss)		{
	//	sprintf						( c_gloss,"%f",o.forcegloss_v);
	//	defines[def_it].Name		=	"FORCE_GLOSS";
	//	defines[def_it].Definition	=	c_gloss;
	//	def_it						++	;
	//}
	//if( o.forceskinw)		{
	//	defines[def_it].Name		=	"SKIN_COLOR";
	//	defines[def_it].Definition	=	"1";
	//	def_it						++;
	//}

	//// skinning
	//if( m_skinning<0)		{
	//	defines[def_it].Name		=	"SKIN_NONE";
	//	defines[def_it].Definition	=	"1";
	//	def_it						++	;
	//}
	//if( 0==m_skinning)		{
	//	defines[def_it].Name		=	"SKIN_0";
	//	defines[def_it].Definition	=	"1";
	//	def_it						++;
	//}
	//if( 1==m_skinning)		{
	//	defines[def_it].Name		=	"SKIN_1";
	//	defines[def_it].Definition	=	"1";
	//	def_it						++;
	//}
	//if( 2==m_skinning)		{
	//	defines[def_it].Name		=	"SKIN_2";
	//	defines[def_it].Definition	=	"1";
	//	def_it						++;
	//}
	//if( 3==m_skinning)		{
	//	defines[def_it].Name		=	"SKIN_3";
	//	defines[def_it].Definition	=	"1";
	//	def_it						++;
	//}
	//if( 4==m_skinning)		{
	//	defines[def_it].Name		=	"SKIN_4";
	//	defines[def_it].Definition	=	"1";
	//	def_it						++;
	//}

	//if( o.ssao_blur_on)
	//{
	//	defines[def_it].Name		=	"USE_SSAO_BLUR";
	//	defines[def_it].Definition	=	"1";
	//	def_it						++;
	//}

	////	Igor: need restart options
	//if( RImplementation.o.advancedpp && ps_r2_ls_flags.test( R2FLAG_SOFT_WATER))
	//{
	//	defines[def_it].Name		=	"USE_SOFT_WATER";
	//	defines[def_it].Definition	=	"1";
	//	def_it						++;
	//}

	//if( RImplementation.o.advancedpp && ps_r2_ls_flags.test( R2FLAG_SOFT_PARTICLES))
	//{
	//	defines[def_it].Name		=	"USE_SOFT_PARTICLES";
	//	defines[def_it].Definition	=	"1";
	//	def_it						++;
	//}

	//if( RImplementation.o.advancedpp && ps_r2_ls_flags.test( R2FLAG_DOF))
	//{
	//	defines[def_it].Name		=	"USE_DOF";
	//	defines[def_it].Definition	=	"1";
	//	def_it						++;
	//}

	//if( RImplementation.o.advancedpp && ps_r_sun_shafts)
	//{
	//	sprintf_s					( c_sun_shafts,"%d",ps_r_sun_shafts);
	//	defines[def_it].Name		=	"SUN_SHAFTS_QUALITY";
	//	defines[def_it].Definition	=	c_sun_shafts;
	//	def_it						++;
	//}

	//if( RImplementation.o.advancedpp && ps_r_ssao)
	//{
	//	sprintf_s					( c_ssao,"%d",ps_r_ssao);
	//	defines[def_it].Name		=	"SSAO_QUALITY";
	//	defines[def_it].Definition	=	c_ssao;
	//	def_it						++;
	//}

	//if( RImplementation.o.advancedpp && ps_r_sun_quality)
	//{
	//	sprintf_s					( c_sun_quality,"%d",ps_r_sun_quality);
	//	defines[def_it].Name		=	"SUN_QUALITY";
	//	defines[def_it].Definition	=	c_sun_quality;
	//	def_it						++;
	//}

	//if( RImplementation.o.advancedpp && ps_r2_ls_flags.test( R2FLAG_STEEP_PARALLAX))
	//{
	//	defines[def_it].Name		=	"ALLOW_STEEPPARALLAX";
	//	defines[def_it].Definition	=	"1";
	//	def_it						++;
	//}

	defines.push_back( D3D_SHADER_MACRO());
	defines.back().Name     = 0;
	defines.back().Definition = 0;
}

// namespace resource_manager_predicates
// {
// 
// 
// // 	struct equal
// // 	{
// // 		equal( binder const & binder) : m_descriptor( binder) {}
// // 		
// // 
// // 		binder	m_descriptor;
// // 	};
// };

bool resource_manager::add_shader_code(char const* shader_name, char const* source_code, bool rewrite_exists)
{
	std::string path = "resources/shaders/sm_4_0/";

	resource_manager::shader_source source;

	for ( u32 i=0; i<strlen(source_code); i++)
		source.push_back(source_code[i]);

	if ( rewrite_exists || m_sources.find(shader_name)==m_sources.end() )
	{
		std::string new_path = path + shader_name;
		m_sources[new_path.c_str()] = source;
		return true;
	}
	return false;
}

res_vs_hw*	resource_manager::create_vs_hw( char const * name, shader_defines_list& defines)
{
	// Here need to consider also defines 
	fs::path_string	reg_name( name);
	map_vs_hw::iterator it	= m_vs_hw_registry.find( reg_name.get_buffer());
	
	if( it != m_vs_hw_registry.end())
	{
		return it->second;
	}
	else
	{
		res_vs_hw* vs	= NEW( res_vs_hw);
		vs->mark_registered			();
		m_vs_hw_registry.insert	( utils::mk_pair( vs->set_name( reg_name.get_buffer()), vs));
		vs->set_id				( gen_id( m_vs_hw_registry, m_vs_ids));
		
		if( 0 == _stricmp( name, "null"))
		{
			//vs->m_vs = NULL;
			return vs;
		}

		includes_handler			includer( &m_sources);
		ID3D10Blob*					shader_code	= NULL;
		ID3D10Blob*					error_buf	= NULL;

		u32 pos = reg_name.find( "( ");
		fs::path_string nm( pos==fs::path_string::npos ? name : reg_name.substr( 0, pos));

		fs::path_string				sh_name( get_shader_path());
		sh_name.appendf( "/%s.vs", nm.c_str());

		LPCSTR						c_target = "vs_4_0";
		LPCSTR						c_entry  = "main";
	
		//shader_defines_list	defines;
		//make_defines( local_opts, defines);
		
		map_shader_sources::iterator src_it = m_sources.find( sh_name);
		ASSERT( src_it != m_sources.end()); //What to do if file not found?????
		ASSERT( !src_it->second.empty());


		HRESULT hr = shader_compile( name,
			( LPCSTR)&src_it->second[0],
			src_it->second.size(), defines, 
			&includer,
			c_entry,
			c_target,
			D3D_SHADER_DEBUG | D3D_SHADER_PACK_MATRIX_ROW_MAJOR //| D3D10_SHADER_ENABLE_BACKWARDS_COMPATIBILITY
			/*| D3DXSHADER_PREFER_FLOW_CONTROL*/,
			&shader_code, &error_buf);

		if( SUCCEEDED( hr))
		{
			hr = vs->create_hw_shader( shader_code);
			if( FAILED( hr))
			{
				LOG_ERROR( "!VS: %s", name);
				LOG_ERROR( "shader creation failed");
				FATAL( "!Shader is not created");
			}
		}
		else
		{
			ASSERT( error_buf);
			pcstr const err_msg = ( LPCSTR)error_buf->GetBufferPointer();

			if( !m_compile_error_handler.empty())
				m_compile_error_handler	( enum_shader_type_vertex, name, err_msg);

			XRAY_UNREFERENCED_PARAMETER	( err_msg );
			FATAL( "!Vertex shader \"%s\" is not created. \nerrors: \n%s", name, err_msg);
		}

		safe_release( shader_code);
		safe_release( error_buf);

		//R_CHK		( hr);
		return		vs;
	}
}

void resource_manager::release( res_vs_hw * vs)
{
	if( !vs->registered())
		return;

	LPSTR name = LPSTR( vs->m_name.c_str());
	map_vs_hw::iterator it	= m_vs_hw_registry.find( name);

	if( it != m_vs_hw_registry.end())
	{
		m_vs_ids.push_back( it->second->get_id());
		m_vs_hw_registry.erase( it);
		DELETE( vs, resource_manager_call_destructor_predicate());

		return;
	}

	LOG_ERROR( "! ERROR: Failed to find compiled vertex-shader '%s'",vs->m_name.c_str());
}

res_ps_hw* resource_manager::create_ps_hw( LPCSTR name, shader_defines_list& defines)
{
	fs::path_string		reg_name( name);
	map_ps_hw::iterator	it	= m_ps_hw_registry.find( reg_name.get_buffer());

	if( it != m_ps_hw_registry.end())
	{
		return it->second;
	}
	else
	{
		res_ps_hw* ps	= NEW( res_ps_hw);
		ps->mark_registered();
		m_ps_hw_registry.insert( utils::mk_pair( ps->set_name( reg_name.get_buffer()), ps));
		ps->set_id( gen_id( m_ps_hw_registry, m_ps_ids));

		if( 0 == _stricmp( name, "null"))
		{
			//ps->m_ps = NULL;
			return ps;
		}

		includes_handler			includer( &m_sources);
		ID3D10Blob*					shader_code	= NULL;
		ID3D10Blob*					error_buf	= NULL;
		fs::path_string				sh_name( get_shader_path());
		u32 pos = reg_name.find( "( ");
		fs::path_string nm( pos==fs::path_string::npos ? name : reg_name.substr( 0, pos));
		sh_name.appendf( "/%s.ps", nm.c_str());

		LPCSTR						c_target = "ps_4_0";
		LPCSTR						c_entry  = "main";

		//shader_defines_list	defines;
		//make_defines( local_opts, defines);

		map_shader_sources::iterator src_it = m_sources.find( sh_name);
		ASSERT( src_it != m_sources.end()); //What to do if file not found?????
		ASSERT( !src_it->second.empty());

		HRESULT hr = shader_compile( name,
			( LPCSTR)&src_it->second[0],
			src_it->second.size(), defines, 
			&includer,
			c_entry,
			c_target,
			D3D_SHADER_DEBUG | D3D_SHADER_PACK_MATRIX_ROW_MAJOR
			/*| D3DXSHADER_PREFER_FLOW_CONTROL*/,
			&shader_code, &error_buf);

		if( SUCCEEDED( hr))
		{
			hr = ps->create_hw_shader( shader_code);
			if( FAILED( hr))
			{
				LOG_ERROR( "!PS: %s", name);
				LOG_ERROR( "!shader creation failed");
				FATAL( "!Shader is not created");
			}
		}
		else
		{
			ASSERT( error_buf);
			pcstr const err_msg = ( LPCSTR)error_buf->GetBufferPointer();

			if( !m_compile_error_handler.empty())
				m_compile_error_handler	( enum_shader_type_pixel, name, err_msg);

			XRAY_UNREFERENCED_PARAMETER	( err_msg );
			FATAL( "!Pixel shader \"%s\" is not created. \nerrors: \n%s", name, err_msg);
		}

		safe_release( shader_code);
		safe_release( error_buf);

		return		ps;
	}
}

void resource_manager::release( res_ps_hw* ps)
{
	if( !ps->registered())
		return;

	LPSTR name = LPSTR( ps->m_name.c_str());
	map_ps_hw::iterator it	= m_ps_hw_registry.find( name);

	if( it != m_ps_hw_registry.end())
	{
		m_ps_ids.push_back( it->second->get_id());
		m_ps_hw_registry.erase( it);
		DELETE( ps, resource_manager_call_destructor_predicate());

		return;
	}

	

	LOG_ERROR( "! ERROR: Failed to find compiled pixel-shader '%s'",ps->m_name.c_str());
}

const char* resource_manager::get_shader_path()
{
	return "resources/shaders/sm_4_0";
}

void resource_manager::process_files( resources::fs_iterator	begin)
{
	vector<resources::fs_iterator>	stack;
	stack.push_back( begin);
	fs::path_string name;
	begin.get_full_path( name);
	vector<resources::request>	requests;
	while ( !stack.empty())
	{
		resources::fs_iterator curDir = stack.back();
		stack.pop_back();
		
		resources::fs_iterator it  = curDir.children_begin(),
					end = curDir.children_end();

		for ( ; it != end; ++it)
		{
			fs::path_string name;
			if( it.is_folder())
			{
				stack.push_back( it);
			}
			else
			{
				fs::path_string	file_name;
				it.get_full_path( file_name);

				//u32 pos = file_name.size()-3;
				//if( file_name.substr( pos) == ".vs" || 
				//	file_name.substr( pos) == ".ps" ||
				//	file_name.substr( pos) == ".h"  ||
				//	file_name.substr( pos) == ".s")
					m_files_list.push_back( file_name);
			}
		}
	}

	if( m_files_list.empty())
	{
		m_loading_incomplete = false;
		return;
	}

	//to ensure that list does not reallocate
	for ( u32 i = 0; i < m_files_list.size(); ++i)
	{
		resources::request	req;
		req.path = m_files_list[i].get_buffer();
		req.id   = resources::raw_data_class;
		requests.push_back( req);
	}

	resources::query_resources( &requests[0], requests.size(),
		boost::bind( &resource_manager::on_raw_files_load, this, _1),
		::xray::render::g_allocator);
}

resource_manager::resource_manager( ) :
	m_loading_incomplete		( false),
	m_texture_storage			( NULL),
	m_texture_storage_staging	( NULL),
	m_compile_error_handler		( NULL)
{
	load_shader_sources	();

	m_texture_storage = NEW (texture_storage)();
	m_texture_storage_staging = NEW (texture_storage)();
}

resource_manager::~resource_manager( )
{
	for( u32 i = 0; i< m_states.size(); ++i)
		DELETE( m_states[i], resource_manager_call_destructor_predicate());

	DELETE( m_texture_storage);
	DELETE( m_texture_storage_staging);
}

void	resource_manager::initialize_texture_storage	( xray::configs::lua_config_value const & cfg)
{
	u32 memory_amount = device::ref().video_memory_size();

	map_texture::iterator		it = m_texture_registry.begin();
	map_texture::const_iterator	end = m_texture_registry.end();

	u32 mem_usage = 0;
	for( ; it != end; ++it)
		mem_usage += utils::calc_texture_size( it->second->width(), it->second->height(), it->second->format(), it->second->mips_count());

	u32 memory_to_use = memory_amount - mem_usage - 128*1024*1024; //mb
	float staging_percent = 2.f;

 	m_texture_storage->initialize			( cfg, u32(memory_to_use * (100 - staging_percent)/100), D3D_USAGE_DEFAULT);
	m_texture_storage_staging->initialize	( cfg, u32(memory_to_use * staging_percent / 100), D3D_USAGE_STAGING);
}

void resource_manager::on_raw_files_load( resources::queries_result& data)
{
	ASSERT( data.is_successful());
	ASSERT( data.size() == m_files_list.size());

	m_loading_incomplete = false;

	for ( u32 i = 0; i < data.size(); ++i)
	{
		resources::pinned_ptr_const<u8> src  ( data[i].get_managed_resource());
		m_sources[m_files_list[i]].assign( src.c_ptr(), src.c_ptr() + src.size());
	}

	//release file names list
	{
		vector<fs::path_string>	empty;
		m_files_list.swap( empty);
	}
}


void resource_manager::load_shader_sources()
{
	m_loading_incomplete = true;

		resources::query_fs_iterator( get_shader_path(), 
			boost::bind( &resource_manager::process_files, this, _1 ),
			::xray::render::g_allocator);
// 	}
// 	else {
// 		resources::request requests[] = {
// 			{ "resources/shaders/sm_4_0/test.vs",	resources::raw_data_class },
// 			{ "resources/shaders/sm_4_0/test.ps",	resources::raw_data_class },
// 			{ "resources/shaders/sm_4_0/color.ps",	resources::raw_data_class },
// 			{ "resources/shaders/sm_4_0/common.h",	resources::raw_data_class },
// 			{ "resources/shaders/sm_4_0/shared/common.h",	resources::raw_data_class },
// 			{ "resources/shaders/sm_4_0/terrain.vs",	resources::raw_data_class },
// 			{ "resources/shaders/sm_4_0/terrain.ps",	resources::raw_data_class },
// 
// 		};
// 		for ( u32 i=0, n=array_size( requests); i < n; ++i) {
// 			m_files_list.push_back	( requests[i].path);
// 		}
// 
// 		resources::query_resources( &requests[0], array_size( requests),
// 			resources::query_callback( this, &resource_manager::on_raw_files_load),
// 			::xray::render::g_allocator);
// 	}

	while ( m_loading_incomplete)
	{
		if( threading::g_debug_single_thread)
			resources::tick				();

		resources::dispatch_callbacks	();
	}
}

res_constant_table* resource_manager::create_const_table( res_constant_table const& proto)
{
	res_constant_table new_table = proto;
// 	if( new_table.empty())
// 		return NULL;

	new_table.apply_bindings( m_const_bindings);

	for ( u32 it=0; it < m_const_tables.size(); ++it)
		if( m_const_tables[it]->equal( new_table))
			return m_const_tables[it];

	m_const_tables.push_back( NEW( res_constant_table)( new_table));
	m_const_tables.back()->mark_registered();
	
	return m_const_tables.back		();
}

void resource_manager::release( res_constant_table* const_table)
{
	if( !const_table->registered())
		return;

	if( reclaim( m_const_tables, const_table))
	{
		DELETE( const_table, resource_manager_call_destructor_predicate());
		return;
	}

	LOG_ERROR( "!ERROR: Failed to find compiled constant-table");
}

constant_host const * resource_manager::register_constant_binding( constant_binding const & binding)
{
	m_const_bindings.add( binding);

	constant_host * host =  backend::ref().register_constant_host( binding.name());
	host->source = binding.source();

	return host;
}

res_texture* resource_manager::create_texture( LPCSTR phisical_name, u32 mip_level_cut, bool use_pool)
{
	// DBG_VerifyTextures	();
	if( 0 == strcmp( phisical_name,"null"))
		return 0;

	R_ASSERT( phisical_name && phisical_name[0]);

#ifdef	DEBUG
	//simplify_texture( tex_name);
#endif	//	DEBUG

	map_texture::iterator it = m_texture_registry.find( phisical_name);

	if( it!=m_texture_registry.end())
		return	it->second;
	else
		return load_texture( phisical_name, mip_level_cut, use_pool);
}

void resource_manager::on_texture_loaded( resources::queries_result& data, u32 mip_level_cut)
{
	fixed_string<512> name( data[0].get_requested_path());

	if( !data.is_successful() || !data[0].get_managed_resource())
	{
		LOG_ERROR( "Texture %s not found!", name.c_str());
		return;
	}

	resources::pinned_ptr_const<u8>	ptr	( data[0].get_managed_resource());


	name = name.substr( 19);
	name = name.substr( 0, name.length()-4);

	ref_texture tex = create_texture( name.c_str());

	D3DX_IMAGE_INFO	dds_info = {0};
	R_CHK( D3DXGetImageInfoFromMemory( ptr.c_ptr(), ptr.size(), 0, &dds_info, 0));

	D3DX_IMAGE_LOAD_INFO load_info;

	bool staging = false;
	if( staging)
	{
		load_info.Usage			= D3D_USAGE_STAGING;
		load_info.BindFlags		= 0;
		load_info.CpuAccessFlags = D3D_CPU_ACCESS_WRITE;
	}
	else
	{
		load_info.Usage		= D3D_USAGE_DEFAULT;
		load_info.BindFlags	= D3D_BIND_SHADER_RESOURCE;
	}
		
	ID3DBaseTexture* base_tex = NULL;
	HRESULT res = D3DXCreateTextureFromMemory( device::ref().d3d_device(), 
												ptr.c_ptr(), 
												ptr.size(),
												&load_info,
												0,
												&base_tex,
												0);
	R_CHK( res);
	tex->set_hw_texture( base_tex, mip_level_cut);
	base_tex->Release();
}

void resource_manager::on_texture_loaded_staging( resources::queries_result& data, u32 mip_level_cut)
{
/////////////////////////////////////////////////////////////////////////
// Code to use with texture cooker
// 	fixed_string<512> name( data[0].get_requested_path());
// 
// 	if( !data.is_successful() || !data[0].get_unmanaged_resource())
// 	{
// 		LOG_ERROR( "Texture %s not found!", name.c_str());
// 		return;
// 	}
// 
// 	resources::unmanaged_resource_ptr ptr = data[0].get_unmanaged_resource();
// 
// 	render::texture_data_resource* resource = static_cast<render::texture_data_resource*>(ptr.c_ptr());
// 
// 	name = name.substr( 19);
// 	name = name.substr( 0, name.length()-4);
// 
// 	u8 const *	dds_ptr		= (u8 const *) resource->m_buffer.c_ptr();
// 	u32			dds_size	= resource->m_buffer.size();

	fixed_string<512> name( data[0].get_requested_path());
 	
	ref_texture tex = create_texture( name.c_str());

	if( !data.is_successful() || !data[0].get_managed_resource())
	{
		LOG_ERROR( "Texture %s not found!", name.c_str());
		return;
	}

	resources::pinned_ptr_const<u8>	ptr	( data[0].get_managed_resource());

	u8 const *	dds_ptr		= ptr.c_ptr();
	u32			dds_size	= ptr.size();

	dds_info dds_info;
	if( !load_dds_info( dds_ptr, dds_size, dds_info))
	{
		on_texture_loaded( data);
		return;
	}

	if( dds_info.mips_count != utils::calc_mipmap_count( dds_info.width, dds_info.height))
	{
		on_texture_loaded( data);
		return;
	}

	if( mip_level_cut >= dds_info.mips_count-1 )
	{
		ASSERT( 0);
		return;
	}

	u32 resize_factor = 1 << mip_level_cut;

	u32 orig_size =  utils::calc_texture_size( dds_info.width, dds_info.height, dds_info.format, dds_info.mips_count);

	dds_info.width		/=	resize_factor;
	dds_info.height		/=	resize_factor;
	dds_info.mips_count -=	mip_level_cut;
	dds_info.data_size	=	utils::calc_texture_size( dds_info.width, dds_info.height, dds_info.format, dds_info.mips_count);
	dds_info.data_ptr	+=	( orig_size - dds_info.data_size);

	res_texture * pool_texture		= m_texture_storage->get( dds_info.width, dds_info.height, dds_info.format);
	res_texture * staging_texture	= m_texture_storage_staging->get( dds_info.width, dds_info.height, dds_info.format);

	if( !pool_texture || !staging_texture)
		return;

	tex->set_hw_texture( pool_texture->get_hw_texture());

	load_dds_to_texture( dds_info, staging_texture);

	device::ref().d3d_context()->CopyResource( tex->get_hw_texture(), staging_texture->get_hw_texture());
	
	m_texture_storage_staging->release( staging_texture);
}

res_texture*	resource_manager::load_texture				( char const * texture_name, u32 mip_level_cut, bool use_pool)
{
	if( !use_pool)
	{
		res_texture* tex = NEW( res_texture);
		tex->mark_registered();
		m_texture_registry.insert( utils::mk_pair( tex->set_name( texture_name), tex));

		if( strstr( tex->get_name(), "$user$") == 0)
		{
			fixed_string<512> path( "resources/textures/");
			path += tex->get_name();
			path += ".dds";
			ASSERT( strstr( path.c_str(), "\\")==0);
			//fix_path( path.get_buffer());
			resources::query_resource( path.c_str(), 
				resources::raw_data_class, 
				boost::bind( &resource_manager::on_texture_loaded, this, _1, mip_level_cut ),
				::xray::render::g_allocator);
		}

		return tex;
	}
	else
	{
		res_texture* tex = NEW( res_texture)( true);
		tex->mark_registered();

		m_texture_registry.insert( utils::mk_pair( tex->set_name( texture_name), tex));

		ASSERT( strstr( tex->get_name(), "$user$") == 0);

		fixed_string<512> path( "resources/textures/");
		path += tex->get_name();
		path += ".dds";
		ASSERT( strstr( path.c_str(), "\\")==0);
		//fix_path( path.get_buffer());
		resources::query_resource( path.c_str(), 
			resources::raw_data_class, 
			boost::bind( &resource_manager::on_texture_loaded_staging, this, _1, mip_level_cut ),
			::xray::render::g_allocator);

		
		return tex;
	}
}

res_texture* resource_manager::create_texture2d_impl ( u32 width, u32 height, D3D_SUBRESOURCE_DATA const* data, DXGI_FORMAT format, D3D_USAGE usage, u32 mip_levels, u32 array_size )
{
	D3D_TEXTURE2D_DESC texure_desc;
	ZeroMemory( &texure_desc, sizeof(texure_desc));
	texure_desc.Width				= width;
	texure_desc.Height				= height;
	texure_desc.Format				= format;
	texure_desc.SampleDesc.Count	= 1; 
	texure_desc.SampleDesc.Quality	= 0; 
	texure_desc.Usage				= usage;
	texure_desc.MipLevels			= mip_levels;
	texure_desc.ArraySize			= array_size;
	texure_desc.BindFlags			= (usage & D3D_USAGE_STAGING) ? 0 : D3D_BIND_SHADER_RESOURCE;
	texure_desc.CPUAccessFlags		= ((usage & D3D_USAGE_DYNAMIC) || (usage & D3D_USAGE_STAGING)) ? D3D_CPU_ACCESS_WRITE : 0;

	ID3DTexture2D*	d3d_texture;
	HRESULT res = device::ref().d3d_device()->CreateTexture2D( &texure_desc, data, &d3d_texture);
	R_CHK( res);

	res_texture* tex = NEW		( res_texture);

	tex->set_hw_texture			( d3d_texture, 0, (usage & D3D_USAGE_STAGING) > 0);

	d3d_texture->Release		();

	return tex;
}

void resource_manager::release_impl	( res_texture* texture)
{
	DELETE( texture, resource_manager_call_destructor_predicate());
}

res_texture* resource_manager::create_texture2d( char const* user_name, u32 width, u32 height, D3D_SUBRESOURCE_DATA const * data, DXGI_FORMAT format, D3D_USAGE usage, u32 mip_levels, u32 array_size)
{
	res_texture* tex = create_texture2d_impl( width, height, data, format, usage, mip_levels, array_size);

	m_texture_registry.insert	( utils::mk_pair( tex->set_name( user_name), tex));
	tex->mark_registered	();

	return tex;
}

res_texture* resource_manager::create_texture3d( char const * user_name, u32 width, u32 height, u32 depth, D3D_SUBRESOURCE_DATA const * data, DXGI_FORMAT format, u32 usage, u32 mip_levels)
{
	D3D_TEXTURE3D_DESC	desc;
	desc.Width			= width;
	desc.Height			= height;
	desc.Depth			= depth;
	desc.MipLevels		= mip_levels;
	desc.Format			= format;
	desc.Usage			= (D3D_USAGE)usage;
	desc.BindFlags		= D3D_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags		= 0;

	ID3DTexture3D* d3d_texture = NULL;
	R_CHK( device::ref().d3d_device()->CreateTexture3D( &desc, data, &d3d_texture));

	res_texture* tex = NEW		( res_texture);
	m_texture_registry.insert	( utils::mk_pair( tex->set_name( user_name), tex));

	tex->mark_registered			();
	tex->set_hw_texture		( d3d_texture);
	d3d_texture->Release	();

	return tex;
}

void resource_manager::release( res_texture* texture)
{
	// DBG_VerifyTextures	();

	if( !texture->registered())
		return;

	LPSTR name = LPSTR( texture->get_name());
	map_texture::iterator it = m_texture_registry.find( name);

	if( it!=m_texture_registry.end())
	{
		m_texture_registry.erase( it);
		release_impl( texture);
		return;
	}

	LOG_ERROR( "! ERROR: Failed to find texture surface '%s'", texture->get_name());
}


struct load_texture_delegate
{
	load_texture_delegate( ref_texture dest_texture, math::rectangle<int2> dest_rect, u32 arr_ind, char const* user_name):
			dest_texture		( dest_texture),
			dest_rect			( dest_rect),
			user_name			( user_name),
			arr_ind				( arr_ind)
			{}


	void execute( resources::queries_result& data)
	{
		ASSERT		( data.is_successful());

		if( !data[0].get_managed_resource())
			return;

		RECT rect;
		rect  = *( ( RECT*)&dest_rect);
		RECT src_rect;
		src_rect.left = src_rect.top = 0;
		src_rect.right = dest_rect.width();
		src_rect.bottom = dest_rect.height();

		resources::pinned_ptr_const<u8> ptr  ( data[0].get_managed_resource());

		D3DX_IMAGE_INFO	dds_info = {0};
		R_CHK( D3DXGetImageInfoFromMemory( ptr.c_ptr(), ptr.size(), 0, &dds_info, 0));

		D3D_TEXTURE2D_DESC	dest_desc	= dest_texture->get_desc ();

		if(	dds_info.Width		!= dest_desc.Width 
			|| dds_info.Height		!= dest_desc.Height 
			|| dds_info.Depth		!= 1
			|| dds_info.MipLevels	< dest_desc.MipLevels
			|| dds_info.Format		!= dest_desc.Format)
		{
			LOG_ERROR( "The loaded image is not compatible with destination texture!" ) ;
			return;
		}

		D3DX_IMAGE_LOAD_INFO load_info;

		load_info.Usage				= D3D_USAGE_DEFAULT;
		load_info.BindFlags			= 0;
		load_info.CpuAccessFlags	= 0;

		ID3DBaseTexture*	src_tex	= NULL;

		HRESULT res = D3DXCreateTextureFromMemory( device::ref().d3d_device(), 
			ptr.c_ptr(), 
			ptr.size(),
			&load_info, 
			0,
			&src_tex,
			0);

		R_CHK( res);

		for( u32 i = 0; i< dest_desc.MipLevels; ++i)
			//if( i%2 == 0)
			device::ref().d3d_context()->CopySubresourceRegion( dest_texture->get_hw_texture(), D3D10CalcSubresource( i, arr_ind, dest_desc.MipLevels), 0, 0, 0, src_tex, i, NULL);

		DELETE ((load_texture_delegate*)this);
	}

private:
	math::rectangle<int2>	dest_rect;
	ref_texture				dest_texture;
	char const*				user_name;
	u32						arr_ind;
};

bool resource_manager::copy_texture_from_file( ref_texture dest_texture, math::rectangle<int2> dest_rect, u32 arr_ind, char const* src_name)
{
	fixed_string<512> path( "resources/textures/");

	path += src_name;
	path += ".dds";
	ASSERT( strstr( path.c_str(), "\\")==0);
	

	load_texture_delegate* load_delegate = NEW ( load_texture_delegate) ( dest_texture, dest_rect, arr_ind, src_name);

	resources::query_resource( path.c_str(), 
		resources::raw_data_class, 
		boost::bind( &load_texture_delegate::execute, load_delegate, _1 ),
		::xray::render::g_allocator);

	return true;
}

res_buffer* resource_manager::create_buffer ( u32 size, const void * data, enum_buffer_type type, bool dynamic)
{
	res_buffer* new_buffer = NEW(res_buffer)( size, data, type, dynamic);

	m_buffers.push_back( new_buffer);

	return new_buffer;
}

void resource_manager::release	( res_buffer * buffer)
{
	if( reclaim( m_buffers, buffer))
	{
		DELETE( buffer, resource_manager_call_destructor_predicate());
		return;
	}
	
// 	// DELETE_PRIVATE implementation
// 	template < typename T >
// 	struct same_type_tester {
// 		static boost::yes	test( T* );
// 		static boost::no	test( ... );
// 	}; // struct same_type_tester
// 
// 	typedef res_buffer type;
// 	type* instance = buffer;
// 	COMPILE_ASSERT		( sizeof( same_type_tester<type>(instance) ) == sizeof( boost::yes ), Please_specify_correct_type );
// 	
// 	COMPILE_ASSERT		( !boost::is_polymorphic<res_buffer>::value, We_should_use_dynamic_cast_with_void_star_to_get_real_pointer_for_memory_allocator );
// 	buffer->~res_buffer	( );
// 	FREE				( buffer );

	ASSERT("The buffer not found in resource manager resources!");
}

res_state* resource_manager::create_state( state_descriptor & descriptor)
{
	ID3DRasterizerState*	rasterizer_state	=	m_rs_cache.get_state ( descriptor.m_rasterizer_desc);
	ID3DDepthStencilState*	depth_stencil_state =	m_dss_cache.get_state( descriptor.m_depth_stencil_desc);
	ID3DBlendState*			blend_state			=	m_bs_cache.get_state( descriptor.m_effect_desc);


	res_state* new_state = NEW( res_state)( rasterizer_state, 
											depth_stencil_state, 
											blend_state, 
											descriptor.m_stencil_ref);
	
	new_state->mark_registered();
	new_state->set_id	( m_states.size()+1);
	m_states.push_back	( new_state);

	return new_state;
}
void resource_manager::release( res_state * state)
{
	if( !state->registered())
		return;

	if( reclaim_with_id( m_states, state))
	{
		DELETE( state, resource_manager_call_destructor_predicate());
		return;
	}

	LOG_ERROR( "!ERROR: Failed to find compiled stateblock");
}

// res_declaration* resource_manager::create_declaration( DWORD fvf)
// {
// 	D3DVERTEXELEMENT9	dcl[MAX_FVF_DECL_SIZE];
// 	
// 	R_CHK( D3DXDeclaratorFromFVF( fvf, dcl));
// 	return create_declaration( dcl);
// }

res_declaration* resource_manager::create_declaration( D3D_INPUT_ELEMENT_DESC const* dcl, u32 count)
{
	// Search equal code
	for ( u32 it=0; it<m_declarations.size(); it++)
	{
		res_declaration* decl = m_declarations[it];
		if( decl->equal( dcl, count))
			return decl;
	}

	// Create _new
	res_declaration* new_decl = NEW( res_declaration)( dcl, count);
	new_decl->mark_registered();
	m_declarations.push_back( new_decl);

	return new_decl;
}

void resource_manager::release( res_declaration* dcl)
{
	if( !dcl->registered())
		return;

	if( reclaim( m_declarations, dcl))
	{
		DELETE( dcl, resource_manager_call_destructor_predicate());
		return;
	}

	LOG_ERROR( "! ERROR: Failed to find compiled vertex-declarator");
}

res_signature*	resource_manager::create_signature	( ID3DBlob * signature)
{
	for ( u32 it=0; it<m_signatures.size(); it++)
	{
		if( m_signatures[it]->equal( signature))
			return m_signatures[it];
	}

	// Create _new
	res_signature* new_signature = NEW( res_signature)( signature);
	new_signature->mark_registered();
	m_signatures.push_back( new_signature);

	return new_signature;
}

void  resource_manager::release		( res_signature * signature)
{
	if( !signature->registered())
		return;

	if( reclaim( m_signatures, signature))
	{
		DELETE( signature, resource_manager_call_destructor_predicate());
		return;
	}

	LOG_ERROR( "! ERROR: Failed to find created signature.");
}

res_input_layout*	resource_manager::create_input_layout	( res_declaration const * decl, res_signature const * signature)
{
	// Search equal code
	for ( u32 it=0; it<m_input_layouts.size(); it++)
	{
		res_input_layout* layout = m_input_layouts[it];
		if( layout->equal( decl, signature))
			return layout;
	}

	// Create _new
	res_input_layout* new_layout = NEW( res_input_layout)( decl, signature);
	new_layout->mark_registered();
	m_input_layouts.push_back( new_layout);

	return new_layout;
}

void	resource_manager::release		( res_input_layout * layout)
{
	if( !layout->registered())
		return;

	if( reclaim( m_input_layouts, layout))
	{
		DELETE( layout, resource_manager_call_destructor_predicate());
		return;
	}

	LOG_ERROR( "! ERROR: Failed to find created layout");
}


res_rt*	resource_manager::create_render_target( LPCSTR name, u32 w, u32 h, DXGI_FORMAT fmt, res_rt::enum_usage usage)
{
	R_ASSERT( name && name[0] && w && h);

	// ***** first pass - search already created RT
	map_rt::iterator it = m_rt_registry.find( name);

	if( it != m_rt_registry.end())
	{
		return it->second;
	}
	else
	{
		res_rt*	rt = NEW( res_rt);
		rt->mark_registered();
		m_rt_registry.insert( utils::mk_pair( rt->set_name( name), rt));

		//if( Device.b_is_Ready)
			rt->create( name, w, h, fmt, usage);

		return rt;
	}
}

void resource_manager::release( res_rt*	rt)
{
	if( !rt->registered())
		return;

	LPCSTR name = rt->get_name();
	map_rt::iterator it	= m_rt_registry.find( name);

	if( it != m_rt_registry.end())
	{
		m_rt_registry.erase( it);
		DELETE( rt, resource_manager_call_destructor_predicate());
		return;
	}

	LOG_ERROR( "! ERROR: Failed to find render-target '%s'",rt->m_name.c_str());
}

void resource_manager::evict()
{
	// --Porting to DX10_
	// Need to figure out if we need this in DX10 DX11
	//device::ref().d3d_context()->EvictManagedResources();
}

res_constant_buffer*	resource_manager::create_constant_buffer	( name_string const & name, enum_shader_type dest, D3D_CBUFFER_TYPE type, u32 size)
{
	for ( u32 it=0; it<m_const_buffers.size(); it++)
	{
		res_constant_buffer*	buf		= m_const_buffers[it];
		if( buf->type()		== type 
			&& buf->size()		== size
			&& buf->dest()		== dest
			&& buf->name()		== name)			
				return buf;
	}
	res_constant_buffer* cbuffer = NEW( res_constant_buffer)( name, dest, type, size);

	cbuffer->mark_registered();
	m_const_buffers.push_back	( cbuffer);
	return cbuffer;
}

void	resource_manager::release ( res_constant_buffer* cbuffer)
{
	if( !cbuffer->registered())
		return;

	if( reclaim( m_const_buffers, cbuffer))
	{
		DELETE( cbuffer, resource_manager_call_destructor_predicate());
		return;
	}

	LOG_ERROR( "!ERROR: Failed to find constant buffer");
}

res_sampler_state*	resource_manager::create_sampler_state	( sampler_state_descriptor const & sampler_props)
{
	return m_sampler_cache.get_state( sampler_props.m_desc);
}

res_texture_list* resource_manager::create_texture_list( texture_slots const & tex_list)
{
	for( u32 it = 0; it < m_texture_lists.size(); it++)
	{
		res_texture_list* base = m_texture_lists[it];
		if( base->equal( tex_list))
			return base;
	}

	res_texture_list* lst = NEW( res_texture_list)( tex_list);
	lst->mark_registered();
	m_texture_lists.push_back( lst);
	lst->set_id( m_texture_lists.size());
	return lst;
}

void resource_manager::release( res_texture_list* tex_list)
{
	if( !tex_list->registered())
		return;

	if( reclaim_with_id( m_texture_lists, tex_list))
	{
		DELETE( tex_list, resource_manager_call_destructor_predicate());
		return;
	}

	LOG_ERROR( "!ERROR: Failed to find compiled list of textures");
}


res_sampler_list* resource_manager::create_sampler_list	( sampler_slots const & smp_list)
{
	for( u32 it = 0; it < m_sampler_lists.size(); it++)
	{
		res_sampler_list* base = m_sampler_lists[it];
		if( base->equal( smp_list))
			return base;
	}

	res_sampler_list* lst = NEW( res_sampler_list)( smp_list);
	lst->mark_registered();
	m_sampler_lists.push_back( lst);
	lst->set_id( m_sampler_lists.size());
	return lst;

}

void resource_manager::release		( res_sampler_list * smp_list)
{
	if( !smp_list->registered())
		return;

	if( reclaim_with_id( m_sampler_lists, smp_list))
	{
		DELETE( smp_list, resource_manager_call_destructor_predicate());
		return;
	}

	LOG_ERROR( "!ERROR: Failed to find compiled list of samplers");
}

template <class shader, class binder>
bool operator == ( shader const * sh, binder const & bn)
{
	return  sh->equal( bn);
}

res_vs*	resource_manager::create_vs( vs_descriptor const & binder)
{
	vs_cache::iterator it = std::find( m_v_shaders.begin(), m_v_shaders.end(), binder);

	if( it != m_v_shaders.end())
	{
		return *it;
	}
	else
	{
		res_vs * vs = NEW(res_vs)( binder);
		vs->mark_registered();
		m_v_shaders.push_back( vs);
		vs->set_id( m_v_shaders.size());
		return vs;
	}
}

void resource_manager::release	( res_vs * vs)
{
	if( !vs->registered())
		return;

	if( reclaim_with_id( m_v_shaders, vs))
	{
		DELETE( vs, resource_manager_call_destructor_predicate());
		return;
	}

	LOG_ERROR( "!ERROR: Failed to find VS.");
}

res_ps* resource_manager::create_ps( ps_descriptor const & binder)
{
	ps_cache::iterator it = std::find( m_p_shaders.begin(), m_p_shaders.end(), binder);

	if( it != m_p_shaders.end())
	{
		return *it;
	}
	else
	{
		res_ps * ps = NEW(res_ps)( binder);
		ps->mark_registered();
		m_p_shaders.push_back( ps);
		ps->set_id( m_p_shaders.size());

		return ps;
	}
}

void resource_manager::release	( res_ps * ps)
{
	if( !ps->registered())
		return;

	if( reclaim_with_id( m_p_shaders, ps))
	{
		DELETE( ps, resource_manager_call_destructor_predicate());
		return;
	}

	LOG_ERROR( "!ERROR: Failed to find PS.");
}

res_geometry* resource_manager::create_geometry( D3D_INPUT_ELEMENT_DESC const* decl, u32 decl_size, u32 vertex_stride, res_buffer* vb, res_buffer* ib)
{
	R_ASSERT( decl);

	ref_declaration dcl	= resource_manager::ref().create_declaration( decl, decl_size);

	return create_geometry( &*dcl, vertex_stride, vb, ib);
}

res_geometry* resource_manager::create_geometry( res_declaration * dcl, u32 vertex_stride, res_buffer * vb, res_buffer * ib)
{
	R_ASSERT( vb);
	u32 vb_stride = vertex_stride;
	res_geometry g( vb, ib, dcl, vb_stride);

	vec_geoms::const_iterator it = m_geometries.begin();
	vec_geoms::const_iterator end = m_geometries.end();
	for( ; it != end ; ++it)
	{
		if( (*it)->equal( g))
			return *it;
	}

	res_geometry* geom = NEW( res_geometry)( vb, ib, dcl, vb_stride);
	geom->mark_registered();
	m_geometries.push_back( geom);

	return	geom;
}

void resource_manager::release( res_geometry* geom)
{
	if( !geom->registered())
		return;

	if( reclaim( m_geometries, geom))
	{
		DELETE( geom, resource_manager_call_destructor_predicate());
		return;
	}
	LOG_ERROR( "!ERROR: Failed to find complete shader");
}

void resource_manager::set_compile_error_handler	( shader_compile_error_handler const & handler)
{
	m_compile_error_handler = handler;
}

} // namespace render 
} // namespace xray 


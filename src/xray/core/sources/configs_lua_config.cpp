////////////////////////////////////////////////////////////////////////////
//	Created		: 06.03.2009
//	Author		: Dmitriy Iassenev
//	Description	: for the sake of compilation time only, several classes put
//				  their implementation here, do not use this 'technique' for
//				  any other purposes in the projects
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "script_engine_wrapper.h"
#include <boost/crc.hpp>
#include <xray/configs.h>
#include <xray/strings_stream.h>
#include <xray/buffer_string.h>
#include "configs_lua_config_cook.h"
#include "configs_binary_config_cook.h"
#include "resources_manager.h"
#include <xray/fs_utils.h>

#ifdef DEBUG
#	define CS_LUA_USE_DEBUG_LIBRARY
#	define CS_LUABIND_USE_DEBUG_LIBRARY
#	define CS_CORE_USE_DEBUG_LIBRARY
#	define CS_SCRIPT_USE_DEBUG_LIBRARY
#endif // #ifdef DEBUG

#include <cs/lua/library_linkage.h>
#include <luabind/library_linkage.h>
#include <cs/core/library_linkage.h>
#include <cs/script/library_linkage.h>
#include <cs/core/memory.h>

using xray::core::script_engine_wrapper;

static xray::uninitialized_reference<script_engine_wrapper>		s_wrapper;
static xray::uninitialized_reference<xray::threading::mutex>	s_mutex;

static cs::script::world* s_world	= 0;
static pcstr s_super_tables_id		= "__super_tables__";
static pcstr s_super_tables_ids_id	= "__super_tables";

namespace xray {
namespace core {
namespace configs {
	extern memory::doug_lea_allocator_type	g_lua_allocator;
} // namespace configs
} // namespace core
} // namespace xray


static void* CS_CALL allocate		( luabind::memory_allocation_function_parameter, void const* current_buffer, size_t const needed_size )
{
	xray::core::configs::g_lua_allocator.user_current_thread_id	( );

	pvoid buffer					= const_cast<pvoid>(current_buffer);
	if ( !needed_size ) {
		XRAY_FREE_IMPL				( xray::core::configs::g_lua_allocator, buffer );
		return						0;
	}

	if ( !current_buffer )
		return						XRAY_MALLOC_IMPL( xray::core::configs::g_lua_allocator, (u32)needed_size, "lua" );

	return							XRAY_REALLOC_IMPL( xray::core::configs::g_lua_allocator, buffer, (u32)needed_size, "lua" );
}

#include <luabind/lua_include.hpp>

static void export_classes			( );

class lua_stack_guard {
public:
	inline	lua_stack_guard			( bool const check_for_null = true )
	{
		XRAY_UNREFERENCED_PARAMETER	( check_for_null );
		s_mutex->lock				( );
		m_top						= lua_gettop( s_world->virtual_machine() );
//		R_ASSERT					( !check_for_null || !m_top, "%d", m_top );
	}

	inline	~lua_stack_guard		( )
	{
		int const top				= lua_gettop( s_world->virtual_machine() );
		XRAY_UNREFERENCED_PARAMETER	( top );
		R_ASSERT_CMP				( m_top, ==, top );
		s_mutex->unlock				( );
	}

private:
	int	m_top;
}; // struct lua_stack_guard

class lua_mutex_guard {
public:
	inline	lua_mutex_guard			( )
	{
		s_mutex->lock				( );
	}

	inline	~lua_mutex_guard		( )
	{
		s_mutex->unlock				( );
	}

private:
	int	m_top;
}; // struct lua_stack_guard

namespace xray {
namespace core {
namespace configs {

void initialize						( pcstr resource_path, pcstr underscore_G_path )
{
	cs::core::memory_allocator		( &allocate, 0 );

	XRAY_CONSTRUCT_REFERENCE		( s_wrapper, script_engine_wrapper ) ( resource_path, underscore_G_path );
	XRAY_CONSTRUCT_REFERENCE		( s_mutex, threading::mutex );

	R_ASSERT						( !s_world );
	s_world							= cs_script_create_world( *s_wrapper );
	lua_pop							( s_world->virtual_machine(), lua_gettop( s_world->virtual_machine() ) );

	lua_stack_guard					guard;

	export_classes					();
}

void finalize						( )
{
	R_ASSERT						( s_world );
	cs_script_destroy_world			( s_world );
	R_ASSERT						( !s_world );

	XRAY_DESTROY_REFERENCE			( s_mutex );
	XRAY_DESTROY_REFERENCE			( s_wrapper );
}

} // namespace configs
} // namespace core
} // namespace xray

#include <xray/configs.h>

#ifdef NDEBUG
namespace std {
	inline void terminate()
	{
		UNREACHABLE_CODE();
	}
} // namespace std
#endif // #ifdef NDEBUG

using xray::configs::lua_config;

////////////////////////////////////////////////////////////////////////////
//	class xray::configs::lua_config
////////////////////////////////////////////////////////////////////////////

lua_config::lua_config				(
		pcstr const file_name,
		luabind::object const& object
	) :
	m_root							( object )
{
	lua_mutex_guard					mutex_guard;

	xray::core::configs::g_lua_allocator.user_current_thread_id	( );
	m_file_name						= file_name ? strings::duplicate( core::configs::g_lua_allocator, file_name ) : 0;
}

lua_config::~lua_config				( )
{
	lua_mutex_guard					mutex_guard;

	xray::core::configs::g_lua_allocator.user_current_thread_id	( );
	XRAY_FREE_IMPL					( core::configs::g_lua_allocator, m_file_name );
}

//#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
//#include <share.h>

void lua_config::save_as			( pcstr file_name ) const
{
	lua_mutex_guard					mutex_guard;
	xray::core::configs::g_lua_allocator.user_current_thread_id	( );

	string4096						indent;
	indent[0]						= 0;

	strings::stream					stream( &core::configs::g_lua_allocator);
	m_root.save						( stream, indent, sizeof(indent), 0 );

	//int	const open_flags			= _O_RDWR | _O_BINARY | _O_SEQUENTIAL | _O_CREAT | _O_TRUNC;
	//int								file_handle;
	//errno_t error_code				= _sopen_s( &file_handle, file_name, open_flags, _SH_DENYNO, _S_IREAD | _S_IWRITE );

	//if ( error_code ) {
	//	xray::threading::yield		( 1 );
	//	error_code					= _sopen_s( &file_handle, file_name, open_flags, _SH_DENYNO, _S_IREAD | _S_IWRITE );
	//}

	//R_ASSERT						( !error_code, file_name );
	//R_ASSERT						( file_handle != -1, file_name );

	//FILE* const file				= _fdopen( file_handle, "r+b" );
	FILE* file;
	bool const result				= fs::open_file( &file, fs::open_file_write | fs::open_file_create | fs::open_file_truncate, fs::convert_to_portable(file_name).c_str() );
	ASSERT_U						( result, "%s", file_name );
	fwrite							( stream.get_buffer( ), 1, stream.get_buffer_size( ), file );
	fclose							( file );
}

void lua_config::save				( ) const
{
	save_as							( m_file_name );
}

void lua_config::save				( xray::strings::stream& stream ) const
{
	string4096						indent;
	indent[0]						= 0;

	m_root.save						( stream, indent, sizeof(indent), 0 );
}

static void collect_garbage						( )
{
	lua_State* const state			= s_world->virtual_machine( );
	lua_gc							( state, LUA_GCCOLLECT, 0 );
	lua_gc							( state, LUA_GCCOLLECT, 0 );
}

static xray::configs::lua_config* new_config			( pcstr file_name );
static xray::configs::lua_config* new_config_inplace	( xray::mutable_buffer const & lua_config_buffer, pcstr file_name );

namespace xray {
namespace core {
namespace configs {

xray::configs::lua_config* create_lua_config	( pcstr file_name, memory::reader& reader )
{
	lua_stack_guard					guard;

	pstr temp						= 0;
	STR_JOINA						( temp, "modules['", file_name, "']" );

	collect_garbage					( );
	s_world->execute_script			( (pcstr)reader.pointer(), reader.elapsed(), temp );
	collect_garbage					( );

	return							new_config( file_name );
}

xray::configs::lua_config*   create_lua_config_inplace	( mutable_buffer const & in_out_buffer, pcstr file_name, memory::reader& reader )
{
	lua_stack_guard					guard;

	pstr temp						= 0;
	STR_JOINA						( temp, "modules['", file_name, "']" );

	collect_garbage					( );
	s_world->execute_script			( (pcstr)reader.pointer(), reader.elapsed(), temp );
	collect_garbage					( );

	return							new_config_inplace( in_out_buffer, file_name );
}

pvoid lua_allocate							( u32 const size )
{
	lua_mutex_guard					mutex_guard;
	xray::core::configs::g_lua_allocator.user_current_thread_id	( );
	return							XRAY_MALLOC_IMPL( core::configs::g_lua_allocator, size, "lua" );
}

void destroy_lua_config						( xray::resources::unmanaged_resource* config )
{
	lua_config*	real_config			= static_cast_checked<lua_config*>(config);
	real_config->~lua_config		( );
}

void lua_deallocate							( pvoid buffer )
{
	lua_mutex_guard					mutex_guard;
	lua_stack_guard					stack_guard;
	xray::core::configs::g_lua_allocator.user_current_thread_id	( );
	XRAY_FREE_IMPL					( core::configs::g_lua_allocator, buffer );
	collect_garbage					( );
}

} // namespace configs
} // namespace core
} // namespace xray

using xray::configs::lua_config_ptr;

static xray::configs::lua_config_value get_value	(
		xray::configs::lua_config_value& result,
		xray::configs::binary_config_value::const_iterator const i,
		xray::configs::binary_config_value const& value,
		u32 const index
	)
{
	if ( value.type == xray::configs::t_table_indexed )
		return						result[index];

	R_ASSERT						( value.type == xray::configs::t_table_named );
	return							result[ (*i).id ];
}

static void construct				( xray::configs::lua_config_value result, xray::configs::binary_config_value const& value )
{
	using namespace xray::configs;

	binary_config_value::const_iterator	i = value.begin();
	binary_config_value::const_iterator	e = value.end();
	for (u32 j=0; i != e; ++i, ++j ) {
		switch ( (*i).type ) {
			case t_boolean : {
				get_value( result, i, value, j)	= (bool)*i;
				break;
			}
			case t_integer : {
				get_value( result, i, value, j)	= (int)*i;
				break;
			}
			case t_float : {
				get_value( result, i, value, j)	= (float)*i;
				break;
			}
			case t_table_named : {
				construct						( get_value( result, i, value, j), *i);
				break;
			}
			case t_table_indexed : {
				construct						( get_value( result, i, value, j), *i);
				break;
			}
			case t_string : {
				get_value( result, i, value, j)	= (pcstr)*i;
				break;
			}
			case t_float2 : {
				get_value( result, i, value, j)	= (xray::math::float2 const&)*i;
				break;
			}
			case t_float3 : {
				get_value( result, i, value, j)	= (xray::math::float3 const&)*i;
				break;
			}
			case t_float4 : {
				get_value( result, i, value, j)	= (xray::math::float4 const&)*i;
				break;
			}
			default : NODEFAULT();
		}
	}
}

xray::configs::lua_config_ptr xray::configs::create_lua_config	( binary_config_ptr config )
{
	lua_config_ptr result			= create_lua_config( );
	construct						( result->get_root(), config->get_root() );
	return							result;
}

xray::configs::lua_config_ptr xray::configs::create_lua_config_from_string ( pcstr string )
{
	lua_mutex_guard					mutex_guard;
	lua_stack_guard					stack_guard;

	pcstr const file_name			= ":from_clipboard";

	pstr temp						= 0;
	STR_JOINA						( temp, "modules['", file_name, "']" );

	collect_garbage					( );
	s_world->execute_script			( string, strings::length(string), temp );
	collect_garbage					( );

 	lua_config * result				= new_config( file_name );
	resources::g_resources_manager->get_lua_config_cook().register_object_to_delete(result, threading::current_thread_id());
	result->set_creation_source		(resources::resource_base::creation_source_created_by_user, file_name, sizeof(lua_config));
	return							result;
}

using xray::configs::binary_config_value;
using xray::configs::binary_config_ptr;

static void construct				( xray::configs::lua_config_value const& value, binary_config_value& item )
{
	lua_mutex_guard					mutex_guard;
	xray::core::configs::g_lua_allocator.user_current_thread_id	( );

	using namespace xray::configs;

	item.type						= (u16)value.get_type();
	item.data						= xray::memory::uninitialized_value<pbyte>();
	item.count						= xray::memory::uninitialized_value<u16>();
	switch ( item.type ) {
		case t_boolean : {
			*(u32*)&item.data		= bool(value) ? 1 : 0;
			break;
		}
		case t_integer : {
			*(u32*)&item.data		= value;
			break;
		}
		case t_float : {
			*(float*)&item.data		= value;
			break;
		}
		case t_string : {
			item.data				= (pcstr)value;
			break;
		}
		case t_float2 : {
			item.data				= &(xray::math::float2 const&)value;
			break;
		}
		case t_float3 : {
			item.data				= &(xray::math::float3 const&)value;
			break;
		}
		case t_float4 : {
			item.data				= &(xray::math::float4 const&)value;
			break;
		}
		case t_table_indexed :
		case t_table_named : {
			typedef xray::vectora<binary_config_value>	items_type;
			items_type*	const items			= XRAY_NEW_IMPL( xray::core::configs::g_lua_allocator, items_type )( xray::core::configs::g_lua_allocator );
			items->reserve					( value.size() );
			item.data						= items;

			lua_config_value::const_iterator	i = value.begin();
			lua_config_value::const_iterator	e = value.end();
			for ( ; i != e; ++i ) {
				binary_config_value			sub_item;

				if ( value.get_type() != t_table_indexed ) {
					sub_item.id				= i.key();
					if ( !xray::strings::compare(s_super_tables_id, sub_item.id) || !xray::strings::compare(s_super_tables_ids_id, sub_item.id) )
						continue;

					boost::crc_32_type		processor;
					processor.process_block	( sub_item.id, sub_item.id + xray::strings::length(sub_item.id) );
					sub_item.crc			= processor.checksum();
				}
				else {
					if ( i.key().get_type() == t_string )
						continue;

					sub_item.id				= 0;
					sub_item.crc			= 0;
				}

				construct					( *i, sub_item );

				items->push_back			( sub_item );
			}

			break;
		}
		default : NODEFAULT();
	}
}

using xray::configs::lua_config_value;

static void sort					( binary_config_value& value  )
{
	using namespace xray::configs;

	switch ( value.type ) {
		case t_boolean :
		case t_integer :
		case t_float :
		case t_string :
		case t_float2 :
		case t_float3 :
		case t_float4 :
			break;

		case t_table_named :
		case t_table_indexed : {
			typedef xray::vectora<binary_config_value>	items_type;
			items_type* const items	= const_cast<items_type*>( static_cast<items_type const*>(value.data) );

			struct predicate {
				static inline bool compare	( binary_config_value const& left, binary_config_value const& right )
				{
					return			left.crc < right.crc;
				}
			}; // struct predicate

			std::sort				( items->begin(), items->end(), &predicate::compare );

			items_type::iterator i	= items->begin( );
			items_type::iterator const	e = items->end( );
			for ( ; i != e; ++i )
				sort				( *i );

			break;
		}
		default : NODEFAULT();
	}
}

static void optimize				( binary_config_value& value )
{
	sort							( value );
}

#include <xray/memory_stream.h>

static void construct				( binary_config_value& value, xray::memory::stream& fat, xray::memory::stream& data, int const offset )
{
	using namespace xray::configs;

	bool const iterate_only			= offset >= 0;
	if ( !iterate_only ) {
		pcstr const id				= value.id;

		if ( id ) {
			value.id				= xray::horrible_cast< u32, pcstr >( data.get_buffer_size() ).second;
			data.append				( id );
		}
	}

	switch ( value.type ) {
		case t_boolean :
		case t_integer :
		case t_float : {
			fat.append				( &value, sizeof(value) );
			break;
		}
		case t_string : {
			u32 const buffer_size	= data.get_buffer_size();
			data.append				( static_cast<pcstr>(value.data) );
			value.data				= xray::horrible_cast< u32, pcvoid >( buffer_size ).second;
			fat.append				( &value, sizeof(value) );
			break;
		}
		case t_float2 : {
			u32 const buffer_size	= data.get_buffer_size();
			data.append				( value.data, sizeof(xray::math::float2) );
			value.data				= xray::horrible_cast< u32, pcvoid >( buffer_size ).second;
			fat.append				( &value, sizeof(value) );
			break;
		}
		case t_float3 : {
			u32 const buffer_size	= data.get_buffer_size();
			data.append				( value.data, sizeof(xray::math::float3) );
			value.data				= xray::horrible_cast< u32, pcvoid >( buffer_size ).second;
			fat.append				( &value, sizeof(value) );
			break;
		}
		case t_float4 : {
			u32 const buffer_size	= data.get_buffer_size();
			data.append				( value.data, sizeof(xray::math::float4) );
			value.data				= xray::horrible_cast< u32, pcvoid >( buffer_size ).second;
			fat.append				( &value, sizeof(value) );
			break;
		}
		case t_table_indexed :
		case t_table_named : {
			typedef xray::vectora<binary_config_value>	items_type;
			items_type* items		= const_cast<items_type*>( static_cast<items_type const*>(value.data) );
			if ( !iterate_only ) {
				fat.append			( &value, sizeof(value) );
				break;
			}

			binary_config_value* value = xray::pointer_cast<binary_config_value*>(fat.get_buffer() + offset);
			value->data				= xray::horrible_cast< u32, pcvoid >( fat.get_buffer_size() ).second;
			R_ASSERT				( items->size() < 65536 );
			value->count			= (u16)items->size();

			u32 const new_offset	= fat.get_buffer_size( );

			items_type::iterator i	= items->begin( );
			items_type::iterator const	e = items->end( );
			for ( ; i != e; ++i )
				construct			( *i, fat, data, -1 );

			i						= items->begin( );
			for (u32 j = 0; i != e; ++i, ++j ) {
				if ( ((*i).type == t_table_named) || ((*i).type == t_table_indexed) )
					construct		( *i, fat, data, new_offset + j*sizeof(binary_config_value) );
			}

			break;
		}
		default : NODEFAULT();
	}
}

template < typename T >
static void convert					( T& value, u32 const offset )
{
	if ( !value )
		return;

	value							= xray::horrible_cast< u32, T >( xray::horrible_cast<T, u32>(value).second + offset ).second;
}

#include "configs_binary_config.h"

static void fill_info_for_fix_up		( binary_config_value& value, pbyte const buffer, u32 const value_offset, u32 const offset )
{
	lua_mutex_guard					mutex_guard;
	xray::core::configs::g_lua_allocator.user_current_thread_id	( );

	binary_config_value* const serialized = xray::pointer_cast<binary_config_value*>( buffer + value_offset );
	if ( value.id )
		serialized->id			+= offset;

	using namespace xray::configs;

	switch ( value.type ) {
		case t_boolean :
		case t_integer :
		case t_float : {
			break;
		}
		case t_string :
		case t_float2 :
		case t_float3 :
		case t_float4 : {
			(pcbyte&)(serialized->data)	+= offset;
			break;
		}
		case t_table_indexed :
		case t_table_named : {
			typedef xray::vectora<binary_config_value>	items_type;
			items_type* items		= const_cast<items_type*>( static_cast<items_type const*>(value.data) );

			items_type::iterator i	= items->begin( );
			items_type::iterator const	e = items->end( );
			for (u32 j=0; i != e; ++i, ++j )
				fill_info_for_fix_up( *i, buffer, xray::horrible_cast<pcvoid,u32>(serialized->data).second + j*sizeof(binary_config_value), offset );

			XRAY_DELETE_IMPL		( xray::core::configs::g_lua_allocator, items );
			break;
		}
		default : NODEFAULT();
	}
}

void xray::configs::create_binary_config_buffer	( lua_config_value value, xray::memory::stream& fat )
{
	lua_mutex_guard					mutex_guard;
	xray::core::configs::g_lua_allocator.user_current_thread_id	( );

	binary_config_value				root;
	root.id							= 0;
	root.crc						= 0;

	construct						( value, root );
	optimize						( root );

	xray::memory::stream			data( &core::configs::g_lua_allocator );
	data.append						( u32(0) );

	construct						( root, fat, data, -1 );
	construct						( root, fat, data, 0 );

	u32 const offset				= fat.get_buffer_size() - 4;
	fat.append						( data.get_buffer() + sizeof(u32), data.get_buffer_size() - sizeof(u32) );

	fill_info_for_fix_up			( root, fat.get_buffer(), 0, offset );
}

binary_config_ptr xray::configs::create_binary_config	( lua_config_value value )
{
	xray::memory::stream str( &core::configs::g_lua_allocator ); 
	create_binary_config_buffer(value, str);
	return create_binary_config(mutable_buffer(str.get_buffer(),str.get_buffer_size()));
}

binary_config_ptr xray::configs::create_binary_config	( mutable_buffer const& buffer )
{
	lua_mutex_guard					mutex_guard;
	xray::core::configs::binary_config * const result = 
		XRAY_NEW_IMPL( memory::g_mt_allocator, xray::core::configs::binary_config ) (
		(pcbyte)buffer.c_ptr(),
		buffer.size(),
		& memory::g_mt_allocator
		);

	resources::g_resources_manager->get_binary_config_cook().register_object_to_delete(result, threading::current_thread_id());
	return							result;
}

#include <luabind/luabind.hpp>

static xray::configs::lua_config *   new_config	( pcstr file_name )
{
	lua_mutex_guard					mutex_guard;
	xray::core::configs::g_lua_allocator.user_current_thread_id	( );
	
	xray::mutable_buffer buffer		(XRAY_MALLOC_IMPL( xray::core::configs::g_lua_allocator, sizeof(lua_config), "lua" ),
									 sizeof(lua_config));

	return							new_config_inplace( buffer, file_name );
}

static xray::configs::lua_config *   new_config_inplace	( xray::mutable_buffer const & in_out_config, pcstr file_name )
{
	R_ASSERT						(in_out_config.size() >= sizeof(xray::configs::lua_config));

	lua_mutex_guard					mutex_guard;
	xray::core::configs::g_lua_allocator.user_current_thread_id	( );

	lua_State* const state			= s_world->virtual_machine( );
	luabind::object	const modules	= luabind::globals( state )["modules"];
	luabind::object	const object	= modules[ file_name ];
	R_ASSERT						( luabind::type(object) == LUA_TTABLE, "cannot load lua configuration file[%s]\r\n see log file for details", file_name );

	xray::configs::lua_config * const out_result	=	new (in_out_config.c_ptr()) xray::configs::lua_config ( file_name, object );
	return							out_result;
}

lua_config_ptr xray::configs::create_lua_config	( mutable_buffer const & buffer, pcstr file_name )
{
	lua_mutex_guard					mutex_guard;
	lua_stack_guard					guard;

	xray::core::configs::g_lua_allocator.user_current_thread_id	( );

	lua_State* const state			= s_world->virtual_machine( );
	lua_newtable					( state );
	luabind::object					object( luabind::from_stack(state,-1) );
	lua_pop							( state, 1 );

	ASSERT							(buffer.size() >= sizeof(lua_config));
	lua_config * result				= new (buffer.c_ptr()) lua_config ( file_name, object );
	
	resources::g_resources_manager->get_lua_config_cook().register_object_to_delete(result, threading::current_thread_id());
	result->set_creation_source		(resources::resource_base::creation_source_created_by_user, file_name, buffer.size());

	return							result;
}

lua_config_ptr xray::configs::create_lua_config	( pcstr file_name )
{
	xray::core::configs::g_lua_allocator.user_current_thread_id	( );

	return
		create_lua_config(
			mutable_buffer( XRAY_MALLOC_IMPL( core::configs::g_lua_allocator, sizeof(lua_config), "lua_config" ),
							sizeof(lua_config) ),
			file_name
		);
}

lua_config_ptr xray::configs::create_lua_config	( pcstr file_name, lua_config_value const& value )
{
	lua_mutex_guard					mutex_guard;
	lua_stack_guard					guard;

	xray::core::configs::g_lua_allocator.user_current_thread_id	( );

	lua_config * result			= XRAY_NEW_IMPL( core::configs::g_lua_allocator, lua_config ) ( file_name, value.get_object( ) );
	
	resources::g_resources_manager->get_lua_config_cook().register_object_to_delete(result, threading::current_thread_id());
	result->set_creation_source		(resources::resource_base::creation_source_created_by_user, file_name, sizeof(lua_config));

	return							result;
}

using xray::configs::lua_config_value;

lua_config_value::lua_config_value				( ::luabind::object const& object) :
	m_object						( xray::pointer_cast< luabind::object* >(&m_object_fake[0]) ),
	m_table_object					( xray::pointer_cast< luabind::object* >(&m_table_object_fake[0]) ),
	m_field_id						( 0 ),
	m_fixed_up						( true )
{
	lua_mutex_guard					mutex_guard;
	initialize						( object, luabind::object() );
}

lua_config_value::lua_config_value				( ::luabind::object const& object, ::luabind::object const& table_object, pcstr field_id ) :
	m_object						( xray::pointer_cast< luabind::object* >(&m_object_fake[0]) ),
	m_table_object					( xray::pointer_cast< luabind::object* >(&m_table_object_fake[0]) ),
	m_fixed_up						( field_id ? !strchr( field_id, '.' ) : true )
{
	lua_mutex_guard					mutex_guard;
	xray::core::configs::g_lua_allocator.user_current_thread_id	( );

	m_field_id						= field_id ? xray::strings::duplicate( xray::core::configs::g_lua_allocator, field_id ) : 0;

	initialize						( object, table_object );
}

lua_config_value::~lua_config_value				( )
{
	lua_mutex_guard					mutex_guard;
	xray::core::configs::g_lua_allocator.user_current_thread_id	( );

	finalize						( );
}

lua_config_value::lua_config_value				( lua_config_value const& other ) :
	m_object						( xray::pointer_cast< luabind::object* >(&m_object_fake[0]) ),
	m_table_object					( xray::pointer_cast< luabind::object* >(&m_table_object_fake[0]) ),
	m_fixed_up						( other.m_field_id ? !strchr( other.m_field_id, '.' ) : true )
{
	lua_mutex_guard					mutex_guard;
	xray::core::configs::g_lua_allocator.user_current_thread_id	( );

	m_field_id						= other.m_field_id ? xray::strings::duplicate( xray::core::configs::g_lua_allocator, other.m_field_id ) : 0;
	initialize						( *other.m_object, *other.m_table_object );
}

lua_config_value& lua_config_value::operator =	( lua_config_value const& other )
{
	lua_mutex_guard					mutex_guard;
	xray::core::configs::g_lua_allocator.user_current_thread_id	( );

	fix_up							( );
	other.fix_up					( );

	finalize						( );
	initialize						( *other.m_object, *other.m_table_object );

	m_field_id						= other.m_field_id ? xray::strings::duplicate( xray::core::configs::g_lua_allocator, other.m_field_id ) : 0;
	m_fixed_up						= other.m_fixed_up;

	return							*this;
}

lua_config_value& lua_config_value::assign_lua_value( lua_config_value const& other )
{
	lua_mutex_guard					mutex_guard;
	xray::core::configs::g_lua_allocator.user_current_thread_id	( );

	fix_up							( );
	other.fix_up					( );

	lua_State* const state			= m_table_object->interpreter();
	if ( !state )
		return						*this = other;

	lua_State* const other_state	= other.m_object->interpreter();
	R_ASSERT_U						( state == other_state );

	m_table_object->push			( state );
	lua_pushstring					( state, m_field_id );

	bool const is_number			= !!lua_isnumber( state, -1 );
	lua_Number number				= -1;
	if ( is_number ) {
		number						= lua_tonumber( state, -1 );
		lua_pushnumber				( state, number );
		lua_replace					( state, -2 );
	}

	other.m_object->push			( state );
	lua_rawset						( state, -3 );
	lua_pop							( state, 1 );

	if ( is_number )
		*m_object					= (*m_table_object)[ number ];
	else
		*m_object					= (*m_table_object)[ m_field_id ];

	return							*this;
}
	
void lua_config_value::fix_up						( ) const
{
	if ( m_fixed_up )
		return;

	lua_mutex_guard					mutex_guard;
	lua_stack_guard					guard;
	fix_up_impl						( );
}

luabind::object const& lua_config_value::get_object	( ) const
{
	lua_mutex_guard					mutex_guard;
	R_ASSERT						( m_object->is_valid() );
	return							*m_object;
}

void lua_config_value::swap				( lua_config_value& other )
{
	lua_mutex_guard					mutex_guard;
	m_object->swap					( *other.m_object );
	m_table_object->swap			( *other.m_table_object );
	std::swap						( m_field_id, other.m_field_id );
	std::swap						( m_fixed_up, other.m_fixed_up );
}

void lua_config_value::initialize			( ::luabind::object const& object, ::luabind::object const& table_object )
{
	COMPILE_ASSERT					( sizeof(luabind::object) == sizeof(m_object_fake), Error_luabind_object_size_changed );
	COMPILE_ASSERT					( sizeof(luabind::object) == sizeof(m_table_object_fake), Error_luabind_object_size_changed );

	lua_mutex_guard					mutex_guard;
	lua_stack_guard					guard;

	if ( object.is_valid() )
		new (m_object) luabind::object	( object );
	else
		new (m_object) luabind::object	( );

	if ( table_object.is_valid() )
		new (m_table_object) luabind::object( table_object );
	else
		new (m_table_object) luabind::object( );

	m_fixed_up						= m_fixed_up && m_object->is_valid( );
}

void lua_config_value::finalize			( )
{
	lua_mutex_guard					mutex_guard;
	lua_stack_guard					guard;

	xray::core::configs::g_lua_allocator.user_current_thread_id	( );

	XRAY_FREE_IMPL					( xray::core::configs::g_lua_allocator, m_field_id );
	(*m_table_object).~object		( );
	(*m_object).~object				( );
}

static void process_string			( xray::strings::stream& stream, pcstr const value, u32 const length )
{
	u32 const buffer_size			= (length+1)*sizeof(char);
	pstr const temp					= (pstr)ALLOCA( 3*buffer_size - 2 );	// for characters below 32: /31

	pcbyte							i = xray::pointer_cast<pcbyte>(value);
	pcbyte							e = xray::pointer_cast<pcbyte>(value) + length;
	pbyte							j = xray::pointer_cast<pbyte>(temp);

	if ((*i == '"') && (length > 1) && (*(e - 1) == '"')) {
		++i;					
		--e;
	}

	for ( ; i != e; ++i, ++j) {
		*j							= *i;
		if (*i != '\\') {
			if (*i != '"') {
				if (*i >= 32)
					continue;

				if (!*i)
					continue;

				*j					= '\\';
				if (*i<10) {
					*++j			= *i + '0';
					continue;
				}
				*++j				= (*i)/10 + '0';
				*++j				= (*i)%10 + '0';
				continue;
			}
			else
				*j					= '\\';
		}

		*++j						= *i;
	}
	*j								= 0;

	if ( temp[0] != '"' ) {
		stream.append				( "\"%s\"", temp );
		return;
	}

	if ( *(j - 1) != '"' ) {
		stream.append				( "\"%s\"", temp+1 );
		return;
	}

	stream.append					( "%s", temp );
}

static inline bool lua_typestring(lua_State* L, int idx)	{ return (lua_type(L, idx) == LUA_TSTRING); }
static inline bool lua_typenumber(lua_State* L, int idx)	{ return (lua_type(L, idx) == LUA_TNUMBER); }


struct lua_key_values_crc {
	pcstr first;
	u32 second;
}; // struct lua_key_values_crc
static lua_key_values_crc	lua_key_values[] = {
	{ "do", 0 },
	{ "end", 0 },

	{ "while", 0 },

	{ "repeat", 0 },
	{ "until", 0 },

	{ "if", 0 },
	{ "then", 0 },
	{ "else", 0 },
	{ "elseif", 0 },

	{ "for", 0 },

	{ "function", 0 },

	{ "local", 0 },

	{ "return", 0 },

	{ "break", 0 },

	{ "nil", 0 },

	{ "false", 0 },
	{ "true", 0 },

	{ "and", 0 },
	{ "or", 0 },
	{ "not", 0 },
};

static bool is_valid_identifier			( pcstr const string)
{
	if ( (*string >= '0') && (*string <= '9') )
		return						false;

	pcstr i							= string;
	pcstr const	e					= string + xray::strings::length(string);
	for ( ; i != e; ++i) {
		if ( (*i >= 'a') && (*i <= 'z'))
			continue;

		if ( (*i >= 'A') && (*i <= 'Z') )
			continue;

		if ( *i == '_' )
			continue;

		if ( (*i >= '0') && (*i <= '9') )
			continue;

		return						false;
	}

	static bool initialized			= false;
	if ( !initialized ) {
		lua_mutex_guard				mutex_guard;
		lua_key_values_crc* i		= lua_key_values;
		lua_key_values_crc* const e	= lua_key_values + xray::array_size(lua_key_values);
		for ( ; i != e; ++i ) {
			boost::crc_32_type		processor;
			processor.process_block	((*i).first, (*i).first + xray::strings::length((*i).first));
			(*i).second				= processor.checksum();
		}
	}

	boost::crc_32_type				processor;
	processor.process_block			( string, e );
	u32 const crc					= processor.checksum();
	{
		lua_key_values_crc* i		= lua_key_values;
		lua_key_values_crc* const e	= lua_key_values + xray::array_size(lua_key_values);
		for ( ; i != e; ++i ) {
			if ( (*i).second != crc )
				continue;

			if ( xray::strings::equal(string, (*i).first) )
				return				false;
		}
	}

	return							true;
}

static xray::configs::enum_types get_type_impl	( lua_State* const state )
{
	lua_mutex_guard					mutex_guard;
	lua_stack_guard					guard(false);

	using namespace xray::configs;

	switch ( lua_type(state, -1) ) {
		case LUA_TBOOLEAN			: return t_boolean;
		case LUA_TNUMBER			: {
			lua_Number const number	= lua_tonumber( state, -1 );
			lua_Number const integer = (lua_Number)lua_tointeger(state,-1);
			if (number != integer)
				return				t_float;

			return					t_integer;
		}
		case LUA_TSTRING			: return t_string;
		case LUA_TTABLE				: {
			lua_pushnil				( state );
			for ( int i=0; lua_next( state, -2 ); ++i ) {
				if ( !lua_typenumber(state, -2) ) {
					pcstr const string = lua_tostring( state, -2 );
					if ( xray::strings::compare( string, s_super_tables_ids_id) && xray::strings::compare( string, s_super_tables_id) ) {
						lua_pop		( state, 2 );
						return		t_table_named;
					}
				}

				lua_pop				( state, 1 );
			}

			return					t_table_indexed;
		}

		case LUA_TUSERDATA			: {
			luabind::detail::object_rep	*object = luabind::detail::is_class_object(state,-1);
			R_ASSERT				( object, "unknown user data in lua config file found");
			R_ASSERT				( object->crep(), "unknown user data in lua config file found");

			luabind::detail::class_rep	*class_rep = object->crep();
			if ( !xray::strings::compare( class_rep->name(), "float2") )
				return				t_float2;
			
			if ( !xray::strings::compare( class_rep->name(), "float3") )
				return				t_float3;
			
			if ( !xray::strings::compare( class_rep->name(), "float4") )
				return				t_float4;

			FATAL					( "unknown class found in lua config: %s", class_rep->name() );
		}
		case LUA_TNONE				: 
		case LUA_TNIL				: 
		case LUA_TFUNCTION			: 
		case LUA_TLIGHTUSERDATA 	: 
		case LUA_TTHREAD			: 
		default						: NODEFAULT(return t_max);
	}
}

typedef std::pair< pcstr, bool >		key_type;
typedef xray::buffer_vector< key_type >	keys_type;

static void push_some_value				( lua_State* const state, key_type const& pair )
{
	lua_mutex_guard		mutex_guard;
	lua_pushstring		( state, pair.first );
	if ( !pair.second ) {
		lua_pushnumber	( state, lua_tonumber(state, -1) );
		lua_replace		( state, -2 );
	}
}

static bool equal_in_super				( lua_State* const state, key_type const& value )
{
	lua_mutex_guard		mutex_guard;
	lua_stack_guard		guard(false);

	if ( !lua_getmetatable(state, -1) )
		return			false;

	if ( lua_isnil(state, -1) ) {
		lua_pop			( state, 1 );
		return			false;
	}

	lua_pushstring		( state, "__index" );
	lua_gettable		( state, -2 );

	if ( !lua_isfunction(state, -1)) {
		lua_pop			( state, 2 );
		return			false;
	}

	lua_pushvalue		( state, -3 );
	lua_pushstring		( state, value.first );
	luabind::detail::pcall	( state, 2, 1 );

	push_some_value		( state, value );
	lua_rawget			( state, -4 );

	int const type0		= lua_type(state, -1);
	int const type1		= lua_type(state, -2);
	bool const result	= (type0 == type1) && lua_equal( state, -1, -2);
	lua_pop				( state, 3 );
	return				result;
}

static void save						(
		lua_State* const state,
		xray::strings::stream& stream,
		pstr const indent,
		u32 const indent_size,
		pcstr indenter,
		u32 const indent_level,
		pcstr identifier
	)
{
	using namespace xray::configs;
	namespace strings = xray::strings;

	lua_mutex_guard					mutex_guard;
	lua_stack_guard					stack_guard(false);

	enum_types const type			= get_type_impl(state);
	switch ( type ) {
		case t_string				: {
			pcstr const string		= lua_tostring( state, -1 );
			process_string			( stream, string, strings::length(string) );
			stream.append			( ",\r\n" );
			break;
		}
		case t_boolean				: {
			stream.append			( "%s,\r\n", lua_toboolean(state,-1) ? "true" : "false" );
			break;
		}
		case t_integer				: {
			stream.append			( "%d,\r\n", lua_tointeger(state, -1) );
			break;
		}
		case t_float				: {
			stream.append			( "%.32g,\r\n", lua_tonumber(state, -1) );
			break;
		}
		case t_float2				: {
			lua_stack_guard			stack_guard(false);
			luabind::object			object( luabind::from_stack(state,-1) );
			xray::math::float2 const vector = *luabind::object_cast<xray::math::float2*>( object );
			stream.append			( "float2(%.32g,%.32g),\r\n", vector.x, vector.y );
			break;
		}
		case t_float3				: {
			lua_stack_guard			stack_guard(false);
			luabind::object			object( luabind::from_stack(state,-1) );
			xray::math::float3		vector = *luabind::object_cast<xray::math::float3*>( object );
			stream.append			( "float3( %.32g, %.32g, %.32g ),\r\n", vector.x, vector.y, vector.z );
			break;
		}
		case t_float4				: {
			lua_stack_guard			stack_guard(false);
			luabind::object			object( luabind::from_stack(state,-1) );
			xray::math::float4		vector = *luabind::object_cast<xray::math::float4*>( object );
			stream.append			( "float4( %.32g, %.32g, %.32g, %.32g ),\r\n", vector.x, vector.y, vector.z, vector.w );
			break;
		}
		case t_table_indexed		:
		case t_table_named			: {
			lua_stack_guard			stack_guard(false);
			pstr const temp			= (pstr)ALLOCA(indent_size*sizeof(char));
			strings::copy			( temp, ( (u32)indent_size )*sizeof(char), indent );

			bool super_table_only	= true;
			bool use_inheritance	= false;
			u32 super_tables_count	= 0;
			u32 total_count			= 0;
			u32 count				= 0;
			lua_pushnil				( state );
			for ( int i=0; lua_next( state, -2 ); ++i, ++total_count ) {
				++count;

				if ( !lua_typenumber(state, -2) ) {
					R_ASSERT		( lua_typestring(state,-2), "invalid value in lua stack" );
					pcstr const string	= lua_tostring( state, -2 );
					if ( super_table_only && !strings::equal(string, s_super_tables_ids_id) && !strings::equal(string, s_super_tables_id) )
						super_table_only	= false;
					else {
						//if ( strings::equal(string, s_super_tables_id) ) {
						//	lua_stack_guard	stack_guard;

						//	lua_getfield		( state, -3, s_super_tables_id );
						//	R_ASSERT			( !lua_isnil(state,-1) );
						//	R_ASSERT_CMP		( lua_type(state,-1), ==, LUA_TTABLE );
						//	lua_pushnil			( state );
						//	if ( !lua_next(state, -2 ) ) {
						//		lua_pop			(state, 1);
						//		LOG_WARNING		( "lua config: empty super tables table found: %s", identifier );
						//	}
						//	else {
						//		lua_pop			(state, 3);
						//		use_inheritance	= true;
						//	}
						//}
						
						if ( strings::equal(string, s_super_tables_ids_id) ) {
							lua_stack_guard	stack_guard;

							lua_getfield		( state, -3, s_super_tables_ids_id );
							R_ASSERT			( !lua_isnil(state,-1) );
							R_ASSERT_CMP		( lua_type(state,-1), ==, LUA_TTABLE );
							lua_pushnil			( state );
							if ( lua_next(state,-2) ) {
								use_inheritance	= true;
								lua_pop			( state, 3 );
							}
							else
								lua_pop			( state, 1 );
						}

						++super_tables_count;
						if ( strings::equal( string, s_super_tables_ids_id) ) {
							if ( lua_objlen( state, -1 ) != 1 )
								super_table_only	= false;
						}
					}
				}

				lua_pop				( state, 1 );
			}

			super_table_only		= super_table_only && (super_tables_count == 2);

			if ( !super_table_only ) {
				if ( indent_level )
					if ( (type != t_table_indexed) && strings::compare( identifier, s_super_tables_ids_id ) && use_inheritance )
						stream.append	( "create_property_table {\r\n", lua_tostring(state, -2) );
					else
						stream.append	( "{\r\n", lua_tostring(state, -2) );

				strings::append		( indent, indent_size, "\t" );
			}

			keys_type				keys( ALLOCA(count*sizeof(key_type)), count );
			lua_pushnil				( state );
			for ( int i=0; lua_next(state,-2); ++i ) {
				if ( lua_typestring(state,-2) ) {
#ifndef MASTER_GOLD
					{
						pcstr const string	= lua_tostring( state, -2 );
						if ( xray::strings::compare( string, s_super_tables_ids_id) && xray::strings::compare( string, s_super_tables_id) )
							R_ASSERT	( type != t_table_indexed );
					}
#endif // #ifndef MASTER_GOLD

					pcstr const string	= lua_tostring( state, -2 );
					keys.push_back		( std::make_pair(string, true) );
					if ( super_table_only && strings::equal( string, s_super_tables_ids_id) ) {
						lua_pushnumber	( state, 1 );
						lua_gettable	( state, -2 );
						stream.append	( "create_reference( \"%s\" ),\r\n", lua_tostring( state, -1 ) );
						lua_pop			( state, 1 );
					}

					lua_pop			( state, 1 );
					continue;
				}
				else {
					if ( lua_typenumber(state, -2) ) {
						lua_pushvalue	( state, -2 );
						pcstr const string	= lua_tostring( state, -1 );
						keys.push_back	( std::make_pair(string, false) );
						lua_pop			( state, 2 );
						continue;
					}

					R_ASSERT			( lua_isnumber(state, -2) || lua_isstring( state, -2), "invalid key value found" );
					UNREACHABLE_CODE();
				}
			}

			if ( super_table_only )
				break;

			struct string_comparer {
				static inline bool compare	( key_type const left, key_type const right )
				{
					return			xray::strings::compare( left.first, right.first ) < 0;
				}
			}; // struct string_comparer

			if ( type != t_table_indexed )
				std::sort			( keys.begin(), keys.end(), string_comparer::compare );

			keys_type::const_iterator	i = keys.begin();
			keys_type::const_iterator	e = keys.end();
			for ( ; i != e; ++i) {
				lua_stack_guard		guard(false);

				if ( equal_in_super(state, *i) )
					continue;

				push_some_value		( state, *i );
				lua_rawget			( state, -2 );

				pcstr const string	= (*i).first;

				if ( (*i).second && strings::equal( string, s_super_tables_id ) ) {
					lua_pop			( state, 1 );
					continue;
				}

				if ( !(*i).second ) {
					stream.append	( "%s", temp );
				}
				else {
					if ( is_valid_identifier(string) )
						stream.append	( "%s%s = ", temp, string );
					else {
						if ( !indent_level )
							stream.append	( "%sthis[", temp, string );
						else
							stream.append	( "%s[", temp, string );

						process_string	( stream, string, (strings::length(string) + 1)*sizeof(char) );
						stream.append	( "] = ", string );
					}
				}

				save				( state, stream, indent, indent_size, temp, indent_level+1, (*i).first );

				lua_pop				( state, 1 );
			}

			strings::copy			( indent, indent_size, temp );

			if ( indent_level )
				stream.append		( "%s}%s -- %s\r\n\r\n", indenter, (indent_level > 1) ? "," : "", identifier );

			break;
		}
		default						: NODEFAULT();
	}
}

static bool is_table				( luabind::object const& object )
{
	lua_mutex_guard					mutex_guard;
	R_ASSERT						( object.is_valid() );
	return							luabind::type(object) == LUA_TTABLE;
}

void lua_config_value::save					( xray::strings::stream& stream, pstr indent, u32 indent_size, u32 indent_level ) const
{
	lua_mutex_guard					mutex_guard;

	fix_up							( );

	R_ASSERT						( m_object->is_valid() );
	R_ASSERT						( is_table(*m_object) );

	lua_stack_guard					guard;

	lua_State* const state			= m_object->interpreter( );
	m_object->push					( state );
	::save							( state, stream, indent, indent_size, "", indent_level, "" );
	lua_pop							( state, 1 );
}

struct table_exist_predicate {
	lua_State*						m_state;

	inline		table_exist_predicate	(lua_State* const state)
	{
		R_ASSERT					(state);
		m_state						= state;
	}

	inline bool	operator()				( int index, pcstr string, u32 length, bool last ) const
	{
		XRAY_UNREFERENCED_PARAMETERS( index, last, length );

		lua_mutex_guard				mutex_guard;
		lua_pushstring				( m_state, string );
#ifndef MASTER_GOLD
		pcstr const type			= lua_typename( m_state, lua_type(m_state, -2) );
#endif // #ifndef MASTER_GOLD
		R_ASSERT					( lua_istable(m_state,-2), "%s", type );
		lua_gettable				(m_state,-2);
		if (!lua_isnil(m_state,-1)) {
			lua_replace				(m_state,-2);
			return					true;
		}

		return						false;
	}
}; // struct table_exist_predicate

struct table_object_predicate {
	lua_State*						m_state;

	inline		table_object_predicate		(lua_State *state)
	{
		R_ASSERT					(state);
		m_state						= state;
	}

	inline bool operator()						(const int &index, pcstr string, const u32 &length, const bool &last) const
	{
		XRAY_UNREFERENCED_PARAMETERS( &index, length, last );

		lua_mutex_guard				mutex_guard;
		lua_pushstring				( m_state,string);
		lua_gettable				( m_state,-2);
		if (!lua_isnil(m_state,-1)) {
			lua_replace				( m_state,-2);
			return					(true);
		}

		FATAL						("cannot find property table '%s'", string );
		return						(false);
	}
}; // struct table_object_predicate

bool lua_config_value::value_exists		( pcstr field_id ) const
{
	lua_mutex_guard					mutex_guard;
	lua_stack_guard					guard;

	if ( !m_object->is_valid() )
		return						( false );

	if ( !is_table(*m_object) )
		return						( false );

	lua_State* const state			= m_object->interpreter();
	m_object->push					( state );
	
	if ( !strings::iterate_items( field_id, table_exist_predicate(state), '.' ) ) {
		lua_pop						( state, 2 );
		return						false;
	}

	lua_pop							( state, 1 );
	return							true;
}

bool lua_config_value::empty				( ) const
{
	lua_mutex_guard					mutex_guard;

	fix_up							( );

	if ( !is_table(*m_object) )
		return						(true);

	return							( begin() == end() );
}

u32 lua_config_value::size					( ) const
{
	lua_mutex_guard					mutex_guard;

	fix_up							( );

	if ( !is_table(*m_object) )
		return						0;

	u32	count						= 0;
	lua_config_iterator i			= begin();
	lua_config_iterator const e		= end();
	for ( ; i != e; ++i )
		++count;

	return							( count );
}

lua_config_value lua_config_value::operator_brackets_impl	( u32 index ) const
{
	++index;

	string16						field_id;
#ifdef _MSC_VER
	errno_t const error				= _itoa_s( index, field_id, 10 );
	XRAY_UNREFERENCED_PARAMETER		( error );
	R_ASSERT						( !error, "%d", index );
#else // #ifdef _MSC_VER
	sprintf							( field_id, "%d", index );
#endif // #ifdef _MSC_VER

	lua_mutex_guard					mutex_guard;
	if ( m_object->is_valid() && is_table(*m_object) ) {
		luabind::object result		= (*m_object)[ index ];
		if ( luabind::type(result) != LUA_TNIL)
			return					lua_config_value( result, *m_object, field_id );

		return						lua_config_value( luabind::object(), *m_object, field_id );
	}

	R_ASSERT						( m_table_object->is_valid() );
	pstr							new_field_id = 0;
	STR_JOINA						( new_field_id, m_field_id, ".", field_id );
	return							lua_config_value( luabind::object(), *m_table_object, new_field_id );
}

lua_config_value lua_config_value::operator_brackets_impl	( int index ) const
{
	lua_mutex_guard					mutex_guard;
	R_ASSERT						( index >= 0 );
	return							(*this)[(u32)index];
}

lua_config_value lua_config_value::operator_brackets_impl	( pcstr field_id ) const
{
	lua_mutex_guard					mutex_guard;

	if ( m_object->is_valid() && is_table(*m_object) ) {
		luabind::object result		= (*m_object)[ field_id ];
		if ( luabind::type(result) != LUA_TNIL)
			return					lua_config_value( result, *m_object, field_id );

		return						lua_config_value( luabind::object(), *m_object, field_id );
	}

	R_ASSERT						( m_table_object->is_valid() );
	pstr new_field_id				= 0;
	STR_JOINA						( new_field_id, m_field_id, ".", field_id );
	return							lua_config_value( luabind::object(), *m_table_object, new_field_id );
}

lua_config_value lua_config_value::operator[ ]	( u32 index )
{
	return	operator_brackets_impl( index );
}

lua_config_value lua_config_value::operator[ ]	( int index )
{
	return	operator_brackets_impl( index );
}

lua_config_value lua_config_value::operator[ ]	( pcstr field_id )
{
	return	operator_brackets_impl( field_id );
}

lua_config_value const lua_config_value::operator[ ]	( u32 index ) const
{
	return	operator_brackets_impl( index );
}

lua_config_value const lua_config_value::operator[ ]	( int index ) const
{
	return	operator_brackets_impl( index );
}

lua_config_value const lua_config_value::operator[ ]	( pcstr field_id ) const
{
	return	operator_brackets_impl( field_id );
}

template < typename T >
static T cast_number				( luabind::object const& object )
{
	lua_mutex_guard					mutex_guard;
	R_ASSERT						( object.is_valid() );
	R_ASSERT_CMP					( luabind::type(object), ==, LUA_TNUMBER );
	return							luabind::object_cast< T >( object );
}

lua_config_value::operator bool			( ) const
{
	fix_up							( );

	lua_mutex_guard					mutex_guard;
	R_ASSERT						( m_object->is_valid() );
	R_ASSERT						( luabind::type(*m_object) == LUA_TBOOLEAN );
	return							luabind::object_cast< bool >( *m_object );
}

lua_config_value::operator s8				( ) const
{
	fix_up							( );

	return							cast_number<s8>( *m_object );
}

lua_config_value::operator u8				( ) const
{
	fix_up							( );

	return							cast_number<u8>( *m_object );
}

lua_config_value::operator s16				( ) const
{
	fix_up							( );

	return							cast_number<s16>( *m_object );
}

lua_config_value::operator u16				( ) const
{
	fix_up							( );

	return							cast_number<u16>( *m_object );
}

lua_config_value::operator s32				( ) const
{
	fix_up							( );

	return							cast_number<s32>( *m_object );
}

lua_config_value::operator u32				( ) const
{
	fix_up							( );

	return							cast_number<u32>( *m_object );
}

lua_config_value::operator float			( ) const
{
	fix_up							( );

	return							cast_number<float>( *m_object );
}

lua_config_value::operator pcstr			( ) const
{
	fix_up							( );

	lua_mutex_guard					mutex_guard;
	R_ASSERT						( m_object->is_valid() );
	//int t =  luabind::type(*m_object);
	R_ASSERT						( luabind::type(*m_object) == LUA_TSTRING );
	return							luabind::object_cast< pcstr >( *m_object );
}

lua_config_value::operator xray::math::float2	( ) const
{
	fix_up							( );

	lua_mutex_guard					mutex_guard;
	R_ASSERT						( m_object->is_valid() );
	R_ASSERT						( luabind::type(*m_object) == LUA_TUSERDATA );
	return							*luabind::object_cast< xray::math::float2*>( *m_object );
}

lua_config_value::operator xray::math::float3	( ) const
{
	fix_up							( );

	lua_mutex_guard					mutex_guard;
	R_ASSERT						( m_object->is_valid() );
	R_ASSERT						( luabind::type(*m_object) == LUA_TUSERDATA );
	return							*luabind::object_cast< xray::math::float3*>( *m_object );
}

lua_config_value::operator xray::math::float4	( ) const
{
	fix_up							( );

	lua_mutex_guard					mutex_guard;
	R_ASSERT						( m_object->is_valid() );
	R_ASSERT						( luabind::type(*m_object) == LUA_TUSERDATA );
	return							*luabind::object_cast< xray::math::float4*>( *m_object );
}

void lua_config_value::fix_up_impl				( ) const
{
	lua_mutex_guard					mutex_guard;
	lua_stack_guard					guard;

	ASSERT							( !m_fixed_up );
	m_fixed_up						= true;

	ASSERT							( *m_field_id );

	ASSERT							( m_table_object->is_valid() );
	ASSERT							( is_table(*m_table_object) );

	u32 const field_buffer_size		= (strings::length(m_field_id) + 1)*sizeof(char);
	pstr temp						= 0;
	STR_DUPLICATEA					( temp, m_field_id );
	pstr							i = temp;
	luabind::object	current			= *m_table_object;
	for (;;) {
		pstr						j = strchr( i, '.' );
		if ( j )
			*j						= 0;

		current.push				( current.interpreter() );
		lua_State* const state		= current.interpreter( );
		lua_pushstring				( state, i );
		bool const is_number		= !!lua_isnumber( state, -1 );
		lua_Number number			= -1;
		if ( is_number ) {
			number					= lua_tonumber( state, -1 );
			lua_pushnumber			( state, number );
			lua_replace				( state, -2 );
		}
		lua_gettable				( state, -2 );
		bool create_table			= true;
		if ( !lua_isnil( state, -1 ) ) {
			if ( (!j && (lua_type( state, -1) != LUA_TNIL)) || (lua_type( state, -1) == LUA_TTABLE) ) {
				lua_pop				( state, 2 );
				create_table		= false;
			}
		}

		if ( create_table ) {
			lua_pop					( state, 1 );

			if ( is_number )
				lua_pushnumber		( state, (lua_Number)number );
			else
				lua_pushstring		( state, i );

			lua_newtable			( state );
			lua_rawset				( state, -3 );

			lua_pop					( state, 1 );
		}

		*m_table_object				= current;
		if ( is_number )
			*m_object				= current[(int)number];
		else
			*m_object				= current[i];

		if ( !j ) {
			strings::copy			( m_field_id, field_buffer_size, i );
			return;
		}

		ASSERT						( j >= i );
		strings::copy				( m_field_id, u32(j - i) + 1, i );
		current						= *m_object;

		i							= j + 1;
		if ( !*i )
			return;
	}
}

static inline bool equal					( luabind::object const& object, pcstr const value )
{
	lua_mutex_guard					mutex_guard;
	lua_stack_guard					guard(false);

	lua_State* const state			= object.interpreter();
	object.push						( state );
	if ( lua_type( state, -1 ) != LUA_TSTRING ) {
		lua_pop						( state, 1 );
		return						false;
	}

	bool const result				= !xray::strings::compare( lua_tostring(state, -1), value );
	lua_pop							( state, 1 );
	return							result;
}

static inline bool equal					( luabind::object const& object, int const value )
{
	lua_mutex_guard					mutex_guard;
	lua_stack_guard					guard(false);

	lua_State* const state			= object.interpreter();
	object.push						( state );
	if ( lua_type( state, -1 ) != LUA_TNUMBER ) {
		lua_pop						( state, 1 );
		return						false;
	}

	bool const result				= lua_tonumber(state, -1) == value;
	lua_pop							( state, 1 );
	return							result;
}

//static inline bool equal					( luabind::object const& object, u32 const value )
//{
//	lua_mutex_guard					mutex_guard;
//	lua_stack_guard					guard(false);
//
//	lua_State* const state			= object.interpreter();
//	object.push						( state );
//	if ( lua_type( state, -1 ) != LUA_TNUMBER ) {
//		lua_pop						( state, 1 );
//		return						false;
//	}
//
//	bool const result				= lua_tonumber(state, -1) == value;
//	lua_pop							( state, 1 );
//	return							result;
//}

static inline bool equal					( luabind::object const& object, float const value )
{
	lua_mutex_guard					mutex_guard;
	lua_stack_guard					guard(false);

	lua_State* const state			= object.interpreter();
	object.push						( state );
	if ( lua_type( state, -1 ) != LUA_TNUMBER ) {
		lua_pop						( state, 1 );
		return						false;
	}

	bool const result				= lua_tonumber(state, -1) == value;
	lua_pop							( state, 1 );
	return							result;
}

static inline bool equal					( luabind::object const& object, xray::math::float2 const& value )
{
	lua_mutex_guard					mutex_guard;
	lua_stack_guard					guard(false);

	lua_State* const state			= object.interpreter();
	object.push						( state );
	if ( lua_type( state, -1 ) != LUA_TUSERDATA ) {
		lua_pop						( state, 1 );
		return						false;
	}

	luabind::detail::object_rep* const object_rep = luabind::detail::is_class_object(state,-1);
	R_ASSERT						( object_rep, "unknown user data in lua config file found");
	R_ASSERT						( object_rep->crep(), "unknown user data in lua config file found");

	luabind::detail::class_rep* const class_rep = object_rep->crep();
	if ( xray::strings::compare( class_rep->name(), "float2") ) {
		lua_pop						( state, 1 );
		return						false;
	}

	xray::math::float2 const vector	= *luabind::object_cast<xray::math::float2*>( object );
	bool const result				= vector.similar(value);
	lua_pop							( state, 1 );
	return							result;
}

static inline bool equal					( luabind::object const& object, xray::math::float3 const& value )
{
	lua_mutex_guard					mutex_guard;
	lua_stack_guard					guard(false);

	lua_State* const state			= object.interpreter();
	object.push						( state );
	if ( lua_type( state, -1 ) != LUA_TUSERDATA ) {
		lua_pop						( state, 1 );
		return						false;
	}

	luabind::detail::object_rep* const object_rep = luabind::detail::is_class_object(state,-1);
	R_ASSERT						( object_rep, "unknown user data in lua config file found");
	R_ASSERT						( object_rep->crep(), "unknown user data in lua config file found");

	luabind::detail::class_rep* const class_rep = object_rep->crep();
	if ( xray::strings::compare( class_rep->name(), "float3") ) {
		lua_pop						( state, 1 );
		return						false;
	}

	xray::math::float3 const vector	= *luabind::object_cast<xray::math::float3*>( object );
	bool const result				= vector.similar(value);
	lua_pop							( state, 1 );
	return							result;
}

static inline bool equal					( luabind::object const& object, xray::math::float4 const& value )
{
	lua_mutex_guard					mutex_guard;
	lua_stack_guard					guard(false);

	lua_State* const state			= object.interpreter();
	object.push						( state );
	if ( lua_type( state, -1 ) != LUA_TUSERDATA ) {
		lua_pop						( state, 1 );
		return						false;
	}

	luabind::detail::object_rep* const object_rep = luabind::detail::is_class_object(state,-1);
	R_ASSERT						( object_rep, "unknown user data in lua config file found");
	R_ASSERT						( object_rep->crep(), "unknown user data in lua config file found");

	luabind::detail::class_rep* const class_rep = object_rep->crep();
	if ( xray::strings::compare( class_rep->name(), "float4") ) {
		lua_pop						( state, -1 );
		return						false;
	}

	xray::math::float4 const vector	= *luabind::object_cast<xray::math::float4*>( object );
	bool const result				= vector.similar(value);
	lua_pop							( state, 1 );
	return							result;
}

lua_config_value lua_config_value::create_table	( )
{
	R_ASSERT						( m_table_object->is_valid() );
	R_ASSERT						( m_field_id );

	lua_stack_guard					guard;

	fix_up							( );

	lua_State* const state			= m_table_object->interpreter( );

	m_table_object->push			( state );
	lua_pushstring					( state, m_field_id );

	bool const is_number			= !!lua_isnumber( state, -1 );
	lua_Number number				= -1;
	if ( is_number ) {
		number						= lua_tonumber( state, -1 );
		lua_pushnumber				( state, number );
		lua_replace					( state, -2 );
	}

	lua_newtable					( state );
	lua_rawset						( state, -3 );
	lua_pop							( state, 1 );

	if ( is_number )
		*m_object					= (*m_table_object)[ number ];
	else
		*m_object					= (*m_table_object)[ m_field_id ];

	return							*this;
}

void lua_config_value::save	( pcstr file_path ){
	configs::lua_config_ptr config = configs::create_lua_config(file_path);
	(*config)["config"].assign_lua_value( *this );
	config->save();
}

void lua_config_value::clear				( )
{
	if ( m_table_object->is_valid() && m_field_id ) {
		create_table				( );
		return;
	}

	lua_stack_guard					guard;

	fix_up							( );

	R_ASSERT						( m_object->is_valid() );

	lua_State* const state			= m_object->interpreter( );
	m_object->~object				( );
	lua_newtable					( state );
	new (m_object) luabind::object	( luabind::from_stack( state, -1 ) );
	lua_pop							( state, 1 );
}

template < typename T >
lua_config_value lua_config_value::assign	( T const& value )
{
	lua_mutex_guard					mutex_guard;

	R_ASSERT						( m_table_object->is_valid() );
	R_ASSERT						( m_field_id );

	lua_stack_guard					guard;

	fix_up							( );

	if ( equal( (*m_table_object)[ m_field_id ], value) )
		return						*this;

	lua_State* const state			= m_table_object->interpreter( );

	m_table_object->push			( state );
	lua_pushstring					( state, m_field_id );

	bool const is_number			= !!lua_isnumber( state, -1 );
	lua_Number number				= -1;
	if ( is_number ) {
		number						= lua_tonumber( state, -1 );
		lua_pushnumber				( state, number );
		lua_replace					( state, -2 );
	}

	luabind::object					object( state, value );
	object.push						( state );
	lua_rawset						( state, -3 );
	lua_pop							( state, 1 );

	if ( is_number )
		*m_object					= (*m_table_object)[ number ];
	else
		*m_object					= (*m_table_object)[ m_field_id ];

	return							*this;
}

lua_config_value lua_config_value::operator =	( pcstr value )
{
	return							assign( value );
}

lua_config_value lua_config_value::operator =	( bool value )
{
	return							assign( value );
}

lua_config_value lua_config_value::operator =	( int value )
{
	return							assign( value );
}

lua_config_value lua_config_value::operator =	( u32 value )
{
	return							assign( (int)value );
}

lua_config_value lua_config_value::operator =	( float value )
{
	return							assign( value );
}

lua_config_value lua_config_value::operator =	( xray::math::float2 const& value )
{
	return							assign( value );
}

lua_config_value lua_config_value::operator =	( xray::math::float3 const& value )
{
	return							assign( value );
}

lua_config_value lua_config_value::operator =	( xray::math::float4 const& value )
{
	return							assign( value );
}

void lua_config_value::erase				( pcstr field_id )
{
	lua_mutex_guard					mutex_guard;
	lua_stack_guard					guard;

	lua_State* const state			= m_object->interpreter();
	m_object->push					( state );
	lua_pushstring					( state, field_id );
	lua_pushnil						( state );
	lua_rawset						( state, -3 );
	lua_pop							( state, 1 );
}

void lua_config_value::rename				( pcstr new_field_id )
{
	lua_mutex_guard					mutex_guard;
	lua_stack_guard					guard;

	xray::core::configs::g_lua_allocator.user_current_thread_id	( );

	fix_up							( );

	R_ASSERT						( m_table_object->is_valid() );
	R_ASSERT						( is_table(*m_table_object) );
	R_ASSERT						( m_object->is_valid() );

	if( 0==strings::compare( new_field_id, m_field_id ) )
		return;

	(*m_table_object)[new_field_id]	= *m_object;

	lua_config_value( *m_table_object ).erase( m_field_id );

	XRAY_FREE_IMPL					( xray::core::configs::g_lua_allocator, m_field_id );
	m_field_id						= strings::duplicate( xray::core::configs::g_lua_allocator, new_field_id );
}

pcstr lua_config_value::get_field_id		( ) const
{
	fix_up							( );
	return							m_field_id;
}

static void adjust_inheritance				( lua_State* const state, luabind::object& parent_table, pcstr const field, luabind::object& value )
{
	lua_stack_guard				guard;

	parent_table.push			( state );
	R_ASSERT					( !lua_isnil(state,-1) );

	lua_pushstring				( state, "create_property_table" );
	lua_gettable				( state, LUA_GLOBALSINDEX );
	R_ASSERT					( !lua_isnil(state,-1), "cannot get create_property_table. Probably, your current directory is invalid, or file _G.lua is corrupted" );

	value.push					( state );
	R_ASSERT					( !lua_isnil(state,-1) );

	int const error				= luabind::detail::pcall( state, 1, 1 );
	XRAY_UNREFERENCED_PARAMETER	( error );
	R_ASSERT					( !error, lua_tostring( state, -1) );

	lua_pushstring				( state, field );
	lua_insert					( state, -2 );
	lua_settable				( state, -3 );

	lua_pop						( state, 1 );

	value						= parent_table[field];
}

void lua_config_value::add_super_table		( lua_config_value const& super_table )
{
	lua_mutex_guard					mutex_guard;
	lua_stack_guard					guard;

	super_table.fix_up				( );
	R_ASSERT						( is_table(*super_table.m_object) );

	fix_up							( );
	R_ASSERT						( is_table(*m_object) );

	lua_State* const state			= m_object->interpreter( );
	adjust_inheritance				( state, *m_table_object, m_field_id, *m_object );

	luabind::object super_tables	= (*m_object)[s_super_tables_id];
	if ( luabind::type(super_tables) != LUA_TTABLE ) {
		m_object->push				( state );
		lua_pushstring				( state, s_super_tables_id );
		
		lua_newtable				( state );
		lua_pushnumber				( state, 1 );
		super_table.m_object->push	( state );
		lua_rawset					( state, -3 );

		lua_rawset					( state, -3 );

		lua_pop						( state, 1 );
		return;
	}

	u32 count						= 1;
	luabind::iterator				i(super_tables);
	luabind::iterator const			e;
	for ( ; i != e; ++i, ++count ) {
		R_ASSERT					( luabind::type(i.key()) == LUA_TNUMBER, "invalid super table found" );
		if ( *i == *super_table.m_object )
			return;
	}

	super_tables.push				( state );
	lua_pushnumber					( state, count );
	super_table.m_object->push		( state );
	lua_rawset						( state, -3 );

	lua_pop							( state, 1 );
}

void lua_config_value::add_super_table		( pcstr const file_table, pcstr const super_table )
{
	lua_mutex_guard					mutex_guard;
	lua_stack_guard					guard;

	fs::verify_path_is_portable		( file_table );

	bool need_fix_up				= !m_fixed_up;
	if ( !need_fix_up && (!m_object->is_valid() || !is_table(*m_object)) )
		need_fix_up					= true;

	if ( need_fix_up ) {
		m_fixed_up					= false;
		fix_up						( );
	}

	R_ASSERT						( is_table(*m_object) );

	lua_State* const state			= m_object->interpreter();
	luabind::object const modules	= luabind::globals( state )["modules"];
	
	//lua_config_value modules_value(modules);
	//configs::lua_config_ptr tst = configs::create_lua_config("d:/lua.tst");
	//

	//configs::lua_config_value::iterator begin	= modules_value.begin( );
	//configs::lua_config_value::iterator end		= modules_value.end( );
	//for (;begin != end;++begin){
	//	(*tst)[( pcstr )begin.key()] = *begin;
	//}

	//tst->save();

	luabind::object const file		= modules[file_table];
	R_ASSERT						( file.is_valid(), "cannot open file %s", file_table );

	file.push						( state );
	R_ASSERT						( lua_istable(state,-1), "cannot open file %s", file_table );

	bool const no_errors			= strings::iterate_items( super_table, table_exist_predicate(state), '.' );
	if ( !no_errors ) {
		lua_pop						( state, 2 );
		return;
	}

	luabind::object					table( luabind::from_stack(state,-1) );
	lua_pop							( state, 1 );
	add_super_table					( lua_config_value( table ) );

	pstr temp						= 0;
	STR_JOINA						( temp, "modules['", file_table, "'].", super_table );

	luabind::object super_tables	= (*m_object)[s_super_tables_ids_id];
	if ( luabind::type(super_tables) != LUA_TTABLE ) {
		(*m_object).push			( state );
		lua_pushstring				( state, s_super_tables_ids_id );

		lua_newtable				( state );
		lua_pushnumber				( state, 1 );
		lua_pushstring				( state, temp );
		lua_rawset					( state, -3 );

		lua_rawset					( state, -3 );
		lua_pop						( state, 1 );
		return;
	}

	u32 count						= 1;
	luabind::iterator				i(super_tables);
	luabind::iterator const			e;
	for ( ; i != e; ++i, ++count ) {
		R_ASSERT					( luabind::type(*i) == LUA_TSTRING );
		if ( strings::equal( luabind::object_cast<pcstr>(*i), temp ) )
			return;
	}

	super_tables[count]				= (pcstr)temp;
}

static void remove_inheritance				( lua_State* const state, luabind::object& object )
{
	lua_stack_guard					guard;

	object.push						( state );
	lua_pushnil						( state );
	lua_setmetatable				( state, -2 );

	lua_pushstring					( state, s_super_tables_id );
	lua_pushnil						( state );
	lua_rawset						( state, -3 );

	lua_pushstring					( state, s_super_tables_ids_id );
	lua_pushnil						( state );
	lua_rawset						( state, -3 );

	lua_pop							( state, 1 );
}

void lua_config_value::remove_super_table	( lua_config_value const& super_table )
{
	lua_mutex_guard					mutex_guard;
	lua_stack_guard					guard;

	super_table.fix_up				( );
	R_ASSERT						( is_table(*super_table.m_object) );

	fix_up							( );
	R_ASSERT						( is_table(*m_object) );

	R_ASSERT						( is_table(*m_object) );
	lua_State* const state			= m_object->interpreter( );
	luabind::object super_tables	= (*m_object)[s_super_tables_id];
	if ( luabind::type(super_tables) != LUA_TTABLE )
		return;

	u32 real_count					= 0;
	u32 count						= 1;
	lua_newtable					( state );
	luabind::iterator				i(super_tables);
	luabind::iterator const			e;
	for ( ; i != e; ++i, ++count ) {
		R_ASSERT					( luabind::type(i.key()) == LUA_TNUMBER, "invalid super table found" );
		if ( *i == *super_table.m_object )
			continue;

		++real_count;
		lua_pushnumber				( state, count );
		(*i).push					( state );
		lua_rawset					( state, -3 );
	}

	if ( !real_count) {
		lua_pop						( state, 1 );
		lua_pushnil					( state );
		remove_inheritance			( state, *m_object );
	}

	luabind::object					object( luabind::from_stack(state,-1) );
	lua_pop							( state, 1 );
	(*m_object)[s_super_tables_id]	= object;
}

void lua_config_value::remove_super_table	( pcstr const file_table, pcstr const super_table )
{
	lua_mutex_guard					mutex_guard;
	lua_stack_guard					guard;

	fs::verify_path_is_portable		( file_table );

	fix_up							( );
	R_ASSERT						( is_table(*m_object) );

	lua_State* const state			= m_object->interpreter();
	luabind::object const modules	= luabind::globals( state )["modules"];
	luabind::object const file		= modules[file_table];

	file.push						( state );
	bool const no_errors			= strings::iterate_items( super_table, table_exist_predicate(state), '.' );
	if ( !no_errors )
		return;

	luabind::object					table( luabind::from_stack(state,-1) );
	lua_pop							( state, 1 );
	remove_super_table				( lua_config_value( table ) );

	luabind::object super_tables	= (*m_object)[s_super_tables_ids_id];
	if ( luabind::type(super_tables) != LUA_TTABLE ) {
		(*m_object).push			( state );
		lua_pushstring				( state, s_super_tables_ids_id );
		lua_newtable				( state );
		lua_rawset					( state, -3 );
		lua_pop						( state, 1 );
		return;
	}

	pstr temp						= 0;
	STR_JOINA						( temp, "modules['", file_table, "'].", super_table );

	u32 count						= 1;
	lua_newtable					( state );
	luabind::iterator				i(super_tables);
	luabind::iterator const			e;
	for ( ; i != e; ++i, ++count ) {
		R_ASSERT					( luabind::type(*i) == LUA_TSTRING );
		if ( strings::equal( luabind::object_cast<pcstr>(*i), temp ) )
			continue;

		lua_pushnumber				( state, count );
		(*i).push					( state );
		lua_rawset					( state, -3 );
	}

	luabind::object					object( luabind::from_stack(state,-1) );
	lua_pop							( state, 1 );
	(*m_object)[s_super_tables_ids_id]	= object;
}

void lua_config_value::remove_from_parent		( )
{
	lua_mutex_guard					mutex_guard;

	xray::core::configs::g_lua_allocator.user_current_thread_id	( );

	if ( !m_fixed_up )
		fix_up						( );

	R_ASSERT						( m_object->is_valid() );
	R_ASSERT						( m_table_object->is_valid() );
	R_ASSERT						( is_table(*m_table_object) );

	lua_State* const state			= m_table_object->interpreter( );
	lua_pushnil						( state );
	luabind::object					temp( luabind::from_stack(state,-1) );
	lua_pop							( state, 1 );
	(*m_table_object)[m_field_id]	= temp;

	m_table_object->~object			( );
	XRAY_FREE_IMPL					( core::configs::g_lua_allocator, m_field_id );
	new (m_table_object) luabind::object( );
}

xray::configs::enum_types lua_config_value::get_type( ) const
{
	lua_mutex_guard					mutex_guard;
	lua_stack_guard					guard(false);

	fix_up							( );
	
	lua_State* const state			= m_object->interpreter();
	m_object->push					( state );
	enum_types const result			= get_type_impl( state );
	lua_pop							( state, 1 );
	return							result;
}

lua_config_value::iterator lua_config_value::begin	( )
{
	lua_stack_guard					stack_guard;
	lua_mutex_guard					mutex_guard;
	return							((lua_config_value const*)this)->begin();
}

lua_config_value::iterator lua_config_value::end	( )
{
	lua_stack_guard					stack_guard;
	lua_mutex_guard					mutex_guard;
	return							((lua_config_value const*)this)->end();
}

lua_config_value::const_iterator lua_config_value::begin( ) const
{
	lua_stack_guard					guard;

	fix_up							( );

	R_ASSERT						( m_object->is_valid() );
	R_ASSERT						( is_table(*m_object) );

	return							lua_config_iterator( luabind::iterator(*m_object) );
}

lua_config_value::const_iterator lua_config_value::end	( ) const
{
	lua_mutex_guard					mutex_guard;
	lua_stack_guard					guard;

	fix_up							( );

	R_ASSERT						( m_object->is_valid() );
	R_ASSERT						( is_table(*m_object) );

	return							lua_config_iterator( luabind::iterator() );
}

using xray::configs::lua_config_iterator;

lua_config_iterator::lua_config_iterator				( ::luabind::iterator const& iterator ) :
	m_iterator						( xray::pointer_cast< luabind::iterator* >(&m_iterator_fake[0]) )
{
	COMPILE_ASSERT					( sizeof(luabind::iterator) == sizeof(m_iterator_fake), Error_luabind_object_iterator_size_changed );

	lua_mutex_guard					mutex_guard;
	lua_stack_guard					guard;

	new (m_iterator) luabind::iterator( iterator );

	fix_up							( );
}

lua_config_iterator::lua_config_iterator				( lua_config_iterator const& other ) :
	m_iterator						( xray::pointer_cast< luabind::iterator* >(&m_iterator_fake[0]) )
{
	COMPILE_ASSERT					( sizeof(luabind::iterator) == sizeof(m_iterator_fake), Error_luabind_object_iterator_size_changed );

	lua_mutex_guard					mutex_guard;
	lua_stack_guard					guard;

	new (m_iterator) luabind::iterator( *other.m_iterator );
}

lua_config_iterator::~lua_config_iterator				( )
{
	lua_mutex_guard					mutex_guard;
	lua_stack_guard					guard;

	m_iterator->~basic_iterator		( );
}

lua_config_iterator& lua_config_iterator::operator =	( lua_config_iterator const& other )
{
	lua_mutex_guard					mutex_guard;
	lua_stack_guard					guard;

	*m_iterator						= *other.m_iterator;

	return							*this;
}

lua_config_value lua_config_iterator::key				( ) const
{
	lua_mutex_guard					mutex_guard;
	return							m_iterator->key();
}

lua_config_value lua_config_iterator::operator *		( ) const
{
	lua_mutex_guard					mutex_guard;
	return							lua_config_value( **m_iterator );
}

bool lua_config_iterator::operator ==					( lua_config_iterator const& other ) const
{
	lua_mutex_guard					mutex_guard;
	return							*m_iterator == *other.m_iterator;
}

bool lua_config_iterator::operator !=					( lua_config_iterator const& other ) const
{
	lua_mutex_guard					mutex_guard;
	return							*m_iterator != *other.m_iterator;
}

void lua_config_iterator::fix_up						( )
{
	for ( ; (*m_iterator != luabind::iterator() ) && (luabind::type(m_iterator->key()) == LUA_TSTRING); ++*m_iterator ) {
		if ( strings::equal( luabind::object_cast<pcstr>(m_iterator->key()), s_super_tables_id) )
			continue;

		if ( strings::equal( luabind::object_cast<pcstr>(m_iterator->key()), s_super_tables_ids_id) )
			continue;

		break;
	}
}

lua_config_iterator& lua_config_iterator::operator ++	( )
{
	lua_mutex_guard					mutex_guard;
	++*m_iterator;

	fix_up							( );

	return							*this;
}

lua_config_iterator lua_config_iterator::operator ++	( int )
{
	lua_mutex_guard					mutex_guard;
	lua_config_iterator result		= *this;
	++*this;
	return							result;
}

#include <luabind/operator.hpp>
#include <luabind/return_reference_to_policy.hpp>

static void export_classes						( )
{
	lua_stack_guard					guard;

	using namespace xray;
	using math::float2;
	using math::float2_pod;
	using math::float3;
	using math::float3_pod;
	using math::float4;
	using math::float4_pod;

	using namespace luabind;

	struct platform_helper {
		static pcstr platform_id( )
		{
			return
#if XRAY_PLATFORM_XBOX_360
				"xbox360"
#elif XRAY_PLATFORM_PS3 // #if XRAY_PLATFORM_XBOX_360
				"ps3"
#elif XRAY_PLATFORM_WINDOWS_32 // #elif XRAY_PLATFORM_PS3
				"win32"
#elif XRAY_PLATFORM_WINDOWS_64 // #elif XRAY_PLATFORM_WINDOWS_32
				"win64"
#else // #elif XRAY_PLATFORM_WINDOWS_64
#	error please define your platform id here
#endif // #if XRAY_PLATFORM_XBOX_360
			;
		}
	};

	module( s_world->virtual_machine() )
	[
		namespace_( "xray" )[
			def( "platform", platform_helper::platform_id )
		]

		,class_< float2_pod >( "float2_pod" )
			.def( constructor<>() )
			.def( const_self +  other<float2_pod>() )
			.def( const_self -  other<float2_pod>() )
			.def( const_self *  other<float>()  )
			.def( other<float>() * const_self	)
			.def( const_self /  other<float>()	)
			.def( other<float>() / const_self	)
			.def( const_self <  other<float2_pod>() )
			.def( const_self <= other<float2_pod>() )
			.def( const_self == other<float2_pod>() )

		,class_< float2 >( "float2" )
			.def( constructor<>() )
			.def( constructor< float, float >() )
			.def( const_self +  other<float2_pod>() )
			.def( const_self -  other<float2_pod>() )
			.def( const_self *  other<float>()  )
			.def( other<float>() * const_self	)
			.def( const_self /  other<float>()	)
			.def( other<float>() / const_self	)
			.def( const_self <  other<float2_pod>() )
			.def( const_self <= other<float2_pod>() )
			.def( const_self == other<float2_pod>() )
			.def( "dot_product",		&float2::dot_product )
			.def( "cross_product",		&float2::cross_product,		return_reference_to(_1) )
			.def( "normalize",			&float2::normalize,			return_reference_to(_1) )
			.def( "normalize_r",		&float2::normalize_r )
			.def( "normalize_safe",		&float2::normalize_safe,	return_reference_to(_1) )
			.def( "normalize_safe_r",	&float2::normalize_safe_r )
			.def( "abs",				&float2::abs,				return_reference_to(_1) )
			.def( "min",				&float2::min,				return_reference_to(_1) )
			.def( "max",				&float2::max,				return_reference_to(_1) )
			.def( "magnitude",			&float2::magnitude )
			.def( "square_magnitude",	&float2::square_magnitude )
			.def( "similar",			&float2::similar )
			.def( "valid",				&float2::valid )

		,def( "dot_product",			(float (*)(float2_pod const&,float2_pod const&)) &math::dot_product )
		,def( "cross_product",			(float2 (*)(float2_pod const&)) &math::cross_product )
		,def( "normalize",				(float2 (*)(float2_pod const&)) &math::normalize )
		,def( "normalize_safe",			(float2 (*)(float2_pod const&,float2_pod const&)) &math::normalize_safe )

		,class_< float3_pod >( "float3_pod" )
			.def( constructor<>() )
			.def( const_self +  other<float3_pod>() )
			.def( const_self -  other<float3_pod>() )
			.def( const_self *  other<float3_pod>() )
			.def( const_self /  other<float3_pod>() )
			.def( const_self <  other<float3_pod>() )
			.def( const_self <= other<float3_pod>() )
			.def( const_self == other<float3_pod>() )

		,class_< float3, float3_pod >( "float3" )
			.def( constructor<>() )
			.def( constructor< float, float, float >() )
			.def( const_self +  other<float3_pod>() )
			.def( const_self -  other<float3_pod>() )
			.def( const_self *  other<float3_pod>() )
			.def( const_self /  other<float3_pod>() )
			.def( const_self <  other<float3_pod>() )
			.def( const_self <= other<float3_pod>() )
			.def( const_self == other<float3_pod>() )
			.def( "dot_product",		&float3::dot_product )
			.def( "cross_product",		&float3::cross_product,		return_reference_to(_1) )
			.def( "normalize",			&float3::normalize,			return_reference_to(_1) )
			.def( "normalize_r",		&float3::normalize_r )
			.def( "normalize_safe",		&float3::normalize_safe,	return_reference_to(_1) )
			.def( "normalize_safe_r",	&float3::normalize_safe_r )
			.def( "abs",				&float3::abs,				return_reference_to(_1) )
			.def( "min",				&float3::min,				return_reference_to(_1) )
			.def( "max",				&float3::max,				return_reference_to(_1) )
			.def( "magnitude",			&float3::magnitude )
			.def( "square_magnitude",	&float3::square_magnitude )
			.def( "similar",			&float3::similar )
			.def( "valid",				&float3::valid )

		,def( "dot_product",			(float (*)(float3_pod const&,float3_pod const&)) &math::dot_product )
		,def( "cross_product",			(float3 (*)(float3_pod const&,float3_pod const&)) &math::cross_product )
		,def( "normalize",				(float3 (*)(float3_pod const&)) &math::normalize )
		,def( "normalize_safe",			(float3 (*)(float3_pod const&,float3_pod const&)) &math::normalize_safe )
		,def( "normalize_precise",		(float3 (*)(float3_pod const&)) &math::normalize_precise )
		,def( "abs",					(float3 (*)(float3_pod const&)) &math::abs )
		,def( "min",					(float3 (*)(float3_pod const&,float3_pod const&)) &math::min )
		,def( "max",					(float3 (*)(float3_pod const&,float3_pod const&)) &math::max )
		,def( "magnitude",				(float (*)(float3_pod const&)) &math::magnitude )
		,def( "square_magnitude",		(float (*)(float3_pod const&)) &math::square_magnitude )
		,def( "similar",				(bool (*)(float3_pod const&,float3_pod const&,float)) &math::similar )
		,def( "valid",					(bool (*)(float3_pod const&)) &math::valid )

		,class_< float4_pod >( "float4_pod" )
			.def( constructor<>() )
			.def( const_self +  other<float4_pod>() )
			.def( const_self -  other<float4_pod>() )
			.def( const_self /  other<float>() )
			.def( const_self <  other<float4_pod>() )
			.def( const_self <= other<float4_pod>() )
			.def( const_self == other<float4_pod>() )

		,class_< float4, float4_pod >( "float4" )
			.def( constructor<>() )
			.def( constructor< float, float, float, float >() )
			.def( const_self +  other<float4_pod>() )
			.def( const_self -  other<float4_pod>() )
			.def( const_self /  other<float>() )
			.def( const_self <  other<float4_pod>() )
			.def( const_self <= other<float4_pod>() )
			.def( const_self == other<float4_pod>() )
			.def( "normalize",			&float4::normalize,			return_reference_to(_1) )
			.def( "normalize_r",		&float4::normalize_r )
			.def( "normalize_safe",		&float4::normalize_safe,	return_reference_to(_1) )
			.def( "normalize_safe_r",	&float4::normalize_safe_r )
			.def( "abs",				&float4::abs,				return_reference_to(_1) )
			.def( "min",				&float4::min,				return_reference_to(_1) )
			.def( "max",				&float4::max,				return_reference_to(_1) )
			.def( "magnitude",			&float4::magnitude )
			.def( "square_magnitude",	&float4::square_magnitude )
			.def( "similar",			&float4::similar )
			.def( "valid",				&float4::valid )

		,def( "normalize",				(float4 (*)(float4_pod const&)) &math::normalize )
		,def( "normalize_safe",			(float4 (*)(float4_pod const&,float4_pod const&)) &math::normalize_safe )
	];
}
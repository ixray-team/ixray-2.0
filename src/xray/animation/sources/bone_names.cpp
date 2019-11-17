////////////////////////////////////////////////////////////////////////////
//	Created		: 04.02.2010
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "bone_names.h"
#include "skeleton.h"

#include <xray/configs_lua_config.h>

namespace xray {
namespace animation {

void	bone_names::set_bones_number( bone_index_type size )
{
	m_bone_names.resize( size );
}

void	bone_names::set_name( bone_index_type bone_index, pcstr name )
{
	m_bone_names[bone_index] = name;
}

void	bone_names::read( xray::configs::lua_config_value const &cfg )
{
	const bone_index_type size = cfg.size();
	m_bone_names.resize( size );
	for( bone_index_type i = 0; i < size; ++i )
		m_bone_names[i] = string_type ( static_cast<pcstr>( cfg[i] ) );
}

void	bone_names::write( xray::configs::lua_config_value	&cfg )const
{
	const bone_index_type size = m_bone_names.size( );
	for( bone_index_type i = 0; i < size; ++i )
		 cfg[i] = m_bone_names[i].c_str();
}

bone_index_type	bone_names::bone_index( const string_type &name )const
{
	vector < string_type >::const_iterator r =	 std::find( m_bone_names.begin(), m_bone_names.end(),  name  );
	if( r == m_bone_names.end() )
		return ( u32(-1) );
	return u32( r - m_bone_names.begin() );
}

void	bone_names::create_index( const skeleton &skel, vector< bone_index_type > &index )const
{
	const bone_index_type sz = skel.bone_count();

	ASSERT( sz <= m_bone_names.size( ) );

	index.resize( sz );

	for( bone_index_type i = 0; i < sz; ++i )
		index[i] = bone_index( skel.get_bone( i ).id() );

}

} // namespace animation
} // namespace xray
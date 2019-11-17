////////////////////////////////////////////////////////////////////////////
//	Created		: 07.12.2009
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "skeleton.h"
#include <xray/configs_lua_config.h>

using xray::animation::bone;
using xray::animation::bone_index_type;
using xray::animation::skeleton;
using xray::configs::lua_config_value;
using xray::strings::compare;

bone::bone				( 
		pcstr const id,
		bone const* const parent,
		bone const* const children_begin,
		bone const* const children_end,
		bone_index_type const index
	) :
	m_id					( id ),
	m_parent				( parent ),
	m_children_begin		( children_begin ),
	m_children_end			( children_end ),
#ifdef DEBUG
	m_count					( bone_index_type(children_end - children_begin) ),
#endif // #ifdef DEBUG
	m_index					( index )
{
}

static u32 get_bone_count	( lua_config_value const& config )
{
	u32 count				= 0;

	lua_config_value::const_iterator i			= config.begin();
	lua_config_value::const_iterator const e	= config.end();
	for ( ; i != e; ++i, ++count )
		count				+= get_bone_count( *i );

	return					count;
}

static void add_bone		(
		bone const* parent,
		bone* bones_begin,
		bone_index_type index,
		bone_index_type& last_index,
		xray::configs::lua_config_iterator const& config
	)
{
	lua_config_value const value	= *config;
	u32 const children_count		= value.size();

	new (bones_begin+index) bone(
		config.key(),
		parent,
		children_count ? bones_begin + last_index : 0,
		children_count ? bones_begin + last_index + children_count : 0,
		index
	);

	if ( !children_count )
		return;

	bone_index_type const old_last_index = last_index;
	last_index				+= children_count;
	xray::configs::lua_config_iterator i	= value.begin();
	for (u32 new_index=old_last_index, n=new_index + children_count; new_index != n; ++new_index, ++i )
		add_bone			( bones_begin + index, bones_begin, new_index, last_index, i );
}

skeleton::skeleton		( lua_config_value const& config )
{
	u32 bone_count			= get_bone_count( config );
	u32 const buffer_size	= bone_count*sizeof(bone);
	bone* buffer			= static_cast<bone*>( MALLOC(buffer_size, "skeleton") );

	m_bones					= buffer;
	m_bone_count			= bone_count;

	ASSERT_CMP				( config.size(), ==, 1 );
	u32 last_index			= 1;
	add_bone				( 0, m_bones, 0, last_index, config.begin() );
}

skeleton::~skeleton		( )
{
	bone* i					= m_bones;
	bone* const e			= m_bones + m_bone_count;
	for ( ; i != e; ++i )
		i->~bone			( );

	FREE					( m_bones );
}



struct find_pred
{
	
	find_pred( pcstr bone_name ): m_bone_name( bone_name )
	{}

	bool operator () ( const bone& b )
	{
		return ( 0 == compare( b.id(), m_bone_name ) ) ;
	}

	pcstr	m_bone_name;
	

}; // struct find_pred

bone_index_type		xray::animation::skeleton_bone_index( const skeleton & skel, pcstr bone_name )
{
	const u32 bone_count = skel.bone_count();
	const bone *begin = &skel.get_bone(0) , *end = ( &skel.get_bone( bone_count-1 ) + 1 );
	
	find_pred fp( bone_name );

	const bone *res = std::find_if( begin, end, fp  );

	if( res == end )
		return bone_index_type( -1 );
	
	return res->index();

}
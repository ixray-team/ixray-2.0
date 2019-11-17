////////////////////////////////////////////////////////////////////////////
//	Created		: 01.04.2009
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include <xray/configs.h>

using xray::configs::binary_config_value;

binary_config_value::binary_config_value						( ) : 
	id		(NULL),
	data	(NULL),
	crc		(0),
	type	(0),
	count	(0)
{
}

static pcstr str_types[] = 
{
	"empty",
	"u32",
	"float",
	"table_named",
	"table_indexed",
	"string",
	"float2",
	"float3",
};

void binary_config_value::dump							(pcstr prefix) const
{
	LOG_DEBUG	(
		"%scrc=[%u] name=[%s] type=[%s] count=%d", 
		prefix ? prefix : "",
		crc, 
		id, 
		str_types[type],
		(type==t_table_named || type==t_table_indexed) ? count : 0
	);

	if (type==t_table_named || type==t_table_indexed)
	{
		binary_config_value::const_iterator it	= begin();
		binary_config_value::const_iterator it_e	= end();
		for(; it!=it_e; ++it)
		{
			binary_config_value const& itm	= *it;
			pstr pref			= 0;
			STR_JOINA			(pref, prefix ? prefix : "", " " );
			itm.dump			(pref);
		}
	}
}
#include <boost/crc.hpp>

binary_config_value const& binary_config_value::operator[]	( pcstr key ) const
{
	R_ASSERT					(type == t_table_named);
	const_iterator it			= begin();
	const_iterator it_e			= end();

	boost::crc_32_type			processor;
	processor.process_block		(key, key + strings::length(key));
	u32 const crc				= processor.checksum();

#if 1
	const_iterator const result	= std::lower_bound(it, it_e, crc);
	
	R_ASSERT					(result!=it_e, "item not found [%s]", key);
	R_ASSERT					(result->crc==crc, "item not found [%s]", id);
	return						*result;
#else
	for(; it!=it_e; ++it)
		if( (*it).crc == crc )
			return				*it;
#endif
}

bool binary_config_value::value_exists					( pcstr key ) const
{
	R_ASSERT					(type == t_table_named);
	const_iterator it			= begin();
	const_iterator it_e			= end();

	boost::crc_32_type			processor;
	processor.process_block		(key, key + strings::length(key));
	u32 const crc				= processor.checksum();

	const_iterator const result	= std::lower_bound(it, it_e, crc);
	if ( result == it_e)
		return					false;

	if ( result->crc == crc )
		return					false;

	return						true;
}

void binary_config_value::fix_up						( u32 const offset )
{
	if ( id )
		id						+= offset;

	switch ( type ) {
		case t_boolean :
		case t_integer :
		case t_float : {
			break;
		}
		case t_string :
		case t_float2 :
		case t_float3 :
		case t_float4 : {
			(pcbyte&)(data)		+= offset;
			break;
		}
		case t_table_indexed :
		case t_table_named : {
			(pcbyte&)(data)		+= offset;

			for ( u32 i=0, n=count; i < n; ++i )
				((binary_config_value*)data + i)->fix_up( offset );

			break;
		}
		default : NODEFAULT();
	}
}
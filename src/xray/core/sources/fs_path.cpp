////////////////////////////////////////////////////////////////////////////
//	Created		: 06.05.2010
//	Author		: Sergey Chechin
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include <xray/fs_path.h>

namespace xray {
namespace fs {

path_string   convert_to_portable (pcstr const path)
{
	path_string	res					=	path;
	for ( path_string::iterator	it	=	res.begin();
								it	!=	res.end();
								++it )
	{
		if ( *it == '\\' )
		{
			*it						=	'/';
		}
	}

	return								res;
}

pstr convert_to_portable_in_place( pstr const path )
{
	for ( pstr i=strchr(path, '\\'); i; *i='/', i=strchr(i, '\\') );
	return path;
}

} // namespace fs
} // namespace xray
////////////////////////////////////////////////////////////////////////////
//	Created		: 19.05.2010
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include <xray/fs_utils.h>
#include "fs_path_iterator.h"

namespace xray {
namespace fs {

void   verify_path_is_portable (pcstr path_str)
{
	XRAY_UNREFERENCED_PARAMETER	( path_str );
	ASSERT						( path_str );
	ASSERT						( !strchr(path_str, '\\') );

#ifndef MASTER_GOLD
	u32 const path_length	=	strings::length(path_str);
	R_ASSERT					( !path_length || path_str[path_length-1] != '/', 
								  "path should not end with '/' ('%s')", path_str );
#endif // #ifdef DEBUG
}

void   file_name_with_no_extension_from_path (buffer_string * const out_result, pcstr const path)
{
	verify_path_is_portable			(path);

	pcstr file_name_start		=	0;
	
	if ( pcstr const last_slash_pos = strrchr(path, '/') )
		file_name_start			=	last_slash_pos + 1;
	else
		file_name_start			=	path;

	pcstr const last_point_pos	=	strrchr(path, '.');

	if ( last_point_pos && last_point_pos > file_name_start )
		out_result->assign			(file_name_start, last_point_pos);
	else
		* out_result			=	file_name_start;
}

pcstr   extension_from_path (pcstr path)
{
	verify_path_is_portable			(path);
	
	if ( pcstr const last_point_pos = strrchr(path, '.') )
		return						last_point_pos + 1;
	else
		return						path + strings::length(path);
}

void   directory_part_from_path (path_string * const out_result, pcstr const path)
{
	verify_path_is_portable			(path);

	if ( pcstr const last_slash_pos = strrchr(path, '/') )
		out_result->assign			(path, last_slash_pos);
	else
		* out_result			=	"";
}

bool   make_dir_r (pcstr dir_path, bool create_last)
{
	pcstr colon_pos				=	strchr(dir_path, ':');
	
	bool const absolute_path	=	!!colon_pos;
	pcstr	disk_letter_end		=	absolute_path ? colon_pos + 1 : dir_path;

	path_string		cur_path;
	cur_path.append					(dir_path, disk_letter_end);
	if ( absolute_path )
		cur_path				+=	'/';
	u32 const drive_part_length	=	absolute_path ? (u32(disk_letter_end - dir_path) + 1) : 0; // with '/'

	path_iterator	it				(dir_path + drive_part_length, // skip '/' if absolute path
									 int(strings::length(dir_path) - drive_part_length));

	path_iterator	end_it		=	path_iterator::end();
	ASSERT							( it != end_it );

	bool			result		=	true;

	while ( it != end_it )
	{
		fs::path_string				part;
		it.to_string				(part);
		cur_path				+=	part;

		path_iterator next_it	=	it;
		++next_it;

		if ( !create_last && next_it == end_it )
			return					result;

		result					&=	make_dir(cur_path.c_str());
		cur_path				+=	"/";
		it						=	next_it;
	}

	return							result;
}

pcstr   file_name_from_path (pcstr const path)
{
	verify_path_is_portable			(path);

	if ( pcstr const last_slash_pos = strrchr(path, '/') )
		return						last_slash_pos + 1;
	else
		return						path;
}

signalling_bool   calculate_file_size (file_size_type * const out_file_size, pcstr const file_path)
{
	FILE * file				=	NULL;
	if ( !open_file(& file, open_file_read, file_path, false) )
		return					false;
	
	bool const out_result	=	calculate_file_size	(out_file_size, file);
	fclose						(file);
	return						out_result;
}

} // namespace fs
} // namespace xray
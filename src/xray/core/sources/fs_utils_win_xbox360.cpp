////////////////////////////////////////////////////////////////////////////
//	Created		: 06.05.2010
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include <xray/fs_utils.h>	// for fs::open_file_enum, ...
#include <xray/fs_path.h>	// for fs::convert_to_native
#include <fcntl.h>			// for _O_RDWR, _O_RDONLY, _O_WRONLY, ...
#include <share.h>			// for _SH_DENYWR, _SH_DENYNO
#include <sys/stat.h>		// for _S_IREAD, _S_IWRITE
#include <direct.h>			// for _mkdir

namespace xray {
namespace fs {

#ifdef DEBUG
static 
fixed_string512   file_open_flags_to_string (u32 const file_open_flags)
{
	fixed_string512					out_string;
	if ( file_open_flags & _O_RDWR )
		out_string				+=	" RDWR";
	if ( file_open_flags & _O_RDONLY )
		out_string				+=	" RD";
	if ( file_open_flags & _O_WRONLY )
		out_string				+=	" WR";
	if ( file_open_flags & _O_CREAT )
		out_string				+=	" CREATE";
	if ( file_open_flags & _O_TRUNC )
		out_string				+=	" TRUNC";
	if ( file_open_flags & _O_APPEND )
		out_string				+=	" APPEND";

	return							out_string;
}
#endif // #ifdef DEBUG

signalling_bool   open_file (FILE** const						out_file,
							enum_flags<open_file_enum> const	open_flags, 
							pcstr const							portable_path,
							bool								assert_on_fail)
{
	ASSERT							(out_file);
	verify_path_is_portable			(portable_path);
	path_string	native_path		=	convert_to_native(portable_path);

	int	file_open_flags			=	_O_BINARY | _O_SEQUENTIAL;
	int permission_flags		=	((open_flags & open_file_read)  ? _S_IREAD  : 0) | 
									((open_flags & open_file_write) ? _S_IWRITE : 0);
	int sh_permission_flags		=	(open_flags & open_file_write) ? _SH_DENYWR : _SH_DENYNO;

	if ( (open_flags & open_file_read) && (open_flags & open_file_write) )
		file_open_flags			|=	_O_RDWR;
	else if ( (open_flags & open_file_read) )
		file_open_flags			|=	_O_RDONLY;
	else if ( (open_flags & open_file_write) )
		file_open_flags			|=	_O_WRONLY;
	else 
		FATAL("%s", "one or both read or write flags must be specified");

	if ( open_flags & open_file_create )
		file_open_flags			|=	_O_CREAT;
	if ( open_flags & open_file_truncate )
		file_open_flags			|=	_O_TRUNC;
	if ( open_flags & open_file_append )
		file_open_flags			|=	_O_APPEND;

	int		file_handle			=	0;
	errno_t error_code			=	_sopen_s(& file_handle, native_path.c_str(), file_open_flags, 
											 sh_permission_flags, permission_flags);

	if ( error_code ) 
	{
		threading::yield		(1);
		error_code				=	_sopen_s(& file_handle, native_path.c_str(), file_open_flags,		
											 sh_permission_flags, permission_flags);
	}

	bool const success			=	!error_code && file_handle != -1;
	pcstr const fopen_flags		=	"r+b";

	* out_file					=	success ? _fdopen(file_handle, fopen_flags) : NULL;

	if ( assert_on_fail )
		ASSERT						(*out_file,
									 "failed opening file:%s with open flags:%s",
									 native_path.c_str(),
									 file_open_flags_to_string(file_open_flags).c_str());

	return							!!* out_file;
}

signalling_bool   calculate_file_size (file_size_type * const out_file_size, FILE * const file)
{
	ASSERT						(out_file_size);
	CURE_ASSERT					(file, return false);
	* out_file_size			=	(file_size_type)_filelengthi64(_fileno(file));
	return						true;
}

bool   make_dir (pcstr path)
{
	verify_path_is_portable			(path);
	path_string	native_path		=	convert_to_native(path);

	_unlink	(native_path.c_str());
	if ( _mkdir (native_path.c_str()) == -1 )
	{
		errno_t err;
		_get_errno(&err);
		return err == EEXIST ? true : false;
	}

	return true;
}

path_info::type_enum   get_path_info (path_info * out_path_info, pcstr path)
{
	verify_path_is_portable			(path);
	_finddata32i64_t				file_desc;
	intptr_t const handle		=	_findfirst32i64(convert_to_native(path).c_str(), 
													& file_desc);
	if ( handle	==	-1 )
	{
		path_info::type_enum const out_path_type	=	path_info::type_nothing;
		if ( out_path_info )
			out_path_info->type	=	out_path_type;
		return						out_path_type;
	}
	
	path_info::type_enum const out_path_type	=	!!(file_desc.attrib & _A_SUBDIR) ? path_info::type_folder : path_info::type_file;
	if ( out_path_info )
		out_path_info->type		=	out_path_type;

	if ( out_path_info )
		out_path_info->file_last_modify_time	=	file_desc.time_write;

	return							out_path_type;
}

} // namespace fs
} // namespace xray
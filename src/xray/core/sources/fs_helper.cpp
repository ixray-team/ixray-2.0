////////////////////////////////////////////////////////////////////////////
//	Created		: 06.05.2010
//	Author		: Sergey Chechin
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "fs_helper.h"
#include <xray/fs_path.h>
#include "fs_path_iterator.h"
#include <boost/crc.hpp>
#include <xray/fs_utils.h>

namespace xray {
namespace fs {

// @post: position in files are changed
bool   files_equal (FILE* f1, FILE* f2, u32 size)
{
	fseek							(f1, 0, SEEK_SET);
	fseek							(f2, 0, SEEK_SET);
	u32 const	chunk_size		=	128*1024;
	char		file1_chunk			[chunk_size];
	char		file2_chunk			[chunk_size];
	u32			size_left		=	size;

	while ( size_left != 0 )
	{
		u32 const work_size		=	size_left < chunk_size ? size_left : chunk_size;
		u32 const bytes_read1	=	(u32)fread(file1_chunk, 1, work_size, f1);
		R_ASSERT_U					(bytes_read1 == work_size);
		u32 const bytes_read2	=	(u32)fread(file2_chunk, 1, work_size, f2);
		R_ASSERT_U					(bytes_read2 == work_size);
		
		if ( memcmp(file1_chunk, file2_chunk, work_size) )
		{
			return false;
		}

		size_left				-=	work_size;
	}

	return							true;
}

// @post: position in files are changed
void   copy_data (FILE* f_dest, FILE* f_src, u32 size, u32* hash)
{
	u32 const	chunk_size		=	128*1024;
	char		file_chunk			[chunk_size];
	u32			size_left		=	size;

	if ( hash )
	{
		*hash					=	0;
	}

	while ( size_left != 0 )
	{
		u32 const work_size		=	size_left < chunk_size ? size_left : chunk_size;
		u32 const bytes_read	=	(u32)fread(file_chunk, 1, work_size, f_src);
		if ( hash )
		{
			*hash				=	crc32(file_chunk, bytes_read, *hash);
		}
		R_ASSERT					(bytes_read == work_size);
		u32 const bytes_written	=	(u32)fwrite(file_chunk, 1, work_size, f_dest);
		XRAY_UNREFERENCED_PARAMETER	( bytes_written );
		R_ASSERT					(bytes_written == work_size);
		size_left				-=	work_size;
	}
}

typedef	boost::crc_optimal<32, 0x04C11DB7, 0, 0, true, false>	crc_processor;

u32   crc32 (pcstr const data, u32 const size, u32 const start_value)
{
	crc_processor	processor	(start_value);
	processor.process_bytes		(data, size);
	return processor.checksum() & u32(-1);
}

u32   path_crc32 (pcstr const data, u32 const size, u32 const start_value)
{
	crc_processor	processor	(start_value);

	int start_index						=	-1;
	int cur_index						=	0;
	while ( cur_index < (int)size )
	{
		const bool is_path_separator	=	(data[cur_index] == '/') || 
											(data[cur_index] == '\\');
		if ( is_path_separator )
		{
			if ( start_index != -1 )
			{
				processor.process_block		(data+start_index, data+cur_index);
				start_index				=	-1;
			}
		}
		else if ( start_index == -1 )
		{
			start_index					=	cur_index;
		}
		
		++cur_index;
	}
	
	if ( start_index != -1 )
	{
		processor.process_block				(data+start_index, data+cur_index);
	}

	return processor.checksum				();
}

bool   file_exists (pcstr path)
{
	FILE* f								=	0;
	if ( !open_file(&f, open_file_read, path, false) )
		return								false;

	fclose									(f);
	return									true;
}

} // namespace fs
} // namespace xray
////////////////////////////////////////////////////////////////////////////
//	Created		: 15.04.2010
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "ogg_utils.h"
#include <xray/fs_utils.h>

namespace xray {
namespace sound {

namespace ogg_utils {


int ov_seek_func(void* datasource, s64 offset, int whence)	
{
	ogg_file_source* sdata				= (ogg_file_source*)datasource;
	resources::pinned_ptr_const<u8> pdata	( sdata->resource );
	memory::reader	r		(pdata.c_ptr(), pdata.size());
	r.seek					(sdata->pointer);
	switch (whence)
	{
	case SEEK_SET: 
		r.seek		((int)offset);	 
		break;

	case SEEK_CUR: 
		r.advance	((int)offset); 
		break;

	case SEEK_END: 
		r.seek		((int)offset + r.length());
		break;
	}
	sdata->pointer	= r.tell( );
	return 0; 
}

size_t ov_read_func(void *ptr, size_t size, size_t nmemb, void *datasource)
{ 
	ogg_file_source* sdata				= (ogg_file_source*)datasource;
	resources::pinned_ptr_const<u8> pdata	( sdata->resource );

	memory::reader r		(pdata.c_ptr(), pdata.size());
	r.seek					(sdata->pointer);

	size_t exist_block	= math::max	(0ul, math::floor(r.elapsed()/(float)size));
	size_t read_block	= math::min	(exist_block, nmemb);
	
	size_t bytes_to_read = read_block*size;

	r.r					(ptr, (u32)bytes_to_read, (u32)bytes_to_read);
	sdata->pointer	= r.tell( );
	return				read_block;
}

int ov_close_func(void *datasource)									
{	
	XRAY_UNREFERENCED_PARAMETERS(datasource);
	return				0;
}

long ov_tell_func(void *datasource)									
{	
	ogg_file_source* sdata				= (ogg_file_source*)datasource;
	return sdata->pointer;
}

u32 decompress(OggVorbis_File* ovf, pbyte dest, u32& pcm_pointer, u32 bytes_needed)
{
	int			current_section;
	u32			total_readed	= 0;
	long			ret;

	ov_pcm_seek		(ovf, pcm_pointer);

	// Read loop
	while(total_readed < bytes_needed)
	{
		char*	dest_ptr	= (char*)(dest+total_readed);
		ret					= ov_read(	ovf, 
										dest_ptr, 
										bytes_needed-total_readed, 
										0, 
										2, 
										1, 
										&current_section
									);

		// if end of file or read limit exceeded
		if (ret == 0) 
			break;
		else 
		if (ret < 0)
			LOG_ERROR("Error in vorbis bitstream");
		else
			total_readed += ret;
	}
	
	pcm_pointer				= (u32)ov_pcm_tell( ovf );

	return					total_readed;
}

void	encode_sound_file	(	fs::path_string const& input_file_path,
								fs::path_string const& output_file_path,	
								int bits_per_sample,
								int number_of_chanels,
								int samples_per_second,
								int output_bitrate
							)
{
	XRAY_UNREFERENCED_PARAMETERS(&input_file_path, &output_file_path, bits_per_sample, number_of_chanels, samples_per_second, output_bitrate);
#if XRAY_PLATFORM_WINDOWS

	////make path's
	//if(fs::get_path_info(NULL, dir.c_str()) == fs::path_info::type_nothing)
	//{
	//	fs::make_dir_r(dir.c_str());
	//}

	STARTUPINFO		si	= {0};
	si.cb			= sizeof(STARTUPINFO);
	si.wShowWindow	= 0;
	si.dwFlags		= STARTF_USESHOWWINDOW;

	PROCESS_INFORMATION		pi = {0};

//.	char buf[12];
	fs::path_string		run_string;
	run_string		+= "oggenc2.exe";
	//run_string 		+= " -B ";
	//_itoa_s			(bits_per_sample, buf, 10);
	//run_string		+= buf;
	//run_string 		+= " -C ";
	//_itoa_s			(number_of_chanels, buf, 10);
	//run_string		+= buf;
	//run_string 		+= " -R ";
	//_itoa_s			(samples_per_second, buf, 10);
	//run_string		+= buf;
	//run_string		+= " -b ";
	//_itoa_s			(output_bitrate, buf, 10);
	//run_string		+= buf;
	run_string 		+= " -o \"";

	run_string		+= "../../";

	run_string		+= output_file_path.c_str();
	run_string 		+= "\" \"";
	
	run_string		+= "../../";

	run_string		+= input_file_path.c_str();
	run_string		+= "\"";
	LOG_INFO("%s", run_string.c_str());

	fs::path_string		full_output_file_path	=	"../../";
	full_output_file_path	+=	output_file_path;
	fs::make_dir_r		(full_output_file_path.c_str(), false);

	if(CreateProcess(NULL, run_string.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
		WaitForSingleObject(pi.hProcess, INFINITE);
#endif // #if XRAY_PLATFORM_WINDOWS
}

void make_sound_rms( fs::path_string& end_file_path )
{
	XRAY_UNREFERENCED_PARAMETER	( end_file_path );

#if XRAY_PLATFORM_WINDOWS
	//make path's
	fs::path_string			file_path;

	file_path				= end_file_path;
	file_path.set_length	(file_path.length()-8);
	file_path.append		(".ogg");

	STARTUPINFO				si;
	ZeroMemory				( &si, sizeof(si) );
	si.cb					= sizeof(STARTUPINFO);
	si.wShowWindow			= 0;
	si.dwFlags				= STARTF_USESHOWWINDOW;

	PROCESS_INFORMATION		pi;
	ZeroMemory				( &pi, sizeof(pi) );

	fixed_string512			cl_args;
	cl_args					+= "xray_rms_generator-debug.exe";
	cl_args 				+= " -o=\"";
	cl_args 				+= end_file_path.c_str();
	cl_args 				+= "\" -i=\"";
	cl_args 				+= file_path.c_str();
	cl_args 				+= "\"";

	if(CreateProcess(NULL, cl_args.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
		WaitForSingleObject(pi.hProcess, INFINITE);
#endif // #if XRAY_PLATFORM_WINDOWS
}

}; // namespace ogg_utils

} // namespace sound
} // namespace xray

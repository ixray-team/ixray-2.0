////////////////////////////////////////////////////////////////////////////
//	Created		: 12.02.2009
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include <xray/console_command.h>
#include <xray/console_command_processor.h>
#include <xray/fs_utils.h>

extern xray::console_commands::console_command* s_console_command_root;

namespace xray {
namespace console_commands{


static void show_help(console_command* command)
{
	console_command::info_str	info_buff;
	command->info				(info_buff);
	pcstr out_str				= NULL;
	STR_JOINA					(out_str, command->name(), ":", info_buff);
	LOG_INFO					(out_str);
}

console_command* find(pcstr str)
{
	console_command* result = NULL;

	console_command* current = s_console_command_root;

	while(current)
	{
		if( 0==strings::compare(current->name(), str) )
			break;

		current = current->prev();
	}
	result	= current;
	return	result;
}

u32 get_similar(pcstr starts_from, console_command** dst, u32 dst_size)
{
	u32 result = 0;
	console_command* current = s_console_command_root;

	while(current && result<dst_size)
	{
		pcstr current_name = current->name();
		if( current_name==strstr(current_name, starts_from) )
		{
			*dst	= current;
			++dst;
			++result;
		}
		current = current->prev();
	}

	return		result;
}

void execute(pcstr command_to_execute)
{
	pcstr pos	= strchr(command_to_execute, ' ');

	pcstr cmd	= NULL;
	if(pos)
	{
		size_t len	= pos-command_to_execute+1;//+zero

		pstr p		= ( pstr )ALLOCA(len);
		strings::copy_n	(p, len, command_to_execute, len-1);
		cmd			= p;
	}else
		cmd = command_to_execute;

	pcstr args	= (pos) ? pos+1 : NULL;

	console_command* command	= find(cmd);
	if(!command)
	{
		LOG_WARNING("unknown command [%s]", cmd);
		return;
	}

	if( (args==NULL || 0==strings::length(args)) && command->need_args())
	{
		console_command::status_str	buff;
		command->status				(buff);
		LOG_INFO					(buff);
	}else
		command->execute	(args);
}


void show_help(pcstr str)
{
	if(str)
	{
		console_command* command	= find(str);
		if(!command)
		{
			LOG_ERROR	("unknown command [%s]", str);
			return;
		}else
			show_help	(command);
		return;
	}

	console_command* current = s_console_command_root;
	while(current)
	{
		show_help		(current);
		current			= current->prev();
	}
}

void save(pcstr name)
{

	if(!name)
	{
	//	LOG_INFO("specify file name please");
	///	return;
		name ="startup.cfg";
	} 

	FILE*		f;
	pcstr fn = 0;
	STR_JOINA(fn,"../../resources/", name);
	if(!fs::open_file(&f, fs::open_file_write | fs::open_file_create | fs::open_file_truncate, fn, false))
	{
		LOG_INFO("unable to open file [%s]", fn);
		return;
	}

	console_command* current = s_console_command_root;
	pcstr out_str			= NULL;
	while(current)
	{
		if(current->serializable())
		{
			console_command::status_str	buff;
			current->status	(buff);
			STR_JOINA		(out_str, current->name(), " ", buff, "\n");
			fwrite			(out_str, sizeof(char), strings::length(out_str), f);
		}
		current				= current->prev();
	}

	fclose		(f);
}

static bool is_line_term (char a) {	return (a==13)||(a==10); };

static u32	advance_term_string(memory::reader& F)
{
	u32 sz		= 0;
	while (!F.eof()) 
	{
		F.advance(1);				//Pos++;
        sz++;
		if (!F.eof() && is_line_term(*(char*)F.pointer()) ) 
		{
        	while(!F.eof() && is_line_term(*(char*)F.pointer())) 
				F.advance(1);		//Pos++;
			break;
		}
	}
    return sz;
}

static void r_string(memory::reader& F, string4096& dest)
{
	char *src 	= (char *) F.pointer();
	u32 sz 		= advance_term_string(F);
	xray::strings::copy_n	(dest, sizeof(dest), src, sz);
}

void load(memory::reader& F)
{
	string4096				str;
	while (!F.eof())
	{
		r_string			(F, str);
		execute				(str);
	}
}

} //namespace console_commands
} //namespace xray